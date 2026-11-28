/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "obj_value.hpp"
#include "edit_listino.hpp"
#include "spells.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "autoenums.hpp"
#include "constants.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "utility.hpp"
#include "object_instance.hpp"
#include "multiclass.hpp"
#include "config.hpp"
#include "flags.hpp"
#include "procarea.hpp"
#if USE_MYSQL
#include "Sql.hpp"
#include "odb/account-odb.hxx"
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>
#endif

namespace Alarmud {

/* Seed del listino unico: unica sorgente dei default (e fallback se il DB
 * e' vuoto o manca una riga). Prezzi BASE, senza moltiplicatore artifact.
 * Per i campi pool la pagina pubblica e' gia' con artifact: qui va il base. */
const EditListinoDefault kEditListinoDefaults[] = {
	{"none", "Nessuno", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_NONE, 0UL, 0, "", 1, 0},
	{"sex", "Sesso", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SEX, 0UL, 0, "", 1, 0},
	{"level", "Livello", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_LEVEL, 0UL, 0, "", 1, 0},
	{"age", "Eta", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AGE, 0UL, 0, "", 1, 0},
	{"char_weight", "Peso", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_CHAR_WEIGHT, 0UL, 0, "", 1, 0},
	{"char_height", "Altezza", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_CHAR_HEIGHT, 0UL, 0, "", 1, 0},
	{"gold", "Oro", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_GOLD, 0UL, 0, "", 1, 0},
	{"exp", "Esperienza", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_EXP, 0UL, 0, "", 1, 0},
	{"saving_para", "Saving para", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SAVING_PARA, 0UL, 0, "", 1, 0},
	{"saving_rod", "Saving rod", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SAVING_ROD, 0UL, 0, "", 1, 0},
	{"saving_petri", "Saving petri", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SAVING_PETRI, 0UL, 0, "", 1, 0},
	{"saving_breath", "Saving breath", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SAVING_BREATH, 0UL, 0, "", 1, 0},
	{"saving_spell", "Saving spell", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SAVING_SPELL, 0UL, 0, "", 1, 0},
	{"save_all", "Saving tutti", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SAVE_ALL, 0UL, 0, "", 1, 0},
	{"eat_spell", "Eat spell", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_EAT_SPELL, 0UL, 0, "", 1, 0},
	{"backstab", "Backstab", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_BACKSTAB, 0UL, 0, "", 1, 0},
	{"kick", "Kick", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_KICK, 0UL, 0, "", 1, 0},
	{"sneak", "Sneak", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SNEAK, 0UL, 0, "", 1, 0},
	{"hide", "Hide", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_HIDE, 0UL, 0, "", 1, 0},
	{"bash", "Bash", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_BASH, 0UL, 0, "", 1, 0},
	{"pick", "Pick", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_PICK, 0UL, 0, "", 1, 0},
	{"steal", "Steal", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_STEAL, 0UL, 0, "", 1, 0},
	{"track", "Track", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_TRACK, 0UL, 0, "", 1, 0},
	{"haste", "Haste", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_HASTE, 0UL, 0, "", 1, 0},
	{"slow", "Slow", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SLOW, 0UL, 0, "", 1, 0},
	{"attacks", "Attacks", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_ATTACKS, 0UL, 0, "", 1, 0},
	{"find_traps", "Find traps", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_FIND_TRAPS, 0UL, 0, "", 1, 0},
	{"ride", "Ride", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_RIDE, 0UL, 0, "", 1, 0},
	{"race_slayer", "Race slayer", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_RACE_SLAYER, 0UL, 0, "", 1, 0},
	{"align_slayer", "Align slayer", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_ALIGN_SLAYER, 0UL, 0, "", 1, 0},
	{"mod_thirst", "Sete", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_MOD_THIRST, 0UL, 0, "", 1, 0},
	{"mod_hunger", "Fame", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_MOD_HUNGER, 0UL, 0, "", 1, 0},
	{"mod_drunk", "Ubriachezza", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_MOD_DRUNK, 0UL, 0, "", 1, 0},
	{"t_str", "Temp STR", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_STR, 0UL, 0, "", 1, 0},
	{"t_int", "Temp INT", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_INT, 0UL, 0, "", 1, 0},
	{"t_dex", "Temp DEX", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_DEX, 0UL, 0, "", 1, 0},
	{"t_wis", "Temp WIS", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_WIS, 0UL, 0, "", 1, 0},
	{"t_con", "Temp CON", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_CON, 0UL, 0, "", 1, 0},
	{"t_chr", "Temp CHR", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_CHR, 0UL, 0, "", 1, 0},
	{"t_hps", "Temp HP", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_HPS, 0UL, 0, "", 1, 0},
	{"t_move", "Temp Move", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_MOVE, 0UL, 0, "", 1, 0},
	{"t_mana", "Temp Mana", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_T_MANA, 0UL, 0, "", 1, 0},
	{"skip", "Skip", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SKIP, 0UL, 0, "", 1, 0},
	{"str", "Forza", "stat", 15000000L, 15, 1, 1, 3, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_STR, 0UL, 0, "", 1, 1},
	{"dex", "Destrezza", "stat", 15000000L, 15, 1, 1, 3, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_DEX, 0UL, 0, "", 1, 1},
	{"intel", "Intelligenza", "stat", 15000000L, 15, 1, 1, 3, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_INT, 0UL, 0, "", 1, 1},
	{"wis", "Saggezza", "stat", 15000000L, 15, 1, 1, 3, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WIS, 0UL, 0, "", 1, 1},
	{"con", "Costituzione", "unused", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_CON, 0UL, 0, "", 1, 0},
	{"chr", "Carisma", "stat", 15000000L, 10, 1, 1, 3, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_CHR, 0UL, 0, "", 1, 1},
	{"mana", "Mana", "pool", 1500000L, 22, 15, 1, 0, 1, 0, 1, 150, "", 0, 0, 0, 0L, 0L, 0, APPLY_MANA, 0UL, 0, "", 10, 1},
	{"hit", "Hit", "pool", 3000000L, 3, 1, 1, 0, 1, 0, 1, 100, "", 0, 0, 0, 0L, 0L, 0, APPLY_HIT, 0UL, 0, "", 10, 1},
	{"move", "Move", "pool", 1000000L, 1, 1, 1, 0, 1, 0, 1, 100, "", 0, 0, 0, 0L, 0L, 0, APPLY_MOVE, 0UL, 0, "", 10, 1},
	{"mana_regen", "Mana regen", "pool", 3000000L, 44, 15, 1, 0, 1, 0, 1, 50, "", 0, 0, 0, 0L, 0L, 0, APPLY_MANA_REGEN, 0UL, 0, "", 5, 1},
	{"hit_regen", "Hit regen", "pool", 3000000L, 44, 15, 1, 0, 1, 0, 1, 50, "", 0, 0, 0, 0L, 0L, 0, APPLY_HIT_REGEN, 0UL, 0, "", 5, 1},
	{"move_regen", "Move regen", "pool", 2000000L, 4, 1, 1, 0, 1, 0, 1, 50, "", 0, 0, 0, 0L, 0L, 0, APPLY_MOVE_REGEN, 0UL, 0, "", 5, 1},
	{"spellfail", "Spellfail", "pool", 20000000L, 24, 1, -1, 0, 1, 0, 1, 100, "", 50, 5, 120, 100000000L, 500000L, 50, APPLY_SPELLFAIL, 0UL, 0, "", 1, 1},
	{"ac", "Classe armatura", "combat", 10000000L, 15, 1, -1, 40, 10, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AC, 0UL, 0, "", 1, 1},
	{"hitroll", "Hitroll", "combat", 45000000L, 50, 1, 1, 2, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_HITROLL, 0UL, 0, "", 1, 1},
	{"damroll", "Damroll", "combat", 100000000L, 120, 1, 1, 2, 1, 1, 0, 30, "dam_sp", 0, 0, 0, 0L, 0L, 0, APPLY_DAMROLL, 0UL, 0, "", 1, 1},
	{"spellpower", "Spellpower", "combat", 100000000L, 120, 1, 1, 2, 1, 1, 0, 30, "dam_sp", 0, 0, 0, 0L, 0L, 0, APPLY_SPELLPOWER, 0UL, 0, "", 1, 1},
	{"hitndam", "Hit-n-Dam", "combat", 145000000L, 170, 1, 1, 2, 1, 1, 0, 30, "dam_sp", 0, 0, 0, 0L, 0L, 0, APPLY_HITNDAM, 0UL, 0, "", 1, 1},
	{"hitnsp", "Hit-n-SP", "combat", 145000000L, 170, 1, 1, 2, 1, 1, 0, 30, "dam_sp", 0, 0, 0, 0L, 0L, 0, APPLY_HITNSP, 0UL, 0, "", 1, 1},
	{"immune_acid", "Resi acido", "resist_bit", 75000000L, 90, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_ACID, 50, "resist_acid", 1, 1},
	{"immune_elec", "Resi elettricita", "resist_bit", 150000000L, 180, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_ELEC, 50, "resist_elec", 1, 1},
	{"immune_fire", "Resi fuoco", "resist_bit", 100000000L, 120, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_FIRE, 50, "resist_fire", 1, 1},
	{"immune_cold", "Resi freddo", "resist_bit", 75000000L, 90, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_COLD, 50, "resist_cold", 1, 1},
	{"immune_energy", "Resi energia", "resist_bit", 150000000L, 180, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_ENERGY, 50, "resist_energy", 1, 1},
	{"immune_drain", "Resi drain", "resist_bit", 30000000L, 35, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_DRAIN, 50, "resist_drain", 1, 1},
	{"immune_hold", "Resi hold", "resist_bit", 75000000L, 90, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_HOLD, 50, "resist_hold", 1, 1},
	{"immune_poison", "Resi veleno", "resist_bit", 30000000L, 35, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_POISON, 50, "resist_poison", 1, 1},
	{"immune_slash", "Resi slash", "resist_bit", 150000000L, 200, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_SLASH, 50, "resist_slash", 1, 1},
	{"immune_pierce", "Resi pierce", "resist_bit", 150000000L, 200, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_PIERCE, 50, "resist_pierce", 1, 1},
	{"immune_blunt", "Resi blunt", "resist_bit", 300000000L, 400, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_BLUNT, 50, "resist_blunt", 1, 1},
	{"immune_sleep", "Resi sleep", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_SLEEP, 50, "resist_sleep", 1, 0},
	{"immune_charm", "Resi charm", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_CHARM, 50, "resist_charm", 1, 0},
	{"immune_nonmag", "Resi nonmag", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_NONMAG, 50, "resist_nonmag", 1, 0},
	{"immune_plus1", "Resi plus1", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_PLUS1, 50, "resist_plus1", 1, 0},
	{"immune_plus2", "Resi plus2", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_PLUS2, 50, "resist_plus2", 1, 0},
	{"immune_plus3", "Resi plus3", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_PLUS3, 50, "resist_plus3", 1, 0},
	{"immune_plus4", "Resi plus4", "resist_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_IMMUNE, IMM_PLUS4, 50, "resist_plus4", 1, 0},
	{"mimmune_drain", "Immunita drain", "immune_bit", 100000000L, 100, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_DRAIN, 100, "resist_drain", 1, 1},
	{"mimmune_charm", "Immunita charm", "immune_bit", 60000000L, 60, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_CHARM, 100, "resist_charm", 1, 1},
	{"mimmune_poison", "Immunita veleno", "immune_bit", 100000000L, 100, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_POISON, 100, "resist_poison", 1, 1},
	{"mimmune_fire", "Immunita fire", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_FIRE, 100, "resist_fire", 1, 0},
	{"mimmune_cold", "Immunita cold", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_COLD, 100, "resist_cold", 1, 0},
	{"mimmune_elec", "Immunita elec", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_ELEC, 100, "resist_elec", 1, 0},
	{"mimmune_energy", "Immunita energy", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_ENERGY, 100, "resist_energy", 1, 0},
	{"mimmune_blunt", "Immunita blunt", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_BLUNT, 100, "resist_blunt", 1, 0},
	{"mimmune_pierce", "Immunita pierce", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_PIERCE, 100, "resist_pierce", 1, 0},
	{"mimmune_slash", "Immunita slash", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_SLASH, 100, "resist_slash", 1, 0},
	{"mimmune_acid", "Immunita acid", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_ACID, 100, "resist_acid", 1, 0},
	{"mimmune_sleep", "Immunita sleep", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_SLEEP, 100, "resist_sleep", 1, 0},
	{"mimmune_hold", "Immunita hold", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_HOLD, 100, "resist_hold", 1, 0},
	{"mimmune_nonmag", "Immunita nonmag", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_NONMAG, 100, "resist_nonmag", 1, 0},
	{"mimmune_plus1", "Immunita plus1", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_PLUS1, 100, "resist_plus1", 1, 0},
	{"mimmune_plus2", "Immunita plus2", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_PLUS2, 100, "resist_plus2", 1, 0},
	{"mimmune_plus3", "Immunita plus3", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_PLUS3, 100, "resist_plus3", 1, 0},
	{"mimmune_plus4", "Immunita plus4", "immune_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_M_IMMUNE, IMM_PLUS4, 100, "resist_plus4", 1, 0},
	{"susc_fire", "Suscettibilita fire", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_FIRE, -50, "resist_fire", 1, 0},
	{"susc_cold", "Suscettibilita cold", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_COLD, -50, "resist_cold", 1, 0},
	{"susc_elec", "Suscettibilita elec", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_ELEC, -50, "resist_elec", 1, 0},
	{"susc_energy", "Suscettibilita energy", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_ENERGY, -50, "resist_energy", 1, 0},
	{"susc_blunt", "Suscettibilita blunt", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_BLUNT, -50, "resist_blunt", 1, 0},
	{"susc_pierce", "Suscettibilita pierce", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_PIERCE, -50, "resist_pierce", 1, 0},
	{"susc_slash", "Suscettibilita slash", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_SLASH, -50, "resist_slash", 1, 0},
	{"susc_acid", "Suscettibilita acid", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_ACID, -50, "resist_acid", 1, 0},
	{"susc_poison", "Suscettibilita poison", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_POISON, -50, "resist_poison", 1, 0},
	{"susc_drain", "Suscettibilita drain", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_DRAIN, -50, "resist_drain", 1, 0},
	{"susc_sleep", "Suscettibilita sleep", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_SLEEP, -50, "resist_sleep", 1, 0},
	{"susc_charm", "Suscettibilita charm", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_CHARM, -50, "resist_charm", 1, 0},
	{"susc_hold", "Suscettibilita hold", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_HOLD, -50, "resist_hold", 1, 0},
	{"susc_nonmag", "Suscettibilita nonmag", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_NONMAG, -50, "resist_nonmag", 1, 0},
	{"susc_plus1", "Suscettibilita plus1", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_PLUS1, -50, "resist_plus1", 1, 0},
	{"susc_plus2", "Suscettibilita plus2", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_PLUS2, -50, "resist_plus2", 1, 0},
	{"susc_plus3", "Suscettibilita plus3", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_PLUS3, -50, "resist_plus3", 1, 0},
	{"susc_plus4", "Suscettibilita plus4", "susc_bit", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SUSC, IMM_PLUS4, -50, "resist_plus4", 1, 0},
	{"aff_invisible", "Invisibilita", "affect", 30000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_INVISIBLE, 0, "", 1, 1},
	{"aff_telepathy", "Telepathy", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_TELEPATHY, 0, "", 1, 1},
	{"aff_waterbreath", "Waterbreath", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_WATERBREATH, 0, "", 1, 1},
	{"aff_true_sight", "True sight", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_TRUE_SIGHT, 0, "", 1, 1},
	{"aff_scrying", "Spy / Scrying", "affect", 150000000L, 180, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_SCRYING, 0, "", 1, 1},
	{"aff_protect_from_evil", "Protection from evil", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_PROTECT_FROM_EVIL, 0, "", 1, 1},
	{"aff_sense_life", "Sense life", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_SENSE_LIFE, 0, "", 1, 1},
	{"aff_flying", "Fly", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_FLYING, 0, "", 1, 1},
	{"aff_globe_darkness", "Darkness", "affect", 50000000L, 70, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_GLOBE_DARKNESS, 0, "", 1, 1},
	{"aff_blind", "Blind", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_BLIND, 0, "", 1, 0},
	{"aff_detect_evil", "Detect Evil", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_DETECT_EVIL, 0, "", 1, 0},
	{"aff_detect_invisible", "Detect Invisible", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_DETECT_INVISIBLE, 0, "", 1, 0},
	{"aff_detect_magic", "Detect Magic", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_DETECT_MAGIC, 0, "", 1, 0},
	{"aff_life_prot", "Life Prot", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_LIFE_PROT, 0, "", 1, 0},
	{"aff_sanctuary", "Sanctuary", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_SANCTUARY, 0, "", 1, 0},
	{"aff_dragon_ride", "Dragon Ride", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_DRAGON_RIDE, 0, "", 1, 0},
	{"aff_growth", "Growth", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_GROWTH, 0, "", 1, 0},
	{"aff_curse", "Curse", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_CURSE, 0, "", 1, 0},
	{"aff_poison", "Poison", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_POISON, 0, "", 1, 0},
	{"aff_tree_travel", "Tree Travel", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_TREE_TRAVEL, 0, "", 1, 0},
	{"aff_paralysis", "Paralysis", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_PARALYSIS, 0, "", 1, 0},
	{"aff_infravision", "Infravision", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_INFRAVISION, 0, "", 1, 0},
	{"aff_sleep", "Sleep", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_SLEEP, 0, "", 1, 0},
	{"aff_travelling", "Travelling", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_TRAVELLING, 0, "", 1, 0},
	{"aff_sneak", "Sneak", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_SNEAK, 0, "", 1, 0},
	{"aff_hide", "Hide", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_HIDE, 0, "", 1, 0},
	{"aff_silence", "Silence", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_SILENCE, 0, "", 1, 0},
	{"aff_charm", "Charm", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_CHARM, 0, "", 1, 0},
	{"aff_follow", "Follow", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_FOLLOW, 0, "", 1, 0},
	{"aff_fireshield", "Fireshield", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_FIRESHIELD, 0, "", 1, 0},
	{"aff_group", "Group", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_GROUP, 0, "", 1, 0},
	{"aff_undef_aff_1", "Undef Aff 1", "affect", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_SPELL, AFF_UNDEF_AFF_1, 0, "", 1, 0},
	{"aff2_danger_sense", "Psionic danger sense", "aff2", 150000000L, 180, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_DANGER_SENSE, 0, "", 1, 1},
	{"aff2_animal_invis", "Animal Invis", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_ANIMAL_INVIS, 0, "", 1, 0},
	{"aff2_heat_stuff", "Heat Stuff", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_HEAT_STUFF, 0, "", 1, 0},
	{"aff2_log_me", "Log Me", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_LOG_ME, 0, "", 1, 0},
	{"aff2_berserk", "Berserk", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_BERSERK, 0, "", 1, 0},
	{"aff2_parry", "Parry", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_PARRY, 0, "", 1, 0},
	{"aff2_con_order", "Con Order", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_CON_ORDER, 0, "", 1, 0},
	{"aff2_afk", "Afk", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_AFK, 0, "", 1, 0},
	{"aff2_pkiller", "Pkiller", "aff2", 0L, 0, 1, 1, 0, 1, 0, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_AFF2, AFF2_PKILLER, 0, "", 1, 0},
	{"resist_acid", "Resistenza acido (num)", "resist_num", 1500000L, 9, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_elec", "Resistenza elettricita (num)", "resist_num", 3000000L, 18, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_fire", "Resistenza fuoco (num)", "resist_num", 2000000L, 12, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_cold", "Resistenza freddo (num)", "resist_num", 1500000L, 9, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_energy", "Resistenza energia (num)", "resist_num", 3000000L, 18, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_drain", "Resistenza drain (num)", "resist_num", 600000L, 7, 10, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_hold", "Resistenza hold (num)", "resist_num", 1500000L, 9, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_poison", "Resistenza veleno (num)", "resist_num", 600000L, 7, 10, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_slash", "Resistenza slash (num)", "resist_num", 3000000L, 4, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_pierce", "Resistenza pierce (num)", "resist_num", 3000000L, 4, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_blunt", "Resistenza blunt (num)", "resist_num", 6000000L, 8, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_charm", "Resistenza charm (num)", "resist_num", 600000L, 3, 5, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_sleep", "Resistenza sleep (num)", "resist_num", 0L, 0, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_nonmag", "Resistenza nonmag (num)", "resist_num", 0L, 0, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_plus1", "Resistenza plus1 (num)", "resist_num", 0L, 0, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_plus2", "Resistenza plus2 (num)", "resist_num", 0L, 0, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_plus3", "Resistenza plus3 (num)", "resist_num", 0L, 0, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"resist_plus4", "Resistenza plus4 (num)", "resist_num", 0L, 0, 1, 1, 100, 1, 0, 0, 100, "", 0, 0, 0, 0L, 0L, 0, 0, 0UL, 0, "", 1, 0},
	{"artifact", "Artifact", "extra", 100000000L, 100, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, 0, ITEM_IMMUNE, 0, "", 1, 1},
	{"wspell_cause_light", "Cause light", "weapon_spell", 40000000L, 80, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_CAUSE_LIGHT, 0, "", 1, 1},
	{"wspell_cause_serious", "Cause serious", "weapon_spell", 100000000L, 200, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_CAUSE_SERIOUS, 0, "", 1, 1},
	{"wspell_cause_critical", "Cause critical", "weapon_spell", 150000000L, 300, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_CAUSE_CRITICAL, 0, "", 1, 1},
	{"wspell_chill_touch", "Chill touch", "weapon_spell", 100000000L, 200, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_CHILL_TOUCH, 0, "", 1, 1},
	{"wspell_faerie_fire", "Faerie fire", "weapon_spell", 20000000L, 40, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_FAERIE_FIRE, 0, "", 1, 1},
	{"wspell_curse", "Curse", "weapon_spell", 20000000L, 40, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_CURSE, 0, "", 1, 1},
	{"wspell_blindness", "Blindness", "weapon_spell", 150000000L, 300, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_BLINDNESS, 0, "", 1, 1},
	{"wspell_sunray", "Sunray", "weapon_spell", 100000000L, 200, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_SUNRAY, 0, "", 1, 1},
	{"wspell_warp_weapon", "Warp weapon", "weapon_spell", 20000000L, 40, 1, 1, 1, 1, 1, 0, 0, "", 0, 0, 0, 0L, 0L, 0, APPLY_WEAPON_SPELL, SPELL_WARP_WEAPON, 0, "", 1, 1}

};
const std::size_t kEditListinoDefaultsCount =
	sizeof(kEditListinoDefaults) / sizeof(kEditListinoDefaults[0]);

const EditListinoRuleDefault kEditListinoRulesDefaults[] = {
	{"artifact_mult", 3, 2},
	{"class_mult_1", 1, 1},
	{"class_mult_2", 3, 2},
	{"class_mult_3", 2, 1},
	{"malus_mult", 2, 1},
	{"xp_split_by_classes", 1, 1},
	{"rune_rounding", 1, 1},
	{"over_progression", 1, 1},
	{"artifact_cost_xp", 100000000, 1},
	{"broker_fee_percent", 25, 1}
};
const std::size_t kEditListinoRulesDefaultsCount =
	sizeof(kEditListinoRulesDefaults) / sizeof(kEditListinoRulesDefaults[0]);

const EditListinoCapGroupDefault kEditListinoCapGroupDefaults[] = {
	{"dam_sp", "Damroll + Spellpower"}
};
const std::size_t kEditListinoCapGroupDefaultsCount =
	sizeof(kEditListinoCapGroupDefaults) / sizeof(kEditListinoCapGroupDefaults[0]);

namespace {

[[nodiscard]] constexpr long SignedAffectCost(int modifier, long positiveUnit,
											  long negativeUnit) noexcept {
	if(modifier < 0) {
		return static_cast<long>(modifier) * negativeUnit;
	}
	return static_cast<long>(modifier) * positiveUnit;
}

[[nodiscard]] long ClampNonNegative(long v) noexcept {
	return std::max(0L, v);
}

[[nodiscard]] std::map<int, long> SumAffectsByLocation(const struct obj_data* obj);

[[nodiscard]] bool is_bitfield_apply(int loc) noexcept {
	return loc == APPLY_AFF2 || loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE ||
		   loc == APPLY_SPELL;
}

[[nodiscard]] unsigned or_affect_bits(const struct obj_data* obj, int loc) {
	unsigned bits = 0;
	if(!obj) {
		return 0;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			bits |= static_cast<unsigned>(obj->affected[i].modifier);
		}
	}
	return bits;
}

/* Unita' listino per un solo APPLY: legge edit_listino (cache, fallback default). */
[[nodiscard]] long AffectSlotValue(int location, int mod) {
	if(location == APPLY_NONE || location == APPLY_SKIP) {
		return 0;
	}

	if(location == APPLY_WEAPON_SPELL) {
		const EditListinoEntry* e =
			edit_listino_find_apply(location, static_cast<unsigned long>(mod));
		if(e == nullptr || !e->mud_supported || !e->enabled || !e->on_obj) {
			return 0;
		}
		return edit_listino_raw_unit_value(*e);
	}

	if(is_bitfield_apply(location)) {
		long valore = 0;
		for(const EditListinoEntry* e : edit_listino_rows_for_apply(location)) {
			if(e->apply_bit != 0 &&
			   IS_SET(static_cast<unsigned>(mod), e->apply_bit)) {
				valore += edit_listino_raw_unit_value(*e);
			}
		}
		return valore;
	}

	const EditListinoEntry* e = edit_listino_find_apply(location, 0UL);
	if(e == nullptr || e->xp == 0 || !e->mud_supported || !e->enabled || !e->on_obj) {
		return 0;
	}
	const long unit = edit_listino_raw_unit_value(*e);
	const int step = edit_listino_entry_step(*e);
	if(e->sign < 0) {
		/* AC / spellfail: piu' negativo = piu' valore. Prezzo a slot: / step. */
		return (-static_cast<long>(mod) * unit) / static_cast<long>(step);
	}
	const auto& rules = edit_listino_rules_cache();
	const int den = rules.malus_mult_den > 0 ? rules.malus_mult_den : 1;
	const long malus = (unit * static_cast<long>(rules.malus_mult_num)) / den;
	return SignedAffectCost(mod, unit, malus) / static_cast<long>(step);
}

/*
 * Listino staff vs baseline (create/proto): conta solo bonus aggiunti.
 * Togliere affect gia' in baseline non abbassa il valore.
 * Senza ITEM2_PAID_MALUS i malus proto (mod < 0) non generano delta quando
 * spariscono; con il flag il recupero malus conta nel costo listino
 * (non nel credit edit_pool).
 */
[[nodiscard]] long IncrementalStaffAffectValore(const struct obj_data* edited,
												  const struct obj_data* baseline) {
	if(!edited || !baseline) {
		return 0;
	}
	const bool paid_malus = IS_OBJ_STAT2(edited, ITEM2_PAID_MALUS);
	const auto a = SumAffectsByLocation(edited);
	const auto b = SumAffectsByLocation(baseline);
	long valore = 0;

	auto ia = a.begin();
	auto ib = b.begin();
	while(ia != a.end() || ib != b.end()) {
		int loc = 0;
		if(ib == b.end() || (ia != a.end() && ia->first < ib->first)) {
			loc = ia->first;
			++ia;
		}
		else if(ia == a.end() || (ib != b.end() && ib->first < ia->first)) {
			loc = ib->first;
			++ib;
		}
		else {
			loc = ia->first;
			++ia;
			++ib;
		}

		if(is_bitfield_apply(loc)) {
			const unsigned added =
				or_affect_bits(edited, loc) & ~or_affect_bits(baseline, loc);
			if(added != 0) {
				valore += AffectSlotValue(loc, static_cast<int>(added));
			}
			continue;
		}

		const long cur = a.count(loc) ? a.at(loc) : 0;
		const long base = b.count(loc) ? b.at(loc) : 0;
		if(loc == APPLY_AC) {
			if(cur < base) {
				valore += AffectSlotValue(loc, static_cast<int>(cur - base));
			}
			continue;
		}
		const long base_eff = paid_malus ? base : std::max(0L, base);
		const long delta = cur - base_eff;
		if(delta > 0) {
			valore += AffectSlotValue(loc, static_cast<int>(delta));
		}
	}
	return valore;
}

/* Come CheckValueObj ma i contributi affect negativi valgono 0 (malus ignorati). */
[[nodiscard]] ExpValue CheckValueObjNonNegAffects(const struct obj_data* obj) {
	if(obj == nullptr) {
		return {};
	}

	long valore = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const long slot =
			AffectSlotValue(obj->affected[i].location, obj->affected[i].modifier);
		valore += std::max(0L, slot);
	}

	const long dayUnits = static_cast<long>(
		std::ceil(static_cast<double>(obj->obj_flags.cost_per_day) / 1000.0));
	const long derent = dayUnits * (kObjValuePriceExp / 10000);
	const int rune = static_cast<int>(dayUnits * kObjValuePriceRune);

	return ExpValue{valore, derent, rune};
}

[[nodiscard]] ExpValue DiffFromRaw(const ExpValue& edited, const ExpValue& original) noexcept {
	ExpValue diff;
	/* Edit: aumento di valore rispetto al prototipo. */
	diff.valore = ClampNonNegative((edited.valore - original.valore) * kObjValueStorageScale);
	/*
	 * Derent: si "spende" per abbassare cost_per_day rispetto al prototipo
	 * (formula ProvaLocale su value_exp_total - value_exp).
	 */
	diff.derent = ClampNonNegative((original.derent - edited.derent) * kObjValueStorageScale);
	diff.rune = static_cast<int>(ClampNonNegative(static_cast<long>(original.rune - edited.rune)));
	return diff;
}

[[nodiscard]] bool DiffHasValue(const ExpValue& d) noexcept {
	return d.valore != 0 || d.derent != 0 || d.rune != 0;
}

[[nodiscard]] std::map<int, long> SumAffectsByLocation(const struct obj_data* obj) {
	std::map<int, long> out;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		if(loc == APPLY_NONE || loc == APPLY_SKIP || mod == 0) {
			continue;
		}
		out[loc] += mod;
	}
	return out;
}

[[nodiscard]] std::string TypeName(int type, const char* names[]) {
	char buf[MAX_STRING_LENGTH];
	sprinttype(type, names, buf);
	return buf;
}

[[nodiscard]] std::string BitNames(unsigned long bits, const char* names[]) {
	char buf[MAX_STRING_LENGTH];
	sprintbit(bits, names, buf);
	return buf;
}

void AppendLine(std::ostringstream& out, const std::string& line) {
	out << "  " << line << "\n\r";
}

void AppendFlagDelta(std::ostringstream& out, unsigned long edited, unsigned long original,
					 const char* names[], const char* label) {
	const unsigned long added = edited & ~original;
	const unsigned long removed = original & ~edited;
	if(added != 0) {
		AppendLine(out, std::string("+ ") + label + ": " + BitNames(added, names));
	}
	if(removed != 0) {
		AppendLine(out, std::string("- ") + label + ": " + BitNames(removed, names));
	}
}

/** Emette linee +/− affect vs proto, distinguendo quota wiz [delta] e [trasferimento]. */
void AppendAffectDeltaLines(std::ostringstream& out, int loc, long total_delta,
							unsigned long long instance_id) {
	long broker = 0;
#if USE_MYSQL
	if(instance_id != 0) {
		broker = object_instance_sum_broker_delta(instance_id, loc);
	}
#else
	(void)instance_id;
#endif
	auto emit = [&](long amount, const char* tag) {
		if(amount == 0) {
			return;
		}
		if(amount > 0) {
			AppendLine(out, "+ " + TypeName(loc, apply_types) + " by " +
								std::to_string(amount) + " [" + tag + "]");
		} else {
			/* Armor e malus: mostra il segno nel by (es. by -10 [trasferimento]). */
			AppendLine(out, "+ " + TypeName(loc, apply_types) + " by " +
								std::to_string(amount) + " [" + tag + "]");
		}
	};
	/*
	 * Netto vs proto zero ma c'e' quota trasferimento: etichetta comunque lo
	 * spostamento (es. armor tornata al valore proto dopo un transfer).
	 * Non inventare un [delta] compensativo.
	 */
	if(total_delta == 0) {
		if(broker != 0) {
			emit(broker, "trasferimento");
		}
		return;
	}
	/* Clamp broker dentro total_delta (stesso segno / non oltre). */
	if(loc == APPLY_AC) {
		/* total e broker tipicamente negativi se miglioramento. */
		if(total_delta >= 0) {
			broker = 0;
		} else {
			if(broker > 0) {
				broker = 0;
			}
			if(broker < total_delta) {
				broker = total_delta;
			}
		}
	} else {
		if(total_delta <= 0) {
			broker = 0;
		} else {
			if(broker < 0) {
				broker = 0;
			}
			if(broker > total_delta) {
				broker = total_delta;
			}
		}
	}
	const long wiz = total_delta - broker;
	/* Per rimozioni nette vs proto (total_delta<0 e non AC improve): riga unica. */
	if(loc != APPLY_AC && total_delta < 0 && broker == 0) {
		AppendLine(out, "- " + TypeName(loc, apply_types) + " by " +
							std::to_string(-total_delta) + " [delta]");
		return;
	}
	emit(wiz, "delta");
	emit(broker, "trasferimento");
}

[[nodiscard]] bool IsCombatChannelApply(int loc) noexcept {
	return loc == APPLY_HITROLL || loc == APPLY_DAMROLL || loc == APPLY_SPELLPOWER ||
		   loc == APPLY_HITNDAM || loc == APPLY_HITNSP;
}

[[nodiscard]] long CombatEffHr(const std::map<int, long>& m) {
	const auto get = [&](int loc) -> long {
		const auto it = m.find(loc);
		return it == m.end() ? 0 : it->second;
	};
	return get(APPLY_HITROLL) + get(APPLY_HITNDAM) + get(APPLY_HITNSP);
}

[[nodiscard]] long CombatEffDam(const std::map<int, long>& m) {
	const auto get = [&](int loc) -> long {
		const auto it = m.find(loc);
		return it == m.end() ? 0 : it->second;
	};
	return get(APPLY_DAMROLL) + get(APPLY_HITNDAM);
}

[[nodiscard]] long CombatEffSp(const std::map<int, long>& m) {
	const auto get = [&](int loc) -> long {
		const auto it = m.find(loc);
		return it == m.end() ? 0 : it->second;
	};
	return get(APPLY_SPELLPOWER) + get(APPLY_HITNSP);
}

[[nodiscard]] std::string DescribeStructuralDiff(const struct obj_data* edited,
												 const struct obj_data* original) {
	std::ostringstream out;
	const unsigned long long iid =
		edited ? edited->db_instance_id : 0ull;

	const auto a = SumAffectsByLocation(edited);
	const auto b = SumAffectsByLocation(original);
	auto ia = a.begin();
	auto ib = b.begin();
	while(ia != a.end() || ib != b.end()) {
		if(ib == b.end() || (ia != a.end() && ia->first < ib->first)) {
			if(!IsCombatChannelApply(ia->first)) {
				AppendAffectDeltaLines(out, ia->first, ia->second, iid);
			}
			++ia;
		}
		else if(ia == a.end() || (ib != b.end() && ib->first < ia->first)) {
			if(!IsCombatChannelApply(ib->first)) {
				/* Presente solo sul proto: rimosso dall'edit (sempre delta wiz). */
				AppendLine(out, "- " + TypeName(ib->first, apply_types) + " by " +
									std::to_string(ib->second) + " [delta]");
			}
			++ib;
		}
		else {
			if(!IsCombatChannelApply(ia->first)) {
				const long delta = ia->second - ib->second;
				AppendAffectDeltaLines(out, ia->first, delta, iid);
			}
			++ia;
			++ib;
		}
	}

	/*
	 * Combat: solo differenza sui canali effective (HITNDAM in HR+DAM, HITNSP in HR+SP).
	 * Evita di mostrare +2 HR e +4 DAM quando il proto aveva gia' HIT-N-DAM +2.
	 */
	{
		long d_hr = CombatEffHr(a) - CombatEffHr(b);
		long d_dam = CombatEffDam(a) - CombatEffDam(b);
		long d_sp = CombatEffSp(a) - CombatEffSp(b);
		const auto get = [](const std::map<int, long>& m, int loc) -> long {
			const auto it = m.find(loc);
			return it == m.end() ? 0 : it->second;
		};
		const long hd_raw = std::max(0L, get(a, APPLY_HITNDAM) - get(b, APPLY_HITNDAM));
		const long hs_raw = std::max(0L, get(a, APPLY_HITNSP) - get(b, APPLY_HITNSP));
		const long hr_raw = std::max(0L, get(a, APPLY_HITROLL) - get(b, APPLY_HITROLL));
		const long dam_raw = std::max(0L, get(a, APPLY_DAMROLL) - get(b, APPLY_DAMROLL));
		const long sp_raw = std::max(0L, get(a, APPLY_SPELLPOWER) - get(b, APPLY_SPELLPOWER));

		const long hd = std::min({std::max(0L, d_hr), std::max(0L, d_dam), hd_raw});
		if(hd > 0) {
			AppendAffectDeltaLines(out, APPLY_HITNDAM, hd, iid);
			d_hr -= hd;
			d_dam -= hd;
		}
		const long hs = std::min({std::max(0L, d_hr), std::max(0L, d_sp), hs_raw});
		if(hs > 0) {
			AppendAffectDeltaLines(out, APPLY_HITNSP, hs, iid);
			d_hr -= hs;
			d_sp -= hs;
		}
		const long hr_pos = std::min(std::max(0L, d_hr), hr_raw);
		if(hr_pos > 0) {
			AppendAffectDeltaLines(out, APPLY_HITROLL, hr_pos, iid);
			d_hr -= hr_pos;
		}
		const long dam_pos = std::min(std::max(0L, d_dam), dam_raw);
		if(dam_pos > 0) {
			AppendAffectDeltaLines(out, APPLY_DAMROLL, dam_pos, iid);
			d_dam -= dam_pos;
		}
		const long sp_pos = std::min(std::max(0L, d_sp), sp_raw);
		if(sp_pos > 0) {
			AppendAffectDeltaLines(out, APPLY_SPELLPOWER, sp_pos, iid);
			d_sp -= sp_pos;
		}
		/* Residui negativi di canale (raro): segnala come rimozione. */
		if(d_hr < 0) {
			AppendLine(out, "- " + TypeName(APPLY_HITROLL, apply_types) + " by " +
								std::to_string(-d_hr) + " [delta]");
		}
		if(d_dam < 0) {
			AppendLine(out, "- " + TypeName(APPLY_DAMROLL, apply_types) + " by " +
								std::to_string(-d_dam) + " [delta]");
		}
		if(d_sp < 0) {
			AppendLine(out, "- " + TypeName(APPLY_SPELLPOWER, apply_types) + " by " +
								std::to_string(-d_sp) + " [delta]");
		}
	}

	AppendFlagDelta(out, edited->obj_flags.extra_flags, original->obj_flags.extra_flags,
					extra_bits, "extra");
	AppendFlagDelta(out, edited->obj_flags.extra_flags2, original->obj_flags.extra_flags2,
					extra_bits2, "extra2");
	AppendFlagDelta(out, edited->obj_flags.wear_flags, original->obj_flags.wear_flags,
					wear_bits, "wear");
	AppendFlagDelta(out, edited->obj_flags.bitvector, original->obj_flags.bitvector,
					affected_bits, "affect bits");

	if(edited->obj_flags.cost_per_day != original->obj_flags.cost_per_day) {
		AppendLine(out, "cost_per_day: " + std::to_string(original->obj_flags.cost_per_day) +
							" -> " + std::to_string(edited->obj_flags.cost_per_day));
	}
	if(edited->obj_flags.cost != original->obj_flags.cost) {
		AppendLine(out, "cost: " + std::to_string(original->obj_flags.cost) + " -> " +
							std::to_string(edited->obj_flags.cost));
	}
	if(edited->obj_flags.weight != original->obj_flags.weight) {
		AppendLine(out, "weight: " + std::to_string(original->obj_flags.weight) + " -> " +
							std::to_string(edited->obj_flags.weight));
	}
	for(int v = 0; v < 4; ++v) {
		if(edited->obj_flags.value[v] != original->obj_flags.value[v]) {
			AppendLine(out, "value[" + std::to_string(v) + "]: " +
								std::to_string(original->obj_flags.value[v]) + " -> " +
								std::to_string(edited->obj_flags.value[v]));
		}
	}
	if(edited->obj_flags.type_flag != original->obj_flags.type_flag) {
		AppendLine(out, "type: " + TypeName(original->obj_flags.type_flag, item_types) +
							" -> " + TypeName(edited->obj_flags.type_flag, item_types));
	}

	return out.str();
}

[[nodiscard]] bool IsMarkedEdited(const struct obj_data* obj) noexcept {
	return IS_OBJ_STAT2(obj, ITEM2_EDIT) || IS_OBJ_STAT2(obj, ITEM2_PERSONAL);
}

[[nodiscard]] std::string ResolveEditOwnerName(const struct obj_data* obj) {
	if(obj && obj->personal_owner[0] != '\0') {
		return obj->personal_owner;
	}
	return object_instance_extract_ed_owner(obj ? obj->name : nullptr);
}

[[nodiscard]] int CountClassesInFileU(const struct char_file_u& st) {
	int tot = 0;
	for(int i = 0; i < MAX_CLASS; ++i) {
		if(st.level[i]) {
			++tot;
		}
	}
	return tot;
}

[[nodiscard]] int CountOwnerClassesMysql(const std::string& name) {
#if !USE_MYSQL
	(void)name;
	return 0;
#else
	if(name.empty()) {
		return 0;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == name);
		if(!pg || !pg->id) {
			return 0;
		}
		DB* db = Sql::getMysql();
		if(!db) {
			return 0;
		}
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT COUNT(*) FROM character_classes WHERE toon_id=" << pg->id
			<< " AND level > 0";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const int n = (row && row[0]) ? static_cast<int>(strtol(row[0], nullptr, 10)) : 0;
		mysql_free_result(res);
		return n;
	}
	catch(...) {
		return 0;
	}
#endif
}

