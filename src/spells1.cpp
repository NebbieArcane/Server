/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: spells1.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
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
#include "spells1.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "spells.hpp"
#include "fight.hpp"
#include "handler.hpp"
namespace Alarmud {


/* Global data */

void cast_burning_hands(byte level, struct char_data* ch,const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_BURNING_HANDS)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		spell_burning_hands(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in burning hands!");
		break;
	}
}


void cast_call_lightning(byte level, struct char_data* ch,const char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_CALL_LIGHTNING)) {
		return;
	}

	switch(type) {
	case SPELL_TYPE_SPELL:
		if(OUTSIDE(ch) && weather_info.sky >= SKY_RAINING) {
			spell_call_lightning(level, ch, victim, 0);
		}
		else {
			send_to_char("Non riesci ad evocare il potere dei $c0012fulmini$c0007!\n\r", ch);
		}
		break;
	case SPELL_TYPE_POTION:
		if(OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
			spell_call_lightning(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_SCROLL:
		if(OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
			if(victim) {
				spell_call_lightning(level, ch, victim, 0);
			}
			else if(!tar_obj) {
				spell_call_lightning(level, ch, ch, 0);
			}
		}
		break;
	case SPELL_TYPE_STAFF:
		if(OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
			staff_for_each(ch, StaffTargets::Hostile, StaffOptions{}.skip_untouchable().group_victim_first(), [&](char_data* v) {
				spell_call_lightning(level, ch, v, nullptr);
			});
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in call lightning!");
		break;
	}
}


void cast_chill_touch(byte level, struct char_data* ch,const char* arg, int type,
					  struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_CHILL_TOUCH)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
		spell_chill_touch(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in chill touch!");
		break;
	}
}


void cast_shocking_grasp(byte level, struct char_data* ch,const char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_SHOCKING_GRASP)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_shocking_grasp(level, ch, victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in shocking grasp!");
		break;
	}
}


void cast_colour_spray(byte level, struct char_data* ch,const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_COLOUR_SPRAY)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_colour_spray(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_colour_spray(level, ch, victim, 0);
		}
		else if(!tar_obj) {
			spell_colour_spray(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_colour_spray(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in colour spray!");
		break;
	}
}


void cast_earthquake(byte level, struct char_data* ch,const char* arg, int type,
					 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_EARTHQUAKE)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND: //Acidus 2003, ora il quake si usa anche su armi
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_earthquake(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in earthquake!");
		break;
	}
}


void cast_energy_drain(byte level, struct char_data* ch,const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_ENERGY_DRAIN)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_energy_drain(level, ch, victim, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_energy_drain(level, ch, ch, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_energy_drain(level, ch, victim, 0);
		}
		else if(!tar_obj) {
			spell_energy_drain(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_energy_drain(level, ch, victim, 0);
		}
		break;
	case SPELL_TYPE_STAFF:
		staff_for_each(ch, StaffTargets::Hostile, StaffOptions{}.skip_untouchable(), [&](char_data* v) {
			spell_energy_drain(level, ch, v, nullptr);
		});
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in energy drain!");
		break;
	}
}


void cast_fireball(byte level, struct char_data* ch,const char* arg, int type,
				   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_FIREBALL)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_STAFF:
		spell_fireball(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in fireball");
		break;
	}
}


void cast_harm(byte level, struct char_data* ch,const char* arg, int type,
			   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_HARM)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_harm(level, ch, victim, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_harm(level, ch, ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		staff_for_each(ch, StaffTargets::Hostile, StaffOptions{}.skip_untouchable(), [&](char_data* v) {
			spell_harm(level, ch, v, nullptr);
		});
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in harm!");
		break;
	}
}


void cast_lightning_bolt(byte level, struct char_data* ch,const char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_LIGHTNING_BOLT)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_lightning_bolt(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_lightning_bolt(level, ch, victim, 0);
		}
		else if(!tar_obj) {
			spell_lightning_bolt(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_lightning_bolt(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in lightning bolt!");
		break;
	}
}

void cast_acid_blast(byte level, struct char_data* ch,const char* arg, int type,
					 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_ACID_BLAST)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_acid_blast(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_acid_blast(level, ch, victim, 0);
		}
		else {
			spell_acid_blast(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_acid_blast(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in acid blast!");
		break;
	}
}

void cast_cone_of_cold(byte level, struct char_data* ch,const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_CONE_OF_COLD)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_cone_of_cold(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR,"Serious screw-up in cone of cold!");
		break;
	}
}

void cast_ice_storm(byte level, struct char_data* ch,const char* arg, int type,
					struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_ICE_STORM)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
		spell_ice_storm(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR,"Serious screw-up in ice storm");
		break;
	}
}

