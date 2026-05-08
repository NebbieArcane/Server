import {
  Card, CardBody, CardHeader,
  Divider, Grid, H1, H2,
  Pill, Row, Stack, Stat, Table, Text,
  useHostTheme,
} from 'cursor/canvas';

const devA = 'Dev A — Database';
const devB = 'Dev B — C++17';
const devBoth = 'Entrambi';

export default function Roadmap() {
  useHostTheme();

  const badge = (who: string) => {
    const tone = who === devA ? 'info' : who === devB ? 'warning' : 'neutral';
    const short = who === devBoth ? 'Entrambi' : who === devA ? 'Dev A' : 'Dev B';
    return <Pill tone={tone} size="sm">{short}</Pill>;
  };

  return (
    <Stack gap={28} style={{ padding: 24, maxWidth: 960, margin: '0 auto' }}>
      <Stack gap={4}>
        <H1>Roadmap — DB e C++17 in parallelo</H1>
        <Text tone="secondary">
          Un dev lavora al database, l'altro alla modernizzazione C++17 — in contemporanea.
          Funziona, ma serve una regola fondamentale per evitare conflitti.
        </Text>
      </Stack>

      <Grid columns={3} gap={12}>
        <Stat value="1" label="Regola anti-conflitto" tone="warning" />
        <Stat value="4" label="File condivisi a rischio" tone="warning" />
        <Stat value="2x" label="Velocità teorica vs sequenziale" tone="success" />
      </Grid>

      <Divider />

      {/* ── Il problema centrale ── */}
      <Stack gap={12}>
        <H2>Il problema: structs.hpp è il punto di conflitto</H2>
        <Text>
          Entrambi i lavori toccano gli stessi file. Se un dev cambia `char*` in `std::string` in
          `structs.hpp` mentre l'altro ci aggiunge i `#pragma db` di ODB, i merge saranno un incubo continuo.
        </Text>
        <Table
          headers={['File', 'C++17 lo tocca?', 'DB lo tocca?', 'Rischio']}
          rows={[
            ['src/structs.hpp', 'Sì — char* → string, macro → metodi', 'Sì — pragma db, nuovi tipi', 'ALTO'],
            ['src/db.cpp', 'Sì — FILE* → stream, globals', 'Sì — nuove funzioni load/save DB', 'ALTO'],
            ['src/reception.cpp', 'Sì — stile legacy', 'Sì — load_char_objs da DB', 'MEDIO'],
            ['src/utils.hpp', 'Sì — macro GET_* → metodi', 'No', 'BASSO'],
          ]}
          rowTone={['danger', 'danger', 'warning', undefined]}
        />
      </Stack>

      <Divider />

      {/* ── La regola che risolve tutto ── */}
      <Card>
        <CardHeader>La regola che rende possibile il lavoro parallelo</CardHeader>
        <CardBody>
          <Stack gap={12}>
            <Text>
              Dev A **non tocca structs.hpp**.
              Le classi ODB per il DB vivono in file **separati** dentro `src/odb/`
              — non aggiunge pragma alle struct di gioco esistenti.
            </Text>
            <Text>
              Dev B **non tocca src/odb/ e src/Sql.cpp**.
              Modernizza tutto il resto senza entrare nel layer DB.
            </Text>
            <Text tone="secondary">
              Le "classi ODB" sono modelli DB autonomi (es. `CharacterRecord` in `odb/character.hpp`)
              che vengono popolati leggendo i campi di `char_data` — non dipendono
              dalla sua rappresentazione interna. Quando i due branch si fondono, si scrive un unico
              adapter `to_db()` / `from_db()` che fa il bridge.
            </Text>
          </Stack>
        </CardBody>
      </Card>

      <Divider />

      {/* ── Divisione lavori ── */}
      <Stack gap={12}>
        <H2>Divisione dei compiti</H2>
        <Grid columns={2} gap={16}>

          <Card>
            <CardHeader trailing={badge(devA)}>Dev A — Database</CardHeader>
            <CardBody>
              <Stack gap={10}>
                <Text size="small" tone="secondary">
                  File esclusivi: `src/odb/*`, `src/Sql.cpp`, `src/Sql.hpp`, `src/db.cpp` (solo nuove funzioni)
                </Text>
                <Divider />
                <Text size="small" weight="semibold">Personaggio e inventario</Text>
                <Table
                  headers={['Compito']}
                  rows={[
                    ['Definire CharacterRecord, SkillRecord in odb/character.hpp'],
                    ['Schema: character_stats, character_skills, character_affects'],
                    ['Schema: character_equipment, character_inventory'],
                    ['Script migrazione players.dat + rent/ → DB'],
                    ['load_char_from_db() / save_char_to_db() in parallelo ai file'],
                  ]}
                />
                <Divider />
                <Text size="small" weight="semibold">Mondo e achievements</Text>
                <Table
                  headers={['Compito']}
                  rows={[
                    ['Schema: rooms, room_exits, zones, zone_resets'],
                    ['Schema: mob_prototypes, obj_prototypes, obj_affects, shops'],
                    ['Schema: achievement_defs, character_achievements'],
                    ['Script importazione myst.wld/mob/obj/zon/shp → DB'],
                    ['world.hpp ODB (già placeholder vuoto in src/odb/)'],
                  ]}
                />
              </Stack>
            </CardBody>
          </Card>

          <Card>
            <CardHeader trailing={badge(devB)}>Dev B — C++17</CardHeader>
            <CardBody>
              <Stack gap={10}>
                <Text size="small" tone="secondary">
                  File esclusivi: `structs.hpp`, `utils.hpp`, `utilities/*`, `constants.cpp`, `act.*.cpp`, `handler.cpp`
                </Text>
                <Divider />
                <Text size="small" weight="semibold">Strutture e memoria</Text>
                <Table
                  headers={['Compito']}
                  rows={[
                    ['C++17 toolchain completato: GCC 12 + ODB 2.5 su Jammy, build Docker/Vagrant allineate e warning cleanup in corso'],
                    ['char* → std::string in structs.hpp (name, title, descr...)'],
                    ['Macro CREATE/REALLOC → new + std::unique_ptr dove possibile'],
                    ['character_list/object_list → std::list<char_data>'],
                    ['room_db/mob_index/obj_index → std::unordered_map<int, *>'],
                  ]}
                />
                <Divider />
                <Text size="small" weight="semibold">I/O, utilities, accessor</Text>
                <Table
                  headers={['Compito']}
                  rows={[
                    ['Convertire utilities/*.c in .cpp'],
                    ['sprintf/FILE* → std::ostringstream/fstream'],
                    ['Macro GET_NAME, IS_NPC ecc. → inline methods su char_data'],
                    ['constants.cpp: AchievementsList → std::array constexpr'],
                    ['Rimuovere globali nude → wrapper con accessor statici'],
                  ]}
                />
              </Stack>
            </CardBody>
          </Card>

        </Grid>
      </Stack>

      <Divider />

      <Stack gap={12}>
        <H2>Checklist operativa corrente</H2>
        <Text tone="secondary" size="small">
          Stessa todo usata in sessione agent (id stabili); aggiorna gli stati quando completi.
        </Text>
        <Grid columns={2} gap={16}>
          <Card>
            <CardHeader trailing={badge(devA)}>Dev A — operativo</CardHeader>
            <CardBody>
              <Table
                headers={['Id', 'Compito']}
                rows={[
                  ['a1-schema', 'Estendere schema ODB in src/odb/account.hpp (toonData, toonSkills, toonAffects)'],
                  ['a1-loadsave', 'Riscrivere load_char/save_char/store_to_char/char_to_store in db.cpp per MySQL'],
                  ['a1-refund', 'Adattare do_refund in act.wizard.cpp per backup/restore da MySQL'],
                  ['a2-achievements', 'toonAchievement su MySQL, logica centralizzata; eliminare file .achie'],
                  ['a3-world-schema', 'Schema MySQL zone/room/mob/obj + script import CircleMUD'],
                  ['a3-world-loader', 'Riscrivere boot_world/boot_zones in db.cpp da MySQL'],
                  ['a4-web-editor', 'API REST + editor CRUD mondo con reload in-game'],
                ]}
              />
            </CardBody>
          </Card>
          <Card>
            <CardHeader trailing={badge(devB)}>Dev B — operativo</CardHeader>
            <CardBody>
              <Table
                headers={['Id', 'Compito']}
                rows={[
                  ['b1-cleanup', 'Rimuovere utilities/ (C legacy) e src/attic/ (vecchio SQL), aggiornare CMakeLists'],
                  ['b2-modernize', 'Sostituire pattern C-style con C++ moderno (coordinare con Dev A su structs/db)'],
                  ['b2-vt100-review', 'Rivedere VT100 (comm.cpp, act.info.cpp), compatibilità e fallback'],
                  ['b2-mini-infobar-compatible', 'Mini-infobar VT100 compatibile, comando esplicito, redraw sicuro'],
                  ['b3-world-cpp', 'Dopo schema mondo: zone_table/world → vector/unordered_map'],
                  ['b4-docker-prod-setup', 'Docker production-grade (compose, secrets, backup, healthcheck)'],
                  ['cpp17-toolchain', 'Completato: C++17 + ODB 2.5 + build Docker/Vagrant'],
                ]}
              />
            </CardBody>
          </Card>
        </Grid>
      </Stack>

      <Divider />

      {/* ── Sync point ── */}
      <Stack gap={12}>
        <H2>Sync point — quando si ri-uniscono i branch</H2>
        <Text tone="secondary">
          Le due linee di lavoro si incontrano in 3 momenti. Queste sessioni vanno fatte insieme.
        </Text>
        <Table
          headers={['Sync', 'Quando', 'Cosa si fa insieme']}
          rows={[
            ['S1 — Schema accordo', 'Prima di scrivere codice', 'Decidere i tipi delle colonne DB partendo dalle struct attuali. Dev A sa cosa mettere in DB, Dev B sa come cambieranno le struct.'],
            ['S2 — Adapter layer', 'Dopo ~4 settimane', 'Scrivere to_db(char_data) / from_db(CharacterRecord) che converte tra il modello di gioco (modernizzato) e il modello DB.'],
            ['S3 — Sostituzione file', 'Fine progetto', 'Attivare il flag USE_DB_LOAD, disattivare il caricamento da file, validare in staging.'],
          ]}
        />
      </Stack>

      <Divider />

      {/* ── Timeline parallela ── */}
      <Stack gap={12}>
        <H2>Timeline parallela</H2>
        <Table
          headers={['Settimane', 'Dev A (Database)', 'Dev B (C++17)', 'Sync']}
          rows={[
            ['1', 'S1: accordo sullo schema', 'S1: accordo sullo schema', 'S1 — insieme'],
            ['2–4', 'ODB character.hpp + schema personaggio', 'Modernizzazione C++17 (toolchain completata), char* → string, CMake', '—'],
            ['4–6', 'Script migrazione players.dat/rent/', 'Utilities, FILE*, macro → metodi', 'S2 — adapter layer'],
            ['6–9', 'Schema mondo + script importazione world files', 'unordered_map, globals, constants', '—'],
            ['9–11', 'Integrazione load/save DB nel codice server', 'Refactoring act.*.cpp, handler.cpp', 'S3 — attivazione DB'],
            ['11–12', 'Achievements DB + test finali', 'Cleanup e -Wpedantic zero warnings', '—'],
          ]}
        />
      </Stack>

      <Divider />

      {/* ── Pro/contro ── */}
      <Stack gap={12}>
        <H2>Pro e contro di questa strategia</H2>
        <Grid columns={2} gap={16}>
          <Stack gap={8}>
            <Text weight="semibold" tone="secondary">Vantaggi</Text>
            <Text size="small">Le fasi non sono più sequenziali — si lavora in parallelo dall'inizio e si finisce prima.</Text>
            <Text size="small">Ogni dev ha un dominio chiaro senza sovrapposizioni quotidiane.</Text>
            <Text size="small">Il DB può essere testato con le struct attuali (non serve aspettare il C++17).</Text>
            <Text size="small">La modernizzazione C++ non è bloccata da decisioni DB.</Text>
          </Stack>
          <Stack gap={8}>
            <Text weight="semibold" tone="secondary">Rischi</Text>
            <Text size="small">I 3 sync point richiedono disponibilità coordinata — se uno è bloccato, si blocca anche l'altro.</Text>
            <Text size="small">Se Dev B cambia un campo in structs.hpp (es. da char* a string), Dev A deve aggiornare il suo adapter.</Text>
            <Text size="small">Branch paralleli per settimane accumulano divergenze. Merge frequenti su main riducono il rischio.</Text>
          </Stack>
        </Grid>
      </Stack>

      <Divider />

      {/* ── Backup e refund ── */}
      <Stack gap={12}>
        <H2>Decisione architetturale: backup e comando refund</H2>
        <Text tone="secondary">
          I backup zip esistenti contengono file binari (`char_file_u`). Il comando `refund` estrae
          lo zip e copia i file in `lib/players/`. Questo meccanismo deve continuare a funzionare
          anche dopo la migrazione al DB, per qualsiasi backup passato o futuro.
        </Text>

        <Text size="small" tone="secondary">
          Ha senso scrivere sia su file che su DB? È ridondante, ma temporaneamente necessario.
          Le tre opzioni possibili:
        </Text>
        <Table
          headers={['Opzione', 'Come funziona', 'Pro', 'Contro']}
          rows={[
            ['A — Dual Write (scelta attuale)', 'save scrive su DB + file. Load legge solo DB. Backup zippa i file come oggi.', 'Zero modifiche a backup e refund. Funziona con zip di qualsiasi data.', 'Doppio write ad ogni save. File su disco che sembrano attivi ma non lo sono.'],
            ['B — Backup da mysqldump', 'Cron fa mysqldump invece di zippare file. refund ripristina da dump SQL.', 'Una sola sorgente di verità. Niente file .dat.', 'Riscrivere cron di backup e comando refund in act.wizard.cpp.'],
            ['C — Export JSON per pg', 'Cron esporta ogni pg in JSON → zip. refund legge JSON → INSERT nel DB.', 'Formato leggibile e estensibile.', 'Riscrivere backup e refund. Più lavoro con meno urgenza.'],
          ]}
          rowTone={['success', undefined, undefined]}
        />
        <Text size="small" tone="secondary">
          Raccomandazione: parti con A (Dual Write) perché non blocca la migrazione e il refund continua a funzionare
          su zip di qualsiasi data, anche quelli di 3 anni fa. Passa a B dopo, quando il DB è stabile e rodato —
          è la scelta giusta a lungo termine ma non vale il rischio ora.
        </Text>

        <Card>
          <CardHeader>Strategia scelta: Dual Write — come funziona in dettaglio</CardHeader>
          <CardBody>
            <Stack gap={10}>
              <Table
                headers={['Operazione', 'Comportamento']}
                rows={[
                  ['save_char()', 'Scrive su DB + scrive file binario in lib/players/ (shadow)'],
                  ['load_char()', 'Legge solo da DB — il file non viene mai letto in condizioni normali'],
                  ['backup cron', 'Zippa lib/players/ come oggi — invariato per sempre'],
                  ['refund', 'Estrae zip, copia file — invariato per sempre'],
                  ['login dopo refund', 'Auto-import: trova il file → importa nel DB → cancella il file'],
                ]}
              />
            </Stack>
          </CardBody>
        </Card>

        <Card>
          <CardHeader>legacy_loader.cpp — da mantenere per sempre</CardHeader>
          <CardBody>
            <Stack gap={8}>
              <Text>
                Il codice che legge i formati binari (`char_file_u`, `obj_file_u`, `.aux`) non va mai rimosso.
                Va spostato in un modulo dedicato chiaramente marcato come **sola lettura**.
              </Text>
              <Table
                headers={['File/Struct', 'Regola']}
                rows={[
                  ['char_file_u', 'MAI modificare — formato di lettura zip storici'],
                  ['obj_file_u / obj_file_elem', 'MAI modificare'],
                  ['old_obj_file_u', 'MAI modificare'],
                  ['legacy_loader.cpp (nuovo)', 'Solo lettura, zero write — usato da auto-import e refund'],
                ]}
                rowTone={['danger', 'danger', 'danger', 'warning']}
              />
            </Stack>
          </CardBody>
        </Card>
      </Stack>

      <Divider />

      {/* ── Agenda S1 aggiornata ── */}
      <Stack gap={12}>
        <H2>Agenda S1 aggiornata</H2>
        <Text tone="secondary">Punti da decidere insieme prima di scrivere una riga di codice DB.</Text>
        <Table
          headers={['Punto', 'Argomento']}
          rows={[
            ['S1a', 'Concordare i tipi delle colonne DB partendo da char_file_u (la mappa del tesoro)'],
            ['S1b', 'Definire i nomi delle tabelle: character_stats, character_skills, character_affects, character_equipment, character_inventory, character_achievements, character_quests'],
            ['S1c', 'Confermare strategia Dual Write: save_char_to_db() chiama anche save_char_to_file()'],
            ['S1d', 'Dev A: legacy_loader.cpp è il PRIMO deliverable — deve esistere prima che Dev B tocchi le struct'],
            ['S1e', 'Concordare il feature flag: USE_DB_LOAD / USE_FILE_LOAD'],
            ['S1f', 'Decidere cosa fare dei char* in char_special_data non presenti in char_file_u (poofin, prompt, email, ecc.) — salvati altrove o nuovi campi DB?'],
          ]}
        />
      </Stack>

      <Divider />

      <Text tone="secondary" size="small">
        Consiglio: merge su main almeno ogni settimana con la strategia "feature flag" —
        il nuovo codice DB vive dietro `#ifdef USE_DB` / `USE_FILE` finché non è stabile,
        così entrambi i branch compilano sempre senza rompere nulla.
      </Text>
    </Stack>
  );
}
