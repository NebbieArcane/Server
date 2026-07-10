#!/usr/bin/env python3
"""Simula roll classe boss procarea + regola add CL/MU vs random."""

from __future__ import annotations

import random
from collections import Counter
from dataclasses import dataclass
from typing import Literal

RollClass = Literal[
    "MU_CL", "CL", "MU", "W", "TH", "PSI", "DR", "MO", "BB", "PA", "RA"
]

# Boss: prevalentemente CL/MU (70% castery), 30% altre classi singole.
BOSS_WEIGHTS: dict[RollClass, int] = {
    "MU_CL": 30,
    "CL": 10,
    "MU": 12,
    "W": 8,
    "TH": 5,
    "PSI": 6,
    "DR": 7,
    "MO": 4,
    "BB": 7,
    "PA": 5,
    "RA": 6,
}

MELEE_CLASSES = frozenset({"W", "TH", "MO", "BB"})
CASTER_CLASSES = frozenset({"CL", "MU", "PSI", "DR", "PA", "RA"})
HYBRID_CAST = frozenset({"MU_CL", "PA", "RA"})


@dataclass
class BossBuild:
    roll: RollClass
    acts: set[str]
    has_mu: bool
    is_melee: bool
    is_caster: bool
    mult_att_bonus: int
    dam_bonus_pct: int
    spellpower_bonus: int


def boss_has_mu(acts: set[str]) -> bool:
    return "MU" in acts


def roll_weighted(weights: dict[RollClass, int]) -> RollClass:
    items = list(weights.items())
    total = sum(w for _, w in items)
    r = random.randint(0, total - 1)
    acc = 0
    for key, w in items:
        acc += w
        if r < acc:
            return key
    return items[-1][0]


def solo_non_fighter_boss() -> BossBuild:
    """Regola attuale: solo PG non-basher → boss solo CL."""
    return build_boss("CL", solo_non_fighter=True)


def build_boss(roll: RollClass, solo_non_fighter: bool = False) -> BossBuild:
    if solo_non_fighter:
        roll = "CL"

    acts: set[str] = set()
    if roll == "MU_CL":
        acts = {"MU", "CL"}
    elif roll == "MU":
        acts = {"MU"}
    elif roll == "CL":
        acts = {"CL"}
    elif roll == "TH":
        acts = {"TH"}
    elif roll == "PSI":
        acts = {"PSI"}
    elif roll == "DR":
        acts = {"DR"}
    elif roll == "PA":
        acts = {"PA"}
    elif roll == "RA":
        acts = {"RA"}
    else:
        acts = {roll}

    pure_melee = acts <= MELEE_CLASSES and roll != "TH"
    melee_focus = roll in MELEE_CLASSES or roll == "TH" or roll == "MO"
    is_caster = bool(acts & CASTER_CLASSES) or roll == "MU_CL"

    mult_att_bonus = 0
    dam_bonus_pct = 0
    spellpower_bonus = 0

    if melee_focus and not (roll == "PA" or roll == "RA"):
        if roll in MELEE_CLASSES or roll == "TH" or roll == "MO" or roll == "BB":
            mult_att_bonus = 1
            dam_bonus_pct = 15
    if roll == "PA" or roll == "RA":
        mult_att_bonus = 0
        dam_bonus_pct = 8
        spellpower_bonus = 4
    if is_caster and roll not in MELEE_CLASSES:
        spellpower_bonus = max(spellpower_bonus, 6 + (2 if roll == "MU_CL" else 0))
    if roll == "MU_CL":
        spellpower_bonus = max(spellpower_bonus, 8)

    return BossBuild(
        roll=roll,
        acts=acts,
        has_mu=boss_has_mu(acts),
        is_melee=melee_focus and roll not in {"PA", "RA", "MU_CL", "CL", "MU", "PSI", "DR"},
        is_caster=is_caster,
        mult_att_bonus=mult_att_bonus,
        dam_bonus_pct=dam_bonus_pct,
        spellpower_bonus=spellpower_bonus,
    )


