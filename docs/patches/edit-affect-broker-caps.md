# EditAffectBroker — caps, armi, DAM+SP

Implementato in:

- `src/mob.editor.cpp`
- `src/edit_broker_whitelist.inc` (Enable/Disable per location e per bit oedit)
- `src/object_instance.cpp`
- `src/object_instance.hpp`
- `src/obj_value.cpp` (etichette `[delta]` / `[broker]` in show edits)

## Regole implementate

### Blocchi
- `APPLY_WEAPON_SPELL` / `EAT_SPELL` / `RACE_SLAYER` / `ALIGN_SLAYER` → messaggio staff
- Bit fuori listino (resi/imm/spell) → staff
- Bit già presente su B → stop con nome bit
- Non-stack non-arma già su B → stop
- Fuori dalla **tabella** listino-edit (`SAVING_*` / `SAVE_ALL`, spellfail, CON, …):
  non compaiono in lista e non si risolvono al pick. Caso tipico: baseline procarea con
  `SAVE_ALL -5` poi tolto → senza whitelist `cur>base` inventava `SAVING_ALL by 5`.
- Hit/Mana/Move e i regen: in tabella ma edit sul personaggio → non trasferibili sul pezzo.
  Artifact e spell arma restano fuori dal path affect (flag / staff).
- `ITEM2_PAID_MALUS` su A: il recupero malus proto (pagato a listino doppio) **non** è
  trasferibile. Solo la parte sopra 0 (AC: sotto 0). Es. proto INT −3, pezzo a +2 →
  `INT by 2`. Vale per tutte le location numeriche a segno / canali combat.

### Accorpabili (bitfield: 4 insiemi, coesistenza su B)
Famiglie diverse **possono** stare assieme sul destinazione. Config in `edit_broker_whitelist.inc` (`BrokerBitClass` + Enable/Disable). SUSC: Disable finché non abilitate.

- Resistenze (`APPLY_IMMUNE`): acid/elec/fire/cold/energy/drain/hold/poison + slash/pierce/blunt
- Immunità (`APPLY_M_IMMUNE`): drain/charm/poison
- Spell (`APPLY_SPELL` + `AFF2`): telepathy, darkness, waterbreath, truesight, invis, sense life, spy/scrying, prot evil, fly, danger sense
- Suscettibilità (`APPLY_SUSC`): non trasferibili (Disable)

Placement: pacchetti = slot su A con bit Enable del delta. Default Merge sullo stesso slot di B (o primo libero). Se B ha già l'insieme e ha slot liberi, o se A ha più pacchetti e B ha ≥2 liberi: in conferma `si`/`nod` = accorpa, `nuovo` = slot separati.

### Accorpabili (numerici — tabella listino-edit, solo pezzo)
- Stats: STR/DEX/INT/WIS/CHR
- Affects pezzo: armor, hitroll, damroll, spellpower
- Accorpamenti: hit-n-dam, hit-n-sp
- Non trasferibili (pool PG): hit, mana, move, hit/mana/move regen
- Tetti pezzo (sotto) restano su combat/stats/armor come prima

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
