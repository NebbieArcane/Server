/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: create.mob.c,v 1.2 2002/02/13 12:31:00 root Exp $
*/
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
#include "create.mob.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "modify.hpp"
#include "snew.hpp"
#include "structs.hpp"
#include "vt100c.hpp"

namespace Alarmud {


#define MOB_MAIN_MENU         0
#define CHANGE_MOB_NAME       1
#define CHANGE_MOB_SHORT      2
#define CHANGE_MOB_LONG       3
#define CHANGE_MOB_DESC       4
#define CHANGE_MOB_ACT_FLAGS  5
#define CHANGE_MOB_AFF_FLAGS  6
#define CHANGE_MOB_ALIGN      7
#define CHANGE_MOB_HITP       8
#define CHANGE_MOB_ARMOR      9
#define CHANGE_MOB_DAMSIZE   10
#define CHANGE_MOB_DAMNUMB   11
#define CHANGE_MOB_DAMPLUS   12
#define CHANGE_MOB_RACE      13
#define CHANGE_MOB_MULTATT   14
#define CHANGE_MOB_EXP       15
#define CHANGE_MOB_DPOS      16
#define CHANGE_MOB_RESIST    17
#define CHANGE_MOB_IMMUNE    18
#define CHANGE_MOB_SUSCEP    19
#define CHANGE_MOB_SOUND     20
#define CHANGE_MOB_DSOUND    21
#define CHANGE_MOB_SPECIAL   22
#define MOB_HIT_RETURN       99

#define ENTER_CHECK        1



const char* mob_edit_menu = "    1) Name                    2) Short description\n\r"
							"    3) Long description        4) Description\n\r"
							"    5) Action flags            6) Affect flags\n\r"
							"    7) Alignment               8) Maximum hitpoints\n\r"
							"    9) Armor class            10) Damage size of die\n\r"
							"   11) Damage number of die   12) Damage roll plus\n\r"
							"   13) Race                   14) Number of attacks\n\r"
							"   15) Exp flags/amount       16) Default position\n\r"
							"   17) Resistances            18) Immunities\n\r"
							"   19) Susceptibilities       20) Sounds\n\r"
							"   21) Distant sounds         22) Special\n\r\n\r";


void ChangeMobActFlags(struct char_data* ch, const char* arg, int type) {
	int i, row, update;
	char buf[255];

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 31) {
			return;
		}
		i = 1<<update;

