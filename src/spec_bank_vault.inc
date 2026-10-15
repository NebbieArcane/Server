/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* spec_bank_vault.inc - listino BASE delle cassette di sicurezza.
 *
 * Incluso solo da spec_bank.cpp. Al boot le righe mancanti della tabella
 * vault_plan vengono create da qui (INSERT IGNORE): da quel momento i valori
 * nel DB sono quelli validi e si possono modificare senza ricompilare.
 * Cambiare un valore qui NON aggiorna una riga gia' presente nel DB.
 *
 * Colonne: scope, tier, capienza (oggetti), prezzo acquisto unico,
 * affitto mensile, prezzo acquisto con canone, canone mensile (acquisto con
 * canone), rune degli Dei per acquisto unico e per acquisto con canone
 * (GET_RUNEDEI del PG),
 * tassa per oggetto depositato (solo personale). 0 = formula non disponibile.
 *
 * Tier: 1 Bronzo, 2 Argento, 3 Oro, 4 Platino, 5 Mithril.
 */

constexpr VaultPlanSeed kVaultPlanSeeds[] = {
	/* scope                 tier  cap   unico      affitto  acquisto  canone  r.un r.ac tassa */
	{VAULT_SCOPE_PERSONAL,  1,  100,    250000,   40000,   150000,  12000,  0,  0,   250},
	{VAULT_SCOPE_PERSONAL,  2,  250,    750000,  100000,   450000,  30000,  0,  0,   750},
	{VAULT_SCOPE_PERSONAL,  3,  500,   2000000,  250000,  1200000,  75000,  0,  0,  2000},
	{VAULT_SCOPE_PERSONAL,  4, 1000,   5000000,  600000,  3000000, 180000,  0,  0,  5000},
	{VAULT_SCOPE_PERSONAL,  5, 2500,  15000000,       0,  9000000, 400000, 25, 15,     0},

	{VAULT_SCOPE_ACCOUNT,   1,  100,    500000,   80000,   300000,  24000,  0,  0,     0},
	{VAULT_SCOPE_ACCOUNT,   2,  250,   1500000,  200000,   900000,  60000,  0,  0,     0},
	{VAULT_SCOPE_ACCOUNT,   3,  500,   4000000,  500000,  2400000, 150000,  0,  0,     0},
	{VAULT_SCOPE_ACCOUNT,   4, 1000,  10000000, 1200000,  6000000, 360000,  0,  0,     0},
	{VAULT_SCOPE_ACCOUNT,   5, 2500,  30000000,       0, 18000000, 800000, 50, 30,     0},

	{VAULT_SCOPE_CLAN,      1,  100,    500000,       0,        0,      0,  0,  0,     0},
	{VAULT_SCOPE_CLAN,      2,  250,   1500000,       0,        0,      0,  0,  0,     0},
	{VAULT_SCOPE_CLAN,      3,  500,   4000000,       0,        0,      0,  0,  0,     0},
	{VAULT_SCOPE_CLAN,      4, 1000,  10000000,       0,        0,      0,  0,  0,     0},
	{VAULT_SCOPE_CLAN,      5, 2500,  40000000,       0,        0,      0,  0,  0,     0},
};
