#!/usr/bin/env python3
"""Rigenera procarea_mob_desc.inc e procarea_band_stats.inc dal catalogo mob.

Sorgente testi: scripts/procarea_mob_catalog.py
Stats: generate-procarea-mobs.py -> src/procarea_band_stats.inc
"""

from __future__ import annotations

import importlib.util
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
DESC_INC = ROOT / "src/procarea_mob_desc.inc"
THEME_INC = ROOT / "src/procarea_mob_themes.inc"
BAND_STATS_INC = ROOT / "src/procarea_band_stats.inc"
AUTOENUMS_HPP = ROOT / "src/autoenums.hpp"
CATALOG_PY = ROOT / "scripts/procarea_mob_catalog.py"
MARKER_START = "# Dimensione Effimera:"  # solo per export .mob opzionale (generate())

BOSS_VNUM_BASE = 65000
BOSS_PER_TIER = 55
MOB_VNUM_BASE = 65000
MOBS_PER_TIER = 170
ARCHETYPE_COUNT = 225
TEMPLATE_BANDS = 10
LEGACY_TIERS = 6
EXPECTED_ARCHETYPES = 225
DICE_RE = re.compile(r"^(\d+)d(\d+)\+(-?\d+)$")
STAT_LINE_RE = re.compile(r"^8 8 ([345]) (\S+) (\S+) (\S+)$")

MOB_DESC_RE = re.compile(
    r"MOB_DESC\(\s*(\d+)\s*,\s*(boss|mob|trap)\s*,\s*(\d+)\s*,\s*(RACE_\w+)\s*,\s*((?:\s*\"(?:\\.|[^\"\\])*\"\s*,?\s*)+)\)",
    re.MULTILINE,
)
RACE_ENUM_RE = re.compile(r"^\s*(RACE_\w+)\s*=\s*(\d+)", re.MULTILINE)
STRING_RE = re.compile(r"\"((?:\\.|[^\"\\])*)\"")

# Short nel .inc senza articolo; il generatore aggiunge Il/La/Gli/Le come in combattimento.
HAS_ARTICLE_RE = re.compile(
    r"^(Il |Lo |La |L'|Gli |Le |I |Un |Una |Uno |Un'|"
    r"il |lo |la |gli |le |i |un |una |uno |un')",
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
        "anguilla",
        "aracnide",
        "ape",
    }
)
L_APOSTROPHE_MASC = frozenset(
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
        "augure",
        "arconte",
        "archivista",
        "erinia",
    }
)
MASCULINE_FIRST = frozenset(
    {
        "predatore",
        "signore",
        "custode",
        "drago",
        "capo",
        "demone",
        "gigante",
        "re",
        "lich",
        "principe",
        "balrog",
        "goblin",
        "troll",
        "ghoul",
        "vampiro",
        "ent",
        "capobanda",
        "sovrano",
        "arconte",
        "oracolo",
        "guerriero",
        "ragno",
        "worg",
        "parassito",
        "fulmine",
        "cristallo",
        "ghiaccio",
        "serpente",
        "fantasma",
        "scheletro",
        "spettro",
        "orco",
        "alfa",
        "guardiano",
        "insetto",
        "pipistrello",
    }
)
FEMININE_FIRST = frozenset(
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
        "vipera",
        "gelatina",
        "mimica",
        "ragnatela",
        "nube",
        "porta",
        "trappola",
        "marea",
        "fessura",
        "vespa",
        "polvere",
    }
)
MASC_PLURAL_FIRST = frozenset({"scheletri"})
FEM_PLURAL_FIRST = frozenset({"vespe", "rondini", "liane", "tarantole", "api"})


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


# IMM_SLEEP / IMM_HOLD / IMM_CHARM — regole susc/immunita' mob antro.
IMM_SLEEP = 1024
IMM_HOLD = 4096
IMM_CHARM = 2048


def _first_word(short: str) -> str:
    return short.strip().split()[0].lower() if short.strip() else ""


