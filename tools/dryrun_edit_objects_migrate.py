#!/usr/bin/env python3
"""
Dry-run migrazione stock edit 34k → object_instance (Fase 2, solo lettura).

Scorre objects/<vnum> nel range LOW_EDITED_ITEMS..HIGH_EDITED_ITEMS, propone
base_vnum e classifica i pezzi. Non scrive su MySQL né rinomina/cancella file.

Uso tipico (da Server/ o ovunque):

  python3 tools/dryrun_edit_objects_migrate.py \\
    --objects-dir mudroot/lib/objects \\
    --myst-obj mudroot/lib/myst.obj \\
    --findoriginal-csv mudroot/lib/edited_objects_findoriginal.csv \\
    --output /tmp/edit-migrate-dryrun.csv

Opzionale impatto rent (lento ~secondi su migliaia di file):

  ... --rent-dir mudroot/lib/rent

Exit code: 0 sempre se lo scan completa; summary su stderr.
"""

from __future__ import annotations

import argparse
import csv
import re
import struct
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Set, Tuple

LOW_EDITED = 34030
HIGH_EDITED = 35999

ITEM2_EDIT = 2
ITEM2_PERSONAL = 8
ITEM2_INSERT = 16

# Stesso layout di audit-pg-equip-recovery.py / reception rent.
RENT_HEADER_FMT = "<20s5i"
RENT_HEADER_SIZE = struct.calcsize(RENT_HEADER_FMT)
RENT_ELEM_NEW_FMT = "<Hxx4iiiiI128s128s256sBBxx" + "hxxi" * 5 + "i"
RENT_ELEM_NEW_SIZE = struct.calcsize(RENT_ELEM_NEW_FMT)
RENT_ELEM_OLD_FMT = "<Hxx4iiiiI128s128s256sBBxx" + "hxxi" * 5
RENT_ELEM_OLD_SIZE = struct.calcsize(RENT_ELEM_OLD_FMT)

ED_TOKEN_RE = re.compile(r"^ED(.+)$", re.IGNORECASE)
VNUM_HEADER_RE = re.compile(r"^#(\d+)\s*$")
MYST_VNUM_RE = re.compile(r"^#(\d+)\s*$")


@dataclass
class ParsedEdit:
    edit_vnum: int
    path: Path
    header_vnum: int = 0
    name_keys: str = ""
    short_desc: str = ""
    long_desc: str = ""
    type_flag: int = 0
    extra_flags: int = 0
    wear_flags: int = 0
    value0: int = 0
    value1: int = 0
    value2: int = 0
    value3: int = 0
    weight: int = 0
    cost: int = 0
    cost_per_day: int = 0
    extra_flags2: int = 0
    affect_count: int = 0
    ed_owners: List[str] = field(default_factory=list)
    parse_error: str = ""


@dataclass
class DryRunRow:
    edit_vnum: int
    header_vnum: int
    proposed_base: int
    status: str
    notes: str
    short_desc: str
    name_keys: str
    ed_owners: str
    type_flag: int
    cost: int
    cost_per_day: int
    extra_flags2: int
    has_edit_flag: int
    has_personal_flag: int
    has_insert_flag: int
    affect_count: int
    findorig_proto1: int
    findorig_score1: int
    findorig_short1: str
    base_in_world: int
    rent_refs: int
    path: str


def is_edit_range(vnum: int) -> bool:
    return LOW_EDITED <= vnum <= HIGH_EDITED


def read_tilde_string(lines: List[str], idx: int) -> Tuple[str, int]:
    """Legge una stringa Circle terminata da ~ (anche multi-riga)."""
    parts: List[str] = []
    while idx < len(lines):
        line = lines[idx]
        idx += 1
        if "~" in line:
            parts.append(line[: line.index("~")])
            break
        parts.append(line)
    return "\n".join(parts), idx


