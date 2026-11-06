/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __INVENTORY_OVERFLOW_HPP
#define __INVENTORY_OVERFLOW_HPP

#include "typedefs.hpp"
#include <string>
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

/**
 * Riga listino claim: pezzi identici accorpati.
 * price = fee unitaria; item_count = quantita' nel gruppo; overflow_ids = id DB.
 */
struct InventoryOverflowListItem {
	std::vector<unsigned long long> overflow_ids;
	std::string short_desc;
	int item_count = 0;
	long price = 0;
	/** true se proto cost >= LIM_ITEM_COST_MIN. */
	bool is_limited = false;
};

/**
 * Listina piatta (ogni oggetto attivo), gruppi di pezzi identici.
 * owner_name: se non null e actor e' QUESTMASTER+, deposito di quel PG; altrimenti il proprio.
 * resolved_owner_out: se non null, nome canonico toon (casing DB) dopo resolve.
 */
bool inventory_overflow_list(struct char_data* actor,
							 std::vector<InventoryOverflowListItem>& out,
							 const char* owner_name = nullptr,
							 std::string* resolved_owner_out = nullptr);

/** PG con pezzi attivi in overflow (staff list all). */
struct InventoryOverflowOwnerCount {
	std::string name;
	int item_count = 0;
};

/**
 * Elenco PG con deposito overflow attivo, ordinato per pezzi desc.
 * Solo QUESTMASTER+. out vuoto + true = nessuno.
 */
bool inventory_overflow_list_owners(struct char_data* actor,
								   std::vector<InventoryOverflowOwnerCount>& out);

/** Fee unitaria per un id nel deposito risolto. 0 se assente. */
long inventory_overflow_claim_price(struct char_data* actor, unsigned long long overflow_id,
									const char* owner_name = nullptr);

/**
 * Riscatta uno o piu' pezzi (solo quegli id, senza sottoalbero).
 * I figli in deposito restano e diventano pezzi singoli (parent azzerato).
 * owner_name: come list; staff su altro PG → oggetti all'actor, senza fee.
 */
bool inventory_overflow_claim(struct char_data* actor,
							  const std::vector<unsigned long long>& overflow_ids,
							  const char* owner_name = nullptr);

} // namespace Alarmud

#endif /* __INVENTORY_OVERFLOW_HPP */
