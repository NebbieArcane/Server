/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* character_item_loss.hpp - audit perdite oggetti dal PG (MySQL). */
#ifndef SRC_CHARACTER_ITEM_LOSS_HPP_
#define SRC_CHARACTER_ITEM_LOSS_HPP_

#include "config.hpp"
#include "flags.hpp"

#include <string>
#include <string_view>
#include <vector>

/** Ore di retention in DB. Override: -DITEM_LOSS_RETENTION_HOURS=N.
 *  0 = non purgare (tieni tutto). Default: 168 (7 giorni). */
#ifndef ITEM_LOSS_RETENTION_HOURS
#define ITEM_LOSS_RETENTION_HOURS 168
#endif

namespace Alarmud {

struct obj_data;
struct char_data;

/** Valori `character_item_loss.kind` (varchar aperta). */
inline constexpr std::string_view kItemLossDrop = "DROP";
inline constexpr std::string_view kItemLossDropAll = "DROP_ALL";
inline constexpr std::string_view kItemLossGive = "GIVE";
inline constexpr std::string_view kItemLossSteal = "STEAL";
inline constexpr std::string_view kItemLossJunk = "JUNK";
inline constexpr std::string_view kItemLossDestroy = "DESTROY";
inline constexpr std::string_view kItemLossCombatBreak = "COMBAT_BREAK";
inline constexpr std::string_view kItemLossShopSell = "SHOP_SELL";
inline constexpr std::string_view kItemLossAuction = "AUCTION";
inline constexpr std::string_view kItemLossOther = "OTHER";

/** Default giorni per `show loss` (ceil ore/24; se retention off → 7). */
inline constexpr int kItemLossShowDefaultDays =
	ITEM_LOSS_RETENTION_HOURS > 0
		? ((ITEM_LOSS_RETENTION_HOURS + 23) / 24)
		: 7;

/** Max giorni accettati da `show loss`. */
inline constexpr int kItemLossShowMaxDays =
	ITEM_LOSS_RETENTION_HOURS > 0
		? ((ITEM_LOSS_RETENTION_HOURS + 23) / 24)
		: 365;

[[nodiscard]] std::string item_loss_pc_name(const char_data* ch);

#if USE_MYSQL

/** Una perdita. loser = PG che perde. Ignora NPC / immortali / MySQL off. */
void character_item_loss_log(char_data* loser, obj_data* obj, std::string_view kind,
							 std::string_view detail = {});

/** Batch in una sola transazione (es. drop all). */
void character_item_loss_log_list(char_data* loser, const std::vector<obj_data*>& objs,
								  std::string_view kind, std::string_view detail = {});

/** Wiz: show loss <nome> [giorni]. Default = kItemLossShowDefaultDays. */
void character_item_loss_show(char_data* ch, std::string_view name,
							  int days = kItemLossShowDefaultDays);

/** Boot: cancella righe piu' vecchie di ITEM_LOSS_RETENTION_HOURS (no-op se 0). */
void character_item_loss_purge_old();

#else

inline void character_item_loss_log(char_data*, obj_data*, std::string_view,
									std::string_view = {}) {}
inline void character_item_loss_log_list(char_data*, const std::vector<obj_data*>&,
										 std::string_view, std::string_view = {}) {}
inline void character_item_loss_show(char_data*, std::string_view,
									 int = kItemLossShowDefaultDays) {}
inline void character_item_loss_purge_old() {}

#endif

} // namespace Alarmud

#endif /* SRC_CHARACTER_ITEM_LOSS_HPP_ */