[[nodiscard]] int ResolveOwnerClassCount(const std::string& owner_name) {
	if(owner_name.empty()) {
		return 0;
	}
	struct char_data* online = get_char(owner_name.c_str());
	if(online && !IS_NPC(online)) {
		return HowManyClasses(online);
	}
	const int from_mysql = CountOwnerClassesMysql(owner_name);
	if(from_mysql > 0) {
		return from_mysql;
	}
	char_file_u st {};
	if(load_char_mysql(owner_name.c_str(), &st)) {
		const int n = CountClassesInFileU(st);
		if(n > 0) {
			return n;
		}
	}
	if(load_char(owner_name.c_str(), &st)) {
		return CountClassesInFileU(st);
	}
	return 0;
}

[[nodiscard]] double ClassMultFromCount(int class_count) noexcept {
	return edit_listino_class_mult(class_count);
}

} // namespace

ExpValue ScaleObjExpValue(const ExpValue& raw, long scale) noexcept {
	return ExpValue{
		raw.valore * scale,
		raw.derent * scale,
		raw.rune
	};
}

ExpValue CheckValueObj(const struct obj_data* obj) {
	if(obj == nullptr) {
		return {};
	}

	long valore = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		valore += AffectSlotValue(obj->affected[i].location, obj->affected[i].modifier);
	}

	const long dayUnits = static_cast<long>(
		std::ceil(static_cast<double>(obj->obj_flags.cost_per_day) / 1000.0));
	const long derent = dayUnits * (kObjValuePriceExp / 10000);
	const int rune = static_cast<int>(dayUnits * kObjValuePriceRune);

	return ExpValue{valore, derent, rune};
}

