/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: sound.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <string>
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
#include "sound.hpp"
#include "comm.hpp"
#include "db.hpp"

namespace Alarmud {
/*
 * AlarMUD v2.0
 * See license.doc for distribution terms.   BenemMUD is based on DIKUMUD
*/

int RecGetObjRoom(struct obj_data* obj) {
	if(obj->in_room != NOWHERE) {
		return(obj->in_room);
	}
	else if(obj->carried_by) {
		return(obj->carried_by->in_room);
	}
	else if(obj->equipped_by) {
		return(obj->equipped_by->in_room);
	}
	else if(obj->in_obj) {
		return(RecGetObjRoom(obj->in_obj));
	}
	return NOWHERE;
}

namespace {
/**
 * I suoni presi dai file (es. action_description degli oggetti ITEM_AUDIO)
 * spesso non finiscono con un a capo: senza, due suoni nello stesso pulse
 * finivano attaccati sulla stessa riga.
 */
std::string noise_line(const char* snd) {
	std::string line = snd;
	if(line.empty() || (line.back() != '\n' && line.back() != '\r')) {
		line += "\n\r";
	}
	return line;
}
} // namespace

void MakeNoise(int room, const char* local_snd, const char* distant_snd) {
	room_data* const rp = real_roomp(room);
	if(rp == nullptr) {
		return;
	}
	if(local_snd != nullptr && *local_snd) {
		const std::string local = noise_line(local_snd);
		for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			if(!IS_NPC(ch) && !IS_AFFECTED(ch, AFF_SILENCE)) {
				send_to_char(local.c_str(), ch);
			}
		}
	}
	if(distant_snd == nullptr || !*distant_snd) {
		return;
	}
	const std::string distant = noise_line(distant_snd);
	for(int door = 0; door <= 5; door++) {
		if(rp->dir_option[door] == nullptr || rp->dir_option[door]->to_room == room) {
			continue;
		}
		room_data* const orp = real_roomp(rp->dir_option[door]->to_room);
		if(orp == nullptr) {
			continue;
		}
		for(char_data* ch = orp->people; ch != nullptr; ch = ch->next_in_room) {
			if(!IS_NPC(ch) && !IS_SET(ch->specials.act, PLR_DEAF) && !IS_AFFECTED(ch, AFF_SILENCE)) {
				send_to_char(distant.c_str(), ch);
			}
		}
	}
}

void MakeSound(unsigned long localPulse) {
	int room;
	char buffer[128];
	struct obj_data* obj;
	struct char_data* ch;

	/*
	 *  objects
	 */

	for(obj = object_list; obj; obj = obj->next) {
		if(ITEM_TYPE(obj) == ITEM_AUDIO) {
			if(((obj->obj_flags.value[0]) &&
					(localPulse % obj->obj_flags.value[0])==0) ||
					(!number(0,5))) {
				if(obj->carried_by) {
					room = obj->carried_by->in_room;
				}
				else if(obj->equipped_by) {
					room = obj->equipped_by->in_room;
				}
				else if(obj->in_room != NOWHERE) {
					room = obj->in_room;
				}
				else {
					room = RecGetObjRoom(obj);
				}
				/*
				 *  broadcast to room
				 */

				if(obj->action_description) {
					MakeNoise(room, obj->action_description, obj->action_description);
				}
			}
		}
	}

	/*
	 *   mobiles
	 */

	for(ch = character_list; ch; ch = ch->next) {
		if(IS_NPC(ch) && (ch->player.sounds) && (number(0,5)==0)) {
			if(ch->specials.default_pos > POSITION_SLEEPING) {
				if(GET_POS(ch) > POSITION_SLEEPING) {
					/*
					 *  Make the sound;
					 */
					MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
				}
				else if(GET_POS(ch) == POSITION_SLEEPING) {
					/*
					 * snore
					 */
					snprintf(buffer, 127, "%s russa sonoramente.\n\r",
							 ch->player.short_descr);
					MakeNoise(ch->in_room, buffer,
							  "Qualcuno russa sonoramente qui vicino.\n\r");
				}
			}
			else if(GET_POS(ch) == ch->specials.default_pos) {
				/*
				 * Make the sound
				 */
				MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
			}
		}
	}
}


} // namespace Alarmud

