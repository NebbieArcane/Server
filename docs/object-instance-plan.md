# Object instance — piano fasi (libero range 34k)

Obiettivo: liberare i vnum `34030–35999` (`LOW_EDITED_ITEMS`..`HIGH_EDITED_ITEMS`) senza perdere l’eq editato. Ogni pezzo editato diventa una **istanza** (`object_instance`) agganciata a un **base_vnum** mondo, non un prototipo in `objects/`.

## Schema (ODB)

Definizione in `src/odb/account.hpp` (model version **1.4**):

- tabelle `object_instance`, `object_instance_affect`
- colonna `character_inventory.instance_id` (NULL = legacy)

Al boot `Sql::dbUpdate()` fa `odb::schema_catalog::migrate` — **niente DDL manuale / script SQL da applicare a mano**.

Dopo aver toccato `account.hpp`: ricompila (CMake target `account` / `build.sh`) così ODB rigenera `account-*-mysql.*` e il changelog.

## Fase 0

- Model ODB account 1.4 + migrate automatico.
- `obj_data::db_instance_id` riservato.

## Fase 1 (in corso)

- `osave <obj> db [base_vnum]` → `object_instance` (file path invariato).
- Inventorio MySQL: colonna `instance_id`; load applica overlay istanza.
- `oedit` lavora sull'oggetto in mano (gia' materializzato).

## Fase 2 — migrazione stock 34k

1. Per ogni `objects/N` nel range: insert istanza (`legacy_edit_vnum = N`, `base_vnum` da `char_vnum` / parent).
2. Aggiorna inventari / rent / ground che riferiscono `N` → `base + instance_id`.
3. Archivia file; togli range da `obj_index` / zone che lo aspettano vuoto.

## Fase 3 — cleanup

- Comandi wiz (`show obj` / `where`) per istanze.
- Rare count su `cost` istanza / base.
- Eventuale rimozione definitiva del range riservato.

## Non fare ancora

- Migrazione massiva 34k in produzione.
- Cancellazione `objects/` edit senza backup.