def parse_object_file(path: Path, edit_vnum: int) -> ParsedEdit:
    out = ParsedEdit(edit_vnum=edit_vnum, path=path)
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        out.parse_error = f"read_error:{exc}"
        return out

    lines = text.splitlines()
    if not lines:
        out.parse_error = "empty_file"
        return out

    m = VNUM_HEADER_RE.match(lines[0].strip())
    if not m:
        out.parse_error = f"bad_header:{lines[0][:40]!r}"
        return out
    out.header_vnum = int(m.group(1))

    idx = 1
    try:
        out.name_keys, idx = read_tilde_string(lines, idx)
        out.short_desc, idx = read_tilde_string(lines, idx)
        out.long_desc, idx = read_tilde_string(lines, idx)
        _action, idx = read_tilde_string(lines, idx)

        if idx >= len(lines):
            out.parse_error = "truncated_after_strings"
            return out
        tew = lines[idx].split()
        idx += 1
        if len(tew) < 3:
            out.parse_error = "bad_type_extra_wear"
            return out
        out.type_flag = int(tew[0])
        out.extra_flags = int(tew[1])
        out.wear_flags = int(tew[2])

        if idx >= len(lines):
            out.parse_error = "truncated_values"
            return out
        vals = lines[idx].split()
        idx += 1
        if len(vals) < 4:
            out.parse_error = "bad_values"
            return out
        out.value0, out.value1, out.value2, out.value3 = map(int, vals[:4])

        if idx >= len(lines):
            out.parse_error = "truncated_weight_cost"
            return out
        wcc = lines[idx].split()
        idx += 1
        if len(wcc) < 3:
            out.parse_error = "bad_weight_cost"
            return out
        out.weight = int(wcc[0])
        out.cost = int(wcc[1])
        out.cost_per_day = int(wcc[2])

        while idx < len(lines):
            tag = lines[idx].strip()
            idx += 1
            if not tag:
                continue
            if tag == "E":
                _kw, idx = read_tilde_string(lines, idx)
                _desc, idx = read_tilde_string(lines, idx)
            elif tag == "A":
                if idx >= len(lines):
                    break
                idx += 1  # loc mod
                out.affect_count += 1
            elif tag == "F":
                if idx >= len(lines):
                    break
                out.extra_flags2 = int(lines[idx].split()[0])
                idx += 1
            elif tag == "P":
                _a, idx = read_tilde_string(lines, idx)
                _b, idx = read_tilde_string(lines, idx)
            else:
                # ignora sezioni sconosciute / coda
                break
    except (ValueError, IndexError) as exc:
        out.parse_error = f"parse:{exc}"
        return out

    for tok in out.name_keys.split():
        em = ED_TOKEN_RE.match(tok)
        if em:
            out.ed_owners.append(em.group(1))
    return out


