#!/usr/bin/env python3
"""Genera procarea.mob da src/procarea_mob_desc.inc + profili di difficolta'."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from textwrap import dedent

ROOT = Path(__file__).resolve().parents[1]
DESC_INC = ROOT / "src/procarea_mob_desc.inc"
OUT = ROOT / "mudroot/lib/procarea.mob"
MYST = ROOT / "mudroot/lib/myst.mob"
MARKER_START = "# Antro Effimero:"
MARKER_END = "#99999"

BOSS_VNUM_BASE = 65000
BOSS_PER_TIER = 10
MOB_VNUM_BASE = 65100
MOBS_PER_TIER = 9

MOB_DESC_RE = re.compile(
    r"MOB_DESC\(\s*(\d+)\s*,\s*(boss|mob|trap)\s*,\s*(\d+)\s*,\s*((?:\s*\"(?:\\.|[^\"\\])*\"\s*,?\s*)+)\)",
    re.MULTILINE,
)
STRING_RE = re.compile(r"\"((?:\\.|[^\"\\])*)\"")


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
    stat_lines: tuple[str, ...]
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
        stat_lines=("8 8 1 0 0", "8 8 2 0 0", "8 8 1 0 256", "8 8 2 0 512"),
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
        stat_lines=("8 8 0 0 0", "8 8 1 0 0", "8 8 0 0 512", "8 8 0 0 1024"),
        aff_flags=("0", "0", "512"),
        immune_flags=("0", "256", "512", "1024"),
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
        stat_lines=("8 8 4 64 0", "8 8 4 0 0", "8 8 4 0 2048", "8 8 4 0 4096"),
        aff_flags=("0", "64", "2048", "4096"),
        immune_flags=("0", "256", "512", "1024"),
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
        stat_lines=(
            "8 8 4 0 0",
            "8 8 4 0 2048",
            "8 8 4 0 4096",
            "8 8 4 0 1|16|256",
        ),
        aff_flags=("0", "2048", "4096", "1|16|256"),
        immune_flags=("0", "512", "1024", "2048"),
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
        stat_lines=(
            "6 6 4 16|32|64|32768",
            "6 6 4 32|64|32768",
            "6 6 4 16|32|64",
        ),
        aff_flags=("16|32|64|32768", "16|32|64", "32|64|32768"),
        immune_flags=("1024|2048|4096|32768", "2048|4096", "1024|4096|32768"),
    ),
}


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
    if len(descs) != 95:
        raise ValueError(f"Expected 95 MOB_DESC entries, found {len(descs)}")
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

    stat_line = pick(seed, profile.stat_lines)
    if desc.role == "trap":
        immune = pick(seed + 5, profile.immune_flags)
        parts = stat_line.split()
        if len(parts) >= 1:
            parts[-1] = immune
            stat_line = " ".join(parts)
    elif desc.role == "boss" and desc.tier < 5:
        aff = pick(seed + 1, profile.aff_flags)
        immune = pick(seed + 2, profile.immune_flags)
        parts = stat_line.split()
        if len(parts) >= 5:
            if aff != "0":
                parts[-2] = aff
            if immune != "0":
                parts[-1] = immune
            stat_line = " ".join(parts)

    act = profile.act
    if desc.tier == 5 and desc.role == "boss":
        act = "2 8|128|2048|32768|65536|16777216|33554432|134217728 -1000 A 5"
    elif desc.tier == 4 and desc.role == "boss":
        act = "2|2097152 8|128|2048|32768|65536|134217728 -1000 A 2"

    fight = f"{level} {hitroll} {damroll} {ac} {dice}"
    extras = f"{gold} 0 {profile.position} {profile.position}"
    return fight, extras, stat_line, act


def vnum_for(desc: MobDesc) -> int:
    if desc.role == "boss":
        return BOSS_VNUM_BASE + (desc.tier - 1) * BOSS_PER_TIER + desc.slot
    return MOB_VNUM_BASE + (desc.tier - 1) * MOBS_PER_TIER + desc.slot


def render_mob(desc: MobDesc) -> str:
    profile = TIER_PROFILES[desc.tier]
    fight, extras, stat_line, act = build_stats(desc, profile)
    vnum = vnum_for(desc)
    return dedent(
        f"""\
        #{vnum}
        {desc.keywords}~
        {desc.short}~
        {desc.long}~
        {desc.look}~
        {act}
        {fight}
        {extras}
        {stat_line}
        {desc.agg}~
        {desc.sound}~
        """
    )


def generate(descs: list[MobDesc]) -> str:
    bosses = sorted([d for d in descs if d.role == "boss"], key=lambda d: (d.tier, d.slot))
    others = sorted([d for d in descs if d.role != "boss"], key=lambda d: (d.tier, d.slot))
    chunks = [
        f"{MARKER_START} mob procedurali dedicati ({BOSS_VNUM_BASE}-{MOB_VNUM_BASE + 5 * MOBS_PER_TIER - 1})",
        "# Testi: src/procarea_mob_desc.inc | Stats: generate-procarea-mobs.py",
        "",
    ]
    current_tier = 0
    for desc in bosses:
        if desc.tier != current_tier:
            current_tier = desc.tier
            chunks.append(f"# --- Tier {current_tier} boss ---")
        chunks.append(render_mob(desc))
    current_tier = 0
    for desc in others:
        if desc.tier != current_tier:
            current_tier = desc.tier
            chunks.append(f"# --- Tier {current_tier} mob ---")
        chunks.append(render_mob(desc))
    return "\n".join(chunks) + "\n"


def splice_myst(block: str) -> None:
    text = MYST.read_text(encoding="latin-1", errors="replace")
    start = text.find(MARKER_START)
    end = text.find(MARKER_END)
    if start == -1 or end == -1:
        raise SystemExit(f"Marker not found in {MYST}")
    MYST.write_text(text[:start] + block + text[end:], encoding="latin-1")


def main() -> None:
    descs = parse_desc_inc(DESC_INC)
    block = generate(descs)
    OUT.write_text(block, encoding="latin-1")
    splice_myst(block)
    print(f"Parsed {len(descs)} descriptions from {DESC_INC}")
    print(f"Wrote {OUT}")
    print(f"Updated {MYST}")


if __name__ == "__main__":
    main()