ExpValue CheckDiffValue(struct obj_data* obj) {
	return AnalyzeObjEdit(obj).diff;
}

ObjEditAnalysis AnalyzeObjEditAgainst(struct obj_data* obj, const struct obj_data* baseline,
									  bool staff_incremental_absolute) {
	ObjEditAnalysis report;
	if(obj == nullptr || baseline == nullptr) {
		return report;
	}

	const ExpValue edited = CheckValueObj(obj);
	const ExpValue base_val = CheckValueObj(baseline);
	const bool paid_malus = IS_OBJ_STAT2(obj, ITEM2_PAID_MALUS);

	if(staff_incremental_absolute) {
		/* Solo affect aggiunti vs baseline; togliere rolled/proto non e' negativo. */
		const long inc = IncrementalStaffAffectValore(obj, baseline);
		report.absolute.valore = ClampNonNegative(inc);
		report.absolute.derent = ClampNonNegative(edited.derent - base_val.derent);
		report.absolute.rune =
			static_cast<int>(ClampNonNegative(static_cast<long>(edited.rune - base_val.rune)));
		report.diff.valore = ClampNonNegative(inc * kObjValueStorageScale);
		report.diff.derent =
			ClampNonNegative((base_val.derent - edited.derent) * kObjValueStorageScale);
		report.diff.rune = static_cast<int>(
			ClampNonNegative(static_cast<long>(base_val.rune - edited.rune)));
	}
	else if(paid_malus) {
		/* Recupero malus proto nel costo listino; il pool ignora comunque. */
		report.diff = DiffFromRaw(edited, base_val);
		report.absolute = edited;
	}
	else {
		/* Senza PAID_MALUS: malus proto/pezzo non gonfiano il diff (es. forziere
		 * MOVE -30 tolto da dispel). */
		report.diff =
			DiffFromRaw(CheckValueObjNonNegAffects(obj), CheckValueObjNonNegAffects(baseline));
		report.absolute = edited;
	}

	report.owner_name = ResolveEditOwnerName(obj);
	report.owner_classes = ResolveOwnerClassCount(report.owner_name);
	report.class_mult = ClassMultFromCount(report.owner_classes);
	if(report.class_mult != 1.0 && report.diff.valore > 0) {
		report.diff.valore =
			static_cast<long>(std::llround(static_cast<double>(report.diff.valore) *
										   report.class_mult));
	}

	/* Edit PG: listino ufficiale + artifact (regola edit_listino, default 3/2),
	 * indipendente dal flag ITEM_IMMUNE sull'istanza. */
	if(report.diff.valore > 0) {
		const auto& rules = edit_listino_rules_cache();
		const int den = rules.artifact_mult_den > 0 ? rules.artifact_mult_den : 1;
		report.diff.valore =
			(report.diff.valore * rules.artifact_mult_num) / den;
	}

	/* Flag artifact aggiunto vs baseline: costo regola, x class_mult, senza 3/2. */
	if(IS_OBJ_STAT(obj, ITEM_IMMUNE) && !IS_OBJ_STAT(baseline, ITEM_IMMUNE)) {
		report.diff.valore += EditArtifactFlagListinoCost(obj);
		const int ar = EditArtifactFlagListinoRunes();
		if(ar > 0) {
			const long long sum = static_cast<long long>(report.diff.rune) + ar;
			report.diff.rune = sum > 0x7fffffffLL ? 0x7fffffff : static_cast<int>(sum);
		}
	}

	report.changes = DescribeStructuralDiff(obj, baseline);
	report.listino_flags = edit_listino_obj_step_flags(obj);
	report.has_edit = IsMarkedEdited(obj) || DiffHasValue(report.diff) ||
					  !report.changes.empty();
	return report;
}