def load_myst_vnums(myst_obj: Path) -> Set[int]:
    found: Set[int] = set()
    with myst_obj.open(encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = MYST_VNUM_RE.match(line)
            if m:
                found.add(int(m.group(1)))
    return found


def load_findoriginal(path: Path) -> Dict[int, Tuple[int, int, str]]:
    """edit_vnum → (proto1_vnum, proto1_score, proto1_short)."""
    out: Dict[int, Tuple[int, int, str]] = {}
    with path.open(newline="", encoding="utf-8", errors="replace") as fh:
        reader = csv.DictReader(fh)
        for row in reader:
            try:
                edit = int(row["edit_vnum"])
                proto = int(row.get("proto1_vnum") or 0)
                score = int(row.get("proto1_score") or 0)
                short = (row.get("proto1_short") or "").strip()
            except (KeyError, ValueError):
                continue
            out[edit] = (proto, score, short)
    return out


def iter_edit_files(objects_dir: Path) -> Iterable[Path]:
    for path in sorted(objects_dir.iterdir(), key=lambda p: p.name):
        if not path.is_file() or not path.name.isdigit():
            continue
        vnum = int(path.name)
        if is_edit_range(vnum):
            yield path


def propose_base(
    parsed: ParsedEdit,
    findorig: Dict[int, Tuple[int, int, str]],
) -> Tuple[int, str, str, int, int, str]:
    """
    Returns: proposed_base, status, notes, fo_vnum, fo_score, fo_short
    """
    fo_vnum, fo_score, fo_short = findorig.get(parsed.edit_vnum, (0, 0, ""))

    if parsed.parse_error:
        return 0, "PARSE_ERROR", parsed.parse_error, fo_vnum, fo_score, fo_short

    header = parsed.header_vnum
    if header > 0 and not is_edit_range(header):
        return header, "OK_HEADER", "base from file #header", fo_vnum, fo_score, fo_short

    notes = []
    if header == parsed.edit_vnum:
        notes.append("header_eq_edit_vnum")
    elif is_edit_range(header):
        notes.append(f"header_in_edit_range:{header}")
    else:
        notes.append(f"bad_header:{header}")

    if fo_vnum > 0 and not is_edit_range(fo_vnum):
        return (
            fo_vnum,
            "NEED_BASE_HINT",
            ",".join(notes) + f";findoriginal_proto1={fo_vnum}",
            fo_vnum,
            fo_score,
            fo_short,
        )

    if fo_vnum > 0 and is_edit_range(fo_vnum):
        notes.append(f"findoriginal_proto1_also_edit:{fo_vnum}")

    return 0, "NEED_BASE", ",".join(notes), fo_vnum, fo_score, fo_short


def base_exists(base: int, world: Set[int], objects_dir: Path) -> bool:
    if base <= 0:
        return False
    if base in world:
        return True
    return (objects_dir / str(base)).is_file()


def detect_rent_elem_size(file_size: int, num_objs: int) -> int:
    if num_objs > 0:
        for elem_size in (RENT_ELEM_NEW_SIZE, RENT_ELEM_OLD_SIZE):
            if RENT_HEADER_SIZE + num_objs * elem_size == file_size:
                return elem_size
    for elem_size in (RENT_ELEM_NEW_SIZE, RENT_ELEM_OLD_SIZE):
        rem = file_size - RENT_HEADER_SIZE
        if rem > 0 and rem % elem_size == 0:
            return elem_size
    return RENT_ELEM_NEW_SIZE


def scan_rent_refs(rent_dir: Path, edit_vnums: Set[int]) -> Dict[int, int]:
    """Conta occorrenze di item_number (vnum) nei rent file."""
    counts: Counter[int] = Counter()
    for path in rent_dir.iterdir():
        if not path.is_file():
            continue
        name = path.name
        if name.endswith(".aux") or name.startswith("."):
            continue
        try:
            data = path.read_bytes()
        except OSError:
            continue
        if len(data) < RENT_HEADER_SIZE:
            continue
        try:
            _owner, _gold, _cost, _upd, _stay, nitems = struct.unpack_from(
                RENT_HEADER_FMT, data, 0
            )
        except struct.error:
            continue
        elem_size = detect_rent_elem_size(len(data), nitems)
        if nitems < 0 or RENT_HEADER_SIZE + nitems * elem_size > len(data):
            nitems = max(0, (len(data) - RENT_HEADER_SIZE) // elem_size)
        off = RENT_HEADER_SIZE
        for _ in range(nitems):
            if off + elem_size > len(data):
                break
            item_number = struct.unpack_from("<H", data, off)[0]
            if item_number in edit_vnums:
                counts[item_number] += 1
            off += elem_size
    return dict(counts)


def build_rows(
    objects_dir: Path,
    world: Set[int],
    findorig: Dict[int, Tuple[int, int, str]],
    rent_refs: Dict[int, int],
) -> List[DryRunRow]:
    rows: List[DryRunRow] = []
    for path in iter_edit_files(objects_dir):
        edit_vnum = int(path.name)
        parsed = parse_object_file(path, edit_vnum)
        proposed, status, notes, fo_v, fo_s, fo_short = propose_base(parsed, findorig)
        in_world = 1 if base_exists(proposed, world, objects_dir) else 0
        if status == "OK_HEADER" and not in_world:
            status = "OK_HEADER_MISSING"
            notes = (notes + ";base_not_in_myst_obj_or_objects").strip(";")
        elif status == "NEED_BASE_HINT" and not in_world:
            notes = (notes + ";hint_base_not_in_world").strip(";")

        rows.append(
            DryRunRow(
                edit_vnum=edit_vnum,
                header_vnum=parsed.header_vnum,
                proposed_base=proposed,
                status=status,
                notes=notes,
                short_desc=parsed.short_desc.replace("\n", " ")[:120],
                name_keys=parsed.name_keys.replace("\n", " ")[:80],
                ed_owners=",".join(parsed.ed_owners),
                type_flag=parsed.type_flag,
                cost=parsed.cost,
                cost_per_day=parsed.cost_per_day,
                extra_flags2=parsed.extra_flags2,
                has_edit_flag=1 if (parsed.extra_flags2 & ITEM2_EDIT) else 0,
                has_personal_flag=1 if (parsed.extra_flags2 & ITEM2_PERSONAL) else 0,
                has_insert_flag=1 if (parsed.extra_flags2 & ITEM2_INSERT) else 0,
                affect_count=parsed.affect_count,
                findorig_proto1=fo_v,
                findorig_score1=fo_s,
                findorig_short1=fo_short[:80],
                base_in_world=in_world,
                rent_refs=rent_refs.get(edit_vnum, 0),
                path=str(path),
            )
        )
    return rows


def write_csv(path: Path, rows: List[DryRunRow]) -> None:
    fields = [
        "edit_vnum",
        "header_vnum",
        "proposed_base",
        "status",
        "notes",
        "short_desc",
        "name_keys",
        "ed_owners",
        "type_flag",
        "cost",
        "cost_per_day",
        "extra_flags2",
        "has_edit_flag",
        "has_personal_flag",
        "has_insert_flag",
        "affect_count",
        "findorig_proto1",
        "findorig_score1",
        "findorig_short1",
        "base_in_world",
        "rent_refs",
        "path",
    ]
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            writer.writerow({k: getattr(row, k) for k in fields})


def print_summary(rows: List[DryRunRow]) -> None:
    by_status = Counter(r.status for r in rows)
    print(f"Edit files scanned: {len(rows)} ({LOW_EDITED}..{HIGH_EDITED})", file=sys.stderr)
    for status, n in sorted(by_status.items()):
        print(f"  {status:20s} {n:5d}", file=sys.stderr)

    missing_flag = sum(1 for r in rows if r.extra_flags2 == 0)
    with_ed = sum(1 for r in rows if r.ed_owners)
    rent_hit = sum(1 for r in rows if r.rent_refs > 0)
    rent_slots = sum(r.rent_refs for r in rows)
    print(f"  with ED* owner token: {with_ed}", file=sys.stderr)
    print(f"  without F/extra2:     {missing_flag}", file=sys.stderr)
    print(f"  rent files hitting:   {rent_hit} edits / {rent_slots} slots", file=sys.stderr)

    risky = [
        r
        for r in rows
        if r.status in ("NEED_BASE", "PARSE_ERROR", "OK_HEADER_MISSING")
        or (r.status == "NEED_BASE_HINT" and not r.base_in_world)
    ]
    print(f"  needs attention:      {len(risky)}", file=sys.stderr)
    for r in risky[:15]:
        print(
            f"    {r.edit_vnum} status={r.status} base={r.proposed_base} "
            f"notes={r.notes!r} short={r.short_desc[:40]!r}",
            file=sys.stderr,
        )
    if len(risky) > 15:
        print(f"    ... +{len(risky) - 15} more (see CSV)", file=sys.stderr)


def default_under_repo(rel: str) -> Path:
    here = Path(__file__).resolve()
    server_root = here.parent.parent
    return server_root / rel


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument(
        "--objects-dir",
        type=Path,
        default=default_under_repo("mudroot/lib/objects"),
        help="Directory objects/ (default: Server/mudroot/lib/objects)",
    )
    ap.add_argument(
        "--myst-obj",
        type=Path,
        default=default_under_repo("mudroot/lib/myst.obj"),
        help="Catalogo mondo per verificare base_vnum",
    )
    ap.add_argument(
        "--findoriginal-csv",
        type=Path,
        default=default_under_repo("mudroot/lib/edited_objects_findoriginal.csv"),
        help="CSV findoriginal (opzionale se assente)",
    )
    ap.add_argument(
        "--rent-dir",
        type=Path,
        default=None,
        help="Se indicato, conta riferimenti item_number ai vnum 34k nei rent",
    )
    ap.add_argument(
        "--output",
        "-o",
        type=Path,
        default=Path("/tmp/edit-migrate-dryrun.csv"),
        help="CSV di output (default: /tmp/edit-migrate-dryrun.csv)",
    )
    args = ap.parse_args()

    if not args.objects_dir.is_dir():
        print(f"objects-dir non trovata: {args.objects_dir}", file=sys.stderr)
        return 1

    world: Set[int] = set()
    if args.myst_obj.is_file():
        print(f"Indexing {args.myst_obj} ...", file=sys.stderr)
        world = load_myst_vnums(args.myst_obj)
        print(f"  myst.obj vnums: {len(world)}", file=sys.stderr)
    else:
        print(f"WARN: myst.obj assente ({args.myst_obj}); base_in_world solo via objects/", file=sys.stderr)

    findorig: Dict[int, Tuple[int, int, str]] = {}
    if args.findoriginal_csv.is_file():
        findorig = load_findoriginal(args.findoriginal_csv)
        print(f"findoriginal rows: {len(findorig)}", file=sys.stderr)
    else:
        print(f"WARN: findoriginal CSV assente ({args.findoriginal_csv})", file=sys.stderr)

    rent_refs: Dict[int, int] = {}
    if args.rent_dir is not None:
        if not args.rent_dir.is_dir():
            print(f"rent-dir non trovata: {args.rent_dir}", file=sys.stderr)
            return 1
        print(f"Scanning rent dir {args.rent_dir} ...", file=sys.stderr)
        edit_set = {int(p.name) for p in iter_edit_files(args.objects_dir)}
        rent_refs = scan_rent_refs(args.rent_dir, edit_set)
        print(f"  distinct edit vnums in rent: {len(rent_refs)}", file=sys.stderr)

    rows = build_rows(args.objects_dir, world, findorig, rent_refs)
    write_csv(args.output, rows)
    print_summary(rows)
    print(f"Wrote {args.output}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
