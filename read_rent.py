#!/usr/bin/env python3
"""
Legge i file rent binari del MUD (formato obj_file_u / obj_file_elem)
e mostra l'equipaggiamento dei personaggi.

Supporta automaticamente sia il formato nuovo (con extra_flags2, elem=596 byte)
che quello vecchio (senza extra_flags2, elem=592 byte).

Uso:
    python3 read_rent.py <file_rent>          # un singolo file
    python3 read_rent.py <cartella_rent>      # tutti i file nella cartella
    python3 read_rent.py                      # usa la path di default
"""

import struct
import sys
import os

# ---------------------------------------------------------------------------
# Posizioni indossate (e_wear enum)
# ---------------------------------------------------------------------------
WEAR_NAMES = {
    0:  "Luce",
    1:  "Dito destro",
    2:  "Dito sinistro",
    3:  "Collo 1",
    4:  "Collo 2",
    5:  "Corpo",
    6:  "Testa",
    7:  "Gambe",
    8:  "Piedi",
    9:  "Mani",
    10: "Braccia",
    11: "Scudo",
    12: "Addosso",
    13: "Vita",
    14: "Polso destro",
    15: "Polso sinistro",
    16: "Impugnata (wield)",
    17: "Tenuta (hold)",
    18: "Schiena",
    19: "Orecchio destro",
    20: "Orecchio sinistro",
    21: "Occhi",
    22: "Arma caricata",
}

DEFAULT_RENT_DIR = os.path.expanduser(
    "~/Documents/GitHub/Server/mudroot/lib/rent"
)

# ---------------------------------------------------------------------------
# Formato header (comune a entrambe le versioni): 40 byte
#   char owner[20], int gold_left, int total_cost,
#   int last_update, int minimum_stay, int number
# ---------------------------------------------------------------------------
HEADER_FMT  = "<20s5i"
HEADER_SIZE = struct.calcsize(HEADER_FMT)   # 40

# ---------------------------------------------------------------------------
# Formato elemento NUOVO (con extra_flags2): 596 byte
#   ush_int item_number + 2 pad + int value[4] + extra_flags + weight +
#   timer + bitvector + name[128] + sd[128] + desc[256] +
#   wearpos(B) + depth(B) + 2 pad + affected[5](h+2pad+i each) + extra_flags2
# ---------------------------------------------------------------------------
ELEM_NEW_FMT  = "<Hxx4iiiiI128s128s256sBBxx" + "hxxi" * 5 + "i"
ELEM_NEW_SIZE = struct.calcsize(ELEM_NEW_FMT)   # 596

# ---------------------------------------------------------------------------
# Formato elemento VECCHIO (senza extra_flags2): 592 byte
# ---------------------------------------------------------------------------
ELEM_OLD_FMT  = "<Hxx4iiiiI128s128s256sBBxx" + "hxxi" * 5
ELEM_OLD_SIZE = struct.calcsize(ELEM_OLD_FMT)   # 592


def decode(b: bytes) -> str:
    """Decodifica una stringa C (null-terminated) da bytes."""
    return b.split(b"\x00")[0].decode("latin-1", errors="replace")


def detect_format(size: int, num_objs: int):
    """
    Restituisce (fmt, elem_size, label) in base alla dimensione del file.
    Prima prova il formato nuovo (596), poi il vecchio (592).
    """
    if num_objs > 0:
        for fmt, esz, label in [
            (ELEM_NEW_FMT, ELEM_NEW_SIZE, "nuovo"),
            (ELEM_OLD_FMT, ELEM_OLD_SIZE, "vecchio"),
        ]:
            if HEADER_SIZE + num_objs * esz == size:
                return fmt, esz, label

    # Fallback: prova a ricavare num_objs dalla dimensione
    for fmt, esz, label in [
        (ELEM_NEW_FMT, ELEM_NEW_SIZE, "nuovo"),
        (ELEM_OLD_FMT, ELEM_OLD_SIZE, "vecchio"),
    ]:
        remainder = size - HEADER_SIZE
        if remainder > 0 and remainder % esz == 0:
            return fmt, esz, label

    # Default al formato nuovo
    return ELEM_NEW_FMT, ELEM_NEW_SIZE, "nuovo (presunto)"


def read_rent_file(path: str) -> None:
    size = os.path.getsize(path)
    if size < HEADER_SIZE:
        print(f"  [file vuoto o troppo piccolo: {size} byte]")
        return

    with open(path, "rb") as f:
        data = f.read()

    owner, gold, total_cost, last_update, min_stay, num_objs = struct.unpack_from(
        HEADER_FMT, data, 0
    )
    owner_name = decode(owner)

    elem_fmt, elem_size, fmt_label = detect_format(size, num_objs)

    # Se num_objs è corrotto, ricavalo dalla dimensione
    if num_objs < 0 or HEADER_SIZE + num_objs * elem_size > size:
        num_objs = (size - HEADER_SIZE) // elem_size
        print(f"  ATTENZIONE: numero oggetti corretto automaticamente → {num_objs}")

    print(f"  Personaggio  : {owner_name}")
    print(f"  Gold lasciati: {gold}")
    print(f"  Formato file : {fmt_label} ({elem_size} byte/oggetto)")
    print(f"  Oggetti      : {num_objs}")

    if num_objs == 0:
        print("  (nessun oggetto)")
        return

    print()
    print(f"  {'Pos':22} {'D':1} {'VNum':6}  Nome")
    print(f"  {'-'*22} {'-'} {'-'*6}  {'-'*40}")

    offset = HEADER_SIZE
    for _ in range(num_objs):
        if offset + elem_size > size:
            break
        fields = struct.unpack_from(elem_fmt, data, offset)
        offset += elem_size

        item_number = fields[0]
        name    = decode(fields[9])
        sd      = decode(fields[10])
        wearpos = fields[12]
        depth   = fields[13]

        # wearpos=0 → in inventario; wearpos>0 → equipaggiato a slot (wearpos-1)
        if wearpos == 0:
            wear_label = "[ inventario ]"
        else:
            wear_label = WEAR_NAMES.get(wearpos - 1, f"pos#{wearpos - 1}")

        indent = "  " * depth
        display_name = name if name else sd
        print(f"  {indent}{wear_label:22} {depth} {item_number:6}  {display_name}")


def main() -> None:
    args = sys.argv[1:]

    if not args:
        targets = [DEFAULT_RENT_DIR]
    else:
        targets = args

    for target in targets:
        if os.path.isdir(target):
            skip_ext = {".aux", ".dead", ".gitignore", ".txt", ".md",
                        ".cpp", ".py", ".h", ".hpp"}
            files = sorted(
                os.path.join(target, f)
                for f in os.listdir(target)
                if not any(f.endswith(e) for e in skip_ext)
                and not f.startswith(".")
                and os.path.isfile(os.path.join(target, f))
            )
        elif os.path.isfile(target):
            files = [target]
        else:
            print(f"Errore: '{target}' non trovato.")
            continue

        for path in files:
            if os.path.getsize(path) == 0:
                continue
            print(f"\n{'='*60}")
            print(f"File: {os.path.basename(path)}")
            print('='*60)
            read_rent_file(path)

    print()


if __name__ == "__main__":
    main()
