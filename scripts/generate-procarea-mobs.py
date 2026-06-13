#!/usr/bin/env python3
"""Rigenera procarea_band_stats.inc da src/procarea_mob_desc.inc.

I testi mob sono compilati direttamente da procarea_mob_desc.inc in procarea.cpp;
non serve più scrivere procarea.mob né myst.mob per le istanze effimere.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from textwrap import dedent

ROOT = Path(__file__).resolve().parents[1]
DESC_INC = ROOT / "src/procarea_mob_desc.inc"
BAND_STATS_INC = ROOT / "src/procarea_band_stats.inc"
MARKER_START = "# Antro Effimero:"  # solo per export .mob opzionale (generate())

BOSS_VNUM_BASE = 65000
BOSS_PER_TIER = 10
MOB_VNUM_BASE = 65100
MOBS_PER_TIER = 9
ARCHETYPE_COUNT = 19
EXPECTED_ARCHETYPES = 19
DICE_RE = re.compile(r"^(\d+)d(\d+)\+(-?\d+)$")
STAT_LINE_RE = re.compile(r"^8 (\d+) 3 (\S+) (\S+) (\S+)$")

MOB_DESC_RE = re.compile(
    r"MOB_DESC\(\s*(\d+)\s*,\s*(boss|mob|trap)\s*,\s*(\d+)\s*,\s*((?:\s*\"(?:\\.|[^\"\\])*\"\s*,?\s*)+)\)",
    re.MULTILINE,
)
STRING_RE = re.compile(r"\"((?:\\.|[^\"\\])*)\"")

# Short nel .inc senza articolo; il generatore li aggiunge come in myst.mob ("Un elementale...").
HAS_ARTICLE_RE = re.compile(
    r"^(Un |Una |Il |Lo |La |L'|Uno |Un'|un |una |il |lo |la |uno |un')",
    re.IGNORECASE,
)
LO_FIRST = frozenset({"gnomo", "scheletro", "spettro"})
L_APOSTROPHE_FIRST = frozenset(
    {
        "alfa",
        "esecutore",
        "ermite",
        "oracolo",
        "araldo",
        "augusto",
        "antico",
        "avatar",
        "arpione",
        "ombra",
        "augure",
        "arconte",
        "archivista",
        "erinia",
    }
)
# IMM_SLEEP / IMM_HOLD / IMM_CHARM — regole susc/immunita' mob antro.
IMM_SLEEP = 1024
IMM_HOLD = 4096
IMM_CHARM = 2048

UNA_FIRST = frozenset(
    {
        "regina",
        "dama",
        "matriarca",
        "larva",
        "rana",
        "anguilla",
        "voce",
        "divinita",
        "arpia",
        "sentinella",
    }
)


def lower_first_char(text: str) -> str:
    if not text:
        return text
    return text[0].lower() + text[1:]


def flags_to_str(total: int) -> str:
    if total == 0:
        return "0"
    bits: list[str] = []
    power = 1
    value = total
    while value:
        if value & 1:
            bits.append(str(power))
        value >>= 1
        power <<= 1
    return "|".join(bits)


def apply_charm_immunity(immune: str, m_immune: str, susc: str) -> tuple[str, str, str]:
    """Tutti i mob antro: M_immune charm; mai susc charm."""
    return (
        immune,
        flags_to_str(flags_or(m_immune) | IMM_CHARM),
        flags_to_str(flags_or(susc) & ~IMM_CHARM),
    )


def sanitize_susc(flags: str) -> str:
    """Rimuove suscettibilita' sleep, hold e charm da un bitfield .mob."""
    if not flags or flags.strip() == "0":
        return "0"
    total = 0
    for part in flags.split("|"):
        part = part.strip()
        if part:
            total |= int(part)
    total &= ~IMM_SLEEP
    total &= ~IMM_HOLD
    total &= ~IMM_CHARM
    if total == 0:
        return "0"
    bits: list[str] = []
    power = 1
    value = total
    while value:
        if value & 1:
            bits.append(str(power))
        value >>= 1
        power <<= 1
    return "|".join(bits)


