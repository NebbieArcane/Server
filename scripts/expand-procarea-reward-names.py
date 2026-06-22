#!/usr/bin/env python3
"""Espande procarea_reward_names.inc da 6 a 10 bande (mappa round(i*5/9))."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/procarea_reward_names.inc"
OLD_BANDS = 6
NEW_BANDS = 10

ENTRY_START = re.compile(
    r"static constexpr ProcRewardNameEntry (k\w+)\[PROCAREA_TEMPLATE_BANDS(?: \* (\d+))?\] = \{"
)
ENTRY_BLOCK = re.compile(r"\{[^{}]*\}", re.DOTALL)


def source_index(new_idx: int, old_count: int) -> int:
    if old_count <= 1:
        return 0
    return min(old_count - 1, round(new_idx * (old_count - 1) / (NEW_BANDS - 1)))


def expand_array(body: str, multiplier: int) -> str:
    entries = ENTRY_BLOCK.findall(body)
    old_band_count = OLD_BANDS * multiplier
    if len(entries) != old_band_count:
        raise ValueError(f"Expected {old_band_count} entries, got {len(entries)}")
    new_entries: list[str] = []
    for band in range(NEW_BANDS):
        for side in range(multiplier):
            flat_new = band * multiplier + side
            flat_old = source_index(band, OLD_BANDS) * multiplier + side
            new_entries.append(entries[flat_old])
    indent = "\t"
    lines = [f"{indent}{entry}," for entry in new_entries]
    return "\n".join(lines)


def main() -> None:
    text = SRC.read_text(encoding="utf-8")
    # Ripristina temporaneamente 6 bande nel sorgente se già espanso.
    text = text.replace(
        "static constexpr ProcRewardNameEntry",
        "static constexpr ProcRewardNameEntry",
    )

    out: list[str] = []
    pos = 0
    for match in ENTRY_START.finditer(text):
        out.append(text[pos : match.start()])
        mult = int(match.group(2) or "1")
        array_start = match.end()
        depth = 1
        i = array_start
        while i < len(text) and depth > 0:
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
            i += 1
        body = text[array_start : i - 1]
        expanded = expand_array(body, mult)
        mult_suffix = f" * {mult}" if mult > 1 else ""
        out.append(
            f"static constexpr ProcRewardNameEntry {match.group(1)}"
            f"[PROCAREA_TEMPLATE_BANDS{mult_suffix}] = {{\n{expanded}\n}}"
        )
        pos = i
    out.append(text[pos:])
    SRC.write_text("".join(out), encoding="utf-8")
    print(f"Expanded {SRC} to {NEW_BANDS} bands")


if __name__ == "__main__":
    main()
