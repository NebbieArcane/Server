"""Indici temi procarea (allineati a kThemeSets in procarea_themes.inc)."""

from __future__ import annotations

THEME_COUNT = 50

# 0..49 — label da procarea_themes.inc
THEME_LABELS: tuple[str, ...] = (
    "Antro Selvaggio",          # 0
    "Cripta Sepolta",           # 1
    "Labirinto di Nebbia",      # 2
    "Rovine Sommerse",          # 3
    "Forgia Abbandonata",       # 4
    "Galleria di Ghiaccio",     # 5
    "Tana dei Ragni",           # 6
    "Catacombe Bruciate",       # 7
    "Labirinto di Cristallo",   # 8
    "Antro Fungino",            # 9
    "Tempio Rovinato",          # 10
    "Voragine Profonda",        # 11
    "Ossuario Antico",          # 12
    "Santuario Profanato",      # 13
    "Giardino Pietrificato",    # 14
    "Miniera Abbandonata",      # 15
    "Palude Nera",              # 16
    "Torre Spezzata",           # 17
    "Canyon del Tuono",         # 18
    "Nido del Corvo Re",        # 19
    "Biblioteca Sepolta",       # 20
    "Arena Dimenticata",        # 21
    "Labirinto di Spine",       # 22
    "Grotta Lunare",            # 23
    "Caverna di Sale",          # 24
    "Moria delle Ombre",        # 25
    "Fangorn Profondo",         # 26
    "Tomba dei Re Caduti",      # 27
    "Ponte del Baratro",        # 28
    "Rovine di Osgiliath",      # 29
    "Covo degli Uruk",          # 30
    "Galleria Elfica",          # 31
    "Ceneri di Gorgoroth",      # 32
    "Fortezza di Helm",         # 33
    "Sentieri dei Morti",       # 34
    "Fosse di Isengard",        # 35
    "Tunnels di Ungol",         # 36
    "Bosco di Mirkwood",        # 37
    "Cantine di Bree",          # 38
    "Tesoro di Smaug",          # 39
    "Sale di Erebor",           # 40
    "Rova Nera Profonda",       # 41
    "Fogne di Waterdeep",       # 42
    "Rovine di Neverwinter",    # 43
    "Cripta di Baldur",         # 44
    "Fessura Planare",          # 45
    "Castello delle Ombre",     # 46
    "Antro del Drago",          # 47
    "Caverna degli Occhi",      # 48
    "Fosso Infernale",          # 49
)

# Alias brevi — temi 0-24
WILD, CRYPT, MIST, DROWNED = 0, 1, 2, 3
FORGE, ICE, SPIDER, FIRE = 4, 5, 6, 7
CRYSTAL, FUNGUS, TEMPLE, ABYSS = 8, 9, 10, 11
OSSUARY, PROFANED, STONEGARDEN, MINE = 12, 13, 14, 15
SWAMP, TOWER, CANYON, CROW = 16, 17, 18, 19
LIBRARY, ARENA, THORNS, MOON, SALT = 20, 21, 22, 23, 24

# Alias — temi 25-49
MORIA, FANGORN, BARROW, CHASM = 25, 26, 27, 28
OSGILIATH, URUK_LAIR, LORIEN, GORGOROTH = 29, 30, 31, 32
HELM, DEAD_PATHS, ISENGARD, UNGOL = 33, 34, 35, 36
MIRKWOOD, BREE, SMAUG, EREBOR = 37, 38, 39, 40
DROW_DEEP, WATERDEEP, NEVERWINTER, BALDUR = 41, 42, 43, 44
SIGIL, RAVENLOFT, DRAGON_LAIR, BEHOLDER, NINE_HELLS = 45, 46, 47, 48, 49


def theme_mask(theme_ids: list[int] | tuple[int, ...] | None) -> int:
    """0 = universale (fallback ovunque). Altrimenti bitmask temi ammessi."""
    if not theme_ids:
        return 0
    mask = 0
    for tid in theme_ids:
        if 0 <= tid < THEME_COUNT:
            mask |= 1 << tid
    return mask


def emit_theme_mask_array(entries: list[dict]) -> str:
    lines: list[str] = []
    for idx, entry in enumerate(entries):
        mask = theme_mask(entry.get("themes"))
        comment = entry.get("short", entry.get("keywords", ""))
        lines.append(f"\t{mask}ull, /* idx {idx} {comment} */")
    return "\n".join(lines)
