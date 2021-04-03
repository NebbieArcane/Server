/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: spells2.c,v 1.3 2002/02/13 12:30:59 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <strings.h>
#include <cstdlib>
#include <cstring>
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
#include "spells2.hpp"
#include "act.move.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magic3.hpp"
#include "regen.hpp"
#include "spell_parser.hpp"

namespace Alarmud {
/*AlarMUD*/


#define MAX_MAGE_POLY 48
/* total number of polies choices */

struct PolyType PolyList[MAX_MAGE_POLY] = {
    {   "goblin",        4, 1331    },
    {   "pappagallo",    4, 1332    },  //  pappagallo blu
    {   "pipistrello",   4, 1333    },
    {   "orco",          5, 1334    },
    {   "troglodita",    5, 1335    },
    {   "gnoll",         6, 1336    },
    {   "pappagallo",    6, 1337    },  //  pappagallo nero
    {   "lucertola",     6, 1338    },
    {   "ogre",          8, 1339    },
    {   "pappagallo",    8, 1340    },  //  pappagallo rosso
    {   "lupo",          8, 1341    },
    {   "ragno",         9, 1342    },
    {   "belva",         9, 1343    },
    {   "minotauro",     9, 1344    },
    {   "serpente",     10, 1345    },
    {   "toro",         10, 1346    },
    {   "incubus",      10, 1347    },  // cl lvl 25
    {   "ratto",        11, 1348    },
    {   "albero",       12, 1349    },
    {   "magi",         12, 1350    },  //  ogre magi
    {   "nero",         12, 1351    },  //  drago nero
    {   "colline",      13, 1352    },  //  gigante delle colline
    {   "troll",        14, 1353    },
    {   "coccodrillo",  14, 1354    },
    {   "giovane",      14, 1355    },  //  giovane mindlflayer
    {   "orso",         16, 1356    },
    {   "blu" ,         16, 1357    },  //  drago blu
    {   "enfan",        18, 1358    },  //  enfan blu
    {   "lamia",        18, 1359    },
    {   "mannaro",      19, 1360    },  //  ratto mannaro
    {   "viverna",      20, 1361    },
    {   "mindflayer",   20, 1362    },  // mindlflayer
    {   "tarantola",    20, 1363    },
    {   "roc",          22, 1364    },
    {   "mostrofango",  23, 1365    },
    {   "viola",        23, 1366    },  //  enfan viola
    {   "ghiacci",      24, 1367    },  //  gigante dei ghiacci
    {   "drow",         25, 1368    },
    {   "bianco",       26, 1369    },  //  drago bianco
    {   "maestro",      28, 1370    },  //  maestro mindflyer
    {   "rosso",        30, 1371    },
    {   "canguro",      35, 1372    },
    {   "brontosauro",  35, 1373    },
    {   "mulichort",    40, 1374    },
    {   "beholder",     45, 1375    }  /* number 47 (48) */
};

#define LAST_POLY_MOB 46 /* I think this is the highest level you get a */
/*  new poly, but I ain't sure.... */

#define MAX_DRUID_POLY 17  /* max number of Druid polies */
struct PolyType DruidList[MAX_DRUID_POLY] = {
	{"cane",        12, 1312},
	{"gufo",     12, 1308},
	{"rana",      15, 1304},
	{"topo",     18, 1302},
	{"granchio",       21, 1301},
	{"tigre",      24, 1300},
	{"rospo",   27, 1314},
	{"lucertola",      30, 1315},
	{"orso",     33, 1311},
	{"ragno",     36, 1309},
	{"lupo", 39, 1303},
	{"serpente",      42, 1310},
	{"verme",       45, 1316},
	{"pipistrello",  48, 1305},
	{"gallo",      49, 1306},
	{"scorpione", 50, 1307},
	{"ankheg", 51, 1313}
};

#define LAST_DRUID_MOB 17           /* last level you get a new poly type */

void cast_resurrection(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(!tar_obj) {
			return;
		}
		spell_resurrection(level, ch, 0, tar_obj);
		break;
	case SPELL_TYPE_STAFF:
		if(!tar_obj) {
			return;
		}
		spell_resurrection(level, ch, 0, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious problem in 'resurrection'");
		break;
	}

}


void cast_major_track(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_track(level, ch, tar_ch, NULL);
		break;
	case SPELL_TYPE_POTION:
		spell_track(level, ch, ch, NULL);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_track(level, ch, tar_ch, NULL);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_track(level, ch, tar_ch, NULL);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room) {
			if(tar_ch != ch) {
				spell_track(level, ch, tar_ch, NULL);
			}
		}
		break;
	default:
		mudlog(LOG_SYSERR, "Serious problem in 'track'");
		break;
	}
}

