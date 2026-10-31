/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __INVENTORY_OVERFLOW_HPP
#define __INVENTORY_OVERFLOW_HPP

#include "typedefs.hpp"
#include <vector>

namespace Alarmud {

/** Ore prima soft-delete OVERFLOW_EXPIRE (solo boot).
 *  Override: -DOVERFLOW_EXPIRE_HOURS=N. 0 = non soft-delete. Default: 168 (7 giorni). */
#ifndef OVERFLOW_EXPIRE_HOURS
#define OVERFLOW_EXPIRE_HOURS 168
#endif

/** Ogni N minuti arma un drain: al massimo 1 PG persistito per pulse. */
constexpr int OVERFLOW_CHECKPOINT_MINUTES = 5;

/**
 * Hold dopo begin_persist: excess staccati dal PG, scritti in overflow.
 * Il chiamante salva rent/body mentre ContaOggetti <= MAX_OBJ_SAVE, poi finish.
 */
struct InventoryOverflowHold {
	struct char_data* ch = nullptr;
	/** Root staccati (alberi). */
	std::vector<struct obj_data*> roots;
	/** Parent container al detach (nullptr = era in carrying). */
	std::vector<struct obj_data*> parents;
	bool active = false;
};

/**
 * Piano condiviso (stessa priorita' di begin_persist / do_save_rent):
 * eccesso da togliere per rientrare in MAX_OBJ_SAVE.
 * Ordine: carry non protetto → carry protetto → contenuto wear (mai il wearato).
 * Protetti: EDIT/QUEST/PERSONAL/INSERT/procarea.
 */
void inventory_plan_overflow(struct char_data* ch, int need,
							 std::vector<struct obj_data*>& excess_roots);

/**
 * Se > MAX_OBJ_SAVE: sceglie l'eccesso (inventory_plan_overflow), lo stacca, riscrive
 * character_inventory_overflow (set attivo sostituito). Niente extract permanente.
 * Se <= MAX_OBJ_SAVE: svuota overflow attivo (anti-stale).
 * Poi: salvare il PG, quindi inventory_overflow_finish_persist.
 * @return false se non migrato / non applicabile; true se hold valido (anche vuoto).
 */
bool inventory_overflow_begin_persist(struct char_data* ch, InventoryOverflowHold& hold);

/**
 * Dopo il save: reattach=true (LD/checkpoint, pezzi restano usabili) oppure
 * extract (forcerent / fine sessione mondo) — i pezzi restano solo in overflow DB.
 */
void inventory_overflow_finish_persist(InventoryOverflowHold& hold, bool reattach);

/** Pulse: ogni 5 min drain, max 1 PG/pulse; stesso PG al max 1 volta per round
 * (reattach lascia il conteggio >200, altrimenti thrash). */
void inventory_overflow_checkpoint_pulse(unsigned long now_pulse);

/** Soft-delete overflow attivi piu' vecchi di OVERFLOW_EXPIRE_HOURS (boot). */
void inventory_overflow_expire_stale();

/** Stub prezzo recupero. */
long inventory_overflow_claim_price(unsigned long long overflow_id);

/** Stub claim mob. */
bool inventory_overflow_claim(struct char_data* ch, unsigned long long overflow_id);

} // namespace Alarmud

#endif /* __INVENTORY_OVERFLOW_HPP */
