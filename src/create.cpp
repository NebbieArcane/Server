/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 *
 * $Id: create.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
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
#include "create.hpp"
#include "comm.hpp"
#include "modify.hpp"
#include "snew.hpp"
#include "structs.hpp"
#include "utility.hpp"
#include "vt100c.hpp"

namespace Alarmud {

#define MAIN_MENU           0

#define CHANGE_NAME         1
#define CHANGE_DESC         2
#define CHANGE_FLAGS        3
#define CHANGE_TYPE         4
#define CHANGE_TYPE2        5
#define CHANGE_TYPE3        6

#define CHANGE_EXIT         7

#define CHANGE_EXIT_NORTH   8
#define CHANGE_EXIT_EAST    9
#define CHANGE_EXIT_SOUTH   10
#define CHANGE_EXIT_WEST    11
#define CHANGE_EXIT_UP      12
#define CHANGE_EXIT_DOWN    13
#define CHANGE_EXIT_DELETE  14

#define CHANGE_NUMBER_NORTH 15
#define CHANGE_NUMBER_EAST  16
#define CHANGE_NUMBER_SOUTH 17
#define CHANGE_NUMBER_WEST  18
#define CHANGE_NUMBER_UP    19
#define CHANGE_NUMBER_DOWN  20

#define CHANGE_KEY_NORTH    21
#define CHANGE_KEY_EAST     22
#define CHANGE_KEY_SOUTH    23
#define CHANGE_KEY_WEST     24
#define CHANGE_KEY_UP       25
#define CHANGE_KEY_DOWN     26

#define CHANGE_EXIT_KEYWORD_NORTH 27
#define CHANGE_EXIT_KEYWORD_EAST  28
#define CHANGE_EXIT_KEYWORD_SOUTH 29
#define CHANGE_EXIT_KEYWORD_WEST  30
#define CHANGE_EXIT_KEYWORD_UP    31
#define CHANGE_EXIT_KEYWORD_DOWN  32

#define CHANGE_EXIT_DESC 33

#define ENTER_CHECK        1



const char* edit_menu = "    1) Name                       2) Description\n\r"
						"    3) Flags                      4) Sector Type\n\r"
						"    5) Exits                      6) Delete Exits\n\r\n\r";

const char* exit_menu = "    1) North                      2) East\n\r"
						"    3) South                      4) West\n\r"
						"    5) Up                         6) Down\n\r"
						"\n\r";
const char* aszExitName[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down"
};

void ChangeRoomFlags(struct room_data* rp, struct char_data* ch, const char* arg, int type) {
	int i, row, update;
	char buf[255];

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.edit = MAIN_MENU;
			UpdateRoomMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 31) {
			return;
		}
		i = 1<<update;

		if(IS_SET(rp->room_flags, i)) {
			REMOVE_BIT(rp->room_flags, i);
		}
		else {
			SET_BIT(rp->room_flags, i);
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Room Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 32; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            sprintf(buf, "%-2d [%s] %s", i + 1, ((rp->room_flags & (1<<i)) ? "X" : " "), room_bits[i]);
            send_to_char(buf, ch);
        }