void cast_minor_track(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_track(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_track(level, ch, ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_track(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_track(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room) {
			if(tar_ch != ch) {
				spell_track(level, ch, tar_ch, 0);
			}
		}
		break;
	default:
		mudlog(LOG_SYSERR, "Serious problem in 'track'");
		break;
	}

}

void cast_mana(byte level, struct char_data* ch, const char* arg, int type,
			   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_POTION:
		spell_mana(level, ch, ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_mana(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_mana(level, ch, tar_ch, 0);
			}
		break;
	case SPELL_TYPE_SPELL:
		spell_mana(-1,ch,tar_ch,0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious problem in 'mana'");
		break;
	}

}
void cast_armor(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(affected_by_spell(tar_ch, SPELL_ARMOR)) {
			send_to_char("Non succede nulla.\n\r", ch);
			return;
		}
		if(ch != tar_ch) {
			act("$N viene protett$B da un'$c0011armatura magica$c0007.", FALSE, ch, 0, tar_ch, TO_CHAR);
		}

		spell_armor(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(affected_by_spell(ch, SPELL_ARMOR)) {
			return;
		}
		spell_armor(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		if(affected_by_spell(tar_ch, SPELL_ARMOR)) {
			return;
		}
		spell_armor(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(affected_by_spell(tar_ch, SPELL_ARMOR)) {
			return;
		}
		spell_armor(level,ch,ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in armor!");
		break;
	}
}

void cast_mirror_images(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
			send_to_char("Non succede nulla.\n\r", ch);
			return;
		}
		spell_mirror_images(level,ch,ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
			return;
		}
		spell_mirror_images(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
			return;
		}
		spell_mirror_images(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
			return;
		}
		spell_mirror_images(level,ch,ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mirror_images!");
		break;
	}
}
void cast_stone_skin(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(affected_by_spell(ch, SPELL_STONE_SKIN)) {
			send_to_char("Non succede nulla.\n\r", ch);
			return;
		}
		spell_stone_skin(level,ch,ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(affected_by_spell(ch, SPELL_STONE_SKIN)) {
			return;
		}
		spell_stone_skin(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(affected_by_spell(ch, SPELL_STONE_SKIN)) {
			return;
		}
		spell_stone_skin(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(affected_by_spell(ch, SPELL_STONE_SKIN)) {
			return;
		}
		spell_stone_skin(level,ch,ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in stone_skin!");
		break;
	}
}

void cast_astral_walk(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:

		spell_astral_walk(level, ch, NULL, NULL);
		break;

	default :
		mudlog(LOG_SYSERR, "Serious screw-up in astral walk!");
		break;
	}
}


void cast_teleport(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_teleport(level, ch, tar_ch, 0);
		break;

	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_teleport(level, ch, tar_ch, 0);
		break;

	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_teleport(level, ch, tar_ch, 0);
			}
		break;

	default :
		mudlog(LOG_SYSERR, "Serious screw-up in teleport!");
		break;
	}
}


void cast_bless(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(tar_obj) {
			/* It's an object */
			if(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
				send_to_char("Non succede nulla.\n\r", ch);
				return;
			}
			spell_bless(level,ch,0,tar_obj);

		}
		else {
			/* Then it is a PC | NPC */

			if(affected_by_spell(tar_ch, SPELL_BLESS) ||
					(GET_POS(tar_ch) == POSITION_FIGHTING)) {
				send_to_char("Non succede nulla.\n\r", ch);
				return;
			}
			spell_bless(level,ch,tar_ch,0);
		}
		break;
	case SPELL_TYPE_POTION:
		if(affected_by_spell(ch, SPELL_BLESS) ||
				(GET_POS(ch) == POSITION_FIGHTING)) {
			return;
		}
		spell_bless(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			/* It's an object */
			if(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
				return;
			}
			spell_bless(level,ch,0,tar_obj);

		}
		else {
			/* Then it is a PC | NPC */

			if(!tar_ch) {
				tar_ch = ch;
			}

			if(affected_by_spell(tar_ch, SPELL_BLESS) ||
					(GET_POS(tar_ch) == POSITION_FIGHTING)) {
				return;
			}
			spell_bless(level,ch,tar_ch,0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			/* It's an object */
			if(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
				return;
			}
			spell_bless(level,ch,0,tar_obj);

		}
		else {
			/* Then it is a PC | NPC */

			if(affected_by_spell(tar_ch, SPELL_BLESS) ||
					(GET_POS(tar_ch) == POSITION_FIGHTING)) {
				return;
			}
			spell_bless(level,ch,tar_ch,0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in bless!");
		break;
	}
}

void cast_infravision(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
			send_to_char("Non succede nulla.\n\r", ch);
			return;
		}
		spell_infravision(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(IS_AFFECTED(ch, AFF_INFRAVISION)) {
			return;
		}
		spell_infravision(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		if(IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
			return;
		}
		spell_infravision(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
			return;
		}
		spell_infravision(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch)
				if(!(IS_AFFECTED(tar_ch, AFF_INFRAVISION))) {
					spell_infravision(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in infravision!");
		break;
	}
}

void cast_true_seeing(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
			send_to_char("Non succede nulla.\n\r", ch);
			return;
		}
		spell_true_seeing(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
			return;
		}
		spell_true_seeing(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		if(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
			return;
		}
		spell_true_seeing(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
			return;
		}
		spell_true_seeing(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch)
				if(!(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT))) {
					spell_true_seeing(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in true_seeing!");
		break;
	}
}

void cast_blindness(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(IS_AFFECTED(tar_ch, AFF_BLIND)) {
			send_to_char("Non succede nulla.\n\r", ch);
			return;
		}
		spell_blindness(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(IS_AFFECTED(ch, AFF_BLIND)) {
			return;
		}
		spell_blindness(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		if(IS_AFFECTED(tar_ch, AFF_BLIND)) {
			return;
		}
		spell_blindness(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		if(IS_AFFECTED(tar_ch, AFF_BLIND)) {
			return;
		}
		spell_blindness(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(ch, tar_ch))
				if(!(IS_AFFECTED(tar_ch, AFF_BLIND))) {
					spell_blindness(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in blindness!");
		break;
	}
}

void cast_light(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_light(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_light(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_light(level,ch,ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in light!");
		break;
	}
}

void cast_cont_light(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cont_light(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_cont_light(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_cont_light(level,ch,ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in continual light!");
		break;
	}
}

void cast_calm(byte level, struct char_data* ch, const char* arg, int type,
			   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_calm(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_calm(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_calm(level,ch,ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people;
				tar_ch; tar_ch = tar_ch->next_in_room) {
			spell_calm(level,ch,tar_ch,0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in continual light!");
		break;
	}
}

void cast_web(byte level, struct char_data* ch, const char* arg, int type,
			  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_web(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_web(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_web(level,ch,ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people;
				tar_ch; tar_ch = tar_ch->next_in_room)
			if(!in_group(ch, tar_ch)) {
				spell_web(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in web!");
		break;
	}
}

void cast_clone(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {
	send_to_char("Not *YET* implemented.", ch);
	return;
}


void cast_control_weather(byte level,struct char_data* ch, const char* arg, int type,
						  struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buffer[MAX_STRING_LENGTH];

	switch(type) {
	case SPELL_TYPE_SPELL:

		one_argument(arg,buffer);

		if(str_cmp("migliore", buffer) && str_cmp("peggiore",buffer)) {
			send_to_char("Vuoi che diventi $c0014migliore$c0007 o $c0008peggiore$c0007?\n\r",ch);
			return;
		}
		if(!OUTSIDE(ch)) {
			send_to_char("Devi essere all'aperto.\n\r",ch);
		}

		if(!str_cmp("migliore",buffer)) {
			if(weather_info.sky == SKY_CLOUDLESS) {
				return;
			}
			if(weather_info.sky == SKY_CLOUDY) {
				send_to_outdoor("Le $c0015nuvole$c0007 scompaiono.\n\r");
				weather_info.sky = SKY_CLOUDLESS;
			}
			if(weather_info.sky == SKY_RAINING) {
				if((time_info.month>3)&&(time_info.month < 14)) {
					send_to_outdoor("$c0012Smette di piovere.$c0007\n\r");
				}
				else {
					send_to_outdoor("$c0015Smette di nevicare.$c0007\n\r");
				}
				weather_info.sky=SKY_CLOUDY;
			}
			if(weather_info.sky == SKY_LIGHTNING) {
				if((time_info.month>3)&&(time_info.month<14)) {
					send_to_outdoor("$c0012Non si vedono piu' $c0011fulmini$c0012, ma continua a piovere.$c0007\n\r");
				}
				else {
					send_to_outdoor("La tormenta e' finita, ma $c0015nevica$c0007 ancora.\n\r");
				}
				weather_info.sky=SKY_RAINING;
			}
			return;
		}
		else {
			if(weather_info.sky == SKY_CLOUDLESS) {
				send_to_outdoor("Il $c0014cielo$c0007 si sta $c0015annuvolando$c0007.\n\r");
				weather_info.sky=SKY_CLOUDY;
				return;
			}
			if(weather_info.sky == SKY_CLOUDY) {
				if((time_info.month > 3) && (time_info.month < 14)) {
					send_to_outdoor("$c0012Inizia a piovere.$c0007\n\r");
				}
				else {
					send_to_outdoor("$c0015Inizia a nevicare.$c0007\n\r");
				}
				weather_info.sky=SKY_RAINING;
			}
			if(weather_info.sky == SKY_RAINING) {
				if((time_info.month>3)&&(time_info.month < 14)) {
					send_to_outdoor("$c0012Sei nel mezzo di un temporale tremendo.$c0007\n\r");
				}
				else {
					send_to_outdoor("$c0015Sei nel mezzo di una tormenta.$c0007\n\r");
				}
				weather_info.sky=SKY_LIGHTNING;
			}
			if(weather_info.sky == SKY_LIGHTNING) {
				return;
			}

			return;
		}
		break;

	default :
		mudlog(LOG_SYSERR, "Serious screw-up in control weather!");
		break;
	}
}



void cast_create_food(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		act("$n crea magicamente un fungo.",FALSE, ch, 0, 0, TO_ROOM);
		spell_create_food(level,ch,0,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(tar_ch) {
			return;
		}
		spell_create_food(level,ch,0,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in create food!");
		break;
	}
}

void cast_create_water(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
			send_to_char("Non puo' contenere $c0012acqua$c0007.\n\r", ch);
			return;
		}
		spell_create_water(level,ch,0,tar_obj);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in create water!");
		break;
	}
}




void cast_water_breath(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_water_breath(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_water_breath(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		spell_water_breath(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in water breath");
		break;
	}
}



void cast_flying(byte level, struct char_data* ch, const char* arg, int type,
				 struct char_data* tar_ch, struct obj_data* tar_obj) {


	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_fly(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_fly(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		spell_fly(level,ch,tar_ch,0);
		break;

	default :
		mudlog(LOG_SYSERR, "Serious screw-up in fly");
		break;
	}
}


void cast_fly_group(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_fly_group(level,ch,0,0);
		break;
	case SPELL_TYPE_POTION:
		spell_fly(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in fly");
		break;
	}
}

void cast_heroes_feast(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_heroes_feast(level,ch,0,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in heroes feast");
		break;
	}
}



void cast_cure_blind(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cure_blind(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_cure_blind(level,ch,ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_cure_blind(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cure blind!");
		break;
	}
}



void cast_cure_critic(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cure_critic(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		spell_cure_critic(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_cure_critic(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_cure_critic(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_cure_critic(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cure critic!");
		break;

	}
}



void cast_cure_light(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cure_light(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_cure_light(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_cure_light(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_cure_light(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cure light!");
		break;
	}
}

void cast_cure_serious(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cure_serious(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_cure_serious(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_cure_serious(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_cure_serious(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cure serious!");
		break;
	}
}

void cast_refresh(byte level, struct char_data* ch, const char* arg, int type,
				  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_refresh(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_refresh(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_refresh(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_refresh(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in refresh!");
		break;
	}
}

void cast_second_wind(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_second_wind(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_second_wind(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:

	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_second_wind(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in second_wind!");
		break;
	}
}

void cast_shield(byte level, struct char_data* ch, const char* arg, int type,
				 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_shield(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_shield(level,ch,ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_shield(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_shield(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in shield!");
		break;
	}

}


void cast_curse(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(tar_obj) {  /* It is an object */
			spell_curse(level,ch,0,tar_obj);
		}
		else {              /* Then it is a PC | NPC */
			spell_curse(level,ch,tar_ch,0);
		}
		break;
	case SPELL_TYPE_POTION:
		spell_curse(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {  /* It is an object */
			spell_curse(level,ch,0,tar_obj);
		}
		else {              /* Then it is a PC | NPC */
			if(!tar_ch) {
				tar_ch = ch;
			}
			spell_curse(level,ch,tar_ch,0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {  /* It is an object */
			spell_curse(level,ch,0,tar_obj);
		}
		else {              /* Then it is a PC | NPC */
			if(!tar_ch) {
				tar_ch = ch;
			}
			spell_curse(level,ch,tar_ch,0);
		}
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_curse(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR,"Serious screw up in curse! Char = %s.",
			   ch->player.name);
		break;
	}
}


void cast_detect_evil(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(affected_by_spell(tar_ch, SPELL_DETECT_EVIL)) {
			send_to_char("Non succede nulla.\n\r", tar_ch);
			return;
		}
		spell_detect_evil(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(affected_by_spell(ch, SPELL_DETECT_EVIL)) {
			return;
		}
		spell_detect_evil(level,ch,ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch)
				if(!(IS_AFFECTED(tar_ch, AFF_DETECT_EVIL))) {
					spell_detect_evil(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in detect evil!");
		break;
	}
}



void cast_detect_invisibility(byte level, struct char_data* ch, const char* arg, int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
			send_to_char("Non succede nulla.\n\r", tar_ch);
			return;
		}
		spell_detect_invisibility(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)) {
			return;
		}
		spell_detect_invisibility(level,ch,ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(in_group(ch,tar_ch))
				if(!(IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE))) {
					spell_detect_invisibility(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in detect invisibility!");
		break;
	}
}



void cast_detect_magic(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(affected_by_spell(tar_ch, SPELL_DETECT_MAGIC)) {
			send_to_char("Non succede nulla.\n\r", tar_ch);
			return;
		}
		spell_detect_magic(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		if(affected_by_spell(ch, SPELL_DETECT_MAGIC)) {
			return;
		}
		spell_detect_magic(level,ch,ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch)
				if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC))) {
					spell_detect_magic(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in detect magic!");
		break;
	}
}



void cast_detect_poison(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_detect_poison(level, ch, tar_ch,tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_detect_poison(level, ch, ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			spell_detect_poison(level, ch, 0, tar_obj);
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_detect_poison(level, ch, tar_ch, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in detect poison!");
		break;
	}
}



void cast_dispel_evil(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_dispel_evil(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_dispel_evil(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_dispel_evil(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_dispel_evil(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_dispel_evil(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in dispel evil!");
		break;
	}
}

void cast_dispel_good(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_dispel_good(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_dispel_good(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_dispel_good(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_dispel_good(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_dispel_good(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in dispel good!");
		break;
	}
}

void cast_faerie_fire(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_faerie_fire(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_faerie_fire(level,ch,ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_faerie_fire(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_faerie_fire(level, ch, tar_ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_faerie_fire(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in dispel good!");
		break;
	}
}



void cast_enchant_weapon(byte level, struct char_data* ch, const char* arg, int type,
						 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_enchant_weapon(level, ch, 0,tar_obj);
		break;

	case SPELL_TYPE_SCROLL:
		if(!tar_obj) {
			return;
		}
		spell_enchant_weapon(level, ch, 0,tar_obj);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in enchant weapon!");
		break;
	}
}



void cast_heal(byte level, struct char_data* ch, const char* arg, int type,
			   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_heal(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_heal(level, ch, ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_heal(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in heal!");
		break;
	}
}


void cast_invisibility(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		if(tar_obj) {
			if(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
				send_to_char("Non succede nulla.\n\r", ch);
			}
			else {
				spell_invisibility(level, ch, 0, tar_obj);
			}
		}
		else {   /* tar_ch */
			if(IS_AFFECTED(tar_ch, AFF_INVISIBLE)) {
				send_to_char("Non succede nulla.\n\r", ch);
			}
			else {
				spell_invisibility(level, ch, tar_ch, 0);
			}
		}
		break;
	case SPELL_TYPE_POTION:
		if(!IS_AFFECTED(ch, AFF_INVISIBLE)) {
			spell_invisibility(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			if(!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))) {
				spell_invisibility(level, ch, 0, tar_obj);
			}
		}
		else {   /* tar_ch */
			if(!tar_ch) {
				tar_ch = ch;
			}

			if(!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
				spell_invisibility(level, ch, tar_ch, 0);
			}
		}
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			if(!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))) {
				spell_invisibility(level, ch, 0, tar_obj);
			}
		}
		else {   /* tar_ch */
			if(!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
				spell_invisibility(level, ch, tar_ch, 0);
			}
		}
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch)
				if(!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
					spell_invisibility(level,ch,tar_ch,0);
				}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in invisibility!");
		break;
	}
}




void cast_locate_object(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_locate_object(level, ch, NULL, tar_obj);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in locate object!");
		break;
	}
}


void cast_poison(byte level, struct char_data* ch, const char* arg, int type,struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
		spell_poison(level, ch, tar_ch, tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_poison(level, ch, ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room) {
			if(tar_ch != ch) {
				spell_poison(level,ch,tar_ch,0);
			}
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in poison!");
		break;
	}
}


void cast_protection_from_evil(byte level, struct char_data* ch, const char* arg, int type,
							   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_protection_from_evil(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_protection_from_evil(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_protection_from_evil(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_protection_from_evil(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in protection from evil!");
		break;
	}
}

void cast_protection_from_evil_group(byte level, struct char_data* ch, const char* arg, int type,
									 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_protection_from_evil_group(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_protection_from_evil_group(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_protection_from_evil_group(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_protection_from_evil_group(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in protection from evil!");
		break;
	}
}


void cast_remove_curse(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_remove_curse(level, ch, tar_ch, tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_remove_curse(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			spell_remove_curse(level, ch, 0, tar_obj);
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_remove_curse(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_remove_curse(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in remove curse!");
		break;
	}
}



void cast_remove_poison(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_remove_poison(level, ch, tar_ch, tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_remove_poison(level, ch, ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_remove_poison(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in remove poison!");
		break;
	}
}


void cast_remove_paralysis(byte level, struct char_data* ch, const char* arg, int type,  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_remove_paralysis(level, ch, tar_ch, tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_remove_paralysis(level, ch, ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_remove_paralysis(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_remove_paralysis(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in remove paralysis!");
		break;
	}
}




void cast_sanctuary(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_sanctuary(level, ch, tar_ch, 0);
		break;

	case SPELL_TYPE_WAND:
	case SPELL_TYPE_POTION:
		spell_sanctuary(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_sanctuary(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_sanctuary(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in sanctuary!");
		break;
	}
}

void cast_fireshield(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_fireshield(level, ch, tar_ch, 0);
		break;

	case SPELL_TYPE_WAND:
	case SPELL_TYPE_POTION:
		spell_fireshield(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_fireshield(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_fireshield(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in fireshield!");
		break;
	}
}


void cast_sleep(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_sleep(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_sleep(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_sleep(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_sleep(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people;
				tar_ch; tar_ch = tar_ch->next_in_room) {
			if(tar_ch != ch) {
				spell_sleep(level, ch, tar_ch, 0);
			}
		}

		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in sleep!");
		break;
	}
}


void cast_strength(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
		spell_strength(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_strength(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_strength(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_strength(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in strength!");
		break;
	}
}


void cast_ventriloquate(byte level, struct char_data* ch, const char* arg,
						int type, struct char_data* tar_ch,
						struct obj_data* tar_obj) {
	struct char_data* tmp_ch;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char buf4[MAX_STRING_LENGTH];

	if(type != SPELL_TYPE_SPELL) {
		mudlog(LOG_SYSERR, "Attempt to ventriloquate by non-cast-spell.");
		return;
	}

	for(; *arg && (*arg == ' '); arg++);

	if(tar_obj) {
		sprintf(buf1, "$c0015[$c0005$p$c0015] dice '%s'", arg);
		sprintf(buf2, "Qualcuno cerca di imitare $p che dice '%s'", arg);
		sprintf(buf4, "Provi a far dire '%s' a $p.", arg);
	}
	else {
		sprintf(buf1, "$c0015[$c0005$N$c0015] dice '%s'", arg);
		sprintf(buf2, "Qualcuno cerca di imitare $N che dice '%s'", arg);
		sprintf(buf4, "Provi a far dire '%s' a $N.", arg);
	}

	sprintf(buf3, "$c0015[$c0005qualcuno$c0015] dice '%s'", arg);

	act(buf4, FALSE, ch, tar_obj, tar_ch, TO_CHAR);

	for(tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room) {

		if(tmp_ch != ch && tmp_ch != tar_ch) {
			if(IS_IMMORTAL(ch)) { // SALVO agli immortali entra sempre
				act(buf1, FALSE, tmp_ch, tar_obj, tar_ch, TO_CHAR);
			}
			else if(saves_spell(tmp_ch, SAVING_SPELL)) {
				act(buf2, FALSE, tmp_ch, tar_obj, tar_ch, TO_CHAR);
			}
			else {
				act(buf1, FALSE, tmp_ch, tar_obj, tar_ch, TO_CHAR);
			}
		}
		else {
			if(tmp_ch == tar_ch) {
				act(buf3, FALSE, tar_ch, NULL, NULL, TO_CHAR);
			}
		}
	}
}

void cast_word_of_recall(byte level, struct char_data* ch, const char* arg,int type,
						 struct char_data* tar_ch, struct obj_data* tar_obj) {
	struct char_data* tmp_ch;
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_word_of_recall(level, ch, ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_word_of_recall(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		tar_ch = ch;
		spell_word_of_recall(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_word_of_recall(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ; tar_ch ; tar_ch = tmp_ch) {
			tmp_ch = tar_ch->next_in_room;
			if(tar_ch != ch) {
				spell_word_of_recall(level,ch,tar_ch,0);
			}
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in word of recall!");
		break;
	}
}



void cast_summon(byte level, struct char_data* ch, const char* arg, int type,
				 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {

	case SPELL_TYPE_SPELL:
		spell_summon(level, ch, tar_ch, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in summon!");
		break;
	}
}



void cast_charm_person(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_charm_person(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) {
			return;
		}
		spell_charm_person(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_charm_person(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in charm person!");
		break;
	}
}

void cast_charm_monster(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_charm_monster(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) {
			return;
		}
		spell_charm_monster(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_charm_monster(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in charm monster!");
		break;
	}
}



void cast_sense_life(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_sense_life(level, ch, ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_sense_life(level, ch, ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_sense_life(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in sense life!");
		break;
	}
}


void cast_identify(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_identify(level, ch, tar_ch, tar_obj);
		/*if (!IS_IMMORTAL(ch)) {
		    send_to_char("Ti gira la testa e cadi a terra senza conoscenza!\n\r",ch);
		    WAIT_STATE(ch, PULSE_VIOLENCE*6);
		    GET_POS(ch) = POSITION_STUNNED;
		 } ACIDUS 2003 - eliminato lo stun da ident */
		break;
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SCROLL:
		spell_identify(level, ch, tar_ch, tar_obj);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in identify!");
		break;
	}
}

#define MAX_BREATHS 3
struct pbreath {
	int        vnum, spell[MAX_BREATHS];
};
struct pbreath breath_potions[] = {
	{ 3970, {SPELL_FIRE_BREATH, 0} },
	{ 3971, {SPELL_GAS_BREATH, 0} },
	{ 3972, {SPELL_FROST_BREATH, 0} },
	{ 3973, {SPELL_ACID_BREATH, 0} },
	{ 3974, {SPELL_LIGHTNING_BREATH, 0} },
	{ 0,{0,0}},
};

void cast_dragon_breath(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* potion) {
	struct pbreath*        scan;
	int        i;
	struct affected_type af;

	if(!potion) {
		return;
	}

	for(scan = breath_potions;
			scan->vnum && (potion->item_number < 0 ||
						   scan->vnum != obj_index[potion->item_number].iVNum);
			scan++)
		;
	if(scan->vnum==0) {
		send_to_char("Hey, this potion isn't in my list!\n\r", ch);
		mudlog(LOG_SYSERR, "unlisted breath potion %s %d",potion->short_description,
			   (potion->item_number >= 0 ?obj_index[potion->item_number].iVNum : 0));
		return;
	}

	for(i=0; i<MAX_BREATHS && scan->spell[i]; i++) {
		if(!affected_by_spell(ch, scan->spell[i])) {
			af.type = scan->spell[i];
			af.duration = 1+dice(1,2);
			if(GET_CON(ch) < 4) {
				send_to_char("Hai lo stomaco troppo debole per sopportare questa pozione.\n\r", ch);
				act("$n sputacchia la pozione per terra.",
					FALSE, ch, 0,ch, TO_NOTVICT);
				break;
			}
			if(level > MIN(GET_CON(ch)-1, GetMaxLevel(ch))) {
				send_to_char("!GACK! Sei troppo debole per sopportare tutta la potenza della pozione.\n\r", ch);
				act("$n emette uno strano verso e cade per un attimo a terra.",
					FALSE, ch, 0,ch, TO_NOTVICT);
				level = MIN(GET_CON(ch)-1, GetMaxLevel(ch));
			}
			af.modifier = -level;
			af.location = APPLY_CON;
			af.bitvector = 0;
			affect_to_char(ch, &af);
			send_to_char("Senti il tuo stomaco scombussolato da delle potenti forze...\n\r", ch);
		}
	}
}

void cast_fire_breath(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_fire_breath(level, ch, tar_ch, 0);
		break;   /* It's a spell.. But people can'c cast it! */
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in firebreath!");
		break;
	}
}

void cast_frost_breath(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_frost_breath(level, ch, tar_ch, 0);
		break;   /* It's a spell.. But people can'c cast it! */
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in frostbreath!");
		break;
	}
}

void cast_acid_breath(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_acid_breath(level, ch, tar_ch, 0);
		break;   /* It's a spell.. But people can'c cast it! */
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in acidbreath!");
		break;
	}
}

void cast_gas_breath(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_gas_breath(level,ch,tar_ch,0);
		break;
	/* THIS ONE HURTS!! */
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in gasbreath!");
		break;
	}
}

void cast_lightning_breath(byte level, struct char_data* ch, const char* arg, int type,
						   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_lightning_breath(level, ch, tar_ch, 0);
		break;   /* It's a spell.. But people can'c cast it! */
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in lightningbreath!");
		break;
	}
}



void cast_knock(byte level, struct char_data* ch, const char* arg, int type,
				struct char_data* tar_ch, struct obj_data* tar_obj) {
	int door;
	char dir[MAX_INPUT_LENGTH];
	char otype[MAX_INPUT_LENGTH];
	struct obj_data* obj;
	struct char_data* victim;

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND: {

		argument_interpreter(arg, otype, dir);

		if(!otype[0]) {
			send_to_char("Cosa vuoi aprire?\n\r",ch);
			return;
		}

		if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
			send_to_char("Mi spiace... questo incantesimo funziona solo sulle porte.\n", ch);
			return;
		}
		else if((door = find_door(ch, otype, dir)) >= 0) {
			if(!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
				send_to_char("E' assurdo!\n\r", ch);
			}
			else if(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
				send_to_char("Ti accorgi che quella porta e' gia' aperta.\n\r", ch);
			}
			else if(EXIT(ch, door)->key < 0) {
				send_to_char("Non trovi nessuna serratura da forzare.\n\r", ch);
			}
			else if(!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
				send_to_char("Oh... non era chiusa a chiave dopotutto.\n\r", ch);
			}
			else if(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) {
				send_to_char("Non riuscirai ad aprirla...\n\r", ch);
			}
			else {
				if((EXIT(ch, door)->keyword) &&
						strcmp("secret", fname(EXIT(ch, door)->keyword)))
					act("$n apre magicamente la serratura di $F.", 0, ch, 0,
						EXIT(ch, door)->keyword, TO_ROOM);
				else {
					act("$n apre magicamente la serratura.", TRUE, ch, 0, 0, TO_ROOM);
				}
				send_to_char("La serratura cede rapidamente al tuo incantesimo.\n\r", ch);
				raw_unlock_door(ch, EXIT(ch, door), door);

			}
		}
	}
	break;
	default:
		mudlog(LOG_SYSERR, "serious error in Knock.");
		break;
	}
}


void cast_know_alignment(byte level, struct char_data* ch, const char* arg,
						 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_know_alignment(level, ch, tar_ch,tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_know_alignment(level, ch, ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_know_alignment(level, ch, tar_ch, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in know alignment!");
		break;
	}
}

void cast_weakness(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
		spell_weakness(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_weakness(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_weakness(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch, ch)) {
				spell_weakness(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in weakness!");
		break;
	}
}

void cast_dispel_magic(byte level, struct char_data* ch, const char* arg,
					   int type,struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
		spell_dispel_magic(level, ch, tar_ch, tar_obj);
		break;
	case SPELL_TYPE_POTION:
		spell_dispel_magic(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			spell_dispel_magic(level, ch, 0, tar_obj);
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_dispel_magic(level, ch, tar_ch, 0);
		break;

	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_dispel_magic(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in dispel magic");
		break;
	}
}


void cast_animate_dead(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {

	struct obj_data* i;

	if(NoSummon(ch)) {
		return;
	}

	switch(type) {

	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			if(IS_CORPSE(tar_obj)) {
				spell_animate_dead(level, ch, 0, tar_obj);
			}
			else {
				send_to_char("Non e' un cadavere!\n\r",ch);
				return;
			}
		}
		else {
			send_to_char("Non e' un cadavere!\n\r",ch);
			return;
		}
		break;
	case SPELL_TYPE_POTION:
		send_to_char("Il tuo corpo si rivolta contro il $c0012liquido magico$c0007.\n\r",ch);
		GET_HIT(ch) = 0;
		alter_hit(ch,0);
		break;
	case SPELL_TYPE_STAFF:
		for(i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
			if(GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
				spell_animate_dead(level,ch,0,i);
			}
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in animate_dead!");
		break;
	}
}


void cast_succor(byte level, struct char_data* ch, const char* arg, int type,
				 struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_succor(level, ch, 0, 0);
	}
}

void cast_paralyze(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_paralyze(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_paralyze(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_paralyze(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_paralyze(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(tar_ch != ch) {
				spell_paralyze(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in paralyze");
		break;
	}
}

void cast_fear(byte level, struct char_data* ch, const char* arg, int type,
			   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_fear(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_fear(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_fear(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_fear(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_fear(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in fear");
		break;
	}
}

void cast_turn(byte level, struct char_data* ch, const char* arg, int type,
			   struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_turn(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_turn(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_turn(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_turn(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in turn");
		break;
	}
}

void cast_faerie_fog(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		spell_faerie_fog(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in faerie fog!");
		break;
	}
}


void cast_poly_self(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buffer[40];
	int mobn = -1, X=LAST_POLY_MOB, found=FALSE;
	struct char_data* mob;
    struct affected_type af;

	one_argument(arg,buffer);

	if(IS_NPC(ch)) {
		send_to_char("Naaaaaa... non lo vuoi realmente.\n\r",ch);
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL: {
		while(!found) {
			if(PolyList[X].level > level) {
				X--;
			}
			else {
				if(!str_cmp(PolyList[X].name, buffer)) {
					mobn = PolyList[X].number;
					found = TRUE;
				}
				else {
					X--;
				}
				if(X < 0) {
					break;
				}
			}
		}

		if(!found || mobn < 0) {
			send_to_char("Non trovo nulla di simile.\n\r", ch);
			return;
		}
		else
        {
            af.type      = SPELL_POLY_SELF;
            af.duration  = (GetMaxLevel(ch) - PolyList[X].level) *  2 + 2;
            af.modifier  = 0;
            af.location  = APPLY_NONE;
            af.bitvector = 0;

            affect_to_char(ch, &af);

			mob = read_mobile(mobn, VIRTUAL);
			if(mob) {
				spell_poly_self(level, ch, mob, 0);
			}
			else {
				send_to_char("Non riesci ad evocare l'immagine di quella creatura.\n\r", ch);
			}
			return;
		}

	}
	break;

	default: {
		mudlog(LOG_SYSERR, "Problem in poly_self");
	}
	break;
	}

}


#define LONG_SWORD   3022
#define SHIELD       3042
#define RAFT         3060
#define BAG          3032
#define WATER_BARREL 6013
#define BREAD        3010

void cast_minor_creation(byte level, struct char_data* ch, const char* arg, int type,
						 struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buffer[40];
	int obj;
	struct obj_data* o;

	one_argument(arg,buffer);

	if(!str_cmp(buffer, "spada")) {
		obj = LONG_SWORD;
	}
	else if(!str_cmp(buffer, "scudo")) {
		obj=SHIELD;
	}
	else if(!str_cmp(buffer, "zattera")) {
		obj=RAFT;
	}
	else if(!str_cmp(buffer, "borsa")) {
		obj=BAG;
	}
	else if(!str_cmp(buffer, "barile")) {
		obj=WATER_BARREL;
	}
	else if(!str_cmp(buffer, "pagnotta")) {
		obj=BREAD;
	}
	else {
		send_to_char("Non c'e' nulla di simile.\n\r", ch);
		return;
	}

	o = read_object(obj, VIRTUAL);
	if(!o) {
		send_to_char("Non c'e' nulla di simile.\n\r", ch);
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_minor_create(level, ch, 0, o);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in minor_create.");
		break;
	}

}

#define FIRE_ELEMENTAL  10
#define WATER_ELEMENTAL 11
#define AIR_ELEMENTAL   12
#define EARTH_ELEMENTAL 13

#define RED_STONE       5233
#define PALE_BLUE_STONE 5230
#define GREY_STONE      5239
#define CLEAR_STONE     5243

void cast_conjure_elemental(byte level, struct char_data* ch, const char* arg, int type,
							struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buffer[40];
	int mob, obj;
	struct obj_data* sac;
	struct char_data* el;


	one_argument(arg,buffer);

	if(NoSummon(ch)) {
		return;
	}

	if(!str_cmp(buffer, "fuoco")) {
		mob = FIRE_ELEMENTAL;
		obj = RED_STONE;
	}
	else if(!str_cmp(buffer, "acqua")) {
		mob = WATER_ELEMENTAL;
		obj = PALE_BLUE_STONE;
	}
	else if(!str_cmp(buffer, "aria")) {
		mob = AIR_ELEMENTAL;
		obj = CLEAR_STONE;
	}
	else if(!str_cmp(buffer, "terra")) {
		mob = EARTH_ELEMENTAL;
		obj = GREY_STONE;
	}
	else {
		send_to_char("Non ci sono elementali di quel tipo disponibili.\n\r", ch);
		return;
	}
	if(!ch->equipment[HOLD]) {
		send_to_char("Devi avere in mano la pietra giusta!\n\r", ch);
		return;
	}

	sac = unequip_char(ch, HOLD);
	if(sac) {
		obj_to_char(sac, ch);
		if(ObjVnum(sac) != obj) {
			send_to_char("Fallisci, non hai la pietra giusta!\n\r", ch);
			return;
		}
		el = read_mobile(mob, VIRTUAL);
		if(!el) {
			send_to_char("Non ci sono elementali di quel tipo disponibili\n\r", ch);
			return;
		}
	}
	else {
		send_to_char("Devi avere in mano la pietra giusta!\n\r", ch);
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_conjure_elemental(level, ch, el, sac);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in conjure_elemental.");
		break;
	}

}

#define DEMON_TYPE_I     20
#define DEMON_TYPE_II    21
#define DEMON_TYPE_III   22
#define DEMON_TYPE_IV    23
#define DEMON_TYPE_V     24
#define DEMON_TYPE_VI    25

#define TYPE_VI_ITEM     27002
#define TYPE_V_ITEM      5107
#define TYPE_IV_ITEM     5113
#define TYPE_III_ITEM    1101
#define TYPE_II_ITEM     21014
#define TYPE_I_ITEM      5105

void cast_cacaodemon(byte level, struct char_data* ch, const char* arg, int type,
					 struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buffer[40];
	int mob, obj;
	struct obj_data* sac;
	struct char_data* el;
	int held = FALSE, wielded = FALSE;
	send_to_char("Provi ad evocare un demone ma fallisci... Probabilmente l'incantesimo e' impossibile da lanciare...\n\r",ch);
	return;
	one_argument(arg,buffer);

	if(NoSummon(ch)) {
		return;
	}

	if(!str_cmp(buffer, "uno")) {
		mob = DEMON_TYPE_I;
		obj = TYPE_I_ITEM;
	}
	else if(!str_cmp(buffer, "due")) {
		mob = DEMON_TYPE_II;
		obj = TYPE_II_ITEM;
	}
	else if(!str_cmp(buffer, "tre")) {
		mob = DEMON_TYPE_III;
		obj = TYPE_III_ITEM;
	}
	else if(!str_cmp(buffer, "quattro")) {
		mob = DEMON_TYPE_IV;
		obj = TYPE_IV_ITEM;
	}
	else if(!str_cmp(buffer, "cinque")) {
		mob = DEMON_TYPE_V;
		obj = TYPE_V_ITEM;
	}
	else if(!str_cmp(buffer, "sei")) {
		mob = DEMON_TYPE_VI;
		obj = TYPE_VI_ITEM;
	}
	else {
		send_to_char("It seems that all demons of that type are currently in the service of others.\n\r", ch);
		return;
	}

	if(!ch->equipment[WIELD] && !ch->equipment[HOLD]) {
		send_to_char("You must wield or hold an item to offer the demon for its services.\n\r",ch);
		return;
	}

	if(ch->equipment[WIELD]) {
		if(ch->equipment[WIELD]->item_number >= 0 &&
				obj_index[ch->equipment[WIELD]->item_number].iVNum == obj) {
			wielded = TRUE;
		}
	}
	if(ch->equipment[HOLD]) {
		if(ch->equipment[WIELD]->item_number >= 0 &&
				obj_index[ch->equipment[HOLD]->item_number].iVNum == obj) {
			held = TRUE;
		}
	}

	if(!wielded && !held) {
		send_to_char("You do the spell perfectly, but no demon comes.\n\r", ch);
		send_to_char("You realize that the demon was dissatisfied with your offering and\n\r",ch);
		send_to_char("wants you to offer it something else.\n\r",ch);
		return;
	}

	sac = unequip_char(ch,(held ? HOLD : WIELD));
	if((sac) && (GET_LEVEL(ch,CLERIC_LEVEL_IND) > 40) && IS_EVIL(ch)) {
		if(sac->obj_flags.cost >= 200) {
			equip_char(ch,sac,(held ? HOLD : WIELD));
		}
		else {
			obj_to_char(sac, ch);
		}
	}
	else {
		obj_to_char(sac, ch);
	}

	if(sac) {
		if(ObjVnum(sac) != obj) {
			send_to_char("Your offering must be an item that the demon values.\n\r", ch);
			return;
		}
		el = read_mobile(mob, VIRTUAL);
		if(!el) {
			send_to_char("You sense that all demons of that kind are in others' services...\n\r", ch);
			return;
		}
	}
	else {
		send_to_char("You must be holding or wielding the item you are offering to the demon.\n\r", ch);
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_cacaodemon(level, ch, el, sac);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in cacaodemon.");
		break;
	}

}


void cast_mon_sum1(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(5, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}

void cast_mon_sum2(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(7, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}

void cast_mon_sum3(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(9, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}

void cast_mon_sum4(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(11, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}

void cast_mon_sum5(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(13, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}

void cast_mon_sum6(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(15, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}

void cast_mon_sum7(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_Create_Monster(17, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in monster_summoning_1");
		break;
	}
}


void cast_tree_travel(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_tree_travel(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in tree travel");
		break;
	}
}

void cast_speak_with_plants(byte level, struct char_data* ch, const char* arg,
							int type,  struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(!tar_obj) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_speak_with_plants(level, ch, 0, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in speak_with_plants");
		break;
	}
}

void cast_transport_via_plant(byte level, struct char_data* ch, const char* arg,
							  int type,  struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(!tar_obj) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_transport_via_plant(level, ch, 0, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in transport_via_plant");
		break;
	}
}

void cast_haste(byte level, struct char_data* ch, const char* arg,
				int type,  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_haste(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_haste(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_haste(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_haste(level,ch,tar_ch,0);
			}
		break;
	case SPELL_TYPE_POTION:
		spell_haste(level, ch, tar_ch, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in haste");
		break;
	}
}


void cast_slow(byte level, struct char_data* ch, const char* arg,
			   int type,  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_slow(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_slow(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_slow(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_slow(level,ch,tar_ch,0);
			}
		break;
	case SPELL_TYPE_POTION:
		spell_slow(level, ch, tar_ch, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in slow");
		break;
	}
}

void cast_reincarnate(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(!tar_obj) {
			return;
		}
		spell_reincarnate(level, ch, 0, tar_obj);
		break;
	case SPELL_TYPE_STAFF:
		if(!tar_obj) {
			return;
		}
		spell_reincarnate(level, ch, 0, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious problem in 'reincarnate'");
		break;
	}

}


void cast_changestaff(byte level, struct char_data* ch, const char* arg,
					  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buffer[40];

	one_argument(arg,buffer);

	if(NoSummon(ch)) {
		return;
	}

	if(!ch->equipment[HOLD]) {
		send_to_char("Devi tenere in mano un bastone!\n\r", ch);
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_changestaff(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in changestaff.");
		break;
	}

}



void cast_pword_kill(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(!tar_ch) {
		tar_ch = ch;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_pword_kill(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in pword_kill.");
		break;
	}

}

void cast_pword_blind(byte level, struct char_data* ch, const char* arg,
					  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(!tar_ch) {
		tar_ch = ch;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_pword_blind(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in pword_blind.");
		break;
	}
}


void cast_chain_lightn(byte level, struct char_data* ch, const char* arg,
					   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(!tar_ch) {
		tar_ch = ch;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_chain_lightn(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in chain lightn.");
		break;
	}
}

void cast_scare(byte level, struct char_data* ch, const char* arg,
				int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(!tar_ch) {
		tar_ch = ch;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_scare(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in scare.");
		break;
	}
}

void cast_familiar(byte level, struct char_data* ch, const char* arg,
				   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	char buf[128];

	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_familiar(level, ch, &tar_ch, 0);

		if(tar_ch) {
			sprintf(buf, "%s %s", GET_NAME(tar_ch), fname(arg));
			free(GET_NAME(tar_ch));
			GET_NAME(tar_ch) = (char*)malloc(strlen(buf)+1);
			strcpy(GET_NAME(tar_ch), buf);
		}
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in familiar.");
		break;
	}
}

void cast_aid(byte level, struct char_data* ch, const char* arg,
			  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	if(!tar_ch) {
		tar_ch = ch;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_aid(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in scare.");
		break;
	}
}


void cast_holyword(byte level, struct char_data* ch, const char* arg,
				   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_holyword(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in holy word.");
		break;
	}
}
void cast_unholyword(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_holyword(-level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in unholy word.");
		break;
	}
}


void cast_golem(byte level, struct char_data* ch, const char* arg,
				int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_golem(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in golem.");
		break;
	}
}


void cast_command(byte level, struct char_data* ch, const char* arg,
				  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	const char* p;
	char buf[128];
	/* have to parse the argument to get the command */


	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in command.");
		return;
		break;
	}

	for(; *arg==' '; arg++);

	if(arg && *arg) {
		p = fname(arg);

		if((GetMaxLevel(tar_ch) < 6 && GET_INT(tar_ch) < 13) ||
				!saves_spell(tar_ch, SAVING_PARA)) {
			if(strcmp(p, "quit")) {
                sprintf(buf, "Ordini a $N: '$c0015%s$c0007'.", p);
                act(buf, FALSE, ch, 0, tar_ch, TO_CHAR);
                sprintf(buf, "$n ti ordina: '$c0015%s$c0007'.", p);
				act(buf, FALSE, ch, 0, tar_ch, TO_VICT);
				command_interpreter(tar_ch, p);
				return;
			}
		}

        sprintf(buf, "Provi ad ordinare a $N: '$c0015%s$c0007'.", p);
        act(buf, FALSE, ch, 0, tar_ch, TO_VICT);
        sprintf(buf, "$n ha cercato di ordinarti: '$c0015%s$c0007'.", p);
		act(buf, FALSE, ch, 0, tar_ch, TO_VICT);

		if(!IS_PC(tar_ch)) {
			hit(tar_ch, ch, TYPE_UNDEFINED);
		}

	}
	else {
		return;
	}

}

void cast_change_form(byte level, struct char_data* ch, const char* arg,
					  int type, struct char_data* tar_ch,
					  struct obj_data* tar_obj) {
	char buffer[40];
	int mobn = -1, X=LAST_DRUID_MOB, found=FALSE;
	struct char_data* mob;
	struct affected_type af;
	//struct room_data *pRoom;
	long pRoom;

	one_argument(arg,buffer);

	if(IS_NPC(ch)) {
		send_to_char("Non lo vuoi fare realmente.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch, SPELL_CHANGE_FORM)) {
		send_to_char("Puoi cambiare forma solo una volta ogni 12 ore.\n\r", ch);
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:

		while(!found) {
			if(DruidList[X].level > level) {
				X--;
			}
			else {
				if(!str_cmp(DruidList[X].name, buffer)) {
					mobn = DruidList[X].number;
					found = TRUE;
				}
				else {
					X--;
				}
				if(X < 0) {
					break;
				}
			}
		}

		if(!found || mobn < 0) {
			send_to_char("Non c'e' nulla di simile.\n\r", ch);
			return;
		}
		else {

			af.type      = SPELL_CHANGE_FORM;
			af.duration  = 12;
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = 0;


			/* ACIDUS test shock da change_form di liv 50-51 */
			if(DruidList[X].level >= 50) {
				if(check_peaceful(ch, "")) {
					send_to_char("In questo luogo non sei sufficientemente in contatto con la forma $c0003animale$c0007 che cerchi di raggiungere.\n\r", ch);
					return;
				}

				if(number(1,10) == 1 && !IS_IMMORTAL(ch)) {
					act("Alteri completamente la tua forma, la personalita' della forma $c0003animale$c0007 prende il sopravvento, la tua anima abbandona il corpo!", TRUE, ch, NULL, ch,
						TO_CHAR);
					act("$n altera completamente la sua forma, la personalita' della forma $c0003animale$c0007 prende il sopravvento, la sua anima abbandona il corpo!", TRUE, ch, NULL, ch,
						TO_NOTVICT);
					affect_to_char(ch, &af);

					pRoom = (ch->in_room);

					damage(ch, ch, GET_MAX_HIT(ch)*12, SPELL_CHANGE_FORM, 5);

					if((mob = read_mobile(real_mobile(mobn), REAL))) {
						char_to_room(mob, pRoom);
					}
					return;
				}
			}
			/* fine shock */

			mob = read_mobile(mobn, VIRTUAL);
			if(mob) {
				affect_to_char(ch, &af);

				spell_poly_self(level, ch, mob, 0);

                if(HasClass(ch, CLASS_DRUID) && DruidList[X].level >= 50 && IS_PC(ch))
                {
                    if(IS_POLY(ch))
                    {
                        ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_DRUID_2] += 1;
                        if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                            SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                    }
                    else
                    {
                        ch->specials.achievements[CLASS_ACHIE][ACHIE_DRUID_2] += 1;
                        if(!IS_SET(ch->specials.act,PLR_ACHIE))
                            SET_BIT(ch->specials.act, PLR_ACHIE);
                    }

                    CheckAchie(ch, ACHIE_DRUID_2, CLASS_ACHIE);
                }
			}
			else {
				send_to_char("Non riesci ad evocare un'immagine di quella creatura.\n\r", ch);
			}
			return;
		}

		break;

	default:
		mudlog(LOG_SYSERR, "Problem in change form");
		break;
	}

}

void cast_shillelagh(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_shillelagh(level, ch, ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in shillelagh.");
		break;
	}
}

void cast_goodberry(byte level, struct char_data* ch, const char* arg,
					int type, struct char_data* tar_ch,
					struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_goodberry(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in goodberry.");
		break;
	}
}


void cast_elemental_blade(byte level, struct char_data* ch, const char* arg,
						  int type, struct char_data* tar_ch,
						  struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
		spell_elemental_blade(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in flame blade.");
		break;
	}
}

void cast_animal_growth(byte level, struct char_data* ch, const char* arg,
						int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_animal_growth(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in animal_growth.");
		break;
	}
}

void cast_insect_growth(byte level, struct char_data* ch, const char* arg,
						int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_insect_growth(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in insect_growth.");
		break;
	}
}



void cast_creeping_death(byte level, struct char_data* ch, const char* arg,
						 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	const char* p;
	int i;


	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:

		/* get the argument, parse it into a direction */
		for(; *arg==' '; arg++);
		if(!*arg) {
			send_to_char("In quale direzione vuoi lanciare gli insetti?\n\r", ch);
			return;
		}
		p = fname(arg);
		for(i=0; i<6; i++) {
			if(strncmp(p,dirs[i],strlen(p))==0) {
				i++;
				break;
			}
			if(i == 6) {
				send_to_char("In quale direzione vuoi lanciare gli insetti?\n\r", ch);
				return;
			}
		}

		spell_creeping_death(level, ch, 0, i);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in creeping_death.");
		break;
	}
}

void cast_commune(byte level, struct char_data* ch, const char* arg,
				  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	/* no break */
	case SPELL_TYPE_SCROLL:
		spell_commune(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in commune.");
		break;
	}
}

void cast_feeblemind(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_POTION:
		spell_feeblemind(level, ch, ch, 0);
		/* FALLTHRU */
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_feeblemind(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in feeblemind.");
		break;
	}
}


void cast_animal_summon_1(byte level, struct char_data* ch, const char* arg,
						  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_animal_summon(1, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in animal summon.");
		break;
	}
}

void cast_animal_summon_2(byte level, struct char_data* ch, const char* arg,
						  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_animal_summon(2, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in animal summon.");
		break;
	}
}

void cast_animal_summon_3(byte level, struct char_data* ch, const char* arg,
						  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_animal_summon(3, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in animal summon.");
		break;
	}
}

void cast_fire_servant(byte level, struct char_data* ch, const char* arg,
					   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_elemental_summoning(level, ch, 0, SPELL_FIRE_SERVANT);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in fire servant.");
		break;
	}
}


void cast_earth_servant(byte level, struct char_data* ch, const char* arg,
						int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_elemental_summoning(level, ch, 0, SPELL_EARTH_SERVANT);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in earth servant.");
		break;
	}
}


void cast_water_servant(byte level, struct char_data* ch, const char* arg,
						int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_elemental_summoning(level, ch, 0, SPELL_WATER_SERVANT);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in water servant.");
		break;
	}
}


void cast_wind_servant(byte level, struct char_data* ch, const char* arg,
					   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_elemental_summoning(level, ch, 0, SPELL_WIND_SERVANT);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in wind servant.");
		break;
	}
}



void cast_veggie_growth(byte level, struct char_data* ch, const char* arg,
						int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_veggie_growth(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in veggie_growth.");
		break;
	}
}


void cast_charm_veggie(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_charm_veggie(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) {
			return;
		}
		spell_charm_veggie(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_charm_veggie(level,ch,tar_ch,0);
			}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in charm veggie!");
		break;
	}
}

void cast_tree(byte level, struct char_data* ch, const char* arg, int type,
			   struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(NoSummon(ch)) {
		return;
	}

	if(IS_NPC(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_STAFF:
		spell_tree(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in tree");
		break;
	}
}



void cast_animate_rock(byte level, struct char_data* ch, const char* arg,
					   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	if(NoSummon(ch)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_animate_rock(level, ch, 0, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in animate_rock.");
		break;
	}
}

void cast_travelling(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_travelling(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in travelling.");
		break;
	}
}

void cast_animal_friendship(byte level, struct char_data* ch, const char* arg,
							int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_animal_friendship(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in animal friendship.");
		break;
	}
}

void cast_invis_to_animals(byte level, struct char_data* ch, const char* arg,
						   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_POTION:
		spell_invis_to_animals(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in invis to animals.");
		break;
	}
}

void cast_slow_poison(byte level, struct char_data* ch, const char* arg,
					  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_slow_poison(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in slow_poison.");
		break;
	}
}

void cast_entangle(byte level, struct char_data* ch, const char* arg,
				   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_entangle(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch) && !IS_IMMORTAL(tar_ch)) {
				spell_entangle(level,ch,tar_ch,0);
			}

	default:
		mudlog(LOG_SYSERR, "serious screw-up in entangle.");
		break;
	}
}

void cast_snare(byte level, struct char_data* ch, const char* arg,
				int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_snare(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch) && !IS_IMMORTAL(tar_ch)) {
				spell_snare(level,ch,tar_ch,0);
			}

	default:
		mudlog(LOG_SYSERR, "serious screw-up in snare.");
		break;
	}
}

void cast_gust_of_wind(byte level, struct char_data* ch, const char* arg,
					   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_gust_of_wind(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in snare.");
		break;
	}
}


void cast_barkskin(byte level, struct char_data* ch, const char* arg,
				   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_barkskin(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch)) {
				spell_barkskin(level,ch,tar_ch,0);
			}
	default:
		mudlog(LOG_SYSERR, "serious screw-up in barkskin.");
		break;
	}
}


void cast_warp_weapon(byte level, struct char_data* ch, const char* arg,
					  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_warp_weapon(level, ch, tar_ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in warp_weapon.");
		break;
	}
}



void cast_heat_stuff(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_heat_stuff(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in heat_stuff.");
		break;
	}
}

void cast_sunray(byte level, struct char_data* ch, const char* arg,
				 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_sunray(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in sunray.");
		break;
	}
}

void cast_find_traps(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_find_traps(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_STAFF:
		spell_find_traps(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in find_traps.");
		break;
	}
}

void cast_firestorm(byte level, struct char_data* ch, const char* arg,
					int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_firestorm(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in firestorm.");
		break;
	}
}


void cast_dust_devil(byte level, struct char_data* ch, const char* arg,
					 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_dust_devil(level, ch, 0, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in dust_devil.");
		break;
	}
}

void cast_know_monster(byte level, struct char_data* ch, const char* arg,
					   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			return;
		}
		spell_know_monster(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in know_monster.");
		break;
	}
}


void cast_silence(byte level, struct char_data* ch, const char* arg,
				  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_POTION:
		spell_silence(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_silence(level, ch, tar_ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		for(tar_ch = real_roomp(ch->in_room)->people ;
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if(!in_group(tar_ch,ch) && !IS_IMMORTAL(tar_ch)) {
				spell_silence(level,ch,tar_ch,0);
			}
	default:
		mudlog(LOG_SYSERR, "serious screw-up in silence.");
		break;
	}
}

void cast_dragon_ride(byte level, struct char_data* ch, const char* arg,
					  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	switch(type) {
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_dragon_ride(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in dragon_ride.");
		break;
	}
}


void cast_mount(byte level, struct char_data* ch, const char* arg,
				int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

	send_to_char("Non puoi piu' lanciare questo incantesimo!\n\r", ch);
	return;

	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_mount(level, ch, ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in mount.");
		break;
	}
}

void cast_sending(byte level, struct char_data* ch, const char* arg,
				  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {

}



void cast_portal(byte level, struct char_data* ch, const char* arg,
				 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		if(!tar_ch) {
			tar_ch = ch;
		}
		spell_portal(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in portal");
		break;

	}
}

void cast_teleport_wo_error(byte level, struct char_data* ch, const char* arg,
							int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_POTION:
	case SPELL_TYPE_SPELL:

		spell_astral_walk(level, ch, NULL, NULL);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in teleport without error!");
		break;
	}
}


/* NEW STUFF */

void cast_globe_darkness(byte level, struct char_data* ch, const char* arg,
						 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_globe_darkness(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in globe of darkness.");
		break;
	}
}
void cast_globe_minor_inv(byte level, struct char_data* ch, const char* arg,
						  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_globe_minor_inv(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in minor globe.");
		break;
	}

}
void cast_globe_major_inv(byte level, struct char_data* ch, const char* arg,
						  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_globe_major_inv(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in major globe.");
		break;
	}

}
void cast_prot_energy_drain(byte level, struct char_data* ch, const char* arg,
							int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_energy_drain(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-energy drain.");
		break;
	}

}
void cast_prot_dragon_breath(byte level, struct char_data* ch, const char* arg,
							 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_dragon_breath(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-dragon breath.");
		break;
	}

}


void cast_anti_magic_shell(byte level, struct char_data* ch, const char* arg,
						   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_POTION:
		spell_anti_magic_shell(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_anti_magic_shell(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_anti_magic_shell(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_anti_magic_shell!");
		break;
	}
}

void cast_comp_languages(byte level, struct char_data* ch, const char* arg, int type,
						 struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_comp_languages(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_POTION:
		spell_comp_languages(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_comp_languages(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_comp_languages(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in comprehend languages!");
		break;
	}
}



void cast_prot_fire(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_POTION:
		spell_prot_fire(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_prot_fire(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_prot_fire(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_prot_fire!");
		break;
	}
}
void cast_prot_cold(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_POTION:
		spell_prot_cold(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_prot_cold(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_prot_cold(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_prot_cold!");
		break;
	}
}
void cast_prot_energy(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_POTION:
		spell_prot_energy(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_prot_energy(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_prot_energy(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_prot_energy!");
		break;
	}
}
void cast_prot_elec(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_POTION:
		spell_prot_elec(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_SCROLL:
		if(tar_obj) {
			return;
		}
		spell_prot_elec(level,ch,tar_ch,0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		spell_prot_elec(level,ch,tar_ch,0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_prot_elec!");
		break;
	}
}


void cast_enchant_armor(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_enchant_armor(level, ch, 0,tar_obj);
		break;
	case SPELL_TYPE_SCROLL:
		if(!tar_obj) {
			return;
		}
		spell_enchant_armor(level, ch, 0,tar_obj);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in enchant armor!");
		break;
	}
}

void cast_messenger(byte level, struct char_data* ch, const char* arg,
					int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
}


void cast_prot_dragon_breath_fire(byte level, struct char_data* ch, const char* arg,
								  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_dragon_breath_fire(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-dragon breath_fire.");
		break;
	}

}

void cast_prot_dragon_breath_frost(byte level, struct char_data* ch, const char* arg,
								   int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_dragon_breath_frost(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-dragon breath_frost.");
		break;
	}

}


void cast_prot_dragon_breath_elec(byte level, struct char_data* ch, const char* arg,
								  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_dragon_breath_elec(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-dragon breath_elec.");
		break;
	}

}


void cast_prot_dragon_breath_acid(byte level, struct char_data* ch, const char* arg,
								  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_dragon_breath_acid(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-dragon breath_acid.");
		break;
	}

}


void cast_prot_dragon_breath_gas(byte level, struct char_data* ch, const char* arg,
								 int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_prot_dragon_breath_gas(level, ch, tar_ch, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "serious screw-up in prot-dragon breath_gas.");
		break;
	}

}
} // namespace Alarmud
