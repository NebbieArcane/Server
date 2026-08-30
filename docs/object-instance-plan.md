# Object instance — piano fasi (libero range 34k)

Obiettivo: liberare i vnum `34030–35999` (`LOW_EDITED_ITEMS`..`HIGH_EDITED_ITEMS`) senza perdere l’eq editato. Ogni pezzo editato diventa una **istanza** (`object_instance`) agganciata a un **base_vnum** mondo, non un prototipo in `objects/`.

## Schema (ODB)

Definizione in `src/odb/account.hpp` (model version **1.12**):

- tabelle `object_instance`, `object_instance_affect`, `object_instance_event`
- su `object_instance`: `owner_toon_id` / `owner_name`, `created_by_*`, `updated_by_*`,
  **`deleted` / `deleted_on`**,   **`source`** (`procarea_loot`, `god_edit`, `clan_symbol`)
- colonna `character_inventory.instance_id` (NULL = legacy)
- su `character_stats` (**1.8**): `edit_hp/mana/move` + regen, `overedit_*`,
  `edit_pool_migrated` — pool listino sul PG (cap attivo + credito overedit)

Al boot `Sql::dbUpdate()` fa `odb::schema_catalog::migrate`, poi **sempre**
`account_schema_heal()`: applica in modo idempotente **tutti** gli step dalla
version 2 fino alla current (ADD COLUMN / CREATE TABLE|INDEX, ignora duplicati)
e allinea `schema_version` se il contatore e' rimasto indietro per via del
DDL auto-commit di MySQL. A ogni nuovo model `1.N` aggiungere `heal_vN` in
`odb_schema_heal.cpp`.