void cast_meteor_swarm(byte level, struct char_data* ch,const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_METEOR_SWARM)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_meteor_swarm(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_meteor_swarm(level, ch, victim, 0);
		}
		else {
			spell_meteor_swarm(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_meteor_swarm(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in meteor swarm!");
		break;
	}
}

void cast_flamestrike(byte level, struct char_data* ch,const char* arg, int type,
					  struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_FLAMESTRIKE)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_flamestrike(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_flamestrike(level, ch, victim, 0);
		}
		else if(!tar_obj) {
			spell_flamestrike(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_flamestrike(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in flamestrike!");
		break;
	}
}

void cast_magic_missile(byte level, struct char_data* ch,const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_MAGIC_MISSILE)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_magic_missile(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_magic_missile(level, ch, victim, 0);
		}
		else if(!tar_obj) {
			spell_magic_missile(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_magic_missile(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in magic missile!");
		break;
	}
}


void cast_cause_light(byte level, struct char_data* ch,const char* arg, int type,
					  struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_CAUSE_LIGHT)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_WAND:
		spell_cause_light(level, ch, victim, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_cause_light(level, ch, ch, 0);
		break;
	case SPELL_TYPE_STAFF:
		staff_for_each(ch, StaffTargets::Hostile, StaffOptions{}, [&](char_data* v) {
			spell_cause_light(level, ch, v, nullptr);
		});
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cause light wounds!");
		break;
	}
}

void cast_cause_serious(byte level, struct char_data* ch,const char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_CAUSE_SERIOUS)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cause_serious(level, ch, victim, 0);
		break;
	case SPELL_TYPE_POTION:
		spell_cause_serious(level, ch, ch, 0);
		break;
	case SPELL_TYPE_WAND:
		if(tar_obj) {
			return;
		}
		if(!victim) {
			victim = ch;
		}
		spell_cause_serious(level, ch, victim, 0);
		break;
	case SPELL_TYPE_STAFF:
		staff_for_each(ch, StaffTargets::Hostile, StaffOptions{}.skip_untouchable(), [&](char_data* v) {
			spell_cause_serious(level, ch, v, nullptr);
		});
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cause serious wounds!");
		break;
	}
}


void cast_cause_critic(byte level, struct char_data* ch,const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_CAUSE_CRITICAL)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_cause_critical(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(!victim) {
			victim = ch;
		}
		spell_cause_critical(level,ch,victim,0);
		break;
	case SPELL_TYPE_WAND:
		if(!victim) {
			victim = ch;
		}
		spell_cause_critical(level,ch,victim,0);
		break;
	case SPELL_TYPE_POTION:
		if(!victim) {
			victim = ch;
		}
		spell_cause_critical(level, ch, victim, 0);
		break;
	case SPELL_TYPE_STAFF:
		staff_for_each(ch, StaffTargets::Hostile, StaffOptions{}.skip_untouchable(), [&](char_data* v) {
			spell_cause_critical(level, ch, v, nullptr);
		});
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cause critical!");
		break;
	}
}


void cast_geyser(byte level, struct char_data* ch, const char* arg, int type,
				 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_GEYSER)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_STAFF:
		spell_geyser(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in geyser!");
		break;
	}
}


void cast_green_slime(byte level, struct char_data* ch,const char* arg, int type,
					  struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_GREEN_SLIME)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_green_slime(level, ch, victim, 0);
		break;
	case SPELL_TYPE_SCROLL:
		if(victim) {
			spell_green_slime(level, ch, victim, 0);
		}
		else if(!tar_obj) {
			spell_green_slime(level, ch, ch, 0);
		}
		break;
	case SPELL_TYPE_WAND:
		if(victim) {
			spell_green_slime(level, ch, victim, 0);
		}
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in green Slime!");
		break;
	}
}


void cast_incendiary_cloud(byte level, struct char_data* ch,const char* arg,
						   int type, struct char_data* victim,
						   struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_INCENDIARY_CLOUD)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		spell_incendiary_cloud(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR,"Serious screw-up in incendiary cloud!");
		break;
	}
}

void cast_prismatic_spray(byte level, struct char_data* ch,const char* arg,
						  int type, struct char_data* victim,
						  struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, nullptr, SPELL_PRISMATIC_SPRAY)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		spell_prismatic_spray(level, ch, 0, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in prismatic spray!");
		break;
	}
}

void cast_wizard_eye(byte level, struct char_data* ch,const char* arg, int type,
					 struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_WIZARDEYE)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		spell_wizard_eye(level, ch,victim, 0);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_wizard_eye!");
		break;
	}
}

void cast_disintegrate(byte level, struct char_data* ch,const char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj) {
	if(blocked_by_anti_magic(ch, victim, SPELL_DISINTEGRATE)) {
		return;
	}
	switch(type) {
	case SPELL_TYPE_WAND:
	case SPELL_TYPE_SPELL:
	case SPELL_TYPE_STAFF:
	case SPELL_TYPE_SCROLL:
		spell_disintegrate(level, ch,victim, tar_obj);
		break;
	default :
		mudlog(LOG_SYSERR, "Serious screw-up in cast_disintegrate");
		break;
	}
}
} // namespace Alarmud