		if(i != ACT_POLYSELF)  {         /* do not allow polyself flags */
			if(IS_SET(ch->specials.mobedit->specials.act, i)) {
				REMOVE_BIT(ch->specials.mobedit->specials.act, i);
			}
			else {
				SET_BIT(ch->specials.mobedit->specials.act, i);
			}
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Mobile Action Flags:");
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
            sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.mobedit->specials.act & (1<<i)) ? "X" : " "), action_bits[i]);
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

        send_to_char("\n\rMobile Action Flags:\n\r\n\r", ch);

        for(i = 0; i < 32; i++)
        {
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((ch->specials.mobedit->specials.act & (1<<i)) ? "X" : " ") % action_bits[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((ch->specials.mobedit->specials.act & (1<<i)) ? "X" : " ") % action_bits[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to toggle, <C/R> to return to main menu.\n\r--> ",ch);
}


void ChangeMobAffFlags(struct char_data* ch, const char* arg, int type) {
	int a, row, update;
	char buf[255];
	long i, check=0;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 36) {
			return;
		}
		i=1;
        if(update > 0 && update < 32)
            for(a=1; a<=update; a++) {
                i*=2;
            }
        else
            for(a=1; a<=update-32; a++) {
                i*=2;
            }

        if(update<32)
        {
            if(IS_SET(ch->specials.mobedit->specials.affected_by, i)) {
                REMOVE_BIT(ch->specials.mobedit->specials.affected_by, i);
            }
            else {
                SET_BIT(ch->specials.mobedit->specials.affected_by, i);
            }
        }
        else
        {
            if(IS_SET(ch->specials.mobedit->specials.affected_by2, i)) {
                REMOVE_BIT(ch->specials.mobedit->specials.affected_by2, i);
            }
            else {
                SET_BIT(ch->specials.mobedit->specials.affected_by2, i);
            }
        }
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Mobile Affect Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 41; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            check=1;
            if(i > 0 && i < 32)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }
            else
                for(a=1; a<=(i-32); a++)
                {
                    check*=2;
                }
            if (i < 32)
            {
                sprintf(buf, "%-2ld [%s] %s", i + 1, ((ch->specials.mobedit->specials.affected_by & (check)) ? "X" : " "), affected_bits[ i ]);
                send_to_char(buf, ch);
            }
            else
            {
                sprintf(buf, "%-2ld [%s] %s", i + 1, ((ch->specials.mobedit->specials.affected_by2 & (check)) ? "X" : " "), affected_bits2[ i-32 ]);
                send_to_char(buf, ch);
            }
        }

        sprintf(buf, VT_CURSPOS, 21, 1);
        send_to_char(buf, ch);
    }
    else
    {
        string sb;
        boost::format fmt ("     %-2d [%s] %s");
        char buf2[255];
        int x = 0;

        send_to_char("\n\rMobile Affect Flags:\n\r\n\r", ch);

        for(i = 0; i < 41; i++)
        {
            check=1;
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i > 0 && i < 32)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }
            else
                for(a=1; a<=(i-32); a++)
                {
                    check*=2;
                }

            if(i < 32)
            {
                if(i & 1)
                {
                    fmt2 % "" % (i + 1) % ((ch->specials.mobedit->specials.affected_by & (check)) ? "X" : " ") % affected_bits[i];
                    sb.append(fmt2.str().c_str());
                }
                else
                {
                    fmt % (i + 1) % ((ch->specials.mobedit->specials.affected_by & (check)) ? "X" : " ") % affected_bits[i];
                    sb.append(fmt.str().c_str());
                    x = strlen(fmt.str().c_str());
                }
            }
            else
            {
                if(i & 1)
                {
                    fmt2 % "" % (i + 1) % ((ch->specials.mobedit->specials.affected_by2 & (check)) ? "X" : " ") % affected_bits2[i-32];
                    sb.append(fmt2.str().c_str());
                }
                else
                {
                    fmt % (i + 1) % ((ch->specials.mobedit->specials.affected_by2 & (check)) ? "X" : " ") % affected_bits2[i-32];
                    sb.append(fmt.str().c_str());
                    x = strlen(fmt.str().c_str());
                }
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to toggle, <C/R> to return to main menu.\n\r--> ",ch);
}



ACTION_FUNC(do_medit) {
	char name[MAX_INPUT_LENGTH];
	struct char_data* mob;
	int i;

	if(IS_NPC(ch)) {
		return;
	}

	if((IS_NPC(ch)) || (GetMaxLevel(ch)<DIO)) {
		return;
	}

	if(!ch->desc) { /* someone is forced to do something. can be bad! */
		return;    /* the ch->desc->str field will cause problems... */
	}


	for(i = 0; *(arg + i) == ' '; i++);
	if(!*(arg + i)) {
		send_to_char("Quale mob vuoi modificare?\n\r", ch);
		return;
	}

	arg = one_argument(arg, name);

	if(!(mob = (struct char_data*)get_char_room_vis(ch, name)))         {
		send_to_char("Questo mob non e' qui.\n\r",ch);
		return;
	}

	if(IS_PC(mob)) {
		send_to_char("Non puoi modificare i giocatori.\n\r",ch);
		return;
	}

	if(GetMaxLevel(ch) < CREATORE && !IS_SET(ch->player.user_flags,CAN_MOB_EDIT)) {
		send_to_char("Mi dispiace, ma non hai l'autorizzazione per modificare i mobs.\n\r",ch);
		return;
	}

	ch->specials.mobedit=mob;
	ch->specials.medit = MOB_MAIN_MENU;
	ch->desc->connected = CON_MOB_EDITING;

	act("$n inizia a $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.", FALSE, ch, 0, 0, TO_ROOM);
	GET_POS(ch)=POSITION_SLEEPING;

	UpdateMobMenu(ch);
}


