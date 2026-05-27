#!/usr/bin/env python3
"""
Export character_* rows for one toon to docs/legacy-import-<name>.csv
Usage (inside Vagrant or anywhere with mysql client):
  python3 docs/export-legacy-import-csv.py montero
  python3 docs/export-legacy-import-csv.py montero --toon-id 4
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from typing import List, Optional


def mysql_query(sql: str, host: str, user: str, password: str, database: str) -> List[str]:
    cmd = [
        "mysql",
        "-h",
        host,
        "-u",
        user,
        f"-p{password}",
        database,
        "-B",
        "-N",
        "-e",
        sql,
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr or proc.stdout)
        raise SystemExit(proc.returncode)
    text = proc.stdout.strip()
    return text.split("\n") if text else []


def csv_cell(value: str) -> str:
    if not value:
        return ""
    if re.search(r'[;"\r\n]', value):
        return '"' + value.replace('"', '""') + '"'
    return value


def row_line(cols: List[str]) -> str:
    return ";".join(csv_cell(c) for c in cols)


def section(title: str, header: List[str], rows: List[List[str]]) -> List[str]:
    out = [f"=== {title} ===;righe={len(rows)}"]
    if header:
        out.append(row_line(header))
    for r in rows:
        out.append(row_line(r))
    out.append("")
    return out


def split_row(line: str) -> List[str]:
    return line.split("\t") if line else []


def sanitize_text_field(s: str) -> str:
    """Single-line, human-readable (newlines shown as escapes)."""
    return s.replace("\r", "\\r").replace("\n", "\\n")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("name", help="Character name (for filename and lookup)")
    ap.add_argument("--toon-id", type=int, default=0)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--user", default="root")
    ap.add_argument("--password", default="secret")
    ap.add_argument("--database", default="nebbie")
    ap.add_argument(
        "-o",
        "--output",
        help="Output path (default: docs/legacy-import-<name>.csv)",
    )
    args = ap.parse_args()

    name_lc = args.name.lower()
    out_path = args.output or f"docs/legacy-import-{name_lc}.csv"

    def q(sql: str) -> List[str]:
        return mysql_query(sql, args.host, args.user, args.password, args.database)

    if args.toon_id:
        tid = args.toon_id
    else:
        rows = q(
            f"SELECT id FROM toon WHERE LOWER(name)=LOWER('{name_lc}') LIMIT 1"
        )
        if not rows:
            sys.stderr.write(f"No toon for name {args.name!r}\n")
            raise SystemExit(1)
        tid = int(rows[0])

    lines: List[str] = []
    lines.append(f"Legacy import dump - {args.name};toon_id={tid}")
    lines.append("# character_core: description su una riga (\\n \\r); extra_str non e' in DB")
    lines.append("")

    # toon
    toon_h = ["id", "name", "title", "level", "owner_id", "lastlogin", "lasthost"]
    toon_rows = [split_row(r) for r in q(
        f"SELECT id,name,title,level,owner_id,IFNULL(lastlogin,'NULL'),IFNULL(lasthost,'') "
        f"FROM toon WHERE id={tid}"
    )]
    lines.extend(section("toon", toon_h, toon_rows))

    # character_core — cleaned columns
    core_sql = f"""
SELECT
  toon_id,
  REPLACE(REPLACE(description, CHAR(13), '\\\\r'), CHAR(10), '\\\\n'),
  class_primary, sex, race, birth, played, last_logon,
  weight, height, hometown,
  talks_0, talks_1, talks_2, speaks,
  user_flags, extra_flags, age_modifier,
  IFNULL(authcode,''), wimpy_level, load_room, start_room,
  spells_to_learn, alignment, act, affected_by, affected_by2,
  condition_drunk, condition_full, condition_thirst,
  save_throw_0, save_throw_1, save_throw_2, save_throw_3,
  save_throw_4, save_throw_5, save_throw_6, save_throw_7,
  updated_at
