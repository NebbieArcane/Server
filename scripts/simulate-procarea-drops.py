#!/usr/bin/env python3
"""Simula roll premi procarea (PG 51, modello ibrido, 3 drop per fascia effettiva)."""

from __future__ import annotations

import random
from dataclasses import dataclass, field
from enum import IntEnum

random.seed(20260621)

TEMPLATE_BANDS = 10
BONUS_MAX = 5
GROUP_MAX_LEVEL = 51

# APPLY_* (subset used)
APPLY_HIT = 13
APPLY_MANA = 14
APPLY_HITROLL = 18
APPLY_DAMROLL = 19
APPLY_HITNDAM = 24
APPLY_SPELLPOWER = 45
APPLY_SAVE_ALL = 17
APPLY_SPELLFAIL = 46
APPLY_HIT_REGEN = 47
APPLY_MANA_REGEN = 48
APPLY_IMMUNE = 49

APPLY_NAMES = {
    APPLY_HIT: "HP",
    APPLY_MANA: "mana",
    APPLY_HITROLL: "hitroll",
    APPLY_DAMROLL: "damroll",
    APPLY_HITNDAM: "hit&dam",
    APPLY_SPELLPOWER: "spellpower",
    APPLY_SAVE_ALL: "save all",
    APPLY_SPELLFAIL: "spellfail",
    APPLY_HIT_REGEN: "hit regen",
    APPLY_MANA_REGEN: "mana regen",
    APPLY_IMMUNE: "IMMUNE",
}

SPELLFAIL_MIN = [5, 6, 8, 9, 11, 12, 14, 15, 17, 20]
SPELLFAIL_MAX = [10, 11, 14, 15, 18, 19, 22, 23, 26, 30]

SHIELD_AC = [4, 5, 5, 6, 7, 8, 9, 9, 10, 12]
SHIELD_NAMES = [
    "disco opalescente del velo brumoso",
    "scudo del crepuscolo brumoso",
    "scudo dal sigillo pulsante",
    "rotella dall'eco spenta",
    "buckler dalla Piazza specchiata",
    "scudo della soglia intermedia",
    "pavise dalla croce DarkStar",
    "scudo di lamiera vigilata",
    "scudo di lamiera finale",
    "scudo disco assorbente",
]
FINGER_NAMES = [
    "anello del sigillo infranto",
    "anello dell'eco svanita",
    "anello della luna specchiata",
    "anello della catena spezzata",
    "anello del giuramento antico",
    "anello dell'orbita nera",
    "anello dell'orbita nera",
    "anello dell'orbita nera",
    "anello dell'orbita nera",
    "anello del sole ingoiato",
]
BODY_NAMES = [
    "corazza del velo cristallino",
    "giubba dal tessuto instabile",
    "brune nebbiose d'argento",
    "corazza dalla piastra oscura",
    "corazza dalla piastra oscura",
    "usbergo della sala finale",
    "usbergo della sala finale",
    "corazza dal carapace vuoto",
    "corazza dal carapace vuoto",
    "corazza dal carapace vuoto",
]

BODY_AC_LO, BODY_AC_HI = 6, 14
GEAR_COSTS = [500, 950, 1200, 1700, 2200, 2850, 3500, 4250, 5000, 7500]

WEAPON_DICE = [
    (2, 8), (2, 9), (3, 6), (3, 7), (3, 7), (3, 8), (4, 7), (4, 7), (4, 8), (4, 8)
]
WEAPON_HIT_DAM = [3, 3, 3, 3, 4, 4, 4, 5, 5, 5]
WEAPON_NAMES = [
    "lama del velo spezzato",
    "lama al verso interrotto",
    "lama dal filo argentato",
    "lama dal filo oscuro",
    "lama dal filo spezzato",
    "lama dal filo negato",
    "lama dal filo negato",
    "lama dal filo negato",
    "lama dal filo negato",
    "lama dal filo negato",
]

ELEMENTAL = ["fuoco", "freddo", "elettricita", "energia", "acido"]
PHYSICAL = [("perforante", 8500), ("taglio", 1450), ("contundente", 50)]

HYBRID_WEIGHTS = [
    (APPLY_HIT, 16),
    (APPLY_MANA, 14),
    (APPLY_SAVE_ALL, 14),
    (APPLY_MANA_REGEN, 12),
    (APPLY_HIT_REGEN, 12),
    (APPLY_SPELLPOWER, 10),
    (APPLY_SPELLFAIL, 8),
    (APPLY_DAMROLL, 8),
    (APPLY_HITROLL, 6),
    (APPLY_HITNDAM, 6),
]