def add_article_short(short: str) -> str:
    s = short.strip()
    if not s or HAS_ARTICLE_RE.match(s):
        return s
    first = s.split()[0].lower()
    body = lower_first_char(s)
    if first in LO_FIRST:
        return f"Lo {body}"
    if first in L_APOSTROPHE_FIRST or first[0] in "aeiou":
        return f"L'{body}"
    if first in UNA_FIRST or (first.endswith("a") and first not in {"arma"}):
        return f"Una {body}"
    if len(first) >= 2 and first[0] == "s" and first[1] not in "aeiouh":
        return f"Uno {body}"
    if first[0] == "z" or first.startswith(("gn", "ps", "pn")):
        return f"Uno {body}"
    return f"Un {body}"


@dataclass(frozen=True)
class MobDesc:
    tier: int
    role: str
    slot: int
    keywords: str
    short: str
    long: str
    look: str
    agg: str
    sound: str


@dataclass(frozen=True)
class TierProfile:
    act: str
    level: tuple[int, int]
    hitroll: tuple[int, int]
    damroll: tuple[int, int]
    ac: tuple[int, int]
    dice: tuple[str, ...]
    dice_bonus: tuple[int, int]
    gold: tuple[int, int]
    position: int
    # (immune, M_immune, susc) — vedi read_mobile() in db.cpp dopo position/default_pos
    stat_triples: tuple[tuple[str, str, str], ...]
    aff_flags: tuple[str, ...]
    immune_flags: tuple[str, ...]


TIER_PROFILES: dict[int, TierProfile] = {
    1: TierProfile(
        act="2|4|4194304 32768 349 L 3",
        level=(7, 12),
        hitroll=(12, 19),
        damroll=(4, 9),
        ac=(3, 8),
        dice=("1d6", "1d8", "2d4", "2d5"),
        dice_bonus=(3, 8),
        gold=(0, 200),
        position=18,
        stat_triples=(
            ("0", "0", "0"),
            ("0", "1", "0"),
            ("0", "0", "256"),
            ("0", "0", "512"),
        ),
        aff_flags=("0", "0", "256"),
        immune_flags=("0", "0", "256", "512"),
    ),
    2: TierProfile(
        act="2|4|32|64 8|32768|65536 -1000 L 4",
        level=(28, 39),
        hitroll=(-2, 3),
        damroll=(-10, -6),
        ac=(20, 31),
        dice=("2d6", "2d7", "2d8"),
        dice_bonus=(3, 10),
        gold=(200, 1800),
        position=72,
        stat_triples=(
            ("0", "0", "0"),
            ("0", "1", "0"),
            ("0", "0", "512"),
            ("0", "0", "2048"),
        ),
        aff_flags=("0", "0", "512"),
        immune_flags=("0", "256", "512", "2048"),
    ),
    3: TierProfile(
        act="2|32|67108864 8|128|32768 -1000 A 2",
        level=(35, 43),
        hitroll=(-3, 1),
        damroll=(-6, -3),
        ac=(105, 140),
        dice=("2d6", "2d8"),
        dice_bonus=(8, 14),
        gold=(1400, 3200),
        position=1,
        stat_triples=(
            ("64", "0", "0"),
            ("0", "0", "0"),
            ("0", "0", "2048"),
            ("0", "0", "512"),
        ),
        aff_flags=("0", "64", "2048", "4096"),
        immune_flags=("0", "256", "512", "2048"),
    ),
    4: TierProfile(
        act="2|2097152 8|128|2048|32768|65536 -1000 A 2",
        level=(44, 51),
        hitroll=(-3, 1),
        damroll=(-5, -2),
        ac=(245, 305),
        dice=("2d8", "2d7"),
        dice_bonus=(10, 20),
        gold=(3200, 6200),
        position=49,
        stat_triples=(
            ("0", "0", "0"),
            ("0", "0", "2048"),
            ("0", "0", "512"),
            ("1|16|256", "0", "0"),
        ),
        aff_flags=("0", "2048", "4096", "1|16|256"),
        immune_flags=("0", "256", "512", "2048"),
    ),
    5: TierProfile(
        act="2 8|128|32768|65536 -1000 A 5",
        level=(50, 57),
        hitroll=(-19, -12),
        damroll=(-13, -8),
        ac=(650, 760),
        dice=("2d6", "2d8"),
        dice_bonus=(28, 42),
        gold=(8000, 22000),
        position=30,
        stat_triples=(
            ("16|32|64|32768", "0", "0"),
            ("32|64|32768", "0", "0"),
            ("16|32|64", "0", "0"),
        ),
        aff_flags=("16|32|64|32768", "16|32|64", "32|64|32768"),
        immune_flags=("1024|2048|4096|32768", "2048|4096", "1024|4096|32768"),
    ),
    6: TierProfile(
        act="2 8|128|32768|65536 -1000 A 5",
        level=(57, 60),
        hitroll=(-15, -8),
        damroll=(-9, -4),
        ac=(820, 950),
        dice=("3d7", "3d8"),
        dice_bonus=(35, 50),
        gold=(18000, 40000),
        position=30,
        stat_triples=(
            ("16|32|64|32768", "0", "0"),
            ("32|64|32768", "1024|2048", "0"),
            ("16|32|64|32768", "0", "0"),
        ),
        aff_flags=("16|32|64|32768", "32|64|32768", "1024|2048|4096"),
        immune_flags=("1024|2048|4096|32768", "2048|4096|32768", "1024|4096|32768"),
    ),
}