def roll_add(boss: BossBuild, solo_non_fighter: bool) -> str:
    if solo_non_fighter:
        # add pool senza MU: solo CL (boss già CL senza MU)
        return "CL"

    if not boss.has_mu:
        return random.choice(["CL", "MU"])

    # random: semplificato come corridoio fascia 5
    corridor = [19, 15, 11, 4, 2, 10, 7, 10, 8, 11, 3]
    labels = ["W", "CL", "MU", "TH", "PSI", "DR", "MO", "BB", "PA", "RA", "none"]
    total = sum(corridor)
    r = random.randint(0, total - 1)
    acc = 0
    for label, w in zip(labels, corridor):
        acc += w
        if r < acc:
            return label
    return labels[-1]


def simulate(n_bosses: int = 5000, adds_per_boss: int = 2, solo_non_fighter: bool = False) -> None:
    boss_counter: Counter[str] = Counter()
    add_counter: Counter[str] = Counter()
    add_mode_cl_mu = 0
    add_mode_random = 0
    examples: list[str] = []

    for _ in range(n_bosses):
        if solo_non_fighter:
            boss = solo_non_fighter_boss()
        else:
            boss = build_boss(roll_weighted(BOSS_WEIGHTS))

        boss_counter[boss.roll] += 1
        if not boss.has_mu:
            add_mode_cl_mu += adds_per_boss
        else:
            add_mode_random += adds_per_boss

        for _ in range(adds_per_boss):
            add_counter[roll_add(boss, solo_non_fighter)] += 1

        if len(examples) < 8 and random.random() < 0.02:
            acts = "+".join(sorted(boss.acts)) if boss.acts else "nessuna"
            buffs = []
            if boss.mult_att_bonus:
                buffs.append(f"mult_att+{boss.mult_att_bonus}")
            if boss.dam_bonus_pct:
                buffs.append(f"danno+{boss.dam_bonus_pct}%")
            if boss.spellpower_bonus:
                buffs.append(f"spellpower+{boss.spellpower_bonus}")
            add_rule = "add CL/MU" if not boss.has_mu else "add random"
            examples.append(
                f"Boss [{boss.roll}] act={acts} | {', '.join(buffs) or 'no buff'} | {add_rule}"
            )

    mode = "SOLO non-basher" if solo_non_fighter else "GRUPPO"
    print(f"\n=== {mode} ({n_bosses} boss, {adds_per_boss} add ciascuno) ===")
    print("\nDistribuzione boss:")
    for k, v in boss_counter.most_common():
        print(f"  {k:5} {v/n_bosses*100:5.1f}%")

    caster_pct = sum(
        boss_counter[k] for k in boss_counter if k in {"MU_CL", "CL", "MU", "PSI", "DR", "PA", "RA"}
    ) / n_bosses * 100
    print(f"  → boss con componente caster: ~{caster_pct:.1f}%")
    print(f"  → boss SENZA MU (add forzati CL/MU): {sum(1 for _ in range(n_bosses)) - boss_counter['MU'] - boss_counter['MU_CL'] if not solo_non_fighter else n_bosses:.0f} sim")

    no_mu = sum(v for k, v in boss_counter.items() if k not in {"MU", "MU_CL"})
    print(f"  → boss senza MU: {no_mu/n_bosses*100:.1f}%")

    print("\nDistribuzione add:")
    for k, v in add_counter.most_common():
        print(f"  {k:5} {v/(n_bosses*adds_per_boss)*100:5.1f}%")

    total_adds = n_bosses * adds_per_boss
    print(f"\nRegola add: CL/MU forzato {add_mode_cl_mu/total_adds*100:.1f}% | random {add_mode_random/total_adds*100:.1f}%")

    print("\nEsempi:")
    for ex in examples:
        print(f"  • {ex}")


if __name__ == "__main__":
    random.seed(42)
    simulate(5000, 2, solo_non_fighter=False)
    simulate(2000, 2, solo_non_fighter=True)
