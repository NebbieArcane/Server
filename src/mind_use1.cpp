/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: mind_use1.c,v 1.2 2002/02/13 12:31:00 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
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
#include "mind_use1.hpp"
#include "comm.hpp"
#include "handler.hpp"
#include "mindskills1.hpp"
#include "spells.hpp"

namespace Alarmud {

/*
***         BenemMUD
***         PSI skills
*/




void mind_use_burn(byte level, struct char_data* ch, const char* arg, int type,
				   struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_burn(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_burn");
		break;
	}
}



void mind_use_teleport(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_teleport(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_teleport");
		break;
	}
}

void mind_use_probability_travel(byte level, struct char_data* ch, const char* arg, int type,
								 struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_probability_travel(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_probability_travel");
		break;
	}
}

void mind_use_danger_sense(byte level, struct char_data* ch, const char* arg, int type,
						   struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_danger_sense(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_danger_sense");
		break;
	}
}

void mind_use_clairvoyance(byte level, struct char_data* ch, const char* arg, int type,
						   struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_clairvoyance(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_clairvoyance");
		break;
	}
}

void mind_use_disintegrate(byte level, struct char_data* ch, const char* arg, int type,
						   struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_disintegrate(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_disintegrate");
		break;
	}
}

void mind_use_telekinesis(byte level, struct char_data* ch, const char* arg, int type,
						  struct char_data* victim, struct obj_data* tar_obj) {
	const char* p;
	int i=-1;
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		if(!ch->specials.fighting) {
			/* get the argument, parse it into a direction */
			for(; *arg==' '; arg++);
			if(!*arg) {
				send_to_char("Devi indicare una direzione!\n\r", ch);
				return;
			}
			p = fname(arg);
			for(i=0; i<6; i++) {
				if(strncmp(p,dirs[i],strlen(p))==0) {
					i++;
					break;
				}
				if(i == 6) {
					send_to_char("Devi indicare una direzione!\n\r", ch);
					return;
				}
			}
		} /* end ! fighting */
		mind_telekinesis(level, ch, victim, i);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_telekinesis");
		break;
	}
}

void mind_use_levitation(byte level, struct char_data* ch, const char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_levitation(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_levitation");
		break;
	}
}

void mind_use_cell_adjustment(byte level, struct char_data* ch, const char* arg, int type,
							  struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_cell_adjustment(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_cell_adjustment");
		break;
	}
}

void mind_use_chameleon(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_chameleon(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_chameleon");
		break;
	}
}

void mind_use_psi_strength(byte level, struct char_data* ch, const char* arg, int type,
						   struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_psi_strength(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_psi_strength");
		break;
	}
}

void mind_use_mind_over_body(byte level, struct char_data* ch, const char* arg, int type,
							 struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_mind_over_body(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_mind_over_body");
		break;
	}
}

void mind_use_domination(byte level, struct char_data* ch, const char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_domination(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_domination");
		break;
	}
}

void mind_use_mind_wipe(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_mind_wipe(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_wipe");
		break;
	}
}

void mind_use_psychic_crush(byte level, struct char_data* ch, const char* arg, int type,
							struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_psychic_crush(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_psyic_crush");
		break;
	}
}

void mind_use_tower_iron_will(byte level, struct char_data* ch, const char* arg, int type,
							  struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_tower_iron_will(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR,"Serious screw-up in mind_tower_iron_will");
		break;
	}
}

void mind_use_mindblank(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_mindblank(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR,"Serious screw-up in mind_mindblank");
		break;
	}
}

void mind_use_psychic_impersonation(byte level, struct char_data* ch, const char* arg, int type,
									struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_psychic_impersonation(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_psychic_impersonation");
		break;
	}
}

void mind_use_ultra_blast(byte level, struct char_data* ch, const char* arg, int type,
						  struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_ultra_blast(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_ultra_blast");
		break;
	}
}

void mind_use_intensify(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		mind_intensify(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in mind_intensify");
		break;
	}
}

} // namespace Alarmud