class ItemType(IntEnum):
    ARMOR = 9
    JEWEL = 13
    WEAPON = 5


@dataclass
class Affect:
    location: int
    modifier: int


@dataclass
class SimItem:
    label: str
    base: str
    band: int = 0
    ac: int | None = None
    dice: str | None = None
    hit_dam: str | None = None
    affects: list[Affect] = field(default_factory=list)
    extra: list[str] = field(default_factory=list)
    cost: int = 0


def number(lo: int, hi: int) -> int:
    return random.randint(lo, hi)


def weighted_pick(weights: list[int]) -> int:
    total = sum(weights)
    if total <= 0:
        return -1
    roll = number(1, total)
    for i, w in enumerate(weights):
        roll -= w
        if roll <= 0:
            return i
    return len(weights) - 1


def shield_band_factor(band: int) -> float:
    return min(1.0, 0.20 + band * 0.20)


def scaled_roll(ref_min: int, ref_max: int, scale: float) -> int:
    lo = max(1, round(ref_min * scale))
    hi = max(lo, round(ref_max * scale))
    return number(lo, hi)


def roll_modifier(location: int, band: int) -> int:
    scale = shield_band_factor(band)
    if location == APPLY_HIT:
        return scaled_roll(10, 30, scale)
    if location == APPLY_MANA:
        return scaled_roll(40, 100, scale)
    if location in (APPLY_HIT_REGEN, APPLY_MANA_REGEN):
        return scaled_roll(5, 30, scale)
    if location in (APPLY_HITROLL, APPLY_DAMROLL, APPLY_SPELLPOWER):
        return scaled_roll(1, 4, scale)
    if location == APPLY_HITNDAM:
        return scaled_roll(1, 3, scale)
    if location == APPLY_SAVE_ALL:
        mag_min = max(1, round(2.0 * scale))
        mag_max = max(mag_min, round(5.0 * scale))
        return -number(mag_min, mag_max)
    if location == APPLY_SPELLFAIL:
        return -number(SPELLFAIL_MIN[band], SPELLFAIL_MAX[band])
    return 1


def bonus_allowed(affects: list[Affect], location: int) -> bool:
    if any(a.location == location for a in affects):
        return False
    has_sp = any(a.location == APPLY_SPELLPOWER for a in affects)
    has_melee = any(a.location in (APPLY_DAMROLL, APPLY_HITNDAM) for a in affects)
    if has_sp and location in (APPLY_DAMROLL, APPLY_HITNDAM):
        return False
    if has_melee and location == APPLY_SPELLPOWER:
        return False
    if has_melee and not has_sp and location == APPLY_SPELLFAIL:
        return False
    return True


def apply_bonus_roll(item: SimItem, band: int) -> bool:
    candidates = [(loc, w) for loc, w in HYBRID_WEIGHTS if bonus_allowed(item.affects, loc)]
    if not candidates:
        return False
    pick = weighted_pick([w for _, w in candidates])
    loc = candidates[pick][0]
    item.affects.append(Affect(loc, roll_modifier(loc, band)))
    return True


def roll_immune(physical: bool) -> str:
    if physical:
        roll = number(0, 9999)
        acc = 0
        for name, w in PHYSICAL:
            acc += w
            if roll < acc:
                return f"IMMUNE {name}"
    return f"IMMUNE {random.choice(ELEMENTAL)}"


def lerp_ac(lo: int, hi: int, band: int) -> int:
    if TEMPLATE_BANDS <= 1:
        return lo
    return lo + (hi - lo) * band // (TEMPLATE_BANDS - 1)