void UpdateMobMenu(struct char_data* ch) {
	char buf[255];
	struct char_data* mob;

	mob = ch->specials.mobedit;

	send_to_char(VT_HOMECLR, ch);
	sprintf(buf, VT_CURSPOS, 1, 1);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        send_to_char("\n\r\n\r", ch);
    }
	sprintf(buf, "Mobile Name: %s", GET_NAME(mob));
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        send_to_char("\n\r", ch);
    }
	sprintf(buf, VT_CURSPOS, 3, 1);
	send_to_char(buf, ch);
	send_to_char("Menu:\n\r", ch);
	send_to_char(mob_edit_menu, ch);
	send_to_char("--> ", ch);
}


void MobEdit(struct char_data* ch, const char* arg) {
	if(ch->specials.medit == MOB_MAIN_MENU) {
		if(!*arg || *arg == '\n') {
			ch->desc->connected = CON_PLYNG;
			act("$n smette $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.", FALSE, ch, 0, 0, TO_ROOM);
			GET_POS(ch)=POSITION_STANDING;
			return;
		}
		switch(atoi(arg)) {
		case 0:
			UpdateMobMenu(ch);
			return;
		case 1:
			ch->specials.medit = CHANGE_MOB_NAME;
			ChangeMobName(ch, "", ENTER_CHECK);
			return;
		case 2:
			ch->specials.medit = CHANGE_MOB_SHORT;
			ChangeMobShort(ch, "", ENTER_CHECK);
			return;
		case 3:
			ch->specials.medit = CHANGE_MOB_LONG;
			ChangeMobLong(ch, "", ENTER_CHECK);
			return;
		case 4:
			ch->specials.medit = CHANGE_MOB_DESC;
			ChangeMobDesc(ch, "", ENTER_CHECK);
			return;
		case 5:
			ch->specials.medit = CHANGE_MOB_ACT_FLAGS;
			ChangeMobActFlags(ch, "", ENTER_CHECK);
			return;
		case 6:
			ch->specials.medit = CHANGE_MOB_AFF_FLAGS;
			ChangeMobAffFlags(ch, "", ENTER_CHECK);
			return;
		case 7:
			ch->specials.medit = CHANGE_MOB_ALIGN;
			ChangeMobAlign(ch, "", ENTER_CHECK);
			return;
		case 8:
			ch->specials.medit = CHANGE_MOB_HITP;
			ChangeMobHitp(ch, "", ENTER_CHECK);
			return;
		case 9:
			ch->specials.medit = CHANGE_MOB_ARMOR;
			ChangeMobArmor(ch, "", ENTER_CHECK);
			return;
		case 10:
			ch->specials.medit = CHANGE_MOB_DAMSIZE;
			ChangeMobDamsize(ch, "", ENTER_CHECK);
			return;
		case 11:
			ch->specials.medit = CHANGE_MOB_DAMNUMB;
			ChangeMobDamnumb(ch, "", ENTER_CHECK);
			return;
		case 12:
			ch->specials.medit = CHANGE_MOB_DAMPLUS;
			ChangeMobDamplus(ch, "", ENTER_CHECK);
			return;
		case 13:
			ch->specials.medit = CHANGE_MOB_RACE;
			ChangeMobRace(ch, "", ENTER_CHECK);
			return;
		case 14:
			ch->specials.medit = CHANGE_MOB_MULTATT;
			ChangeMobMultatt(ch, "", ENTER_CHECK);
			return;
		case 15:
			ch->specials.medit = CHANGE_MOB_EXP;
			ChangeMobExp(ch, "", ENTER_CHECK);
			return;
		case 16:
			ch->specials.medit = CHANGE_MOB_DPOS;
			ChangeMobDpos(ch, "", ENTER_CHECK);
			return;
		case 17:
			ch->specials.medit = CHANGE_MOB_RESIST;
			ChangeMobResist(ch, "", ENTER_CHECK);
			return;
		case 18:
			ch->specials.medit = CHANGE_MOB_IMMUNE;
			ChangeMobImmune(ch, "", ENTER_CHECK);
			return;
		case 19:
			ch->specials.medit = CHANGE_MOB_SUSCEP;
			ChangeMobSuscep(ch, "", ENTER_CHECK);
			return;
		case 20:
			ch->specials.medit = CHANGE_MOB_SOUND;
			ChangeMobSound(ch, "", ENTER_CHECK);
			return;
		case 21:
			ch->specials.medit = CHANGE_MOB_DSOUND;
			ChangeMobDsound(ch, "", ENTER_CHECK);
			return;
        case 22:
            ch->specials.medit = CHANGE_MOB_SPECIAL;
            ChangeMobSpecial(ch, "", ENTER_CHECK);
            return;
		default:
			UpdateMobMenu(ch);
			return;
		}
	}

	switch(ch->specials.medit) {
	case CHANGE_MOB_NAME:
		ChangeMobName(ch, arg, 0);
		return;
	case CHANGE_MOB_SHORT:
		ChangeMobShort(ch, arg, 0);
		return;
	case CHANGE_MOB_LONG:
		ChangeMobLong(ch, arg, 0);
		return;
	case CHANGE_MOB_DESC:
		ChangeMobDesc(ch, arg, 0);
		return;
	case CHANGE_MOB_ACT_FLAGS:
		ChangeMobActFlags(ch, arg, 0);
		return;
	case CHANGE_MOB_AFF_FLAGS:
		ChangeMobAffFlags(ch, arg, 0);
		return;
	case CHANGE_MOB_ALIGN:
		ChangeMobAlign(ch, arg, 0);
		return;
	case CHANGE_MOB_HITP:
		ChangeMobHitp(ch, arg, 0);
		return;
	case CHANGE_MOB_ARMOR:
		ChangeMobArmor(ch, arg, 0);
		return;
	case CHANGE_MOB_DAMSIZE:
		ChangeMobDamsize(ch, arg, 0);
		return;
	case CHANGE_MOB_DAMNUMB:
		ChangeMobDamnumb(ch, arg, 0);
		return;
	case CHANGE_MOB_DAMPLUS:
		ChangeMobDamplus(ch, arg, 0);
		return;
	case CHANGE_MOB_RACE:
		ChangeMobRace(ch, arg, 0);
		return;
	case CHANGE_MOB_MULTATT:
		ChangeMobMultatt(ch, arg, 0);
		return;
	case CHANGE_MOB_EXP:
		ChangeMobExp(ch, arg, 0);
		return;
	case CHANGE_MOB_DPOS:
		ChangeMobDpos(ch, arg, 0);
		return;
	case CHANGE_MOB_RESIST:
		ChangeMobResist(ch, arg, 0);
		return;
	case CHANGE_MOB_IMMUNE:
		ChangeMobImmune(ch, arg, 0);
		return;
	case CHANGE_MOB_SUSCEP:
		ChangeMobSuscep(ch, arg, 0);
		return;
	case CHANGE_MOB_SOUND:
		ChangeMobSound(ch, arg, 0);
		return;
	case CHANGE_MOB_DSOUND:
		ChangeMobDsound(ch, arg, 0);
		return;
	case MOB_HIT_RETURN:
		MobHitReturn(ch, arg, 0);
		return;
    case CHANGE_MOB_SPECIAL:
        ChangeMobSpecial(ch, arg, 0);
        return;
	default:
		mudlog(LOG_ERROR, "Got to bad spot in MobEdit");
		return;
	}
}


