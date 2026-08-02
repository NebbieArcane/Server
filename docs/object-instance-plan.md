# Object instance — piano fasi (libero range 34k)

Obiettivo: liberare i vnum `34030–35999` (`LOW_EDITED_ITEMS`..`HIGH_EDITED_ITEMS`) senza perdere l’eq editato. Ogni pezzo editato diventa una **istanza** (`object_instance`) agganciata a un **base_vnum** mondo, non un prototipo in `objects/`.

## Schema (ODB)

Definizione in `src/odb/account.hpp` (model version **1.4**):

- tabelle `object_instance`, `object_instance_affect`
- colonna `character_inventory.instance_id` (NULL = legacy)

Al boot `Sql::dbUpdate()` fa `odb::schema_catalog::migrate` — **niente DDL manuale / script SQL da applicare a mano**.

Dopo aver toccato `account.hpp`: ricompila (CMake target `account` / `build.sh`) così ODB rigenera `account-*-mysql.*` e il changelog.

## Fase 0 (attuale)

- Model ODB + migrate automatico.
- Nessun load/save/osave ancora usa `instance_id`.
- File in `objects/34xxx` e flusso oedit/osave **invariati**.
- `obj_data::db_instance_id` riservato per wiring futuro.

## Fase 1 — nuovi edit senza file

1. Nuovo oedit: crea riga `object_instance` + affect; **non** assegna vnum 34k / `osave` file.
2. Equip: `character_inventory.item_number = base_vnum`, `instance_id` valorizzato.
3. Load inventorio: se `instance_id` → `read_object(base)` + overlay da tabella.
4. Save inventorio: aggiorna riga istanza (allineare un solo path di write).

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