def roll_bonuses(item: SimItem, band: int, *, try_ac: bool, allow_physical: bool) -> None:
    if try_ac and item.ac is not None:
        chance = 30 + band * 10
        if number(0, 99) < chance:
            jitter_max = 2 if band >= 7 else 1 if band >= 5 else 0
            jitter = number(0, jitter_max)
            if jitter:
                item.ac += jitter
                item.extra.append(f"AC upgrade +{jitter}")

    for _ in range(BONUS_MAX):
        if not apply_bonus_roll(item, band):
            break

    if len(item.affects) < BONUS_MAX and not any(a.location == APPLY_IMMUNE for a in item.affects):
        item.affects.append(Affect(APPLY_IMMUNE, 0))
        item.extra.append(roll_immune(allow_physical))

    jitter = number(90, 115)
    item.cost = max(1, GEAR_COSTS[band] * jitter // 100)


def roll_weapon_proc(item: SimItem) -> None:
    if item.band < 7:
        return
    roll = number(0, 99999)
    if roll < 10:
        item.extra.append("WEAPON_SPELL cause critical")
    elif roll < 310:
        item.extra.append("WEAPON_SPELL cause serious")
    elif roll < 1310:
        item.extra.append("WEAPON_SPELL cause light")
    elif roll < 1360:
        slayers = ["good", "neutral", "evil"]
        item.extra.append(f"ALIGN_SLAYER {random.choice(slayers)}")
    elif roll < 2360:
        item.extra.append("RACE_SLAYER non-umano")


def simulate_shield(band: int) -> SimItem:
    item = SimItem(
        label="Scudo",
        base=SHIELD_NAMES[band],
        ac=SHIELD_AC[band],
    )
    roll_bonuses(item, band, try_ac=True, allow_physical=False)
    return item


def simulate_finger(band: int) -> SimItem:
    item = SimItem(label="Anello", base=FINGER_NAMES[band])
    roll_bonuses(item, band, try_ac=False, allow_physical=False)
    return item


def simulate_body(band: int) -> SimItem:
    item = SimItem(
        label="Corazza",
        base=BODY_NAMES[band],
        ac=lerp_ac(BODY_AC_LO, BODY_AC_HI, band),
    )
    roll_bonuses(item, band, try_ac=True, allow_physical=True)
    return item


def simulate_weapon(band: int) -> SimItem:
    n, s = WEAPON_DICE[band]
    hd = WEAPON_HIT_DAM[band]
    item = SimItem(
        label="Arma",
        base=WEAPON_NAMES[band],
        dice=f"{n}d{s}",
        hit_dam=f"+{hd}/+{hd}",
    )
    item.band = band
    item.cost = GEAR_COSTS[band] * number(90, 115) // 100
    item.extra.append("RESISTANT")
    roll_weapon_proc(item)
    return item


def format_affects(item: SimItem) -> str:
    parts = []
    for a in item.affects:
        if a.location == APPLY_IMMUNE:
            continue
        name = APPLY_NAMES.get(a.location, str(a.location))
        sign = "+" if a.modifier > 0 else ""
        parts.append(f"{sign}{a.modifier} {name}")
    for e in item.extra:
        if e.startswith("IMMUNE"):
            parts.append(e)
    return ", ".join(parts) if parts else "(nessun bonus)"


def format_item(item: SimItem) -> str:
    lines = [f"**{item.label}** — {item.base}"]
    stats = []
    if item.ac is not None:
        stats.append(f"AC {item.ac}")
    if item.dice:
        stats.append(item.dice)
    if item.hit_dam:
        stats.append(f"hit/dam {item.hit_dam}")
    if item.cost:
        stats.append(f"~{item.cost} monete")
    if stats:
        lines.append("Base: " + " | ".join(stats))
    lines.append("Roll: " + format_affects(item))
    proc = [e for e in item.extra if not e.startswith("IMMUNE") and not e.startswith("AC")]
    if proc:
        lines.append("Extra: " + ", ".join(proc))
    ac_note = [e for e in item.extra if e.startswith("AC")]
    if ac_note:
        lines.append("Upgrade: " + ", ".join(ac_note))
    return "\n".join(lines)


POWER_RANGES = [
    "< 500",
    "500–1000",
    "1000–1800",
    "1800–2800",
    "2800–4000",
    "4000–5200",
    "5200–6400",
    "6400–7600",
    "7600–8800",
    "≥ 8800",
]


def main() -> None:
    print("# Simulazione drop procarea")
    print("PG 51 (5 bonus), modello ibrido, seed fisso 20260621\n")
    for band in range(TEMPLATE_BANDS):
        fascia = band + 1
        print(f"## Fascia {fascia}/10 — potenza {POWER_RANGES[band]}\n")
        for sim_fn in (simulate_shield, simulate_finger, simulate_body):
            print(format_item(sim_fn(band)))
            print()
        if band >= 7:
            print(format_item(simulate_weapon(band)))
            print()


if __name__ == "__main__":
    main()