        sprintf(buf, VT_CURSPOS, 20, 1);
        send_to_char(buf, ch);
    }
    else
    {
        string sb;
        boost::format fmt ("     %-2d [%s] %s");
        char buf2[255];
        int x = 0;

        send_to_char("\n\rRoom Flags:\n\r\n\r", ch);

        for(i = 0; i < 32; i++)
        {
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((rp->room_flags & (1<<i)) ? "X" : " ") % room_bits[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((rp->room_flags & (1<<i)) ? "X" : " ") % room_bits[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to toggle, <C/R> to return to main menu.\n\r--> ", ch);
}


ACTION_FUNC(do_redit) {
	struct room_data* rp;

	rp = real_roomp(ch->in_room);

	if(IS_NPC(ch)) {
		return;
	}

	if((IS_NPC(ch)) || (GetMaxLevel(ch)<IMMORTALE)) {
		return;
	}

	if(!ch->desc) { /* someone is forced to do something. can be bad! */
		return;    /* the ch->desc->str field will cause problems... */
	}


	if((GetMaxLevel(ch) < 56) && (rp->zone != GET_ZONE(ch))) {
		send_to_char("Mi dispiace, ma non hai l'autorizzazione per modificare "
					 "questa zona.\n\r", ch);
		return;
	}


	ch->specials.edit = MAIN_MENU;
	ch->desc->connected = CON_EDITING;

	act("$n inizia a $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.", FALSE, ch, 0, 0, TO_ROOM);

	UpdateRoomMenu(ch);
}


void UpdateRoomMenu(struct char_data* ch) {
	char buf[255];
	struct room_data* rp;
    int x = 0, i = 0;

	rp = real_roomp(ch->in_room);

	send_to_char(VT_HOMECLR, ch);
	sprintf(buf, VT_CURSPOS, 1, 1);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        send_to_char("\n\r\n\r", ch);
    }
	sprintf(buf, "Room Name: %s", rp->name);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        x = strlen(buf);
    }
	sprintf(buf, VT_CURSPOS, 1, 40);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        for (i = 0; i < (40 - (x > 39 ? 39 : x)); i++)
            send_to_char(" ", ch);
    }
    sprintf(buf, "Number: %ld", rp->number);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        x = strlen(buf);
    }
	sprintf(buf, VT_CURSPOS, 1, 60);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        for (i = 0; i < (20 - (x > 20 ? 19 : x)); i++)
            send_to_char(" ", ch);
    }
    sprintf(buf, "Sector Type: %s", sector_types[rp->sector_type]);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        send_to_char("\n\r\n\r", ch);
    }
	sprintf(buf, VT_CURSPOS, 3, 1);
	send_to_char(buf, ch);
	send_to_char("Menu:\n\r", ch);
	send_to_char(edit_menu, ch);
	send_to_char("--> ", ch);
}