def format_stat_line(position: int, immune: str, m_immune: str, susc: str) -> str:
    """Riga position/default_pos/sex+immune per mob L/A (db.cpp read_mobile)."""
    return f"8 {position} 3 {immune} {m_immune} {susc}"


def parse_desc_inc(path: Path) -> list[MobDesc]:
    text = path.read_text(encoding="utf-8")
    descs: list[MobDesc] = []
    for match in MOB_DESC_RE.finditer(text):
        tier = int(match.group(1))
        role = match.group(2)
        slot = int(match.group(3))
        strings = [s.replace("\\\"", '"').replace("\\\\", "\\") for s in STRING_RE.findall(match.group(4))]
        if len(strings) != 6:
            raise ValueError(f"MOB_DESC T{tier} {role} {slot}: expected 6 strings, got {len(strings)}")
        descs.append(
            MobDesc(tier, role, slot, strings[0], strings[1], strings[2], strings[3], strings[4], strings[5])
        )
    if len(descs) != EXPECTED_ARCHETYPES:
        raise ValueError(
            f"Expected {EXPECTED_ARCHETYPES} archetype MOB_DESC entries (tier 1), found {len(descs)}"
        )
    if any(desc.tier != 1 for desc in descs):
        raise ValueError("Hybrid mode: only tier 1 MOB_DESC entries are supported")
    return descs


def mix_seed(tier: int, role: str, slot: int) -> int:
    role_key = {"mob": 17, "trap": 43, "boss": 71}[role]
    return tier * 1000 + role_key * 10 + slot


def pick(seed: int, pool: tuple[str, ...]) -> str:
    return pool[seed % len(pool)]


def pick_range(seed: int, lo: int, hi: int, salt: int = 0) -> int:
    span = hi - lo + 1
    return lo + (seed * 13 + salt * 7) % span