ObjEditAnalysis AnalyzeProcareaStaffEdit(struct obj_data* obj) {
	ObjEditAnalysis report;
	if(obj == nullptr || !procarea_obj_is_reward(obj)) {
		return report;
	}
#if USE_MYSQL
	if(obj->db_instance_id == 0) {
		return report;
	}
	struct obj_data* baseline =
		object_instance_materialize_create_baseline(obj->db_instance_id);
	if(baseline == nullptr) {
		return report;
	}
	report = AnalyzeObjEditAgainst(obj, baseline, true);
	extract_obj(baseline);
#else
	(void)obj;
#endif
	return report;
}

[[nodiscard]] int ResolvePrototypeVnum(const struct obj_data* obj) {
	int iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;

	const bool useOriginal =
		(IS_OBJ_STAT2(obj, ITEM2_PERSONAL) || IS_OBJ_STAT2(obj, ITEM2_EDIT)) &&
		obj->char_vnum > 0 &&
		static_cast<int>(obj->char_vnum) != iVNum;

	if(useOriginal) {
		return static_cast<int>(obj->char_vnum);
	}
	return iVNum;
}

ObjEditAnalysis AnalyzeObjEdit(struct obj_data* obj) {
	ObjEditAnalysis report;
	if(obj == nullptr) {
		return report;
	}

	const int iVNum = ResolvePrototypeVnum(obj);
	const int rNum = real_object(iVNum);
	struct obj_data* original = nullptr;
	if(rNum >= 0) {
		original = read_object(rNum, REAL);
	}

	if(original == nullptr) {
		report.absolute = CheckValueObj(obj);
		report.diff = DiffFromRaw(report.absolute, ExpValue{});
		report.listino_flags = edit_listino_obj_step_flags(obj);
		if(IsMarkedEdited(obj) || DiffHasValue(report.diff)) {
			report.changes = "  (prototipo non disponibile)\n\r";
			report.has_edit = true;
		}
		return report;
	}

	report = AnalyzeObjEditAgainst(obj, original, false);
	extract_obj(original);
	return report;
}