void ChangeMobSpecial(struct char_data* ch, const char* arg, int type) {
	char buf[256], proc[256], parms[256];
	struct char_data* mob;
	struct OtherSpecialProcEntry* op;
	int i;
	int lastotherproc = 0;

	if(type != ENTER_CHECK)
	{
		if(!*arg || (*arg == '\n'))
		{
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}

	mob = ch->specials.mobedit;
	if(type != ENTER_CHECK)
	{
		arg = one_argument(arg, proc);
		only_argument(arg, parms);

		if(strstr(proc, "nessuna") || strstr(proc, "none"))
		{
			mob_index[mob->nr].func         = NULL;
			mob_index[mob->nr].specname     = NULL;
			mob_index[mob->nr].specparms    = NULL;
		}
		else
		{
			for(i = 0; strcmp(otherproc[i].nome, "zFineprocedure"); i++)
			{
				lastotherproc++;
			}

			if(!(op = (struct OtherSpecialProcEntry*) bsearch(&proc, otherproc, lastotherproc, sizeof(struct OtherSpecialProcEntry), nomecompare)))
			{
				mudlog(LOG_ERROR, "mobile_assign: Mobile %d not found in database.", mob_index[mob->nr].iVNum);
			}
			else
			{
				mob_index[mob->nr].func         = op->proc;
				mob_index[mob->nr].specname     = op->nome;
				mob_index[mob->nr].specparms    = strdup(parms);
			}
		}

		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mobile Special: %s %s", 	mob_index[mob->nr].specname ?
													mob_index[mob->nr].specname :
													"nothing",
													mob_index[mob->nr].specparms ?
													mob_index[mob->nr].specparms :
													"");
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Mobile Special (name, parameters), type 'none' for no-special: ", ch);

	return;
}


void ChangeMobName(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		if(mob->player.name) {
			free(mob->player.name);
		}
		mob->player.name = (char*)strdup(arg);
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mobile Name: %s",GET_NAME(mob));
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Mobile Name: ", ch);

	return;
}

void ChangeMobShort(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		if(mob->player.short_descr) {
			free(mob->player.short_descr);
		}
		mob->player.short_descr = (char*)strdup(arg);
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mobile Short Description: %s", mob->player.short_descr);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Mobile Short Description: ", ch);

	return;
}

