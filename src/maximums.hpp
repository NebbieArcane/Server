/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __MAXIMUMS_HPP
#define __MAXIMUMS_HPP
/***************************  System  include ************************************/
#include <optional>
#include <vector>
/***************************  Local    include ************************************/
namespace Alarmud {

struct title_type {
	const char* title_m;
	const char* title_f;
	int exp;
};

void ClassSpecificStuff(struct char_data* ch) ;
char* ClassTitles(struct char_data* ch) ;
int GetExtimatedHp(struct char_data* ch) ;
int GetHpGain(struct char_data* ch, int iClass,int livello,int compat,int check=0) ;
void ObjFromCorpse(struct obj_data* c) ;
/** Store: livello nuovo registrato in character_level_hp. Skip: creazione in corso
 *  (toon non ancora su DB), lo storico lo scrive poi level_hp_db_seed_new. */
enum class LevelHpRecord { Store, Skip };
void advance_level(struct char_data* ch, int iClass);
void advance_level(struct char_data* ch, int iClass, LevelHpRecord record);
void check_idling(struct char_data* ch) ;
void drop_level(struct char_data* ch, int iClass, int goddrain) ;
/* ---- Storico HP per livello (character_level_hp) ---- */
/** Una riga della baseline: hp assegnati a (classe, livello). */
struct LevelHpCell {
	int class_index;
	int level;
	int hp;
};
/**
 * Baseline per chi non ha storico: max_hit meno gli HP iniziali medi (bonus razza compreso)
 * ripartito su ogni livello posseduto con peso = media attesa di GetHpGain, almeno 1
 * a livello, resto al resto maggiore (somma esatta). levels: MAX_CLASS valori.
 * Vuota per immortali o senza classi.
 */
[[nodiscard]] std::vector<LevelHpCell> level_hp_baseline(const int* levels, int con, int max_hit, int race);
/** Esito di level_hp_baseline_missing: celle nuove e se il residuo non bastava (1 a livello). */
struct LevelHpFill {
	std::vector<LevelHpCell> cells;
	bool clamped = false; /* storico incoerente: residuo minore dei livelli mancanti */
};
/**
 * Come level_hp_baseline ma solo per i livelli posseduti senza riga in existing:
 * residuo = max_hit - HP iniziali stimati - somma hp delle righe esistenti possedute,
 * stessi pesi e resto maggiore. Le righe esistenti non si toccano. Se il residuo e' minore
 * dei livelli mancanti: clamped (celle a 1, da non usare: il boot ricostruisce lo storico posseduto).
 */
[[nodiscard]] LevelHpFill level_hp_baseline_missing(const int* levels, int con, int max_hit, int race,
													const std::vector<LevelHpCell>& existing);
/** Storico atteso ("media attesa"): HP iniziali stimati + guadagno medio di ogni livello posseduto. */
struct LevelHpExpected {
	int start = 0;  /* HP iniziali stimati */
	int target = 0; /* start + somma delle celle, max 32767; 0 = immortale o senza classi */
	std::vector<LevelHpCell> cells; /* livelli posseduti 1..min(livello, BARONE), hp = guadagno medio */
};
[[nodiscard]] LevelHpExpected level_hp_expected(const int* levels, int con, int race);
/** HP extra dati dalla razza alla creazione (roll_abilities), prima della divisione per classi. */
[[nodiscard]] int creation_race_hp_bonus(int race);
/* Lo storico vive solo nel DB (db.cpp, level_hp_db_*): niente copia in memoria. */
/** HP registrati per quel livello (la riga resta anche dopo la perdita); nullopt = riga assente o errore DB. */
[[nodiscard]] std::optional<int> hp_for_level_loss(struct char_data* ch, int class_index, int level);
/** true se la pergamena della memoria non cambierebbe nulla (nessuna riga con hp != base_hp, o errore DB). */
[[nodiscard]] bool level_hp_matches_base(struct char_data* ch);
/** Pozione del destino: ritira ogni riga (anche persa), scrive solo quelle cambiate.
 *  Variazione di max_hit; nullopt = nessuno storico o errore DB (nulla cambiato). */
[[nodiscard]] std::optional<int> level_hp_reroll(struct char_data* ch);
/** Pergamena della memoria: hp = base_hp dove differiscono. Variazione di max_hit;
 *  nullopt = nulla da cambiare o errore DB. */
[[nodiscard]] std::optional<int> level_hp_restore_base(struct char_data* ch);
void gain_condition(struct char_data* ch,int condition,int value) ;
int gain_corretto(struct char_data* ch,int gain) ;
void gain_exp(struct char_data* ch, int gain) ;
/** Messaggio al pg quando gli xp sono al limite (aneddoto + "E' ora di editare."). */
void send_exp_limit_message(struct char_data* ch);
/** Barone con GET_EXP > PRINCEEXP -> Principe (anche sul pg se poly), messaggio e log una volta. */
void check_prince_promotion(struct char_data* ch);
/** Manda i messaggi di promozione rimandati (al pg e annuncio a tutti): a fine pulse. */
void flush_prince_promotions();
/**
 * Tetto di gilda: per ogni classe sotto il massimo razziale gli xp non possono
 * superare titles[i][liv+2].exp - 1; vale il piu' basso. INT_MAX se nessuno.
 */
[[nodiscard]] long long exp_guild_cap(const struct char_data* ch);
/**
 * Aggiunge amount xp senza passare da gain_exp, tagliando al tetto di gilda e
 * al massimo di un int (in quel caso WARNING + messaggio del limite).
 * Ritorna gli xp aggiunti davvero (0 = niente).
 */
[[nodiscard]] int add_exp_checked(struct char_data* ch, int amount);
void gain_exp_regardless(struct char_data* ch, int gain, int iClass, int iMaxLevel);
void gain_exp_rev(struct char_data* ch, int gain) ;
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6) ;
int hit_gain(struct char_data* ch) ;// Gaia 2001
int hit_limit(struct char_data* ch) ;
int mana_gain(struct char_data* ch) ;
int mana_limit(struct char_data* ch) ;
int move_gain(struct char_data* ch) ;
int move_limit(struct char_data* ch) ;
void set_title(struct char_data* ch) ;
} // namespace Alarmud
#endif // __MAXIMUMS_HPP

