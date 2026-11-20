/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __ZONE_SLOTS_HPP
#define __ZONE_SLOTS_HPP
/***************************  System  include ************************************/
#include <cstdint>
#include <optional>
#include <vector>
/***************************  Local    include ************************************/
namespace Alarmud {

struct char_data;
struct reset_com;

/*
 * Posti dei reset di zona.
 *
 * Ogni mob caricato da una riga M/C riceve un numero di serie
 * (char_data::reset_serial, 0 = nessun posto). I numeri vivi stanno in una
 * mappa serie -> mob, svuotata in free_char: un mob distrutto libera il suo
 * posto, e dopo un reboot tutti i posti sono liberi.
 *
 * Righe (calcolate da setup_zone dopo renum_zone_table):
 * - riga M "capo": non sta nella catena if_flag di un'altra M. Ha spawn_max
 *   posti nella zona (gruppo group_id): arg2 se e' l'unica M di quel mob nella
 *   zona e arg2 > 1 (punto di spawn), altrimenti 1.
 * - righe della catena (if_flag dopo il capo): owner_ref = riga del mob a cui
 *   appartengono. Le M dipendenti appartengono al capo, le C all'ultima M, le
 *   altre righe (G/E/P/O/H/F/Z/D) all'ultimo mob. I posti delle M/C
 *   dipendenti (spawn_max: come sopra per le M, 1 per le C) sono PER OGNI
 *   istanza del proprietario: gruppi per (serie del proprietario, riga).
 * - una C fuori da ogni catena ha un posto suo (group_id), come una M capo.
 * Quando un proprietario muore i suoi dipendenti ancora vivi diventano
 * orfani: restano legati alla loro riga dipendente (zona, riga) in un
 * gruppo di orfani. Il prossimo proprietario di quella riga (nuovo o vivo
 * da rifornire) li adotta prima di caricarne di nuovi. Un orfano che segue
 * qualcuno (charmato, pet) esce dal gruppo; uno che muore ne esce in
 * free_char; zload (forget_zone) svuota gli orfani della zona.
 */
namespace zone_slots {

/** Dove sta un mob dei reset: zona (-1 = non piu' trovata, es. dopo zload),
 *  riga, mob vivi e posti del gruppo, serie del proprietario (0 = capo o
 *  orfano), orfano in attesa di un nuovo proprietario. */
struct SlotLocation {
	int zone;
	int cmd;
	int alive;
	int max;
	std::uint64_t master;
	bool orphan;
};

/** Riga proprietaria nella catena (-1 = nessuna: riga capo o fuori catena). */
[[nodiscard]] int owner_of(const reset_com& line);

/** Mob vivi della riga capo / C fuori catena (toglie i morti dal gruppo). */
[[nodiscard]] std::vector<char_data*> line_members(const reset_com& line);
/** Mob vivi della riga dipendente cmd per questa istanza del proprietario. */
[[nodiscard]] std::vector<char_data*> dependent_members(const char_data& owner, int cmd);
/** Lega il mob appena caricato alla riga capo / C fuori catena (zona, cmd). */
void add_line_member(int zone, int cmd, char_data& mob);
/** Lega il mob appena caricato alla riga dipendente (zona, cmd) del proprietario. */
void add_dependent(const char_data& owner, int zone, int cmd, char_data& mob);
/** Adotta fino a wanted orfani della riga dipendente (zona, cmd) per il
 *  proprietario: escono dagli orfani ed entrano nel suo gruppo. Gli orfani
 *  che seguono qualcuno vengono tolti e non adottati; quelli per cui
 *  can_join e' false (es. in combattimento) restano orfani per un altro
 *  reset. Restituisce gli adottati (il chiamante li sposta e li fa seguire). */
[[nodiscard]] std::vector<char_data*> adopt_orphans(
		const char_data& owner, int zone, int cmd, int wanted,
		bool (*can_join)(const char_data&));

/** Il mob viene distrutto: libera il suo posto; i suoi dipendenti vivi diventano orfani. */
void release(char_data& mob);
/** true se il mob con questo numero esiste ancora. */
[[nodiscard]] bool is_alive(std::uint64_t serial);

/** Dopo renum_zone_table: proprietari, posti e gruppi delle righe della zona. */
void setup_zone(int zone);
/** Prima di buttare la tabella dei comandi (zload): dimentica gruppi e orfani della zona. */
void forget_zone(int zone);

/** Riga di reset del mob con questo numero, se c'e' (per stat). */
[[nodiscard]] std::optional<SlotLocation> find(std::uint64_t serial);

} // namespace zone_slots
} // namespace Alarmud
#endif // __ZONE_SLOTS_HPP