FROM character_core WHERE toon_id={tid}
""".strip()
    core_h = [
        "toon_id",
        "description",
        "class_primary",
        "sex",
        "race",
        "birth",
        "played",
        "last_logon",
        "weight",
        "height",
        "hometown",
        "talks_0",
        "talks_1",
        "talks_2",
        "speaks",
        "user_flags",
        "extra_flags",
        "age_modifier",
        "authcode",
        "wimpy_level",
        "load_room",
        "start_room",
        "spells_to_learn",
        "alignment",
        "act",
        "affected_by",
        "affected_by2",
        "condition_drunk",
        "condition_full",
        "condition_thirst",
        "save_throw_0",
        "save_throw_1",
        "save_throw_2",
        "save_throw_3",
        "save_throw_4",
        "save_throw_5",
        "save_throw_6",
        "save_throw_7",
        "updated_at",
    ]
    core_rows = [split_row(r) for r in q(core_sql)]
    lines.extend(section("character_core", core_h, core_rows))

    stats_h = [
        "toon_id",
        "str",
        "str_add",
        "intel",
        "wis",
        "dex",
        "con",
        "chr",
        "extra",
        "extra2",
        "mana",
        "max_mana",
        "mana_gain",
        "hit",
        "max_hit",
        "hit_gain",
        "move",
        "max_move",
        "move_gain",
        "p_rune_dei",
        "points_extra1",
        "points_extra2",
        "points_extra3",
        "armor",
        "gold",
        "bank_gold",
        "exp",
        "true_exp",
        "extra_dual",
        "hitroll",
        "damroll",
        "libero",
    ]
    lines.extend(
        section(
            "character_stats",
            stats_h,
            [split_row(r) for r in q(f"SELECT * FROM character_stats WHERE toon_id={tid}")],
        )
    )

    lines.extend(
        section(
            "character_classes",
            ["toon_id", "class_index", "level"],
            [
                split_row(r)
                for r in q(
                    f"SELECT toon_id, class_index, level FROM character_classes "
                    f"WHERE toon_id={tid} ORDER BY class_index"
                )
            ],
        )
    )

    lines.extend(
        section(
            "character_skills",
            ["toon_id", "skill_id", "learned", "flags", "special", "nummem"],
            [
                split_row(r)
                for r in q(
                    f"SELECT * FROM character_skills WHERE toon_id={tid} ORDER BY skill_id"
                )
            ],
        )
    )

    for empty in (
        "character_affects",
        "character_resistance",
        "character_aliases",
        "character_achievements",
        "character_mercy",
        "character_quest_progress",
    ):
        n = len(q(f"SELECT 1 FROM {empty} WHERE toon_id={tid}"))
        lines.append(f"=== {empty} ===;righe={n}")
        lines.append("")

    lines.extend(
        section(
            "character_prefs",
            ["toon_id", "pref_key", "pref_value"],
            [
                split_row(r)
                for r in q(
                    f"SELECT toon_id, pref_key, pref_value FROM character_prefs "
                    f"WHERE toon_id={tid}"
                )
            ],
        )
    )

    lines.extend(
        section(
            "character_rent",
            ["toon_id", "gold_left", "total_cost", "last_update", "minimum_stay", "object_count"],
            [split_row(r) for r in q(f"SELECT * FROM character_rent WHERE toon_id={tid}")],
        )
    )

    inv_h = [
        "id",
        "toon_id",
        "list_index",
        "item_number",
        "value0",
        "value1",
        "value2",
        "value3",
        "extra_flags",
        "extra_flags2",
        "weight",
        "timer",
        "bitvector",
        "obj_name",
        "short_desc",
        "description",
        "wear_pos",
        "depth",
    ]
    lines.extend(
        section(
            "character_inventory",
            inv_h,
            [
                split_row(r)
                for r in q(
                    f"SELECT id,toon_id,list_index,item_number,value0,value1,value2,value3,"
                    f"extra_flags,extra_flags2,weight,timer,bitvector,obj_name,short_desc,"
                    f"description,wear_pos,depth FROM character_inventory "
                    f"WHERE toon_id={tid} ORDER BY list_index"
                )
            ],
        )
    )

    lines.extend(
        section(
            "character_inventory_affect",
            ["inventory_id", "list_index", "affect_slot", "location", "modifier"],
            [
                split_row(r)
                for r in q(
                    f"SELECT cia.inventory_id,ci.list_index,cia.affect_slot,cia.location,"
                    f"cia.modifier FROM character_inventory_affect cia "
                    f"JOIN character_inventory ci ON ci.id=cia.inventory_id "
                    f"WHERE ci.toon_id={tid} ORDER BY ci.list_index,cia.affect_slot"
                )
            ],
        )
    )

    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
        if lines and not lines[-1].endswith("\n"):
            f.write("\n")

    print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