def _uses_lo_form(first: str) -> bool:
    if first in LO_FIRST:
        return True
    if len(first) >= 2 and first[0] == "s" and first[1] not in "aeiouh":
        return True
    return first[:1] == "z" or first.startswith(("gn", "ps", "pn"))


def _uses_l_apostrophe(first: str) -> bool:
    if first in L_APOSTROPHE_FIRST:
        return True
    return bool(first) and first[0] in "aeiou"


def _is_feminine_plural(first: str) -> bool:
    return first in FEM_PLURAL_FIRST


def _is_plural(first: str) -> bool:
    if first in MASC_PLURAL_FIRST or first in FEM_PLURAL_FIRST:
        return True
    return len(first) >= 5 and first.endswith("i")


def _is_feminine(first: str, sex: int) -> bool:
    if first in MASCULINE_FIRST:
        return False
    if first in FEMININE_FIRST or first in FEM_PLURAL_FIRST:
        return True
    if sex == 2:
        return True
    if sex == 1:
        return False
    return len(first) > 1 and first.endswith("a") and first not in {"arma"}


def sex_from_long_desc(long_desc: str) -> int | None:
    """SEX_MALE / SEX_FEMALE da articolo nella long_desc, se presente."""
    lg = long_desc.strip().lower()
    if lg.startswith(("gli ", "un ", "uno ", "lo ", "il ", "i ")):
        return 1
    if lg.startswith(("le ", "una ", "la ")):
        return 2
    if lg.startswith("un'"):
        return 2
    if lg.startswith("l'"):
        first = lg[2:].split()[0] if len(lg) > 2 else ""
        return 1 if first in L_APOSTROPHE_MASC else 2
    return None


def sex_from_short(short: str) -> int:
    """SEX_NEUTRAL=0, SEX_MALE=1, SEX_FEMALE=2."""
    first = _first_word(short)
    if first in FEM_PLURAL_FIRST or first in FEMININE_FIRST:
        return 2
    if first in MASC_PLURAL_FIRST or _is_plural(first):
        return 1
    prefix = article_prefix(short, 0)
    if prefix in ("la", "le", "l'"):
        return 2
    if prefix in ("il", "lo", "gli", "i", "l'"):
        return 1
    return 0


def archetype_sex(desc: MobDesc) -> int:
    from_long = sex_from_long_desc(desc.long)
    if from_long is not None:
        return from_long
    return sex_from_short(desc.short)


def add_article_short(short: str, sex: int) -> str:
    s = short.strip()
    if not s or HAS_ARTICLE_RE.match(s):
        return s
    first = _first_word(s)
    body = lower_first_char(s)
    plural = _is_plural(first)
    feminine = _is_feminine(first, sex)

    if plural:
        if _is_feminine_plural(first) or (feminine and first.endswith("e")):
            return f"Le {body}"
        if _uses_lo_form(first) or _uses_l_apostrophe(first):
            return f"Gli {body}"
        return f"I {body}"

    if feminine:
        if _uses_l_apostrophe(first):
            return f"L'{body}"
        return f"La {body}"

    if _uses_lo_form(first):
        return f"Lo {body}"
    if _uses_l_apostrophe(first):
        return f"L'{body}"
    return f"Il {body}"


def article_prefix(short: str, sex: int = 0) -> str:
    """Prefisso articolo determinativo (senza titolo)."""
    s = short.strip()
    if not s or HAS_ARTICLE_RE.match(s):
        return "il"
    first = _first_word(s)
    plural = _is_plural(first)
    feminine = _is_feminine(first, sex)

    if plural:
        if _is_feminine_plural(first) or (feminine and first.endswith("e")):
            return "le"
        if _uses_lo_form(first) or _uses_l_apostrophe(first):
            return "gli"
        return "i"

    if feminine:
        if _uses_l_apostrophe(first):
            return "l'"
        return "la"

    if _uses_lo_form(first):
        return "lo"
    if _uses_l_apostrophe(first):
        return "l'"
    return "il"


