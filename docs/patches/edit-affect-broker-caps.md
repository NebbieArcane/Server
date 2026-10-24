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

### Accorpabili (bitfield: 4 classi esclusive su B)
Sul pezzo destinazione queste classi **non si combinano** tra loro (contano anche bit già presenti dal proto). Dentro la stessa classe i bit possono stare assieme (OR sullo stesso slot di location).

- Resistenze a spell (`APPLY_IMMUNE`): acid/elec/fire/cold/energy/drain/hold/poison
- Resistenze ai fisici (`APPLY_IMMUNE`): slash/pierce/blunt
- Immunità (`APPLY_M_IMMUNE`): drain/charm/poison
- Spell (`APPLY_SPELL` + `AFF2`): telepathy, darkness, waterbreath, truesight, invis, sense life, spy/scrying, prot evil, fly, danger sense

Lista transfer: `APPLY_IMMUNE` compare come due righe separate (spell vs fisici) se entrambe sono presenti come delta.

### Accorpabili (numerici)
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

### PROCAREA-REWARD (baseline)
- Prototipo broker/distruggi = **create-snapshot** (bonus rolled inclusi), non il template mondo 651xx.
- Premi già in DB: `object_instance_materialize_create_baseline`.
- Destinazione B senza instance: in anteprima/tetti si usa una **baseline sospesa** (affect rolled attuali in memoria). La scrittura MySQL avviene solo nel **commit atomico** a conferma (create-event = rolled, poi stato post-transfer + eventi broker). Annulla / timeout / fail commit → nessun record DB, pezzo invariato. Poi B diventa EDIT+PERSONAL.
- Pezzo A già istanziato: delta trasferibile solo vs create-baseline (i rolled non sono più “edit”).
- Distruggi e analisi staff: stesso create-baseline (`AnalyzeProcareaStaffEdit` / `AnalyzeObjEditAgainst`).

## History
Detail transfer include `location=%d` e `broker_delta=%+d` per i transfer successivi.
