/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SERVICE_BALANCE_HPP
#define __SERVICE_BALANCE_HPP

namespace Alarmud {

/**
 * Prezzi servizi NPC: derent (RentEditor) e disincanta (AffectSlotRemover).
 * Persistenza: tabella service_balance (KV), CREATE al boot.
 *
 *   configure derent [set|reset|help]
 *   configure disincanta [set|reset|help]
 *
 * Derent: rune, exp, gold (default 1 / 2000000 / 300000)
 * Disincanta: rune, exp, coins (default 1 / 0 / 300000)
 *
 * In configure i valori accettano punti migliaia e suffissi k/m
 * (via parse_amount), es. 300k, 2m, 1.500.000.
 */
struct DerentBalanceConfig {
	int rune = 1;
	long exp = 2000000L;
	long gold = 300000L;
};

struct DisincantaBalanceConfig {
	int rune = 1;
	long exp = 0L;
	long coins = 300000L;
};

void service_balance_boot();
void service_balance_save();
void service_balance_reset_derent();
void service_balance_reset_disincanta();

[[nodiscard]] const DerentBalanceConfig& derent_balance_config();
[[nodiscard]] const DisincantaBalanceConfig& disincanta_balance_config();

void derent_balance_wiz(struct char_data* ch, const char* arg);
void disincanta_balance_wiz(struct char_data* ch, const char* arg);

} // namespace Alarmud

#endif