def build_stats(desc: MobDesc, profile: TierProfile) -> tuple[str, str, str, str]:
    seed = mix_seed(desc.tier, desc.role, desc.slot)
    level = pick_range(seed, *profile.level, 1)
    hitroll = pick_range(seed, *profile.hitroll, 2)
    damroll = pick_range(seed, *profile.damroll, 3)
    ac = pick_range(seed, *profile.ac, 4)

    if desc.role == "boss":
        level = min(profile.level[1] + 3, level + 2)
        damroll += 1 if profile.damroll[1] >= 0 else -1
        hitroll += 1 if profile.hitroll[1] >= 0 else -1
        ac = max(profile.ac[0], ac - 1)
    elif desc.role == "trap":
        ac = max(profile.ac[0], ac - 2)
        hitroll += 1 if hitroll >= 0 else -1

    dice_base = pick(seed, profile.dice)
    dice_plus = pick_range(seed, *profile.dice_bonus, 5)
    if desc.role == "boss":
        dice_plus += 2
    dice = f"{dice_base}+{dice_plus}"

    gold = pick_range(seed, *profile.gold, 6)
    if desc.role == "boss":
        gold = int(gold * 1.4)

    immune, m_immune, susc = pick(seed, profile.stat_triples)
    if desc.role == "trap":
        susc = pick(seed + 5, profile.immune_flags)
    elif desc.role == "boss" and desc.tier < 5:
        aff = pick(seed + 1, profile.aff_flags)
        boss_immune = pick(seed + 2, profile.immune_flags)
        if aff != "0":
            m_immune = aff
        if boss_immune != "0":
            susc = boss_immune
    elif desc.role == "boss" and desc.tier >= 5:
        aff = pick(seed + 1, profile.aff_flags)
        boss_immune = pick(seed + 2, profile.immune_flags)
        if aff != "0":
            immune = aff
        if boss_immune != "0":
            m_immune = boss_immune

    if desc.role == "mob" and desc.tier >= 2:
        susc = "0"

    susc = sanitize_susc(susc)
    immune, m_immune, susc = apply_charm_immunity(immune, m_immune, susc)

    stat_line = format_stat_line(profile.position, immune, m_immune, susc)

    act = profile.act
    if desc.tier >= 5 and desc.role == "boss":
        act = "2 8|128|2048|32768|65536|16777216|33554432|134217728 -1000 A 5"
    elif desc.tier == 4 and desc.role == "boss":
        act = "2|2097152 8|128|2048|32768|65536|134217728 -1000 A 2"

    fight = f"{level} {hitroll} {damroll} {ac} {dice}"
    # -1: gold/exp/race su questa riga; position/default_pos sulla riga stat (come myst.mob L)
    extras = f"-1 {gold} 0 0"
    return fight, extras, stat_line, act


def archetype_index(desc: MobDesc) -> int:
    if desc.role == "boss":
        return desc.slot
    return BOSS_PER_TIER + desc.slot


def vnum_for(desc: MobDesc) -> int:
    if desc.role == "boss":
        return BOSS_VNUM_BASE + desc.slot
    return MOB_VNUM_BASE + desc.slot


def flags_or(text: str) -> int:
    total = 0
    for part in text.split("|"):
        part = part.strip()
        if part:
            total |= int(part)
    return total


def parse_act_line(act: str) -> tuple[int, int]:
    parts = act.split()
    if not parts:
        return 0, 0
    act_flags = flags_or(parts[0])
    affected_by = flags_or(parts[1]) if len(parts) > 1 else 0
    return act_flags, affected_by


def combat_from_build(desc: MobDesc, tier: int) -> dict[str, int]:
    profile = TIER_PROFILES[tier]
    tier_desc = MobDesc(
        tier,
        desc.role,
        desc.slot,
        desc.keywords,
        desc.short,
        desc.long,
        desc.look,
        desc.agg,
        desc.sound,
    )
    fight, extras, stat_line, act = build_stats(tier_desc, profile)
    fight_parts = fight.split()
    if len(fight_parts) != 5:
        raise ValueError(f"Unexpected fight line '{fight}' for {desc.role} slot {desc.slot}")
    level_s, thac0_s, armor_field_s, hp_bonus_s, dice_s = fight_parts
    dice_match = DICE_RE.match(dice_s)
    if dice_match is None:
        raise ValueError(f"Unexpected dice format '{dice_s}' for {desc.role} slot {desc.slot}")
    stat_match = STAT_LINE_RE.match(stat_line)
    if stat_match is None:
        raise ValueError(f"Unexpected stat line '{stat_line}'")
    act_flags, affected_by = parse_act_line(act)
    gold_parts = extras.split()
    gold = int(gold_parts[1]) if len(gold_parts) > 1 else 0
    return {
        "level": int(level_s),
        "hitroll": 20 - int(thac0_s),
        "armor": 10 * int(armor_field_s),
        "hp_bonus": int(hp_bonus_s),
        "dam_n": int(dice_match.group(1)),
        "dam_s": int(dice_match.group(2)),
        "dam_plus": int(dice_match.group(3)),
        "gold": gold,
        "position": int(stat_match.group(1)),
        "default_pos": int(stat_match.group(1)),
        "immune": flags_or(stat_match.group(2)),
        "m_immune": flags_or(stat_match.group(3)),
        "susc": flags_or(stat_match.group(4)),
        "act": act_flags,
        "affected_by": affected_by,
    }