void RoomEdit(struct char_data* ch, const char* arg) {
	if(ch->specials.edit == MAIN_MENU) {
		if(!*arg || *arg == '\n') {
			ch->desc->connected = CON_PLYNG;
			act("$n smette $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.", FALSE, ch, 0, 0, TO_ROOM);
			return;
		}

		switch(atoi(arg)) {
		case 0:
			UpdateRoomMenu(ch);
			return;
		case CHANGE_NAME:
			ch->specials.edit = CHANGE_NAME;
			ChangeRoomName(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
			return;
		case CHANGE_DESC:
			ch->specials.edit = CHANGE_DESC;
			ChangeRoomDesc(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
			return;
		case CHANGE_FLAGS:
			ch->specials.edit = CHANGE_FLAGS;
			ChangeRoomFlags(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
			return;
		case CHANGE_TYPE:
			ch->specials.edit = CHANGE_TYPE;
			ChangeRoomType(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
			return;
		case 5:
			ch->specials.edit = CHANGE_EXIT;
			ChangeExitDir(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
			return;
        case 6:
            ch->specials.edit = CHANGE_EXIT_DELETE;
            DeleteExit(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
            return;
		default:
			UpdateRoomMenu(ch);
			return;
		}
	}

	switch(ch->specials.edit) {
	case CHANGE_NAME:
		ChangeRoomName(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_DESC:
		ChangeRoomDesc(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_FLAGS:
		ChangeRoomFlags(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_TYPE:
	case CHANGE_TYPE2:
	case CHANGE_TYPE3:
		ChangeRoomType(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_EXIT:
		ChangeExitDir(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_EXIT_NORTH:
	case CHANGE_EXIT_EAST:
	case CHANGE_EXIT_SOUTH:
	case CHANGE_EXIT_WEST:
	case CHANGE_EXIT_UP:
	case CHANGE_EXIT_DOWN:
		AddExitToRoom(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_KEY_NORTH:
	case CHANGE_KEY_EAST:
	case CHANGE_KEY_SOUTH:
	case CHANGE_KEY_WEST:
	case CHANGE_KEY_UP:
	case CHANGE_KEY_DOWN:
		ChangeKeyNumber(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_NUMBER_NORTH:
	case CHANGE_NUMBER_EAST:
	case CHANGE_NUMBER_SOUTH:
	case CHANGE_NUMBER_WEST:
	case CHANGE_NUMBER_UP:
	case CHANGE_NUMBER_DOWN:
		ChangeExitNumber(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_EXIT_KEYWORD_NORTH:
	case CHANGE_EXIT_KEYWORD_EAST:
	case CHANGE_EXIT_KEYWORD_SOUTH:
	case CHANGE_EXIT_KEYWORD_WEST:
	case CHANGE_EXIT_KEYWORD_UP:
	case CHANGE_EXIT_KEYWORD_DOWN:
		ChangeExitKeyword(real_roomp(ch->in_room), ch, arg, 0);
		return;
	case CHANGE_EXIT_DESC:
		ch->specials.edit = CHANGE_EXIT;
		ChangeExitDir(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
		break;
    case CHANGE_EXIT_DELETE:
        DeleteExit(real_roomp(ch->in_room), ch, arg, 0);
        break;
	default:
		mudlog(LOG_ERROR, "Got to bad spot in RoomEdit");
		return;
	}
}


void ChangeRoomName(struct room_data* rp, struct char_data* ch, const char* arg,int type) {
	char buf[255];

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.edit = MAIN_MENU;
			UpdateRoomMenu(ch);
			return;
		}

	if(type != ENTER_CHECK) {
		if(rp->name) {
			free(rp->name);
		}
		rp->name = (char*)strdup(arg);
		ch->specials.edit = MAIN_MENU;
		UpdateRoomMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Room Name: %s", rp->name);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Room Name: ", ch);

	return;
}


void ChangeRoomDesc(struct room_data* rp, struct char_data* ch, const char* arg, int type) {
	char buf[255];

	if(type != ENTER_CHECK) {
		ch->specials.edit = MAIN_MENU;
		UpdateRoomMenu(ch);
	}
	else {
		sprintf(buf, VT_HOMECLR);
		send_to_char(buf, ch);

		sprintf(buf, "Current Room Description:\n\r");
		send_to_char(buf, ch);
		send_to_char(rp->description, ch);
		send_to_char("\n\r\n\rNew Room Description:\n\r", ch);
		send_to_char("(Terminate with a @. Press <C/R> again to continue)\n\r", ch);
		free(rp->description);
		rp->description = NULL;
		ch->desc->str = &rp->description;
		ch->desc->max_str = MAX_STRING_LENGTH;
	}
}


void ChangeRoomType(struct room_data* rp, struct char_data* ch, const char* arg, int type) {
	int i, row, update;
	char buf[255];

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.edit = MAIN_MENU;
			UpdateRoomMenu(ch);
			return;
		}
	}

	update = atoi(arg);
	update--;

	if(type != ENTER_CHECK) {
		switch(ch->specials.edit) {
		case CHANGE_TYPE:
			if(update < 0 || update > 11) {
				return;
			}
			else {
				rp->sector_type = update;
				if(rp->sector_type == SECT_WATER_NOSWIM) {
					send_to_char("\n\rRiver Speed: ", ch);
					ch->specials.edit = CHANGE_TYPE2;
				}
				else {
					ch->specials.edit = MAIN_MENU;
					UpdateRoomMenu(ch);
				}
				return;
			}
		case CHANGE_TYPE2:
			rp->river_speed = update;
			send_to_char("\n\rRiver Direction (0 - 5): ", ch);
			ch->specials.edit = CHANGE_TYPE3;
			return;
		case CHANGE_TYPE3:
			update++;
			if(update < 0 || update > 5) {
				send_to_char("Direction must be between 0 and 5.\n\r", ch);
				return;
			}
			rp->river_dir = update;
			ch->specials.edit = MAIN_MENU;
			UpdateRoomMenu(ch);
			return;
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Sector Type: %s", sector_types[rp->sector_type]);
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < E_SECTOR_TYPES_COUNT; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            sprintf(buf, "%-2d %s", i + 1, sector_types[i]);
            send_to_char(buf, ch);
        }

        sprintf(buf, VT_CURSPOS, 20, 1);
        send_to_char(buf, ch);
    }
    else
    {
        string sb;
        boost::format fmt ("     %-2d %s");
        char buf2[255];
        int x = 0;

        sprintf(buf, "\n\rSector Type: %s\n\r\n\r", sector_types[rp->sector_type]);
        send_to_char(buf, ch);

        for(i = 0; i < E_SECTOR_TYPES_COUNT; i++)
        {
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d %s\n\r");
            boost::format fmt2 (buf2);

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % sector_types[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % sector_types[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to set to, <C/R> to return to main menu.\n\r--> ", ch);
}


void ChangeExitDir(struct room_data* rp, struct char_data* ch, const char* arg,
				   int type) {
	int update, x = 0, i = 0;
	char buf[1024];

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.edit = MAIN_MENU;
			UpdateRoomMenu(ch);
			return;
		}

		update = atoi(arg) - 1;
		if(update == -1) {
			ChangeExitDir(rp, ch, "", ENTER_CHECK);
			return;
		}

		if(update >= 0 && update <= 5) {
			ch->specials.edit = CHANGE_EXIT_NORTH + update;
			AddExitToRoom(rp, ch, "", ENTER_CHECK);
		}
		else {
			ChangeExitDir(rp, ch, "", ENTER_CHECK);
		}
	}
	else
    {
        send_to_char(VT_HOMECLR, ch);
        if(ch->term != VT100)
        {
            send_to_char("\n\r\n\r", ch);
        }
        sprintf(buf, "Room Name: %s", rp->name);
        send_to_char(buf, ch);
        if(ch->term != VT100)
        {
            x = strlen(buf);
        }
        sprintf(buf, VT_CURSPOS, 1, 40);
        send_to_char(buf, ch);
        if(ch->term != VT100)
        {
            for (i = 0; i < (40 - (x > 39 ? 39 : x)); i++)
                send_to_char(" ", ch);
        }
        sprintf(buf, "Room Number: %ld", rp->number);
        send_to_char(buf, ch);
        if(ch->term != VT100)
        {
            send_to_char("\n\r\n\r", ch);
        }
        sprintf(buf, VT_CURSPOS, 4, 1);
        send_to_char(buf, ch);
        send_to_char(exit_menu, ch);
        send_to_char("--> ", ch);
	}
}


void AddExitToRoom(struct room_data* rp, struct char_data* ch, const char* arg,
				   int type) {
	int update, dir, row, i = 0;
	char buf[255];

#if 0
#endif

	dir = ch->specials.edit - CHANGE_EXIT_NORTH;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.edit = CHANGE_NUMBER_NORTH + dir;
			sprintf(buf, "\n\r\n\rOld room # of %s exit: %ld.",
					aszExitName[ dir ], rp->dir_option[dir]->to_room);
			send_to_char(buf, ch);
			sprintf(buf, "\n\rRoom # of %s exit: ", aszExitName[ dir ]);
			send_to_char(buf, ch);
			ChangeExitNumber(rp, ch, "", ENTER_CHECK);
			return;
		}

		update = atoi(arg) - 1;

		if(update >= 0 && update <= 8) {
			i = 1 << update;

			if(IS_SET(rp->dir_option[dir]->exit_info, i)) {
				REMOVE_BIT(rp->dir_option[dir]->exit_info, i);
			}
			else {
				SET_BIT(rp->dir_option[dir]->exit_info, i);
			}
		}
	}
	else if(!rp->dir_option[ dir ]) {
		CREATE(rp->dir_option[dir], struct room_direction_data, 1);
		rp->dir_option[dir]->exit_info = 0;
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Flags of %s exit:", aszExitName[ dir ]);
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 9; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            sprintf(buf, "%-2d [%s] %s", i + 1, ((rp->dir_option[dir]->exit_info & (1<<i)) ? "X" : " "), exit_bits[ i ]);
            send_to_char(buf, ch);
        }

        sprintf(buf, VT_CURSPOS, 20, 1);
        send_to_char(buf, ch);
    }
    else
    {
        string sb;
        boost::format fmt ("     %-2d [%s] %s");
        char buf2[255];
        int x = 0;

        sprintf(buf, "\n\rFlags of %s exit:\n\r\n\r", aszExitName[ dir ]);
        send_to_char(buf, ch);

        for(i = 0; i < 9; i++)
        {
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((rp->dir_option[dir]->exit_info & (1<<i)) ? "X" : " ") % exit_bits[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((rp->dir_option[dir]->exit_info & (1<<i)) ? "X" : " ") % exit_bits[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to toggle, <C/R> to return to continue.\n\r--> ", ch);
}


void ChangeExitNumber(struct room_data* rp, struct char_data* ch, const char* arg, int type) {
	int dir, update;
	char buf[255];

	dir = ch->specials.edit - CHANGE_NUMBER_NORTH;

	if(type == ENTER_CHECK) {
		return;
	}

	if(arg && *arg) {
		update = atoi(arg);

		if(update < 0 || update > WORLD_SIZE) {
			sprintf(buf,"\n\rRoom number must be between 0 and %d.\n\r",WORLD_SIZE);
			send_to_char(buf, ch);
			sprintf(buf, "\n\r\n\rOld room # of %s exit: %ld.",
					aszExitName[ dir ], rp->dir_option[dir]->to_room);
			send_to_char(buf, ch);
			sprintf(buf, "\n\rRoom # of %s exit: ", aszExitName[ dir ]);
			send_to_char(buf, ch);
			return;
		}

		rp->dir_option[dir]->to_room = update;
	}

	ch->specials.edit = CHANGE_KEY_NORTH + dir;

	sprintf(buf, "\n\r\n\rOld key # of %s exit: %ld.",
			aszExitName[ dir ], rp->dir_option[dir]->key);
	send_to_char(buf, ch);
	sprintf(buf, "\n\rKey # of %s exit (0 for none): ", aszExitName[ dir ]);
	send_to_char(buf, ch);

	ChangeKeyNumber(rp, ch, "", ENTER_CHECK);
}


void ChangeKeyNumber(struct room_data* rp, struct char_data* ch, const char* arg,
					 int type) {
	int dir;
	int update;
	char buf[255];

	dir = ch->specials.edit - CHANGE_KEY_NORTH;

	if(type == ENTER_CHECK) {
		return;
	}

	if(arg && *arg) {
		update = atoi(arg);

		if(!rp->dir_option[dir]->keyword) {
			rp->dir_option[dir]->keyword = (char*)strdup("");
		}

		if(update < 0) {
			send_to_char("\n\rKey number must be greater than 0.\n\r", ch);
			sprintf(buf, "\n\r\n\rOld key # of %s exit: %ld.",
					aszExitName[ dir ], rp->dir_option[ dir ]->key);
			send_to_char(buf, ch);
			sprintf(buf, "\n\rKey # of %s exit (0 for none): ", aszExitName[ dir ]);
			send_to_char(buf, ch);
			return;
		}

		rp->dir_option[dir]->key = update;
	}

	ch->specials.edit =  CHANGE_EXIT_KEYWORD_NORTH + dir;
	sprintf(buf, "\n\rOld %s exit keywords: ", aszExitName[ dir ]);
	send_to_char(buf, ch);
	if(rp->dir_option[ dir ]->keyword && *(rp->dir_option[ dir ]->keyword)) {
		send_to_char(rp->dir_option[ dir ]->keyword, ch);
	}
	else {
		send_to_char("(none)", ch);
	}
	sprintf(buf, "\n\rInsert %s exit keywords: ", aszExitName[ dir ]);
	send_to_char(buf, ch);
}

void ChangeExitKeyword(struct room_data* rp, struct char_data* ch, const char* arg,
					   int type) {
	char buf[ 255 ];
	int dir = ch->specials.edit - CHANGE_EXIT_KEYWORD_NORTH;

	if(rp->dir_option[ dir ]->keyword) {
		free(rp->dir_option[ dir ]->keyword);
	}

	rp->dir_option[dir]->keyword = (char*)strdup(arg);
	if(rp->dir_option[dir]->keyword[0]=='\n' or rp->dir_option[dir]->keyword[0]=='\r') {
		rp->dir_option[dir]->keyword[0]='0';
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current %s exit description:\n\r", aszExitName[ dir ]);
	send_to_char(buf, ch);
	if(rp->dir_option[ dir ]->general_description &&
			*rp->dir_option[ dir ]->general_description) {
		send_to_char(rp->dir_option[ dir ]->general_description, ch);
	}
	else {
		send_to_char("(none)", ch);
	}
	sprintf(buf, "\n\r\n\rNew %s exit description:\n\r", aszExitName[ dir ]);
	send_to_char(buf, ch);
	send_to_char("(Terminate with a @. Press <C/R> again to continue)\n\r", ch);
	if(rp->dir_option[ dir ]->general_description) {
		free(rp->dir_option[ dir ]->general_description);
	}
	rp->dir_option[ dir ]->general_description = NULL;
	ch->desc->str = &rp->dir_option[ dir ]->general_description;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->specials.edit =  CHANGE_EXIT_DESC;
}


void DeleteExit(struct room_data* rp, struct char_data* ch, const char* arg, int type)
{
    int update, x = 0, i = 0;
    char buf[1024];

    if(type != ENTER_CHECK)
    {
        if(!*arg || (*arg == '\n'))
        {
            ch->specials.edit = MAIN_MENU;
            UpdateRoomMenu(ch);
            return;
        }

        update = atoi(arg) - 1;
        if(update == -1)
        {
            DeleteExit(rp, ch, "", ENTER_CHECK);
            return;
        }

        if(update >= 0 && update <= 5)
        {
            if(rp->dir_option[update])
            {
                free(rp->dir_option[update]->general_description);
                free(rp->dir_option[update]->keyword);
                free(rp->dir_option[update]);
                rp->dir_option[update] = NULL;
            }
            else
            {
                sprintf(buf, "\n\rThere is not any exits to %s.\n\r", aszExitName[ update ] );
                send_to_char(buf, ch);
                DeleteExit(rp, ch, "", ENTER_CHECK);
                return;
            }
            sprintf(buf, "\n\rDone. Deleted exit to %s.\n\r", aszExitName[ update ] );
            send_to_char(buf, ch);
            DeleteExit(rp, ch, "", ENTER_CHECK);
            return;
        }
        else
        {
            DeleteExit(rp, ch, "", ENTER_CHECK);
        }
    }
    else
    {
        send_to_char(VT_HOMECLR, ch);
        if(ch->term != VT100)
        {
            send_to_char("\n\r\n\r", ch);
        }
        sprintf(buf, "Room Name: %s", rp->name);
        send_to_char(buf, ch);
        if(ch->term != VT100)
        {
            x = strlen(buf);
        }
        sprintf(buf, VT_CURSPOS, 1, 40);
        send_to_char(buf, ch);
        if(ch->term != VT100)
        {
            for (i = 0; i < (40 - (x > 39 ? 39 : x)); i++)
                send_to_char(" ", ch);
        }
        sprintf(buf, "Room Number: %ld", rp->number);
        send_to_char(buf, ch);
        if(ch->term != VT100)
        {
            send_to_char("\n\r\n\r", ch);
        }
        sprintf(buf, VT_CURSPOS, 4, 1);
        send_to_char(buf, ch);
        send_to_char(exit_menu, ch);
        send_to_char("--> ", ch);
    }
}
} // namespace Alarmud

