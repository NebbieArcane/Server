# EditAffectBroker — caps, armi, DAM+SP

Implementato in:

- `src/mob.editor.cpp`
- `src/object_instance.cpp`
- `src/object_instance.hpp`
- `src/obj_value.cpp` (etichette `[delta]` / `[broker]` in show edits)

## Regole implementate

### Blocchi
- `APPLY_WEAPON_SPELL` / `EAT_SPELL` / `RACE_SLAYER` / `ALIGN_SLAYER` → messaggio staff
- Bit fuori listino (resi/imm/spell) → staff
- Bit già presente su B → stop con nome bit
- Non-stack non-arma già su B → stop

### Accorpabili
- Resistenze (`APPLY_IMMUNE`): acid/elec/fire/cold/energy/drain/hold/poison + slash/pierce/blunt
- Immunità (`APPLY_M_IMMUNE`): drain/charm/poison
- Spell (`APPLY_SPELL` + `AFF2`): telepathy, darkness, waterbreath, truesight, invis, sense life, spy/scrying, prot evil, fly, danger sense
- Numerici: hitroll, damroll, spellpower, armor, hit-n-dam, hit-n-sp, STR/DEX/INT/WIS/CHR

### Tetti pezzo (edit + broker, una volta ciascuno)
| Canale | edit | broker |
|--------|------|--------|
| HR effective (HR+HITNDAM+HITNSP) | +2 | +2 |
| DAM effective (DAM+HITNDAM) | +2 | +2 |
| SP effective (SP+HITNSP) | +2 | +2 |
| Stats (no CON) | +3 | +3 |
| Armor | −40 | −40 |

Broker già su B: somma `broker_delta` dagli event `affect_transfer` con note `brokeraggio ADD` e `location=` / `broker_delta=` nel detail.

### Toon-wide
- DAM+SP edit ≤ 30 (HITNDAM→dam, HITNSP→sp), come `show edits`
- Net −A +B; rifiuta se projected > 30

## History
Detail transfer include `location=%d` e `broker_delta=%+d` per i transfer successivi.