void ChangeMobLong(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		if(mob->player.long_descr) {
			free(mob->player.long_descr);
		}
		mob->player.long_descr = (char*)strdup(arg);
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mobile Long Description: %s", mob->player.long_descr);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Mobile Long Description: ", ch);

	return;
}


void ChangeMobDesc(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;

	if(type != ENTER_CHECK) {
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	mob=ch->specials.mobedit;
	sprintf(buf, "Current Mobile Description:\n\r");
	send_to_char(buf, ch);
	send_to_char(mob->player.description, ch);
	send_to_char("\n\r\n\rNew Mobile Description:\n\r", ch);
	send_to_char("(Terminate with a @. Press <C/R> again to continue)\n\r", ch);
	free(mob->player.description);
	mob->player.description = NULL;
	ch->desc->str = &mob->player.description;
	ch->desc->max_str = MAX_STRING_LENGTH;
	return;
}


void ChangeMobAlign(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	long change;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<-1000 || change>1000) {
			change=0;
		}
		GET_ALIGNMENT(mob) = change;
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mob Alignment: %d", GET_ALIGNMENT(mob));
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Alignment [between -1000[evil] and 1000[good]: ",
				 ch);

	return;
}



void ChangeMobHitp(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	int change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<=0) {
			change=1;
		}
		mob->points.max_hit = change;
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Maximum Hit Points: %d", GET_MAX_HIT(mob));
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Maximum Hit Points: ", ch);

	return;
}



void ChangeMobArmor(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	signed int change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<-100 || change>100) {
			change=0;
		}
		mob->points.armor = change;
		sprintf(buf,"%d %d",change,mob->points.armor);
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mob Armor Class: %d", mob->points.armor);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Mob Armor Class[between -100[armored] and 100[naked]: ", ch);

	return;
}



void ChangeMobDamplus(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	int change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<=0 || change>255) {
			change=1;
		}
		mob->points.damroll = change;
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mob Damroll Plus: %d", mob->points.damroll);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Damroll Plus: ", ch);

	return;
}



void ChangeMobDamsize(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	int change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<=0 || change>255) {
			change=1;
		}
		mob->specials.damsizedice = change;
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Mob Damage Dice Number of Sides: %d", mob->specials.damsizedice);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Damage Dice Number of Sides: ", ch);

	return;
}


