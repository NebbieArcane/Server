# Object instance — piano fasi (libero range 34k)

Obiettivo: liberare i vnum `34030–35999` (`LOW_EDITED_ITEMS`..`HIGH_EDITED_ITEMS`) senza perdere l’eq editato. Ogni pezzo editato diventa una **istanza** (`object_instance`) agganciata a un **base_vnum** mondo, non un prototipo in `objects/`.

## Schema (ODB)

Definizione in `src/odb/account.hpp` (model version **1.7**):

- tabelle `object_instance`, `object_instance_affect`, `object_instance_event`
- su `object_instance`: `owner_toon_id` / `owner_name`, `created_by_*`, `updated_by_*`,
  **`deleted` / `deleted_on`** (soft-delete)
- colonna `character_inventory.instance_id` (NULL = legacy)

Al boot `Sql::dbUpdate()` fa `odb::schema_catalog::migrate` — **niente DDL manuale / script SQL da applicare a mano**.

Dopo aver toccato `account.hpp`: ricompila (CMake target `account` / `build.sh`) così ODB rigenera `account-*-mysql.*` e il changelog. In alternativa: container `nebbiearcane/mudcompiler` con ODB 2.5.

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

1. Per ogni `objects/N` nel range: insert istanza (`legacy_edit_vnum = N`, `base_vnum` da `char_vnum` / parent).
2. Aggiorna inventari / rent / ground che riferiscono `N` → `base + instance_id`.
3. Archivia file; togli range da `obj_index` / zone che lo aspettano vuoto.

## Fase 3 — cleanup

- Comandi wiz (`show obj` / `where`) per istanze.
- `show obj` / conteggi `db` consapevoli di `instance_id` (oltre al boot rari già in 1.5).
- Eventuale rimozione definitiva del range riservato.
- `oload db` per nome/owner; drop dipendenza da `ED*` a runtime.

## Non fare ancora

- Migrazione massiva 34k in produzione.
- Cancellazione `objects/` edit senza backup.