@dataclass(frozen=True)
class MobDesc:
    tier: int
    role: str
    slot: int
    race: int
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
        act="2|4194304 32768 349 L 3",
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


def format_stat_line(sex: int, immune: str, m_immune: str, susc: str) -> str:
    """Riga position/default_pos/sex+immune per mob L/A (db.cpp read_mobile)."""
    # 8=STANDING; sex 3..5 => player.sex 0..2 + triple immune
    return f"8 8 {3 + sex} {immune} {m_immune} {susc}"


def parse_race_enums(path: Path) -> dict[str, int]:
    text = path.read_text(encoding="utf-8")
    races = {match.group(1): int(match.group(2)) for match in RACE_ENUM_RE.finditer(text)}
    if not races:
        raise ValueError(f"No RACE_* enums found in {path}")
    return races


def load_catalog_module():
    spec = importlib.util.spec_from_file_location("procarea_mob_catalog", CATALOG_PY)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Cannot load catalog from {CATALOG_PY}")
    module = importlib.util.module_from_spec(spec)
    sys.modules["procarea_mob_catalog"] = module
    spec.loader.exec_module(module)
    return module


def emit_desc_inc(path: Path) -> tuple[list[MobDesc], list[dict]]:
    catalog = load_catalog_module()
    bosses = catalog.get_bosses()
    mobs = catalog.get_mobs()
    traps = catalog.get_all_traps()
    if len(bosses) != catalog.BOSS_COUNT:
        raise ValueError(f"Expected {catalog.BOSS_COUNT} bosses, got {len(bosses)}")
    if len(mobs) != catalog.MOB_COUNT:
        raise ValueError(f"Expected {catalog.MOB_COUNT} mobs, got {len(mobs)}")
    if len(traps) != catalog.TRAP_COUNT:
        raise ValueError(f"Expected {catalog.TRAP_COUNT} traps, got {len(traps)}")

    catalog_entries: list[dict] = []

    lines: list[str] = [
        "/* procarea_mob_desc.inc",
        f" * Testi mob Dimensione Effimera ({catalog.archetype_count()} archetipi).",
        " * Generato da scripts/generate-procarea-mobs.py — non editare a mano.",
        " *",
        " * short_title: titolo senza articolo; procarea_spawn.cpp aggiunge Il/La/Gli/Le.",
        " * long_desc: frase di presenza in stanza (con articolo).",
        " * look / sound: procarea.cpp aggiunge \\n\\r in spawn.",
        " * agg: messaggio aggressivita' (player.sounds).",
        " * race: razza fissa per archetipo (e_races in autoenums.hpp).",
        " *",
        f" * Ordine vettore = indice archetipo: boss 0-{len(bosses) - 1},",
        f" * mob 0-{len(mobs) - 1} (indici {len(bosses)}-{len(bosses) + len(mobs) - 1}),",
        f" * trap 0-{len(traps) - 1} (indici {len(bosses) + len(mobs)}-{catalog.archetype_count() - 1}).",
        " */",
        "",
        "#ifndef __PROCAREA_MOB_DESC_INC",
        "#define __PROCAREA_MOB_DESC_INC",
        "",
        "struct ProcMobArchetypeText {",
        "\tconst char* keywords;",
        "\tconst char* short_title;",
        "\tconst char* long_desc;",
        "\tconst char* look;",
        "\tconst char* agg;",
        "\tconst char* sound;",
        "\tint race;",
        "};",
        "",
        "#define MOB_DESC(tier, role, slot, race, kw, st, lg, lk, ag, so) \\",
        "\t{ kw, st, lg, lk, ag, so, race },",
        "",
        "static const ProcMobArchetypeText kProcMobArchetypeTexts[] = {",
        "",
    ]

    descs: list[MobDesc] = []

    def append_entry(role: str, slot: int, entry: dict) -> None:
        race_name = entry["race"]
        catalog_entries.append(entry)
        lines.append(
            "MOB_DESC(1, {role}, {slot}, {race},\n"
            '\t"{kw}",\n'
            '\t"{st}",\n'
            '\t"{lg}",\n'
            '\t"{lk}",\n'
            '\t"{ag}",\n'
            '\t"{so}")'.format(
                role=role,
                slot=slot,
                race=race_name,
                kw=entry["keywords"].replace('"', '\\"'),
                st=entry["short"].replace('"', '\\"'),
                lg=entry["long"].replace('"', '\\"'),
                lk=entry["look"].replace('"', '\\"'),
                ag=entry["agg"].replace('"', '\\"'),
                so=entry["sound"].replace('"', '\\"'),
            )
        )
        lines.append("")
        descs.append(
            MobDesc(
                1,
                role,
                slot,
                parse_race_enums(AUTOENUMS_HPP)[race_name],
                entry["keywords"],
                entry["short"],
                entry["long"],
                entry["look"],
                entry["agg"],
                entry["sound"],
            )
        )

    for slot, entry in enumerate(bosses):
        append_entry("boss", slot, entry)
    for slot, entry in enumerate(mobs):
        append_entry("mob", slot, entry)
    for slot, entry in enumerate(traps):
        append_entry("trap", slot, entry)

    lines.extend(
        [
            "};",
            "",
            "#undef MOB_DESC",
            "",
            "#endif /* __PROCAREA_MOB_DESC_INC */",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")
    return descs, catalog_entries


def emit_theme_inc(entries: list[dict], path: Path) -> None:
    from procarea_theme_tags import THEME_COUNT, emit_theme_mask_array

    body = emit_theme_mask_array(entries)
    path.write_text(
        f"""/* generated by scripts/generate-procarea-mobs.py — do not edit */
#ifndef __PROCAREA_MOB_THEMES_INC
#define __PROCAREA_MOB_THEMES_INC

/** Bit N = tema N ammesso; 0 = universale (fallback). */
static constexpr unsigned long long kProcArchetypeThemeMask[{ARCHETYPE_COUNT}] = {{
{body}
}};

#endif
""",
        encoding="utf-8",
    )


def parse_desc_inc(path: Path) -> list[MobDesc]:
    race_enums = parse_race_enums(AUTOENUMS_HPP)
    text = path.read_text(encoding="utf-8")
    descs: list[MobDesc] = []
    for match in MOB_DESC_RE.finditer(text):
        tier = int(match.group(1))
        role = match.group(2)
        slot = int(match.group(3))
        race_name = match.group(4)
        if race_name not in race_enums:
            raise ValueError(f"MOB_DESC T{tier} {role} {slot}: unknown race {race_name}")
        race = race_enums[race_name]
        strings = [s.replace("\\\"", '"').replace("\\\\", "\\") for s in STRING_RE.findall(match.group(5))]
        if len(strings) != 6:
            raise ValueError(f"MOB_DESC T{tier} {role} {slot}: expected 6 strings, got {len(strings)}")
        descs.append(
            MobDesc(
                tier,
                role,
                slot,
                race,
                strings[0],
                strings[1],
                strings[2],
                strings[3],
                strings[4],
                strings[5],
            )
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

    stat_line = format_stat_line(archetype_sex(desc), immune, m_immune, susc)

    act = profile.act
    if desc.tier >= 5 and desc.role == "boss":
        act = "2 8|128|2048|32768|65536|16777216|33554432|134217728 -1000 A 5"
    elif desc.tier == 4 and desc.role == "boss":
        act = "2|2097152 8|128|2048|32768|65536|134217728 -1000 A 2"

    fight = f"{level} {hitroll} {damroll} {ac} {dice}"
    # -1: gold/exp/race su questa riga; position/default_pos sulla riga stat (come myst.mob L)
    extras = f"-1 {gold} 0 {desc.race}"
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


def parse_mult_att(act: str) -> float:
    """Numero attacchi dalla riga act (.mob L/A), es. '... L 3' o '... A 5'."""
    parts = act.split()
    mobtypes = frozenset({"L", "A", "B", "N", "S"})
    for i, part in enumerate(parts):
        if part in mobtypes and i + 1 < len(parts):
            try:
                return float(parts[i + 1])
            except ValueError:
                break
    return 1.0


def combat_from_build(desc: MobDesc, tier: int) -> dict[str, int]:
    profile = TIER_PROFILES[tier]
    tier_desc = MobDesc(
        tier,
        desc.role,
        desc.slot,
        desc.race,
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
        "position": 8,
        "default_pos": 8,
        "sex": archetype_sex(desc),
        "immune": flags_or(stat_match.group(2)),
        "m_immune": flags_or(stat_match.group(3)),
        "susc": flags_or(stat_match.group(4)),
        "act": act_flags,
        "affected_by": affected_by,
        "mult_att": parse_mult_att(act),
    }


def lerp_int(lo: int, hi: int, frac: float) -> int:
    return int(round(lo + (hi - lo) * frac))


def combat_interp(desc: MobDesc, band_idx: int) -> dict[str, int | float]:
    tier_f = 1.0 + band_idx * (LEGACY_TIERS - 1) / (TEMPLATE_BANDS - 1)
    tier_lo = int(tier_f)
    tier_hi = min(tier_lo + 1, LEGACY_TIERS)
    frac = tier_f - tier_lo if tier_hi > tier_lo else 0.0
    lo = combat_from_build(desc, tier_lo)
    hi = combat_from_build(desc, tier_hi)
    out: dict[str, int | float] = {}
    flag_keys = {
        "immune",
        "m_immune",
        "susc",
        "act",
        "affected_by",
        "sex",
        "position",
        "default_pos",
    }
    for key in lo:
        if key in flag_keys:
            out[key] = hi[key] if frac >= 0.5 else lo[key]
        elif key == "mult_att":
            out[key] = float(lo[key]) + (float(hi[key]) - float(lo[key])) * frac
        else:
            out[key] = lerp_int(int(lo[key]), int(hi[key]), frac)
    return out


def emit_band_stats_inc(archetypes: list[MobDesc], path: Path) -> None:
    ordered = sorted(
        archetypes,
        key=lambda d: (0 if d.role == "boss" else 1, d.slot if d.role == "boss" else d.slot),
    )
    band_blocks: list[str] = []
    for band_idx in range(TEMPLATE_BANDS):
        rows: list[str] = []
        for desc in ordered:
            combat = combat_interp(desc, band_idx)
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
                        str(combat["sex"]),
                        str(combat["immune"]),
                        str(combat["m_immune"]),
                        str(combat["susc"]),
                        str(combat["act"]),
                        str(combat["affected_by"]),
                        str(combat["mult_att"]),
                    ]
                )
                + f"}}, /* band {band_idx} idx {idx} {desc.role} {desc.slot} */"
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
\tint sex;
\tunsigned immune;
\tunsigned m_immune;
\tunsigned susc;
\tlong act;
\tlong affected_by;
\tfloat mult_att;
}};

static constexpr ProcArchetypeCombat kProcBandCombat[{TEMPLATE_BANDS}][{ARCHETYPE_COUNT}] = {{
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
    short = add_article_short(desc.short, archetype_sex(desc))
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
    descs, catalog_entries = emit_desc_inc(DESC_INC)
    if len(descs) != EXPECTED_ARCHETYPES:
        raise ValueError(f"Expected {EXPECTED_ARCHETYPES} archetypes, got {len(descs)}")
    if len(catalog_entries) != EXPECTED_ARCHETYPES:
        raise ValueError(
            f"Expected {EXPECTED_ARCHETYPES} catalog entries, got {len(catalog_entries)}"
        )
    emit_theme_inc(catalog_entries, THEME_INC)
    emit_band_stats_inc(descs, BAND_STATS_INC)
    print(f"Wrote {DESC_INC} ({len(descs)} archetypes)")
    print(f"Wrote {THEME_INC}")
    print(f"Wrote {BAND_STATS_INC}")


if __name__ == "__main__":
    main()