void ChangeMobDamnumb(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	int change;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}

	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<=0) {
			change=1;
		}
		if(change<=0 || change>255) {
			change=1;
		}
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	sprintf(buf, "Current Number of Times Damage Dice is rolled: %d",
			mob->specials.damnodice);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Number of Times Damage Dice is rolled: ", ch);

	return;
}



void ChangeMobMultatt(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	int change;

	mob=ch->specials.mobedit;

	if(type != ENTER_CHECK) {
		if(!*arg || *arg == '\n' || mob->specials.mobtype == 'S') {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}

	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<1) {
			change = 1;
		}
		mob->mult_att = change;
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	if(mob->specials.mobtype=='S') {
		send_to_char("This is a simple mobile, and does not have a number of "
					 "attacks field.\n\rHit return.\n\r\n\r", ch);
		ch->specials.medit =  MOB_HIT_RETURN;
		MobHitReturn(ch, "", ENTER_CHECK);
		return;
	}
	sprintf(buf, "Current Mob Number of Attacks: %f", mob->mult_att);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Number of Attacks: ", ch);

	return;
}

void MobHitReturn(struct char_data* ch, const char* arg, int type) {

	if(type != ENTER_CHECK) {
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	send_to_char("\n\rHit return: ", ch);

	return;
}

void ChangeMobExp(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	long change, expamount;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}


	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<0) {
			change=0;
		}
		if(mob->specials.mobtype=='S') {
			GET_EXP(mob) = change;
		}
		else {
			if(change>32) {
				send_to_char("That expflag is too high!\n\r",ch);
				ch->specials.medit =  MOB_HIT_RETURN;
				MobHitReturn(ch, "", ENTER_CHECK);
				return;
			}
			mob->specials.exp_flag = change;
			expamount = DetermineExp(mob, mob->specials.exp_flag) + mob->points.gold;
			sprintf(buf,"This mobile will have around %ld exp the next time it is "
					"loaded.\n\r", expamount);
			send_to_char(buf,ch);
			ch->specials.medit =  MOB_HIT_RETURN;
			MobHitReturn(ch, "", ENTER_CHECK);
			return;
		}

		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	if(mob->specials.mobtype=='S') {
		sprintf(buf, "Current Mob Exp: %d", GET_EXP(mob));
	}
	else {
		sprintf(buf, "Current Mob Expflag: %d", mob->specials.exp_flag);
	}
	send_to_char(buf, ch);
	if(mob->specials.mobtype=='S') {
		send_to_char("\n\r\n\rNew Exp Amount: ", ch);
	}
	else {
		send_to_char("\n\r\n\rNew Expflag: ", ch);
	}

	return;
}


void ChangeMobDpos(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;
	int change;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}


	mob=ch->specials.mobedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<0 || change>9) {
			change=0;
		}
		mob->specials.default_pos = change;
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	send_to_char("Positions:\n\r",ch);
	send_to_char(" 0 - dead            1 - mortally wounded\n\r",ch);
	send_to_char(" 2 - incapacitated   3 - stunned\n\r",ch);
	send_to_char(" 4 - sleeping        5 - resting\n\r",ch);
	send_to_char(" 6 - sitting         7 - fighting\n\r",ch);
	send_to_char(" 8 - standing        9 - mounted\n\r\n\r\n\r",ch);
	sprintf(buf, "Current Mob Default Position: %d", mob->specials.default_pos);
	send_to_char(buf, ch);
	send_to_char("\n\r\n\rNew Default Position: ", ch);

	return;
}


