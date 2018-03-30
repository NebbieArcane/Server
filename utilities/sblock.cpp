/*
 * sizes : compile it with or without -m32 to check if sizes differs between architecture
 */
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <climits>
using std::cout;
using std::endl;
const char* spells[]= {
	"armor",               /* 1 */
	"teleport",
	"bless",
	"blindness",
	"burning hands",
	"call lightning",
	"charm person",
	"chill touch",
	"clone",
	"colour spray",
	"control weather",     /* 11 */
	"create food",
	"create water",
	"cure blind",
	"cure critic",
	"cure light",
	"curse",
	"detect evil",
	"detect invisibility",
	"detect magic",
	"detect poison",       /* 21 */
	"dispel evil",
	"earthquake",
	"enchant weapon",
	"energy drain",
	"fireball",
	"harm",
	"heal",
	"invisibility",
	"lightning bolt",
	"locate object",      /* 31 */
	"magic missile",
	"poison",
	"protection from evil",
	"remove curse",
	"sanctuary",
	"shocking grasp",
	"sleep",
	"strength",
	"summon",
	"ventriloquate",      /* 41 */
	"word of recall",
	"remove poison",
	"sense life",         /* 44 */

	/* RESERVED SKILLS */
	"sneak",        /* 45 */
	"hide",
	"steal",
	"backstab",
	"pick",
	"kick",         /* 50 */
	"bash",
	"rescue",
	/* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

	"identify",           /* 53 */
	"infravision",
	"cause light",
	"cause critical",
	"flamestrike",
	"dispel good",
	"weakness",
	"dispel magic",
	"knock",
	"know alignment",
	"animate dead",
	"paralyze",
	"remove paralysis",
	"fear",
	"acid blast",  /* 67 */
	"water breath",
	"fly",
	"cone of cold",   /* 70 */
	"meteor swarm",
	"ice storm",
	"shield",
	"monsum one",
	"monsum two",
	"monsum three",
	"monsum four",
	"monsum five",
	"monsum six",
	"monsum seven",  /* 80 */
	"fireshield",
	"charm monster",
	"cure serious",
	"cause serious",
	"refresh",
	"second wind",
	"turn",
	"succor",
	"create light",
	"continual light",        /* 90 */
	"calm",
	"stone skin",
	"conjure elemental",
	"true sight",
	"minor creation",
	"faerie fire",
	"faerie fog",
	"cacaodemon",
	"polymorph self",
	"mana",        /* 100 */
	"astral walk",
	"resurrection",
	"heroes feast",  /* 103 */
	"group fly",
	"breath",
	"web",
	"minor track",
	"major track",
	"golem",
	"find familiar",        /* 110 */
	"changestaff",
	"holy word",
	"unholy word",
	"power word kill",
	"power word blind",
	"chain lightning",
	"scare",
	"aid",
	"command",
	"change form",        /* 120 */
	"feeblemind",
	"shillelagh",
	"goodberry",
	"elemental blade",
	"animal growth",
	"insect growth",
	"creeping death",
	"commune",
	"animal summon one",
	"animal summon two",        /* 130 */
	"animal summon three",
	"fire servant",
	"earth servant",
	"water servant",
	"wind servant",
	"reincarnate",
	"charm vegetable",
	"vegetable growth",
	"tree",
	"animate rock",        /* 140 */
	"tree travel",
	"travelling",
	"animal friendship",
	"invis to animals",
	"slow poison",
	"entangle",
	"snare",
	"gust of wind",
	"barkskin",
	"sunray",        /* 150 */
	"warp weapon",
	"heat stuff",
	"find traps",
	"firestorm",
	"haste",
	"slowness",
	"dust devil",
	"know monster",
	"transport via plant",
	"speak with plants",        /* 160 */
	"silence",
	"sending",
	"teleport without error",
	"portal",
	"dragon ride",
	"mount",
	"" /* SPELL_NO_MESSAGE*/,
	"168 non implementata", // SALVO descr
	"mantra", /*SKILL_MANTRA*/
	"first aid",        /* 170 */
	"sign language",
	"riding",
	"switch opponents",
	"dodge",
	"remove trap",
	"retreat",
	"quivering palm",
	"safe fall",
	"feign death",
	"hunt",        /* 180 */
	"find trap",
	"spring leap",
	"disarm",
	"read magic",
	"evaluate",
	"spy",
	"doorbash",
	"swim",
	"necromancy",
	"vegetable lore",        /* 190 */
	"demonology",
	"animal lore",
	"reptile lore",
	"people lore",
	"giant lore",
	"other lore",        /* 196 */
	"disguise",
	"climb",
	"finger",
	"geyser",        /* 200 SALVO descr */
	"mirror images",
	"tspy",
	"eavesdrop",
	"Parry",
	"miner", // ACIDUS 2003
	"green slime",	/* SALVO descr */
	"berserk",
	"tan",
	"avoid back attack",
	"find food",    /* 210 */
	"find water",   /* 211 */
	"pray",               /* spell Prayer, 212 */
	"memorizing",
	"bellow",
	"darkness",
	"minor invulnerability",
	"major invulnerability",
	"protection from drain",
	"protection from breath",
	"anti magic shell",         /* anti magic shell */
	"doorway",
	"psi portal",
	"psi summon",
	"psi invisibility",
	"canibalize",
	"flame shroud",
	"aura sight",
	"great sight",
	"psionic blast",
	"hypnosis",
	"meditate",
	"scry",
	"adrenalize",
	"brew",
	"ration",
	"warcry",
	"blessing",
	"lay on hands",
	"heroic rescue",
	"dual wield",
	"psi shield",
	"protection from evil group",
	"prismatic spray",
	"incendiary cloud",
	"disintegrate",
	"language common",
	"language elvish",
	"language halfling",
	"language dwarvish",
	"language orcish",
	"language giantish",
	"language ogre",
	"language gnomish",
	"esp",
	"comprehend languages",
	"protection from fire",
	"protection from cold",
	"protection from energy",
	"protection from electricity",
	"enchant armor",
	"messenger",
	"protection fire breath",
	"protection frost breath",
	"protection electric breath",
	"protection acid breath",
	"protection gas breath",
	"wizardeye",
	"mind burn",
	"clairvoyance",
	"psionic danger sense",
	"psionic disintegrate",
	"telekinesis",
	"levitation",
	"cell adjustment",
	"chameleon",
	"psionic strength",
	"mind over body",
	"probability travel",
	"psionic teleport",
	"domination",
	"mind wipe",
	"psychic crush",
	"tower of iron will",
	"mindblank",
	"psychic impersonation",
	"ultra blast",
	"intensify",
	"spot",
	"\n"
};

int search_block(const char* arg, const char** list, bool exact) {
	size_t l=std::max(strlen(arg),1UL);
	for(int i=0;** (list+i) != '\n'; i++) {
		if (exact) {
			if(!strcasecmp(arg, *(list+i))) {
				return(i);
			}
		}
		else {
			if(!strncasecmp(arg, *(list+i),l)) {
				return(i);
			}
		}
	}
	return -1;
}
int old_search_block(const char* argument,int begin,int length,const char** list,int mode) {
	std::string key(argument);
	int rc=search_block(key.substr(begin,length).c_str(),list,mode);
	return rc>=0?rc+1:rc;
}
const char* show(char* key,int n) {
	cout << key << ": Id= " << n << " (";
	if (n>=0) {
		cout << spells[n-1];
	}
	else {
		cout << "Not Found";
	}
	cout << ")" << endl;
}
int main(int argc,char** argv) {
	if (argc > 1) {
		show(argv[1],old_search_block(argv[1],1,std::strlen(argv[1])-2,spells,0));
		show(argv[1],old_search_block(argv[1],1,std::strlen(argv[1])-2,spells,1));
		show(argv[1],search_block(argv[1],spells,0));
		show(argv[1],search_block(argv[1],spells,1));
	}
	return 0;
}
