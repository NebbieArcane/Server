"""Utility per descrizioni stanze procarea (lunghezza minima, righe d'atmosfera)."""

from __future__ import annotations

MIN_DESC_LINES = 4

_SECTOR_EXTRAS: dict[str, list[str]] = {
    "SECT_AIR": [
        "Il vuoto sotto i piedi non promette alcuna tenuta.",
        "Frammenti di roccia fluttuano appena fuori dal sentiero.",
        "La nebbia effimera pulsa come un secondo respiro.",
        "Echi metallici rimbalzano da direzioni impossibili.",
        "Luce lontana filtra da fessure che cambiano forma.",
        "Ogni passo sembra spostarti senza avanzare davvero.",
        "L'aria qui non obbedisce a nessun nord.",
    ],
    "SECT_INSIDE": [
        "L'eco dei passi rimbalza su pareti vicine.",
        "Polvere antica si solleva a ogni movimento.",
        "Una corrente d'aria fredda tradisce un varco nascosto.",
        "Ombre lunghe si allungano dove la luce finisce.",
        "Odori stagnanti si mescolano a tracce di ferro e pietra.",
        "Il silenzio qui ha peso, come se qualcuno ascoltasse.",
    ],
    "SECT_FOREST": [
        "Resina e muschio impregnano l'aria umida.",
        "Rami scricchiolano oltre il campo visivo.",
        "Luce verdastra filtra a macchia di leopardo tra le fronde.",
        "Radici nodose affondano nel terreno molle.",
        "Un fruscio lontano si ferma appena ti fermi tu.",
    ],
    "SECT_MOUNTAIN": [
        "Pietra nuda e vento tagliente dominano lo spazio.",
        "La roccia e' fredda al tatto e coperta da polvere sottile.",
        "Voci lontane si perdono nel ronzio del vuoto.",
        "Crepe profonde scendono verso l'oscurita'.",
        "Cristalli di ghiaccio scintillano dove passa poca luce.",
    ],
    "SECT_HILLS": [
        "Terreno irregolare costringe a calibrare ogni passo.",
        "Erba rada e ciottoli rotolano sotto i piedi.",
        "Il vento sale e scende a ondate irregolari.",
        "Tracce di passaggio recente segnano il terreno.",
    ],
    "SECT_WATER_NOSWIM": [
        "Acqua scura riflette poco e nasconde molto.",
        "Un odore di alghe marcite aleggia sopra la superficie.",
        "Onde lievi sfiorano le pareti come un respiro lento.",
        "Schiuma verdastra si accumula negli angoli piu' bassi.",
    ],
    "SECT_WATER_SWIM": [
        "L'acqua e' limpida ma fredda fino alle ossa.",
        "Correnti sottili tirano verso passaggi piu' profondi.",
        "Bollicine salgono da fessure nel fondo.",
    ],
    "SECT_DESERT": [
        "Sabbia fine entra ovunque, anche tra i vestiti.",
        "Il calore distorce i contorni in lontananza.",
        "Vento secco sferza senza preavviso.",
    ],
    "SECT_CITY": [
        "Mattoni consumati e intonaco scrostato raccontano abbandono.",
        "Un odore di cenere e pioggia vecchia aleggia nell'aria.",
        "Vetri rotti scricchiolano sotto i detriti.",
    ],
}

_KIND_EXTRAS: dict[str, list[str]] = {
    "entrance": [
        "Qualcosa nel profondo sembra richiamarti oltre la soglia.",
        "La transizione verso l'interno e' netta e irreversibile.",
    ],
    "corridor": [
        "Il passaggio si restringe e poi torna ad aprirsi senza logica.",
        "Segni sulle pareti indicano che altri sono passati di recente.",
    ],
    "treasure": [
        "Un luccichio falso attira lo sguardo verso gli angoli.",
        "L'aria qui e' piu' ferma, come in attesa di una scelta.",
    ],
    "trap": [
        "Il pericolo qui non si annuncia fino a un passo di troppo.",
    ],
    "boss": [
        "La pressione dell'aria aumenta verso il centro della sala.",
        "Qualcosa di antico domina lo spazio con presenza opprimente.",
    ],
}

_GENERIC_EXTRAS: list[str] = [
    "L'atmosfera effimera rende incerto cio' che vedi e cio' che ricordi.",
    "Per un istante hai la sensazione di essere osservato.",
    "Un suono lontano si spegne appena volgi la testa.",
    "La luce vacilla come se la stanza respirasse.",
]


def _desc_lines(desc: str) -> list[str]:
    return [line.strip() for line in desc.strip().split("\n") if line.strip()]


def _pick_extras(
    needed: int,
    sector: str,
    room_name: str,
    room_kind: str,
    existing: list[str],
) -> list[str]:
    if needed <= 0:
        return []

    joined = " ".join(existing).casefold()
    pool: list[str] = []
    pool.extend(_SECTOR_EXTRAS.get(sector, []))
    pool.extend(_KIND_EXTRAS.get(room_kind, []))
    pool.extend(_GENERIC_EXTRAS)

    seen: set[str] = set()
    unique: list[str] = []
    for line in pool:
        key = line.casefold()
        if key in seen:
            continue
        seen.add(key)
        if line.casefold() in joined:
            continue
        unique.append(line)

    if not unique:
        unique = _GENERIC_EXTRAS[:]

    start = abs(hash((room_name, room_kind, sector))) % len(unique)
    picked: list[str] = []
    for i in range(needed):
        picked.append(unique[(start + i) % len(unique)])
    return picked


def ensure_min_lines(
    desc: str,
    min_lines: int,
    sector: str,
    room_name: str,
    room_kind: str,
) -> str:
    lines = _desc_lines(desc)
    if len(lines) >= min_lines:
        return "\n".join(lines) + "\n"

    extras = _pick_extras(min_lines - len(lines), sector, room_name, room_kind, lines)
    lines.extend(extras)
    return "\n".join(lines[:min_lines]) + "\n"