long EditAffectDeltaListinoCost(struct obj_data* obj, int location, int delta_mod) {
	if(obj == nullptr || location == APPLY_NONE || location == APPLY_SKIP ||
	   delta_mod == 0) {
		return 0;
	}

	const long raw = AffectSlotValue(location, delta_mod);
	if(raw <= 0) {
		return 0;
	}

	long scaled = raw * kObjValueStorageScale;
	const std::string owner = ResolveEditOwnerName(obj);
	const int classes = ResolveOwnerClassCount(owner);
	const double mult = ClassMultFromCount(classes);
	if(mult != 1.0) {
		scaled = static_cast<long>(
			std::llround(static_cast<double>(scaled) * mult));
	}

	/* Artifact sul listino edit (regola edit_listino, default 3/2), anche se
	 * il pezzo non ha ancora ITEM_IMMUNE. */
	if(scaled > 0) {
		const auto& rules = edit_listino_rules_cache();
		const int den = rules.artifact_mult_den > 0 ? rules.artifact_mult_den : 1;
		scaled = (scaled * rules.artifact_mult_num) / den;
	}
	return scaled;
}

long EditArtifactFlagListinoCost(const struct obj_data* obj) {
	const auto& rules = edit_listino_rules_cache();
	int den = rules.artifact_cost_xp_den > 0 ? rules.artifact_cost_xp_den : 1;
	long long base = rules.artifact_cost_xp_num;
	if(base <= 0) {
		const EditListinoEntry* row = edit_listino_find("artifact");
		base = (row != nullptr && row->xp > 0) ? row->xp : 100000000LL;
		den = 1;
	}
	const std::string owner = obj ? ResolveEditOwnerName(obj) : std::string();
	const int classes = std::max(1, ResolveOwnerClassCount(owner));
	const auto [cn, cd] = edit_listino_class_mult_frac(classes);
	const int cden = cd > 0 ? cd : 1;
	/* artifact_cost_xp x class_mult (1, 3/2, 2), NON x n_classi, senza artifact_mult. */
	const long long numer = base * static_cast<long long>(cn);
	const long long denom = static_cast<long long>(den) * static_cast<long long>(cden);
	if(denom <= 0) {
		return 0;
	}
	const long long xp = numer / denom;
	if(xp > 0x7fffffffLL) {
		return 0x7fffffff;
	}
	return static_cast<long>(xp);
}

int EditArtifactFlagListinoRunes() {
	const EditListinoEntry* row = edit_listino_find("artifact");
	if(row == nullptr) {
		return 0;
	}
	const int den = row->rune_den > 0 ? row->rune_den : 1;
	return row->rune_num / den; /* riga attuale: 100/1 = 100 rune, no class_mult */
}

} // namespace Alarmud