def emit_band_stats_inc(archetypes: list[MobDesc], path: Path) -> None:
    ordered = sorted(
        archetypes,
        key=lambda d: (0 if d.role == "boss" else 1, d.slot if d.role == "boss" else d.slot),
    )
    band_blocks: list[str] = []
    for band in range(1, 7):
        rows: list[str] = []
        for desc in ordered:
            combat = combat_from_build(desc, band)
            idx = archetype_index(desc)
            rows.append(
                "\t\t{" + ", ".join(
                    [
                        str(combat["level"]),
                        str(combat["hitroll"]),
                        str(combat["armor"]),
                        str(combat["hp_bonus"]),
                        str(combat["dam_n"]),
                        str(combat["dam_s"]),
                        str(combat["dam_plus"]),
                        str(combat["gold"]),
                        str(combat["position"]),
                        str(combat["default_pos"]),
                        str(combat["immune"]),
                        str(combat["m_immune"]),
                        str(combat["susc"]),
                        str(combat["act"]),
                        str(combat["affected_by"]),
                    ]
                )
                + f"}}, /* band {band - 1} idx {idx} {desc.role} {desc.slot} */"
            )
        band_blocks.append("\t{\n" + "\n".join(rows) + "\n\t}")
    body = ",\n".join(band_blocks)
    path.write_text(
        f"""/* generated by scripts/generate-procarea-mobs.py — do not edit */
#ifndef __PROCAREA_BAND_STATS_INC
#define __PROCAREA_BAND_STATS_INC

struct ProcArchetypeCombat {{
\tint level;
\tint hitroll;
\tint armor;
\tint hp_bonus;
\tint dam_n;
\tint dam_s;
\tint dam_plus;
\tint gold;
\tint position;
\tint default_pos;
\tunsigned immune;
\tunsigned m_immune;
\tunsigned susc;
\tlong act;
\tlong affected_by;
}};

static constexpr ProcArchetypeCombat kProcBandCombat[{6}][{ARCHETYPE_COUNT}] = {{
{body}
}};

#endif
""",
        encoding="utf-8",
    )


def mob_text_trailing_blank(text: str) -> str:
    """Riga vuota finale nel .mob (\\n\\r) per separare i blocchi in gioco."""
    if text and not text.endswith("\n\r"):
        return text + "\n\r"
    return text


def mob_field(text: str) -> str:
    """Campo stringa .mob: testo (con eventuale \\n\\r finale) e delimitatore ~."""
    return f"{mob_text_trailing_blank(text)}~\n"


def render_mob(desc: MobDesc) -> str:
    profile = TIER_PROFILES[1]
    fight, extras, stat_line, act = build_stats(desc, profile)
    vnum = vnum_for(desc)
    short = add_article_short(desc.short)
    return (
        f"#{vnum}\n"
        f"{desc.keywords}~\n"
        f"{short}~\n"
        f"{mob_field(desc.long)}"
        f"{mob_field(desc.look)}"
        f"{act}\n"
        f"{fight}\n"
        f"{extras}\n"
        f"{stat_line}\n"
        f"{desc.agg}~\n"
        f"{mob_field(desc.sound)}"
    )


def generate(descs: list[MobDesc]) -> str:
    bosses = sorted([d for d in descs if d.role == "boss"], key=lambda d: (d.tier, d.slot))
    others = sorted([d for d in descs if d.role != "boss"], key=lambda d: (d.tier, d.slot))
    last_vnum = MOB_VNUM_BASE + MOBS_PER_TIER - 1
    chunks = [
        f"{MARKER_START} mob procedurali dedicati ({BOSS_VNUM_BASE}-{last_vnum})",
        "# Testi: src/procarea_mob_desc.inc | Shell tier1 .mob | Stats runtime: procarea_band_stats.inc",
        "",
        "# --- Boss archetipi ---",
    ]
    for desc in bosses:
        chunks.append(render_mob(desc))
    chunks.append("# --- Mob e trap archetipi ---")
    for desc in others:
        chunks.append(render_mob(desc))
    return "\n".join(chunks) + "\n"


def main() -> None:
    descs = parse_desc_inc(DESC_INC)
    emit_band_stats_inc(descs, BAND_STATS_INC)
    print(f"Parsed {len(descs)} archetypes from {DESC_INC}")
    print(f"Wrote {BAND_STATS_INC}")


if __name__ == "__main__":
    main()