void ChangeMobRace(struct char_data* ch, const char* arg, int type) {
	int update,row=2,i,a=0,column;
	char buf[255];

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}


	update = atoi(arg);
	update--;

	if(type != ENTER_CHECK) {
		switch(ch->specials.medit) {
		case CHANGE_MOB_RACE:
			if(update < 0 || update > MAX_RACE) {
				return;
			}
			else {
				GET_RACE(ch->specials.mobedit) = update;
				ch->specials.medit = MOB_MAIN_MENU;
				UpdateMobMenu(ch);
				return;
			}
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Mobile race: %s", RaceName[GET_RACE(ch->specials.mobedit)]);
        send_to_char(buf, ch);

        for(i = 0; i < MAX_RACE; i++)
        {
            a++;
            if(a==1)
            {
                column=5;
            }
            else if(a==2)
            {
                column = 30;
            }
            else if(a==3) {
                column = 55;
            }
            else
            {
                column = 0;
            }
            sprintf(buf, VT_CURSPOS, row + 1, column);
            if(a==3) {
                row++;
                a=0;
            }
            send_to_char(buf, ch);
            sprintf(buf, "%-2d %s", i + 1, RaceName[i]);
            send_to_char(buf, ch);
        }
#if 0
        sprintf(buf, VT_CURSPOS, 21, 1);
        send_to_char(buf, ch);
#endif
    }
    else
    {
        string sb;
        char buf2[255];
        boost::format fmt ("     %-2d %s");
        int x = 0;

        sprintf(buf, "\n\rMobile race: %s\n\r\n\r", RaceName[GET_RACE(ch->specials.mobedit)]);
        send_to_char(buf, ch);

        for(i = 0; i < MAX_RACE; i++)
        {
            a++;
            if(a == 1)
            {
                boost::format fmt ("     %-2d %s");
                fmt % (i + 1) % RaceName[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }
            else if(a == 2)
            {
                sprintf(buf2, "%s", "%-");
                sprintf(buf2, "%s%d", buf2, 30-x);
                strcat(buf2, "s%-2d %s");
                boost::format fmt2 (buf2);
                fmt2 % "" % (i + 1) % RaceName[i];
                sb.append(fmt2.str().c_str());
                x += strlen(fmt2.str().c_str());
                fmt2.clear();
            }
            else if(a == 3)
            {
                sprintf(buf2, "%s", "%-");
                sprintf(buf2, "%s%d", buf2, 55-x);
                strcat(buf2, "s%-2d %s\n\r");
                boost::format fmt (buf2);
                fmt % "" % (i + 1) % RaceName[i];
                sb.append(fmt.str().c_str());
                x = 0;
                a = 0;
            }

            fmt.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("\n\rSelect the race number to set to, <C/R> to return to "
				 "main menu.\n\r--> ", ch);
}

void ChangeMobResist(struct char_data* ch, const char* arg, int type) {
	int i, a, check=0, row, update;
	char buf[255];

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}


	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 17) {
			return;
		}
		i=1;
		if(update>0)
			for(a=1; a<=update; a++) {
				i*=2;
			}

		if(IS_SET(ch->specials.mobedit->immune, i)) {
			REMOVE_BIT(ch->specials.mobedit->immune, i);
		}
		else {
			SET_BIT(ch->specials.mobedit->immune, i);
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Mobile Resistances Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 18; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            check=1;
            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }
            sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.mobedit->immune & (check)) ? "X" : " "), immunity_names[i]);
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

        send_to_char("\n\rMobile Resistances Flags:\n\r\n\r", ch);

        for(i = 0; i < 18; i++)
        {
            check = 1;
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((ch->specials.mobedit->immune & (check)) ? "X" : " ") % immunity_names[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((ch->specials.mobedit->immune & (check)) ? "X" : " ") % immunity_names[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to toggle, <C/R> to return to main menu.\n\r--> ",ch);
}


void ChangeMobImmune(struct char_data* ch, const char* arg, int type) {
	int i, a, check=0, row, update;
	char buf[255];

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}
	}


	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 17) {
			return;
		}
		i=1;
		if(update>0)
			for(a=1; a<=update; a++) {
				i*=2;
			}

		if(IS_SET(ch->specials.mobedit->M_immune, i)) {
			REMOVE_BIT(ch->specials.mobedit->M_immune, i);
		}
		else {
			SET_BIT(ch->specials.mobedit->M_immune, i);
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Mobile Immunities Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 18; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            check=1;
            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }
            sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.mobedit->M_immune & (check)) ? "X" : " "), immunity_names[i]);
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

        send_to_char("\n\rMobile Immunities Flags:\n\r\n\r", ch);

        for(i = 0; i < 18; i++)
        {
            check = 1;
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((ch->specials.mobedit->M_immune & (check)) ? "X" : " ") % immunity_names[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((ch->specials.mobedit->M_immune & (check)) ? "X" : " ") % immunity_names[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

	send_to_char("Select the number to toggle, <C/R> to return to main menu.\n\r--> ",ch);
}


void ChangeMobSuscep(struct char_data* ch, const char* arg, int type) {
	int i, a, check=0, row, update;
	char buf[255];

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.medit = MOB_MAIN_MENU;
			UpdateMobMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 17) {
			return;
		}
		i=1;
		if(update>0)
			for(a=1; a<=update; a++) {
				i*=2;
			}

		if(IS_SET(ch->specials.mobedit->susc, i)) {
			REMOVE_BIT(ch->specials.mobedit->susc, i);
		}
		else {
			SET_BIT(ch->specials.mobedit->susc, i);
		}
	}

    if(ch->term == VT100)
    {
        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        sprintf(buf, "Mobile Susceptibilities Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 18; i++)
        {
            sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }
            send_to_char(buf, ch);
            check=1;
            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }
            sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.mobedit->susc & (check)) ? "X" : " "), immunity_names[i]);
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

        send_to_char("\n\rMobile Susceptibilities Flags:\n\r\n\r", ch);

        for(i = 0; i < 18; i++)
        {
            check = 1;
            sprintf(buf2, "%s", "%-");
            sprintf(buf2, "%s%d", buf2, 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((ch->specials.mobedit->susc & (check)) ? "X" : " ") % immunity_names[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((ch->specials.mobedit->susc & (check)) ? "X" : " ") % immunity_names[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\r\n\n\r");
        page_string(ch->desc, sb.c_str(), true);
    }

    send_to_char("Select the number to toggle, <C/R> to return to main menu.\n\r--> ",ch);
}


void ChangeMobSound(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;

	if(type != ENTER_CHECK) {
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	mob=ch->specials.mobedit;
	if(mob->specials.mobtype!='L') {
		send_to_char("Please use a base mobile that already has sound, this one does not.\n\r",ch);
		ch->specials.medit =  MOB_HIT_RETURN;
		MobHitReturn(ch, "", ENTER_CHECK);
		return;
	}

	sprintf(buf, "Current Mobile Sound:\n\r");
	send_to_char(buf, ch);
	send_to_char(mob->player.sounds, ch);
	send_to_char("\n\r\n\rNew Mobile Sound:\n\r", ch);
	send_to_char("(Terminate with a @. Press <C/R> again to continue)\n\r", ch);
	free(mob->player.sounds);
	mob->player.sounds = NULL;
	ch->desc->str = &mob->player.sounds;
	ch->desc->max_str = MAX_STRING_LENGTH;
	return;
}


void ChangeMobDsound(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct char_data* mob;

	if(type != ENTER_CHECK) {
		ch->specials.medit = MOB_MAIN_MENU;
		UpdateMobMenu(ch);
		return;
	}

	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);

	mob=ch->specials.mobedit;
	if(mob->specials.mobtype!='L') {
		send_to_char("Please use a base mobile that already has sound, this one does not.\n\r",ch);
		ch->specials.medit =  MOB_HIT_RETURN;
		MobHitReturn(ch, "", ENTER_CHECK);
		return;
	}

	sprintf(buf, "Current Mobile Distant Sound:\n\r");
	send_to_char(buf, ch);
	send_to_char(mob->player.distant_snds, ch);
	send_to_char("\n\r\n\rNew Mobile Distant Sound:\n\r", ch);
	send_to_char("(Terminate with a @. Press <C/R> again to continue)\n\r", ch);
	free(mob->player.distant_snds);
	mob->player.distant_snds = NULL;
	ch->desc->str = &mob->player.distant_snds;
	ch->desc->max_str = MAX_STRING_LENGTH;
	return;
}


} // namespace Alarmud
