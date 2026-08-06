# Object instance — piano fasi (libero range 34k)

Obiettivo: liberare i vnum `34030–35999` (`LOW_EDITED_ITEMS`..`HIGH_EDITED_ITEMS`) senza perdere l’eq editato. Ogni pezzo editato diventa una **istanza** (`object_instance`) agganciata a un **base_vnum** mondo, non un prototipo in `objects/`.

## Schema (ODB)

Definizione in `src/odb/account.hpp` (model version **1.8**):

- tabelle `object_instance`, `object_instance_affect`, `object_instance_event`
- su `object_instance`: `owner_toon_id` / `owner_name`, `created_by_*`, `updated_by_*`,
  **`deleted` / `deleted_on`** (soft-delete)
- colonna `character_inventory.instance_id` (NULL = legacy)
- su `character_stats` (**1.8**): `edit_hp/mana/move` + regen, `overedit_*`,
  `edit_pool_migrated` — pool listino sul PG (cap attivo + credito overedit)

Al boot `Sql::dbUpdate()` fa `odb::schema_catalog::migrate` — **niente DDL manuale / script SQL da applicare a mano**.

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
- EditMaster: rifiuta hp/mana/move/regen sull'eq.

## Simbolo di casata (`ITEM_CLAN_SYMBOL`)

- Tipo `31`, wear `WEAR_CLAN_SYMBOL` (23), flag `ITEM_WEAR_CLAN_SYMBOL`.
- Label eq: `<come simbolo di casata>`; V0 = `toon_id` del principe.
- Cap listino: ≤2 dam, ≤1 spell affect, no resistenze; no re-edit / no refund.
- Conversione legacy: lista vnum in `tools/clan_symbol_vnums.txt` (da staff) → migrate
  type/wear/V0 su objects + inventori + istanze (TODO quando arriva la lista).

## Ownership e audit

- Runtime: `obj_data::personal_owner` (preferito da `pers_on`).
- Legacy: keyword `ED<name>` ancora scritta da `SetPersonOnSave` per rent/file.
- In DB le keyword sono **senza** `ED*`; owner in `owner_name`.
- Ogni `osave … db` / sync scrive un `object_instance_event` (`create` / `update`).
- `odelete db` = soft-delete: event `delete`, riga resta con `deleted=1`.
- Numeri wiz = **lista densa** (`show db` / `show db deleted`), non il pk MySQL.
- Schema 1.6: colonna `object_instance_event.detail` (TEXT).
- Schema 1.7: soft-delete `object_instance.deleted` / `deleted_on`.

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
