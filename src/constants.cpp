/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD */
/* $Id: constants.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $ */
/***************************  System  include ************************************/
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "constants.hpp"
#include "maximums.hpp"
#include "trap.hpp"
namespace Alarmud {
/* modificato da Jethro, agosto 2011 */
const char* ignoreKeywords[] = {
	"the",
	"in",
	"on",
	"at",
	"\n"
};
const char* eqKeywords[] = {
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"about",
	"waist",
	"wrist",
	"wield",
	"hold",
	"shield",
	"back",
	"ears",
	"eye",
	"\n"
};

const char* exitKeywords[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
};
const char* lookKeywords[]= {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"in",
	"at",
	"",  /* Look at '' case */
	"room",
	"\n"
};
const char* stats[]= {
	"STR",
	"INT",
	"WIS",
	"DEX",
	"CON",
	"CHR",
	"\0"
};
/*Messaggi per quando una spell scade*/
const char* spell_wear_off_msg[] = {
											"RESERVED DB.C",
	/*	  1 SPELL_ARMOR					*/	"La tua $c0011armatura magica$c0007 si dissolve.",
	/*	  2 SPELL_TELEPORT				*/	"!Teleport!",
	/*	  3 SPELL_BLESS					*/	"Senti svanire la $c0015benedizione Divina$c0007.",
	/*	  4 SPELL_BLINDNESS				*/	"Senti svanire la tua $c0015cecita'$c0007.",
	/*	  5 SPELL_BURNING_HANDS			*/	"!Burning Hands!",
	/*	  6 SPELL_CALL_LIGHTNING		*/	"!Call Lightning",
	/*	  7 SPELL_CHARM_PERSON			*/	"Controlli di nuovo le tue azioni.",
	/*	  8 SPELL_CHILL_TOUCH			*/	"Ti senti piu' $c0009forte$c0007 adesso.",
	/*	  9 SPELL_CLONE					*/	"!Clone!",
	/*	 10 SPELL_COLOUR_SPRAY			*/	"!Color Spray!",
	/*	 11 SPELL_CONTROL_WEATHER		*/	"!Control Weather!",
	/*	 12 SPELL_CREATE_FOOD			*/	"!Create Food!",
	/*	 13 SPELL_CREATE_WATER			*/	"!Create Water!",
	/*	 14 SPELL_CURE_BLIND			*/	"!Cure Blind!",
	/*	 15 SPELL_CURE_CRITIC			*/	"!Cure Critic!",
	/*	 16 SPELL_CURE_LIGHT			*/	"!Cure Light!",
	/*	 17 SPELL_CURSE					*/	"Ti senti molto meglio.",
	/*	 18 SPELL_DETECT_EVIL			*/	"Senti svanire la tua percezione del $c0009Male$c0007.",
	/*	 19 SPELL_DETECT_INVISIBLE		*/	"Non riesci piu' a vedere l'$c0011invisibile$c0007.",
	/*	 20 SPELL_DETECT_MAGIC			*/	"Non riesci piu' a sentire la presenza della $c0012magia$c0007.",
	/*	 21 SPELL_DETECT_POISON			*/	"Non riesci piu' a sentire la presenza dei $c0010veleni$c0007.",
	/*	 22 SPELL_DISPEL_EVIL			*/	"!Dispel Evil!",
	/*	 23 SPELL_EARTHQUAKE			*/	"!Earthquake!",
	/*	 24 SPELL_ENCHANT_WEAPON		*/	"!Enchant Weapon!",
	/*	 25 SPELL_ENERGY_DRAIN			*/	"!Energy Drain!",
	/*	 26 SPELL_FIREBALL				*/	"!Fireball!",
	/*	 27 SPELL_HARM					*/	"!Harm!",
	/*	 28 SPELL_HEAL					*/	"!Heal",
	/*	 29 SPELL_INVISIBLE				*/	"Torni visibile.",
	/*	 30 SPELL_LIGHTNING_BOLT		*/	"!Lightning Bolt!",
	/*	 31 SPELL_LOCATE_OBJECT			*/	"!Locate object!",
	/*	 32 SPELL_MAGIC_MISSILE			*/	"!Magic Missile!",
	/*	 33 SPELL_POISON				*/	"Il $c0010veleno$c0007 non scorre piu' nelle tue vene.",
	/*	 34 SPELL_PROTECT_FROM_EVIL		*/	"Senti svanire la protezione dal $c0009Male$c0007.",
	/*	 35 SPELL_REMOVE_CURSE			*/	"!Remove Curse!",
	/*	 36 SPELL_SANCTUARY				*/	"L'$c0015aura bianca$c0007 che ti circondava svanisce.",
	/*	 37 SPELL_SHOCKING_GRASP		*/	"!Shocking Grasp!",
	/*	 38 SPELL_SLEEP					*/	"Ti senti riposat$b.",
	/*	 39 SPELL_STRENGTH				*/	"Non ti senti piu' cosi' $c0009forte$c0007.",
	/*	 40 SPELL_SUMMON				*/	"!Summon!",
	/*	 41 SPELL_VENTRILOQUATE			*/	"!Ventriloquate!",
	/*	 42 SPELL_WORD_OF_RECALL		*/	"!Word of Recall!",
	/*	 43 SPELL_REMOVE_POISON			*/	"!Remove Poison!",
	/*	 44 SPELL_SENSE_LIFE			*/	"Non percepisci piu' cosi' chiaramente quello che ti circonda.",
	/*	 45 SKILL_SNEAK					*/	"Smetti di muoverti silenziosamente.",
	/*	 46 SKILL_HIDE					*/	"!Hide!",
	/*	 47 SKILL_STEAL					*/	"!Steal!",
	/*	 48 SKILL_BACKSTAB				*/	"!Backstab!",
	/*	 49 SKILL_PICK_LOCK				*/	"!Pick Lock!",
	/*	 50 SKILL_KICK					*/	"!Kick!",
	/*	 51 SKILL_BASH					*/	"!Bash!",
	/*	 52 SKILL_RESCUE				*/	"!Rescue!",
	/*	 53 SPELL_IDENTIFY				*/	"!Identify!",
	/*	 54 SPELL_INFRAVISION			*/	"Perdi l'orientamento non appena la tua capacita' di vedere al $c0008buio$c0007 scompare.",
	/*	 55 SPELL_CAUSE_LIGHT			*/	"!cause light!",
	/*	 56 SPELL_CAUSE_CRITICAL		*/	"!cause crit!",
	/*	 57 SPELL_FLAMESTRIKE			*/	"!flamestrike!",
	/*	 58 SPELL_DISPEL_GOOD			*/	"!dispel good!",
	/*	 59 SPELL_WEAKNESS				*/	"Senti ritornare le tue forze...",
	/*	 60 SPELL_DISPEL_MAGIC			*/	"!dispel magic!",
	/*	 61 SPELL_KNOCK					*/	"!knock!",
	/*	 62 SPELL_KNOW_ALIGNMENT		*/	"!know alignment!",
	/*	 63 SPELL_ANIMATE_DEAD			*/	"!animate dead!",
	/*	 64 SPELL_PARALYSIS				*/	"Lentamente ricominci a muoverti.",
	/*	 65 SPELL_REMOVE_PARALYSIS		*/	"!remove paralysis!",
	/*	 66 SPELL_FEAR					*/	"!fear!",
	/*	 67 SPELL_ACID_BLAST			*/	"!acid blast!",
	/*	 68 SPELL_WATER_BREATH			*/	"La tua capacita' di respirare sott'$c0012acqua$c0007 svanisce.",
	/*	 69 SPELL_FLY					*/	"Torni dolcemente a $c0007terra$c0007, la tua capacita' di volare svanisce.",
	/*	 70 SPELL_CONE_OF_COLD			*/	"!cone of cold!",
	/*	 71 SPELL_METEOR_SWARM			*/	"!meteor swarm!",
	/*	 72 SPELL_ICE_STORM				*/	"!ice storm!",
	/*	 73 SPELL_SHIELD				*/	"Il tuo $c0011scudo$c0007 magico si dissolve.",
	/*	 74 SPELL_MON_SUM_1				*/	"!monsum one!",
	/*	 75 SPELL_MON_SUM_2				*/	"!monsum two!",
	/*	 76 SPELL_MON_SUM_3				*/	"!monsum three!",
	/*	 77 SPELL_MON_SUM_4				*/	"!monsum four!",
	/*	 78 SPELL_MON_SUM_5				*/	"!monsum five!",
	/*	 79 SPELL_MON_SUM_6				*/	"!monsum six!",
	/*	 80 SPELL_MON_SUM_7				*/	"!monsum seven!",
	/*	 81 SPELL_FIRESHIELD			*/	"Lo scudo di $c0001fuoco$c0007 che ti proteggeva si dissolve.",
	/*	 82 SPELL_CHARM_MONSTER			*/	"L'incantesimo che ti legava a qualcuno si dissolve.",
	/*	 83 SPELL_CURE_SERIOUS			*/	"!cure serious!",
	/*	 84 SPELL_CAUSE_SERIOUS			*/	"!cause serious!",
	/*	 85 SPELL_REFRESH				*/	"!refresh!",
	/*	 86 SPELL_SECOND_WIND			*/	"!second wind!",
	/*	 87 SPELL_TURN					*/	"!turn!",
	/*	 88 SPELL_SUCCOR				*/	"!succor!",
	/*	 89 SPELL_LIGHT					*/	"!light!",
	/*	 90 SPELL_CONT_LIGHT			*/	"!continual light!",
	/*	 91 SPELL_CALM					*/	"!calm!",
	/*	 92 SPELL_STONE_SKIN			*/	"La tua pelle torna normale.",
	/*	 93 SPELL_CONJURE_ELEMENTAL		*/	"!conjure elemental!",
	/*	 94 SPELL_TRUE_SIGHT			*/	"La tua $c0015visione$c0007 torna normale.",
	/*	 95 SPELL_MINOR_CREATE			*/	"!minor creation!",
	/*	 96 SPELL_FAERIE_FIRE			*/	"L'$c0013alone rosa$c0007 luminoso intorno a te sparisce.",
	/*	 97 SPELL_FAERIE_FOG			*/	"!faerie fog!",
	/*	 98 SPELL_CACAODEMON			*/	"!cacaodemon!",
	/*	 99 SPELL_POLY_SELF				*/	"Ritorni alla tua forma originale.",
	/*	100 SPELL_MANA					*/	"La tua $c0011protezione magica$c0007 scompare.",
	/*	101 SPELL_ASTRAL_WALK			*/	"!astral walk!",
	/*	102 SPELL_RESURRECTION			*/	"!resurrection!",
	/*	103 SPELL_H_FEAST				*/	"!heroes feast!",
	/*	104 SPELL_FLY_GROUP				*/	"Ti senti piu' $c0008pesante$c0007, la tua capacita' di volare ti sta abbandonando.",
	/*	105 SPELL_DRAGON_BREATH			*/	"!dragon breath!",
	/*	106 SPELL_WEB					*/	"Finalmente ti liberi dalle $c0008ragnatele$c0007.",
	/*	107 SPELL_MINOR_TRACK			*/	"Perdi la tua abilita' di seguire le tracce.",
	/*	108 SPELL_MAJOR_TRACK			*/	"La tua capacita' di seguire la tracce scompare.",
	/*	109 SPELL_GOLEM					*/	"!golem!",
	/*	110 SPELL_FAMILIAR				*/	"Puoi nuovamente evocare famigli.",
	/*	111 SPELL_CHANGESTAFF			*/	"!changestaff!",
	/*	112 SPELL_HOLY_WORD				*/	"!holy word!",
	/*	113 SPELL_UNHOLY_WORD			*/	"!unholy word!",
	/*	114 SPELL_PWORD_KILL			*/	"!pwk!",
	/*	115 SPELL_PWORD_BLIND			*/	"!pwb!",
	/*	116 SPELL_CHAIN_LIGHTNING		*/	"!chain lightning!",
	/*	117 SPELL_SCARE					*/	"!scare!",
	/*	118 SPELL_AID					*/	"Perdi l'$c0015aiuto Divino$c0007.",
	/*	119 SPELL_COMMAND				*/	"!command!",
	/*	120 SPELL_CHANGE_FORM			*/	"$c0010Torni alla tua forma originale.$c0007",
	/*	121 SPELL_FEEBLEMIND			*/	"Ti senti piu' $c0011intelligente$c0007.",
	/*	122 SPELL_SHILLELAGH			*/	"!shillelagh!",
	/*	123 SPELL_GOODBERRY				*/	"!goodberry!",
	/*	124 SPELL_ELEMENTAL_BLADE		*/	"!elemental blade!",
	/*	125 SPELL_ANIMAL_GROWTH			*/	"Le tue dimensioni tornano normali.",
	/*	126 SPELL_INSECT_GROWTH			*/	"Le tue dimensioni tornano normali.",
	/*	127 SPELL_CREEPING_DEATH		*/	"Ti senti meglio, la terribile sensazione che avevi prima e' passata.",
	/*	128 SPELL_COMMUNE				*/	"!commune!",
	/*	129 SPELL_ANIMAL_SUM_1			*/	"Puoi invocare di nuovo l'aiuto degli $c0003animali$c0007.",
	/*	130 SPELL_ANIMAL_SUM_2			*/	"Puoi invocare di nuovo l'aiuto degli $c0003animali$c0007.",
	/*	131 SPELL_ANIMAL_SUM_3			*/	"Puoi invocare di nuovo l'aiuto degli $c0003animali$c0007.",
	/*	132 SPELL_FIRE_SERVANT			*/	"$c0009Puoi nuovamente evocare elementali.",
	/*	133 SPELL_EARTH_SERVANT			*/	"$c0003Puoi nuovamente evocare elementali.",
	/*	134 SPELL_WATER_SERVANT			*/	"$c0012Puoi nuovamente evocare elementali.",
	/*	135 SPELL_WIND_SERVANT			*/	"$c0015Puoi nuovamente evocare elementali.",
	/*	136 SPELL_REINCARNATE			*/	"!reincarnate!",
	/*	137 SPELL_CHARM_VEGGIE			*/	"L'incantesimo che ti legava a qualcuno svanisce.",
	/*	138 SPELL_VEGGIE_GROWTH			*/	"Le tue dimensioni tornano normali.",
	/*	139 SPELL_TREE					*/	"Torni alla tua forma originale.",
	/*	140 SPELL_ANIMATE_ROCK			*/	"!Animate rock!",
	/*	141 SPELL_TREE_TRAVEL			*/	"Non ti senti piu' in contatto con gli $c0003alberi$c0007 come prima.",
	/*	142 SPELL_TRAVELLING			*/	"Senti svanire la capacita' di muoverti con meno fatica.",
	/*	143 SPELL_ANIMAL_FRIENDSHIP		*/	"Ti senti meno in contatto con gli $c0003animali$c0007.",
	/*	144 SPELL_INVIS_TO_ANIMALS		*/	"Torni visibile... almeno per gli $c0003animali$c0007.",
	/*	145 SPELL_SLOW_POISON			*/	"La tua resistenza al $c0010veleno$c0007 scompare.",
	/*	146 SPELL_ENTANGLE				*/	"Sei libero di muoverti.",
	/*	147 SPELL_SNARE					*/	"Non si piu' intrappolat$b!",
	/*	148 SPELL_GUST_OF_WIND			*/	"!gust of wind!",
	/*	149 SPELL_BARKSKIN				*/	"La tua pelle perde la consistenza della $c0003corteccia$c0007.",
	/*	150 SPELL_SUNRAY				*/	"!sunray!",
	/*	151 SPELL_WARP_WEAPON			*/	"!warp weapon!",
	/*	152 SPELL_HEAT_STUFF			*/	"Il tuo equipaggiamento finalmente si $c0014raffredda$c0007!",
	/*	153 SPELL_FIND_TRAPS			*/	"Non riesci piu' a vedere le $c0008trappole$c0007.",
	/*	154 SPELL_FIRESTORM				*/	"!firestorm!",
	/*	155 SPELL_HASTE					*/	"$c0008Senti i tuoi movimenti rallentare!",
	/*	156 SPELL_SLOW					*/	"$c0015Senti che i tuoi movimenti riacquistano la loro velocita'!",
	/*	157 SPELL_DUST_DEVIL			*/	"Puoi evocare nuovamente $c0009demoni$c0007 di $c0011sabbia$c0007.",
	/*	158 SPELL_KNOW_MONSTER			*/	"!know monster!",
	/*	159 SPELL_TRANSPORT_VIA_PLANT	*/	"!transport via plant!",
	/*	160 SPELL_SPEAK_WITH_PLANT		*/	"!speak with plant!",
	/*	161 SPELL_SILENCE				*/	"Puoi parlare di nuovo.",
	/*	162 SPELL_SENDING				*/	"!sending!",
	/*	163 SPELL_TELEPORT_WO_ERROR		*/	"!teleport without error!",
	/*	164 SPELL_PORTAL				*/	"!portal!",
	/*	165 SPELL_DRAGON_RIDE			*/	"Perdi l'affinita' con i $c0003draghi$c0007.",
	/*	166 SPELL_MOUNT					*/	"!mount!",
	/*	167 SPELL_NO_MESSAGE			*/	"",
	/*	168 to_do						*/	"!168!",
	/*	169 SKILL_MANTRA				*/	"La tua $c0011mente$c0007 ed il tuo $c0014corpo$c0007 perdono la loro sincronia.",
	/*	170 SKILL_FIRST_AID				*/	"Puoi medicarti di nuovo.",
	/*	171 SKILL_SIGN					*/	"!sign!",
	/*	172 SKILL_RIDE					*/	"!riding!",
	/*	173 SKILL_SWITCH_OPP			*/	"!switch!",
	/*	174 SKILL_DODGE					*/	"!dodge!",
	/*	175 SKILL_REMOVE_TRAP			*/	"!remove trap!",
	/*	176 SKILL_RETREAT				*/	"!retreat!",
	/*	177 SKILL_QUIV_PALM				*/	"Senti le tue mani $c0015vibrare$c0007...",
	/*	178 SKILL_SAFE_FALL				*/	"!safe fall!",
	/*	179 SKILL_FEIGN_DEATH			*/	"!feign death!",
	/*	180 SKILL_HUNT					*/	"!hunt!",
	/*	181 SKILL_FIND_TRAP				*/	"!find traps!",
	/*	182 SKILL_SPRING_LEAP			*/	"!spring leap!",
	/*	183 SKILL_DISARM				*/	"!disarm!",
	/*	184 SKILL_READ_MAGIC			*/	"!read magic!",
	/*	185 SKILL_EVALUATE				*/	"!evalutate!",
	/*	186 SKILL_SPY					*/	"Puoi spiare di nuovo.",
	/*	187 SKILL_DOORBASH				*/	"!doorbash!",
	/*	188 SKILL_SWIM					*/	"Puoi nuotare di nuovo.",
	/*	189 SKILL_CONS_UNDEAD			*/	"!consider undead!",
	/*	190 SKILL_CONS_VEGGIE			*/	"!consider veggie!",
	/*	191 SKILL_CONS_DEMON			*/	"!consider demon!",
	/*	192 SKILL_CONS_ANIMAL			*/	"!consider animal!",
	/*	193 SKILL_CONS_REPTILE			*/	"!consider reptile!",
	/*	194 SKILL_CONS_PEOPLE			*/	"!consider people!",
	/*	195 SKILL_CONS_GIANT			*/	"!consider giant!",
	/*	196 SKILL_CONS_OTHER			*/	"!consider other!",
	/*	197 SKILL_DISGUISE				*/	"Puoi mascherarti nuovamente.",
	/*	198 SKILL_CLIMB					*/	"!climb!",
	/*	199 SKILL_FINGER				*/	"!finger!",
	/*	200 SPELL_GEYSER				*/	"!geyser!",
	/*	201 SPELL_MIRROR_IMAGES			*/	"La tua $c0012immagine illusoria$c0007 svanisce.",
	/*	202 SKILL_TSPY					*/	"Puoi origliare di nuovo.",
	/*	203 SKILL_EAVESDROP				*/	"!eavesdrop!",
	/*	204 SKILL_PARRY					*/	"!parry!",
	/*	205 SKILL_MINER					*/	"!miner!",   /* ACIDUS 2003 */
	/*	206 SPELL_GREEN_SLIME			*/	"!green slime!",
	/*	207 SKILL_BERSERK				*/	"!berserk!",
	/*	208 SKILL_TAN					*/	"!tan!",
	/*	209 SKILL_AVOID_BACK_ATTACK		*/	"!avoid backattack!",
	/*	210 SKILL_FIND_FOOD				*/	"!find food!",
	/*	211 SKILL_FIND_WATER			*/	"!find water!",
	/*	212 SPELL_PRAYER				*/	"Ti senti in colpa sapendo che non hai piu' pregato.",
	/*	213 SKILL_MEMORIZE				*/	"Smetti di memorizzare e meditare.",
	/*	214 SKILL_BELLOW				*/	"!bellow!",
	/*	215 SPELL_GLOBE_DARKNESS		*/	"Il $c0008globo di oscurita'$c0007 intorno a te si dissolve.",
	/*	216 SPELL_GLOBE_MINOR_INV		*/	"II $c0012globo$c0007 protettivo attorno al tuo corpo si dissolve.",
	/*	217 SPELL_GLOBE_MAJOR_INV		*/	"Il $c0014globo$c0007 protettivo attorno al tuo corpo $c0015brilla$c0007 e si dissolve improvvisamente",
	/*	218 SPELL_PROT_ENERGY_DRAIN		*/	"Ti senti meno protetto dal tocco dei $c0008Non-Morti$c0007.",
	/*	219 SPELL_PROT_DRAGON_BREATH	*/	"Ti senti meno protetto dal soffio dei $c0003draghi$c0007.",
	/*	220 SPELL_ANTI_MAGIC_SHELL		*/	"Lo scudo $c0012anti-magia$c0007 intorno a te tremola e scompare.",
	/*	221 SKILL_DOORWAY				*/	"!doorway!",
	/*	222 SKILL_PORTAL				*/	"!psi_portal!",
	/*	223 SKILL_SUMMON				*/	"!psi_summon!",
	/*	224 SKILL_INVIS					*/	"!psi_invis!",
	/*	225 SKILL_CANIBALIZE			*/	"!canibalize!",
	/*	226 SKILL_FLAME_SHROUD			*/	"!flame shroud!",
	/*	227 SKILL_AURA_SIGHT			*/	"!aura sight!",
	/*	228 SKILL_GREAT_SIGHT			*/	"!great sight!",
	/*	229 SKILL_PSIONIC_BLAST			*/	"Il tuo cervello si sta lentamente riprendendo...",
	/*	230 SKILL_HYPNOSIS				*/	"!hypnosis!",
	/*	231 SKILL_MEDITATE				*/	"Senti di aver meditato abbastanza.",
	/*	232 SKILL_SCRY					*/	"!scry!",
	/*	233 SKILL_ADRENALIZE			*/	"La $c0009furia$c0007 scompare lasciando il posto ad un senso di $c0014calma$c0007.",
	/*	234 SKILL_BREW					*/	"!brew!",
	/*	235 SKILL_RATION				*/	"!ration!",
	/*	236 SKILL_HOLY_WARCRY			*/	"!holy warcry!",
	/*	237 SKILL_BLESSING				*/	"$c0015Puoi invocare i tuoi Dei di nuovo.",
	/*	238 SKILL_LAY_ON_HANDS			*/	"$c0015Puoi curarti di nuovo.",
	/*	239 SKILL_HEROIC_RESCUE			*/	"!heroic rescue!",
	/*	240 SKILL_DUAL_WIELD			*/	"!dual wield!",
	/*	241 SKILL_PSI_SHIELD			*/	"Lo $c0011scudo$c0007 creato dalla tua mente tremola e svanisce.",
	/*	242 SPELL_PROT_FROM_EVIL_GROUP	*/	"Ti senti meno protett$b dal $c0009Male$c0007.",
	/*	243 SPELL_PRISMATIC_SPRAY		*/	"!prismatic spry!",
	/*	244 SPELL_INCENDIARY_CLOUD		*/	"!incendiary cloud!",
	/*	245 SPELL_DISINTEGRATE			*/	"!disintegrate!",
	/*	246 LANG_COMMON					*/	"!lang common!",
	/*	247 LANG_ELVISH					*/	"!lang elvish!",
	/*	248 LANG_HALFLING				*/	"!lang halfling!",
	/*	249 LANG_DWARVISH				*/	"!lang dwarvish!",
	/*	250 LANG_ORCISH					*/	"!lang orcish!",
	/*	251 LANG_GIANTISH				*/	"!lang giantish!",
	/*	252 LANG_OGRE					*/	"!lang ogre!",
	/*	253 LANG_GNOMISH				*/	"!lang gnomish!",
	/*	254 SKILL_ESP					*/	"Smetti di ascoltare i pensieri degli altri.",
	/*	255 SPELL_COMP_LANGUAGES		*/	"Senti che la tua capacita' di comprende gli altri linguaggi svanisce.",
	/*	256 SPELL_PROT_FIRE				*/	"La tua protezione dal $c0009fuoco$c0007 svanisce.",
	/*	257 SPELL_PROT_COLD				*/	"La tua protezione dal $c0014freddo$c0007 svanisce.",
	/*	258 SPELL_PROT_ENERGY			*/	"La tua protezione dall'$c0011energia$c0007 svanisce.",
	/*	259 SPELL_PROT_ELEC				*/	"La tua protezione dall'$c0012elettricita'$c0007 svanisce.",
	/*	260 SPELL_ENCHANT_ARMOR			*/	"!enchant armor!",
	/*	261 SPELL_MESSENGER				*/	"!messenger!",
	/*	262 SPELL_PROT_BREATH_FIRE		*/	"La tua protezione dal soffio $c0009infuocato$c0007 dei draghi si dissolve.",
	/*	263 SPELL_PROT_BREATH_FROST		*/	"La tua protezione dal soffio $c0014gelido$c0007 dei draghi si dissolve.",
	/*	264 SPELL_PROT_BREATH_ELEC		*/	"La tua protezione dal soffio $c0012elettrico$c0007 dei draghi si dissolve.",
	/*	265 SPELL_PROT_BREATH_ACID		*/	"La tua protezione dal soffio $c0010acido$c0007 dei draghi si dissolve.",
	/*	266 SPELL_PROT_BREATH_GAS		*/	"La tua protezione dal soffio $c0011gassoso$c0007 dei draghi si dissolve.",
	/*	267 SPELL_WIZARDEYE				*/	"Il tuo occhio magico scompare con un sonoro $c0012*$c5011POP$c0012*$c0007!",
	/*	268 SKILL_MIND_BURN				*/	"!mind burn!",
	/*	269 SKILL_CLAIRVOYANCE			*/	"Non riesci piu' a concentrare la tua mente sui luoghi che ti circondano.",
	/*	270 SKILL_DANGER_SENSE			*/	"Non riesci piu' a percepire i $c0009pericoli$c0007 vicini.",
	/*	271 SKILL_DISINTEGRATE			*/	"!psi_disint!",
	/*	272 SKILL_TELEKINESIS			*/	"!telekinesis!",
	/*	273 SKILL_LEVITATION			*/	"Fluttui gentilmente a $c0003terra$c0007 mentre il tuo potere di levitazione si esaurisce.",
	/*	274 SKILL_CELL_ADJUSTMENT		*/	"!cell adjustment!",
	/*	275 SKILL_CHAMELEON				*/	"!chameleon!",
	/*	276 SKILL_PSI_STRENGTH			*/	"Senti che la $c0009forza$c0007 fisica generata dalla tua mente ti abbandona.",
	/*	277 SKILL_MIND_OVER_BODY		*/	"Inizi ad avere fame e sete!",
	/*	278 SKILL_PROBABILITY_TRAVEL	*/	"!probability travel!",
	/*	279 SKILL_PSI_TELEPORT			*/	"!psi teleport!",
	/*	280 SKILL_DOMINATION			*/	"!domination!",
	/*	281 SKILL_MIND_WIPE				*/	"!mind wipe!",
	/*	282 SKILL_PSYCHIC_CRUSH			*/	"!psychic crush!",
	/*	283 SKILL_TOWER_IRON_WILL		*/	"$c0012La tua volonta' non riesce piu' a proteggerti.",
	/*	284 SKILL_MINDBLANK				*/	"$c0008Non riesci piu' a tenere sgombra la mente.",
	/*	285 SKILL_PSYCHIC_IMPERSONATION	*/	"Torni ad essere te stess$b.",
	/*	286 SKILL_ULTRA_BLAST			*/	"!ultra blast!",
	/*	287 SKILL_INTENSIFY				*/	"Le tue capacita' tornano normali mentre la tua mente rallenta.",
	/*	288 SKILL_SPOT					*/	"!spot!",
	/*	289 SKILL_IMMOLATION			*/	"!immolation!",
	/*	290 STATUS_QUEST				*/	"$c0009Senti d'esser pront$b per una nuova missione.",
	/*	291 SKILL_DAIMOKU				*/	"Il tuo stato di ascesi ti fa sentire decisamente meglio!",
	/*	292 SKILL_FORGE					*/	"!forge!",
	/*	293 SKILL_DETERMINE				*/	"!determine!",
	/*	294 SKILL_EQUILIBRIUM			*/	"!equilibrium!",
											"\n"
};
/*Messaggio per la stanza quando una spell/skill scade*/
const char* spell_wear_off_room_msg[] = {
											"RESERVED DB.C",
	/*	  1 SPELL_ARMOR					*/	"L'$c0011armatura magica$c0007 di $n si dissolve lentamente.",
	/*	  2 SPELL_TELEPORT				*/	"!Teleport!",
	/*	  3 SPELL_BLESS					*/	"$n perde la $c0015benedizione Divina$c0007.",
	/*	  4 SPELL_BLINDNESS				*/	"$n sbatte velocemente le palpebre.",
	/*	  5 SPELL_BURNING_HANDS			*/	"!Burning Hands!",
	/*	  6 SPELL_CALL_LIGHTNING		*/	"!Call Lightning!",
	/*	  7 SPELL_CHARM_PERSON			*/	"$n si scuote e trema.",
	/*	  8 SPELL_CHILL_TOUCH			*/	"$n sembra un po' piu' $c0009forte$c0007.",
	/*	  9 SPELL_CLONE					*/	"!Clone!",
	/*	 10 SPELL_COLOUR_SPRAY			*/	"!Color Spray!",
	/*	 11 SPELL_CONTROL_WEATHER		*/	"!Control Weather!",
	/*	 12 SPELL_CREATE_FOOD			*/	"!Create Food!",
	/*	 13 SPELL_CREATE_WATER			*/	"!Create Water!",
	/*	 14 SPELL_CURE_BLIND			*/	"!Cure Blind!",
	/*	 15 SPELL_CURE_CRITIC			*/	"!Cure Critic!",
	/*	 16 SPELL_CURE_LIGHT			*/	"!Cure Light!",
	/*	 17 SPELL_CURSE					*/	"$n sembra sentirsi meglio.",
	/*	 18 SPELL_DETECT_EVIL			*/	"L'$c0009alone rosso$c0007 nello sguardo di $n sparisce.",
	/*	 19 SPELL_DETECT_INVISIBLE		*/	"L'$c0011alone giallo$c0007 nello sguardo di $n scompare.",
	/*	 20 SPELL_DETECT_MAGIC			*/	"L'$c0012alone blu$c0007 nello sguardo di $n si spegne.",
	/*	 21 SPELL_DETECT_POISON			*/	"Lo sguardo di $n perde l'$c0015alone bianco$c0007.",
	/*	 22 SPELL_DISPEL_EVIL			*/	"!Dispel Evil!",
	/*	 23 SPELL_EARTHQUAKE			*/	"!Earthquake!",
	/*	 24 SPELL_ENCHANT_WEAPON		*/	"!Enchant Weapon!",
	/*	 25 SPELL_ENERGY_DRAIN			*/	"!Energy Drain!",
	/*	 26 SPELL_FIREBALL				*/	"!Fireball!",
	/*	 27 SPELL_HARM					*/	"!Harm!",
	/*	 28 SPELL_HEAL					*/	"!Heal!",
	/*	 29 SPELL_INVISIBLE				*/	"$c0015$n$c0015 si materializza davanti i tuoi occhi.",
	/*	 30 SPELL_LIGHTNING_BOLT		*/	"!Lightning Bolt!",
	/*	 31 SPELL_LOCATE_OBJECT			*/	"!Locate object!",
	/*	 32 SPELL_MAGIC_MISSILE			*/	"!Magic Missile!",
	/*	 33 SPELL_POISON				*/	"$n sembra essere guarit$b.",
	/*	 34 SPELL_PROTECT_FROM_EVIL		*/	"$n sembra essere meno protett$b dal $c0009Male$c0007.",
	/*	 35 SPELL_REMOVE_CURSE			*/	"!Remove Curse!",
	/*	 36 SPELL_SANCTUARY				*/	"L'$c0015aura bianca$c0007 che avvolgeva il corpo di $n si spegne.",
	/*	 37 SPELL_SHOCKING_GRASP		*/	"!Shocking Grasp!",
	/*	 38 SPELL_SLEEP					*/	"$n mormora qualcosa e si scuote dal sonno.",
	/*	 39 SPELL_STRENGTH				*/	"$n sembra piu' $c0014debole$c0007.",
	/*	 40 SPELL_SUMMON				*/	"!Summon!",
	/*	 41 SPELL_VENTRILOQUATE			*/	"!Ventriloquate!",
	/*	 42 SPELL_WORD_OF_RECALL		*/	"!Word of Recall!",
	/*	 43 SPELL_REMOVE_POISON			*/	"!Remove Poison!",
	/*	 44 SPELL_SENSE_LIFE			*/	"$n smette di guardarsi intorno.",
	/*	 45 SKILL_SNEAK					*/	"",  /* NO MESSAGE FOR SNEAK*/
	/*	 46 SKILL_HIDE					*/	"!Hide!",
	/*	 47 SKILL_STEAL					*/	"!Steal!",
	/*	 48 SKILL_BACKSTAB				*/	"!Backstab!",
	/*	 49 SKILL_PICK_LOCK				*/	"!Pick Lock!",
	/*	 50 SKILL_KICK					*/	"!Kick!",
	/*	 51 SKILL_BASH					*/	"!Bash!",
	/*	 52 SKILL_RESCUE				*/	"!Rescue!",
	/*	 53 SPELL_IDENTIFY				*/	"!Identify!",
	/*	 54 SPELL_INFRAVISION			*/	"$n sbatte le palpebre e si guarda intorno spaesat$b, l'$c0009alone rosso$c0007 nel suo sguardo scompare.",
	/*	 55 SPELL_CAUSE_LIGHT			*/	"!cause light!",
	/*	 56 SPELL_CAUSE_CRITICAL		*/	"!cause crit!",
	/*	 57 SPELL_FLAMESTRIKE			*/	"!flamestrike!",
	/*	 58 SPELL_DISPEL_GOOD			*/	"!dispel magic!",
	/*	 59 SPELL_WEAKNESS				*/	"$n sembra piu' $c0009forte$c0007!",
	/*	 60 SPELL_DISPEL_MAGIC			*/	"!dispel good!",
	/*	 61 SPELL_KNOCK					*/	"!knock!",
	/*	 62 SPELL_KNOW_ALIGNMENT		*/	"!know alignment!",
	/*	 63 SPELL_ANIMATE_DEAD			*/	"!animate dead!",
	/*	 64 SPELL_PARALYSIS				*/	"$n ricomincia a muoversi lentamente.",
	/*	 65 SPELL_REMOVE_PARALYSIS		*/	"!remove paralysis!",
	/*	 66 SPELL_FEAR					*/	"!fear!",
	/*	 67 SPELL_ACID_BLAST			*/	"!acid blast!",
	/*	 68 SPELL_WATER_BREATH			*/	"$n rincomincia a respirare profondamente.",
	/*	 69 SPELL_FLY					*/	"Le forze magiche che mantenevano in volo $n svaniscono.",
	/*	 70 SPELL_CONE_OF_COLD			*/	"!cone of cold!",
	/*	 71 SPELL_METEOR_SWARM			*/	"!meteor swarm!",
	/*	 72 SPELL_ICE_STORM				*/	"!ice storm!",
	/*	 73 SPELL_SHIELD				*/	"Lo $c0011scudo$c0007 magico di $n si dissolve.",
	/*	 74 SPELL_MON_SUM_1				*/	"!monsum one!",
	/*	 75 SPELL_MON_SUM_2				*/	"!monsum two!",
	/*	 76 SPELL_MON_SUM_3				*/	"!monsum three!",
	/*	 77 SPELL_MON_SUM_4				*/	"!monsum four!",
	/*	 78 SPELL_MON_SUM_5				*/	"!monsum five!",
	/*	 79 SPELL_MON_SUM_6				*/	"!monsum six!",
	/*	 80 SPELL_MON_SUM_7				*/	"!monsum seven!",
	/*	 81 SPELL_FIRESHIELD			*/	"Lo scudo $c0009infuocato$c0007 che avvolgeva $n svanisce.",
	/*	 82 SPELL_CHARM_MONSTER			*/	"$n si scuote e trema.",
	/*	 83 SPELL_CURE_SERIOUS			*/	"!cure serious!",
	/*	 84 SPELL_CAUSE_SERIOUS			*/	"!cause serious!",
	/*	 85 SPELL_REFRESH				*/	"!refresh!",
	/*	 86 SPELL_SECOND_WIND			*/	"!second wind!",
	/*	 87 SPELL_TURN					*/	"!turn!",
	/*	 88 SPELL_SUCCOR				*/	"!succor!",
	/*	 89 SPELL_LIGHT					*/	"!light!",
	/*	 90 SPELL_CONT_LIGHT			*/	"!continual light!",
	/*	 91 SPELL_CALM					*/	"!calm!",
	/*	 92 SPELL_STONE_SKIN			*/	"La pelle di $n perde il suo aspetto $c0003granitico$c0007 e torna normale.",
	/*	 93 SPELL_CONJURE_ELEMENTAL		*/	"!conjure elemental!",
	/*	 94 SPELL_TRUE_SIGHT			*/	"$n sbatte rapidamente le palpebre, l'alone $c0015argenteo$c0007 scompare dai suoi occhi.",
	/*	 95 SPELL_MINOR_CREATE			*/	"!minor creation!",
	/*	 96 SPELL_FAERIE_FIRE			*/	"L'$c0013alone rosa$c0007 intorno al corpo di $n scompare.",
	/*	 97 SPELL_FAERIE_FOG			*/	"!faerie fog!",
	/*	 98 SPELL_CACAODEMON			*/	"!cacaodemon!",
	/*	 99 SPELL_POLY_SELF				*/	"$n riprende la sua vera forma.",
	/*	100 SPELL_MANA					*/	"La $c0011protezione magica$c0007 attorno al corpo di $n scompare.",
	/*	101 SPELL_ASTRAL_WALK			*/	"!astral walk!",
	/*	102 SPELL_RESURRECTION			*/	"!resurrection!",
	/*	103 SPELL_H_FEAST				*/	"!heroes feast!",
	/*	104 SPELL_FLY_GROUP				*/	"$n scende dolcemente a $c0003terra$c0007.",
	/*	105 SPELL_DRAGON_BREATH			*/	"!dragon breath!",
	/*	106 SPELL_WEB					*/	"$n sembra liberarsi delle $c0008ragnatele$c0007 che l$b imprigionavano.",
	/*	107 SPELL_MINOR_TRACK			*/	"$n ha uno sguardo confuso.",
	/*	108 SPELL_MAJOR_TRACK			*/	"$n ha uno sguardo $c0015MOLTO$c0007 confuso.",
	/*	109 SPELL_GOLEM					*/	"!golem!",
	/*	110 SPELL_FAMILIAR				*/	"",
	/*	111 SPELL_CHANGESTAFF			*/	"!changestaff!",
	/*	112 SPELL_HOLY_WORD				*/	"!holy word!",
	/*	113 SPELL_UNHOLY_WORD			*/	"!unholy word!",
	/*	114 SPELL_PWORD_KILL			*/	"!pwk!",
	/*	115 SPELL_PWORD_BLIND			*/	"!pwb!",
	/*	116 SPELL_CHAIN_LIGHTNING		*/	"!chain lightning!",
	/*	117 SPELL_SCARE					*/	"!scare!",
	/*	118 SPELL_AID					*/	"$n perde l'$c0015aiuto Divino$c0007.",
	/*	119 SPELL_COMMAND				*/	"!command!",
	/*	120 SPELL_CHANGE_FORM			*/	"$c0010$n$c0010 riprende la sua vera forma.",
	/*	121 SPELL_FEEBLEMIND			*/	"Lo sguardo di $n sembra riacquistare $c0011intelligenza$c0007.",
	/*	122 SPELL_SHILLELAGH			*/	"!shillelagh!",
	/*	123 SPELL_GOODBERRY				*/	"!goodberry!",
	/*	124 SPELL_ELEMENTAL_BLADE		*/	"!elemental blade!",
	/*	125 SPELL_ANIMAL_GROWTH			*/	"$n rimpicciolisce a vista d'occhio.",
	/*	126 SPELL_INSECT_GROWTH			*/	"$n rimpicciolisce a vista d'occhio.",
	/*	127 SPELL_CREEPING_DEATH		*/	"$n sembra sentirsi meglio...",
	/*	128 SPELL_COMMUNE				*/	"!commune!",
	/*	129 SPELL_ANIMAL_SUM_1			*/	"",
	/*	130 SPELL_ANIMAL_SUM_2			*/	"",
	/*	131 SPELL_ANIMAL_SUM_3			*/	"",
	/*	132 SPELL_FIRE_SERVANT			*/	"",
	/*	133 SPELL_EARTH_SERVANT			*/	"",
	/*	134 SPELL_WATER_SERVANT			*/	"",
	/*	135 SPELL_WIND_SERVANT			*/	"",
	/*	136 SPELL_REINCARNATE			*/	"!reincarnate!",
	/*	137 SPELL_CHARM_VEGGIE			*/	"$n si scuote e trema.",
	/*	138 SPELL_VEGGIE_GROWTH			*/	"$n rimpicciolisce a vista d'occhio.",
	/*	139 SPELL_TREE					*/	"$n riprende la sua vera forma.",
	/*	140 SPELL_ANIMATE_ROCK			*/	"!Animate rock!",
	/*	141 SPELL_TREE_TRAVEL			*/	"",
	/*	142 SPELL_TRAVELLING			*/	"$n sembra meno $c0010agile$c0007.",
	/*	143 SPELL_ANIMAL_FRIENDSHIP		*/	"",
	/*	144 SPELL_INVIS_TO_ANIMALS		*/	"L'immagine di $n $c0008v$c0007i$c0008b$c0007r$c0008a$c0007 per un attimo.",
	/*	145 SPELL_SLOW_POISON			*/	"$n sembra meno resistente ai $c0010veleni$c0007.",
	/*	146 SPELL_ENTANGLE				*/	"$n si libera!",
	/*	147 SPELL_SNARE					*/	"$n riesce a liberarsi da una trappola!",
	/*	148 SPELL_GUST_OF_WIND			*/	"!gust of wind!",
	/*	149 SPELL_BARKSKIN				*/	"La pelle di $n perde l'aspetto della $c0003corteccia$c0007.",
	/*	150 SPELL_SUNRAY				*/	"!sunray!",
	/*	151 SPELL_WARP_WEAPON			*/	"!warp weapon!",
	/*	152 SPELL_HEAT_STUFF			*/	"L'equipaggiamento di $n smette di $c0001bruciare$c0007.",
	/*	153 SPELL_FIND_TRAPS			*/	"Lo sguardo di $n perde il $c0015luccichio$c0007 iniziale.",
	/*	154 SPELL_FIRESTORM				*/	"!firestorm!",
	/*	155 SPELL_HASTE					*/	"$c0008$n$c0008 sembra muoversi piu' lentamente.",
	/*	156 SPELL_SLOW					*/	"$c0015$n$c0015 sembra riacquistare velocita'.",
	/*	157 SPELL_DUST_DEVIL			*/	"",
	/*	158 SPELL_KNOW_MONSTER			*/	"!know monster!",
	/*	159 SPELL_TRANSPORT_VIA_PLANT	*/	"!transport via plant!",
	/*	160 SPELL_SPEAK_WITH_PLANT		*/	"!speak with plant!",
	/*	161 SPELL_SILENCE				*/	"$n riacquista la favella... poveri noi!",
	/*	162 SPELL_SENDING				*/	"!sending!",
	/*	163 SPELL_TELEPORT_WO_ERROR		*/	"!teleport without error!",
	/*	164 SPELL_PORTAL				*/	"!portal!",
	/*	165 SPELL_DRAGON_RIDE			*/	"",
	/*	166 SPELL_MOUNT					*/	"!mount!",
	/*	167 SPELL_NO_MESSAGE			*/	"",
	/*	168 to_do						*/	"!168!",
	/*	169 SKILL_MANTRA				*/	"$n sembra tremare per un attimo e perdere il sincronismo.",
	/*	170 SKILL_FIRST_AID				*/	"",
	/*	171 SKILL_SIGN					*/	"!sign!",
	/*	172 SKILL_RIDE					*/	"!riding!",
	/*	173 SKILL_SWITCH_OPP			*/	"!switch!",
	/*	174 SKILL_DODGE					*/	"!dodge!",
	/*	175 SKILL_REMOVE_TRAP			*/	"!remove trap!",
	/*	176 SKILL_RETREAT				*/	"!retreat!",
	/*	177 SKILL_QUIV_PALM				*/	"Le mani di $n $c0015vibrano$c0007 per un attimo.",
	/*	178 SKILL_SAFE_FALL				*/	"!safe fall!",
	/*	179 SKILL_FEIGN_DEATH			*/	"!feign death!",
	/*	180 SKILL_HUNT					*/	"!hunt!",
	/*	181 SKILL_FIND_TRAP				*/	"!find traps!",
	/*	182 SKILL_SPRING_LEAP			*/	"!spring leap!",
	/*	183 SKILL_DISARM				*/	"!disarm!",
	/*	184 SKILL_READ_MAGIC			*/	"!read magic!",
	/*	185 SKILL_EVALUATE				*/	"!evalutate!",
	/*	186 SKILL_SPY					*/	"",
	/*	187 SKILL_DOORBASH				*/	"!doorbash!",
	/*	188 SKILL_SWIM					*/	"",
	/*	189 SKILL_CONS_UNDEAD			*/	"!consider undead!",
	/*	190 SKILL_CONS_VEGGIE			*/	"!consider veggie!",
	/*	191 SKILL_CONS_DEMON			*/	"!consider demon!",
	/*	192 SKILL_CONS_ANIMAL			*/	"!consider animal!",
	/*	193 SKILL_CONS_REPTILE			*/	"!consider reptile!",
	/*	194 SKILL_CONS_PEOPLE			*/	"!consider people!",
	/*	195 SKILL_CONS_GIANT			*/	"!consider giant!",
	/*	196 SKILL_CONS_OTHER			*/	"!consider other!",
	/*	197 SKILL_DISGUISE				*/	"",
	/*	198 SKILL_CLIMB					*/	"!climb!",
	/*	199 SKILL_FINGER				*/	"!finger!",
	/*	200 SPELL_GEYSER				*/	"!geyser!",
	/*	201 SPELL_MIRROR_IMAGES			*/	"L'$c0012immagine illusoria$c0007 di $n scompare.",
	/*	202 SKILL_TSPY					*/	"Puoi origliare di nuovo.",
	/*	203 SKILL_EAVESDROP				*/	"!eavesdrop!",
	/*	204 SKILL_PARRY					*/	"!parry!",
	/*	205 SKILL_MINER					*/	"!miner!",
	/*	206 SPELL_GREEN_SLIME			*/	"!green slime!",
	/*	207 SKILL_BERSERK				*/	"!berserk!",
	/*	208 SKILL_TAN					*/	"!tan!",
	/*	209 SKILL_AVOID_BACK_ATTACK		*/	"!avoid backattack!",
	/*	210 SKILL_FIND_FOOD				*/	"!find food!",
	/*	211 SKILL_FIND_WATER			*/	"!find water!",
	/*	212 SPELL_PRAYER				*/	"",
	/*	213 SKILL_MEMORIZE				*/	"$n smette di studiare il suo libro degli $c0012i$c0011n$c0012c$c0011a$c0012n$c0011t$c0012e$c0011s$c0012i$c0011m$c0012i$c0007.",
	/*	214 SKILL_BELLOW				*/	"!bellow!",
	/*	215 SPELL_GLOBE_DARKNESS		*/	"Il $c0008globo oscuro$c0007 che avvolgeva $n si dissolve.",
	/*	216 SPELL_GLOBE_MINOR_INV		*/	"Il $c0012globo$c0007 di protezione attorno a $n scompare.",
	/*	217 SPELL_GLOBE_MAJOR_INV		*/	"Il $c0014globo$c0007 di protezione attorno a $n scompare con un $c0011bagliore$c0007.",
	/*	218 SPELL_PROT_ENERGY_DRAIN		*/	"$n sembra meno protett$b dai $c0008Non-Morti$c0007.",
	/*	219 SPELL_PROT_DRAGON_BREATH	*/	"$n sembra aver piu' paura dei $c0003draghi$c0007.",
	/*	220 SPELL_ANTI_MAGIC_SHELL		*/	"Lo scudo $c0015brillante$c0007 attorno al corpo di $n si dissolve in una cascata di $c0011scintille$c0007.",
	/*	221 SKILL_DOORWAY				*/	"!doorway!",
	/*	222 SKILL_PORTAL				*/	"!psi_portal!",
	/*	223 SKILL_SUMMON				*/	"!psi_summon!",
	/*	224 SKILL_INVIS					*/	"!psi_invis!",
	/*	225 SKILL_CANIBALIZE			*/	"!canibalize!",
	/*	226 SKILL_FLAME_SHROUD			*/	"!flame shroud!",
	/*	227 SKILL_AURA_SIGHT			*/	"!aura sight!",
	/*	228 SKILL_GREAT_SIGHT			*/	"!great sight!",
	/*	229 SKILL_PSIONIC_BLAST			*/	"$n sembra meno stordit$b.",
	/*	230 SKILL_HYPNOSIS				*/	"!hypnosis!",
	/*	231 SKILL_MEDITATE				*/	"$n smette di meditare e sembra piu' rilassat$b ora.",
	/*	232 SKILL_SCRY					*/	"!scry!",
	/*	233 SKILL_ADRENALIZE			*/	"La $c0009furia$c0007 che possedeva $n scompare, addesso e' molto piu' $c0014calm$b$c0007.",
	/*	234 SKILL_BREW					*/	"!brew!",
	/*	235 SKILL_RATION				*/	"!ration!",
	/*	236 SKILL_HOLY_WARCRY			*/	"!holy warcry!",
	/*	237 SKILL_BLESSING				*/	"",
	/*	238 SKILL_LAY_ON_HANDS			*/	"",
	/*	239 SKILL_HEROIC_RESCUE			*/	"!heroic rescue!",
	/*	240 SKILL_DUAL_WIELD			*/	"!dual wield!",
	/*	241 SKILL_PSI_SHIELD			*/	"Lo $c0011scudo$c0007 psionico di $n tremola e si dissolve.",
	/*	242 SPELL_PROT_FROM_EVIL_GROUP	*/	"$n sembra temere di nuovo gli esseri $c0009malvagi$c0007.",
	/*	243 SPELL_PRISMATIC_SPRAY		*/	"!prismatic spry!",
	/*	244 SPELL_INCENDIARY_CLOUD		*/	"!incendiary cloud!",
	/*	245 SPELL_DISINTEGRATE			*/	"!disintegrate!",
	/*	246 LANG_COMMON					*/	"!lang common!",
	/*	247 LANG_ELVISH					*/	"!lang elvish!",
	/*	248 LANG_HALFLING				*/	"!lang halfling!",
	/*	249 LANG_DWARVISH				*/	"!lang dwarvish!",
	/*	250 LANG_ORCISH					*/	"!lang orcish!",
	/*	251 LANG_GIANTISH				*/	"!lang giantish!",
	/*	252 LANG_OGRE					*/	"!lang ogre!",
	/*	253 LANG_GNOMISH				*/	"!lang gnomish!",
	/*	254 SKILL_ESP					*/	"$n per un attimo cambia espressione...",
	/*	255 SPELL_COMP_LANGUAGES		*/	"Hai avuto l'impressione di veder cambiare colore alle orecchie di $n.",
	/*	256 SPELL_PROT_FIRE				*/	"$n sembra meno protett$b dal $c0009fuoco$c0007.",
	/*	257 SPELL_PROT_COLD				*/	"$n sembra meno protett$b dal $c0014freddo$c0007.",
	/*	258 SPELL_PROT_ENERGY			*/	"$n sembra meno protett$b dall'$c0011energia$c0007.",
	/*	259 SPELL_PROT_ELEC				*/	"$n sembra meno protett$b dall'$c0012elettricita'$c0007.",
	/*	260 SPELL_ENCHANT_ARMOR			*/	"!enchant armor!",
	/*	261 SPELL_MESSENGER				*/	"!messenger!",
	/*	262 SPELL_PROT_BREATH_FIRE		*/	"Lo scudo di protezione dal soffio di $c0009fuoco$c0007 che avvolgeva $n scompare.",
	/*	263 SPELL_PROT_BREATH_FROST		*/	"Lo scudo di protezione dal soffio $c0014gelido$c0007 che avvolgeva $n scompare.",
	/*	264 SPELL_PROT_BREATH_ELEC		*/	"Lo scudo di protezione dal soffio $c0012elettrico$c0007 che avvolgeva $n scompare",
	/*	265 SPELL_PROT_BREATH_ACID		*/	"Lo scudo di protezione dal soffio $c0010acido$c0007 che avvolgeva $n scompare.",
	/*	266 SPELL_PROT_BREATH_GAS		*/	"Lo scudo di protezione dal soffio $c0011gassoso$c0007 che avvolgeva $n scompare.",
	/*	267 SPELL_WIZARDEYE				*/	"Senti un $c0012*$c5011POP$c0012*$c0007 e l'occhio magico di $n scompare.",
	/*	268 SKILL_MIND_BURN				*/	"!mind burn!",
	/*	269 SKILL_CLAIRVOYANCE			*/	"$n tremola per un attimo.",
	/*	270 SKILL_DANGER_SENSE			*/	"$n sembra guardarsi intorno impaurit$b.",
	/*	271 SKILL_DISINTEGRATE			*/	"!psi_disint!",
	/*	272 SKILL_TELEKINESIS			*/	"!telekinesis!",
	/*	273 SKILL_LEVITATION			*/	"$n atterra dolcemente.",
	/*	274 SKILL_CELL_ADJUSTMENT		*/	"!cell adjustment!",
	/*	275 SKILL_CHAMELEON				*/	"!chameleon!",
	/*	276 SKILL_PSI_STRENGTH			*/	"$n sembra un po' piu' $c0014debole$c0007.",
	/*	277 SKILL_MIND_OVER_BODY		*/	"$n sembra aver fame.",
	/*	278 SKILL_PROBABILITY_TRAVEL	*/	"!probability travel!",
	/*	279 SKILL_PSI_TELEPORT			*/	"!psi teleport!",
	/*	280 SKILL_DOMINATION			*/	"!domination!",
	/*	281 SKILL_MIND_WIPE				*/	"!mind wipe!",
	/*	282 SKILL_PSYCHIC_CRUSH			*/	"!psychic crush!",
	/*	283 SKILL_TOWER_IRON_WILL		*/	"",
	/*	284 SKILL_MINDBLANK				*/	"",
	/*	285 SKILL_PSYCHIC_IMPERSONATION	*/	"",
	/*	286 SKILL_ULTRA_BLAST			*/	"!ultra blast!",
	/*	287 SKILL_INTENSIFY				*/	"Ti e' sembrato di vedere delle $c0011scintille$c0007 intorno alla testa di $n.",
	/*	288 SKILL_SPOT					*/	"!spot!",
	/*	289 SKILL_IMMOLATION			*/	"!immolation!",
	/*	290 STATUS_QUEST				*/	"",
	/*	291 SKILL_DAIMOKU				*/	"$n esce dal $c0011n$c0009i$c0014r$c0009v$c0011a$c0009n$c0014a!",
	/*	292 SKILL_FORGE					*/	"!forge!",
	/*	293 SKILL_DETERMINE				*/	"!determine!",
	/*	294 SKILL_EQUILIBRIUM			*/	"!equilibrium!",
											"\n"
};
/* Messaggio di preavviso di fine spell*/
const char* spell_wear_off_soon_msg[] = {
											"RESERVED DB.C",
	/*	  1 SPELL_ARMOR					*/	"La tua $c0011armatura magica$c0007 vacilla.",
	/*	  2 SPELL_TELEPORT				*/	"!Teleport!",
	/*	  3 SPELL_BLESS					*/	"",
	/*	  4 SPELL_BLINDNESS				*/	"",
	/*	  5 SPELL_BURNING_HANDS			*/	"!Burning Hands!",
	/*	  6 SPELL_CALL_LIGHTNING		*/	"!Call Lightning",
	/*	  7 SPELL_CHARM_PERSON			*/	"Ti sembra di tornare in te.",
	/*	  8 SPELL_CHILL_TOUCH			*/	"Ti senti un po' piu' $c0009forte$c0007.",
	/*	  9 SPELL_CLONE					*/	"!Clone!",
	/*	 10 SPELL_COLOUR_SPRAY			*/	"!Color Spray!",
	/*	 11 SPELL_CONTROL_WEATHER		*/	"!Control Weather!",
	/*	 12 SPELL_CREATE_FOOD			*/	"!Create Food!",
	/*	 13 SPELL_CREATE_WATER			*/	"!Create Water!",
	/*	 14 SPELL_CURE_BLIND			*/	"!Cure Blind!",
	/*	 15 SPELL_CURE_CRITIC			*/	"!Cure Critic!",
	/*	 16 SPELL_CURE_LIGHT			*/	"!Cure Light!",
	/*	 17 SPELL_CURSE					*/	"",
	/*	 18 SPELL_DETECT_EVIL			*/	"",
	/*	 19 SPELL_DETECT_INVISIBLE		*/	"",
	/*	 20 SPELL_DETECT_MAGIC			*/	"",
	/*	 21 SPELL_DETECT_POISON			*/	"",
	/*	 22 SPELL_DISPEL_EVIL			*/	"!Dispel Evil!",
	/*	 23 SPELL_EARTHQUAKE			*/	"!Earthquake!",
	/*	 24 SPELL_ENCHANT_WEAPON		*/	"!Enchant Weapon!",
	/*	 25 SPELL_ENERGY_DRAIN			*/	"!Energy Drain!",
	/*	 26 SPELL_FIREBALL				*/	"!Fireball!",
	/*	 27 SPELL_HARM					*/	"!Harm!",
	/*	 28 SPELL_HEAL					*/	"!Heal!",
	/*	 29 SPELL_INVISIBLE				*/	"Torni visibile per un momento, poi scompari di nuovo.",
	/*	 30 SPELL_LIGHTNING_BOLT		*/	"!Lightning Bolt!",
	/*	 31 SPELL_LOCATE_OBJECT			*/	"!Locate object!",
	/*	 32 SPELL_MAGIC_MISSILE			*/	"!Magic Missile!",
	/*	 33 SPELL_POISON				*/	"Ti sembra di stare meglio, gli effetti del $c0010veleno$c0007 sono meno forti ora.",
	/*	 34 SPELL_PROTECT_FROM_EVIL		*/	"",
	/*	 35 SPELL_REMOVE_CURSE			*/	"!Remove Curse!",
	/*	 36 SPELL_SANCTUARY				*/	"L'$c0015aura bianca$c0007 che ti circonda inizia a diventare meno intensa.",
	/*	 37 SPELL_SHOCKING_GRASP		*/	"!Shocking Grasp!",
	/*	 38 SPELL_SLEEP					*/	"",
	/*	 39 SPELL_STRENGTH				*/	"Ti senti un po' piu' $c0014debole$c0007.",
	/*	 40 SPELL_SUMMON				*/	"!Summon!",
	/*	 41 SPELL_VENTRILOQUATE			*/	"!Ventriloquate!",
	/*	 42 SPELL_WORD_OF_RECALL		*/	"!Word of Recall!",
	/*	 43 SPELL_REMOVE_POISON			*/	"!Remove Poison!",
	/*	 44 SPELL_SENSE_LIFE			*/	"",
	/*	 45 SKILL_SNEAK					*/	"",  /* NO MESSAGE FOR SNEAK*/
	/*	 46 SKILL_HIDE					*/	"!Hide!",
	/*	 47 SKILL_STEAL					*/	"!Steal!",
	/*	 48 SKILL_BACKSTAB				*/	"!Backstab!",
	/*	 49 SKILL_PICK_LOCK				*/	"!Pick Lock!",
	/*	 50 SKILL_KICK					*/	"!Kick!",
	/*	 51 SKILL_BASH					*/	"!Bash!",
	/*	 52 SKILL_RESCUE				*/	"!Rescue!",
	/*	 53 SPELL_IDENTIFY				*/	"!Identify!",
	/*	 54 SPELL_INFRAVISION			*/	"Cominci a perdere la tua capacita' di $c0009infravisione$c0007.",
	/*	 55 SPELL_CAUSE_LIGHT			*/	"!cause light!",
	/*	 56 SPELL_CAUSE_CRITICAL		*/	"!cause crit!",
	/*	 57 SPELL_FLAMESTRIKE			*/	"!flamestrike!",
	/*	 58 SPELL_DISPEL_GOOD			*/	"!dispel good!",
	/*	 59 SPELL_WEAKNESS				*/	"Ti sembra di sentirti un po' piu' $c0009forte$c0007.",
	/*	 60 SPELL_DISPEL_MAGIC			*/	"!dispel magic!",
	/*	 61 SPELL_KNOCK					*/	"!knock!",
	/*	 62 SPELL_KNOW_ALIGNMENT        */	"!know alignment!",
	/*	 63 SPELL_ANIMATE_DEAD          */	"!animate dead!",
	/*	 64 SPELL_PARALYSIS             */	"",
	/*	 65 SPELL_REMOVE_PARALYSIS      */	"!remove paralysis!",
	/*	 66 SPELL_FEAR                  */	"!fear!",
	/*	 67 SPELL_ACID_BLAST            */	"!acid blast!",
	/*	 68 SPELL_WATER_BREATH          */	"Per un attimo fatichi a respirare, stai perdendo la capacita' di respirare sott'$c0012acqua$c0007.",
	/*	 69 SPELL_FLY					*/	"Ti senti un po' piu' pesante, stai perdendo la capacita' di volare.",
	/*	 70 SPELL_CONE_OF_COLD          */	"!cone of cold!",
	/*	 71 SPELL_METEOR_SWARM          */	"!meteor swarm!",
	/*	 72 SPELL_ICE_STORM             */	"!ice storm!",
	/*	 73 SPELL_SHIELD                */	"Il tuo $c0011scudo$c0007 magico tremola per un attimo.",
	/*	 74 SPELL_MON_SUM_1             */	"!monsum one!",
	/*	 75 SPELL_MON_SUM_2             */	"!monsum two!",
	/*	 76 SPELL_MON_SUM_3             */	"!monsum three!",
	/*	 77 SPELL_MON_SUM_4             */	"!monsum four!",
	/*	 78 SPELL_MON_SUM_5             */	"!monsum five!",
	/*	 79 SPELL_MON_SUM_6             */	"!monsum six!",
	/*	 80 SPELL_MON_SUM_7             */	"!monsum seven!",
	/*	 81 SPELL_FIRESHIELD            */	"Lo scudo di $c0009fuoco$c0007 che ti avvolge proteggendoti perde di intensita'.",
	/*	 82 SPELL_CHARM_MONSTER         */	"Ti sembra di tornare in te.",
	/*	 83 SPELL_CURE_SERIOUS          */	"!cure serious!",
	/*	 84 SPELL_CAUSE_SERIOUS         */	"!cause serious!",
	/*	 85 SPELL_REFRESH               */	"!refresh!",
	/*	 86 SPELL_SECOND_WIND           */	"!second wind!",
	/*	 87 SPELL_TURN                  */	"!turn!",
	/*	 88 SPELL_SUCCOR                */	"!succor!",
	/*	 89 SPELL_LIGHT                 */	"!light!",
	/*	 90 SPELL_CONT_LIGHT            */	"!continual light!",
	/*	 91 SPELL_CALM                  */	"!calm!",
	/*	 92 SPELL_STONE_SKIN            */	"La tua pelle inizia a perdere la consistenza del $c0003granito$c0007.",
	/*	 93 SPELL_CONJURE_ELEMENTAL     */	"!conjure elemental!",
	/*	 94 SPELL_TRUE_SIGHT            */	"I tuoi occhi $c0015brillano$c0007 per un attimo.",
	/*	 95 SPELL_MINOR_CREATE          */	"!minor creation!",
	/*	 96 SPELL_FAERIE_FIRE           */	"L'$c0013alone rosa$c0007 che ti circonda comincia a svanire.",
	/*	 97 SPELL_FAERIE_FOG            */	"!faerie fog!",
	/*	 98 SPELL_CACAODEMON            */	"!cacaodemon!",
	/*	 99 SPELL_POLY_SELF             */	"Perdi per un attimo il controllo del tuo aspetto.",
	/*	100 SPELL_MANA                  */	"La $c0011protezione magica$c0007 attorno al tuo corpo scompare per un attimo.",
	/*	101 SPELL_ASTRAL_WALK           */	"!astral walk!",
	/*	102 SPELL_RESURRECTION          */	"!resurrection!",
	/*	103 SPELL_H_FEAST               */	"!heroes feast!",
	/*	104 SPELL_FLY_GROUP             */	"Hai per un attimo la sensazione di cadere nel $c0008vuoto$c0007.",
	/*	105 SPELL_DRAGON_BREATH         */	"!dragon breath!",
	/*	106 SPELL_WEB					*/	"Le $c0008ragnatele$c0007 che ti avvolgono si stanno indebolendo.",
	/*	107 SPELL_MINOR_TRACK           */	"Inizi a sentirti confus$b.",
	/*	108 SPELL_MAJOR_TRACK           */	"Inizi a sentirti confus$b.",
	/*	109 SPELL_GOLEM                 */	"!golem!",
	/*	110 SPELL_FAMILIAR              */	"",
	/*	111 SPELL_CHANGESTAFF           */	"!changestaff!",
	/*	112 SPELL_HOLY_WORD             */	"!holy word!",
	/*	113 SPELL_UNHOLY_WORD           */	"!unholy word!",
	/*	114 SPELL_PWORD_KILL            */	"!pwk!",
	/*	115 SPELL_PWORD_BLIND           */	"!pwb!",
	/*	116 SPELL_CHAIN_LIGHTNING       */	"!chain lightning!",
	/*	117 SPELL_SCARE                 */	"!scare!",
	/*	118 SPELL_AID					*/	"Inizi a perdere l'$c0015aiuto divino$c0007.",
	/*	119 SPELL_COMMAND               */	"!command!",
	/*	120 SPELL_CHANGE_FORM           */	"$c0010Perdi per un attimo il controllo del tuo aspetto.",
	/*	121 SPELL_FEEBLEMIND            */	"",
	/*	122 SPELL_SHILLELAGH            */	"!shillelagh!",
	/*	123 SPELL_GOODBERRY             */	"!goodberry!",
	/*	124 SPELL_ELEMENTAL_BLADE       */	"!elemental blade!",
	/*	125 SPELL_ANIMAL_GROWTH         */	"Hai come la sensazione di rimpicciolire.",
	/*	126 SPELL_INSECT_GROWTH         */	"Hai come la sensazione di rimpicciolire.",
	/*	127 SPELL_CREEPING_DEATH        */	"",
	/*	128 SPELL_COMMUNE               */	"!commune!",
	/*	129 SPELL_ANIMAL_SUM_1          */	"Ti senti meno in contatto con gli $c0003animali$c0007.",
	/*	130 SPELL_ANIMAL_SUM_2          */	"Ti senti meno in contatto con gli $c0003animali$c0007.",
	/*	131 SPELL_ANIMAL_SUM_3          */	"Ti senti meno in contatto con gli $c0003animali$c0007.",
	/*	132 SPELL_FIRE_SERVANT          */	"",
	/*	133 SPELL_EARTH_SERVANT         */	"",
	/*	134 SPELL_WATER_SERVANT         */	"",
	/*	135 SPELL_WIND_SERVANT          */	"",
	/*	136 SPELL_REINCARNATE           */	"!reincarnate!",
	/*	137 SPELL_CHARM_VEGGIE          */	"Ti sembra di tornare in te.",
	/*	138 SPELL_VEGGIE_GROWTH         */	"Hai come la sensazione di rimpicciolire.",
	/*	139 SPELL_TREE                  */	"Perdi per un attimo il controllo del tuo aspetto.",
	/*	140 SPELL_ANIMATE_ROCK          */	"!Animate rock!",
	/*	141 SPELL_TREE_TRAVEL           */	"Ti senti meno in contatto con gli $c0003alberi$c0007.",
	/*	142 SPELL_TRAVELLING            */	"Ti senti meno $c0010agile$c0007 per un attimo.",
	/*	143 SPELL_ANIMAL_FRIENDSHIP     */	"",
	/*	144 SPELL_INVIS_TO_ANIMALS      */	"Inizi a tornare visibile agli $c0003animali$c0007.",
	/*	145 SPELL_SLOW_POISON           */	"La tua resistenza al $c0010veleno$c0007 diminuisce.",
	/*	146 SPELL_ENTANGLE              */	"Senti i tuoi muscoli meno intorpiditi.",
	/*	147 SPELL_SNARE                 */	"Sei quasi riuscit$b a liberarti dalla trappola!",
	/*	148 SPELL_GUST_OF_WIND          */	"!gust of wind!",
	/*	149 SPELL_BARKSKIN              */	"La tua pelle sta perdendo la consistenza della $c0003corteccia$c0007.",
	/*	150 SPELL_SUNRAY                */	"!sunray!",
	/*	151 SPELL_WARP_WEAPON           */	"!warp weapon!",
	/*	152 SPELL_HEAT_STUFF            */	"La tua armatura si $c0014raffredda$c0007 un po'.",
	/*	153 SPELL_FIND_TRAPS            */	"Stai perdendo la tua capacita' di scovare le $c0008trappole$c0007.",
	/*	154 SPELL_FIRESTORM             */	"!firestorm!",
	/*	155 SPELL_HASTE                 */	"$c0008Sembra che il mondo stia diventando piu' veloce.",
	/*	156 SPELL_SLOW                  */	"$c0015Sembra che il mondo stia rallentando.",
	/*	157 SPELL_DUST_DEVIL            */	"",
	/*	158 SPELL_KNOW_MONSTER          */	"!know monster!",
	/*	159 SPELL_TRANSPORT_VIA_PLANT   */	"!transport via plant!",
	/*	160 SPELL_SPEAK_WITH_PLANT      */	"!speak with plant!",
	/*	161 SPELL_SILENCE               */	"",
	/*	162 SPELL_SENDING               */	"!sending!",
	/*	163 SPELL_TELEPORT_WO_ERROR     */	"!teleport without error!",
	/*	164 SPELL_PORTAL                */	"!portal!",
	/*	165 SPELL_DRAGON_RIDE           */	"",
	/*	166 SPELL_MOUNT                 */	"!mount!",
	/*	167 SPELL_NO_MESSAGE            */	"",
	/*	168 to_do                       */	"!168!",
	/*	169 SKILL_MANTRA                */	"Fatichi a mantenere la concentrazione.",
	/*	170 SKILL_FIRST_AID             */	"",
	/*	171 SKILL_SIGN                  */	"!sign!",
	/*	172 SKILL_RIDE                  */	"!riding!",
	/*	173 SKILL_SWITCH_OPP            */	"!switch!",
	/*	174 SKILL_DODGE                 */	"!dodge!",
	/*	175 SKILL_REMOVE_TRAP           */	"!remove trap!",
	/*	176 SKILL_RETREAT               */	"!retreat!",
	/*	177 SKILL_QUIV_PALM             */	"Una leggera $c0015vibrazione$c0007 pervade le tue mani.",
	/*	178 SKILL_SAFE_FALL             */	"!safe fall!",
	/*	179 SKILL_FEIGN_DEATH           */	"!feign death!",
	/*	180 SKILL_HUNT                  */	"!hunt!",
	/*	181 SKILL_FIND_TRAP             */	"!find traps!",
	/*	182 SKILL_SPRING_LEAP           */	"!spring leap!",
	/*	183 SKILL_DISARM                */	"!disarm!",
	/*	184 SKILL_READ_MAGIC            */	"!read magic!",
	/*	185 SKILL_EVALUATE              */	"!evalutate!",
	/*	186 SKILL_SPY					*/	"",
	/*	187 SKILL_DOORBASH              */	"!doorbash!",
	/*	188 SKILL_SWIM                  */	"",
	/*	189 SKILL_CONS_UNDEAD           */	"!consider undead!",
	/*	190 SKILL_CONS_VEGGIE           */	"!consider veggie!",
	/*	191 SKILL_CONS_DEMON            */	"!consider demon!",
	/*	192 SKILL_CONS_ANIMAL           */	"!consider animal!",
	/*	193 SKILL_CONS_REPTILE          */	"!consider reptile!",
	/*	194 SKILL_CONS_PEOPLE           */	"!consider people!",
	/*	195 SKILL_CONS_GIANT            */	"!consider giant!",
	/*	196 SKILL_CONS_OTHER            */	"!consider other!",
	/*	197 SKILL_DISGUISE              */	"",
	/*	198 SKILL_CLIMB                 */	"!climb!",
	/*	199 SKILL_FINGER                */	"!finger!",
	/*	200 SPELL_GEYSER                */	"!geyser!",
	/*	201 SPELL_MIRROR_IMAGES         */	"La tua $c0012immagine illusoria$c0007 vacilla per un attimo.",
	/*	202 SKILL_TSPY                  */	"Le conversazioni della stanza si perdono in un mormorio indistinto.",
	/*	203 SKILL_EAVESDROP             */	"!eavesdrop!",
	/*	204 SKILL_PARRY                 */	"!parry!",
	/*	205 SKILL_MINER                 */	"!miner!",
	/*	206 SPELL_GREEN_SLIME           */	"!green slime!",
	/*	207 SKILL_BERSERK               */	"!berserk!",
	/*	208 SKILL_TAN					*/	"!tan!",
	/*	209 SKILL_AVOID_BACK_ATTACK     */	"!avoid backattack!",
	/*	210 SKILL_FIND_FOOD             */	"!find food!",
	/*	211 SKILL_FIND_WATER            */	"!find water!",
	/*	212 SPELL_PRAYER                */	"",
	/*	213 SKILL_MEMORIZE              */	"Scorri le ultime pagine dell'incantesimo.",
	/*	214 SKILL_BELLOW                */	"!bellow!",
	/*	215 SPELL_GLOBE_DARKNESS        */	"Senti svanire l'$c0008oscurita'$c0007 che ti avvolge.",
	/*	216 SPELL_GLOBE_MINOR_INV       */	"Il $c0012globo$c0007 di protezione attorno a te emette un $c0015bagliore$c0007.",
	/*	217 SPELL_GLOBE_MAJOR_INV       */	"Il $c0014globo$c0007 di protezione attorno a te tremola per un secondo.",
	/*	218 SPELL_PROT_ENERGY_DRAIN     */	"Inizi a temere il tocco dei $c0008Non-Morti$c0007.",
	/*	219 SPELL_PROT_DRAGON_BREATH    */	"Inizi a temere il soffio dei $c0003draghi$c0007.",
	/*	220 SPELL_ANTI_MAGIC_SHELL      */	"Lo scudo $c0012anti-magia$c0007 intorno a te emette qualche $c0011scintilla$c0007.",
	/*	221 SKILL_DOORWAY               */	"!doorway!",
	/*	222 SKILL_PORTAL                */	"!psi_portal!",
	/*	223 SKILL_SUMMON                */	"!psi_summon!",
	/*	224 SKILL_INVIS                 */	"!psi_invis!",
	/*	225 SKILL_CANIBALIZE            */	"!canibalize!",
	/*	226 SKILL_FLAME_SHROUD          */	"!flame shroud!",
	/*	227 SKILL_AURA_SIGHT            */	"!aura sight!",
	/*	228 SKILL_GREAT_SIGHT           */	"!great sight!",
	/*	229 SKILL_PSIONIC_BLAST         */	"Cominci a riprenderti dal colpo di $c0011energia$c0007 psionica.",
	/*	230 SKILL_HYPNOSIS              */	"!hypnosis!",
	/*	231 SKILL_MEDITATE              */	"",
	/*	232 SKILL_SCRY                  */	"!scry!",
	/*	233 SKILL_ADRENALIZE            */	"Lentamente ti sembra di prendere il controllo.",
	/*	234 SKILL_BREW                  */	"!brew!",
	/*	235 SKILL_RATION                */	"!ration!",
	/*	236 SKILL_HOLY_WARCRY           */	"!holy warcry!",
	/*	237 SKILL_BLESSING              */	"$c0015Senti che i tuoi Dei ti stanno tornando a dare il loro favore.",
	/*	238 SKILL_LAY_ON_HANDS          */	"$c0015Ti senti quasi pront$b.",
	/*	239 SKILL_HEROIC_RESCUE         */	"!heroic rescue!",
	/*	240 SKILL_DUAL_WIELD            */	"!dual wield!",
	/*	241 SKILL_PSI_SHIELD            */	"Il tuo $c0011scudo$c0007 psionico tremola per un attimo.",
	/*	242 SPELL_PROT_FROM_EVIL_GROUP  */	"La tua protezione dal $c0009Male$c0007 sparisce per un momento.",
	/*	243 SPELL_PRISMATIC_SPRAY       */	"!prismatic spry!",
	/*	244 SPELL_INCENDIARY_CLOUD      */	"!incendiary cloud!",
	/*	245 SPELL_DISINTEGRATE          */	"!disintegrate!",
	/*	246 LANG_COMMON                 */	"!lang common!",
	/*	247 LANG_ELVISH                 */	"!lang elvish!",
	/*	248 LANG_HALFLING               */	"!lang halfling!",
	/*	249 LANG_DWARVISH               */	"!lang dwarvish!",
	/*	250 LANG_ORCISH                 */	"!lang orcish!",
	/*	251 LANG_GIANTISH               */	"!lang giantish!",
	/*	252 LANG_OGRE					*/	"!lang ogre!",
	/*	253 LANG_GNOMISH                */	"!lang gnomish!",
	/*	254 SKILL_ESP					*/	"Cominci a perdere la capacita' di leggere il pensiero.",
	/*	255 SPELL_COMP_LANGUAGES        */	"Cominci a perdere la capacita' di comprendere i linguaggi.",
	/*	256 SPELL_PROT_FIRE             */	"La tua protezione dal $c0009fuoco$c0007 si indebolisce un attimo.",
	/*	257 SPELL_PROT_COLD             */	"La tua protezione dal $c0014freddo$c0007 si indebolisce un attimo.",
	/*	258 SPELL_PROT_ENERGY           */	"La tua protezione dall'$c0011energia$c0007 si indebolisce un attimo.",
	/*	259 SPELL_PROT_ELEC             */	"La tua protezione dall'$c0012elettricita'$c0007 si indebolisce per un attimo.",
	/*	260 SPELL_ENCHANT_ARMOR         */	"!enchant armor!",
	/*	261 SPELL_MESSENGER             */	"!messenger!",
	/*	262 SPELL_PROT_BREATH_FIRE      */	"Il tuo globo di protezione dal soffio $c0009infuocato$c0007 emette un $c0015bagliore$c0007 ed inizia a $c5009pulsare$c0007.",
	/*	263 SPELL_PROT_BREATH_FROST     */	"Il tuo globo di protezione dal soffio $c0014gelido$c0007 emette un $c0015bagliore$c0007 ed inizia a $c5014pulsare$c0007.",
	/*	264 SPELL_PROT_BREATH_ELEC      */	"Il tuo globo di protezione dal soffio $c0012elettrico$c0007 emette un $c0015bagliore$c0007 ed inizia a $c5012pulsare$c0007.",
	/*	265 SPELL_PROT_BREATH_ACID      */	"Il tuo globo di protezione dal soffio $c0010acido$c0007 emette un $c0015bagliore$c0007 ed inizia a $c5010pulsare$c0007.",
	/*	266 SPELL_PROT_BREATH_GAS       */	"Il tuo globo di protezione dal soffio $c0011gassoso$c0007 emette un $c0015bagliore$c0007 ed inizia a $c5011pulsare$c0007.",
	/*	267 SPELL_WIZARDEYE             */	"Perdi per un momento il controllo del tuo occhio magico.",
	/*	268 SKILL_MIND_BURN             */	"!mind burn!",
	/*	269 SKILL_CLAIRVOYANCE          */	"Ti sembra di perdere la concentrazione della mente sui luoghi che ti circondano.",
	/*	270 SKILL_DANGER_SENSE          */	"Inizi a perdere la percezione dei $c0009pericoli$c0007 attorno a te.",
	/*	271 SKILL_DISINTEGRATE          */	"!psi_disint!",
	/*	272 SKILL_TELEKINESIS           */	"!telekinesis!",
	/*	273 SKILL_LEVITATION            */	"Per un attimo hai la sensazione di cadere.",
	/*	274 SKILL_CELL_ADJUSTMENT       */	"!cell adjustment!",
	/*	275 SKILL_CHAMELEON             */	"!chameleon!",
	/*	276 SKILL_PSI_STRENGTH          */	"Perdi il controllo dalla tua mente e ti senti piu' $c0014debole$c0007, ma la sensazione passa subito.",
	/*	277 SKILL_MIND_OVER_BODY        */	"",
	/*	278 SKILL_PROBABILITY_TRAVEL    */	"!probability travel!",
	/*	279 SKILL_PSI_TELEPORT          */	"!psi teleport!",
	/*	280 SKILL_DOMINATION            */	"!domination!",
	/*	281 SKILL_MIND_WIPE             */	"!mind wipe!",
	/*	282 SKILL_PSYCHIC_CRUSH         */	"!psychic crush!",
	/*	283 SKILL_TOWER_IRON_WILL       */	"$c0012La tua volonta' vacilla.",
	/*	284 SKILL_MINDBLANK             */	"$c0008Per un attimo ti sembra di non riuscire piu' a tenere sgombra la mente.",
	/*	285 SKILL_PSYCHIC_IMPERSONATION */	"",
	/*	286 SKILL_ULTRA_BLAST           */	"!ultra blast!",
	/*	287 SKILL_INTENSIFY             */	"La tua mente inizia a rallentare...",
	/*	288 SKILL_SPOT                  */	"!spot!",
	/*	289 SKILL_IMMOLATION            */	"!immolation!",
	/*	290 STATUS_QUEST                */	"$c5009Il Tempo per la tua missione e' agli sgoccioli!",
	/*	291 SKILL_DAIMOKU               */	"",
	/*	292 SKILL_FORGE                 */	"!forge!",
	/*	293 SKILL_DETERMINE             */	"!determine!",
	/*	294 SKILL_EQUILIBRIUM           */	"!equilibrium!",
											"\n"
};

const char* spell_wear_off_soon_room_msg[] = {
											"RESERVED DB.C",
	/*	  1 SPELL_ARMOR                 */	"",
	/*	  2 SPELL_TELEPORT              */	"!Teleport!",
	/*	  3 SPELL_BLESS                 */	"",
	/*	  4 SPELL_BLINDNESS             */	"",
	/*	  5 SPELL_BURNING_HANDS         */	"!Burning Hands!",
	/*	  6 SPELL_CALL_LIGHTNING        */	"!Call Lightning",
	/*	  7 SPELL_CHARM_PERSON          */	"$n sembra tornare in controllo di se.",
	/*	  8 SPELL_CHILL_TOUCH           */	"$n sta riacquisendo le sue $c0009forze$c0007.",
	/*	  9 SPELL_CLONE                 */	"!Clone!",
	/*	 10 SPELL_COLOUR_SPRAY          */	"!Color Spray!",
	/*	 11 SPELL_CONTROL_WEATHER       */	"!Control Weather!",
	/*	 12 SPELL_CREATE_FOOD           */	"!Create Food!",
	/*	 13 SPELL_CREATE_WATER          */	"!Create Water!",
	/*	 14 SPELL_CURE_BLIND            */	"!Cure Blind!",
	/*	 15 SPELL_CURE_CRITIC           */	"!Cure Critic!",
	/*	 16 SPELL_CURE_LIGHT            */	"!Cure Light!",
	/*	 17 SPELL_CURSE                 */	"",
	/*	 18 SPELL_DETECT_EVIL           */	"L'$c0009alone rosso$c0007 attorno agli occhi di $n si affievolisce.",
	/*	 19 SPELL_DETECT_INVISIBLE      */	"L'$c0011alone giallo$c0007 attorno agli occhi di $n diventa $c0008opaco$c0007.",
	/*	 20 SPELL_DETECT_MAGIC          */	"L'$c0012alone blu$c0007 attorno agli $n diventa meno brillante.",
	/*	 21 SPELL_DETECT_POISON         */	"Per un attimo gli occhi di $n perdono l'$c0015alone bianco$c0007",
	/*	 22 SPELL_DISPEL_EVIL           */	"!Dispel Evil!",
	/*	 23 SPELL_EARTHQUAKE            */	"!Earthquake!",
	/*	 24 SPELL_ENCHANT_WEAPON        */	"!Enchant Weapon!",
	/*	 25 SPELL_ENERGY_DRAIN          */	"!Energy Drain!",
	/*	 26 SPELL_FIREBALL              */	"!Fireball!",
	/*	 27 SPELL_HARM                  */	"!Harm!",
	/*	 28 SPELL_HEAL                  */	"!Heal!",
	/*	 29 SPELL_INVISIBLE             */	"$n torna visibile per un attimo, poi scompare di nuovo.",
	/*	 30 SPELL_LIGHTNING_BOLT        */	"!Lightning Bolt!",
	/*	 31 SPELL_LOCATE_OBJECT         */	"!Locate object!",
	/*	 32 SPELL_MAGIC_MISSILE         */	"!Magic Missile!",
	/*	 33 SPELL_POISON                */	"",
	/*	 34 SPELL_PROTECT_FROM_EVIL     */	"",
	/*	 35 SPELL_REMOVE_CURSE          */	"!Remove Curse!",
	/*	 36 SPELL_SANCTUARY             */	"L'$c0015aura bianca$c0007 intorno al corpo di $n perde di intensita'.",
	/*	 37 SPELL_SHOCKING_GRASP        */	"!Shocking Grasp!",
	/*	 38 SPELL_SLEEP                 */	"$n sembra quasi svegli$b.",
	/*	 39 SPELL_STRENGTH              */	"$n sembra quasi piu' $c0014debole$c0007.",
	/*	 40 SPELL_SUMMON                */	"!Summon!",
	/*	 41 SPELL_VENTRILOQUATE         */	"!Ventriloquate!",
	/*	 42 SPELL_WORD_OF_RECALL        */	"!Word of Recall!",
	/*	 43 SPELL_REMOVE_POISON         */	"!Remove Poison!",
	/*	 44 SPELL_SENSE_LIFE            */	"",
	/*	 45 SKILL_SNEAK                 */	"",  /* NO MESSAGE FOR SNEAK*/
	/*	 46 SKILL_HIDE                  */	"!Hide!",
	/*	 47 SKILL_STEAL                 */	"!Steal!",
	/*	 48 SKILL_BACKSTAB              */	"!Backstab!",
	/*	 49 SKILL_PICK_LOCK             */	"!Pick Lock!",
	/*	 50 SKILL_KICK                  */	"!Kick!",
	/*	 51 SKILL_BASH                  */	"!Bash!",
	/*	 52 SKILL_RESCUE                */	"!Rescue!",
	/*	 53 SPELL_IDENTIFY              */	"!Identify!",
	/*	 54 SPELL_INFRAVISION           */	"",
	/*	 55 SPELL_CAUSE_LIGHT           */	"!cause light!",
	/*	 56 SPELL_CAUSE_CRITICAL        */	"!cause crit!",
	/*	 57 SPELL_FLAMESTRIKE           */	"!flamestrike!",
	/*	 58 SPELL_DISPEL_GOOD           */	"!dispel magic!",
	/*	 59 SPELL_WEAKNESS              */	"Gli occhi di $n non sono piu' cosi' $c0009rossi$c0007.",
	/*	 60 SPELL_DISPEL_MAGIC          */	"!dispel good!",
	/*	 61 SPELL_KNOCK                 */	"!knock!",
	/*	 62 SPELL_KNOW_ALIGNMENT        */	"!know alignment!",
	/*	 63 SPELL_ANIMATE_DEAD          */	"!animate dead!",
	/*	 64 SPELL_PARALYSIS             */	"Ti e' sembrato che $n si sia moss$b.",
	/*	 65 SPELL_REMOVE_PARALYSIS      */	"!remove paralysis!",
	/*	 66 SPELL_FEAR                  */	"!fear!",
	/*	 67 SPELL_ACID_BLAST            */	"!acid blast!",
	/*	 68 SPELL_WATER_BREATH          */	"$n respira affannosamente per un attimo.",
	/*	 69 SPELL_FLY					*/	"La magia che sorreggeva $n inizia a svanire.",
	/*	 70 SPELL_CONE_OF_COLD          */	"!cone of cold!",
	/*	 71 SPELL_METEOR_SWARM          */	"!meteor swarm!",
	/*	 72 SPELL_ICE_STORM             */	"!ice storm!",
	/*	 73 SPELL_SHIELD                */	"Lo $c0011scudo$c0007 di $n tremola leggermente.",
	/*	 74 SPELL_MON_SUM_1             */	"!monsum one!",
	/*	 75 SPELL_MON_SUM_2             */	"!monsum two!",
	/*	 76 SPELL_MON_SUM_3             */	"!monsum three!",
	/*	 77 SPELL_MON_SUM_4             */	"!monsum four!",
	/*	 78 SPELL_MON_SUM_5             */	"!monsum five!",
	/*	 79 SPELL_MON_SUM_6             */	"!monsum six!",
	/*	 80 SPELL_MON_SUM_7             */	"!monsum seven!",
	/*	 81 SPELL_FIRESHIELD            */	"Lo scudo di $c0009fuoco$c0007 che avvolge il corpo di $n diminuisce di intensita'.",
	/*	 82 SPELL_CHARM_MONSTER         */	"$n sembra tornare in controllo di se.",
	/*	 83 SPELL_CURE_SERIOUS          */	"!cure serious!",
	/*	 84 SPELL_CAUSE_SERIOUS         */	"!cause serious!",
	/*	 85 SPELL_REFRESH               */	"!refresh!",
	/*	 86 SPELL_SECOND_WIND           */	"!second wind!",
	/*	 87 SPELL_TURN                  */	"!turn!",
	/*	 88 SPELL_SUCCOR                */	"!succor!",
	/*	 89 SPELL_LIGHT                 */	"!light!",
	/*	 90 SPELL_CONT_LIGHT            */	"!continual light!",
	/*	 91 SPELL_CALM                  */	"!calm!",
	/*	 92 SPELL_STONE_SKIN            */	"",
	/*	 93 SPELL_CONJURE_ELEMENTAL     */	"!conjure elemental!",
	/*	 94 SPELL_TRUE_SIGHT            */	"",
	/*	 95 SPELL_MINOR_CREATE          */	"!minor creation!",
	/*	 96 SPELL_FAERIE_FIRE           */	"L'$c0013alone rosa$c0007 attorno al corpo di $n tremola per un attimo.",
	/*	 97 SPELL_FAERIE_FOG            */	"!faerie fog!",
	/*	 98 SPELL_CACAODEMON            */	"!cacaodemon!",
	/*	 99 SPELL_POLY_SELF             */	"$n per un attimo riprende il suo aspetto normale.",
	/*	100 SPELL_MANA                  */	"L'$c0011alone protettivo$c0007 attorno al corpo di $n $c5007pulsa$c0007.",
	/*	101 SPELL_ASTRAL_WALK           */	"!astral walk!",
	/*	102 SPELL_RESURRECTION          */	"!resurrection!",
	/*	103 SPELL_H_FEAST               */	"!heroes feast!",
	/*	104 SPELL_FLY_GROUP             */	"$n pare quasi cadere nel vuoto, poi si riprende.",
	/*	105 SPELL_DRAGON_BREATH         */	"!dragon breath!",
	/*	106 SPELL_WEB					*/	"Le $c0008ragnatele$c0007 che imprigionano $n sembrano allentarsi.",
	/*	107 SPELL_MINOR_TRACK           */	"",
	/*	108 SPELL_MAJOR_TRACK           */	"",
	/*	109 SPELL_GOLEM                 */	"!golem!",
	/*	110 SPELL_FAMILIAR              */	"",
	/*	111 SPELL_CHANGESTAFF           */	"!changestaff!",
	/*	112 SPELL_HOLY_WORD             */	"!holy word!",
	/*	113 SPELL_UNHOLY_WORD           */	"!unholy word!",
	/*	114 SPELL_PWORD_KILL            */	"!pwk!",
	/*	115 SPELL_PWORD_BLIND           */	"!pwb!",
	/*	116 SPELL_CHAIN_LIGHTNING       */	"!chain lightning!",
	/*	117 SPELL_SCARE                 */	"!scare!",
	/*	118 SPELL_AID					*/	"",
	/*	119 SPELL_COMMAND               */	"!command!",
	/*	120 SPELL_CHANGE_FORM           */	"$c0010$n$c0010 per un attimo riprende il suo aspetto normale.",
	/*	121 SPELL_FEEBLEMIND            */	"$n inizia a sembrarti meno stupid$b.",
	/*	122 SPELL_SHILLELAGH            */	"!shillelagh!",
	/*	123 SPELL_GOODBERRY             */	"!goodberry!",
	/*	124 SPELL_ELEMENTAL_BLADE       */	"!elemental blade!",
	/*	125 SPELL_ANIMAL_GROWTH         */	"Ti sembra che $n stia diventando piu' piccol$b.",
	/*	126 SPELL_INSECT_GROWTH         */	"Ti sembra che $n stia diventando piu' piccol$b.",
	/*	127 SPELL_CREEPING_DEATH        */	"",
	/*	128 SPELL_COMMUNE               */	"!commune!",
	/*	129 SPELL_ANIMAL_SUM_1          */	"",
	/*	130 SPELL_ANIMAL_SUM_2          */	"",
	/*	131 SPELL_ANIMAL_SUM_3          */	"",
	/*	132 SPELL_FIRE_SERVANT          */	"",
	/*	133 SPELL_EARTH_SERVANT         */	"",
	/*	134 SPELL_WATER_SERVANT         */	"",
	/*	135 SPELL_WIND_SERVANT          */	"",
	/*	136 SPELL_REINCARNATE           */	"!reincarnate!",
	/*	137 SPELL_CHARM_VEGGIE          */	"$n sembra tornare in controllo di se.",
	/*	138 SPELL_VEGGIE_GROWTH         */	"Ti sembra che $n stia diventando piu' piccol$b.",
	/*	139 SPELL_TREE                  */	"$n per un attimo riprende il suo aspetto normale.",
	/*	140 SPELL_ANIMATE_ROCK          */	"!Animate rock!",
	/*	141 SPELL_TREE_TRAVEL           */	"",
	/*	142 SPELL_TRAVELLING            */	"",
	/*	143 SPELL_ANIMAL_FRIENDSHIP     */	"",
	/*	144 SPELL_INVIS_TO_ANIMALS      */	"",
	/*	145 SPELL_SLOW_POISON           */	"",
	/*	146 SPELL_ENTANGLE              */	"$n e' ancora bloccat$b da $c0010foglie$c0007 ed $c0003arbusti$c0007.",
	/*	147 SPELL_SNARE                 */	"$n e' ancora intrappolat$b!",
	/*	148 SPELL_GUST_OF_WIND          */	"!gust of wind!",
	/*	149 SPELL_BARKSKIN              */	"La pelle di $n comincia a perdere il suo aspetto di $c0003corteccia$c0007.",
	/*	150 SPELL_SUNRAY                */	"!sunray!",
	/*	151 SPELL_WARP_WEAPON           */	"!warp weapon!",
	/*	152 SPELL_HEAT_STUFF            */	"Sembra che l'armatura di $n si stia $c0014raffreddando$c0007.",
	/*	153 SPELL_FIND_TRAPS            */	"",
	/*	154 SPELL_FIRESTORM             */	"!firestorm!",
	/*	155 SPELL_HASTE                 */	"",
	/*	156 SPELL_SLOW                  */	"",
	/*	157 SPELL_DUST_DEVIL            */	"",
	/*	158 SPELL_KNOW_MONSTER          */	"!know monster!",
	/*	159 SPELL_TRANSPORT_VIA_PLANT   */	"!transport via plant!",
	/*	160 SPELL_SPEAK_WITH_PLANT      */	"!speak with plant!",
	/*	161 SPELL_SILENCE               */	"",
	/*	162 SPELL_SENDING               */	"!sending!",
	/*	163 SPELL_TELEPORT_WO_ERROR     */	"!teleport without error!",
	/*	164 SPELL_PORTAL                */	"!portal!",
	/*	165 SPELL_DRAGON_RIDE           */	"",
	/*	166 SPELL_MOUNT                 */	"!mount!",
	/*	167 SPELL_NO_MESSAGE            */	"",
	/*	168 to_do                       */	"!168!",
	/*	169 SKILL_MANTRA                */	"",
	/*	170 SKILL_FIRST_AID             */	"",
	/*	171 SKILL_SIGN                  */	"!sign!",
	/*	172 SKILL_RIDE                  */	"!riding!",
	/*	173 SKILL_SWITCH_OPP            */	"!switch!",
	/*	174 SKILL_DODGE                 */	"!dodge!",
	/*	175 SKILL_REMOVE_TRAP           */	"!remove trap!",
	/*	176 SKILL_RETREAT               */	"!retreat!",
	/*	177 SKILL_QUIV_PALM             */	"Per un istante ti e' sembrato di vedre $c0015vibrare$c0007 le mani di $n.",
	/*	178 SKILL_SAFE_FALL             */	"!safe fall!",
	/*	179 SKILL_FEIGN_DEATH           */	"!feign death!",
	/*	180 SKILL_HUNT                  */	"!hunt!",
	/*	181 SKILL_FIND_TRAP             */	"!find traps!",
	/*	182 SKILL_SPRING_LEAP           */	"!spring leap!",
	/*	183 SKILL_DISARM                */	"!disarm!",
	/*	184 SKILL_READ_MAGIC            */	"!read magic!",
	/*	185 SKILL_EVALUATE              */	"!evalutate!",
	/*	186 SKILL_SPY					*/	"",
	/*	187 SKILL_DOORBASH              */	"!doorbash!",
	/*	188 SKILL_SWIM                  */	"",
	/*	189 SKILL_CONS_UNDEAD           */	"!consider undead!",
	/*	190 SKILL_CONS_VEGGIE           */	"!consider veggie!",
	/*	191 SKILL_CONS_DEMON            */	"!consider demon!",
	/*	192 SKILL_CONS_ANIMAL           */	"!consider animal!",
	/*	193 SKILL_CONS_REPTILE          */	"!consider reptile!",
	/*	194 SKILL_CONS_PEOPLE           */	"!consider people!",
	/*	195 SKILL_CONS_GIANT            */	"!consider giant!",
	/*	196 SKILL_CONS_OTHER            */	"!consider other!",
	/*	197 SKILL_DISGUISE              */	"",
	/*	198 SKILL_CLIMB                 */	"!climb!",
	/*	199 SKILL_FINGER                */	"!finger!",
	/*	200 SPELL_GEYSER                */	"!geyser!",
	/*	201 SPELL_MIRROR_IMAGES         */	"L'$c0012immagine illusoria$c0007 di $n inizia a svanire.",
	/*	202 SKILL_TSPY                  */	"",
	/*	203 SKILL_EAVESDROP             */	"!eavesdrop!",
	/*	204 SKILL_PARRY                 */	"!parry!",
	/*	205 SKILL_MINER                 */	"!miner!",
	/*	206 SPELL_GREEN_SLIME           */	"!green slime!",
	/*	207 SKILL_BERSERK               */	"!berserk!",
	/*	208 SKILL_TAN					*/	"!tan!",
	/*	209 SKILL_AVOID_BACK_ATTACK     */	"!avoid backattack!",
	/*	210 SKILL_FIND_FOOD             */	"!find food!",
	/*	211 SKILL_FIND_WATER            */	"!find water!",
	/*	212 SPELL_PRAYER                */	"",
	/*	213 SKILL_MEMORIZE              */	"$n scorre le pagine del suo libro, leggendo attentamente.",
	/*	214 SKILL_BELLOW                */	"!bellow!",
	/*	215 SPELL_GLOBE_DARKNESS        */	"L'$c0008oscurita'$c0007 che avvolge $n vacilla.",
	/*	216 SPELL_GLOBE_MINOR_INV       */	"Il $c0012globo$c0007 protettivo attorno a $n emette un lampo di luce.",
	/*	217 SPELL_GLOBE_MAJOR_INV       */	"Il $c0014globo$c0007 protettivo attorno a $n tremola per un momento.",
	/*	218 SPELL_PROT_ENERGY_DRAIN     */	"La protezione di $n contro i $c0008Non-Morti$c0007 vacilla.",
	/*	219 SPELL_PROT_DRAGON_BREATH    */	"",
	/*	220 SPELL_ANTI_MAGIC_SHELL      */	"Il $c0012globo$c0007 protettivo attorno a $n $c0011scintilla$c0007 improvvisamente.",
	/*	221 SKILL_DOORWAY               */	"!doorway!",
	/*	222 SKILL_PORTAL                */	"!psi_portal!",
	/*	223 SKILL_SUMMON                */	"!psi_summon!",
	/*	224 SKILL_INVIS                 */	"!psi_invis!",
	/*	225 SKILL_CANIBALIZE            */	"!canibalize!",
	/*	226 SKILL_FLAME_SHROUD          */	"!flame shroud!",
	/*	227 SKILL_AURA_SIGHT            */	"!aura sight!",
	/*	228 SKILL_GREAT_SIGHT           */	"!great sight!",
	/*	229 SKILL_PSIONIC_BLAST         */	"$n sembra riprendersi.",
	/*	230 SKILL_HYPNOSIS              */	"!hypnosis!",
	/*	231 SKILL_MEDITATE              */	"Per un attimo $n smette di meditare, poi ricomincia.",
	/*	232 SKILL_SCRY                  */	"!scry!",
	/*	233 SKILL_ADRENALIZE            */	"",
	/*	234 SKILL_BREW                  */	"!brew!",
	/*	235 SKILL_RATION                */	"!ration!",
	/*	236 SKILL_HOLY_WARCRY           */	"!holy warcry!",
	/*	237 SKILL_BLESSING              */	"",
	/*	238 SKILL_LAY_ON_HANDS          */	"",
	/*	239 SKILL_HEROIC_RESCUE         */	"!heroic rescue!",
	/*	240 SKILL_DUAL_WIELD            */	"!dual wield!",
	/*	241 SKILL_PSI_SHIELD            */	"Lo $c0011scudo$c0007 protettivo di $n trema un attimo.",
	/*	242 SPELL_PROT_FROM_EVIL_GROUP  */	"",
	/*	243 SPELL_PRISMATIC_SPRAY       */	"!prismatic spry!",
	/*	244 SPELL_INCENDIARY_CLOUD      */	"!incendiary cloud!",
	/*	245 SPELL_DISINTEGRATE          */	"!disintegrate!",
	/*	246 LANG_COMMON                 */	"!lang common!",
	/*	247 LANG_ELVISH                 */	"!lang elvish!",
	/*	248 LANG_HALFLING               */	"!lang halfling!",
	/*	249 LANG_DWARVISH               */	"!lang dwarvish!",
	/*	250 LANG_ORCISH                 */	"!lang orcish!",
	/*	251 LANG_GIANTISH               */	"!lang giantish!",
	/*	252 LANG_OGRE					*/	"!lang ogre!",
	/*	253 LANG_GNOMISH                */	"!lang gnomish!",
	/*	254 SKILL_ESP					*/	"",
	/*	255 SPELL_COMP_LANGUAGES        */	"",
	/*	256 SPELL_PROT_FIRE             */	"La protezione dal $c0009fuoco$c0007 di $n svanisce per un momento.",
	/*	257 SPELL_PROT_COLD             */	"La protezione dal $c0014freddo$c0007 di $n svanisce per un momento.",
	/*	258 SPELL_PROT_ENERGY           */	"La protezione dall'$c0011energia$c0007 di $n svanisce per un momento.",
	/*	259 SPELL_PROT_ELEC             */	"La protezione dall'$c0012elettricita'$c0007 di $n svanisce per un momento.",
	/*	260 SPELL_ENCHANT_ARMOR         */	"!enchant armor!",
	/*	261 SPELL_MESSENGER             */	"!messenger!",
	/*	262 SPELL_PROT_BREATH_FIRE      */	"Il globo di protezione dal soffio $c0009infuocato$c0007 di $n emette un $c0011lampo$c0007.", /* 262 */
	/*	263 SPELL_PROT_BREATH_FROST     */	"Il globo di protezione dal soffio $c0014gelido$c0007 di $n emette un $c0011lampo$c0007.",
	/*	264 SPELL_PROT_BREATH_ELEC      */	"Il globo di protezione dal soffio $c0012elettrico$c0007 di $n emette un $c0011lampo$c0007.",
	/*	265 SPELL_PROT_BREATH_ACID      */	"Il globo di protezione dal soffio $c0010acido$c0007 di $n emette un $c0011lampo$c0007.",
	/*	266 SPELL_PROT_BREATH_GAS       */	"Il globo di protezione dal soffio $c0011gassoso$c0007 di $n emette un $c0011lampo$c0007.",
	/*	267 SPELL_WIZARDEYE             */	"$n perde per un attimo il controllo del suo occhio magico.",
	/*	268 SKILL_MIND_BURN             */	"!mind burn!",
	/*	269 SKILL_CLAIRVOYANCE          */	"",
	/*	270 SKILL_DANGER_SENSE          */	"",
	/*	271 SKILL_DISINTEGRATE          */	"!psi_disint!",
	/*	272 SKILL_TELEKINESIS           */	"!telekinesis!",
	/*	273 SKILL_LEVITATION            */	"",
	/*	274 SKILL_CELL_ADJUSTMENT       */	"!cell adjustment!",
	/*	275 SKILL_CHAMELEON             */	"!chameleon!",
	/*	276 SKILL_PSI_STRENGTH          */	"",
	/*	277 SKILL_MIND_OVER_BODY        */	"",
	/*	278 SKILL_PROBABILITY_TRAVEL    */	"!probability travel!",
	/*	279 SKILL_PSI_TELEPORT          */	"!psi teleport!",
	/*	280 SKILL_DOMINATION            */	"!domination!",
	/*	281 SKILL_MIND_WIPE             */	"!mind wipe!",
	/*	282 SKILL_PSYCHIC_CRUSH         */	"!psychic crush!",
	/*	283 SKILL_TOWER_IRON_WILL       */	"",
	/*	284 SKILL_MINDBLANK             */	"",
	/*	285 SKILL_PSYCHIC_IMPERSONATION */	"",
	/*	286 SKILL_ULTRA_BLAST           */	"!ultra blast!",
	/*	287 SKILL_INTENSIFY             */	"",
	/*	288 SKILL_SPOT                  */	"!spot!",
	/*	289 SKILL_IMMOLATION            */	"!immolation!",
	/*	290 STATUS_QUEST                */	"",
	/*	291 SKILL_DAIMOKU               */	"",
	/*	292 SKILL_FORGE                 */	"!forge!",
	/*	293 SKILL_DETERMINE             */	"!determine!",
	/*	294 SKILL_EQUILIBRIUM           */	"!equilibrium!",
											"\n"

};



int rev_dir[] = {
	2,
	3,
	0,
	1,
	5,
	4
};

int TrapDir[] = {
	TRAP_EFF_NORTH,
	TRAP_EFF_EAST,
	TRAP_EFF_SOUTH,
	TRAP_EFF_WEST,
	TRAP_EFF_UP,
	TRAP_EFF_DOWN
};

int movement_loss[]= {
	1,	/* Inside     */
	2,	/* City       */
	2,	/* Field      */
	3,	/* Forest     */
	4,	/* Hills      */
	6,	/* Mountains  */
	8,	/* Swimming   */
	10,	/* Unswimable */
	2,	/* Flying     */
	20,	/* Submarine  */
	4,	/* Desert     */
	1,	/* Tree       */
	2	/* Dark City  */
};

const  char* exits[] = {
	"$c0010Nord ",
	"$c0011Est  ",
	"$c0012Sud  ",
	"$c0013Ovest",
	"$c0014Alto ",
	"$c0015Basso"
};
/* used in listing exits for this room */
const  char* listexits[] = {
	"$c0010Nord",
	"$c0011Est",
	"$c0012Sud",
	"$c0013Ovest",
	"$c0014Alto",
	"$c0015Basso"
};




const char* dirsTo[] = {
	"a nord",
	"ad est",
	"a sud",
	"ad ovest",
	"verso l'alto",
	"verso il basso",
	"\n"
};

const char* dirsFrom[] = {
	"da nord",
	"da est",
	"da sud",
	"da ovest",
	"giu dall'alto",
	"su dal basso",
	"\n"
};

const char* dirs[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
};


const char* ItemDamType[] = {
	"$c0009brucia$c0007",
	"$c0014congela$c0007",
	"$c0012elettrifica$c0007",
	"$c0001danneggia$c0007",
	"$c0010corrode$c0007"
};

const char* weekdays[7] = {
	"il $c0015Giorno delle Spade$c0007",
	"il $c0003Giorno degli Archi$c0007",
	"il $c0015Giorno delle Asce$c0007",
	"il $c0015Giorno dei Martelli$c0007",
	"il $c0012Giorno della Battaglia$c0007",
	"il $c0011Giorno della Grazia$c0007",
	"il $c0014Giorno della Pace$c0007"
};

const char* month_name[17] = {
	"$c0015Mese dell'Inverno$c0007",     	/* 0 */
	"$c0015Mese del Lupo Bianco$c0007",
	"$c0014Mese della Grande Gelata$c0007",
	"$c0009Mese delle Antiche Forze$c0007",
	"$c0011Mese del Grande Pianto$c0007",
	"$c0010Mese della Primavera$c0007",
	"$c0003Mese della $c0010N$c0003a$c0010t$c0003u$c0010r$c0003a$c0007",
	"$c0013Mese della Fertilita'$c0007",
	"$c0003Mese del Drago$c0007",
	"$c0011Mese del Sole$c0007",
	"$c0009Mese del Caldo$c0007",
	"$c0012Mese della Battaglia$c0007",
	"$c0009Mese della Guerra fra gli Dei$c0007",
	"$c0008Mese delle Ombre$c0007",
	"$c0008Mese delle Ombre Lunghe$c0007",
	"$c0008Mese dell'Oscurita' Primordiale$c0007",
	"$c0009Mese del Malvagio$c0007"
};

//	questa tabella sembra inutilizzata
const int sharp[] = {
	0,
	0,
	0,
	1,	/* Slashing */
	0,
	0,
	0,
	0,	/* Bludgeon */
	0,
	0,
	0,
	0 	/* Pierce   */
};

const char* eqWhere[] = {
	"<come luce>             ",
	"<sul dito destro>       ",
	"<sul dito sinistro>     ",
	"<intorno al collo>      ",
	"<intorno al collo>      ",
	"<sul corpo>             ",
	"<in testa>              ",
	"<sulle gambe>           ",
	"<ai piedi>              ",
	"<sulle mani>            ",
	"<sulle braccia>         ",
	"<come scudo>            ",
	"<intorno al corpo>      ",
	"<intorno alla vita>     ",
	"<al polso destro>       ",
	"<al polso sinistro>     ",
	"<impugnato>             ",
	"<tenuto>                ",
	"<sulla schiena>         ",
	"<all'orecchio destro>   ",
	"<all'orecchio sinistro> ",
	"<davanti agli occhi>    ",
	"<incoccata>             "
};

const char* drinks[]= {
	"$c0012acqua$c0007",
	"$c0011birra$c0007",
	"$c0001vino$c0007",
	"$c0011birra chiara$c0007",
	"$c0003birra scura$c0007",
	"$c0003whiskey$c0007",
	"$c0011limonata$c0007",
	"acquavite",
	"vodka",
	"$c0002succo di melma$c0007",
	"$c0015latte$c0007",
	"$c0003te$c0007",
	"$c0008caffe'$c0007",
	"$c0001sangue$c0007",
	"$c0014acqua salata$c0007",
	"$c0001coca cola$c0007",
	"\n"
};

const char* drinknames[]= {
	"acqua",
	"birra",
	"vino",
	"birra",
	"birra",
	"whiskey",
	"limonata",
	"acquavite",
	"vodka",
	"succo",
	"latte",
	"te",
	"caffe'",
	"sangue",
	"salata",
	"cola",
	"\n"
};

/*************************************************************/
/* XI-2009 Jethro permette il Mindflayer PSI sino a Principe */
/* in previsione di renderla razza giocabile MU(SO) MK PSI   */
/* anche biclassata nelle 3 combinazioni					 */
/*************************************************************/

int RacialMax[MAX_RACE+1][MAX_CLASS] = {

	/*						mu  cl  wa  th  dr  mk  ba  so  pa  ra psi */
	/* halfbred		*/	{	25, 25, 25, 25, 25, 25, 10, 25,  0,  0,  0	},
	/* Human		*/	{	51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51	},
	/* moonelf		*/	{	51, 30, 20, 51, 51, 20,  0, 51, 40, 51, 40	},
	/* dwarf		*/	{	 0, 51, 51, 40,  0, 40,  0,  0, 51,  0, 20	},
	/* halfling		*/	{	20, 40, 30, 51, 51, 20,  0, 20,  0,  0, 51	},
	/* gnome		*/	{	40, 51, 30, 40, 20, 20,  0, 40,  0,  0,  0	},
	/* reptile		*/	{	20, 30, 40, 50, 51, 40,  0, 20,  0,  0,  0	},
	/* SPECIAL		*/	{	50, 50, 50,  1, 51,  1,  0, 50,  0,  0,  0	},
	/* lycanth		*/	{	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50	},
	/* dragon		*/	{	51, 20, 51, 30, 20, 51,  0, 51,  0,  0,  0	},
	/* undead		*/	{	35, 35, 35, 35, 35, 35,  0, 35,  0,  0,  0	},
	/* orc			*/	{	 0, 30, 51, 45,  0,  0, 40,  0,  0,  0,  0	},
	/* insect		*/	{	30, 20, 40, 51, 50, 30, 10, 30,  0,  0,  0	},
	/* arachid		*/	{	20, 30, 40, 51, 50, 30, 10, 20,  0,  0,  0	},
	/* dinosaur		*/	{	20, 30, 51, 40, 50, 30, 10, 20,  0,  0,  0	},
	/* fish			*/	{	20, 40, 30, 50, 51, 30, 10, 20,  0,  0,  0	},
	/* bird			*/	{	40, 30, 30, 50, 51, 20, 10, 40,  0,  0,  0	},
	/* giant		*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* predator		*/	{	40, 30, 51, 20, 30, 50, 10, 40,  0,  0,  0	},
	/* parasite		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* slime		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* demon		*/	{	51, 30, 20, 50, 30, 20,  0, 51,  0,  0,  0	},
	/* snake		*/	{	40, 30, 30, 51, 50, 20, 10, 40,  0,  0,  0	},
	/* herbiv		*/	{	30, 40, 20, 30, 51, 50, 10, 30,  0,  0,  0	},
	/* tree			*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* veggie		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* elment		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* planar		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* devil		*/	{	50, 51, 40, 20, 30, 40, 10, 50,  0,  0,  0	},
	/* ghost		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* goblin		*/	{	20, 30, 51, 51,  0,  0, 30, 20,  0,  0,  0	},
	/* troll		*/	{	 0, 35, 51, 35,  0,  0, 51,  0,  0,  0,  0	},
	/* vegman		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* mflayer		*/	{	51, 40, 30, 30, 30, 51, 10, 51,  0,  0, 51	},
	/* primate		*/	{	30, 30, 40, 51, 20, 51, 10, 30,  0,  0,  0	},
	/* enfan		*/	{	50, 30, 51, 40, 20, 30, 10, 50,  0,  0,  0	},
	/* drow			*/	{	51, 51, 30, 40, 20, 30,  0, 51, 40, 51, 40	},
	/* golem		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* skexie		*/	{	50, 30, 40, 51, 30, 20, 10, 50,  0,  0,  0	},
	/* trogman		*/	{	30, 40, 50, 51, 30, 20, 10, 30,  0,  0,  0	},
	/* patryn		*/	{	51, 30, 50, 30, 20, 40, 10, 51,  0,  0,  0	},
	/* labrat		*/	{	50, 30, 51, 40, 20, 30, 10, 50,  0,  0,  0	},
	/* sartan		*/	{	50, 51, 30, 20, 40, 30, 10, 50,  0,  0,  0	},
	/* tytan		*/	{	50, 30, 51, 30, 40, 20, 10, 50,  0,  0,  0	},
	/* smurf		*/	{	10, 10, 10, 10, 10, 10, 10, 10,  0,  0,  0	},
	/* roo			*/	{	40, 30, 51, 30, 20, 51, 10, 40,  0,  0,  0	},
	/* horse		*/	{	30, 40, 20, 30, 51, 50, 10, 30,  0,  0,  0	},
	/* draagdim		*/	{	51, 30, 20, 51, 40, 20, 10, 51,  0,  0,  0	},
	/* astral		*/	{	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* god			*/	{	51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51 },
	/* giant hill	*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* giant frost	*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* giant fire	*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* giant cloud	*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* giant storm	*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* giant stone	*/	{	20, 40, 51, 51, 30, 30, 10, 20,  0,  0,  0	},
	/* drag red		*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag black	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag green	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag white	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag blue	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag silver	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag gold	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag bronze	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag cooper	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* drag brass	*/	{	51, 20, 51, 30, 20, 51, 10, 51,  0,  0,  0	},
	/* unded vamp	*/	{	51, 35, 51, 35, 35, 35, 10, 51,  0,  0,  0	},
	/* unded lich	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* unded wight	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* unded ghast	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* unded spect	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* unded zombi	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* unded skele	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* unded ghoul	*/	{	35, 35, 35, 35, 35, 35, 10, 35,  0,  0,  0	},
	/* halfelf		*/	{	51, 51, 35, 51, 35, 30,  0, 51, 51, 51, 40	},
	/* half-ogre	*/	{	 0, 30, 51,  0,  0,  0,  0,  0,  0,  0,  0	},
	/* half-orc		*/	{	 0, 30, 45, 51,  0,  0,  0,  0,  0,  0,  0	},
	/* half-gian	*/	{	 0,  0, 51,  0,  0,  0, 51,  0,  0,  0, 51 },
	/* lizardman	*/	{	20, 51, 51, 35, 15, 40,  0, 20, 45, 30,  0	},
	/* dark-dwarf	*/	{	20, 51, 51, 45,  0, 40,  0, 30,  0,  0,  0	},
	/* deep-gnome	*/	{	20, 51, 51, 35,  0, 40,  0, 20,  0,  0,  0	},
	/* gnoll		*/	{	20, 40, 51, 51,  0, 30,  0, 20,  0,  0,  0	},
	/* goldelf		*/	{	51, 35, 15, 51, 51, 20,  0, 51, 45, 51, 40	},
	/* wildelf		*/	{	30, 35, 51, 51, 51,  0,  0, 30,  0, 51, 40	},
	/* sea-elf		*/	{	51, 25, 30, 51, 51, 20,  0, 51, 40, 51, 40 }
	/*						mu  cl  wa  th  dr  mk  ba  so  pa  ra psi */
};

/* Ogni razza puo' avere una HomeTown diversa,
 * Il primo indice e' per la nascita, il secondo per quando mi sono perso
 * per un qualche motivo la stanza :-)
 */

int RacialHome[MAX_RACE+1][2] = {
	/*							Born	Life	*/
	/* halfbred		 0*/	{	   1,	   1	},
	/* Human		 1*/	{	7800,	3008	},
	/* moonelf		 2*/	{	7800,	3008	},
	/* dwarf		 3*/	{	7800,	3008	},
	/* halfling		 4*/	{	7800,	1158	},
	/* gnome		 5*/	{	7800,	3008	},
	/* reptile		 6*/	{	   1,	   1	},
	/* SPECIAL		 7*/	{	   1,	   1	},
	/* lycanth		 8*/	{	   1,	   1	},
	/* dragon		 9*/	{	   1,	   1	},
	/* undead		10*/	{	   1,	   1	},
	/* orc			 1*/	{	7800,	1157	},
	/* insect		 2*/	{	   1,	   1	},
	/* arachnid		 3*/	{	   1,	   1	},
	/* dinosaur		 4*/	{	   1,	   1	},
	/* fish			 5*/	{	   1,	   1	},
	/* bird			 6*/	{	   1,	   1	},
	/* giant		 7*/	{	   1,	   1	},
	/* predator		 8*/	{	   1,	   1	},
	/* parasite		 9*/	{	   1,	   1	},
	/* slime		20*/	{	   1,	   1	},
	/* demon		 1*/	{	7800,	1157	},
	/* snake		 2*/	{	   1,	   1	},
	/* herbiv		 3*/	{	   1,	   1	},
	/* tree			 4*/	{	   1,	   1	},
	/* veggie		 5*/	{	   1,	   1	},
	/* elment		 6*/	{	   1,	   1	},
	/* planar		 7*/	{	   1,	   1	},
	/* devil		 8*/	{	   1,	   1	},
	/* ghost		 9*/	{	   1,	   1	},
	/* goblin		30*/	{	7800,	1157	},
	/* troll		 1*/	{	7800,	1157	},
	/* vegman		 2*/	{	   1,	   1	},
	/* mflayer		 3*/	{	   1,	   1	},
	/* primate		 4*/	{	   1,	   1	},
	/* enfan		 5*/	{	   1,	   1	},
	/* drow			 6*/	{	7800,	3008	},
	/* golem		 7*/	{	   1,	   1	},
	/* skexie		 8*/	{	   1,	   1	},
	/* trogman		 9*/	{	   1,	   1	},
	/* patryn		40*/	{	   1,	   1	},
	/* labrat		 1*/	{	   1,	   1	},
	/* sartan		 2*/	{	   1,	   1	},
	/* tytan		 3*/	{	   1,	   1	},
	/* smurf		 4*/	{	   1,	   1	},
	/* roo			 5*/	{	   1,	   1	},
	/* horse		 6*/	{	   1,	   1	},
	/* draagdim		 7*/	{	   1,	   1	},
	/* astral		 8*/	{	   1,	   1	},
	/* god			 9*/	{	1000,	1000	},
	/* giant hill	50*/	{	7800,	   1	},
	/* giant frost	 1*/	{	7800,	  40	},
	/* giant fire	 2*/	{	7800,	  40	},
	/* giant cloud	 3*/	{	7800,	  40	},
	/* giant storm	 4*/	{	7800,	  40	},
	/* giant stone	 5*/	{	7800,	  40	},
	/* drag red		 6*/	{	  51,	  20	},
	/* drag black	 7*/	{	  51,	  20	},
	/* drag green	 8*/	{	  51,	  20	},
	/* drag white	 9*/	{	  51,	  20	},
	/* drag blue	60*/	{	  51,	  20	},
	/* drag silver	 1*/	{	  51,	  20	},
	/* drag gold	 2*/	{	  51,	  20	},
	/* drag bronze	 3*/	{	  51,	  20	},
	/* drag cooper	 4*/	{	  51,	  20	},
	/* drag brass	 5*/	{	  51,	  20	},
	/* unded vamp	 6*/	{	7800,	3008	},
	/* unded lich	 7*/	{	  35,	  35	},
	/* unded wight	 8*/	{	  35,	  35	},
	/* unded ghast	 9*/	{	  35,	  35	},
	/* unded spec	70*/	{	  35,	  35	},
	/* unded zombi	 1*/	{	  35,	  35	},
	/* unded skele	 2*/	{	  35,	  35	},
	/* unded ghoul	 3*/	{	  35,	  35	},
	/* halfelf		 4*/	{	7800,	3008	},
	/* half-ogre	 5*/	{	7800,	3008	},
	/* half-orc		 6*/	{	7800,	3008	},
	/* half-giant	 7*/	{	7800,	3008	},
	/* lizardman	 8*/	{	7800,	  51	},
	/* dark-dwarf	 9*/	{	7800,	1157	},
	/* deep-gnome	80*/	{	7800,	1157	},
	/* gnoll		 1*/	{	7800,	  40	},
	/* goldelf		 2*/	{	7800,	3008	},
	/* wildelf		 3*/	{	7800,	3008	},
	/* sea-elf		 4*/	{	7800,	3008	}
	/*							1st		2st		*/
};


int ItemSaveThrows[E_ITEM_TYPE_MAX][E_DAMAGE_TYPE_MAX] = {
//		fire	cold	elec	blow	acid
	{	15,		 2,		10,		10,		10	},	//	light
	{	19, 	 2,		16,		 2,		 7	},	//	scroll
	{	11,		 2,		 2,		13,		 9	},	//	wand
	{	 7,		 2,		 2,		10,		 8	},	//	staff
	{	 6,		 2,		 2,		 7,		13	},	//	weapon
	{	10,		10,		10,		10,		10	},	//	fireweapon
	{	10,		10,		10,		10,		10	},	//	missile
	{	 6,		 2,		 2,		 7,		13	},	//	treasure
	{	 6,		 2,		 2,		 7,		13	},	//	armor
	{	 7,		 6,		 2,		20,		 5	},	//	potion
	{	10,		10,		10,		10,		10	},	//	worn			not defined
	{	10,		10,		10,		10,		10	},	//	other			not defined
	{	10,		10,		10,		10,		10	},	//	trash			not defined
	{	10,		10,		10,		10,		10	},	//	trap			not defined
	{	19,		 2,		 2,		16,		 7	},	//	container
	{	10,		10,		10,		10,		10	},	//	note			not defined
	{	 7,		 6,		 2,		20,		 5	},	//	drinkcon
	{	 6,		 2,		 2,		 7,		13	},	//	key
	{	 6,		 3,		 2,		 3,		10	},	//	food
	{	10,		10,		10,		10,		10	},	//	money			not defined
	{	11,		 2,		 2,		13,		 9	},	//	pen
	{	 7,		 2,		 2,		10,		 8	},	//	boat
	{	10,		10,		10,		10,		10	},	//	audio			not defined
	{	10,		10,		10,		10,		10	},	//	bord			not defined
	{	10,		10,		10,		10,		10	},	//	tree			not defined
	{	10,		10,		10,		10,		10	},	//	rock			not defined
	{	10,		10,		10,		10,		10	},	//	mined gem		not defined
	{	10,		10,		10,		10,		10	},	//	mined mineral	not defined
	{	10,		10,		10,		10,		10	},	//	bar				not defined
	{	 6,		 2,		 2,		 7,		13	}	//	jewel
};


int drink_aff[][3] = {
	{	 0,		 1,		10	},	//	LIQ_WATER
	{	 3,		 2,		 5	},	//	LIQ_BEER
	{	 5,		 2,		 5	},	//	LIQ_WINE
	{	 2,		 2,		 5	},	//	LIQ_ALE
	{	 1,		 2,		 5	},	//	LIQ_DARKALE
	{	 6,		 1,		 4	},	//	LIQ_WHISKY
	{	 0,		 1,		 8	},	//	LIQ_LEMONADE
	{	10,		 0,		 0	},	//	LIQ_FIREBRT
	{	10,		 1,		10	},	//	LIQ_LOCALSPC
	{	 0,		 4,		-8	},	//	LIQ_SLIME
	{	 0,		 3,		 6	},	//	LIQ_MILK
	{	 0,		 1,		 6	},	//	LIQ_TEA
	{	 0,		 1,		 6	},	//	LIQ_COFFE
	{	 0,		 2,		-1	},	//	LIQ_BLOOD
	{	 0,		 1,		-2	},	//	LIQ_SALTWATER
	{	 0,		 1,		 5	},	//	LIQ_COKE
	{	 0,		 0,		 0	}
};

const char* color_liquid[]= {
	"$c0014chiaro$c0007",
	"$c0003marrone$c0007",
	"$c0014chiaro$c0007",
	"$c0003marrone$c0007",
	"$c0004scuro$c0007",
	"$c0011dorato$c0007",
	"$c0009rosso$c0007",
	"$c0002verde$c0007",
	"$c0015cristallino$c0007",
	"$c0010verde chiaro$c0007",
	"$c0015bianco$c0007",
	"$c0003marrone$c0007",
	"$c0008nero$c0007",
	"$c0002verde$c0007",
	"$c0014chiaro$c0007",
	"$c0008nero$c0007",
	"\n"
};

const char* fullness[] = {
	"con un po'",
	"mezzo pieno",
	"quasi pieno",
	"pieno"
};


struct title_type titles[MAX_CLASS][ABS_MAX_LVL] = {
	{		/* Mage */
		{	"Man",						"Woman",							0	},
		{	"Supplicant",				"Supplicant",						1	},	/* Modificati da EleiMiShill */
		{	"Apprentice",				"Apprentice",					 1600	},
		{	"Student",					"Student",						 3300	},
		{	"Scholar",					"Scholar",						 5300	},
		{	"Trickster",				"Trickster",					 7800	},
		{	"Illusionist",				"Illusionist",					11000	},
		{	"Cabalist",					"Cabalist",						14800	},
		{	"Apparitionist",			"Apparitionist",				19400	},
		{	"Medium",					"Gypsy",						25000	},
		{	"Scribe",					"Scribe",						32000	},	/* 10 */
		{	"Sage",						"Sage",							41000	},
		{	"Seer",						"Seeress",						52000	},
		{	"Divinater",				"Divinatress",					65000	},
		{	"Alchemist",				"Alchemist",					81000	},
		{	"Evoker",					"Evoker",					   100000	},
		{	"Necromancer",				"Necromancer",				   125000	},
		{	"Abjurer",					"Abjuress",					   155000	},
		{	"Invoker",					"Invoker",					   192000	},
		{	"Enchanter",				"Enchantress",				   238000	},
		{	"Conjurer",					"Conjuress",				   294000	},	/* 20 */
		{	"Summoner",					"Summoner",					   363000	},
		{	"Magician",					"Magician",					   447000	},
		{	"Spiritualist",				"Spiritualist",				   552000	},
		{	"Savant",					"Savanti",					   680000	},
		{	"Shaman",					"Shamaness",				   838000	},
		{	"Mystic",					"Mystic",					  1030000	},
		{	"Wiccen",					"Wicca",					  1270000	},
		{	"Mentalist",				"Mentalist",				  1570000	},
		{	"Mnemonicist",				"Mnemonicist",				  1930000	},
		{	"Nueromancer",				"Nueromancer",				  2370000	},	/* 30 */
		{	"Spellbinder",				"Spellbinder",				  2920000	},
		{	"Synergist",				"Synergist",				  3590000	},
		{	"Oculist",					"Oculist",					  4420000	},
		{	"Thaumaturgist",			"Thaumaturgist",			  5440000	},
		{	"SpellMaster",				"Spell Mistress",			  6700000	},
		{	"Dispeller",				"Dispeller",				  8240000	},
		{	"Warlock",					"War Witch",				 10200000	},
		{	"Sorcerer",					"Sorceress",				 12500000	},
		{	"Wizard",					"Wizard",					 15300000	},
		{	"Great Wizard",				"Great Wizard",				 18900000	},	/* 40 */
		{	"Magus",					"Incantrix",				 23200000	},
		{	"Serpent Mage",				"Serpent Mage",				 28600000	},
		{	"Arch Magi",				"Majestrix",				 35200000	},
		{	"Arch Magi",				"Majestrix",				 43300000	},
		{	"Arch Magi",				"Majestrix",				 53200000	},	/* 45 */
		{	"Arch Magi",				"Majestrix",				 65500000	},
		{	"Arch Magi",				"Majestrix",				 80600000	},
		{	"Arch Magi",				"Majestrix",				 99100000	},
		{	"Arch Magi",				"Majestrix",				122000000	},
		{	"Arch Magi",				"Majestrix",				150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* Cleric */
		{	"Man",						"Woman",							0	},
		{	"Layman",					"Laywoman",							1	},	/* Modificati da EleiMiShill*/
		{	"Seeker",					"Seeker",						 1600	},
		{	"Believer",					"Believer",						 3300	},
		{	"Novice",					"Novice",						 5300	},
		{	"Initiate",					"Initiate",						 7800	},
		{	"Attendant",				"Attendant",					11000	},
		{	"Acolyte",					"Acolyte",						14800	},
		{	"Minion",					"Minion",						19400	},
		{	"Adept",					"Adept",						25000	},
		{	"Priest",					"Priestess",					32000	},	/* 10 */
		{	"Missionary",				"Missionary",					41000	},
		{	"Levite",					"Levitess",						52000	},
		{	"Curate",					"Curate",						65000	},
		{	"Chaplain",					"Chaplain",						81000	},
		{	"Padre",					"Matron",					   100000	},
		{	"Canon",					"Canon",					   125000	},
		{	"Vicar",					"Vicaress",					   155000	},
		{	"Deacon",					"Deaconess",				   192000	},
		{	"Shaman",					"Shamaness",				   238000	},
		{	"Speaker",					"Speaker",					   294000	},	/* 20 */
		{	"Confessor",				"Confessor",				   363000	},
		{	"Expositer",				"Expositress",				   447000	},
		{	"Mystic",					"Mystic",					   552000	},
		{	"Hermit",					"Hermitess",				   680000	},
		{	"Faith Healer",				"Faith Healer",				   838000	},
		{	"Healer",					"Healer",					  1030000	},
		{	"Lay Healer",				"Lay Healer",				  1270000	},
		{	"Illuminator",				"Illuminatrix",				  1570000	},
		{	"Evangelist",				"Evangelist",				  1930000	},
		{	"Voice",					"Voice",					  2370000	},	/* 30 */
		{	"Templar",					"Templar",					  2920000	},
		{	"Hospitalier",				"Hospitalier",				  3590000	},
		{	"Chuirgeon",				"Chuirgeoness",				  4420000	},
		{	"Chuirgeon General",		"Chuirgeon General",		  5440000	},
		{	"Inquisitor",				"Inquisitrix",				  6700000	},
		{	"Abbot",					"Mother Superior",			  8240000	},
		{	"Reverend",					"Reverend Mother",			 10200000	},
		{	"Bishop",					"Bishop",					 12500000	},
		{	"Arch Bishop",				"Arch Bishop",				 15300000	},
		{	"Cardinal",					"Mother Superior",			 18900000	},	/* 40 */
		{	"Patriarch",				"Matriarch",				 23200000	},
		{	"Grand Inquisitor",			"Grand Inquisitrix",		 28600000	},
		{	"High Priest",				"High Priestess",			 35200000	},
		{	"High Priest",				"High Priestess",			 43300000	},
		{	"High Priest",				"High Priestess",			 53200000	},	/* 45 */
		{	"High Priest",				"High Priestess",			 65500000	},
		{	"High Priest",				"High Priestess",			 80600000	},
		{	"High Priest",				"High Priestess",			 99100000	},
		{	"High Priest",				"High Priestess",			122000000	},
		{	"High Priest",				"High Priestess",			150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* Warrior */
		{	"Man",						"Woman",							0	},
		{	"Swordpupil",				"Swordpupil",						1	},	/* Modificati da EleiMiShill*/
		{	"Recruit",					"Recruit",						 1600	},
		{	"Runner",					"Runner",						 3300	},
		{	"Sentry",					"Sentry",						 5300	},
		{	"Man-at-arms",				"Woman-at-arms",				 7800	},
		{	"Mercenary",				"Mercenary",					11000	},
		{	"Scout",					"Scout",						14800	},
		{	"Fighter",					"Fighter",						19400	},
		{	"Warrior",					"Warrior",						25000	},
		{	"Swordsman",				"Swordswoman",					32000	},	/* 10 */
		{	"Fencer",					"Fence",						41000	},
		{	"Combatant",				"Combatrix",					52000	},
		{	"Protector",				"Protector",					65000	},
		{	"Defender",					"Defender",						81000	},
		{	"Warder",					"Warder",					   100000	},
		{	"Guardian",					"Guardian",					   125000	},
		{	"Veteran",					"Veteran",					   155000	},
		{	"Hero",						"Heroine",					   192000	},
		{	"Swashbuckler",				"Swashbuckler",				   238000	},
		{	"Myrmidon",					"Amazon",					   294000	},	/* 20 */
		{	"Esquire",					"Esquire",					   363000	},
		{	"Blademaster",				"Blademistress",			   447000	},
		{	"Reeve",					"Reeve",					   552000	},
		{	"Lieutenant",				"Lieutenant",				   680000	},
		{	"Captain",					"Captain",					   838000	},
		{	"Raider",					"Raider",					  1030000	},
		{	"Champion",					"Lady Champion",			  1270000	},
		{	"Dragoon",					"Lady Dragoon",				  1570000	},
		{	"Armiger",					"Armigress",				  1930000	},
		{	"Srcutifer",				"Scrutifer",				  2370000	},	/* 30 */
		{	"Lancer",					"Lancer",					  2920000	},
		{	"Banneret",					"Banneret",					  3590000	},
		{	"Chevalier",				"Chevaliere",				  4420000	},
		{	"Knight Errant",			"Valkyrie",					  5440000	},
		{	"Knight",					"Lady Knight",				  6700000	},
		{	"Marshall",					"Lady Marshall",			  8240000	},
		{	"Keitar",					"Lady Keitar",				 10200000	},
		{	"Paladin",					"Lady Paladin",				 12500000	},
		{	"Justiciar",				"Justictrix",				 15300000	},
		{	"Grand Marshall",			"Grand Marshall",			 18900000	},	/* 40 */
		{	"Grand Knight",				"Grand Knight",				 23200000	},
		{	"Grand Paladin",			"Grand Paladin",			 28600000	},
		{	"Lord",						"Lady",						 35200000	},
		{	"Lord",						"Lady",						 43300000	},
		{	"Lord",						"Lady",						 53200000	},	/* 45 */
		{	"Lord",						"Lady",						 65500000	},
		{	"Lord",						"Lady",						 80600000	},
		{	"Lord",						"Lady",						 99100000	},
		{	"Lord",						"Lady",						122000000	},
		{	"Lord",						"Lady",						150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* Thief */
		{	"Man",						"Woman",							0	},
		{	"Delinquent",				"Delinquent",						1	},	/* Variati da EleiMiShill */
		{	"Miscreant",				"Miscreant",					 1600	},
		{	"Footpad",					"Footpad",						 3300	},
		{	"Pilferer",					"Pilferess",					 5300	},
		{	"Filcher",					"Filcheress",					 7800	},
		{	"Pincher",					"Pincheress",					11000	},
		{	"Snatcher",					"Snatcheress",					14800	},
		{	"Ninja",					"Ninja",						19400	},
		{	"Pick-Pocket",				"Pick-Pocket",					25000	},
		{	"Cut-Purse",				"Cut-Purse",					32000	},	/* 10 */
		{	"Sharper",					"Sharper",						41000	},
		{	"Burgler",					"Burgler",						52000	},
		{	"Robber",					"Robber",						65000	},
		{	"Mugger",					"Mugger",						81000	},
		{	"Magsman",					"Magswoman",				   100000	},
		{	"Bandito",					"Bandita",					   125000	},
		{	"Highwayman",				"Bandit",					   155000	},
		{	"Brigand",					"Brigand",					   192000	},
		{	"Agent",					"Agent",					   238000	},
		{	"Outlaw",					"Outlaw",					   294000	},	/* 20 */
		{	"Blade",					"Blade",					   363000	},
		{	"Quick-Blade",				"Quick-Blade",				   447000	},
		{	"Knifer",					"Knifer",					   552000	},
		{	"Sneak",					"Sneak",					   680000	},
		{	"Thief",					"Thief",					   838000	},
		{	"Special Agent",			"Special Agent",			  1030000	},
		{	"Collector",				"Collector",				  1270000	},
		{	"Hand",						"Hand",						  1570000	},
		{	"Unseen",					"Unseen",					  1930000	},
		{	"Thug",						"Thug",						  2370000	},	/* 30 */
		{	"Cut-Throat",				"Cut-Throat",				  2920000	},
		{	"Grand-Thief",				"Grand-Thief",				  3590000	},
		{	"Repossessor",				"Repossessor",				  4420000	},
		{	"Killer",					"Killer",					  5440000	},
		{	"Secret Agent",				"Secret Agent",				  6740000	},
		{	"Renegade",					"Renegade",					  8240000	},
		{	"Murderer",					"Murderess",				 10200000	},
		{	"Butcher",					"Butcheress",				 12500000	},
		{	"Slayer",					"Slayer",					 15300000	},
		{	"Executioner",				"Executioner",				 18900000	},	/* 40 */
		{	"Assassin",					"Assassin",					 23200000	},
		{	"Master Assassin",			"Mistress Assassin",		 28600000	},
		{	"Master Thief",				"Master Thief",				 35200000	},
		{	"Master Thief",				"Master Thief",				 43300000	},
		{	"Master Thief",				"Master Thief",				 53200000	},	/* 45 */
		{	"Master Thief",				"Master Thief",				 65500000	},
		{	"Master Thief",				"Master Thief",				 80600000	},
		{	"Master Thief",				"Master Thief",				 99100000	},
		{	"Master Thief",				"Master Thief",				122000000	},
		{	"Master Thief",				"Master Thief",				150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* Druid */
		{	"Man",						"Woman",							0	},
		{	"Aspirant",					"Aspirant",							1	},
		{	"Ovate",					"Ovate",						 1600	},	/* Variati */
		{	"Sprout",					"Sprout",						 3300	},	/* da EleiMiShill */
		{	"Courser",					"Courser",						 5300	},
		{	"Tracker",					"Tracker",						 7800	},
		{	"Guide",					"Guide",						11000	},
		{	"Pathfinder",				"Pathfinder",					14800	},
		{	"Green-Peace Keeper",		"Green-Peace Keeper",			19400	},
		{	"Warder",					"Warder",						25000	},
		{	"Pagan",					"Pagan",						32000	},	/* 10 */
		{	"Watcher",					"Watcher",						41000	},
		{	"Woodsman",					"Woodsman",						52000	},
		{	"Pict",						"Pict",							65000	},
		{	"Tree Hugger",				"Tree Hugger",					81000	},
		{	"Celt",						"Celt",						   100000	},
		{	"Animist",					"Animist",					   125000	},
		{	"Hedge",					"Hedge",					   155000	},
		{	"Tender",					"Tender",					   192000	},
		{	"Strider",					"Strider",					   238000	},
		{	"Druid",					"Druidess",					   294000	},	/* 20 */
		{	"Aquarian",					"Aquarian",					   363000	},
		{	"Arbolist",					"Arbolist",					   447000	},
		{	"Dionysian",				"Dionysian",				   552000	},
		{	"Herbalist",				"Herbalist",				   680000	},
		{	"Naturalist",				"Naturalist",				   838000	},
		{	"Silvian",					"Silvian",					  1030000	},
		{	"Forrestal",				"Forrestal",				  1270000	},
		{	"Ancient",					"Ancient",					  1570000	},
		{	"Archdruid",				"Archdruidess",				  1930000	},
		{	"Great Druid",				"Great Druidess",			  2370000	},	/* 30 */
		{	"Grand Druid",				"Grand Druid",				  2920000	},
		{	"Master of Fire",			"Mistress of Fire",			  3590000	},
		{	"Master of Stone",			"Mistress of Stone",		  4420000	},
		{	"Master of Air",			"Mistress of Air",			  5440000	},
		{	"Master of Water",			"Mistress of Water",		  6700000	},
		{	"Hierophant Initiate",		"Hierophant Initiate",		  8240000	},
		{	"Hierophant Druid",			"Heirophant Druidess",		 10200000	},
		{	"Hierophant Adept",			"Heirophant Adept",			 12500000	},
		{	"Heirophant Master",		"Heirophant Master",		 15300000	},
		{	"Numinous Hierophant",		"Numinous Hierophant",		 18900000	},	/* 40 */
		{	"Mystic Hierophant",		"Mystic Hierophant",		 23200000	},
		{	"Arcane Hierophant",		"Arcane Hierophant",		 28600000	},
		{	"Cabal Hierophant",			"Cabal Hierophant",			 35200000	},
		{	"Cabal Hierophant",			"Cabal Hieorphant",			 43300000	},
		{	"Cabal Hierophant",			"Cabal Hierophant",			 53200000	},	/* 45 */
		{	"Cabal Hierophant",			"Cabal Hierophant",			 65500000	},
		{	"Cabal Hierophant",			"Cabal Hierophant",			 80600000	},
		{	"Cabal Hierophant",			"Cabal Hierophant",			 99100000	},
		{	"Cabal Hierophant",			"Cabal Hierophant",			122000000	},
		{	"Cabal Hierophant",			"Cabal Hierophant",			150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* Monk */
		{	"Man",						"Woman",							0	},
		{	"White Belt",				"White Belt",						1	},	/* Variati */
		{	"Initiate",					"Initiate",						 1600	},	/* da EleiMiShill */
		{	"Brother",					"Sister",						 3300	},
		{	"Layman",					"Laywoman",						 5300	},
		{	"Student",					"Student",						 7800	},
		{	"Practitioner",				"Practitioner",					11000	},
		{	"Exponent",					"Exponent",						14800	},
		{	"Obi",						"Obi",							19400	},
		{	"Monk",						"Monk",							25000	},
		{	"Shodan",					"Shodan",						32000	},	/* 10 */
		{	"Shinobi",					"Shinobi",						41000	},
		{	"Genin",					"Kuniochi",						52000	},
		{	"Disciple",					"Disciple",						65000	},
		{	"Chunin",					"Chunin",						81000	},
		{	"Nidan",					"Nidan",					   100000	},
		{	"Yoda",						"Yoda",						   125000	},
		{	"Jonin",					"Jonin",					   155000	},
		{	"Hwarang",					"Hwarang",					   192000	},
		{	"Sandan",					"Sandan",					   238000	},
		{	"Sabom",					"Sabom",					   294000	},	/* 20 */
		{	"Sensei",					"Sensei",					   363000	},
		{	"Sifu",						"Sifu",						   447000	},
		{	"Guru",						"Guru",						   552000	},
		{	"Pendakar",					"Pendakar",					   680000	},
		{	"Yodan",					"Yodan",					   838000	},
		{	"Master",					"Master",					  1030000	},
		{	"Superior Master",			"Superior Master",			  1270000	},
		{	"Ginsu Master",				"Ginsu Master",				  1570000	},
		{	"Godan",					"Godan",					  1930000	},
		{	"Leopard Master",			"Leopard Master",			  2370000	},	/* 30 */
		{	"Tiger Master",				"Tiger Master",				  2920000	},
		{	"Snake Master",				"Snake Master",				  3590000	},
		{	"Crane Master",				"Crane Master",				  4420000	},
		{	"Dragon Master",			"Dragon Master",			  5440000	},
		{	"Rokudan",					"Rokudan",					  6700000	},
		{	"Master of Seasons",		"Master of Seasons",		  8240000	},
		{	"Master of the Winds",		"Master of the Winds",		 10200000	},
		{	"Master of Harmony",		"Master of Harmony",		 12500000	},
		{	"Shogun",					"Shogun",					 15300000	},
		{	"Shichidan",				"Shichidan",				 18900000	},	/* 40 */
		{	"Hachidan",					"Hachidan",					 23200000	},
		{	"Kudan",					"Kudan",					 28600000	},
		{	"Grand Master",				"Grand Master",				 35200000	},
		{	"Grand Master",				"Grand Master",				 43300000	},
		{	"Grand Master",				"Grand Master",				 53200000	},	/* 45 */
		{	"Grand Master",				"Grand Master",				 65500000	},
		{	"Grand Master",				"Grand Master",				 80600000	},
		{	"Grand Master",				"Grand Master",				 99100000	},
		{	"Grand Master",				"Grand Master",				122000000	},
		{	"Grand Master",				"Grand Master",				150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* Barbarian  - da sistemare MD */
		{	"Man",						"Woman",							0	},
		{	"Grunt",					"Grunt",							1	},	/* Variati */
		{	"Brawler",					"Brawler",						 1600	},	/* da EleiMiShill */
		{	"Smasher",					"Smasher",						 3300	},
		{	"Basher",					"Basher",						 5300	},
		{	"Horseman",					"Horsewoman",					 7800	},
		{	"Horsemaster",				"Horsemaster",					11000	},
		{	"Competent",				"Competent",					14800	},
		{	"Pillager",					"Pillager",						19400	},
		{	"Raider",					"Raider",						25000	},
		{	"Destroyer",				"Destroyer",					32000	},	/* 10 */
		{	"Berserker",				"Berserker",					41000	},
		{	"Raging Berserker",			"Ragin Berserker",				52000	},
		{	"Sacker Of Villages",		"Sacker Of Villages",			65000	},
		{	"Sacker Of Towns",			"Sacker Of Towns",				81000	},
		{	"Sacker Of Cities",			"Sacker Of Cities",			   100000	},
		{	"Chieftain",				"Chieftess",				   125000	},
		{	"Hordling",					"Hordling",					   155000	},
		{	"Hordesman",				"Hordeswoman",				   192000	},
		{	"Hordesmaster",				"Hordesmaster",				   238000	},
		{	"Barbarian",				"Barbarian",				   294000	},	/* 20 */
		{	"Barbarian",				"Barbarian",				   363000	},
		{	"Barbarian",				"Barbarian",				   447000	},
		{	"Barbarian",				"Barbarian",				   552000	},
		{	"Barbarian",				"Barbarian",				   680000	},
		{	"Barbarian",				"Barbarian",				   838000	},
		{	"Barbarian",				"Barbarian",				  1030000	},
		{	"Barbarian",				"Barbarian",				  1270000	},
		{	"Barbarian",				"Barbarian",				  1570000	},
		{	"Barbarian",				"Barbarian",				  1930000	},
		{	"Barbarian",				"Barbarian",				  2370000	},	/* 30 */
		{	"Barbarian",				"Barbarian",				  2920000	},
		{	"Barbarian",				"Barbarian",				  3590000	},
		{	"Barbarian",				"Barbarian",				  4420000	},
		{	"Barbarian",				"Barbarian",				  5440000	},
		{	"Barbarian",				"Barbarian",				  6700000	},
		{	"Barbarian",				"Barbarian",				  8240000	},
		{	"Barbarian",				"Barbarian",				 10200000	},
		{	"Barbarian",				"Barbarian",				 12500000	},
		{	"Barbarian",				"Barbarian",				 15300000	},
		{	"Barbarian",				"Barbarian",				 18900000	},	/* 40 */
		{	"Barbarian",				"Barbarian",				 23200000	},
		{	"Barbarian",				"Barbarian",				 28600000	},
		{	"Barbarian",				"Barbarian",				 35200000	},
		{	"Barbarian",				"Barbarian",				 43300000	},
		{	"Barbarian",				"Barbarian",				 53200000	},	/* 45 */
		{	"Barbarian",				"Barbarian",				 65500000	},
		{	"Barbarian",				"Barbarian",				 80600000	},
		{	"Barbarian",				"Barbarian",				 99100000	},
		{	"Conan",					"Sonja",					122000000	},
		{	"Khan",						"Hun",						150000000	}, /* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* sorcerer */
		{	"Man",						"Woman",							0	},	/* Variati da EleiMiShill */
		{	"SpellWeaver",				"SpellWeaver",						1	},
		{	"Bookworm",					"Bookworm",						 1600	},
		{	"BookStudent",				"BookStudent",					 3300	},
		{	"Scholar",					"Scholar",						 5300	},
		{	"Soothsayer",				"Soothsayer",					 7800	},
		{	"Illusionist",				"Illusionist",					11000	},
		{	"Warlock",					"Witch",						14800	},
		{	"Apparitionist",			"Apparitionist",				19400	},
		{	"Medium",					"Gypsy",						25000	},
		{	"Scribe",					"Scribe",						32000	},	/* 10 */
		{	"Sage",						"Sage",							41000	},
		{	"Seer",						"Seeress",						52000	},
		{	"Divinater",				"Divinatress",					65000	},
		{	"Alchemist",				"Alchemist",					81000	},
		{	"Evoker",					"Evoker",					   100000	},
		{	"Necromancer",				"Necromancer",				   125000	},
		{	"Abjurer",					"Abjuress",					   155000	},
		{	"Invoker",					"Invoker",					   192000	},
		{	"Enchanter",				"Enchantress",				   238000	},
		{	"Conjurer",					"Conjuress",				   294000	},	/* 20 */
		{	"Summoner",					"Summoner",					   363000	},
		{	"Magician",					"Magician",					   447000	},
		{	"Spiritualist",				"Spiritualist",				   552000	},
		{	"Savant",					"Savanti",					   680000	},
		{	"Shaman",					"Shamaness",				   838000	},
		{	"Mystic",					"Mystic",					  1030000	},
		{	"Wiccen",					"Wicca",					  1270000	},
		{	"Mentalist",				"Mentalist",				  1570000	},
		{	"Mnemonicist",				"Mnemonicist",				  1930000	},
		{	"Nueromancer",				"Nueromancer",				  2370000	},	/* 30 */
		{	"Spellbinder",				"Spellbinder",				  2920000	},
		{	"Synergist",				"Synergist",				  3590000	},
		{	"Oculist",					"Oculist",					  4420000	},
		{	"Thaumaturgist",			"Thaumaturgist",			  5440000	},
		{	"SpellMaster",				"Spell Mistress",			  6700000	},
		{	"Dispeller",				"Dispeller",				  8240000	},
		{	"Warlock",					"War Witch",				 10200000	},
		{	"Sorcerer",					"Sorceress",				 12500000	},
		{	"Wizard",					"Wizard",					 15300000	},
		{	"Grand Sorcerer",			"Grand Sorceress",			 18900000	},	/* 40 */
		{	"Magus",					"Incantrix",				 23200000	},
		{	"Serpent Mage",				"Serpent Mage",				 28600000	},
		{	"Spell Master",				"Spell Mistress",			 35200000	},
		{	"Spell Master",				"Spell Mistress",			 43300000	},
		{	"Spell Master",				"Spell Mistress",			 53200000	},	/* 45 */
		{	"Spell Master",				"Spell Mistress",			 65500000	},
		{	"Spell Master",				"Spell Mistress",			 80600000	},
		{	"Spell Master",				"Spell Mistress",			 99100000	},
		{	"Spell Master",				"Spell Mistress",			122000000	},
		{	"Spell Master",				"Spell Mistress",			150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* paladin */
		{	"Man",						"Woman",							0	},	/* Variati  */
		{	"Holy Page",				"Holy Page",						1	},	/* da EleiMiShill */
		{	"Holy Squire",				"Holy Squire",					 1600	},
		{	"Holy Warrior",				"Holy Warrior",					 3300	},
		{	"Protector",				"Protector",					 5300	},
		{	"Defender",					"Defender",						 7800	},
		{	"Crusader",					"Crusader",						11000	},
		{	"Holy Brother",				"Holy Sister",					14800	},
		{	"Knight",					"Knight",						19400	},
		{	"Knight Defender",			"Knight Defender",				25000	},
		{	"Knight Crusader",			"Knight Crusader",				32000	},	/* 10 */
		{	"Bodyguard",				"Bodyguard",					41000	},
		{	"Paladin",					"Paladin",						52000	},
		{	"Paladin",					"Paladin",						65000	},
		{	"Paladin",					"Paladin",						81000	},
		{	"Paladin",					"Paladin",					   100000	},
		{	"Paladin",					"Paladin",					   125000	},
		{	"Paladin",					"Paladin",					   155000	},
		{	"Paladin",					"Paladin",					   192000	},
		{	"Paladin",					"Paladin",					   238000	},
		{	"Grand Knight",				"Grand Knight",				   294000	},	/* 20 */
		{	"Grand Knight",				"Grand Knight",				   363000	},
		{	"Grand Knight",				"Grand Knight",				   447000	},
		{	"Grand Knight",				"Grand Knight",				   552000	},
		{	"Grand Knight",				"Grand Knight",				   680000	},
		{	"Grand Knight",				"Grand Knight",				   838000	},
		{	"Grand Knight",				"Grand Knight",				  1030000	},
		{	"Grand Knight",				"Grand Knight",				  1270000	},
		{	"Grand Knight",				"Grand Knight",				  1570000	},
		{	"Grand Knight",				"Grand Knight",				  1930000	},
		{	"Paladin Defender",			"Paladin Defender",			  2370000	},	/* 30 */
		{	"Paladin Defender",			"Paladin Defender",			  2920000	},
		{	"Paladin Defender",			"Paladin Defender",			  3590000	},
		{	"Paladin Defender",			"Paladin Defender",			  4420000	},
		{	"Paladin Defender",			"Paladin Defender",			  5440000	},
		{	"Paladin Defender",			"Paladin Defender",			  6700000	},
		{	"Paladin Defender",			"Paladin Defender",			  8240000	},
		{	"Paladin Defender",			"Paladin Defender",			 10200000	},
		{	"Paladin Defender",			"Paladin Defender",			 12500000	},
		{	"Paladin Defender",			"Paladin Defender",			 15300000	},
		{	"Paladin Defender",			"Paladin Defender",			 18900000	},	/* 40 */
		{	"Paladin Crusader",			"Paladin Crusader",			 23200000	},
		{	"Paladin Crusader",			"Paladin Crusader",			 28600000	},
		{	"Paladin Crusader",			"Paladin Crusader",			 35200000	},
		{	"Paladin Crusader",			"Paladin Crusader",			 43300000	},
		{	"Paladin Crusader",			"Paladin Crusader",			 53200000	},	/* 45 */
		{	"Paladin Crusader",			"Paladin Crusader",			 65500000	},
		{	"Paladin Crusader",			"Paladin Crusader",			 80600000	},
		{	"Paladin Crusader",			"Paladin Crusader",			 99100000	},
		{	"Paladin Crusader",			"Paladin Crusader",			122000000	},
		{	"Grand Paladin",			"Grand Paladin",			150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* ranger */
		{	"Man",						"Woman",							0	},	/* Variati */
		{	"Woodsman",					"Woodswoman",						1	},	/* da EleiMiShill */
		{	"Brother",					"Sister",						 1600	},
		{	"Woodland Brother",			"Woodland Sister",				 3300	},
		{	"Forest Watcher",			"Forest Watcher",				 5300	},
		{	"Forest Protector",			"Forest Protector",				 7800	},
		{	"Forest Guardian",			"Forest Guardian",				11000	},
		{	"Pathseeker",				"Pathseeker",					14800	},
		{	"Pathfinder",				"Pathfinder",					19400	},
		{	"Pathmaker",				"Pathmaker",					25000	},
		{	"Ranger",					"Ranger",						32000	},	/* 10 */
		{	"Wanderer",					"Wanderer",						41000	},
		{	"Forester",					"Forester",						52000	},
		{	"Gatherer",					"Gatherer",						65000	},
		{	"Guardian",					"Guardian",						81000	},
		{	"Searcher",					"Searcher",					   100000	},
		{	"Freeman",					"Freewoman",				   125000	},
		{	"Trapper",					"Trapper",					   155000	},
		{	"Huntsman",					"Huntswoman",				   192000	},
		{	"Frontiersman",				"Frontierswoman",			   238000	},
		{	"Forest Brother",			"Forest Sister",			   294000	},	/* 20 */
		{	"Bounty Hunter",			"Bounty Hunter",			   363000	},
		{	"Animal Tamer",				"Animal Tamer",				   447000	},
		{	"Scout",					"Scout",					   552000	},
		{	"Explorer",					"Explorer",					   680000	},
		{	"Archer",					"Archer",					   838000	},
		{	"Walker",					"Walker",					  1030000	},
		{	"Strider",					"Strider",					  1270000	},
		{	"Bushman",					"Bushwoman",				  1570000	},
		{	"Runner",					"Runner",					  1930000	},
		{	"Tracker",					"Tracker",					  2370000	},	/* 30 */
		{	"Forest Ranger",			"Forest Ranger",			  2920000	},
		{	"Forest Ranger",			"Forest Ranger",			  3590000	},
		{	"Forest Ranger",			"Forest Ranger",			  4420000	},
		{	"Forest Ranger",			"Forest Ranger",			  5440000	},
		{	"Forest Ranger",			"Forest Ranger",			  6700000	},
		{	"Forest Ranger",			"Forest Ranger",			  8240000	},
		{	"Forest Ranger",			"Forest Ranger",			 10200000	},
		{	"Forest Ranger",			"Forest Ranger",			 12500000	},
		{	"Forest Ranger",			"Forest Ranger",			 15300000	},
		{	"Forest Ranger",			"Forest Ranger",			 18900000	},	/* 40 */
		{	"Forest Ranger",			"Forest Ranger",			 23200000	},
		{	"Forest Ranger",			"Forest Ranger",			 28600000	},
		{	"Forest Ranger",			"Forest Ranger",			 35200000	},
		{	"Forest Ranger",			"Forest Ranger",			 43300000	},
		{	"Forest Ranger",			"Forest Ranger",			 53200000	},	/* 45 */
		{	"Forest Ranger",			"Forest Ranger",			 65500000	},
		{	"Forest Ranger",			"Forest Ranger",			 80600000	},
		{	"Forest Ranger",			"Forest Ranger",			 99100000	},
		{	"Forest Ranger",			"Forest Ranger",			122000000	},
		{	"Forest Ranger",			"Forest Ranger",			150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	},

	{		/* psionist */
		{	"Man",						"Woman",							0	},	/*  */
		{	"Brain",					"Brain",							1	},	/**/
		{	"Seeker",					"Seeker",						 1600	},
		{	"Finder",					"Finder",						 3300	},
		{	"Psycho",					"Psycho",						 5300	},
		{	"Psychic",					"Psychic",						 7800	},
		{	"Telepath",					"Telepath",						11000	},
		{	"Controller",				"Controller",					14800	},
		{	"Mind Bender",				"Mind Bender",					19400	},
		{	"Palm Reader",				"Palm Reader",					25000	},
		{	"Fortune Teller",			"Fortune Teller",				32000	},	/* 10 */
		{	"Sensor",					"Sensor",						41000	},
		{	"Divinator",				"Divinator",					52000	},
		{	"Clairsentient",			"Clairsentient",				65000	},
		{	"Firestarter",				"Firestarter",					81000	},
		{	"Metapsionic",				"Metapsionic",				   100000	},
		{	"Psionic Student",			"Psionic Student",			   125000	},
		{	"Psionic Tutor",			"Psionic Tutor",			   155000	},
		{	"Psionic Instructor",		"Psionic Instructor",		   192000	},
		{	"Psionic Teacher",			"Psionic Teacher",			   238000	},
		{	"Grand Psionic",			"Grand Psionic",			   294000	},	/* 20 */
		{	"Psionist",					"Psionist",					   363000	},
		{	"Psionist",					"Psionist",					   447000	},
		{	"Psionist",					"Psionist",					   552000	},
		{	"Psionist",					"Psionist",					   680000	},
		{	"Psionist",					"Psionist",					   838000	},
		{	"Psionist",					"Psionist",					  1030000	},
		{	"Psionist",					"Psionist",					  1270000	},
		{	"Psionist",					"Psionist",					  1570000	},
		{	"Psionist",					"Psionist",					  1930000	},
		{	"Psionist",					"Psionist",					  2370000	},	/* 30 */
		{	"Psionist",					"Psionist",					  2920000	},
		{	"Psionist",					"Psionist",					  3590000	},
		{	"Psionist",					"Psionist",					  4420000	},
		{	"Psionist",					"Psionist",					  5440000	},
		{	"Psionist",					"Psionist",					  6700000	},
		{	"Psionist",					"Psionist",					  8240000	},
		{	"Psionist",					"Psionist",					 10200000	},
		{	"Psionist",					"Psionist",					 12500000	},
		{	"Psionist",					"Psionist",					 15300000	},
		{	"Psionist",					"Psionist",					 18900000	},	/* 40 */
		{	"Psionist",					"Psionist",					 23200000	},
		{	"Psionist",					"Psionist",					 28600000	},
		{	"Psionist",					"Psionist",					 35200000	},
		{	"Psionist",					"Psionist",					 43300000	},
		{	"Psionist",					"Psionist",					 53200000	},	/* 45 */
		{	"Psionist",					"Psionist",					 65500000	},
		{	"Psionist",					"Psionist",					 80600000	},
		{	"Psionist",					"Psionist",					 99100000	},
		{	"Psionist",					"Psionist",					122000000	},
		{	"Psionist",					"Psionist",					150000000	},	/* 50 */
		{	"Principe",					"Principessa",				400000000	},
		{	"Immortale",				"Immortale",				431000000	},
		{	"Dio Minore",				"Dea Minore",				432000000	},
		{	"Dio",						"Dea",						433000000	},
		{	"Maestro degli Dei",		"Maestra degli Dei",		434000000	},
		{	"Creatore",					"Creatrice",				435000000	},
		{	"Maestro del Fato",			"Maestra del Fato",			436000000	},
		{	"Maestro del Creato",		"Maestra del Creato",		437000000	},
		{	"Maestro dei Creatori",		"Maestra dei Creatori",		438000000	},
		{	"Colui che tutto decide",	"Colei che tutto decide",	439000000	}	/* 60 */
	}

};

const char* RaceName[] = {
	"Half-Breed",
	"Human",
	"Moon-Elf",
	"Dwarven",
	"Halfling",
	"Gnome",
	"Reptilian",
	"Mysterion",
	"Lycanthropian",
	"Draconian",
	"Undead",
	"Orcish",
	"Insectoid",
	"Arachnoid",
	"Saurian",
	"Icthyiod",
	"Avian",
	"Giant",
	"Carnivororous",
	"Parasitic",
	"Slime",
	"Demonic",
	"Snake",
	"Herbivorous",
	"Tree",
	"Vegan",
	"Elemental",
	"Planar",
	"Diabolic",
	"Ghostly",
	"Goblinoid",
	"Trollish",
	"Vegman",
	"Mindflayer",
	"Primate",
	"Enfan",
	"Dark-Elf",
	"Golem",
	"Skexie",
	"Troglodyte",
	"Patryn",
	"Labrynthian",
	"Sartan",
	"Tytan",
	"Smurf",
	"Kangaroo",
	"Horse",
	"Ratperson",
	"Astralion",
	"God",
	"Giant Hill",
	"Giant Frost",
	"Giant Fire",
	"Giant Cloud",
	"Giant Storm",
	"Giant Stone",
	"Dragon Red",
	"Dragon Black",
	"Dragon Green",
	"Dragon White",
	"Dragon Blue",
	"Dragon Silver",
	"Dragon Gold",
	"Dragon Bronze",
	"Dragon Copper",
	"Dragon Brass",
	"Undead Vampire",
	"Undead Lich",
	"Undead Wight",
	"Undead Ghast",
	"Undead Spectre",
	"Undead Zombie",
	"Undead Skeleton",
	"Undead Ghoul",
	"Half-Elven",
	"Half-Ogre",
	"Half-Orc",
	"Half-Giant",
	"Lizardman",
	"Dark-Dwarf",
	"Deep-Gnome",
	"Gnoll",
	"Gold-Elf",
	"Wild-Elf",
	"Sea-Elf",
	"\n"
};

const char* item_types[] = {
	"UNDEFINED",
	"LIGHT",
	"SCROLL",
	"WAND",
	"STAFF",
	"WEAPON",
	"FIRE WEAPON",
	"MISSILE",
	"TREASURE",
	"ARMOR",
	"POTION",
	"WORN",
	"OTHER",
	"TRASH",
	"TRAP",
	"CONTAINER",
	"NOTE",
	"LIQUID CONTAINER",
	"KEY",
	"FOOD",
	"MONEY",
	"PEN",
	"BOAT",
	"AUDIO",
	"BOARD",
	"TREE",
	"ROCK",
	"MINED GEM",
	"MINED MINERAL",
	"BAR",
	"JEWEL",
	"\n"
};

const char* item_types_ita[] = {
	"Nessuno",
	"Luce",
	"Pergamena",
	"Bacchetta",
	"Bastone",
	"Arma",
	"Arma da Tiro",
	"Proiettile",
	"Tesoro",
	"Armatura",
	"Pozione",
	"Logoro",
	"Altro",
	"Spazzatura",
	"Trappola",
	"Contenitore",
	"Appunto",
	"Contenitore per Bevande",
	"Chiave",
	"Cibo",
	"Monete",
	"Penna",
	"Nave",
	"Audio",
	"Bacheca",
	"Albero",
	"Roccia",
	"Gemma",
	"Minerale",
	"Lingotto",
	"Gioiello",
	"\n"
};

const char* wear_bits[] = {
	"TAKE",
	"FINGER",
	"NECK",
	"BODY",
	"HEAD",
	"LEGS",
	"FEET",
	"HANDS",
	"ARMS",
	"SHIELD",
	"ABOUT",
	"WAIST",
	"WRIST",
	"WIELD",
	"HOLD",
	"THROW",
	"LIGHT-SOURCE",
	"BACK",
	"EARS",
	"EYES",
	"\n"
};

const char* extra_bits[] = {
	"GLOW",
	"HUM",
	"METAL",
	"MINERAL",
	"ORGANIC",
	"INVISIBLE",
	"MAGIC",
	"NODROP",
	"BLESS",
	"ANTI-GOOD",
	"ANTI-EVIL",
	"ANTI-NEUTRAL",
	"ANTI-CLERIC",
	"ANTI-MAGE",
	"ANTI-THIEF",
	"ANTI-WARRIOR",
	"BRITTLE",
	"RESISTANT",
	"ARTIFACT",
	"ANTI-MEN",
	"ANTI-WOMEN",
	"ANTI-SUN",
	"ANTI-BARBARIAN",
	"ANTI-RANGER",
	"ANTI-PALADIN",
	"ANTI-PSIONIST",
	"ANTI-MONK",
	"ANTI-DRUID",
	"ONLY-CLASS",
	"DIG",
	"SCYTHE",
	"ANTI-SORCERER",
	"\n"
};

const char* extra_bits2[] = {
	"QUEST-ITEM",
	"EDIT",
	"NO-LOCATE",
	"PERSONAL",
	"HAS-GEMS",
	"NO-PRINCE",
	"ONLY-PRINCE",
	"\n"
};

const char* room_bits[] = {
	"DARK",
	"DEATH",
	"NO_MOB",
	"INDOORS",
	"PEACEFUL",
	"NOSTEAL",
	"NO_SUM",
	"NO_MAGIC",
	"TUNNEL",
	"PRIVATE",
	"SILENCE",
	"LARGE",
	"NO_DEATH",
	"SAVE_ROOM",
	"NO_TRACK",
	"NO_MIND",
	"DESERTIC",
	"ARTIC",
	"UNDERGROUND",
	"HOT",
	"WET",
	"COLD",
	"DRY",
	"BRIGHT",
	"NO_ASTRAL",
	"NO_REGAIN",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"\n"
};

const char* exit_bits[] = {
	"ISDOOR",
	"CLOSED",
	"LOCKED",
	"SECRECT",
	"NOTBASH",
	"PICKPROOF",
	"CLIMB",
	"MALE",
	"NOLOOKT",
	"\n"
};

const char* sector_types[] = {
	"Inside",
	"City",
	"Field",
	"Forest",
	"Hills",
	"Mountains",
	"Water Swim",
	"Water NoSwim",
	"Air",
	"Underwater",
	"Desert",
	"Tree",
	"Dark City",
	"\n"
};

const char* equipment_types[] = {
	"Special",
	"Worn on right finger",
	"Worn on left finger",
	"First worn around Neck",
	"Second worn around Neck",
	"Worn on body",
	"Worn on head",
	"Worn on legs",
	"Worn on feet",
	"Worn on hands",
	"Worn on arms",
	"Worn as shield",
	"Worn about body",
	"Worn around waist",
	"Worn around right wrist",
	"Worn around left wrist",
	"Wielded",
	"Held",
	"Worn on back",
	"Worn in right ear",
	"Worn in left ear",
	"Worn on eyes",
	"\n"
};

const char* affected_bits[] = {
	"BLIND",
	"INVISIBLE",
	"DETECT-EVIL",
	"DETECT-INVISIBLE",
	"DETECT-MAGIC",
	"SENCE-LIFE",
	"LIFE-PROT",
	"SANCTUARY",
	"DRAGON RIDE",
	"GROWTH",
	"CURSE",
	"FLYING",
	"POISON",
	"TREE TRAVEL",
	"PARALYSIS",
	"INFRAVISION",
	"WATER-BREATH",
	"SLEEP",
	"TRAVELLING",
	"SNEAK",
	"HIDE",
	"SILENCE",
	"CHARM",
	"FOLLOW",
	"PROTECT-FROM-EVIL",
	"TRUE SIGHT",
	"SCRYING",
	"FIRESHIELD",
	"GROUP",
	"TELEPATHY",
	"DARKNESS",
	"UNDEFINED",
	"\n"
};

const char* affected_bits2[] = {
	"Invisible to animals",
	"Heat Stuff",
	"Logging",
	"Berserk",
	"Parry",
	"Group-Order",
	"AWAY-FROM-KEYBOARD",
	"Pkilled someone",
	"DANGER-SENSE", 	// Montero 16-Sep-2018 - danger sense
	"QUEST",
	"\n"
};

const char* special_user_flags[] = {
	"NO-DELETE",
	"USE-ANSI",
	"RACE-WARRIOR",
	"SHOW-CLASSES",
	"SHOW-EXITS",
	"MURDER-1",
	"STOLE-1",
	"MURDER-2",
	"STOLE-2",
	"MURDER-X",
	"STOLE-X",
	"PAGE-PAUSE",
	"OBJECT-EDITOR-OK",
	"MOBILE-EDITOR-OK",
	"PWP_MODE",
	"ACHIE_SHOW",
	"WARNINGS_OFF",
	"\n"
};

const char* immunity_names[] = {
	"FIRE",
	"COLD",
	"ELECTRICITY",
	"ENERGY",
	"BLUNT",
	"PIERCE",
	"SLASH",
	"ACID",
	"POISON",
	"DRAIN",
	"SLEEP",
	"CHARM",
	"HOLD",
	"NON-MAGIC",
	"+1",
	"+2",
	"+3",
	"+4",
	"\n"
};

const char* apply_types[] = {
	"NONE",
	"STR",
	"DEX",
	"INT",
	"WIS",
	"CON",
	"CHR",
	"SEX",
	"LEVEL",
	"AGE",
	"CHAR_WEIGHT",
	"CHAR_HEIGHT",
	"MANA",
	"HIT",
	"MOVE",
	"GOLD",
	"EXP",
	"ARMOR",
	"HITROLL",
	"DAMROLL",
	"SAVING_PARA",
	"SAVING_ROD",
	"SAVING_PETRI",
	"SAVING_BREATH",
	"SAVING_SPELL",
	"SAVING_ALL",
	"RESISTANCE",
	"SUSCEPTIBILITY",
	"IMMUNITY",
	"SPELL AFFECT",
	"WEAPON SPELL",
	"EAT SPELL",
	"BACKSTAB",
	"KICK",
	"SNEAK",
	"HIDE",
	"BASH",
	"PICK",
	"STEAL",
	"TRACK",
	"HIT-N-DAM",
	"SPELLFAIL",
	"ATTACKS",
	"HASTE",
	"SLOW",
	"SPELL AFFECT 2",
	"FIND-TRAPS",
	"RIDE",
	"RACE-SLAYER",
	"ALIGN-SLAYER",
	"MANA-REGEN",
	"HIT-REGEN",
	"MOVE-REGEN",
	"MOD-THIRST",
	"MOD-HUNGER",
	"MOD-DRUNK",
	"T_STR",
	"T_INT",
	"T_DEX",
	"T_WIS",
	"T_CON",
	"T_CHR",
	"T_HPS",
	"T_MOVE",
	"T_MANA",
	"SPELLPOWER",
	"HIT-N-SP",
	"\n"
};

const char* pc_class_types[] = {
	"Magic user",
	"Cleric",
	"Warrior",
	"Thief",
	"Druid",
	"Monk",
	"Barbarian",
	"Sorcerer",
	"Paladin",
	"Ranger",
	"Psionist",
	"\n"
};

const char* npc_class_types[] = {
	"Normal",
	"Undead",
	"\n"
};

const char* system_flag_types [] = {
	"NO-PORTAL",
	"NO-ASTRAL",
	"NO-SUMMON",
	"NO-KILL",
	"LOG-ALL",
	"ECLIPSE",
	"NO-DNS",
	"REQ-APPROVAL",
	"NO-COLOR",
	"LOG-MOB",
	"\n"
};

const char* aszLogMessagesType[] = {
	"SYSERR",
	"CHECK",
	"PLAYERS",
	"MOBILES",
	"CONNECT",
	"ERROR",
	"WHO",
	"SAVE",
	"MAIL",
	"RANK",
	"WORLD",
	"QUERY",
	"\n"
};

const char* action_bits[] = {
	"SPEC",
	"SENTINEL",
	"SCAVENGER",
	"ISNPC",
	"NICE-THIEF",
	"AGGRESSIVE",
	"STAY-ZONE",
	"WIMPY",
	"ANNOYING",
	"HATEFUL",
	"AFRAID",
	"IMMORTAL",
	"HUNTING",
	"DEADLY",
	"POLYMORPHED",
	"META_AGGRESSIVE",
	"GUARDING",
	"ILLUSION",
	"HUGE",
	"SCRIPT",
	"GREET",
	"MAGIC-USER",
	"WARRIOR",
	"CLERIC",
	"THIEF",
	"DRUID",
	"MONK",
	"BARBARIAN",
	"PALADIN",
	"RANGER",
	"PSIONIST",
	"ARCHER",
	"\n"
};


const char* player_bits[] = {
	"BRIEF",
	"EMPTY3",
	"COMPACT",
	"DONTSET",
	"WIMPY",
	"NOHASSLE",
	"STEALTH",
	"HUNTING",
	"DEAF",
	"ECHO",
	"NOGOSSIP",
	"NOAUCTION",
	"NONE1",
	"NONE2",
	"NOSHOUT",
	"FREEZE",
	"NOTELL",
	"MAILING",
	"HAS_NEW_EQ",
	"HAS_ACHIEVEMENT",
	"NOBEEP",
	"\n"
};


const char* position_types[] = {
	"Dead",
	"Mortally wounded",
	"Incapacitated",
	"Stunned",
	"Sleeping",
	"Resting",
	"Sitting",
	"Fighting",
	"Standing",
    "Mounted",
	"\n"
};

const char* connected_types[] = {
	"Playing",
	"Get name",
	"Confirm name",
	"Read Password",
	"Get new password",
	"Confirm new password",
	"Get sex",
	"Read messages of today",
	"Read Menu",
	"Get extra description",
	"Get class",
	"Link Dead",
	"New Password",
	"Password Confirm",
	"Wizlocked",
	"Get Race",
	"Racpar",
	"Auth",
	"City Choice",
	"Stat Order",
	"Delete",
	"Delete",
	"Stat Order",
	"Wizard MOTD",
	"Editing",
	"Nuking Theirself",
	"Mage/Sorcerer Question",
	"Object Editing",
	"Mob Editing",
	"Read Newbie message",
	"Read race help",
	"Read race help",
	"Read class help",
	"Read class help",
	"New Roll help",
	"New Roll choose",
	"Extra 1",
	"Extra 2",
	"\n"
};

const char* aszWeaponType[] = {
	"SMITE",
	"STAB",
	"WHIP",
	"SLASH",
	"SMASH",
	"CLEAVE",
	"CRUSH",
	"BLUDGEON",
	"CLAW",
	"BITE",
	"STING",
	"PIERCE",
	"BLAST",
	"RANGE_WEAPON",
	"\n"
};

const char* aszWeaponSpecialEffect[] = {
	"RESERVED",
	"FOR_GIANT",
	"\n"
};

const char* gaszAlignSlayerBits[] = {
	"GOOD",
	"NEUTRAL",
	"EVIL",
	"\n"
};


/* [class], [level] (all) */
int thaco[MAX_CLASS][ABS_MAX_LVL] = {
	/* mage */
	{
	   100,	20,	20,	20,	20,	20,	19,	19,	19,	19,
		19,	18,	18,	18,	18,	18,	17,	17,	17,	17,
		17,	16,	16,	16,	16,	16,	15,	15,	15,	15,
		15,	14,	14,	14,	14,	14,	13,	13,	13,	13,
		13,	12,	12,	12,	12,	12,	11,	11,	11,	11,
		11,	10,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* cleric */
	{
	   100,	20,	20,	20,	19,	19,	19,	18,	18,	18,
		17,	17,	17,	16,	16,	16,	15,	15,	15,	14,
		14,	14,	13,	13,	13,	12,	12,	12,	11,	11,
		11,	10,	10,	10,	 9,	 9,	 9,	 8,	 8,	 8,
		 7,	 7,	 7,	 6,	 6,	 6,	 5,	 5,	 5,	 4,
		 4,	 3,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* fighter */
	{
	   100,	20,	20,	19,	19,	18,	18,	17,	17,	16,
		16,	15,	15,	14,	14,	13,	13,	12,	12,	11,
		11,	10,	10,	 9,	 9,	 8,	 8,	 7,	 7,	 6,
		 6,	 5,	 5,	 4,	 4,	 3,	 3,	 2,	 2,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* thief */
	{
	   100,	20,	20,	20,	19,	19,	19,	19,	18,	18,
		18,	17,	17,	17,	17,	16,	16,	16,	15,	15,
		15,	15,	14,	14,	14,	13,	13,	13,	13,	12,
		12,	12,	11,	11,	11,	11,	10,	10,	10,	 9,
		 9,	 9,	 9,	 8,	 8,	 8,	 7,	 7,	 7,	 6,
		 5,	 3,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* druid */
	{
	   100,	20,	20,	20,	19,	19,	19,	18,	18,	18,
		17,	17,	17,	16,	16,	16,	15,	15,	15,	14,
		14,	14,	13,	13,	13,	12,	12,	12,	11,	11,
		11,	10,	10,	10,	 9,	 9,	 9,	 8,	 8,	 8,
		 7,	 7,	 7,	 6,	 6,	 6,	 5,	 5,	 5,	 4,
		 4,	 4,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* monk  */
	{
	   100,	20,	20,	19,	19,	18,	18,	17,	17,	16,
		16,	15,	15,	14,	14,	13,	13,	12,	12,	11,
		11,	10,	10,	 9,	 9,	 8,	 8,	 7,	 7,	 6,
		 6,	 5,	 5,	 4,	 4,	 3,	 3,	 2,	 2,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* barbarian */
	{
	   100,	20,	20,	19,	19,	18,	18,	17,	17,	16,
		16,	15,	15,	14,	14,	13,	13,	12,	12,	11,
		11,	10,	10,	 9,	 9,	 8,	 8,	 7,	 7,	 6,
		 6,	 5,	 5,	 4,	 4,	 3,	 3,	 2,	 2,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* sorcerer */
	{
	   100,	20,	20,	20,	20,	20,	19,	19,	19,	19,
		19,	18,	18,	18,	18,	18,	17,	17,	17,	17,
		17,	16,	16,	16,	16,	16,	15,	15,	15,	15,
		15,	14,	14,	14,	14,	14,	13,	13,	13,	13,
		13,	12,	12,	12,	12,	12,	11,	11,	11,	11,
		11,	10,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* paladin */
	{
	   100,	20,	20,	19,	19,	18,	18,	17,	17,	16,
		16,	15,	15,	14,	14,	13,	13,	12,	12,	11,
		11,	10,	10,	 9,	 9,	 8,	 8,	 7,	 7,	 6,
		 6,	 5,	 5,	 4,	 4,	 3,	 3,	 2,	 2,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* ranger */
	{
	   100,	20,	20,	19,	19,	18,	18,	17,	17,	16,
		16,	15,	15,	14,	14,	13,	13,	12,	12,	11,
		11,	10,	10,	 9,	 9,	 8,	 8,	 7,	 7,	 6,
		 6,	 5,	 5,	 4,	 4,	 3,	 3,	 2,	 2,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	},
	/* psionist */
	{
	   100,	20,	20,	20,	19,	19,	19,	19,	18,	18,
		18,	17,	17,	17,	17,	16,	16,	16,	15,	15,
		15,	15,	14,	14,	14,	13,	13,	13,	13,	12,
		12,	12,	11,	11,	11,	11,	10,	10,	10,	 9,
		 9,	 9,	 9,	 8,	 8,	 8,	 7,	 7,	 7,	 7,
		 6,	 5,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
		 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1
	}

};

/* [ch] strength apply (all) */
struct str_app_type str_app[31] = {
// 		to_hit,	to_dam,	carry_w, wield_w
	{		-5,		-4,		 0,		 0	},	/* 0  */
	{		-5,		-4,		 1,		 1	},	/* 1  */
	{		-3,		-2,		 1,		 2	},
	{		-3,		-1,		 5,		 3	},	/* 3  */
	{		-2,		-1,		10,		 4	},
	{		-2,		-1,		20,		 5	},	/* 5  */
	{		-1,		 0,		25,		 6	},
	{		-1,		 0,		30,		 7	},
	{		 0,		 0,		40,		 8	},
	{		 0,		 0,		50,		 9	},
	{		 0,		 0,		55,		10	},	/* 10  */
	{		 0,		 0,		70,		11	},
	{		 0,		 0,		80,		12	},
	{		 0,		 0,		90,		13	},
	{		 0,		 0,	   100,		14	},
	{		 0,		 0,	   110,		15	},	/* 15  */
	{		 0,		 1,	   120,		16	},
	{		 1,		 1,	   130,		18	},
	{		 1,		 2,	   140,		20	},	/* 18  */
	{		 3,		 7,	   485,		40	},
	{		 3,		 8,	   535,		40	},	/* 20  */
	{		 4,		 9,	   635,		40	},
	{		 4,		10,	   785,		40	},
	{		 5,		11,	   935,		40	},
	{		 6,		12,	  1235,		40	},
	{		 7,		14,	  1535,		40	},	/* 25  */
	{		 1,		 3,	   155,		22	},	/* 18/01-50    */
	{		 2,		 3,	   170,		24	},	/* 18/51-75    */
	{		 2,		 4,	   185,		26	},	/* 18/76-90    */
	{		 2,		 5,	   255,		28	},	/* 18/91-99    */
	{		 3,		 6,	   355,		30	} 	/* 18/100 (30) */
};

/* [dex] skillapply (thieves only) */
struct dex_skill_type dex_app_skill[26] = {
	{	-99,	-99,	-90,	-99,	-60	},	/* 0 */
	{	-90,	-90,	-60,	-90,	-50	},	/* 1 */
	{	-80,	-80,	-40,	-80,	-45	},
	{	-70,	-70,	-30,	-70,	-40	},
	{	-60,	-60,	-30,	-60,	-35	},
	{	-50,	-50,	-20,	-50,	-30	},	/* 5 */
	{	-40,	-40,	-20,	-40,	-25	},
	{	-30,	-30,	-15,	-30,	-20	},
	{	-20,	-20,	-15,	-20,	-15	},
	{	-15,	-10,	-10,	-20,	-10	},
	{	-10,	 -5,	-10,	-15,	 -5	},	/* 10 */
	{	 -5,	  0,	 -5,	-10,	  0	},
	{	 -2,	  0,	  0,	 -5,	  0	},
	{	 -1,	  0,	  0,	  0,	  0	},
	{	  0,	  0,	  0,	  0,	  0	},
	{	  1,	  0,	  0,	  0,	  0	},	/* 15 */
	{	  2,	  5,	  0,	  0,	  0	},
	{	  5,	 10,	  0,	  5,	  5	},
	{	 10,	 15,	  5,	 10,	 10	},
	{	 15,	 20,	 10,	 15,	 15	},
	{	 15,	 20,	 10,	 15,	 15	},	/* 20 */
	{	 20,	 25,	 10,	 15,	 20	},
	{	 20,	 25,	 15,	 20,	 20	},
	{	 25,	 25,	 15,	 20,	 20	},
	{	 25,	 30,	 15,	 25,	 25	},
	{	 25,	 30,	 15,	 25,	 25	}	/* 25 */
};

/* [level] backstab multiplyer (thieves only) */
byte backstab_mult[ABS_MAX_LVL] = {
	 1,	/* 0 */
	 2,	/* 1 */
	 2,
	 2,
	 2,
	 2,	/* 5 */
	 2,
	 2,
	 3,	/* 8 */
	 3,
	 3,	/* 10 */
	 3,
	 3,
	 3,
	 3,
	 3,	/* 15 */
	 4,	/* 16 */
	 4,
	 4,
	 4,
	 4,	/* 20 */
	 4,
	 4,
	 4,
	 5,	/* 25 */
	 5,
	 5,
	 5,
	 5,
	 5,	/* 30 */
	 5,
	 5,
	 6,
	 6,
	 6,	/* 35 */
	 6,
	 6,
	 6,
	 7,
	 7,	/* 40 */
	 7,
	 7,
	 7,
	 8,
	 8,	/* 45 */
	 8,
	 8,
	 8,
	 9,
	10,	/* 50? */
	12,
	13,
	13,
	13,
	13,
	14,
	15,
	16,
	17,
	18,	/* 60 */
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28	/* 70 */

};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
	{	-7,	-7,	 60	},	/* 0 */
	{	-6,	-6,	 50	},	/* 1 */
	{	-4,	-4,	 50	},
	{	-3,	-3,	 40	},
	{	-2,	-2,	 30	},
	{	-1,	-1,	 20	},	/* 5 */
	{	 0,	 0,	 10	},
	{	 0,	 0,	  0	},
	{	 0,	 0,	  0	},
	{	 0,	 0,	  0	},
	{	 0,	 0,	  0	},	/* 10 */
	{	 0,	 0,	  0	},
	{	 0,	 0,	  0	},
	{	 0,	 0,	  0	},
	{	 0,	 0,	  0	},
	{	 0,	 0,	-10	},	/* 15 */
	{	 1,	 1,	-20	},
	{	 2,	 2,	-30	},
	{	 2,	 2,	-40	},
	{	 3,	 3,	-40	},
	{	 3,	 3,	-40	},	/* 20 */
	{	 4,	 4,	-50	},
	{	 4,	 4,	-50	},
	{	 4,	 4,	-50	},
	{	 5,	 5,	-60	},
	{	 5,	 5,	-60	}	/* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] = {
	{	-4,	 20	},	/* 0 */
	{	-3,	 25	},	/* 1 */
	{	-2,	 30	},
	{	-2,	 35	},
	{	-1,	 40	},
	{	-1,	 45	},	/* 5 */
	{	-1,	 50	},
	{	 0,	 55	},
	{	 0,	 60	},
	{	 0,	 65	},
	{	 0,	 70	},	/* 10 */
	{	 0,	 75	},
	{	 0,	 80	},
	{	 0,	 85	},
	{	 0,	 88	},
	{	 1,	 90	},	/* 15 */
	{	 2,	 95	},	/* 16 */
	{	 3,	 97	},	/* 17 */
	{	 4,	 99	},	/* 18 */
	{	 5,	 99	},
	{	 5,	 99	},	/* 20 */
	{	 6,	 99	},
	{	 6,	 99	},
	{	 7,	 99	},
	{	 8,	 99	},
	{	 9,	100	}	/* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] = {
	{	 0,	-1	},
	{	 1,	-1	},	/* 1 */
	{	 2,	-1	},
	{	 3,	-1	},
	{	 4,	 0	},
	{	 5,	 0	},	/* 5 */
	{	 6,	 0	},
	{	 8,	 0	},
	{	10,	 1	},
	{	12,	 1	},
	{	14,	 1	},	/* 10 */
	{	16,	 1	},
	{	18,	 2	},
	{	20,	 2	},
	{	22,	 2	},
	{	25,	 2	},	/* 15 */
	{	28,	 3	},
	{	32,	 3	},
	{	35,	 3	},
	{	40,	 3	},
	{	45,	 4	},	/* 20 */
	{	50,	 4	},
	{	60,	 5	},
	{	70,	 7	},
	{	80,	10	},
	{	99,	15	}	/* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] = {
	{	0	},
	{	0	},
	{	0	},
	{	1	},
	{	1	},
	{	1	},
	{	1	},
	{	1	},
	{	2	},
	{	2	},
	{	3	},	/* 10 */
	{	3	},	/* 11 */
	{	3	},	/* 12 */
	{	4	},	/* 13 */
	{	4	},	/* 14 */
	{	5	},	/* 15 */
	{	5	},	/* 16 */
	{	6	},	/* 17 */
	{	7	},	/* 18 */
	{	7	},
	{	7	},
	{	7	},
	{	7	},
	{	7	},
	{	7	},
	{	8	}
};

struct chr_app_type chr_apply[26] = {
	{	 0,	-70	},	/* 0 */
	{	 0,	-70	},
	{	 1,	-60	},
	{	 1,	-50	},
	{	 1,	-40	},	/* 4 */
	{	 2,	-30	},
	{	 2,	-20	},
	{	 3,	-10	},
	{	 4,	  0	},
	{	 5,	  0	},	/* 9 */
	{	 6,	  0	},
	{	 7,	  0	},
	{	 8,	  0	},
	{	 9,	  5	},	/* 13 */
	{	10,	 10	},
	{	12,	 15	},
	{	14,	 20	},
	{	17,	 25	},
	{	20,	 30	},	/* 18 */
	{	20,	 40	},
	{	25,	 50	},
	{	25,	 60	},
	{	25,	 70	},
	{	25,	 80	},
	{	25,	 90	},
	{	25,	 95	}
};


const char* spell_desc[] = {
											"!ERROR!",
	/*    1 SPELL_ARMOR                 */	"$n e' circondat$b da uno schermo magico simile ad una $c0011armatura$c0007.",
	/*    2 SPELL_TELEPORT              */	"!Teleport!",
	/*    3 SPELL_BLESS                 */	"$n e' protett$b dalla $c0015benedizione Divina$c0007.",
	/*    4 SPELL_BLINDNESS             */	"$n e' temporaneamente accecat$b.",
	/*    5 SPELL_BURNING_HANDS         */	"!Burning Hands!",
	/*    6 SPELL_CALL_LIGHTNING        */	"!Call Lightning",
	/*    7 SPELL_CHARM_PERSON          */	"$n e' l$b schiav$b di qualcuno!",
	/*    8 SPELL_CHILL_TOUCH           */	"",
	/*    9 SPELL_CLONE                 */	"!Clone!",
	/*   10 SPELL_COLOUR_SPRAY          */	"!Color Spray!",
	/*   11 SPELL_CONTROL_WEATHER       */	"!Control Weather!",
	/*   12 SPELL_CREATE_FOOD           */	"!Create Food!",
	/*   13 SPELL_CREATE_WATER          */	"!Create Water!",
	/*   14 SPELL_CURE_BLIND            */	"!Cure Blind!",
	/*   15 SPELL_CURE_CRITIC           */	"!Cure Critic!",
	/*   16 SPELL_CURE_LIGHT            */	"!Cure Light!",
	/*   17 SPELL_CURSE                 */	"$n e' stat$b $c0009maledett$b$c0007!.",
	/*   18 SPELL_DETECT_EVIL           */	"Gli occhi di $n hanno un alone di $c0009luce rossa$c0007.",
	/*   19 SPELL_DETECT_INVISIBLE      */	"Gli occhi di $n hanno un alone di $c0011luce gialla$c0007.",
	/*   20 SPELL_DETECT_MAGIC          */	"Gli occhi di $n hanno un alone di $c0012luce blu$c0007.",
	/*   21 SPELL_DETECT_POISON         */	"Gli occhi di $n hanno un alone di $c0015luce bianca$c0007.",
	/*   22 SPELL_DISPEL_EVIL           */	"!Dispel Evil!",
	/*   23 SPELL_EARTHQUAKE            */	"!Earthquake!",
	/*   24 SPELL_ENCHANT_WEAPON        */	"!Enchant Weapon!",
	/*   25 SPELL_ENERGY_DRAIN          */	"!Energy Drain!",
	/*   26 SPELL_FIREBALL              */	"!Fireball!",
	/*   27 SPELL_HARM                  */	"!Harm!",
	/*   28 SPELL_HEAL                  */	"!Heal!",
	/*   29 SPELL_INVISIBLE             */	"",
	/*   30 SPELL_LIGHTNING_BOLT        */	"!Lightning Bolt!",
	/*   31 SPELL_LOCATE_OBJECT         */	"!Locate object!",
	/*   32 SPELL_MAGIC_MISSILE         */	"!Magic Missile!",
	/*   33 SPELL_POISON                */	"$n appare molto $c0010sofferente$c0007.",
	/*   34 SPELL_PROTECT_FROM_EVIL     */	"$c0014Darkstar$c0007 protegge $n dai $c0009maligni$c0007.",
	/*   35 SPELL_REMOVE_CURSE          */	"!Remove Curse!",
	/*   36 SPELL_SANCTUARY             */	"$n e' circondat$b da una rilucente $c0015aura bianca$c0007.",
	/*   37 SPELL_SHOCKING_GRASP        */	"!Shocking Grasp!",
	/*   38 SPELL_SLEEP                 */	"$n e' stat$b addormentat$b da qualcuno.",
	/*   39 SPELL_STRENGTH              */	"$n sembra MOLTO $c0009forte$c0007!",
	/*   40 SPELL_SUMMON                */	"!Summon!",
	/*   41 SPELL_VENTRILOQUATE         */	"!Ventriloquate!",
	/*   42 SPELL_WORD_OF_RECALL        */	"!Word of Recall!",
	/*   43 SPELL_REMOVE_POISON         */	"!Remove Poison!",
	/*   44 SPELL_SENSE_LIFE            */	"",
	/*   45 SKILL_SNEAK                 */	"",  /* NO MESSAGE FOR SNEAK*/
	/*   46 SKILL_HIDE                  */	"!Hide!",
	/*   47 SKILL_STEAL                 */	"!Steal!",
	/*   48 SKILL_BACKSTAB              */	"!Backstab!",
	/*   49 SKILL_PICK_LOCK             */	"!Pick Lock!",
	/*   50 SKILL_KICK                  */	"!Kick!",
	/*   51 SKILL_BASH                  */	"!Bash!",
	/*   52 SKILL_RESCUE                */	"!Rescue!",
	/*   53 SPELL_IDENTIFY              */	"!Identify!",
	/*   54 SPELL_INFRAVISION           */	"Gli occhi di $n hanno un lucente riflesso $c0009rosso$c0007.",
	/*   55 SPELL_CAUSE_LIGHT           */	"!cause light!",
	/*   56 SPELL_CAUSE_CRITICAL        */	"!cause crit!",
	/*   57 SPELL_FLAMESTRIKE           */	"!flamestrike!",
	/*   58 SPELL_DISPEL_GOOD           */	"!dispel good!",
	/*   59 SPELL_WEAKNESS              */	"$n sembra MOLTO debole.",
	/*   60 SPELL_DISPEL_MAGIC          */	"!dispel magic!",
	/*   61 SPELL_KNOCK                 */	"!knock!",
	/*   62 SPELL_KNOW_ALIGNMENT        */	"!know alignment!",
	/*   63 SPELL_ANIMATE_DEAD          */	"!animate dead!",
	/*   64 SPELL_PARALYSIS             */	"$n pare impedit$b a muoversi!",
	/*   65 SPELL_REMOVE_PARALYSIS      */	"!remove paralysis!",
	/*   66 SPELL_FEAR                  */	"!fear!",
	/*   67 SPELL_ACID_BLAST            */	"!acid blast!",
	/*   68 SPELL_WATER_BREATH          */	"$n ha qualcosa che l$b fa sembrare un pesce.",
	/*   69 SPELL_FLY                   */	"$n sta volando.",
	/*   70 SPELL_CONE_OF_COLD          */	"!cone of cold!",
	/*   71 SPELL_METEOR_SWARM          */	"!meteor swarm!",
	/*   72 SPELL_ICE_STORM             */	"!ice storm!",
	/*   73 SPELL_SHIELD                */	"$n e' circondat$b da un leggero $c0011scudo$c0007 magico.",
	/*   74 SPELL_MON_SUM_1             */	"!monsum one!",
	/*   75 SPELL_MON_SUM_2             */	"!monsum two!",
	/*   76 SPELL_MON_SUM_3             */	"!monsum three!",
	/*   77 SPELL_MON_SUM_4             */	"!monsum four!",
	/*   78 SPELL_MON_SUM_5             */	"!monsum five!",
	/*   79 SPELL_MON_SUM_6             */	"!monsum six!",
	/*   80 SPELL_MON_SUM_7             */	"!monsum seven!",
	/*   81 SPELL_FIRESHIELD            */	"$n e' circondat$b da $c0009fiamme ardenti$c0007!",
	/*   82 SPELL_CHARM_MONSTER         */	"$n e' l$b schiav$b di qualcuno!",
	/*   83 SPELL_CURE_SERIOUS          */	"!cure serious!",
	/*   84 SPELL_CAUSE_SERIOUS         */	"!cause serious!",
	/*   85 SPELL_REFRESH               */	"!refresh!",
	/*   86 SPELL_SECOND_WIND           */	"!second wind!",
	/*   87 SPELL_TURN                  */	"!turn!",
	/*   88 SPELL_SUCCOR                */	"!succor!",
	/*   89 SPELL_LIGHT                 */	"!light!",
	/*   90 SPELL_CONT_LIGHT            */	"!continual light!",
	/*   91 SPELL_CALM                  */	"!calm!",
	/*   92 SPELL_STONE_SKIN            */	"La pelle di $n ha la consistenza del piu' duro $c0003granito$c0007.",
	/*   93 SPELL_CONJURE_ELEMENTAL     */	"!conjure elemental!",
	/*   94 SPELL_TRUE_SIGHT            */	"Gli occhi di $n emanano una $c0015luce argentea$c0007.",
	/*   95 SPELL_MINOR_CREATE          */	"!minor creation!",
	/*   96 SPELL_FAERIE_FIRE           */	"$n e' circondat$b da un $c0013alone rosa$c0007.",
	/*   97 SPELL_FAERIE_FOG            */	"!faerie fog!",
	/*   98 SPELL_CACAODEMON            */	"!cacaodemon!",
	/*   99 SPELL_POLY_SELF             */	"",
	/*  100 SPELL_MANA                  */	"$n e' circondat$b da una $c5007pulsante$c0007 $c0011aura$c0007 globulare.",
	/*  101 SPELL_ASTRAL_WALK           */	"!astral walk!",
	/*  102 SPELL_RESURRECTION          */	"!resurrection!",
	/*  103 SPELL_H_FEAST               */	"!heroes feast!",
	/*  104 SPELL_FLY_GROUP             */	"$n sta volando.",
	/*  105 SPELL_DRAGON_BREATH         */	"!dragon breath!",
	/*  106 SPELL_WEB                   */	"$n e' ricopert$b da $c0008ragnatele$c0007 appiccicose!",
	/*  107 SPELL_MINOR_TRACK           */	"",
	/*  108 SPELL_MAJOR_TRACK           */	"",
	/*  109 SPELL_GOLEM                 */	"!golem!",
	/*  110 SPELL_FAMILIAR              */	"",
	/*  111 SPELL_CHANGESTAFF           */	"!changestaff!",
	/*  112 SPELL_HOLY_WORD             */	"!holy word!",
	/*  113 SPELL_UNHOLY_WORD           */	"!unholy word!",
	/*  114 SPELL_PWORD_KILL            */	"!pwk!",
	/*  115 SPELL_PWORD_BLIND           */	"!pwb!",
	/*  116 SPELL_CHAIN_LIGHTNING       */	"!chain lightning!",
	/*  117 SPELL_SCARE                 */	"!scare!",
	/*  118 SPELL_AID                   */	"$n appare sorrett$b dal $c0015favore degli Dei$c0007!",
	/*  119 SPELL_COMMAND               */	"!command!",
	/*  120 SPELL_CHANGE_FORM           */	"",
	/*  121 SPELL_FEEBLEMIND            */	"Ha un'espressione rimbecillita e stupida.",
	/*  122 SPELL_SHILLELAGH            */	"!shillelagh!",
	/*  123 SPELL_GOODBERRY             */	"!goodberry!",
	/*  124 SPELL_ELEMENTAL_BLADE       */	"!elemental blade!",
	/*  125 SPELL_ANIMAL_GROWTH         */	"$n e' $c0003ENORME$c0007.",
	/*  126 SPELL_INSECT_GROWTH         */	"$n e' $c0003ENORME$c0007.",
	/*  127 SPELL_CREEPING_DEATH        */	"",
	/*  128 SPELL_COMMUNE               */	"!commune!",
	/*  129 SPELL_ANIMAL_SUM_1          */	"",
	/*  130 SPELL_ANIMAL_SUM_2          */	"",
	/*  131 SPELL_ANIMAL_SUM_3          */	"",
	/*  132 SPELL_FIRE_SERVANT          */	"",
	/*  133 SPELL_EARTH_SERVANT         */	"",
	/*  134 SPELL_WATER_SERVANT         */	"",
	/*  135 SPELL_WIND_SERVANT          */	"",
	/*  136 SPELL_REINCARNATE           */	"!reincarnate!",
	/*  137 SPELL_CHARM_VEGGIE          */	"$n e' l$b schiav$b di qualcuno!",
	/*  138 SPELL_VEGGIE_GROWTH         */	"$n e' $c0003ENORME$c0007.",
	/*  139 SPELL_TREE                  */	"",
	/*  140 SPELL_ANIMATE_ROCK          */	"!Animate rock!",
	/*  141 SPELL_TREE_TRAVEL           */	"L'aspetto di $n e' vagamente $c0015traslucido$c0007.",
	/*  142 SPELL_TRAVELLING            */	"",
	/*  143 SPELL_ANIMAL_FRIENDSHIP     */	"",
	/*  144 SPELL_INVIS_TO_ANIMALS      */	"",
	/*  145 SPELL_SLOW_POISON           */	"",
	/*  146 SPELL_ENTANGLE              */	"$n e' intrappolat$b tra $c0003s$c0010t$c0003e$c0010r$c0003p$c0010i$c0007 e $c0003r$c0010o$c0003v$c0010i$c0007.",
	/*  147 SPELL_SNARE                 */	"$n e' trattenut$b da $c0010liane$c0007 e rampicanti che $d impediscono il movimento.",
	/*  148 SPELL_GUST_OF_WIND          */	"!gust of wind!",
	/*  149 SPELL_BARKSKIN              */	"$n ha la pelle bruna e rugosa come $c0003corteccia$c0007!",
	/*  150 SPELL_SUNRAY                */	"!sunray!",
	/*  151 SPELL_WARP_WEAPON           */	"!warp weapon!",
	/*  152 SPELL_HEAT_STUFF            */	"L'armatura di $n $c0001frigge$c0007 e $c0008fuma$c0007.",
	/*  153 SPELL_FIND_TRAPS            */	"Gli occhi di $n $c0015brillano$c0007.",
	/*  154 SPELL_FIRESTORM             */	"!firestorm!",
	/*  155 SPELL_HASTE                 */	"$c0015$n$c0015 ha i movimenti rapidissimi.",
	/*  156 SPELL_SLOW                  */	"$c0008$n$c0008 si muove con fatica, come al rallentatore!",
	/*  157 SPELL_DUST_DEVIL            */	"",
	/*  158 SPELL_KNOW_MONSTER          */	"!know monster!",
	/*  159 SPELL_TRANSPORT_VIA_PLANT   */	"!transport via plant!",
	/*  160 SPELL_SPEAK_WITH_PLANT      */	"!speak with plant!",
	/*  161 SPELL_SILENCE               */	"$n ha la bocca chiusa magicamente.",
	/*  162 SPELL_SENDING               */	"!sending!",
	/*  163 SPELL_TELEPORT_WO_ERROR     */	"!teleport without error!",
	/*  164 SPELL_PORTAL                */	"!portal!",
	/*  165 SPELL_DRAGON_RIDE           */	"",
	/*  166 SPELL_MOUNT                 */	"!mount!",
	/*  167 SPELL_NO_MESSAGE            */	"",
	/*  168 to_do                       */	"!168!",
	/*  169 SKILL_MANTRA                */	"$n e' concentrat$b in una preghiera.",
	/*  170 SKILL_FIRST_AID             */	"",
	/*  171 SKILL_SIGN                  */	"!sign!",
	/*  172 SKILL_RIDE                  */	"!riding!",
	/*  173 SKILL_SWITCH_OPP            */	"!switch!",
	/*  174 SKILL_DODGE                 */	"!dodge!",
	/*  175 SKILL_REMOVE_TRAP           */	"!remove trap!",
	/*  176 SKILL_RETREAT               */	"!retreat!",
	/*  177 SKILL_QUIV_PALM             */	"",
	/*  178 SKILL_SAFE_FALL             */	"!safe fall!",
	/*  179 SKILL_FEIGN_DEATH           */	"!feign death!",
	/*  180 SKILL_HUNT                  */	"!hunt!",
	/*  181 SKILL_FIND_TRAP             */	"!find traps!",
	/*  182 SKILL_SPRING_LEAP           */	"!spring leap!",
	/*  183 SKILL_DISARM                */	"!disarm!",
	/*  184 SKILL_READ_MAGIC            */	"!read magic!",
	/*  185 SKILL_EVALUATE              */	"!evalutate!",
	/*  186 SKILL_SPY                   */	"",
	/*  187 SKILL_DOORBASH              */	"!doorbash!",
	/*  188 SKILL_SWIM                  */	"",
	/*  189 SKILL_CONS_UNDEAD           */	"!consider undead!",
	/*  190 SKILL_CONS_VEGGIE           */	"!consider veggie!",
	/*  191 SKILL_CONS_DEMON            */	"!consider demon!",
	/*  192 SKILL_CONS_ANIMAL           */	"!consider animal!",
	/*  193 SKILL_CONS_REPTILE          */	"!consider reptile!",
	/*  194 SKILL_CONS_PEOPLE           */	"!consider people!",
	/*  195 SKILL_CONS_GIANT            */	"!consider giant!",
	/*  196 SKILL_CONS_OTHER            */	"!consider other!",
	/*  197 SKILL_DISGUISE              */	"",
	/*  198 SKILL_CLIMB                 */	"!climb!",
	/*  199 SKILL_FINGER                */	"!finger!",
	/*  200 SPELL_GEYSER                */	"!geyser!",
	/*  201 SPELL_MIRROR_IMAGES         */	"Stai guardando l'$c0012immagine$c0007 giusta?",
	/*  202 SKILL_TSPY                  */	"$n ha l'aria molto attenta.",
	/*  203 SKILL_EAVESDROP             */	"!eavesdrop!",
	/*  204 SKILL_PARRY                 */	"!parry!",
	/*  205 SKILL_MINER                 */	"!miner!",
	/*  206 SPELL_GREEN_SLIME           */	"!green slime!",
	/*  207 SKILL_BERSERK               */	"!berserk!",
	/*  208 SKILL_TAN                   */	"!tan!",
	/*  209 SKILL_AVOID_BACK_ATTACK     */	"!avoid backattack!",
	/*  210 SKILL_FIND_FOOD             */	"!find food!",
	/*  211 SKILL_FIND_WATER            */	"!find water!",
	/*  212 SPELL_PRAYER                */	"",
	/*  213 SKILL_MEMORIZE              */	"$n sta studiando il suo libro degli $c0012i$c0011n$c0012c$c0011a$c0012n$c0011t$c0012e$c0011s$c0012i$c0011m$c0012i$c0007.",
	/*  214 SKILL_BELLOW                */	"!bellow!",
	/*  215 SPELL_GLOBE_DARKNESS        */	"Uno spesso strato di $c0008oscurita'$c0007 circonda $n.",
	/*  216 SPELL_GLOBE_MINOR_INV       */	"$n e' protett$b da un piccolo $c0012globo$c0007 magico.",
	/*  217 SPELL_GLOBE_MAJOR_INV       */	"$n e' protett$b da un ampio $c0014globo$c0007 magico.",
	/*  218 SPELL_PROT_ENERGY_DRAIN     */	"$n ha una protezione magica contro i $c0008Non-Morti$c0007.",
	/*  219 SPELL_PROT_DRAGON_BREATH    */	"$n ha una protezione magica contro il soffio dei $c0003draghi$c0007.",
	/*  220 SPELL_ANTI_MAGIC_SHELL      */	"Il corpo di $n e' circondato da un pulsante scudo $c0012anti-magia$c0007.",
	/*  221 SKILL_DOORWAY               */	"!doorway!",
	/*  222 SKILL_PORTAL                */	"!psi_portal!",
	/*  223 SKILL_SUMMON                */	"!psi_summon!",
	/*  224 SKILL_INVIS                 */	"!psi_invis!",
	/*  225 SKILL_CANIBALIZE            */	"!canibalize!",
	/*  226 SKILL_FLAME_SHROUD          */	"!flame shroud!",
	/*  227 SKILL_AURA_SIGHT            */	"!aura sight!",
	/*  228 SKILL_GREAT_SIGHT           */	"!great sight!",
	/*  229 SKILL_PSIONIC_BLAST         */	"$n ha il cervello tramutato in $c0008gelatina$c0007.",
	/*  230 SKILL_HYPNOSIS              */	"!hypnosis!",
	/*  231 SKILL_MEDITATE              */	"$n e' assort$b in meditazione.",
	/*  232 SKILL_SCRY                  */	"!scry!",
	/*  233 SKILL_ADRENALIZE            */	"$n ha uno sguardo $c0001folle$c0007.",
	/*  234 SKILL_BREW                  */	"!brew!",
	/*  235 SKILL_RATION                */	"!ration!",
	/*  236 SKILL_HOLY_WARCRY           */	"!holy warcry!",
	/*  237 SKILL_BLESSING              */	"",
	/*  238 SKILL_LAY_ON_HANDS          */	"",
	/*  239 SKILL_HEROIC_RESCUE         */	"!heroic rescue!",
	/*  240 SKILL_DUAL_WIELD            */	"!dual wield!",
	/*  241 SKILL_PSI_SHIELD            */	"$n e' protett$b da un potente $c0011scudo$c0007 psichico.",
	/*  242 SPELL_PROT_FROM_EVIL_GROUP  */	"$c0014Darkstar$c0007 protegge $n dai $c0009maligni$c0007.",
	/*  243 SPELL_PRISMATIC_SPRAY       */	"!prismatic spry!",
	/*  244 SPELL_INCENDIARY_CLOUD      */	"!incendiary cloud!",
	/*  245 SPELL_DISINTEGRATE          */	"!disintegrate!",
	/*  246 LANG_COMMON                 */	"!lang common!",
	/*  247 LANG_ELVISH                 */	"!lang elvish!",
	/*  248 LANG_HALFLING               */	"!lang halfling!",
	/*  249 LANG_DWARVISH               */	"!lang dwarvish!",
	/*  250 LANG_ORCISH                 */	"!lang orcish!",
	/*  251 LANG_GIANTISH               */	"!lang giantish!",
	/*  252 LANG_OGRE                   */	"!lang ogre!",
	/*  253 LANG_GNOMISH                */	"!lang gnomish!",
	/*  254 SKILL_ESP                   */	"",
	/*  255 SPELL_COMP_LANGUAGES        */	"",
	/*  256 SPELL_PROT_FIRE             */	"$n ha una protezione magica contro il $c0009fuoco$c0007.",
	/*  257 SPELL_PROT_COLD             */	"$n ha una protezione magica contro il $c0014freddo$c0007.",
	/*  258 SPELL_PROT_ENERGY           */	"$n ha una protezione magica contro l'$c0011energia$c0007.",
	/*  259 SPELL_PROT_ELEC             */	"$n ha una protezione magica contro l'$c0012elettricita'$c0007.",
	/*  260 SPELL_ENCHANT_ARMOR         */	"!enchant armor!",
	/*  261 SPELL_MESSENGER             */	"!messenger!",
	/*  262 SPELL_PROT_BREATH_FIRE      */	"$n e' circondat$b da un globo protettivo contro il soffio $c0009infuocato$c0007 dei draghi.",
	/*  263 SPELL_PROT_BREATH_FROST     */	"$n e' circondat$b da un globo protettivo contro il soffio $c0014gelido$c0007 dei draghi.",
	/*  264 SPELL_PROT_BREATH_ELEC      */	"$n e' circondat$b da un globo protettivo contro il soffio $c0012elettrico$c0007 dei draghi.",
	/*  265 SPELL_PROT_BREATH_ACID      */	"$n e' circondat$b da un globo protettivo contro il soffio $c0010acido$c0007 dei draghi.",
	/*  266 SPELL_PROT_BREATH_GAS       */	"$n e' circondat$b da un globo protettivo contro il soffio $c0011gassoso$c0007 dei draghi.",
	/*  267 SPELL_WIZARDEYE             */	"$n ha un occhio volante vicino a $l.",
	/*  268 SKILL_MIND_BURN             */	"!mind burn!",
	/*  269 SKILL_CLAIRVOYANCE          */	"",
	/*  270 SKILL_DANGER_SENSE          */	"$n ti sembra attent$b al pericolo.",
	/*  271 SKILL_DISINTEGRATE          */	"!psi_disint!",
	/*  272 SKILL_TELEKINESIS           */	"!telekinesis!",
	/*  273 SKILL_LEVITATION            */	"$n levita delicatamente a mezz'aria.",
	/*  274 SKILL_CELL_ADJUSTMENT       */	"!cell adjustment!",
	/*  275 SKILL_CHAMELEON             */	"!chameleon!",
	/*  276 SKILL_PSI_STRENGTH          */	"",
	/*  277 SKILL_MIND_OVER_BODY        */	"",
	/*  278 SKILL_PROBABILITY_TRAVEL    */	"!probability travel!",
	/*  279 SKILL_PSI_TELEPORT          */	"!psi teleport!",
	/*  280 SKILL_DOMINATION            */	"!domination!",
	/*  281 SKILL_MIND_WIPE             */	"!mind wipe!",
	/*  282 SKILL_PSYCHIC_CRUSH         */	"!psychic crush!",
	/*  283 SKILL_TOWER_IRON_WILL       */	"",
	/*  284 SKILL_MINDBLANK             */	"",
	/*  285 SKILL_PSYCHIC_IMPERSONATION */	"",
	/*  286 SKILL_ULTRA_BLAST           */	"!ultra blast!",
	/*  287 SKILL_INTENSIFY             */	"Numerose $c0011scintille$c0007 circondano la testa di $n.",
	/*  288 SKILL_SPOT                  */	"!spot!",
	/*  289 SKILL_IMMOLATION            */	"!immolation!",
	/*  290 STATUS_QUEST                */	"$n e' concentrat$b nella sua missione!",
	/*  291 SKILL_DAIMOKU               */	"$n e' in stato di ascesi.",
	/*  292 SKILL_FORGE                 */	"!forge!",
	/*  293 SKILL_DETERMINE             */	"!determine!",
	/*  294 SKILL_EQUILIBRIUM           */	"!equilibrium!",
											"\n"
};


/* Requiem: queste fan parte del diku, le terrei in cantiere per un eventuale implementazione
 rispetto i gain, e renderli un po' piu' complicati richiedendo il completamento
 della quest oltre agli xp */
struct QuestItem QuestList[4][IMMORTAL] = {
	{
		/* magic user */
		{		0,	""},
		{		2,	"It can be found in the donation room, or on your head\n\r"},
		{	 1410,	"Its a heavy bag of white powder\n\r"},
		{	 6010,	"You can make pies out of them, deer seem to like them too\n\r"},
		{	 3013,	"Its a yummy breakfast food, they go great with eggs and cheese\n\r"},
		{	   20,	"If you twiddle your thumbs enough, you'll find one.\n\r"},
		{	24764,	"Dead people might wear them on their hands\n\r"},
		{	  112,	"If you found one of these, it would be mighty strange!\n\r"},
		{	  106,	"Eye of Toad and Toe of Frog, bring me one or the other\n\r"},
		{	  109,	"A child's favorite place holds the object of my desire, on the dark river\n\r"},
		/* 10 */
		{	 3628,	"The latest in New padded footwear\n\r"},
		{	  113,	"A child might play with one, when the skexies aren't around\n\r"},
		{	19204,	"A precious moon in a misty castle\n\r"},
		{	20006,	"Are you a fly? You might run into one. Beware..\n\r"},
		{	 1109,	"Little people have tiny weapons.. bring me one\n\r"},
		{	 6203,	"IReallyReallyWantACurvedBlade\n\r"},
		{	21007,	"I want to be taller and younger, find the nasty children\n\r"},
		{	 5228,	"Don't you find screaming women so disarming?\n\r"},
		{	 7204,	"Vaulted lightning\n\r"},
		{	16043,	"Precious elements can come in dull, dark mines\n\r"},
		/* 20 */
		{	20007,	"You'll catch him napping, no guardian of passing time."},
		{	16903,	"Nature's mistake, carried by a man on a new moon, fish on full."},
		{	 5226,	"Sealed in the hands of a city's failed guardian."},
		{	10900,	"Anachronistic rectangular receptacle holds circular plane."}, /* Sentry, TL */
		{	13840,	"What kind of conditioner does one use for asps?"}, /* Medusa, GQ */
		{	 7406,	"If you don't bring a scroll of recall, you might die a fiery death"}, /* Room 7284, SM */
		{	  120,	"Dock down anchor"}, /* Ixitxachitl, NT */
		{	21008,	"Very useful, behind a hearth."}, /* Dog, OR */
		{	10002,	"He didn't put them to sleep with these, the results were quite deadly"}, /* On Corpse, DCE */
		{	 3648,	"Unsummoned, they pose large problems. What you want is on their queen."}, /* Chieftess, HGS */
		{	15805,	"A single sample of fine plumage, held by a guard and a ghost."}, /* Guard & Brack, SK */
		{	21141,	"In the land of the troglodytes there is a headpiece with unpleasant powers."}, /* Case, TR */
		{	 1532,	"Three witches have the flighty component you need."}, /* Pot, MT */
		{	 5304,	"A spectral force holds the key to advancement in a geometric dead end."}, /* Spectre, PY */
		{	 9496,	"A great golden sword was once taken by the giants of the great with north.  Return it. Bring friends."},
		{	 5105,	"What you need is as dark as the heart of the elf who wields it."}, /* Weaponsmaster, DR */
		{	21011,	"The key to your current problem is in Orshingal, on a haughty strutter."}, /* Enfan, OR */
		{	27004,	"A small explosive pinapple shaped object. Ever see Monty-Python? You might find it in the hands of sorcerous undead men"}, /* ???, Haplo's */
		{	 6616,	"You might smile if drinking a can of this. Look in Prydain."}, /* ???, PRY */
		{	21125,	"With enough of this strong amber drink you'd forget about the nightly ghosts."}, /* Crate, ARO */
		/* 40 */
		{	 5309,	"Powerful items of magic follow... first bring me a medallion of mana\n\r"},
		{	 1585,	"Bubble bubble, toil and trouble, bring me a staff on the double\n\r"},
		{	21003,	"I need some good boots, you know how strange it is to find them\n\r"},
		{	13704,	"Watch for a dragon, find his ring\n\r"},
		{	  252,	"Dead-makers cloaks, buried deep in stone\n\r"},
		{	 3670,	"bottled mana\n\r"},
		{	 1104,	"The master of fireworks, take his silver"},
		{	 5020,	"You're not a real mage till you get platinum from purple\n\r"},
		{	 1599,	"Grand Major\n\r"},
		{	20002,	"She's hiding in her room, but she has my hat!\n\r"}
	},
	{
		/* cleric */
		{		0,	""},
		{		1,	"It can be found in the donation room, or on your head\n\r"},
		{	 1110,	"White and young, with no corners or sides, a golden treasure can be found inside\n\r"},
		{	 3070,	"The armorer might have a pair, but they're not his best on hand\n\r"},
		{	 3057,	"judicandus dies\n\r"},
		{	 6001,	"I want clothes i can play chess on\n\r"},
		{	16033,	"A goblin's favorite food, the eastern path\n\r"},
		{	  107,	"Every righteous cleric should have one, but few do\n\r"},
		{	 4000,	"I have a weakness for cheap rings\n\r"},
		{	 3025,	"cleaver for hire, just outside of midgaard\n\r"},
		/* 10 */
		{	 3649,	"My wife needs something New, to help keep her girlish figure\n\r"},
		{	 7202,	"mindflayers have small ones, especially in the sewers\n\r"},
		{	19203,	"the weapon of a traitor, lost in a fog\n\r"},
		{	15814,	"striped black and white, take what this gelfling-friend offers\n\r"},
		{	  119,	"Play with a gypsy child when he asks you to\n\r"},
		{	 5012,	"You might use these to kill a vampire, they are in the desert\n\r"},
		{	 6809,	"Really cool sunglasses, on a really cool guy, in a really cool place\n\r"},
		{	17021,	"The proof is in the volcano\n\r"},
		{	 3648,	"Giant women have great fashion sense\n\r"},
		{	27001,	"Undead have the strangest law enforcement tools, near a split in the river\n\r"},
		/* 20*/
		{	  105,	"A venomed bite will end your life - you need the jaws that do it."},	/* Rattler fangs, MidT */
		{	 3668,	"Buy some wine from a fortuneteller's brother."},                   	/* Gypsy Bartender */
		{	 1703,	"On a cat, but not in the petting zoo."},                           	/* Bengal tiger , MZ*/
		{	13758,	"Held by a doggie with who will bite you and bite you and bite you."}, /* Cerebus, Hades */
		{	 5240,	"In the old city you'll find the accursed vibrant stone you require."},  /* Lamia, OT */
		{	 5013,	"Where can you go for directions in the desert? Try the wet spot."}, 	/* Oasis Map, GED */
		{	17011,	"{	An unholy symbol on an unholy creature under a fuming mountain."},	/* Amelia, WPM */
		{	 1708,	"Some liar's got it in a cul-de-sac. The high priests of Odin know the way."}, /*Liar in room 10911, TL */
		{	 9203,	"What would you use to swat a very large mosquito? Get it from the giants."}, /* Giant, HGS */
		{	21109,	"A bow made of dark wood, carried by a troglodyte."},                	/* Trog, CT */
		/* 30 */
		{	15817,	"In an secret cold place, a dark flower is carried by a midnight one."}, /* Bechemel, SK */
		{	 9430,	"Argent Ursa, Crevasse of the Arctic Enlargements"},
		{	 6112,	"If you would be king, you'd need one. With a wooded worm."},         	/* Dragon, DH-D */
		{	 1758,	"Carried by a hag in the dark lake under the sewers."},               	/* Sea Hag, SM */
		{	27411,	"This Roo's better than you and she has the stick to prove it."},    	/* Queen Roo, LDU */
		{	 5317,	"The dead don't lie still when properly prepared."}, /* Mummy, PY */
		{	 5033,	"You can get it off a drider, but he won't give it to you."},  /* Drider, DR */
		{	16615,	"South of a bay, past a portal, into a tower, be prepared"},  /* Hoeur, HR */
		{	  121,	"To the far southeast, in the lair of a pair of arachnids."},      	/* Cave Spiders, TR */
		{	13901,	"On the shore, down the river from the troll-bridge"}, /* RHY */
		/* 40 */
		{	 5104,	"Four heads are better than one\n\r"},
		{	15806,	"You don't stand a ghost of a chance against a glow of white and a cloak of fire"},
		{	16022,	"a Powerful, blunt, and fragile weapon\n\r"},
		{	  122,	"The sole possession of a devil down-under\n\r"},
		{	 7220,	"The highest thing at the top of a chain"},
		{	13785,	"From the fairest\n\r"},
		{	 1597,	"Mana in a green ring\n\r"},
		{	 1563,	"Famous, blue and very very rare"},
		{	 5001,	"Search for a banded male\n\r"},
		{	20003,	"Ensnared for power, she holds the helmet of the wise"}

	},
	{
		/* warrior */
		{		0,	""},
		{	   11,	"Something you might find in the donation room, or on your body\n\r"},
		{	16034,	"Goblins have been known to play with these, especially in dark caves\n\r"},
		{	 6000,	"A decent weapon, just the right size for a tree\n\r"},
		{	24760,	"Dead men's feet look like this\n\r"},
		{	 1413,	"You were SUPPOSED to bell the CAT!\n\r"},
		{	18256,	"In the city of Mordilnia, a shield of roygiv\n\r"},
		{	 8121,	"A bag that opens with a ripping sound\n\r"},
		{	  108,	"Floating for safety on the dark\n\r"},
		{	  123,	"A mule stole my hat, now he fights in front of an audience\n\r"},
		/* 10 */
		{	 3621,	"Thank goodness when I broke my arm, I still had my New shield\n\r"},
		{	  117,	"If you get this, someone will be quite howling angry\n\r"},
		{	 7405,	"Sewer Secret Light Sources\n\r"},
		{	 6205,	"my eyes just arent as fast to focus as they used to be\n\r"},
		{	 4051,	"These warriors seem scarred, but its all in their head"}, /* Scarred Warrior, MM */
		{	 5219,	"Fresh deer.. yum!\n\r"},
		{	16015,	"An ugly bird in the mountains once told me: 'A despotic ruler rules with one of these'\n\r"},
		{	 1718,	"Hey, that's not a painting at all! But boy is she ugly! In the new city."}, /* Mimic, NT */
		{	 5032,	"Bushwhacked, Bushwhacked, West, West, Green. Start at the obvious\n\r"},
		{	 3685,	"Mightier than a sword, wielded by a four man\n\r"},
		/* 20 */
		{	 5100,	"Learn humility: I want a common sword\n\r"},
		{	16902,	"They'd all be normal in a moonless world. You need to steal a silver stole"}, /* Werefox, LY */
		{	17022,	"A lion with a woman's torso holds the book you need."}, /* Gynosphinx, WPM */
		{	 5206,	"To hold the girth of a corpulent man, it must be ferrous. In the old city hall."}, /* Cabinet, OT */
		{	 1737,	"In the hands of an elf with a green thumb."}, /* Gardener, NT */
		{	 5306,	"my mommy gave me a knife, but i lost it\n\r"},
		{	21006,	"Childlike, maybe, but they're not children. You need the locked up cloth."}, /* Case, OR */
		{	 9204,	"The largest in the hands of the largest of the large"}, /* Giant Chief, HGS */
		{	 1721,	"Get the toolbook of the trade from the royal cook in the new city."}, /* Chef, NT */
		{	16901,	"Only an elephant's might be as big as this bores' mouthpiece."}, /* Boarish, LY */
		/* 30 */
		{	 6511,	"A bearded woman might be so engaged, but a guard's got this one."},
		{	 5101,	"Dark elves can be noble too, but they won't let you take their arms."}, /* Drow Noble, DR */
		{	 1761,	"In a suspended polygon, in a chest which is not."}, /* Mimic, PY */
		{	15812,	"You think that water can't be sharp? Look under birdland."}, /* Ice Pick, SK */
		{	16046,	"A miner's tool in the dwarven mines"}, /* Shovel, ?? */
		{	21114,	"These skeletal beasts will reel you in, you want the crowbar."}, /* Cave Fisher, MVE */
		{	13762,	"Once in Hades, the key to getting out lies with a long dead warrior."}, /* Skeletal Warrior, Hades */
		{	20005,	"This usurper will think you very tasty, defeat him for the thing you need."}, /* Yevaud, AR */
		{	 5019,	"A nasty potion in the hands of an even nastier desert worm."}, /* Worm, GED */
		{	10002,	"These can be found in the zoo, on 'Al'"}, /* On Corpse, DCE */
		/* 40 */
		{	 5221,	"Weapons are the keys to the remaining quests. First, bring me a Stone golem's sword"},
		{	 9442,	"The weapon of the largest giant in the rift"},
		{	15808,	"Weapon of champions"},
		{	13775,	"By the light of the moon\n\r"},
		{	21004,	"By name, you can assume its the largest weapon in the game\n\r"},
		{	 3092,	"He's always pissed, and so are his guards. take his weapon and make it yours\n\r"},
		{	 5002,	"The weapon of the oldest wyrm\n\r"},
		{	 5107,	"One Two Three Four Five Six\n\r"},
		{	 1430,	"It rises from the ashes, and guards a tower\n\r"},
		{	 5019,	"You're not a REAL fighter til you've had one of these, enchanted\n\r"}

	},
	{
		/* thief   */
		{		0,	"",},
		{		4,	"You might find one of these in the donation room, or in your hand\n\r"},
		{	 3071,	"They're the best on hand for 5 coins\n\r"},
		{	   30,	"At the wrong end of a nasty spell, or a heavy hitter\n\r"},
		{	 3902,	"Michelob or Guiness Stout. which is better?\n\r"},
		{	24767,	"I've heard that skeletons love bleach\n\r"},
		{	 6006,	"Nearly useless in a hearth\n\r"},
		{	 4104,	"Its what makes kobolds green\n\r"},
		{	   42,	"Do she-devils steal, as they flap their bat wings?\n\r"},
		{	19202,	"Animal light, lost in a fog\n\r"},
		{	 3647,	"These New boots were made for walking\n\r"},
		{	 4101,	"Hands only a warrior could love\n\r"},
		{	  116,	"Near a road to somewhere city\n\r"},
		{	  111,	"Only a fool would look at the end of the river\n\r"},
		{	15812,	"I'd love a really cool backstabbing weapon..  Make sure it doesn't melt\n\r"},
		{	17023,	"Being charming can be offensive, especially in a plumed white cap\n\r"},
		{	 9205,	"You could hide a giant in this stuff\n\r"},
		{	10002,	"feeling tired and fuzzy?  Exhibit some stealth, or you just might get eaten\n\r"},
		{	 3690,	"I am an old man, but I will crush you at chess\n\r"},
		{	 5000,	"Find a dark dwarf. Pick something silver\n\r"},
		/* 20 */
		{	15802,	"It's easy work to work a rejected bird for the means to his former home."}, /* Skexie Reject, SK */
		{	 1750,	"In the twisted forest of the Graecians a man in a black cloak has it."}, /* Put on 13731, GRF */
		{	 5012,	"Vampire's bane in a wicker basket near a desert pool.a"}, /* Basket, GED */
		{	20008,	"The toothless dragon eats the means to your advancement."}, /* Young Wormkin, AR */
		{	 6810,	"You are everywhere you look in this frozen northern maze of ice."}, /* Room 6854, ART */
		{	  255,	"Get the happy stick from a desert worm."}, /* Young Worm, GED */
		{	 7190,	"In a secret sewer place a squeaking rodent wears a trinket."}, /* Rat, SM */
		{	 7205,	"The master flayer under the city has it on him, but not in use. Steal it!"}, /* Master mind, SM */
		{	 7230,	"You could be stoned for skinning this subterranean reptilian monster."}, /* Basilisk, SM */
		{	 3690,	"An old man at the park might have one, but these old men are in the new city."}, /* Old man, NT */
		/* 30 */
		{	 1729,	"In the forest north of the new city a traveller lost his way. It's on him."}, /* Lost Adventurer, MT */
		{	 1708,	"It's growing on a cliff face, on the way to the lost kingdom."}, /* In room 21170, MVE */
		{	 1759,	"The moon's phase can change a man. Find the badger in a tavern."}, /* Werebadger, LY */
		{	 1718,	"You'll find it in the only ice cave a stone's throw from a desert."}, /* In room 10010, DCE */
		{	 5243,	"I hope it is clear which stone you will need"}, /* Lamia, OT */
		{	 5302,	"In a hanging desert artifact, the softest golem has the key to your success."}, /* Clay Golem, PY */
		{	21008,	"If your dog were this ugly, you'd lock him in a fireplace too!"}, /* Dog, OR */
		{	 9206,	"It can be electrifying scaling a dragon; a big guy must have done it."}, /* Chieftain, HGS */
		{	 6524,	"The dwarven mazekeeper has the only pair, if you can find him."}, /* Mazekeeper, Dwarf Mines */
		{	 1533,	"Three witches in the mage's tower have the orb you need"}, /* Pot, MT */
		/* 40 */
		{	 9425,	"A huge gemstone, guarded by ice toads, beware their poison"},
		{	 5113,	"The weapon of a maiden, shaped like a goddess"},
		{	21014,	"The dagger of a yellow-belly"},
		{	 5037,	"A thief of great reknown, at least he doesn't use a spoon"},
		{	 1101,	"Elven blade of ancient lore, matches insects blow for blow"},
		{	27000,	"It strikes like a rattlesnake, but not as deadly"},
		{	27409,	"The weapon of a primitive man, just right for killing his mortal foe"},
		{	 1594,	"White wielded by white, glowing white\n\r"},
		{	20001,	"He judges your soul, wields a weapon that shares your name"},
		{	13703,	"Watch for a dragon, he wears that which you seek"}
	}
};

const char* QuestKind [] = {

	"caccia",
	"salvataggio",
	"ricerca",
	"consegna",
	"\0"

};

const char* NameGenStart [] = {
	"A",
	"Ab",
	"Ac",
	"Ad",
	"Af",
	"Agr",
	"Ast",
	"As",
	"Al",
	"Adw",
	"Adr",
	"Ar",
	"B",
	"Br",
	"C",
	"Cr",
	"Ch",
	"Cad",
	"D",
	"Dr",
	"Dw",
	"Ed",
	"Eth",
	"Et",
	"Er",
	"El",
	"Eow",
	"F",
	"Fr",
	"G",
	"Gr",
	"Gw",
	"Gw",
	"Gal",
	"Gl",
	"H",
	"Ha",
	"Ib",
	"Jer",
	"K",
	"Ka",
	"Ked",
	"L",
	"Loth",
	"Lar",
	"Leg",
	"M",
	"Mir",
	"N",
	"Nyd",
	"Ol",
	"Oc",
	"On",
	"P",
	"Pr",
	"R",
	"Rh",
	"S",
	"Sev",
	"T",
	"Tr",
	"Th",
	"Th",
	"V",
	"Y",
	"Yb",
	"Z",
	"W",
	"W",
	"Wic",
	"\0"
};

const char* NameGenMid [] = {
	"a",
	"ae",
	"ai",
	"au",
	"ao",
	"are",
	"ale",
	"ali",
	"ay",
	"ardo",
	"e",
	"ei",
	"ea",
	"ea",
	"eri",
	"era",
	"ela",
	"eli",
	"enda",
	"erra",
	"i",
	"ia",
	"ie",
	"ire",
	"ira",
	"ila",
	"ili",
	"ira",
	"igo",
	"o",
	"oa",
	"oi",
	"oe",
	"ore",
	"u",
	"y",
	"\0"
};

const char* NameGenEnd [] = {
	"a",
	"and",
	"b",
	"bwyn",
	"baen",
	"bard",
	"c",
	"ctred",
	"cred",
	"ch",
	"can",
	"d",
	"dan",
	"don",
	"der",
	"dric",
	"dfrid",
	"dus",
	"f",
	"g",
	"gord",
	"gan",
	"l",
	"li",
	"lgrin",
	"lin",
	"lith",
	"lath",
	"loth",
	"ld",
	"ldric",
	"ldan",
	"m",
	"mas",
	"mos",
	"mar",
	"mond",
	"n",
	"nydd",
	"nidd",
	"nnon",
	"nwan",
	"nyth",
	"nad",
	"nn",
	"nnor",
	"nd",
	"p",
	"r",
	"ron",
	"rd",
	"s",
	"sh",
	"seth",
	"sean",
	"t",
	"th",
	"the",
	"tha",
	"tlan",
	"trem",
	"tram",
	"v",
	"vudd",
	"w",
	"wan",
	"win",
	"wyn",
	"wyr",
	"wyth",
	"\0"
};


const char* att_kick_kill_ch[] = {
	"Il tuo calcio sfonda il torace di $N causandone la morte immediata.",
	"Il tuo calcio distrugge un braccio a $N e gli sfonda la cassa toracica.",
	"Il tuo piede colpisce $N all'inguine. La morte e' immediata.",
	"Il tuo calcio fracassa il cranio di $N.",
	"Calci $N in pieno viso; la sua mascella si frantuma e la sua vita si spegne.",
	"Colpisci $N alle spalle con un calcio violentissimo. $N stramazza e muore all'istante.",
	"Calci $N nella pancia, frantumando diverse costole. $N muore all'istante.",
	"Le ruvide scaglie di $N si sfondano sotto il tuo piede, $N muore all'istante.",
	"Il tuo calcio manda brandelli di corteccia e foglie in ogni dove, uccidendo $N.",
	"Pezzetti di $N si spargono tutt'attorno, ne demolisci il corpo con un gran calcio!",
	"Con un calcio fai volare $N in mezzo alla stanza. Atterra sotto forma di carne trita.",
	"Colpisci $N all'inguine. Muore strillando con toni degni di un soprano.",
	".",  /* GHOST */
	"Piume svolazzanti accompagnano il calcio col quale riduci $N in piccoli pezzi.",
	"Demolisci $N con un calcio, frammenti di carne decomposta volano tutt'attorno.",
	"Rovesci con un calcio $N, che muore all'istante.",
	"Il tuo piede colpisce con violenza la cartilagine di $N, spargendone pezzi ovunque.",
	"Con un forte calcio colpisci le branchie di $N e ne causi la morte immediata.",
	"Il tuo poderoso calcio spedisce $N direttamente alla tomba.",
	"."
};

const char* att_kick_kill_victim[] = {
	"$n ti sferra un calcio in pieno torace uccidendoti.",
	"$n con un calcio ti frantuma un braccio e mezza cassa toracica. Sei mort$b.",
	"Il calcio di $n ti apre in due all'altezza dell'inguine e ti toglie la vita all'istante.",
	"$n apre in due la tua testa. Muori all'istante.",
	"Il piede di $n attraversa le tue mascelle sino a toccarti la parte inferiore del cervello.",
	"$n ti colpisce da dietro, disarticola la tua spina dorsale e ti uccide.",
	"$n, con un calcio nello stomaco, ti spedisce all'altro mondo!!",
	"Le tue scaglie non reggono il potente calcio di $n.",
	"$n ti demolisce con un potente calcio, muori in un turbinio di foglie.",
	"Di te non rimangono che briciole, dopo il calcio di $n.",
	"Il calcio di $n ti fa' volare via, muori prima ancora di atterrare.",
	"$n ti atterra e ti uccide con un forte calcio all'inguine, OUCH!",
	".", /* GHOST */
	"Le tue piume svolazzan via quando $n ti polverizza con un gran calcio!",
	"$n con un calcio riduce il tuo corpo in polvere e rimasugli di carne decomposta.",
	"$n ti colpisce con un calcio cosi' forte da ucciderti all'istante.",
	"$n scuote violentemente il tuo esoscheletro con un potente calcio, uccidendoti.",
	"$n sferra un calcio sulle tue branchie, non puoi piu' respirare! Non ti resta che morire...",
	"$n ti spedisce nella tomba con un poderoso calcio.",
	"."
};

const char* att_kick_kill_room[] = {
	"$n colpisce $N al torace, con rumore di costole frantumate.",
	"$n calcia nel fianco $N, ne distrugge un braccio e diverse costole.",
	"$n punzona all'inguine $N, che muore tra atroci dolori.",
	"$n frantuma la testa di $N, riducendola in un ammasso di frattaglie!",
	"$n devasta il volto di $N con un calcio che ne annienta ossa e vita.",
	"$n uccide $N con un potente calcio nel posteriore!",
	"$n manda diretto alla tomba $N con un calcione dritto nello stomaco!",
	"Il calcio di $n non teme le scaglie di $N, che uccide con un perfetto calcio.",
	"$n distribuisce brandelli di corteccia e foglie tutt'attorno, aprendo in due $N con un calcio.",
	"$n manda in pezzi $N con un calcio feroce.",
	"$n manda in orbita $N, che atterra con un grosso TONFO cui segue solo silenzio.",
	"Di $N non resta che carne trita, quando $n lo colpisce con un gran calcio.",
	".", /* GHOST */
	"$N scompare in una nuvola di piume quando il calcio di $n ne causa la morte.",
	"Il potente calcio di $n tramuta il corpo decomposto di $N in ossa e polvere.",
	"$n con un potentissimo calcio uccide all'istante $N.",
	"Il calcio di $n riduce l'esoscheletro di $N in piccoli frammenti.",
	"$n centra col piede le branchie di $N, causandone immediata morte.",
	"$n manda alla tomba $N con un poderoso calcione!",
	"."
};

const char* att_kick_miss_ch[] = {
	"$N fa' un passo indietro e scansa il tuo calcio.",
	"$N, abilmente, blocca il tuo calcio con un braccio.",
	"$N si scansa, il tuo calcio manca le sue gambe.",
	"$N si abbassa e il tuo calcio vola alto mille miglia.",
	"$N arretra e sogghigna con malvagita', quando il tuo piede sfiora il suo volto.",
	"$N irride il tuo goffo tentativo di colpirl$b nel sedere!",
	"Il tuo calcio nella pancia fa' ridere $N!",
	"$N se la ride, mentre il tuo calcio rimbalza sulle sue robuste scaglie.",
	"Colpisci $N nel fianco, ammaccandoti il piede.",
	"$N scansa facilmente il tuo pietoso tentativo di calcio.",
	"Non hai ben calcolato l'altezza di $N. Il tuo piede vola ben oltre la sua testa.",
	"T'ammacchi l'alluce contro il grosso piede di $N, quando tenti di calciarlo.",
	"Il tuo calcio attraversa $N!!",  /* Ghost */
	"$N vola via con agilita', il tuo calcio e' fallito.",
	"$N si sposta da un lato, scansa il tuo calcio e ti ride in faccia.",
	"Il tuo calcio rimbalza sulla spessa pelle di $N.",
	"Il tuo calcio rimbalza sul robusto esoscheletro di $N.",
	"$N allontana il tuo piede con un colpo di pinna.",
	"$N evita senza problemi il tuo goffo tentativo di calcio.",
	"."
};

const char* att_kick_miss_victim[] = {
	"$n ti manca, il suo tentativo di calcio al torace e' ridicolo.",
	"Blocchi col braccio il fiacco calcio di $n.",
	"Scansi agilmente il debole calcio di $n.",
	"Ti abbassi e il calcio di $n vola alto sulla tua testa.",
	"Fai un saltino indietro e ridi del goffo tentativo di $n di colpirti al volto.",
	"Il piede di $n tenta goffamente di colpire la tua spalla, non hai problemi ad evitarlo.",
	"Ridi del goffo tentativo di $n di affondare il suo piede nel tuo stomaco.",
	"$n ti calcia, ma le tue scaglie son troppo robuste perche' tu ne risenta.",
	"Ridi quando $n s'ammacca il piede sul tuo tronco.",
	"Schivi con agilita' il goffo calcio di $n.",
	"Il calcio di $n non fa' piu' che scompigliarti i capelli.",
	"Il calcetto di $n non scalfisce minimamente la tua pelle.",
	"$n ti attraversa con un inutile e ridicolo calcio.",
	"Agilmente svolazzi lontano dal piede di $n.",
	"Ti fai beffe di $n scansando il suo piede con un'abile mossa.",
	"Il piede di $n rimbalza sulla tua spessa pelle.",
	"$n tenta di colpirti con un calcio... ma sei troppo duro per il suo piede!",
	"$n tenta di colpirti con un piede, che devii con un colpo di pinna.",
	"Eviti il ridicolo tentativo di $n di colpirti con un calcio.",
	"."
};

const char* att_kick_miss_room[] = {
	"$n manca $N con un goffo tentativo di calcio.",
	"$N ferma il piede di $n con un braccio.",
	"$N scansa con facilita' il ridicolo calcetto di $n.",
	"$N si abbassa rapidamente evitando il calcio di $n.",
	"$N fa' un passo indietro e sogghigna maligno, scansando il piede di $n diretto al suo volto.",
	"$n tenta di colpire $N con una pedata nel sedere, fallendo ridicolmente.",
	"$N ride del goffo tentativo di $n di colpirlo nello stomaco con un calcio.",
	"$n tenta di colpire $N con una pedata che rimbalza sulla corazza di scaglie.",
	"$n s'ammacca il piede tentando di prendere a calci $N.",
	"$N evita l'insidioso calcio di $n.",
	"Il calcio di $n manca $N per le sue piccole dimensioni.",
	"$n non riesce a centrare $N all'inguine, e si ammacca l'alluce.",
	"Il piede di $n attraversa $N!!!!",
	"$N svolazza lontano dal piede di $n.",
	"$N ride di $n mentre ne scansa il calcio con abilita'.",
	"La dura scorza di $N respinge il calcio di $n.",
	"$n s'ammacca il piede sul robusto esoscheletro di $N.",
	"$n tenta di colpire $N con un calcio, ma si vede respinto da una pinna.",
	"$N evita il fiacco calcio di $n.",
	"."
};

const char* att_kick_hit_ch[] = {
	"Il tuo calcio colpisce $N al torace.",
	"Centri $N nel fianco con un calcio ben assestato.",
	"Colpisci all'inguine $N con una sonora pedata.",
	"$N perde l'orientamento quando il tuo calcio lo prende in pieno volto.",
	"Il tuo piede centra la mandibola di $N spostandola.",
	"Fracassi con un calcio la spalla di $N, che indietreggia agonizzante.",
	"Centri $N allo stomaco, $N si piega e impreca.",
	"Il tuo piede trova un punto morbido tra le scaglie di $N e vi lascia un bel segno!",
	"Il tuo calcio colpisce $N facendo svolazzare foglie e rametti.",
	"Con un gran calcio spargi tutt'attorno piccoli frammenti di $N.",
	"Assesti un calcione nello stomaco di $N togliendogli il respiro.",
	"Pesti pesantemente un piede a $N. Non puoi far di piu' contro un gigante...",
	".", /* GHOST */
	"Il tuo gran calcio manda $N in orbita.",
	"Calci $N, senti il rumore delle ossa fracide che si sbriciolano...",
	"Centri $N con un calcio ben assestato.",
	"Il tuo calcio spezza l'esoscheletro di $N.",
	"Con un gran calcio sposti le scaglie di $N.",
	"Ti alzi di scatto e centri $N con un potente calcio!",
	"."
};

const char* att_kick_hit_victim[] = {
	"Senti il piede di $n spostarti lo sterno.",
	"Il calcio di $n colpisce in pieno il tuo fianco.",
	"Con un calcio $n ti sposta di fianco, per un attimo perdi l'equilibrio.",
	"Vedi il piede di $n, e subito dopo un arcobaleno di colori...",
	"$n ti calcia in pieno volto.",
	"Il piede di $n centra la tua spalla, AHIA!",
	"Con un calcio nello stomaco $n ti toglie il respiro.",
	"$n trova uno spazio tenero tra le tue squame e ti rifila una sonora pedata.",
	"$n ti calcia cosi' forte da levarti qualche etto di foglie e rametti!",
	"$n ti colpisce nel fianco con un calcio, guardi piccole parti di te abbandonarti...",
	"Ouch! Un calcio di $n ha centrato le tue costole.",
	"$n atterra pesantemente sul tuo piede, che male!",
	".", /* GHOST */
	"La pedata di $n ti manda in volo nell'intorno...",
	"$n ti colpisce con un calcio, le tue ossa emettono sinistri rumori...",
	"$n ti centra con un gran calcio.",
	"$n ti frantuma qualche scaglia con un ben piazzato calcio.",
	"$n si alza di scatto e ti assesta un calcione doloroso.",
	"."
};

const char* att_kick_hit_room[] = {
	"$n centra $N al torace con un bel calcio.",
	"$n punzona il fianco $N con una sonora pedata.",
	"$n sposta di peso $N con un potente calcio.",
	"Gli occhi di $N ruotano per il calcio in pieno volto di $n.",
	"$n sposta la mandibola di $N con un poderoso calcio.",
	"$n centra la spalla di $N con un calcio ben assestato.",
	"$N perde il respiro per il calcio allo stomaco di $n.",
	"$n trova un punto morbido tra le scaglie di $N e vi dirige il piede con maestria.",
	"$n centra $N con un gran calcio che stacca foglie e rametti.",
	"$n colpisce $N con un calcio potente, spargendo frammenti del suo corpo.",
	"$n calcia $N nello stomaco, $N perde il respiro.",
	"$n pesta con violenza un piede di $N, che saltella dolorante.",
	".", /* GHOST */
	"$n manda in orbita $N con un sonoro calcione.",
	"Il calcio di $n fa collassare parte del marcescente corpo di $N!",
	"$n centra il fianco di $N con un poderoso calcio.",
	"$n spezza l'esoscheletro di $N con un potente calcio.",
	"$n calcia $N con tal forza che ne stacca qualche squama!",
	"$n scatta in piedi e centra $N con un gran calcio.",
	"."
};

struct XpAchieTable RewardXp[ABS_MAX_LVL] = {
	//	lev_1_xp	lev_2_xp	lev_3_xp	lev_4_xp	lev_5_xp	lev_6_xp	lev_7_xp	lev_8_xp	lev_9_xp	lev_10_xp
	{		  1,		  1,		  1,		  1,		  1,		  1,		  1,		  1,		  1,		  1	},	//	 0
	{		399,		439,		479,		519,		559,		599,		639,		679,		719,		799	},	//	NOVIZIO
	{		850,		935,	   1020,	   1105,	   1190,	   1275,	   1360,	   1445,	   1530,	   1700	},	//	 2
	{	   1000,	   1100,	   1200,	   1300,	   1400,	   1500,	   1600,	   1700,	   1800,	   2000	},	//	 3
	{	   1250,	   1375,	   1500,	   1625,	   1750,	   1875,	   2000,	   2125,	   2250,	   2500	},	//	 4
	{	   1600,	   1760,	   1920,	   2080,	   2240,	   2400,	   2560,	   2720,	   2880,	   3200	},	//	 5
	{	   1900,	   2090,	   2280,	   2470,	   2660,	   2850,	   3040,	   3230,	   3420,	   3800	},	//	 6
	{	   2300,	   2530,	   2760,	   2990,	   3220,	   3450,	   3680,	   3910,	   4140,	   4600	},	//	 7
	{	   2800,	   3080,	   3360,	   3640,	   3920,	   4200,	   4480,	   4760,	   5040,	   5600	},	//	 8
	{	   3500,	   3850,	   4200,	   4550,	   4900,	   5250,	   5600,	   5950,	   6300,	   7000	},	//	 9
	{	   4500,	   4950,	   5400,	   5850,	   6300,	   6750,	   7200,	   7650,	   8100,	   9000	},	//	10
	{	   5500,	   6050,	   6600,	   7150,	   7700,	   8250,	   8800,	   9350,	   9900,	  11000	},	//	ALLIEVO
	{	   6500,	   7150,	   7800,	   8450,	   9100,	   9750,	  10400,	  11050,	  11700,	  13000	},	//	12
	{	   8000,	   8800,	   9600,	  10400,	  11200,	  12000,	  12800,	  13600,	  14400,	  16000	},	//	13
	{	   9500,	  10450,	  11400,	  12350,	  13300,	  14250,	  15200,	  16150,	  17100,	  19000	},	//	14
	{	  12500,	  13750,	  15000,	  16250,	  17500,	  18750,	  20000,	  21250,	  22500,	  25000	},	//	15
	{	  13500,	  14250,	  15750,	  17250,	  18000,	  19500,	  21000,	  22500,	  24000,	  27000	},	//	16
	{	  16650,	  17575,	  19425,	  21275,	  22200,	  24050,	  25900,	  27750,	  29600,	  33300	},	//	17
	{	  20700,	  21850,	  24150,	  26450,	  27600,	  29900,	  32200,	  34500,	  36800,	  41400	},	//	18
	{	  25200,	  26600,	  29400,	  32200,	  33600,	  36400,	  39200,	  42000,	  44800,	  50400	},	//	19
	{	  31050,	  32775,	  36225,	  39675,	  41400,	  44850,	  48300,	  51750,	  55200,	  62100	},	//	20
	{	  33600,	  35700,	  37800,	  39900,	  42000,	  46200,	  50400,	  54600,	  58800,	  67200	},	//	APPRENDISTA
	{	  42000,	  44625,	  47250,	  49875,	  52500,	  57750,	  63000,	  68250,	  73500,	  84000	},	//	22
	{	  51200,	  54400,	  57600,	  60800,	  64000,	  70400,	  76800,	  83200,	  89600,	 102400	},	//	23
	{	  63200,	  67150,	  71100,	  75050,	  79000,	  86900,	  94800,	 102700,	 110600,	 126400	},	//	24
	{	  76800,	  81600,	  86400,	  91200,	  96000,	 105600,	 115200,	 124800,	 134400,	 153600	},	//	25
	{	  84000,	  90000,	  96000,	 102000,	 108000,	 120000,	 132000,	 144000,	 156000,	 180000	},	//	26
	{	 105000,	 112500,	 120000,	 127500,	 135000,	 150000,	 165000,	 180000,	 195000,	 225000	},	//	27
	{	 126000,	 135000,	 144000,	 153000,	 162000,	 180000,	 198000,	 216000,	 234000,	 270000	},	//	28
	{	 154000,	 165000,	 176000,	 187000,	 198000,	 220000,	 242000,	 264000,	 286000,	 330000	},	//	29
	{	 165000,	 178750,	 192500,	 206250,	 220000,	 233750,	 255750,	 277750,	 302500,	 357500	},	//	30
	{	 170850,	 184250,	 199325,	 212725,	 226125,	 241200,	 264650,	 288100,	 314900,	 378550	},	//	INIZIATO
	{	 174300,	 190900,	 207500,	 221195,	 236550,	 253150,	 277220,	 302120,	 329925,	 394250	},	//	32
	{	 178500,	 196350,	 215220,	 230010,	 246330,	 265200,	 280500,	 296310,	 314160,	 410550	},	//	33
	{	 182700,	 202860,	 223650,	 239400,	 256410,	 277200,	 293895,	 310590,	 329490,	 427140	},	//	34
	{	 187110,	 208670,	 232540,	 249480,	 267575,	 290290,	 307230,	 325710,	 345730,	 446600	},	//	35
	{	 192080,	 214620,	 241570,	 259700,	 278810,	 303800,	 321440,	 341040,	 362110,	 466480	},	//	36
	{	 197225,	 220800,	 251275,	 270250,	 290375,	 317400,	 335800,	 356500,	 379500,	 486450	},	//	37
	{	 202860,	 227080,	 261800,	 281400,	 302400,	 331100,	 350700,	 372400,	 397600,	 508200	},	//	38
	{	 208620,	 233460,	 272700,	 292860,	 315000,	 345600,	 366480,	 389340,	 416700,	 531000	},	//	39
	{	 215000,	 240800,	 284875,	 306375,	 330025,	 363350,	 385710,	 410650,	 440750,	 559000	},	//	40
	{	 226800,	 253800,	 302940,	 326160,	 353700,	 390150,	 414450,	 442260,	 476550,	 604800	},	//	ESPERTO
	{	 247500,	 275880,	 330330,	 355740,	 387750,	 428340,	 455400,	 487740,	 528000,	 669900	},	//	42
	{	 279450,	 309825,	 369765,	 397710,	 434565,	 480330,	 511231,	 549585,	 599400,	 761400	},	//	43
	{	 324225,	 356895,	 422730,	 453915,	 496980,	 549351,	 586575,	 633600,	 696960,	 886050	},	//	44
	{	 387450,	 424350,	 496305,	 531360,	 581790,	 642675,	 688800,	 747840,	 830865,	1054417	},	//	45
	{	 490750,	 532275,	 610417,	 650055,	 709700,	 781425,	 838050,	 913550,	1022270,	1291050	},	//	MAESTRO
	{	 610500,	 656750,	 741850,	 787175,	 858400,	 943500,	1013337,	1109075,	1248750,	1572500	},	//	47
	{	 744250,	 796920,	 889665,	 943480,	1028782,	1130115,	1216219,	1336215,	1514835,	1905280	},	//	48
	{	 896000,	 959000,	1059800,	1122800,	1223600,	1344000,	1450400,	1596000,	1824200,	2296000	},	//	49
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	BARONE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	PRINCIPE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	IMMORTALE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	DIO_MINORE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	DIO
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	MAESTRO_DEGLI_DEI
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	CREATORE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	QUESTMASTER
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	MAESTRO_DEL_CREATO
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	MAESTRO_DEI_CREATORI
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	IMMENSO
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	65
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	}
};

struct RandomMaterialsTable MaterialName[5][100] = {
	//	organic 16
	{	//	female								neutral								4										5									key
		{	"d'ossa",							"d'ossa",							"d'ossa",								"d'ossa",							" ossa"						},
		{	"di legno",							"di legno",							"di legno",								"di legno",							" legno"					},
		{	"in pelle di drago",				"in pelle di drago",				"in pelle di drago",					"in pelle di drago",				" pelle drago"				},
		{	"ricoperta di conchiglie",			"ricoperto di conchiglie",			"ricoperte di conchiglie",				"ricoperti di conchiglie",			" conchiglie"				},
		{	"di legno di quercia",				"di legno di quercia",				"di legno di quercia",					"di legno di quercia",				" quercia"					},
		{	"di seta",							"di seta",							"di seta",								"di seta",							" seta"						},
		{	"di ebano",							"di ebano",							"di ebano",								"di ebano",							" ebano"					},
		{	"ricoperta da una strana corteccia","ricoperto da una strana corteccia","ricoperte da una strana corteccia",	"ricoperti da una strana corteccia"," corteccia"				},
		{	"di sangue condensato",				"di sangue condensato",				"di sangue condensato",					"di sangue condensato",				" sangue"					},
		{	"di ghiaccio",						"di ghiaccio",						"di ghiaccio",							"di ghiaccio",						" ghiaccio"					},
		{	"in pelle di lupo",					"in pelle di lupo",					"in pelle di lupo",						"in pelle di lupo",					" pelle lupo"				},
		{	"in pelle di serpente",				"in pelle di serpente",				"in pelle di serpente",					"in pelle di serpente",				" pelle serpente"   		},
		{	"di cuoio",							"di cuoio",							"di cuoio",								"di cuoio",							" cuoio"					},
		{	"di ebano nero",					"di ebano nero",					"di ebano nero",						"di ebano nero",					" ebano nero"				},
		{	"di pelle",							"di pelle",							"di pelle",								"di pelle",							" pelle"					},
		{	"di legno di frassino",				"di legno di frassino",				"di legno di frassino",					"di legno di frassino",				" frassiono"				}
	},
	//	metallic    17
	{	//	female								neutral								4										5									key
		{	"d'oro",							"d'oro",							"d'oro",								"d'oro",							" oro"						},
		{	"di bronzo",						"di bronzo",						"di bronzo",							"di bronzo",						" bronzo"					},
		{	"di rame",							"di rame",							"di rame",								"di rame",							" rame"						},
		{	"di argento",						"di argento",						"di argento",							"di argento",						" argento"					},
		{	"di ferro",							"di ferro",							"di ferro",								"di ferro",							" ferro"					},
		{	"di mithril",						"di mithril",						"di mithril",							"di mithril",						" mithril"					},
		{	"di platino",						"di platino",						"di platino",							"di platino",						" platino"					},
		{	"di vibranio",						"di vibranio",						"di vibranio",							"di vibranio",						" vibranio"					},
		{	"di metallo",						"di metallo",						"di metallo",							"di metallo",						" metallo"					},
		{	"di acciaio",						"di acciaio",						"di acciaio",							"di acciaio",						" acciaio"					},
		{	"di stagno",						"di stagno",						"di stagno",							"di stagno",						" stagno"					},
		{	"di adamantio",						"di adamantio",						"di adamantio",							"di adamantio",						" adamantio"				},
		{	"di titanio",						"di titanio",						"di titanio",							"di titanio",						" titanio"					},
		{	"di gromril",						"di gromril",						"di gromril",							"di gromril",						" gromril"					},
		{	"di warpietra",						"di warpietra",						"di warpietra",							"di warpietra",						" warpietra"				},
		{	"di saronite",						"di saronite",						"di saronite",							"di saronite",						" saronite"					},
		{	"di azerite",						"di azerite",						"di azerite",							"di azerite",						" azerite"					}
	},
	//	various 29
	{	//	female								neutral								4										5									key
		{	"antica",							"antico",							"antiche",								"antichi",							""							},
		{	"maledetta",						"maledetto",						"maledette",							"maledetti",						""							},
		{	"benedetta",						"benedetto",						"benedette",							"benedetti",						""							},
		{	"invisibile",						"invisibile",						"invisibili",							"invisibili",						""							},
		{	"sacra",							"sacro",							"sacre",								"sacri",							""							},
		{	"magica",							"magico",							"magiche",								"magici",							""							},
		{	"molto antica",						"molto antico",						"molto antiche",						"molto antichi",					""							},
		{	"malandata",						"malandato",						"malandate",							"malandati",						""							},
		{	"infuocata",						"infuocato",						"infuocate",							"infuocati",						""							},
		{	"rara",								"raro",								"rare",									"raroi",							""							},
		{	"rinforzata",						"rinforzato",						"rinforzate",							"rinforzati",						""							},
		{	"scheggiata",						"scheggiato",						"scheggiate",							"scheggiati",						""							},
		{	"con delle crepe",					"con delle crepe",					"con delle crepe",						"con delle crepe",					" crepe"					},
		{	"ammaccata",						"ammaccato",						"ammaccate",							"ammaccati",						""							},
		{	"insanguinata",						"insanguinato",						"insanguinate",							"insanguinati",						""							},
		{	"cesellata",						"cesellato",						"cesellate",							"cesellati",						""							},
		{	"ammantata d'Ombra",				"ammantato d'Ombra",				"ammantate d'Ombra",					"ammantati d'Ombra",				" ombra"					},
		{	"luminosa",							"luminoso",							"luminose",								"luminosi",							""							},
		{	"risplendete di luce",				"risplendete di luce",				"risplendeti di luce",					"risplendeti di luce",				" luce"						},
		{	"d'Ombra",							"d'Ombra",							"d'Ombra",								"d'Ombra",							" ombra"					},
		{	"intarsiata",						"intarsiato",						"intarsiate",							"intarsiati",						""							},
		{	"con venature metalliche",			"con venature metalliche",			"con venature metalliche",				"con venature metalliche",			" venature"					},
		{	"decorata",							"decorato",							"decorate",								"decorati",							""							},
		{	"molto ben lavorata",				"molto ben lavorato",				"molto ben lavorate",					"molto ben lavorati",				""							},
		{	"trasparente",						"trasparente",						"trasparenti",							"trasparenti",						""							},
		{	"leggendaria",						"leggendario",						"leggendarie",							"leggendari",						""							},
		{	"usurata",							"usurato",							"usurate",								"usurati",							""							},
		{	"di rara bellezza",					"di rara bellezza",					"di rara bellezza",						"di rara bellezza",					""							},
		{	"scintillante",						"scintillante",						"scintillanti",							"scintillanti",						""							}
	},
	//	colour 24
	{	//	female								neutral								4										5									key
		{	"$c0009rossa$c0007",				"$c0009rosso$c0007",				"$c0009rosse$c0007",					"$c0009rossi$c0007",				""							},
		{	"$c0008nera$c0007",					"$c0008nero$c0007",					"$c0008nere$c0007",						"$c0008neri$c0007",					""							},
		{	"$c0015bianca$c0007",				"$c0015bianco$c0007",				"$c0015bianche$c0007",					"$c0015bianchi$c0007",				""							},
		{	"$c0010verde$c0007",				"$c0010verde$c0007",				"$c0010verdi$c0007",					"$c0010verdi$c0007",				""							},
		{	"$c0011gialla$c0007",				"$c0011giallo$c0007",				"$c0011gialle$c0007",					"$c0011gialli$c0007",				""							},
		{	"$c0005viola$c0007",				"$c0005viola$c0007",				"$c0005viola$c0007",					"$c0005viola$c0007",				""							},
		{	"arancione",						"arancione",						"arancioni",							"arancioni",						""							},
		{	"$c0004blu$c0007",					"$c0004blu$c0007",					"$c0004blu$c0007",						"$c0004blu$c0007",					""							},
		{	"$c0012azzurra$c0007",				"$c0012azzurro$c0007",				"$c0012azzurre$c0007",					"$c0012azzurri$c0007",				""							},
		{	"$c0012in$c0005da$c0012co$c0007",	"$c0012in$c0005da$c0012co$c0007",	"$c0012in$c0005da$c0012co$c0007",		"$c0012in$c0005da$c0012co$c0007",	""							},
		{	"$c0003marrone$c0007",				"$c0003marrone$c0007",				"$c0003marroni$c0007",					"$c0003marroni$c0007",				""							},
		{	"$c0008nero pece$c0007",			"$c0008nero pece$c0007",			"$c0008nero pece$c0007",				"$c0008nero pece$c0007",			""							},
		{	"$c0011giallo canarino$c0007",		"$c0011giallo canarino$c0007",		"$c0011giallo canarino$c0007",			"$c0011giallo canarino$c0007",		""							},
		{	"$c0012blu intenso$c0007",			"$c0012blu intenso$c0007",			"$c0012blu intenso$c0007",				"$c0012blu intenso$c0007",			""							},
		{	"$c0004blu mare$c0007",				"$c0004blu mare$c0007",				"$c0004blu mare$c0007",					"$c0004blu mare$c0007",				""							},
		{	"$c0001amaranto$c0007",				"$c0001amaranto$c0007",				"$c0001amaranto$c0007",					"$c0001amaranto$c0007",				""							},
		{	"$c0009rosso fuoco$c0007",			"$c0009rosso fuoco$c0007",			"$c0009rosso fuoco$c0007",				"$c0009rosso fuoco$c0007",			""							},
		{	"$c0011dorata$c0007",				"$c0011dorato$c0007",				"$c0011dorate$c0007",					"$c0011dorati$c0007",				""							},
		{	"$c0015argentata$c0007",			"$c0015argentato$c0007",			"$c0015argentate$c0007",				"$c0015argentati$c0007",			""							},
		{	"$c0014celeste$c0007",				"$c0014celeste$c0007",				"$c0014celesti$c0007",					"$c0014celesti$c0007",				""							},
		{	"grigia",							"grigio",							"grigie",								"grigi",							""							},
		{	"bianco sporco",					"bianco sporco",					"bianco sporco",						"bianco sporco",					""							},
		{	"$c0013rosa$c0007",					"$c0013rosa$c0007",					"$c0013rosa$c0007",						"$c0013rosa$c0007",					""							},
		{	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014e$c0007",	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014e$c0007",	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014i$c0007",	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014i$c0007",	""	}
	},
	//	mineral 15
	{	//	female								neutral								4										5									key
		{	"di roccia",						"di roccia",						"di roccia",							"di roccia",						" roccia"					},
		{	"di ossidiana",						"di ossidiana",						"di ossidiana",							"di ossidiana",						" ossidiana"				},
		{	"di argilla",						"di argilla",						"di argilla",							"di argilla",						" argilla"					},
		{	"di diamante",						"di diamante",						"di diamante",							"di diamante",						" diamante"					},
		{	"di cristallo",						"di cristallo",						"di cristallo",							"di cristallo",						" cristallo"				},
		{	"di vetro",							"di vetro",							"di vetro",								"di vetro",							" vetro"					},
		{	"di granito",						"di granito",						"di granito",							"di granito",						" granito"					},
		{	"di marmo",							"di marmo",							"di marmo",								"di marmo",							" marmo"					},
		{	"di quarzo",						"di quarzo",						"di quarzo",							"di quarzo",						" quarzo"					},
		{	"in resina",						"in resina",						"in resina",							"in resina",						" resina"					},
		{	"di basalto",						"di basalto",						"di basalto",							"di basalto",						" basalto"					},
		{	"di avorio",						"di avorio",						"di avorio",							"di avorio",						" avorio"					},
		{	"di kryptonite",					"di kryptonite",					"di kryptonite",						"di kryptonite",					" kryptonite"				},
		{	"d'ambra",							"d'ambra",							"d'ambra",								"d'ambra",							" ambra"					},
		{	"di bachelite",						"di bachelite",						"di bachelite",							"di bachelite",						" bachelite"				}
	}
};

struct RandomEquipTable EquipName[22][20] = {
						//	4 = plurale femminile		5 = plurale neutro
	{	//  finger 9					gender			key
		{	"un anello",				SEX_NEUTRAL,	"anello"				},
		{	"una fede",					SEX_FEMALE,		"fede"      			},
		{	"un anellino",				SEX_NEUTRAL,	"anellino"  			},
		{	"una fedina",				SEX_FEMALE,		"fedina"    			},
		{	"un cerchietto",			SEX_NEUTRAL,	"cerchietto"			},
		{	"un simbolo",				SEX_NEUTRAL,	"simbolo"   			},
		{	"un sigillo",				SEX_NEUTRAL,	"sigillo"   			},
		{	"un tirapugni",				SEX_NEUTRAL,	"tirapugni" 			},
		{	"una fascetta",				SEX_FEMALE,		"fascetta"  			}
	},
	{	//	wrist 9						gender			key
		{	"un bracciale",				SEX_NEUTRAL,	"bracciale"				},
		{	"un braccialetto",			SEX_NEUTRAL,	"braccialetto"			},
		{	"otto braccialetti",		5,				"otto braccialetti"		},
		{	"un ciondolo",				SEX_NEUTRAL,	"ciondolo"				},
		{	"una catenina",				SEX_FEMALE,		"catenina"				},
		{	"un cinturino",				SEX_NEUTRAL,	"cinturino"				},
		{	"delle manette",			4,				"manette"				},
		{	"una fascia",				SEX_FEMALE,		"fascia"				},
		{	"un cerchietto",			SEX_NEUTRAL,	"cerchietto"			}
	},
	{	//	ear 9						gender			key
		{	"un orecchino",				SEX_NEUTRAL,	"orecchino"				},
		{	"un pendente",				SEX_NEUTRAL,	"pendente"				},
		{	"un gioiello",				SEX_NEUTRAL,	"gioiello"				},
		{	"un monile",				SEX_NEUTRAL,	"monile"				},
		{	"una buccola",				SEX_FEMALE,		"buccola"				},
		{	"degli orecchini",			5,				"orecchini"				},
		{	"un anellino",				SEX_NEUTRAL,	"anellino"				},
		{	"un cerchio",				SEX_NEUTRAL,	"cerchio"				},
		{	"una gocciola",				SEX_FEMALE,		"gocciola"				}
	},
	{	//	neck 11						gender			key
		{	"una collana",				SEX_FEMALE,		"collana"				},
		{	"un ciondolo",				SEX_NEUTRAL,	"ciondolo"				},
		{	"una catenina",				SEX_FEMALE,		"catenina"				},
		{	"un amuleto",				SEX_NEUTRAL,	"amuleto"				},
		{	"un medaglione",			SEX_NEUTRAL,	"medaglione"			},
		{	"uno scarabeo",				SEX_NEUTRAL,	"scarabeo"				},
		{	"un pendaglio",				SEX_NEUTRAL,	"pendaglio"				},
		{	"un talismano",				SEX_NEUTRAL,	"talismano"				},
		{	"un monile",				SEX_NEUTRAL,	"monile"				},
		{	"un girocollo",				SEX_NEUTRAL,	"girocollo"				},
		{	"un collare",				SEX_NEUTRAL,	"collare"				}
	},
	{	//	eyes 11						gender			key
		{	"degli occhiali",			5,				"occhiali"				},
		{	"un monocolo",				SEX_NEUTRAL,	"monocolo"				},
		{	"una visiera",				SEX_FEMALE,		"visiera"				},
		{	"delle lenti",				4,				"lenti"					},
		{	"una lente",				SEX_FEMALE,		"lente"					},
		{	"una maschera",				SEX_FEMALE,		"maschera"				},
		{	"una mascherina",			SEX_FEMALE,		"mascherina"			},
		{	"un occhio",				SEX_NEUTRAL,	"occhio"				},
		{	"una benda",				SEX_FEMALE,		"benda"					},
		{	"una celata",				SEX_FEMALE,		"celata"				},
		{	"un visore",				SEX_NEUTRAL,	"visore"				}
	},
	{	//	hold 11						gender			key
		{	"un tomo",					SEX_NEUTRAL,	"tomo"					},
		{	"un corno",					SEX_NEUTRAL,	"corno"					},
		{	"un simbolo",				SEX_NEUTRAL,	"simbolo"				},
		{	"un grimorio",				SEX_NEUTRAL,	"grimorio"				},
		{	"una collanina",			SEX_FEMALE,		"collanina"				},
		{	"un rosario",				SEX_NEUTRAL,	"rosario"				},
		{	"una pietra",				SEX_FEMALE,		"pietra"				},
		{	"una perla",				SEX_FEMALE,		"perla"					},
		{	"una roccia",				SEX_FEMALE,		"roccia"				},
		{	"una gemma",				SEX_FEMALE,		"gemma"					},
		{	"un sigillo",				SEX_NEUTRAL,	"sigillo"				}
	},
	{	//	light 14					gender			key
		{	"una sfera di luce",		SEX_FEMALE,		"sfera luce"			},
		{	"un diamante brillante",	SEX_NEUTRAL,	"diamante brillante"	},
		{	"una fiamma eterna",		SEX_FEMALE,		"fiamma eterna"			},
		{	"una sfera danzante",		SEX_FEMALE,		"sfera danzante"		},
		{	"una torcia infuocata",		SEX_FEMALE,		"torcia infuocata"		},
		{	"una lampada antica",		SEX_FEMALE,		"lampada antica"		},
		{	"una lanterna da minatore",	SEX_FEMALE,		"lanterna minatore"		},
		{	"una lanterna antica",		SEX_FEMALE,		"lanterna antica"		},
		{	"una piccola stella",		SEX_FEMALE,		"stella piccola"		},
		{	"un frammento di stella",	SEX_NEUTRAL,	"stella frammento"		},
		{	"una gemma splendente",		SEX_FEMALE,		"gemma splendente"		},
		{	"una candela",				SEX_FEMALE,		"candela"				},
		{	"una fatina",				SEX_FEMALE,		"fatina"				},
		{	"un cerino",				SEX_NEUTRAL,	"cerino"				},
		{	"un fiammifero",			SEX_NEUTRAL,	"fiammifero"			}
	},
	{	//	back 10						gender			key
		{	"una borsa",				SEX_FEMALE,		"borsa"					},
		{	"uno zaino",				SEX_NEUTRAL,	"zaino"					},
		{	"una sacca",				SEX_FEMALE,		"sacca"					},
		{	"un forziere",				SEX_NEUTRAL,	"forziere"				},
		{	"un barile",				SEX_NEUTRAL,	"barile"				},
		{	"un cesto",					SEX_NEUTRAL,	"cesto"					},
		{	"una gerla",				SEX_FEMALE,		"gerla"					},
		{	"una cassapanca",			SEX_FEMALE,		"cassapanca"			},
		{	"una bisaccia",				SEX_FEMALE,		"bisaccia"				},
		{	"una cesta",				SEX_FEMALE,		"cesta"					}
	},
	{	//	about body 9				gender			key
		{	"un mantello",				SEX_NEUTRAL,	"mantello"				},
		{	"un manto",					SEX_NEUTRAL,	"manto"					},
		{	"una pelliccia",			SEX_FEMALE,		"pelliccia"				},
		{	"un vello",					SEX_NEUTRAL,	"vello"					},
		{	"una cappa",				SEX_FEMALE,		"cappa"					},
		{	"un tabarro",				SEX_NEUTRAL,	"tabarro"				},
		{	"un sari",					SEX_NEUTRAL,	"sari"					},
		{	"una pianeta",				SEX_FEMALE,		"pianeta"				},
		{	"un caftano",				SEX_NEUTRAL,	"caftano"				}
	},
	{	//	waist 9						gender			key
		{	"una cintura",				SEX_FEMALE,		"cintura"				},
		{	"un nastro",				SEX_NEUTRAL,	"nastro"				},
		{	"un cinturone",				SEX_NEUTRAL,	"cinturone"				},
		{	"una corda",				SEX_FEMALE,		"corda"					},
		{	"una liana",				SEX_FEMALE,		"liana"					},
		{	"un gonnellino",			SEX_NEUTRAL,	"gonnellino"			},
		{	"una fascia",				SEX_FEMALE,		"fascia"				},
		{	"una catena",				SEX_FEMALE,		"catena"				},
		{	"una fusciacca",			SEX_FEMALE,		"fusciacca"				}
	},
	{	//	feet 9						gender			key
		{	"degli stivali",			5,				"stivali"				},
		{	"un paio di stivali",		5,				"stivali paio"			},
		{	"delle scarpe",				4,				"scarpe"				},
		{	"un paio di scarpe",		4,				"scarpe paio"			},
		{	"dei calzari",				5,				"calzari"				},
		{	"degli anfibi",				5,				"anfibi"				},
		{	"dei sandali",				5,				"sandali"				},
		{	"degli scarponi",			5,				"scarponi"				},
		{	"un paio di scarponi",		5,				"scarponi paio"			}
	},
	{	//	hands 9						gender			key
		{	"un paio di guanti",		5,				"guanti paio"			},
		{	"dei guanti",				5,				"guanti"				},
		{	"dei guantoni",				5,				"guantoni"				},
		{	"delle fasce",				4,				"fasce"					},
		{	"delle placche",			4,				"placche"				},
		{	"delle protezioni",			4,				"protezioni"			},
		{	"dei gusci",				5,				"gusci"					},
		{	"degli uncini",				5,				"unicini"				},
		{	"degli artigli",			5,				"artigli"				}
	},
	{	//	body 10						gender			key
		{	"una armatura",				SEX_FEMALE,		"armatura"				},
		{	"una veste",				SEX_FEMALE,		"veste"					},
		{	"una corazza",				SEX_FEMALE,		"corazza"				},
		{	"una cotta",				SEX_FEMALE,		"cotta"					},
		{	"un corpetto",				SEX_NEUTRAL,	"corpetto"				},
		{	"un giustacuore",			SEX_NEUTRAL,	"giustacuore"			},
		{	"una uniforme",				SEX_FEMALE,		"uniforme"				},
		{	"un pettorale",				SEX_NEUTRAL,	"pettorale"				},
		{	"un saio",					SEX_NEUTRAL,	"saio"					},
		{	"un'armatura",				SEX_FEMALE,		"armatura"				},
		{	"un kimono",				SEX_NEUTRAL,	"kimono"				}
	},
	{	//	head 9						gender			key
		{	"un elmo",					SEX_NEUTRAL,	"elmo"					},
		{	"una corona",				SEX_FEMALE,		"corona"				},
		{	"una fascia",				SEX_FEMALE,		"fascia"				},
		{	"una bandana",				SEX_FEMALE,		"bandana"				},
		{	"una testa",				SEX_FEMALE,		"testa"					},
		{	"un cappello",				SEX_NEUTRAL,	"cappello"				},
		{	"un cappuccio",				SEX_NEUTRAL,	"cappuccio"				},
		{	"un diadema",				SEX_NEUTRAL,	"diadema"				},
		{	"un copricapo",				SEX_NEUTRAL,	"copricapo"				}
	},

	{	//	legs 9						gender			key
		{	"dei gambali",				5,				"gambali"				},
		{	"dei pantaloni",			5,				"pantaloni"				},
		{	"delle calze",				4,				"calze"					},
		{	"un paio di pantaloni",		5,				"pantaloni paio"		},
		{	"degli schinieri",			5,				"schinieri"				},
		{	"delle fasciature",			4,				"fasciature"			},
		{	"delle stecche",			4,				"stecche"				},
		{	"delle protezioni",			4,				"protezioni"			},
		{	"delle calotte",			4,				"calotte"				}
	},
	{	//	arms 9						gender			key
		{	"delle maniche",			4,				"maniche"				},
		{	"dei bracciali",			5,				"bracciali"				},
		{	"dei parabraccia",			5,				"parabraccia"			},
		{	"una cubitiera",			SEX_FEMALE,		"cubitiera"				},
		{	"delle lastre",				4,				"lastre"				},
		{	"un vambrace",				SEX_NEUTRAL,	"vambrace"				},
		{	"una protezione",			SEX_FEMALE,		"protezione"			},
		{	"delle bende",				4,				"bende"					},
		{	"un carapace",				SEX_NEUTRAL,	"carapace"				}
	},
	{   //  shield 14                   gender          key
		{	"uno scudo",                SEX_NEUTRAL,	"scudo"     		},
		{	"uno scutum",               SEX_NEUTRAL,	"scutum"    		},
		{	"un pavese",                SEX_NEUTRAL,	"pavese"    		},
		{	"uno scudo crociato",       SEX_NEUTRAL,	"scudo crociato"	},
		{	"uno scudo rotondo",        SEX_NEUTRAL,	"scudo rotondo" 	},
		{	"uno scudo vichingo",       SEX_NEUTRAL,	"scudo vichingo"	},
		{	"uno scudo puntuto",        SEX_NEUTRAL,	"scudo puntuto" 	},
		{	"un buckler",               SEX_NEUTRAL,	"buckler"   		},
		{	"una lastra",               SEX_FEMALE,		"lastra"    		},
		{	"uno scudo triangolare",    SEX_NEUTRAL,	"scudo triangolare" },
		{	"uno scudo antisommossa",   SEX_NEUTRAL,	"scudo antisommossa"},
		{	"una porta",                SEX_FEMALE,		"porta"     		},
		{	"un kite",                  SEX_NEUTRAL,	"kite"      		},
		{	"uno scudo a rotella",      SEX_NEUTRAL,	"scudo rotella" 	}
	},
	{   //  blunt weapon 15             gender          key
		{	"un martello",              SEX_NEUTRAL,	"martello"  		},
		{	"una mazza",                SEX_FEMALE,		"mazza"     		},
		{	"una stella del mattino",   SEX_FEMALE,		"stella mattino"	},
		{	"una tazza da te'",         SEX_FEMALE,		"tazza te"  		},
		{	"uno scettro",              SEX_NEUTRAL,	"scettro"   		},
		{	"un manganello",            SEX_NEUTRAL,	"manganello"		},
		{	"una clava",                SEX_FEMALE,		"clava"     		},
		{	"una padella",              SEX_FEMALE,		"padella"   		},
		{	"un tonfa",                 SEX_NEUTRAL,	"tonfa"     		},
		{	"un bastone",               SEX_NEUTRAL,	"bastone"   		},
		{	"una mazza ferrata",        SEX_FEMALE,		"mazza ferrata" 	},
		{	"un mazzafrusto",           SEX_NEUTRAL,	"mazzafrusto"   	},
		{	"un nunchaku",              SEX_NEUTRAL,	"nunchaku"  		},
		{	"un flagello",              SEX_NEUTRAL,	"flagello"  		},
		{	"un martello da guerra",    SEX_NEUTRAL,	"martello guerra"   }
	},
	{   //  slash weapon 19             gender          key
		{	"una spada",                SEX_FEMALE,		"spada"     		},
		{	"una scimitarra",           SEX_FEMALE,		"scimitarra"		},
		{	"una falce",                SEX_FEMALE,		"falce"     		},
		{	"un'ascia",                 SEX_FEMALE,		"ascia"     		},
		{	"una spada corta",          SEX_FEMALE,		"spada corta"   	},
		{	"un'azza",                  SEX_FEMALE,		"azza"      		},
		{	"uno spadone",              SEX_NEUTRAL,	"spadone"   		},
		{	"un'alabarda",              SEX_FEMALE,		"alabarda"  		},
		{	"una sciabola",             SEX_FEMALE,		"sciabola"  		},
		{	"una claymore",             SEX_FEMALE,		"claymore"  		},
		{	"una daga",                 SEX_FEMALE,		"daga"      		},
		{	"una spatha",               SEX_FEMALE,		"spatha"    		},
		{	"un falcione",              SEX_NEUTRAL,	"falcione"  		},
		{	"una flamberga",            SEX_FEMALE,		"flamberga" 		},
		{	"un gladio",                SEX_NEUTRAL,	"gladio"    		},
		{	"una katana",               SEX_FEMALE,		"katana"    		},
		{	"un machete",               SEX_NEUTRAL,	"machete"   		},
		{	"un tomahawk",              SEX_NEUTRAL,	"tomahawk"  		},
		{	"un'ascia bipenne",         SEX_FEMALE,		"ascia bipenne" 	}
	},
	{   //  pierce weapon 15            gender          key
		{	"un pugnale",               SEX_NEUTRAL,	"pugnale"   		},
		{	"uno stocco",               SEX_NEUTRAL,	"stocco"    		},
		{	"un fioretto",              SEX_NEUTRAL,	"fioretto"  		},
		{	"un coltello",              SEX_NEUTRAL,	"coltello"  		},
		{	"una lancia",               SEX_FEMALE,		"lancia"    		},
		{	"un paio di forbici",       4,				"forbici paio"  	},
		{	"uno stiletto",             SEX_NEUTRAL,	"stiletto"  		},
		{	"un tridente",              SEX_NEUTRAL,	"tridente"  		},
		{	"un punteruolo",            SEX_NEUTRAL,	"punteruolo"		},
		{	"una picca",                SEX_FEMALE,		"picca"     		},
		{	"un dirk",                  SEX_NEUTRAL,	"dirk"      		},
		{	"un giavellotto",           SEX_NEUTRAL,	"giavellotto"   	},
		{	"una partigiana",           SEX_FEMALE,		"partigiana"		},
		{	"uno spiedo",               SEX_NEUTRAL,	"spiedo"    		},
		{	"un kris",                  SEX_NEUTRAL,	"kris"      		}
	},
	{   //  wand 9                      gender          key
		{	"una bacchetta",            SEX_FEMALE,		"bacchetta" 		},
		{	"un bastoncino",            SEX_NEUTRAL,	"bastoncino"		},
		{	"un rametto",               SEX_NEUTRAL,	"rametto"   		},
		{	"una stecca",               SEX_FEMALE,		"stecca"    		},
		{	"un corno",                 SEX_NEUTRAL,	"corno"     		},
		{	"un vincastro",             SEX_NEUTRAL,	"vincastro" 		},
		{	"un cucchiaio",             SEX_NEUTRAL,	"cucchiaio" 		},
		{	"una spatola",              SEX_FEMALE,		"spatola"   		},
		{	"un rabdo",                 SEX_NEUTRAL,	"rabdo"     		}
	},
	{   //  staff 9                     gender          key
		{	"un bastone",               SEX_NEUTRAL,	"bastone"   		},
		{	"un'asta",                  SEX_FEMALE,		"asta"      		},
		{	"un pastorale",             SEX_NEUTRAL,	"pastorale" 		},
		{	"uno scettro",              SEX_NEUTRAL,	"scettro"   		},
		{	"un caduceo",               SEX_NEUTRAL,	"caduceo"   		},
		{	"una verga",                SEX_FEMALE,		"verga"     		},
		{	"un bacchio",               SEX_NEUTRAL,	"bucchio"   		},
		{	"un alpenstock",            SEX_NEUTRAL,	"alpenstock"		},
		{	"una pertica",              SEX_FEMALE,		"pertica"   		}
	}
};

struct ObjAchieTable RewardObj[3][20] = {
	{
    	//  Casters
		{   //  Finger
			1374,               ITEM_JEWEL,
			APPLY_MANA_REGEN,   APPLY_MANA,         STAT_RANDOM,        ELEM_RESI_RANDOM,   APPLY_HIT,
			100,                75,                 50,                 25,     			10
		},
		{   //  Wrist
			1375,               ITEM_JEWEL,
			APPLY_SPELLFAIL,    APPLY_MANA,         STAT_RANDOM,        APPLY_HIT,          APPLY_MANA_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Ear
			1376,               ITEM_JEWEL,
			APPLY_MANA,         APPLY_SPELLFAIL,    APPLY_MANA_REGEN,   OBJ_TELEPATHY,      STAT_RANDOM,
			100,                75,                 50,                 25,     			10
		},
		{   //  Neck
			1377,               ITEM_JEWEL,
			APPLY_MANA,         APPLY_SPELLFAIL,    STAT_RANDOM,        APPLY_AC,           APPLY_MANA_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Eyes
			1378,               ITEM_JEWEL,
			OBJ_TRUE_SIGHT,     APPLY_MANA,         STAT_RANDOM,        ELEM_RESI_RANDOM,   APPLY_MANA_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Hold
			1379,               ITEM_JEWEL,
			ELEM_RESI_RANDOM,   APPLY_MANA,         APPLY_MANA_REGEN,   APPLY_SPELLFAIL,    OBJ_INVISIBILITY,
			100,                75,                 50,                 25,     			10
		},
		{   //  Light
			1380,               ITEM_LIGHT,
			OBJ_SENCE_LIFE,     APPLY_MANA,         APPLY_MANA_REGEN,   APPLY_SPELLFAIL,    OBJ_LIFE_PROT,
			100,                75,                 50,                 25,     			10
		},
		{   //  Back
			1381,               ITEM_CONTAINER,
			OBJ_ARTIFACT,       APPLY_MANA,         APPLY_MANA_REGEN,   STAT_RANDOM,        APPLY_HIT,
			100,                80,                 50,                 30,     			10
		},
		{   //  About Body
			1382,               ITEM_ARMOR,
			APPLY_SPELLFAIL,    STAT_RANDOM,        APPLY_MANA_REGEN,   APPLY_HIT,          APPLY_MANA,
			100,                80,                 50,                 30,     			10
		},
		{   //  Waist
			1383,               ITEM_ARMOR,
			APPLY_SPELLFAIL,    APPLY_MANA,         APPLY_MANA_REGEN,   STAT_RANDOM,        APPLY_HIT,
			100,                80,                 50,                 30,     			10
		},
		{   //  Feet
			1384,               ITEM_ARMOR,
			APPLY_MOVE,         OBJ_FLY,            STAT_RANDOM,        APPLY_MOVE_REGEN,   OBJ_SNEAK,
			100,                80,                 50,                 30,     			10
		},
		{   //  Hands
			1385,               ITEM_ARMOR,
			APPLY_MANA_REGEN,   APPLY_SPELLFAIL,    APPLY_HITROLL,      APPLY_MANA,         STAT_RANDOM,
			100,                80,                 50,                 30,     			10
		},
		{   //  Body
			1386,               ITEM_ARMOR,
			APPLY_MANA,         APPLY_MANA_REGEN,   OBJ_RESI_SLASH,     APPLY_SPELLFAIL,    APPLY_HIT,
			100,                80,                 60,                 40,                 20
		},
		{   //  Head
			1387,               ITEM_ARMOR,
			APPLY_MANA,         APPLY_AC,           OBJ_SPY,			APPLY_MANA_REGEN,   APPLY_HIT,
			100,                80,                 60,                 40,                 20
		},
		{   //  Legs
			1388,               ITEM_ARMOR,
			APPLY_MANA_REGEN,   APPLY_MANA,         STAT_RANDOM,        APPLY_SPELLFAIL,    APPLY_AC,
			100,                80,                 60,                 40,                 20
		},
		{   //  Arms
			1389,               ITEM_ARMOR,
			APPLY_HIT,          APPLY_MANA,         STAT_RANDOM,        APPLY_MANA_REGEN,   APPLY_SPELLFAIL,
			100,                80,                 60,                 40,                 20
		},
		{   //  Shield
			1390,               ITEM_ARMOR,
			APPLY_HITROLL,      APPLY_MANA,         OBJ_RESI_PIERCE,    APPLY_MANA_REGEN,   APPLY_AC,
			100,                80,                 60,                 40,                 20
		},
		{   //  Wield
			1391,               ITEM_WEAPON,
			APPLY_MANA,         APPLY_HITROLL,      APPLY_DAMROLL,      APPLY_SPELLFAIL,    APPLY_HITROLL,
			100,                70,                 40,                 20,                 5
		},
		{   //  Wand
			1392,               ITEM_WAND,
			APPLY_MANA,         STAT_RANDOM,        APPLY_MANA_REGEN,   APPLY_SPELLFAIL,    APPLY_HIT,
			100,                70,                 40,                 20,                 5
		},
		{   //  Staff
			1393,               ITEM_STAFF,
			APPLY_MANA,         STAT_RANDOM,        APPLY_HIT,          APPLY_MANA_REGEN,   APPLY_SPELLFAIL,
			100,                70,                 40,                 20,                 5
		}
	},
	{
    	//  Multi
		{   //  Finger
			1374,               ITEM_JEWEL,
			APPLY_SPELLFAIL,    APPLY_MANA,         APPLY_HITNDAM,      APPLY_HIT,          APPLY_HIT_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Wrist
			1375,               ITEM_JEWEL,
			APPLY_HIT,          APPLY_SPELLFAIL,    APPLY_MANA,         APPLY_MANA_REGEN,   APPLY_DAMROLL,
			100,                75,                 50,                 25,     			10
		},
		{   //  Ear
			1376,               ITEM_JEWEL,
			APPLY_MANA,         APPLY_HIT_REGEN,    APPLY_HITROLL,      APPLY_HIT,          APPLY_SPELLFAIL,
			100,                75,                 50,                 25,     			10
		},
		{   //  Neck
			1377,               ITEM_JEWEL,
			APPLY_AC,           APPLY_MANA_REGEN,   APPLY_HIT,          APPLY_HITNDAM,      APPLY_MANA,
			100,                75,                 50,                 25,     			10
		},
		{   //  Eyes
			1378,               ITEM_JEWEL,
			APPLY_HIT,          OBJ_SENCE_LIFE,     APPLY_MANA,         OBJ_SPY,			APPLY_MANA_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Hold
			1379,               ITEM_JEWEL,
			APPLY_HIT,          STAT_RANDOM,        ELEM_RESI_RANDOM,   APPLY_DAMROLL,      OBJ_LIFE_PROT,
			100,                75,                 50,                 25,     			10
		},
		{   //  Light
			1380,               ITEM_LIGHT,
			APPLY_HIT_REGEN,    APPLY_HIT,          APPLY_SPELLFAIL,    APPLY_HITNDAM,      STAT_RANDOM,
			100,                75,                 50,                 25,     			10
		},
		{   //  Back
			1381,               ITEM_CONTAINER,
			OBJ_ARTIFACT,       APPLY_SPELLFAIL,    APPLY_MANA,         APPLY_HIT_REGEN,    APPLY_HIT,
			100,                80,                 50,                 30,     			10
		},
		{   //  About Body
			1382,               ITEM_ARMOR,
			APPLY_AC,           STAT_RANDOM,        ELEM_RESI_RANDOM,   APPLY_HIT,          APPLY_HITNDAM,
			100,                80,                 50,                 30,     			10
		},
		{   //  Waist
			1383,               ITEM_ARMOR,
			APPLY_SPELLFAIL,    OBJ_FLY,			APPLY_MANA,         APPLY_MANA_REGEN,   APPLY_HITROLL,
			100,                80,                 50,                 30,     			10
		},
		{   //  Feet
			1384,               ITEM_ARMOR,
			APPLY_MOVE,         APPLY_MOVE_REGEN,   STAT_RANDOM,        APPLY_HITROLL,      APPLY_AC,
			100,                80,                 50,                 30,     			10
		},
		{   //  Hands
			1385,               ITEM_ARMOR,
			APPLY_HITROLL,      APPLY_MANA,         APPLY_HIT,          APPLY_AC,           APPLY_MANA_REGEN,
			100,                80,                 50,                 30,     			10
		},
		{   //  Body
			1386,               ITEM_ARMOR,
			APPLY_HIT,          APPLY_AC,           OBJ_RESI_SLASH,     APPLY_MANA,         APPLY_HITNDAM,
			100,                80,                 60,                 40,                 20
		},
		{   //  Head
			1387,               ITEM_ARMOR,
			APPLY_HIT,          ELEM_RESI_RANDOM,   OBJ_TRUE_SIGHT,     APPLY_MANA,         APPLY_MANA_REGEN,
			100,                80,                 60,                 40,                 20
		},
		{   //  Legs
			1388,               ITEM_ARMOR,
			APPLY_MANA,         APPLY_HIT,          STAT_RANDOM,        APPLY_MANA_REGEN,   APPLY_HITNDAM,
			100,                80,                 60,                 40,                 20
		},
		{   //  Arms
			1389,               ITEM_ARMOR,
			APPLY_HIT_REGEN,    ELEM_RESI_RANDOM,   APPLY_MANA_REGEN,   APPLY_SPELLFAIL,    APPLY_HITNDAM,
			100,                80,                 60,                 40,                 20
		},
		{   //  Shield
			1390,               ITEM_ARMOR,
			APPLY_HIT,          OBJ_RESI_PIERCE,    APPLY_HITNDAM,      APPLY_SPELLFAIL,    APPLY_HIT_REGEN,
			100,                80,                 60,                 40,                 20
		},
		{   //  Wield
			1391,               ITEM_WEAPON,
			APPLY_HITROLL,      APPLY_DAMROLL,      STAT_RANDOM,        APPLY_HITNDAM,      APPLY_HITROLL,
			100,                70,                 40,                 20,                 5
		},
		{   //  Wand
			1392,               ITEM_WAND,
			APPLY_MANA_REGEN,   APPLY_SPELLFAIL,    APPLY_MANA,         STAT_RANDOM,        APPLY_HITROLL,
			100,                70,                 40,                 20,                 5
		},
		{   //  Staff
			1393,               ITEM_STAFF,
			APPLY_MANA,         APPLY_HIT,          STAT_RANDOM,        APPLY_SPELLFAIL,    APPLY_HIT_REGEN,
			100,                70,                 40,                 20,                 5
		}
	},
	{
    	//  Melee
		{   //  Finger
			1374,               ITEM_JEWEL,
			APPLY_HITROLL,      APPLY_HIT,          APPLY_DAMROLL,      STAT_RANDOM,        APPLY_HIT_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Wrist
			1394,               ITEM_ARMOR,
			APPLY_HIT_REGEN,    APPLY_HITROLL,      STAT_RANDOM,        APPLY_HIT,          APPLY_MANA_REGEN,
			100,                75,                 50,                 25,     			10
		},
		{   //  Ear
			1376,               ITEM_JEWEL,
			APPLY_HITROLL,      APPLY_HIT_REGEN,    STAT_RANDOM,        ELEM_RESI_RANDOM,   APPLY_MANA,
			100,                75,                 50,                 25,     			10
		},
		{   //  Neck
			1377,               ITEM_JEWEL,
			APPLY_HIT,          ELEM_RESI_RANDOM,   STAT_RANDOM,        APPLY_HITNDAM,      APPLY_SPELLFAIL,
			100,                75,                 50,                 25,     			10
		},
		{   //  Eyes
			1395,               ITEM_ARMOR,
			OBJ_SENCE_LIFE,     APPLY_HIT,          APPLY_HIT_REGEN,    OBJ_SPY,			APPLY_MANA,
			100,                75,                 50,                 25,     			10
		},
		{   //  Hold
			1379,               ITEM_JEWEL,
			APPLY_HITROLL,      OBJ_TELEPATHY,      APPLY_DAMROLL,      STAT_RANDOM,        APPLY_SPELLFAIL,
			100,                75,                 50,                 25,     			10
		},
		{   //  Light
			1380,               ITEM_LIGHT,
			APPLY_HITROLL,      STAT_RANDOM,        APPLY_DAMROLL,      STAT_RANDOM,        OBJ_LIFE_PROT,
			100,                75,                 50,                 25,     			10
		},
		{   //  Back
			1396,               ITEM_ARMOR,
			APPLY_HITROLL,      APPLY_AC,           APPLY_HIT_REGEN,    APPLY_DAMROLL,      APPLY_HIT,
			100,                80,                 50,                 30,     			10
		},
		{   //  About Body
			1382,               ITEM_ARMOR,
			APPLY_AC,           APPLY_HITROLL,      APPLY_HIT_REGEN,    APPLY_HIT,          OBJ_INVISIBILITY,
			100,                80,                 50,                 30,     			10
		},
		{   //  Waist
			1383,               ITEM_ARMOR,
            ELEM_RESI_RANDOM,   APPLY_HIT,          APPLY_HIT_REGEN,    APPLY_HITNDAM,      APPLY_SPELLFAIL,
			100,                80,                 50,                 30,     			10
		},
		{   //  Feet
			1384,               ITEM_ARMOR,
			APPLY_MOVE,         APPLY_MOVE_REGEN,   OBJ_FLY,            STAT_RANDOM,        APPLY_HITNDAM,
			100,                80,                 50,                 30,     			10
		},
		{   //  Hands
			1385,               ITEM_ARMOR,
			APPLY_HITNDAM,      APPLY_HIT_REGEN,    APPLY_HITROLL,      APPLY_HIT,          APPLY_DAMROLL,
			100,                80,                 50,                 30,     			10
		},
		{   //  Body
			1386,               ITEM_ARMOR,
			APPLY_HIT,          OBJ_RESI_SLASH,     APPLY_AC,           APPLY_HITNDAM,      APPLY_HIT_REGEN,
			100,                80,                 60,                 40,                 20
		},
		{   //  Head
			1387,               ITEM_ARMOR,
			OBJ_TRUE_SIGHT,     APPLY_HIT,          ELEM_RESI_RANDOM,   APPLY_HITROLL,      APPLY_DAMROLL,
			100,                80,                 60,                 40,                 20
		},
		{   //  Legs
			1388,               ITEM_ARMOR,
			APPLY_HITROLL,      APPLY_HIT,          STAT_RANDOM,        APPLY_AC,           APPLY_HITNDAM,
			100,                80,                 60,                 40,                 20
		},
		{   //  Arms
			1389,               ITEM_ARMOR,
			APPLY_HITNDAM,      APPLY_HIT,          STAT_RANDOM,        APPLY_AC,           APPLY_HIT_REGEN,
			100,                80,                 60,                 40,                 20
		},
		{   //  Shield
			1390,               ITEM_ARMOR,
			APPLY_DAMROLL,      OBJ_RESI_PIERCE,    APPLY_AC,           APPLY_HITROLL,      APPLY_HIT_REGEN,
			100,                80,                 60,                 40,                 20
		},
		{   //  Wield
			1391,               ITEM_WEAPON,
			APPLY_HITROLL,      APPLY_DAMROLL,      APPLY_HITROLL,      APPLY_DAMROLL,      APPLY_HITNDAM,
			100,                70,                 40,                 20,                 5
		},
		{   //  Wand
			1392,               ITEM_WAND,
			0,      			0,      			0,      			0,      			0,
			100,                70,                 40,                 20,                 5
		},
		{   //  Staff
			1393,               ITEM_STAFF,
			0,      			0,      			0,      			0,      			0,
			100,                70,                 40,                 20,                 5
		}
	}
};

struct ObjBonusTable AchieBonus[MAX_CLASS][12] = {
	{   //  CLASS_MAGIC_USER
		{    5,      7,     10,     12,     20  },  //  Mana
		{    3,      5,      5,      5,     10  },  //  Mana Regen
		{    5,      5,      7,     10,     12  },  //  Spellfail
		{    5,      5,      5,      6,      8  },  //  Hit
		{    5,      5,      5,      5,      5  },  //  Hit Regen
		{    5,      5,      5,      5,     10  },  //  Move
		{    5,      5,      5,      5,      5  },  //  Move Regen
		{    1,      1,      1,      1,      2  },  //  Hitroll
		{    1,      1,      1,      1,      1  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      7,      9,     10  },  //  Armor
		{    1,      2,      2,      3,      4  }   //  AC
	},
	{   //  CLASS_CLERIC
		{    5,      7,     10,     12,     15  },  //  Mana
		{    2,      4,      5,      5,      8  },  //  Mana Regen
		{    5,      5,      5,      8,     10  },  //  Spellfail
		{    5,      5,      6,      8,     10  },  //  Hit
		{    5,      5,      5,      6,      8  },  //  Hit Regen
		{    5,      5,      5,      5,     10  },  //  Move
		{    5,      5,      5,      5,      5  },  //  Move Regen
		{    1,      1,      1,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      1  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      8,     10,     12  },  //  Armor
		{    3,      4,      4,      5,      6  }   //  AC
	},
	{   //  CLASS_WARRIOR
		{    5,      5,      5,      5,      5  },  //  Mana
		{    1,      1,      1,      1,      2  },  //  Mana Regen
		{    5,      5,      5,      5,      5  },  //  Spellfail
		{    5,      6,      9,     12,     15  },  //  Hit
		{    5,      5,      8,     12,     15  },  //  Hit Regen
		{   10,     10,     10,     15,     20  },  //  Move
		{    5,      5,     10,     12,     15  },  //  Move Regen
		{    1,      1,      2,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      2  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      7,     10,     12,     15  },  //  Armor
		{    6,      7,      8,      9,     10  }   //  AC
	},
	{   //  CLASS_THIEF
		{    5,      5,      5,      5,      5  },  //  Mana
		{    1,      1,      1,      1,      2  },  //  Mana Regen
		{    5,      5,      5,      5,      5  },  //  Spellfail
		{    5,      5,      5,      6,      8  },  //  Hit
		{    5,      5,      5,      6,      8  },  //  Hit Regen
		{   10,     10,     10,     15,     20  },  //  Move
		{    5,      5,      8,     10,     10  },  //  Move Regen
		{    1,      1,      2,      2,      3  },  //  Hitroll
		{    1,      1,      1,      1,      2  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      5,     10,     10  },  //  Armor
		{    3,      3,      3,      4,      5  }   //  AC
	},
	{   //  CLASS_DRUID
		{    5,      7,     10,     12,     15  },  //  Mana
		{    2,      4,      5,      5,      8  },  //  Mana Regen
		{    5,      5,      5,      8,     10  },  //  Spellfail
		{    5,      5,      6,      8,     10  },  //  Hit
		{    5,      5,      5,      6,      8  },  //  Hit Regen
		{    5,      5,      5,      5,     10  },  //  Move
		{    5,      5,      5,      5,      5  },  //  Move Regen
		{    1,      1,      1,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      1  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      7,      9,     10  },  //  Armor
		{    2,      3,      3,      4,      5  }   //  AC
	},
	{   //  CLASS_MONK
		{    5,      5,      5,      5,      5  },  //  Mana
		{    1,      1,      1,      1,      2  },  //  Mana Regen
		{    5,      5,      5,      5,      5  },  //  Spellfail
		{    5,      5,      7,     10,     12  },  //  Hit
		{    5,      5,      7,     10,     12  },  //  Hit Regen
		{   10,     10,     10,     15,     20  },  //  Move
		{    5,      5,     10,     12,     15  },  //  Move Regen
		{    1,      1,      2,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      2  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      8,     10,     12  },  //  Armor
		{    3,      4,      4,      5,      6  }   //  AC
	},
	{   //  CLASS_BARBARIAN
		{    0,      0,      0,      0,      0  },  //  Mana
		{    0,      0,      0,      0,      0  },  //  Mana Regen
		{    0,      0,      0,      0,      0  },  //  Spellfail
		{    5,      6,      9,     12,     15  },  //  Hit
		{    5,      5,      8,     12,     15  },  //  Hit Regen
		{   10,     10,     10,     15,     20  },  //  Move
		{    5,      5,     10,     12,     15  },  //  Move Regen
		{    1,      1,      1,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      2  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      8,     10,     12  },  //  Armor
		{    4,      5,      5,      6,      7  }   //  AC
	},
	{   //  CLASS_SORCERER
		{    5,      7,     10,     12,     20  },  //  Mana
		{    3,      5,      5,      5,     10  },  //  Mana Regen
		{    5,      5,      7,     10,     12  },  //  Spellfail
		{    5,      5,      5,      6,      8  },  //  Hit
		{    5,      5,      5,      5,      5  },  //  Hit Regen
		{    5,      5,      5,      5,     10  },  //  Move
		{    5,      5,      5,      5,      5  },  //  Move Regen
		{    1,      1,      1,      1,      2  },  //  Hitroll
		{    1,      1,      1,      1,      1  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      7,      9,     10  },  //  Armor
		{    1,      2,      2,      3,      4  }   //  AC
	},
	{   //  CLASS_PALADIN
		{    5,      5,      5,      8,     10  },  //  Mana
		{    1,      1,      2,      3,      5  },  //  Mana Regen
		{    5,      5,      5,      5,      5  },  //  Spellfail
		{    5,      5,      7,     10,     12  },  //  Hit
		{    5,      5,      6,      8,     10  },  //  Hit Regen
		{   10,     10,     10,     10,     15  },  //  Move
		{    5,      5,      5,      5,      5  },  //  Move Regen
		{    1,      1,      2,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      2  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      7,     10,     12,     15  },  //  Armor
		{    5,      6,      6,      7,      8  }   //  AC
	},
	{   //  CLASS_RANGER
		{    5,      5,      5,      5,      5  },  //  Mana
		{    1,      1,      2,      3,      5  },  //  Mana Regen
		{    5,      5,      5,      5,      5  },  //  Spellfail
		{    5,      5,      7,     10,     12  },  //  Hit
		{    5,      5,      7,     10,     12  },  //  Hit Regen
		{   10,     10,     10,     10,     15  },  //  Move
		{    5,      5,      5,      5,      5  },  //  Move Regen
		{    1,      1,      2,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      2  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      8,     10,     10  },  //  Armor
		{    4,      5,      5,      6,      7  }   //  AC
	},
	{   //  CLASS_PSI
		{    5,      7,     10,     12,     20  },  //  Mana
		{    3,      5,      5,      5,     10  },  //  Mana Regen
		{    5,      5,      7,     10,     12  },  //  Spellfail
		{    5,      5,      6,      8,     12  },  //  Hit
		{    5,      5,      6,      8,     12  },  //  Hit Regen
		{    5,      5,      5,      5,     10  },  //  Move
		{    5,      5,      8,     10,     10  },  //  Move Regen
		{    1,      1,      1,      2,      2  },  //  Hitroll
		{    1,      1,      1,      1,      1  },  //  Damroll
		{    1,      1,      1,      1,      1  },  //  Hit&Dam
		{    5,      5,      7,      9,     10  },  //  Armor
		{    1,      2,      2,      3,      4  }   //  AC
	}
};

struct QuestRewardsTable QuestNebbie [MAX_QUEST_ACHIE] [100] = {
	//	Xarah
	{
		{	19755	},
		{	19780	},
		{	19779	},
		{	19778	},
		{	19777	},
		{	19756	},
		{	19781	},
		{	19782	},
		{	19783	},
		{	19784	},
		{	19757	},
		{	19785	},
		{	19786	},
		{	19787	},
		{	19788	},
		{	19758	},
		{	19789	},
		{	19790	},
		{	19759	},
		{	19791	},
		{	19792	},
		{	19760	},
		{	19793	},
		{	19794	},
		{	19795	},
		{	19796	},
		{	19761	},
		{	19699	},
		{	19698	},
		{	19697	},
		{	19696	},
		{	19763	},
		{	19797	},
		{	19798	},
		{	19766	},
		{	19695	},
		{	19694	},
		{	19767	},
		{	19693	},
		{	19692	},
		{	19768	},
		{	19691	},
		{	19690	},
		{	   -1	}
	},

	// Nilmys
	{
		{	 9019	},
		{	 9018	},
		{	 9020	},
		{	 9021	},
		{	 9098	},
		{	 9099	},
		{	 9005	},
		{	 9006	},
		{	 9007	},
		{	 9009	},
		{	 9097	},
		{	 9087	},
		{	 9015	},
		{	 9016	},
		{	 9017	},
		{	 9014	},
		{	 9095	},
		{	 9094	},
		{	 9010	},
		{	 9011	},
		{	 9012	},
		{	 9013	},
		{	 9090	},
		{	 9091	},
		{	 9029	},
		{	 9031	},
		{	 9030	},
		{	 9028	},
		{	 9092	},
		{	 9093	},
		{	 9023	},
		{	 9025	},
		{	 9024	},
		{	 9026	},
		{	 9088	},
		{	 9089	},
		{	   -1	}
	}
};

struct MobQuestAchie QuestMobAchie[MAX_QUEST_ACHIE] = {
	//  Xarah
	{   8,  19709,  19719,  19605,  19712,  19710,  19724,  19602,  19933,      0,      0   },

	//  Nilmys
	{   9,   9007,   9020,   9021,   9047,   9015,   9014,   9018,   8916,   8923,      0   }
};

struct MercySystem QuestNumber[MAX_QUEST_ACHIE] = {
	//  Xarah
	{	11,	1,		31,	2,		46,	3,		51,	3,		56,	4,		61,		"Xarah"		},

	//  Nilmys
	{	11,	1,		31,	2,		46,	3,		51,	3,		56,	4,		61,		"Nilmys"	}
};

struct ClassAchieTable AchievementsList[MAX_ACHIE_CLASSES][MAX_ACHIE_TYPE] = {
	{
		//  CLASS_ACHIE
		{
			0,  /* achie_number */									"",/* achie_string1 */						"", // achie_string2
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,  /* classe */                            			0, /*   grado_diff */						0   //n_livelli
		},
		{
			ACHIE_MAGE_1,                               			"portale fatto",                			"portali fatti",
			"Non sono sicuro dove porti...",            			1,
			"Un taxi signore?",									 50,
			"Scarpe? A cosa servono?",                              300,
			"Un piccolo mondo!",                        			1000,
			"Signore dei Portali",                      			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_MAGIC_USER,										LEV_BEGINNER,                               5
		},
		{
			ACHIE_MAGE_2,                               			"raggio disintegrante lanciato",			"raggi disintegranti lanciati",
			"Ops, non volevo",                          			1,
			"Quasi quasi ci prendo gusto",                          50,
			"Sei sicuro di volermi provocare?",                     300,
			"Nemico? Quale nemico?",                    			1000,
			"Disintegratore",                           			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_MAGIC_USER,										LEV_VERY_EASY,                              5
		},
		{
			ACHIE_MAGE_3,                               			"incantesimo di invisibilita' lanciato",    "incantesimi di invisibilita' lanciati",
			"Cosa mi e' successo?!?",                   			1,
			"Guardami adesso!",									 50,
			"Ti giuro che non ci sono!",                            300,
			"Sono dietro di te",                        			1000,
			"Mr. Nessuno",                              			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_MAGIC_USER,										LEV_VERY_EASY,                              5
		},
		{
			ACHIE_CLERIC_1,                             			"incantesimo di guarigione lanciato",       "incantesimi di guarigione lanciati",
			"Mi sembra di ricordare si faccia cosi'",   			1,
			"Ne ho visti tanti messi peggio",                       50,
			"Vai sereno tank!",									 300,
			"Chi trova un chierico trova un tesoro",    			1000,
			"Lasciatemi passare, sono IL medico!",      			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_CLERIC,											 LEV_BEGINNER,                               5
		},
		{
			ACHIE_CLERIC_2,                             			"resurrezione fatta",           			"resurrezioni fatte",
			"Ci posso provare",                         			1,
			"Ci penso io!",											 50,
			"Morte non ti temo",									300,
			"Immortalis",                               			1000,
			"Nemesi di Thanatos",                       			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_CLERIC,											 LEV_HARD,									5
		},
		{
			ACHIE_CLERIC_3,                             			"incantesimo di santuario lanciato",        "incantesimi di santuario lanciati",
			"Cosi' dovrebbe andare meglio",             			1,
			"Se mi lasci non ti accendo",                           50,
			"Inserisco la spina",									300,
			"Ti copro io, tranquillo!",                 			1000,
			"Portatore di Luce",                        			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_CLERIC,											 LEV_BEGINNER,                               5
		},
		{
			ACHIE_WARRIOR_1,                            			"avversario buttato a terra",   			"avversari buttati a terra",
			"Scusa, sono inciampato",                   			1,
			"Attaccabrighe",											50,
			"Bullo",												300,
			"Bulldozer",                                			1000,
			"Demolitore",                               			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_WARRIOR,											LEV_BEGINNER,                               5
		},
		{
			ACHIE_WARRIOR_2,                            			"atterramento fallito",         			"atterramenti falliti",
			"Buccia di banana",                         			1,
			"Zoppo",												50,
			"Ubriacone",											300,
			"Maldestro",                                			1000,
			"Il guerriero non e' il mio mestiere",      			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_WARRIOR,											LEV_BEGINNER,                               5
		},
		{
			ACHIE_WARRIOR_3,                            			"compagno salvato",             			"compagni salvati",
			"Dici a me?",                               			1,
			"Ti guardo le spalle",									50,
			"Impiccione",											300,
			"Salva donzelle",                           			1000,
			"Risolvi problemi",                         			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_WARRIOR,											LEV_EASY,									5
		},
		{
			ACHIE_THIEF_1,                              			"avversario pugnalato alle spalle",         "avversari pugnalati alle spalle",
			"Non sono stato io!",                       			1,
			"Ci prendo gusto",										50,
			"Toh, un puntaspilli",									300,
			"Non darmi mai le spalle",                  			1000,
			"Punitore",                                 			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_THIEF,											LEV_BEGINNER,                               5
		},
		{
			ACHIE_THIEF_2,                              			"avversario ucciso con un colpo",           "avversari uccisi con un colpo",
			"Era gia' morto!",                          			1,
			"Ora lo zittisco",										50,
			"Ne uccide piu' lo stab della spada",                   300,
			"Fratello morte",                           			1000,
			"Assassino",                                			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_THIEF,											LEV_VERY_EASY,                              5
		},
		{
			ACHIE_THIEF_3,                              			"avversario derubato",          			"avversari derubati",
			"Ti e' caduto qualcosa?",                   			1,
			"Povero e' chi il povero fa",                           50,
			"Robin Hood!",											300,
			"Me lo hai dato tu, giuro!",                			1000,
			"Lupin III",                                			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_THIEF,											LEV_NORMAL,                                 5
		},
		{
			ACHIE_DRUID_1,                              			"massa di insetti evocata",     			"masse di insetti evocate",
			"Mi sto per sentire male",                  			1,
			"Ho mangiato pesante oggi, state attenti!",             50,
			"Forse non c'e' bisogno di entrare...",                 300,
			"Sta a vedere!",                            			1000,
			"Morte Strisciante",                        			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_DRUID,											LEV_NORMAL,                                 5
		},
		{
			ACHIE_DRUID_2,                              			"metamorfosi riuscita",         			"metamorfosi riuscite",
			"Cosa... mi... sta... succedendo...",       			1,
			"Mi sto imbufalendo",									50,
			"Ci vorrebbe la forza di un orso!",                     300,
			"Sei sicuro fossi io?",                     			1000,
			"Doppleganger",                             			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_DRUID,											LEV_EASY,									5
		},
		{
			ACHIE_DRUID_3,                              			"reincarnazione fatta",         			"reincarnazioni fatte",
			"Cosa ho combinato? Scusa, non volevo!",    			1,
			"Non vorrei essere nei tuoi panni!",                    50,
			"Conosci la fiaba del principe ranocchio?",             300,
			"Vorresti essere qualcun altro, vero?",     			1000,
			"Maestro Manipolatore",                     			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_DRUID,											LEV_HARD,									5
		},
		{
			ACHIE_MONK_1,                               			"avversario disarmato",         			"avversari disarmati",
			"Non volevo, scusami :-)",                  			1,
			"Ti e' caduta l'arma?",                                 50,
			"Vuoi vedere che tela faccio volare?",                  300,
			"Arma? Quale arma?",                        			1000,
			"Io e te, senza armi, ora, qui!",           			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_MONK,											 LEV_BEGINNER,                               5
		},
		{
			ACHIE_MONK_2,                               			"palmo vibrante",               			"palmi vibranti",
			"Volevo solo stringerti la mano!",          			1,
			"Tirami il dito!",										50,
			"One Inch Punch",										300,
			"Ti spiezzo con un dito",                   			1000,
			"Nirvana",												10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_MONK,											 LEV_HARD,									5
		},
		{
			ACHIE_MONK_3,                               			"nemico accecato",              			"nemici accecati",
			"Ops, non l'ho fatto apposta...",           			1,
			"Ops, l'ho fatto di nuovo...",                          50,
			"Aspetta, hai qualcosa nell'occhio!",                   300,
			"Guarda qui e sorridi",                     			1000,
			"Tu, non puoi, vedermi!",                   			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_MONK,											 LEV_VERY_EASY,                              5
		},
		{
			ACHIE_BARBARIAN_1,                          			"volta in berserk",             			"volte in berserk",
			"Mi sono un po' innervosito",               			1,
			"Ora mi arrabbio sul serio!",                           50,
			"Statemi lontano adesso",                               300,
			"Prova a ripeterlo, se hai il coraggio!",   			1000,
			"Furia Rossa",                              			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_BARBARIAN,											LEV_BEGINNER,                               5
		},
		{
			ACHIE_BARBARIAN_2,                          			"pugnalata alle spalle evitata",			"pugnalate alle spalle evitate",
			"Che cosa e' stato?",                       			1,
			"Andra' meglio la prossima volta",                      50,
			"Ne sei proprio sicuro?",                               300,
			"Cosa pensavi di fare?",                    			1000,
			"Occhi di Beholder",                        			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_BARBARIAN,											LEV_EASY,									5
		},
		{
			ACHIE_BARBARIAN_3,                          			"medicazione effettuata",       			"medicazioni effettuate",
			"Aspetta che ci soffio sopra!",             			1,
			"Ora si che va meglio!",                                50,
			"Dammi un pezzo della tua maglia!",                     300,
			"E' solo un graffio...",                    			1000,
			"Maestro Sciamano",                         			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_BARBARIAN,											LEV_VERY_EASY,                              5
		},
		{
			ACHIE_SORCERER_1,                           			"portale fatto",                			"portali fatti",
			"Non sono sicuro dove porti...",            			1,
			"Un taxi signore?",									 50,
			"Scarpe? A cosa servono?",                              300,
			"Un piccolo mondo!",                        			1000,
			"Signore dei Portali",                      			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_SORCERER,											 LEV_BEGINNER,                               5
		},
		{
			ACHIE_SORCERER_2,                           			"raggio disintegrante lanciato",			"raggi disintegranti lanciati",
			"Ops, non volevo",                          			1,
			"Quasi quasi ci prendo gusto",                          50,
			"Sei sicuro di volermi provocare?",                     300,
			"Nemico? Quale nemico?",                    			1000,
			"Disintegratore",                           			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_SORCERER,											 LEV_VERY_EASY,                              5
		},
		{
			ACHIE_SORCERER_3,                           			"incantesimo di invisibilita' lanciato",    "incantesimi di invisibilita' lanciati",
			"Cosa mi e' successo?!?",                   			1,
			"Guardami adesso!",									 50,
			"Ti giuro che non ci sono!",                            300,
			"Sono dietro di te",                        			1000,
			"Mr. Nessuno",                              			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_SORCERER,											 LEV_VERY_EASY,                              5
		},
		{
			ACHIE_PALADIN_1,                            			"benedizione invocata",         			"benedizioni invocate",
			"Che cos'e' questa luce?",                  			1,
			"Hai visto la luce!",									50,
			"Gli Dei sono dalla mia parte",                         300,
			"Sono il prescelto!",                       			1000,
			"L'Eletto",                                 			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_PALADIN,											LEV_EASY,									5
		},
		{
			ACHIE_PALADIN_2,                            			"grido di guerra lanciato",     			"grida di guerra lanciati",
			"Volevo solo chiamarti...",                 			1,
			"Posso sussurrarti una cosa all'orecchio?",             50,
			"Huj, Huj, Hajra'",									 	300,
			"Forse ho esagerato con la birra",          			1000,
			"Furia di Guerra",                          			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_PALADIN,											LEV_VERY_EASY,                              5
		},
		{
			ACHIE_PALADIN_3,                            			"consacrazione",                			"consacrazioni",
			"Aiuto, le mie mani brillano!",             			1,
			"Ho le mani d'oro io",									50,
			"Serve un massaggio?",									300,
			"Non ti muovere e fatti toccare!",          			1000,
			"Io porto la Luce nel Mondo",               			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_PALADIN,											LEV_EASY,									5
		},
		{
			ACHIE_RANGER_1,                             			"pelle ottenuta scuoiando",     			"pelli ottenute scuoiando",
			"Povera bestia, mi viene da piangere",      			1,
			"Mi sa che posso farci qualcosa di utile",              50,
			"Dammi una lama e ti concio per le feste!",             300,
			"Faccio solo capolavori",                   			1000,
			"Mastro Conciatore",                        			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_RANGER,											 LEV_VERY_EASY,                              5
		},
		{
			ACHIE_RANGER_2,                             			"evocazione di animali",        			"evocazioni di animali",
			"Cosa vuole da me questa creatura?",        			1,
			"Ho un amico che puo' aiutarci",                        50,
			"Il ranger che sussurrava ai cavalli",                  300,
			"Lasciate che gli animali vengano da me",   			1000,
			"Animal Man",                               			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_RANGER,											LEV_NORMAL,									5
		},
		{
			ACHIE_RANGER_3,                             			"razione ottenuta",             			"razioni ottenute",
			"Povera creatura, mi viene da piangere",    			1,
			"Pero', sai che non vengono male cotte?",               50,
			"Facciamo una o due bistecche, siora?",                 300,
			"Ora ti faccio vedere come ricavarne delle bistecche",  1000,
			"Mastro Macellaio",                         			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_RANGER,											LEV_BEGINNER,                               5
		},
		{
			ACHIE_PSI_1,											"porta dimensionale aperta",    			"porte dimensionali aperte",
			"Ricordavo fosse piu' lontano",             			1,
			"Taxi? No grazie!",									 50,
			"Basta pensarlo e sei li'!",                            300,
			"A cosa servono i maghi se hai uno psi?",   			1000,
			"Il Mondo nelle Mie Mani",                  			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_PSI,												LEV_BEGINNER,                               5
		},
		{
			ACHIE_PSI_2,											"cervello spappolato",          			"cervelli spappolati",
			"Dici che gli ho fatto male?",              			1,
			"Stasera cervello alla coque!",                         50,
			"E' un cervello od un uovo?",                           300,
			"Master Blaster",                           			1000,
			"Mind Melter",                              			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_PSI,												LEV_BEGINNER,                               5
		},
		{
			ACHIE_PSI_3,											"scarica adrenalinica",         			"scariche adrenaliniche",
			"Provala anche solo una volta...",          			1,
			"Forse ho qualcosa che puo' piacerti",                  50,
			"Meglio di una benedizione, vero?",                     300,
			"Ho solo roba buona, io!",                  			1000,
			"Signore dei Cartelli Nebbiosi",            			10000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			CLASS_PSI,												LEV_VERY_EASY,                              5
		}
	},

	//  BOSSKILL_ACHIE
	{
		{   //  0
			ACHIE_EVANGELINE,                           			"Signora delle Bambole uccisa", 			"Evangeline uccise",
			"Ah, non era una Barbie?",                  			1,
			"Le bambole mi inquietano",                 			10,
			"Non sono qui a pettinare bambole!",        			100,
			"Mi piacciono solo quelle gonfiabili...",               500,
			"Master of Puppets",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_CHAMPION,                               5
		},
		{   //  1
			ACHIE_SALOMONE,                             			"Re Salomone ucciso",           			"Re Salomone uccisi",
			"Ah, c'era un piano di sopra?",             			1,
			"Interessante questo scettro",              			10,
			"Io pulisco Apo sul serio!",                			100,
			"Al di sopra dei Cavalieri ci sono io!",                500,
			"Distruttore dell'Equilibrio",							5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_HARD,									5
		},
		{   //  2
			ACHIE_THION,											"Dio delle Ombre ucciso",       			"Thion uccisi",
			"Ma non potevamo farci i fatti nostri?",    			1,
			"The Lord of the Earrings",                 			10,
			"Ammazza (che) Dio",                        			100,
			"Lucertole, spettri ed ombre... tutto qui?",            500,
			"Colui che spazza via l'Ombra",                         5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_EXPERT,                                 5
		},
		{   //  3
			ACHIE_WARLOCK,                              			"Warlock ucciso",               			"Warlock uccisi",
			"E questo da dove esce?",                   			1,
			"Piu' lo spezzetti piu' torna",             			10,
			"Scoperchia bare",                          			100,
			"Pacificatore di anime",                                500,
			"Castiga dinastie",									 5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_EXPERT,                                 5
		},
		{   //  4
			ACHIE_STANISLAV,                            			"Spettro di Stanislav ucciso",				"Spettri di Stanislav uccisi",
			"Ma io non volevo pestarlo!",               			1,
			"Sembrava un vecchietto a modo...",         			10,
			"Misericordioso",                           			100,
			"Scaccia spettri",										500,
			"Sovrano di Nilmys",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_NORMAL,                                 5
		},
		{   //  5
			ACHIE_SAURON,                               			"Oscuro Signore ucciso",        			"Sauron uccisi",
			"Ma io non ci volevo venire...",            			1,
			"Attenti al pozzo!",                        			10,
			"AAA anello cercasi",                       			100,
			"Mandami contro anche i tuoi Nazgul!",                  500,
			"Signore di Mordor",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_HARD,									5
		},
		{   //  6
			ACHIE_XAGELON,                              			"Terrore Divino ucciso",        			"Xagelon uccisi",
			"Ma vedi, sembrava cosi' piccolo...",       			1,
			"Agli Dei piace piccolo",                   			10,
			"Botte piccola, vino buono",                			100,
			"Fattela con quelli alla tua altezza",                  500,
			"Signore di Bofgorak",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_CHAMPION,                               5
		},
		{   //  7
			ACHIE_SLAVALOUS,                            			"Slavalous ucciso",             			"Slavalous uccisi",
			"Che diavoletto!",                          			1,
			"Medaglia medaglia medaglia!",              			10,
			"Il suonatore di campane",                  			100,
			"L'esorcista",											500,
			"Signore dei Diavoli",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_EXPERT,                                 5
		},
		{   //  8
			ACHIE_MORPHAIL,                             			"Sacerdote della Morte ucciso", 			"Morphail uccisi",
			"Il Bevitore di Sangue",                    			1,
			"Araldo della Morte",                       			10,
			"L'uomo del Fiume",                         			100,
			"Il Signore del Fiume",                                 500,
			"Distruttore di filatteri",                             5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_NORMAL,                                 5
		},
		{   //  9
			ACHIE_NARIS,											"Demone della Morte ucciso",    			"Naris uccisi",
			"Non era esattamente un Imp",               			1,
			"Il demone incompreso",                     			10,
			"Demone? Vediamo cosa sa fare!",            			100,
			"Un demone al giorno toglie il paladino di torno",      500,
			"Signore dei Nove Gironi",                              5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_EXPERT,                                 5
		},
		{   //  10
			ACHIE_TSUCHIGUMO,                           			"Tsuchigumo ucciso",            			"Tsuchigumo uccisi",
			"Aracnofobia",                              			1,
			"Occhio che morde!",                        			10,
			"Brutto e peloso",                          			100,
			"Il disinfestatore",									500,
			"Altro che Minamoto!",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_CHAMPION,                               5
		},
		{   //  11
			ACHIE_THANATOS,                             			"Thanatos ucciso",              			"Thanatos uccisi",
			"L'eredita' di Shelin",                     			1,
			"Che i morti restino morti",                			10,
			"Eroe delle Nebbie!",                       			100,
			"Cavaliere delle Nebbie",                               500,
			"Signore delle Nebbie",                                 5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_GOD_MODE,                               5
		},
		{   //  12
			ACHIE_STRAHD,                               			"Signore di Ravenloft ucciso",				"Strahd uccisi",
			"Ho qui con me un paletto di frassino",     			1,
			"Ammazza pipistrelli",                      			10,
			"Ammazza non morti",                        			100,
			"Ammazza Vampiri",										500,
			"Van Helsing",											5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_IMPERIAL,                               5
		},
		{   //  13
			ACHIE_SHELOB,                               			"Shelob uccisa",                			"Shelob uccise",
			"Toglietemi le ragnatele da dosso!",        			1,
			"Questo si che e' un ragno!",               			10,
			"Se ci portassi mia moglie qui...",         			100,
			"E' solo un ragnetto!",                                 500,
			"Stermina Aracnidi",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_HARD,									5
		},
		{   //  14
			ACHIE_ROC,  											"Roc ucciso",                   			"Roc uccisi",
			"E' un aereo? E' un uccello? No! E' un Roc!",           1,
			"L'avidita' puo' essere letale",            			10,
			"Prova ad ingoiarmi",                       			100,
			"Sterminatore di grandi Uccelli!",                      500,
			"Signore dei Roc",										5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_NORMAL,                                 5
		},
		{   //  15
			ACHIE_LAZLO,											"Capitano di Nilmys ucciso",    			"Lazlo uccisi",
			"Ahia! Ma questo fa male!",                 			1,
			"Non saro' io di vedetta!",                 			10,
			"Punitore di soldati",                      			100,
			"Celata di stagno",									 500,
			"Capitano della Guardia",                               5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_HARD,									5
		},
		{   //  16
			ACHIE_HYPNOS,                               			"Signore dei Sogni ucciso",     			"Hypnos uccisi",
			"Salve, soffro di insonnia, mi puo' aiutare?",          1,
			"Ho fatto un brutto sogno",                 			10,
			"Se mi vedi chiudere gli occhi scuotimi!",  			100,
			"Anche tu fai sempre tardi la notte?",                  500,
			"? ? ? ? ? ? ? ? ? ? ? ? ? ?",                          5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_HARD,									5
		},
		{   //  17
			ACHIE_DEMI_LICH,                            			"Demi Lich ucciso",             			"Demi Lich uccisi",
			"Fa freddo o sono io?",                     			1,
			"Per Alar, sei sciupatissimo! Mangia un po'!",          10,
			"Era un lick o un lich?",                   			100,
			"Distruttore di non morti",                             500,
			"Signore dei Lich",										5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_NORMAL,                                 5
		},
		{   //  18
			ACHIE_BALROG,                               			"Balrog ucciso",                			"Balrog uccisi",
			"Ma questo mi fa paura!",                   			1,
			"Frustino infuocato eh... mmm",             			10,
			"Tamburi nella notte",                      			100,
			"Quando scavi troppo a fondo...",                       500,
			"Signore di Moria",										5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_HARD,									5
		},
		{   //  19
			ACHIE_FIDEX_GATOR,                          			"Fidex Gator ucciso",           			"Fodex Gator uccisi",
			"Che bello! Dite che morde?",               			1,
			"Aspettate, io ci so fare con i cani",      			10,
			"Dog sitter",                               			100,
			"Che bel cagnolino! Lo vuoi un croccantino?",           500,
			"Addestratore di cani",                                 5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_EXPERT,                                 5
		},
		{   //  20
			ACHIE_DRAGGSYLIA,                           			"Draggsylia uccisa",            			"Draggsylia uccise",
			"Tranquilli, non soffia questa!",           			1,
			"Io vi sostengo e, se riesco, meno qualcosa",           10,
			"Guardate, un drago!",                      			100,
			"Una lucertola allo spiedo?",                           500,
			"Dominatore di Draghi",                                 5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_CHAMPION,                               5
		},
		{   //  21
			ACHIE_ARKHAT,                               			"Arkhat ucciso",                			"Arkhat uccisi",
			"Va giu' in un colpo!",                     			1,
			"Hey, ha provato a mangiarmi!",             			10,
			"Ma che bel circolo sanguinoleto!",         			100,
			"Ti mangio io se non stai attento!",                    500,
			"Signore delle Tenebre",                                5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_TORMENT,                                5
		},
		{	//	22
			ACHIE_GONHAG,											"Gonhag ucciso",							"Gonhag uccisi",
			"Whisky! Vieni qui bello!",								1,
			"Ho dimenticato il guinzaglio, lo avete?",				10,
			"Hey, non mi sbavare cosi'!",							100,
			"A cuccia su, da bravo",								500,
			"Dominatore di Draghi",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_IMPERIAL,								5
		},
		{	//	23
			ACHIE_BARBALBERO,										"Barbalbero ucciso",						"Barbalbero uccisi",
			"Che bel bastoncino!",									1,
			"Con questo facciamo un bel falo'!",					10,
			"Ho sempre odiato la natura!",							100,
			"Ora mi faccio un bell'arco",							500,
			"Signore di Fangorn",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	24
			ACHIE_ORRORE_DETURPANTE,								"Orrore Deturpante ucciso",					"Orrori Deturpanti uccisi",
			"Sono tutto sporco di fango",							1,
			"Ma questo non si fa schifo da solo?",					10,
			"Qui ora ci coltivo dei mirtilli...",					100,
			"Quanto puzza questo...",								500,
			"Signore della Fenice",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	25
			ACHIE_TIMNUS,											"Timnus ucciso",							"Timus uccisi",
			"Ma era un Dio?",										1,
			"Pensavo fosse uno gnomo...",							10,
			"Avevo bisogno di orecchie per la mia collana",			100,
			"Questi elfi si credono di essere sempre i piu' fighi",	500,
			"Signore degli Elfi",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	26
			ACHIE_SHERAZADE,										"Sherazade uccisa",							"Sherazade uccise",
			"Volevo solo portarti a cena",							1,
			"Hey, ma lo sai che hai degli occhi bellissimi!",		10,
			"Avrei fatto di tutto per te!",							100,
			"Prendi il mio cuore... o prendero' io il tuo!",		500,
			"Signore dell'Harem",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EASY,									5
		},
		{	//	27
			ACHIE_DRAGO_DUNA_NERA,									"Drago delle Dune ucciso",					"Draghi delle Dune uccisi",
			"Ah, ma non era un mucchietto di sabbia?",				1,
			"Voglio fare un bel castello di sabbia",				10,
			"Avete delle formine con voi?",							100,
			"Spaleremo tutto con dei bulldozer",					500,
			"Signore delle Dune",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	28
			ACHIE_INHEP,											"Inhep ucciso",								"Inhep uccisi",
			"Ma questo e' proprio grosso!",							1,
			"Non mi fai paura, gigantone",							10,
			"Sono piu' piccolo ma faccio piu' male!",				100,
			"Piu' sono grossi piu' fanno rumore quando cadono",		500,
			"Signore del Monolito",									5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	29
			ACHIE_ARTEFATTO,										"Costrutto Atlantideo ucciso",				"Artefatti uccisi",
			"Ops, ho rotto il giocattolino",						1,
			"Fermi tutti, lo rimonto io",							10,
			"Mi piace un sacco giocare con i lego",					100,
			"Lo smonto e lo rimonto quanto mi pare e piace",		500,
			"Signore dei Costrutti",								5000,
			"",	/* lvl6 */								 			0,	//	lvl6_val
			"",	/* lvl7 */								 			0,	//	lvl7_val
			"",	/* lvl8 */								 			0,	//	lvl8_val
			"",	/* lvl9 */								 			0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		}
	},

	//  QUEST_ACHIE
	{
		{   //  0
			ACHIE_QUEST_XARAH,                          			"Xarah completata",             			"Xarah completate",
			"Conquistatore di Torri",                   			1,
			"Signore dei Protettori",                               30,
			"Maestro dei Sortilegi",                    			150,
			"Re delle Lucertole",                       			1000,
			"Signore delle Ombre",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_CHAMPION,                               5
		},
		{   //  1
			ACHIE_QUEST_NILMYS,                         			"Nilmys completata",            			"Nilmys completate",
			"Oscuro Signore",                           			1,
			"Colui che cammina nella Nebbia",                       30,
			"Distruttore di Circoli",                   			150,
			"Sterminatore dell'Ombra",                  			1000,
			"Portatore di Luce",									5000,
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_IMPERIAL,                               5
		}
	},

	//  OTHER_ACHIE
	{
		{   //  0
			ACHIE_PKILL_WIN,                            			"Pkill effettuato",             			"Pkill effettuati",
			"Ben fatto!",                               			1,
			"Sei un professionista",                                50,
			"Ma allora sei infame",                                 500,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  1
			ACHIE_PKILL_LOSS,                           			"morte in Pkill",               			"morti in Pkill",
			"Ehi, cosa ti ho fatto?",                   			1,
			"Ma perche' sempre a me?",                              50,
			"Calimero!",											500,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  2
			ACHIE_DEATH,											"morte",                        			"morti",
			"E' solo un graffio",                       			10,
			"E' una ferita leggera",                    			100,
			"Barcollo ma non mollo",                    			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  3
			ACHIE_JUNK, 											"oggetto distrutto",            			"oggetti distrutti",
			"Non gettare la carta!",                    			10,
			"Il riciclo e' il futuro",                  			1000,
			"Ti prego, fermati!",                       			100000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  4
			ACHIE_ASTRAL,                               			"incantesimo di cammino astrale",           "incantesimi di cammino astrale",
			"Noi ce ne andiamo",                        			100,
			"Il mondo a portata di stagno",             			1000,
			"Il piano astrale e' il mio salotto",       			100000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  5
			ACHIE_REPAIR,                               			"oggetto riparato",             			"oggetti riparati",
			"Togliamo i graffi",                        			100,
			"Mai pensato ad un'assicurazione?",         			1000,
			"Il fabbro ti ama",                         			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  6
			ACHIE_MINING,                               			"colpo di piccone a segno",     			"colpi di piccone a segno",
			"Toh, un piccone",                          			100,
			"Se brilla e' buono",                       			1000,
			"Andiam, andiam, andiamo a lavorar...",     			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  7
			ACHIE_BAR,  											"lingotto forgiato",            			"lingotti forgiati",
			"Verso la ricchezza",                       			1000,
			"Non e' tutto oro quello che luccica",      			10000,
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               2
		},
		{   //  8
			ACHIE_GOLDENBAR,                            			"lingotto d'oro forgiato",      			"lingotti d'oro forgiati",
			"Verso la ricchezza",									5000,
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                   			1
		},
		{   //  9
			ACHIE_WEAPONSMITH,                          			"arma forgiata",                			"armi forgiate",
			"E' arrivato l'arrotino",                               50,
			"Ammazza che mazza",                        			100,
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               2
		},
		{   //  10
			ACHIE_QUEST_COMPLETE,                       			"quest completata in totale",   			"quest completate in totale",
			"Investigatore",                            			10,
			"Philip Marlowe",                           			100,
			"Sherlock Holmes",                          			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  11
			ACHIE_QUEST_FAILED,                         			"quest fallita in totale",      			"quest fallite in totale",
			"Scusi signor mob, un'informazione!",       			10,
			"Eppure la sapevo...",                      			100,
			"King of Epic Fail",                        			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  12
			ACHIE_QUEST_HUNT_COMPLETE,                  			"quest di caccia completata",   			"quest di caccia completate",
			"Cacciatore in ascesa",                     			10,
			"Boba Fett",                                			100,
			"Cacciatore di Taglie",                     			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  13
			ACHIE_QUEST_HUNT_FAILED,                    			"quest di caccia fallita",      			"quest di caccia fallite",
			"Ma dove diavolo era?",                     			10,
			"Ehm, era troppo grosso per me...",         			100,
			"Forse e' meglio farseli amici",            			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  14
			ACHIE_QUEST_RESCUE_COMPLETE,                			"quest di salvataggio completata",          "quest di salvataggio completate",
			"Boy scout",                                			10,
			"Salva principesse",                        			100,
			"S.W.A.T.",                                 			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  15
			ACHIE_QUEST_RESCUE_FAILED,                  			"quest di salvataggio fallita", 			"quest di salvataggio fallite",
			"Scusami, avevo da fare...",                			10,
			"Ti giuro che stavo arrivando...",          			100,
			"Per chi mi avete preso, per un chierichetto?",         1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  16  non attiva, futura implementazione
			ACHIE_QUEST_RESEARCH_COMPLETE,              			"quest di ricerca completata",				"quest di ricerca completate",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  17  non attiva, futura implementazione
			ACHIE_QUEST_RESEARCH_FAILED,                			"quest di ricerca fallita",     			"quest di ricerca fallite",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  18  non attiva, futura implementazione
			ACHIE_QUEST_DELIVERY_COMPLETE,              			"quest di consegna completata", 			"quest di consegna completate",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  19  non attiva, futura implementazione
			ACHIE_QUEST_DELIVERY_FAILED,                			"quest di consegna fallita",    			"quest di consegna fallite",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  20  non attiva, serve per il conteggio
			ACHIE_QUEST_TOTAL,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  21  non attiva, serve per il conteggio
			ACHIE_QUEST_HUNT_TOTAL,                     			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  22  non attiva, serve per il conteggio
			ACHIE_QUEST_RESCUE_TOTAL,                   			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  23  non attiva, serve per il conteggio
			ACHIE_QUEST_RESEARCH_TOTAL,                 			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		},
		{   //  24  non attiva, serve per il conteggio
			ACHIE_QUEST_DELIVERY_TOTAL,                 			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   // n_livelli
		}
	},

	//  RACESLAYER_ACHIE        classe -1 = achievement non attivo
	{
		{   //  0
            RACE_HALFBREED,                             			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  1
            RACE_HUMAN, 											"umano ucciso",                 			"umani uccisi",
			"Serial Killer",                            			1000,
			"Misantropo",                               			10000,
			"L'Apocalisse sono io!",                                300000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  2
            RACE_ELVEN, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   // 3
            RACE_DWARF, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  4
            RACE_HALFLING,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  5
            RACE_GNOME, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  6
            RACE_REPTILE,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  7
            RACE_SPECIAL,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  8
            RACE_LYCANTH,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  9
            RACE_DRAGON,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  10
            RACE_UNDEAD,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  11
            RACE_ORC,   											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  12
            RACE_INSECT,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  13
            RACE_ARACHNID,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  14
            RACE_DINOSAUR,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  15
            RACE_FISH,  											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  16
            RACE_BIRD,  											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  17
            RACE_GIANT, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  18
            RACE_PREDATOR,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  19
            RACE_PARASITE,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  20
            RACE_SLIME, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  21
            RACE_DEMON, 											"demone ucciso",                			"demoni uccisi",
			"Per un mondo buono!",                      			100,
			"Il Signore della Loggia Bianca",           			1000,
			"Asmodeus e' il mio scendiletto",                       5000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  22
            RACE_SNAKE, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  23
            RACE_HERBIV,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  24
            RACE_TREE,  											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  25
            RACE_VEGGIE,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  26
            RACE_ELEMENT,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  27
            RACE_PLANAR,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  28
            RACE_DEVIL, 											"diavolo ucciso",               			"diavoli uccisi",
			"Odore di Zolfo",                           			100,
			"Diavoli? Perche' ce ne sono ancora?",      			1000,
			"Araldo del Bene",										5000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  29
            RACE_GHOST, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  30
            RACE_GOBLIN,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  31
            RACE_TROLL, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  32
            RACE_VEGMAN,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  33
            RACE_MFLAYER,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  34
            RACE_PRIMATE,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  35
            RACE_ENFAN, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  36
            RACE_DARK_ELF,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  37
            RACE_GOLEM, 											"golem ucciso",                 			"golem uccisi",
			"E' solo un oggetto!",                      			10,
			"Cio' che e' vivo non si muova!",           			1000,
			"Disgregatore",                             			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  38
            RACE_SKEXIE,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  39
            RACE_TROGMAN,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  40
            RACE_PATRYN,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  41
            RACE_LABRAT,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  42
            RACE_SARTAN,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  43
            RACE_TYTAN, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  44
            RACE_SMURF, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  45
            RACE_ROO,   											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  46
            RACE_HORSE, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  47
            RACE_DRAAGDIM,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  48
            RACE_ASTRAL,											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  49
            RACE_GOD,   											"Dio ucciso",                   			"Dei uccisi",
			"Non temo nessuno",                         			10,
			"Ribelliamoci agli Dei!",                   			100,
			"Godslayer",                                			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  50
            RACE_GIANT_HILL,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  51
            RACE_GIANT_FROST,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  52
            RACE_GIANT_FIRE,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  53
            RACE_GIANT_CLOUD,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  54
            RACE_GIANT_STORM,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  55
            RACE_GIANT_STONE,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  56
            RACE_DRAGON_RED,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  57
            RACE_DRAGON_BLACK,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  58
            RACE_DRAGON_GREEN,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  59
            RACE_DRAGON_WHITE,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  60
            RACE_DRAGON_BLUE,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  61
            RACE_DRAGON_SILVER,                         			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  62
            RACE_DRAGON_GOLD,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  63
            RACE_DRAGON_BRONZE,                         			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  64
            RACE_DRAGON_COPPER,                         			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  65
            RACE_DRAGON_BRASS,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  66
            RACE_UNDEAD_VAMPIRE,                        			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  67
            RACE_UNDEAD_LICH,                           			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  68
            RACE_UNDEAD_WIGHT,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  69
            RACE_UNDEAD_GHAST,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  70
            RACE_UNDEAD_SPECTRE,                        			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  71
            RACE_UNDEAD_ZOMBIE,                         			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  72
            RACE_UNDEAD_SKELETON,                       			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  73
            RACE_UNDEAD_GHOUL,                          			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  74
            RACE_HALF_ELVEN,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  75
            RACE_HALF_OGRE,                             			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  76
            RACE_HALF_ORC,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  77
            RACE_HALF_GIANT,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  78
            RACE_LIZARDMAN,                             			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  79
            RACE_DARK_DWARF,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  80
            RACE_DEEP_GNOME,                            			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  81
            RACE_GNOLL, 											"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  82
            RACE_GOLD_ELF,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  83
            RACE_WILD_ELF,                              			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  84
            RACE_SEA_ELF,                               			"",                             			"",
			"", /* lvl1 */								 			0, //   lvl1_val
			"", /* lvl2 */								 			0, //   lvl2_val
			"", /* lvl3 */								 			0, //   lvl3_val
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
            -1,											 			0, /*   grado_diff */           			0   //n_livelli
		},
		{   //  85
            GROUP_INSECTOID,                            			"insettoide ucciso",            			"insettoidi uccisi",
			"Autan",												100,
			"Ti schiaccio come un...",                  			1000,
			"Mister Raid",                              			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  86
            GROUP_ANIMAL,                               			"animale ucciso",               			"animali uccisi",
			"Carnivoro",                                			10,
			"Amo le carneficine",                       			1000,
			"Animal Killer",                            			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  87
            GROUP_VEGGIE,                               			"vegetale ucciso",              			"vegetali uccisi",
			"Solo un'insalatina leggera",               			10,
			"Ma lo sai che la carne fa male?",          			1000,
			"Mr. Vegan",                                			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  88
            GROUP_SPECIALS,                             			"mostro esotico ucciso",        			"mostri esotici uccisi",
			"Speciale? No, ordinario!",                 			10,
			"Specializzato",                            			100,
			"Special One",                              			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  89
            GROUP_DARKRACES,                            			"abitante del sottosuolo ucciso",           "abitanti del sottosuolo uccisi",
			"Io porto la luce!",                        			10,
			"Paint it White",                           			100,
			"Ritorna nell'ombra!",                       			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  90
            GROUP_DRAKES,                               			"drago ucciso",                 			"draghi uccisi",
			"Soffia Soffia!",                           			100,
			"Flagello di Draghi",                       			1000,
			"Un mondo di scaglie",                      			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  91
            GROUP_GIANTS,                               			"gigante ucciso",               			"giganti uccisi",
			"Mira sempre alle ginocchia!",              			100,
			"Attento che cade!",                        			1000,
			"Chiamami Jack",											5000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  92
            GROUP_RACEHALFBREED,                        			"mezzosangue ucciso",           			"mezzosangue uccisi",
			"Scegli da che parte stare!",               			10,
			"Ne' carne ne' pesce",                      			100,
			"Scusa ma sono xenofobo",                   			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  93
            GROUP_PLANAR,                               			"creatura planare uccisa",      			"creature planari uccise",
			"Ognuno a casa sua",                        			10,
			"Killing Planner",                          			100,
			"Tu... non puoi... passare",                			1000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  94
            GROUP_UNDEAD,                               			"non morto ucciso",             			"non morti uccisi",
			"Spaccaossa",                               			10,
			"Tritaossa",                                			1000,
			"Drain nun te temo!",                       			10000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  95
            GROUP_GREENSKIN,                            			"pelleverde ucciso",            			"pelleverde uccisi",
			"Sono brutti e puzzano!",                   			1000,
			"Muoiono in un niente",                                 5000,
			"Flagello dei pelleverde",                              20000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		},
		{   //  96
            GROUP_HUMANOID,                             			"umanoide ucciso",              			"umanoidi uccisi",
			"Se non e' umano non conta",                            500,
			"Ma due hobbit valgono uno o due?",                     2500,
			"Sterminiamoli tutti",                      			15000,
			"", /* lvl4 */								 			0, //   lvl4_val
			"", /* lvl5 */								 			0, //   lvl5_val
			"", /* lvl6 */								 			0, //   lvl6_val
			"", /* lvl7 */								 			0, //   lvl7_val
			"", /* lvl8 */								 			0, //   lvl8_val
			"", /* lvl9 */								 			0, //   lvl9_val
			"", /* lvl10 */											0, //   lvl10_val
			0,														LEV_BEGINNER,                               3
		}
	}
};
} // namespace Alarmud