Model **1.9**: tabella `procarea_balance` (densita'/premi Dimensione Effimera, WIZ).

Model **1.10**: colonna `object_instance.source` (origine edit / premio procarea).

Model **1.11**: `object_instance.dust_hp/mana/move` + regen — polvere achievement
(`ITEM2_DUSTED`). Non entra nell'edit pool; strip rimette proto pool poi re-applica
`dust_*`. `use` polvere incrementa i contatori; se il pezzo e' eleggibile (34k /
instance) e non ha instance, `object_instance_persist`. All'`osave db`, se
`DUSTED` e `dust_*` a 0, cattura il delta pool vs proto (hp non si editano piu'
sull'eq via EditMaster). Heal boot: instance gia' convertita + `DUSTED` +
`dust_*` 0 + extra ancora presente → copia in `dust_*`.

Model **1.12**: `dust_spellfail` (tracciato, non ancora nello strip pool). Comando
wiz `odust` per correggere contatori + APPLY.

Dopo aver toccato `account.hpp`: ricompila (CMake target `account` / `build.sh`) così ODB rigenera `account-*-mysql.*` e il changelog. In alternativa: container `nebbiearcane/mudcompiler` con ODB 2.5.

## Edit pool (hp/mana/move/regen → PG)

- Massimali listino: hit 100, mana 150, move 100, regen 50 ciascuno.
- `edit_*` applicati in `hit_limit` / `mana_limit` / `move_limit` / `*_gain`.
- `overedit_*` solo credito (nessun effetto in gioco; policy refund/uso TBD).
- Solo pezzi edit (range 34k o `db_instance_id`); credit/strip login solo se `pers_on`.
- **`ITEM_CLAN_SYMBOL`**: mai strip/credit pool (simbolo resta sull'oggetto).
- Boot: `edit_pool_boot_migrate` strippa APPLY pool dalle `object_instance` e accredita l'owner
  (skip `type_flag == ITEM_CLAN_SYMBOL`).
  Per ogni istanza toccata scrive `object_instance_event` (`kind=edit_pool`, actor `edit_pool_boot`)
  con detail degli APPLY rimossi e delta accreditato.
- Login: `edit_pool_migrate_char` strippa residui su inventorio/eq (event `edit_pool` /
  `edit_pool_login`); credit solo se `migrated=0`.
- **Premi procarea (`65100–65325`)**: mai edit pool, anche con `db_instance_id`.
  I bonus rolled al drop restano sull'oggetto; salvataggio staff con
  `osave <obj> db procarea` (o `db <651xx>`). Non usare `osave <obj> 651xx` su file.
  Al drop: flag runtime `ITEM2_PROCAREA_REWARD` (extra2 PROCAREA-REWARD).
- EditMaster: rifiuta hp/mana/move/regen sull'eq.
- **Polvere (`ITEM2_DUSTED` / `dust_*`)**: extra pool vs proto da achievement; non
  accreditata nel listino; dopo strip viene re-applicata sull'eq.

## Simbolo di casata (`ITEM_CLAN_SYMBOL`)

- Tipo `31`, wear `WEAR_CLAN_SYMBOL` (23), flag `ITEM_WEAR_CLAN_SYMBOL`.
- Label eq: `<simbolo del clan>`; V0 = `toon_id` del principe.
- Cap listino: ≤2 dam, ≤1 spell affect, no resistenze; no re-edit / no refund.
- Tabella MySQL `clan_symbol` (CREATE IF NOT EXISTS a boot): `vnum` legacy 34k,
  `base_vnum`, `prince_name`, `prince_toon_id`, `instance_id` (template shared),
  `active`, `updated_at`. Seed una tantum da lista in `clan_symbol.cpp` (non
  sovrascrive righe esistenti). `prince_toon_id` risolto per nome su questo DB.
- Conversione: `clan_symbol_boot_migrate()` dopo `object_instance_boot_migrate`,
  prima di `edit_pool_boot_migrate`. Crea/aggiorna `object_instance` (legacy_edit_vnum
  = 34k), relink inventori (`item_number`→base, `instance_id`, V0, wear_pos),
  legacyimport PG non migrati che ancora tengono il 34k, archivia `objects/<34k>`
  quando non restano ref. Idempotente. I vnum in lista sono skippati da
  `object_instance_boot_migrate` (non sono edit personali).
- Wear: solo principe / vassalli di quel nome (imm ok).
- Lista di riferimento staff: `tools/clan_symbol_vnums.txt` (non richiesta a runtime).

## Ownership e audit

- Runtime: `obj_data::personal_owner` (preferito da `pers_on`).
- Legacy: keyword `ED<name>` ancora scritta da `SetPersonOnSave` per rent/file.
- In DB le keyword sono **senza** `ED*`; owner in `owner_name`.
- Ogni `osave … db` / sync scrive un `object_instance_event` (`create` / `update`).
- `odelete db` = soft-delete: event `delete`, riga resta con `deleted=1`.
- Numeri wiz = **lista densa** (`show db` / `show db deleted`), non il pk MySQL.
- Schema 1.6: colonna `object_instance_event.detail` (TEXT).
- Schema 1.7: soft-delete `object_instance.deleted` / `deleted_on`.
- Schema 1.10: `object_instance.source` — `procarea_loot` | `god_edit` | `clan_symbol`.
- Schema 1.11: `object_instance.dust_*` + extra2 `DUSTED` (polvere vs edit pool).
- Schema 1.12: `object_instance.dust_spellfail`; comando `odust`.

## Fase 0

- Model ODB account 1.4 + migrate automatico.
- `obj_data::db_instance_id` riservato.

## Fase 1

- `osave <obj> db [base_vnum]` / file path invariato.
- Inventorio MySQL: `instance_id` + overlay al load.
- `oload db <id>`, `show instances`, `where instance <id>`, `stat` mostra instance id.
- Help: OSAVE, OLOAD, SHOW, STAT, WHERE.

## Fase 1.5 (owner + audit)

- Schema 1.5: owner / created_by / updated_by + `object_instance_event`.
- Persist strip ED, valorizza owner e actor, event trail.
- `pers_on` dual-path (`personal_owner` poi `ED*`).
- `stat` / `show instances` mostrano owner ed ultimo event.

## Boot — rari (`CountLimitedItemsMysql`)

**Obbligatorio** finché esistono istanze in inventorio:

- senza `instance_id`: rarità = `cost` del prototipo `item_number` (come oggi);
- con `instance_id`: rarità = `object_instance.cost` (non il base); incrementa `obj_index` del **`base_vnum`**;
- al load online continua a non ricontare i rari già sommati al boot (decremento dopo `read_object` / apply).

Nota: `show obj` colonna `db` conta ancora per `item_number` (= base dopo osave db); eventuale join su istanze è cleanup successivo.

## Fase 2 — migrazione stock 34k

Al **boot** (`object_instance_boot_migrate`, prima di `cleanup_migrated_legacy_files`):

1. Solo pezzi **OK_HEADER** con **ED*** (proprietario): `#header` fuori dal 34k,
   `real_object(base) >= 0`, keyword `EDnomepg`. Senza ED restano in `objects/`.
2. Insert/riusa `object_instance` (`legacy_edit_vnum = N`, `base_vnum` da header).
   Pezza transitoria (`procarea_legacy_drop.cpp`): per 34868/34869 il create
   event viene riscritto con la foto **drop** (non il 34k). Eq live invariata.
   Idempotente (`note` contiene `legacy_drop_baseline`). Rimuovere dopo
   migrate produzione.
3. PG non migrati che ancora tengono `N` (rent o inventorio) → **legacyimport** completo.
4. Aggiorna inventori MySQL: `item_number=base`, `instance_id`.
5. Riscrive rent migrati `N→base`; `objects/N` → `deleted/objects/` solo se non restano ref
   (collisioni: `N.<timestamp>`).

**Load/refund unificato** (`object_instance_load_stored` / `normalize_stored`):
- inventorio MySQL, rent file, refund zip→`save_rent_mysql`, legacyimport, count rari
- se `item_number` 34k e esiste istanza `legacy_edit_vnum` → materializza base+instance
- SCRAP salva anche `instance_id`
- ghost/forcerent: usano gli stessi path di load/save (niente ramo separato)

Nomi in `objects/` accettati solo se interamente numerici (`.bak` e simili non rientrano in index).

Pezzi `NEED_BASE` / hint: ancora revisione manuale.

## Fase 3 — cleanup

- Comandi wiz (`show obj` / `where`) per istanze.
- `show obj` / conteggi `db` consapevoli di `instance_id` (oltre al boot rari già in 1.5).
- Eventuale rimozione definitiva del range riservato.
- `oload db` per nome/owner; drop dipendenza da `ED*` a runtime.

## Non fare ancora

- Migrazione massiva 34k in produzione senza backup esterni.
- Pezzi NEED_BASE / hint senza revisione.
