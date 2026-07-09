#!/usr/bin/env python3
"""Accorcia le description dei premi procarea: monoriga, max ~72 caratteri, niente \\n interni."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NAMES_INC = ROOT / "src/procarea_reward_names.inc"
SPAWN_CPP = ROOT / "src/procarea_spawn.cpp"
MAX_LEN = 72

DESC_PAIR_RE = re.compile(
    r'(?P<prefix>\t  )"(?P<l1>(?:[^"\\]|\\.)*\\n)"\s*\n\s*"(?P<l2>(?:[^"\\]|\\.)*\\n)"'
)
DESC_PAIR_INLINE_RE = re.compile(
    r'"(?P<l1>(?:[^"\\]|\\.)*\\n)"\s*"(?P<l2>(?:[^"\\]|\\.)*\\n)"'
)
SHIELD_DESC_RE = re.compile(
    r'(?P<prefix>\t\t)"(?P<l1>(?:[^"\\]|\\.)*\\n)"\s*\n\s*"(?P<l2>(?:[^"\\]|\\.)*\\n)"'
)


def decode_c_string(raw: str) -> str:
    return bytes(raw, "utf-8").decode("unicode_escape")


def encode_c_string(text: str, trailing_newline: bool = False) -> str:
    body = text.rstrip("\n\r")
    if trailing_newline:
        body += "\n"
    body = body.replace("\\", "\\\\").replace('"', '\\"')
    body = body.replace("\n", "\\n")
    return body


def compress_phrases(text: str) -> str:
    replacements = (
        ("E' stato forgiato", "Forgiato"),
        ("si fa piu' denso", "e' piu' denso"),
        ("Pare forgiato", "Forgiato"),
        ("pare forgiato", "forgiato"),
    )
    for old, new in replacements:
        text = text.replace(old, new)
    return text


def finalize(text: str) -> str:
    cleaned = re.sub(r"\s+", " ", text).strip().rstrip(".,;:")
    if len(cleaned) > MAX_LEN:
        cleaned = cleaned[:MAX_LEN].rsplit(" ", 1)[0].rstrip(".,;:")
    # Evita frasi troncate su articolo/pronome (es. "distogli lo.").
    while cleaned and re.search(
        r"\b(lo|la|li|le|gli|ne|ci|vi|si|un|una|il|che|ad|a|di|da|su|per|con|non|"
        r"ogni|del|della|dei|delle|al|alla|nel|nella|sul|sulla|col|coi)\s*$",
        cleaned,
        re.I,
    ):
        cleaned = cleaned.rsplit(" ", 1)[0].rstrip(".,;:")
    return cleaned + "."


def shorten_description(line1: str, line2: str) -> str:
    a = compress_phrases(line1.rstrip("\n").strip())
    b = line2.rstrip("\n").strip()

    if not b:
        return finalize(a)

    if len(a.rstrip(".,;:")) <= MAX_LEN:
        return finalize(a)

    for sep in (";", ":"):
        if sep in a:
            head = a.split(sep, 1)[0].strip()
            tail_b = re.sub(
                r"^(chi lo regge |chi la indossa |chi lo indossa |al dito (?:sinistro|destro) )",
                "",
                b,
                flags=re.I,
            )
            combo = compress_phrases(f"{head}; {tail_b}")
            if len(combo.rstrip(".,;:")) <= MAX_LEN:
                return finalize(combo)
            if len(head.rstrip(".,;:")) <= MAX_LEN:
                return finalize(head)

    joined = compress_phrases(f"{a} {b}")
    if len(joined.rstrip(".,;:")) <= MAX_LEN:
        return finalize(joined)

    return finalize(a)


def replace_desc_pair(match: re.Match[str]) -> str:
    l1 = decode_c_string(match.group("l1"))
    l2 = decode_c_string(match.group("l2"))
    merged = shorten_description(l1, l2)
    prefix = match.group("prefix")
    return f'{prefix}"{encode_c_string(merged)}"'


def replace_desc_pair_inline(match: re.Match[str]) -> str:
    l1 = decode_c_string(match.group("l1"))
    l2 = decode_c_string(match.group("l2"))
    merged = shorten_description(l1, l2)
    return f'"{encode_c_string(merged)}"'


def process_names_inc(text: str) -> str:
    text = DESC_PAIR_RE.sub(replace_desc_pair, text)
    text = DESC_PAIR_INLINE_RE.sub(replace_desc_pair_inline, text)
    return text


def process_spawn_cpp(text: str) -> str:
    text = SHIELD_DESC_RE.sub(replace_desc_pair, text)
    text = text.replace(
        '"Il metallo sembra trattenuto da rune instabili:\\n"\n'
        '\t\t\t\t\t  "e\' un premio nato nella Dimensione Effimera, fuori dal tempo di Myst.\\n"',
        '"Rune instabili lo tengono fuori dal tempo di Myst.\\n"',
        1,
    )
    return text


def main() -> None:
    names = NAMES_INC.read_text(encoding="utf-8")
    names_new = process_names_inc(names)
    if names_new.count("\\n\"") - names_new.count("short_descr"):
        pass
    remaining = len(re.findall(r'\\n"\s*\n\s*"', names_new))
    if remaining:
        raise SystemExit(f"procarea_reward_names.inc: restano {remaining} description su due righe")
    NAMES_INC.write_text(names_new, encoding="utf-8")
    print(f"Updated {NAMES_INC}")

    spawn = SPAWN_CPP.read_text(encoding="utf-8")
    spawn_new = process_spawn_cpp(spawn)
    remaining_sh = len(SHIELD_DESC_RE.findall(spawn_new))
    if remaining_sh:
        raise SystemExit(f"procarea_spawn.cpp: restano {remaining_sh} scudi su due righe")
    SPAWN_CPP.write_text(spawn_new, encoding="utf-8")
    print(f"Updated {SPAWN_CPP}")


if __name__ == "__main__":
    main()
