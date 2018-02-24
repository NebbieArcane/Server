/*AlarMUD*/
/* $Id: handler.c,v 1.2 2002/02/27 01:26:55 Thunder Exp $
 * */
#include "handler.hpp"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#include "events.hpp"
#include "fight.hpp"
#include "protos.hpp"
#include "signals.hpp"
#include "snew.hpp"
#include "utility.hpp"
#include "spell_parser.hpp"


char* fname(char* namelist) {
#define ML 30
	static char holder[ML];
	int i=ML;
	register char* point;

	if( namelist ) {
		for (point = holder; isalpha(*namelist) && i ; i--,namelist++, point++) {
			*point = *namelist;
		}

		*point = '\0';
	}
	else
	{ holder[ 0 ] = 0; }

	return(holder);
}


int split_string(char* str, char* sep, char** argv)
/* str must be writable */
{
	char*        s;
	int        argc=0;
	SetLine(__FILE__,__LINE__);

	s = strtok(str, sep);
	SetLine(__FILE__,__LINE__);
	if ((int)s) {
		SetLine(__FILE__,__LINE__);
		argv[argc++] = s;
		SetLine(__FILE__,__LINE__);
	}
	else {
		SetLine(__FILE__,__LINE__);
		*argv = str;
		SetLine(__FILE__,__LINE__);
		return 1;
	}
	SetLine(__FILE__,__LINE__);

	while( ( s = strtok( NULL, sep ) ) ) {
		SetLine(__FILE__,__LINE__);
		argv[argc++] = s;
	}
	SetLine(__FILE__,__LINE__);
	return argc;
}

/*****************************************************************************
 * Restituisce TRUE se str e` un nome della lista dei nomi in namelist.
 * Il nome in str deve essere completo, ossia niente abbreviazioni.
 * str puo` contenere piu` nomi in tal caso in namelist ci devono essere tutti
 * i nomi in str. Se str finisce con . in namelist ci devono essere solo i
 * nomi in str.
 * */

int isname(const char* str, const char* namelist) {
	char*        argv[100], *xargv[100];
	int        argc, xargc, i,j, exact;
	static char        buf[MAX_INPUT_LENGTH], names[MAX_INPUT_LENGTH], *s;
	char logbuf[255];
	SetStatus("isname");
	SetLine(__FILE__,__LINE__);
	if (!str || !namelist) { return(0); } /* Parm nulli. Torna falso*/
	SetLine(__FILE__,__LINE__);
	if (strlen(str) > MAX_INPUT_LENGTH) {
		mudlog( LOG_SYSERR,"In isname: namelist too long: %s",namelist );
	}
	strncpy(buf, str,MAX_INPUT_LENGTH);
	buf[MAX_INPUT_LENGTH -1] = 0;
	SetLine(__FILE__,__LINE__);
	argc = split_string(buf, "- \t\n\r,", argv);
	SetLine(__FILE__,__LINE__);
	if (strlen(namelist) > MAX_INPUT_LENGTH) {
		mudlog( LOG_SYSERR,"In isname: namelist too long: %s",namelist );
	}
	strncpy(names, namelist,MAX_INPUT_LENGTH);
	names[MAX_INPUT_LENGTH -1] = 0;
	SetLine(__FILE__,__LINE__);
	xargc = split_string(names, "- \t\n\r,", xargv);
	SetLine(__FILE__,__LINE__);
	if (!argc || !argv) { return(0); }
	SetLine(__FILE__,__LINE__);
	s = argv[argc-1];
	SetLine(__FILE__,__LINE__);
	s += strlen(s);
	SetLine(__FILE__,__LINE__);
	if (*(--s) == '.') {
		SetLine(__FILE__,__LINE__);
		exact = 1;
		SetLine(__FILE__,__LINE__);
		*s = 0;
		SetLine(__FILE__,__LINE__);
	}
	else {
		SetLine(__FILE__,__LINE__);
		exact = 0;
	}
	/* the string has now been split into separate words with the '-'
	 replaced by string terminators.  pointers to the beginning of
	 each word are in argv */

	if( exact && argc != xargc )
	{ return 0; }

	for (i=0; i<argc; i++) {
		for (j=0; j<xargc; j++) {
			SetLine(__FILE__,__LINE__);
			if (0==str_cmp(argv[i],xargv[j])) {
				SetLine(__FILE__,__LINE__);
				SetStatus(logbuf,"In");

				/*	    xargv[j] = NULL; Fusse questo? */
				break;
			}
			SetLine(__FILE__,__LINE__);
		}
		if (j>=xargc)
		{ return 0; }
	}
	SetLine(__FILE__,__LINE__);
	return 1;
}

/*****************************************************************************
 * Stessa cosa della procedura precedente solo che in str posso esserci delle
 * abbreviazioni.
 * */

int isname2(const char* str, const char* namelist) {
	char*  argv[100], *xargv[100];
	int   argc, xargc, i,j, exact;
	static char   buf[8000], names[8000], *s;
	SetLine(__FILE__,__LINE__);

	if (!str || !namelist) { return(0); } /* Parm nulli. Torna falso*/
	SetLine(__FILE__,__LINE__);
	strcpy(buf, str);
	SetLine(__FILE__,__LINE__);
	argc = split_string(buf, "- \t\n\r,", argv);
	SetLine(__FILE__,__LINE__);

	strcpy(names, namelist);
	SetLine(__FILE__,__LINE__);
	xargc = split_string(names, "- \t\n\r,", xargv);
	SetLine(__FILE__,__LINE__);

	s = argv[argc-1];
	SetLine(__FILE__,__LINE__);
	s += strlen(s);
	SetLine(__FILE__,__LINE__);
	if( *(--s) == '.' ) {
		SetLine(__FILE__,__LINE__);
		exact = 1;
		SetLine(__FILE__,__LINE__);
		*s = 0;
		SetLine(__FILE__,__LINE__);
	}
	else {
		SetLine(__FILE__,__LINE__);
		exact = 0;
		SetLine(__FILE__,__LINE__);
	}
	/* the string has now been split into separate words with the '-'
	 * replaced by string terminators.  pointers to the beginning of
	 * each word are in argv
	 */
	SetLine(__FILE__,__LINE__);

	if (exact && argc != xargc)
	{ return FALSE; }
	SetLine(__FILE__,__LINE__);

	for (i=0; i < argc; i++) {
		SetLine(__FILE__,__LINE__);
		for( j = 0; j < xargc; j++ ) {
			SetLine(__FILE__,__LINE__);
			if( str_cmp2( argv[i], xargv[j] ) == 0 ) {
				SetLine(__FILE__,__LINE__);
				xargv[j] = NULL;
				SetLine(__FILE__,__LINE__);
				break;
			}
			SetLine(__FILE__,__LINE__);
		}
		SetLine(__FILE__,__LINE__);
		if (j>=xargc)
		{ return FALSE; }
		SetLine(__FILE__,__LINE__);
	}
	SetLine(__FILE__,__LINE__);

	return TRUE;
}

void init_string_block(struct string_block* sb) {
	sb->data = (char*)malloc(sb->size=128);
	*sb->data = '\0';
}

void append_to_string_block(struct string_block* sb, const char* str) {
	int        len;
	len = strlen(sb->data) + strlen(str) + 1;
	if (len > sb->size) {
		if ( len > (sb->size*=2))
		{ sb->size = len; }
		sb->data = (char*)realloc(sb->data, sb->size);
	}
	strcat(sb->data, str);
}

void page_string_block(struct string_block* sb, struct char_data* ch) {
	page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block* sb) {
	free(sb->data);
	sb->data = NULL;
}

#define SECURE_INCR(act,gain) act=(gain>0?MIN(255,act+gain):MAX(0,act+gain))

void affect_modify(struct char_data* ch,byte loc, long mod, long bitv,bool add) {
	int i;

	if (!ch)
	{ return; }
	if (loc == APPLY_SKIP)
	{ return; }

	if (loc == APPLY_IMMUNE) {
		if (add) {
			SET_BIT(ch->immune, mod);
		}
		else {
			REMOVE_BIT(ch->immune, mod);
		}
	}
	else if (loc == APPLY_SUSC) {
		if (add) {
			SET_BIT(ch->susc, mod);
		}
		else {
			REMOVE_BIT(ch->susc, mod);
		}

	}
	else if (loc == APPLY_M_IMMUNE) {
		if (add) {
			SET_BIT(ch->M_immune, mod);
		}
		else {
			REMOVE_BIT(ch->M_immune, mod);
		}
	}
	else if (loc == APPLY_SPELL) {
		if (add) {
			SET_BIT(ch->specials.affected_by, mod);
		}
		else {
			REMOVE_BIT(ch->specials.affected_by, mod);
		}
	}
	else if (loc == APPLY_WEAPON_SPELL) {
		return;
	}
	else if (loc == APPLY_BV2) {
		if (add) {
			SET_BIT(ch->specials.affected_by2, bitv);
		}
		else {
			REMOVE_BIT(ch->specials.affected_by2, bitv);
		}
		return;
	}
	else {
		if (add) {
			SET_BIT(ch->specials.affected_by, bitv);
		}
		else {
			REMOVE_BIT(ch->specials.affected_by, bitv);
			mod = -mod;
		}
	}


	switch(loc) {
	case APPLY_NONE:
	case APPLY_BV2:
		break;

	case APPLY_STR: {
		int nTmpAdd = GET_ADD( ch );
		if( -mod > GET_STR( ch ) )
		{ mod = -GET_STR( ch ); }
		GET_STR( ch ) += mod;
		if( GET_STR( ch ) > 18 && GET_STR(ch) > MaxStrForRace( ch ) ) {
			nTmpAdd += ( GET_STR( ch ) - 18 ) * 10;
		}
		while( nTmpAdd > 100 && GET_STR( ch ) + 1 <= MaxStrForRace( ch ) ) {
			GET_STR( ch ) +=1;
			nTmpAdd -= 100;
		}
		if( nTmpAdd > 100)
		{ nTmpAdd = 100; }
		if( GET_STR( ch ) > MaxStrForRace( ch ) )
		{ GET_STR( ch ) = MaxStrForRace( ch ); }
		GET_ADD( ch ) = (sbyte)nTmpAdd; /* nTmpAdd dovrebbe essere fra 0 e 100 */
		break;
	}

	case APPLY_DEX:
		if( (-mod) > GET_DEX(ch) )
		{ mod = -GET_DEX(ch); }
		GET_DEX(ch) += mod;
		if(GET_DEX(ch) > MaxDexForRace(ch))
		{ GET_DEX(ch)=MaxDexForRace(ch); }
		break;

	case APPLY_INT:
		if( (-mod) > GET_INT(ch) )
		{ mod = -GET_INT(ch); }
		GET_INT(ch) += mod;
		if (GET_INT(ch) > MaxIntForRace(ch))
		{ GET_INT(ch) = MaxIntForRace(ch); }

		break;

	case APPLY_WIS:
		if( (-mod) > GET_WIS(ch) )
		{ mod = -GET_WIS(ch); }
		GET_WIS(ch) += mod;
		if (GET_WIS(ch) > MaxWisForRace(ch))
		{ GET_WIS(ch) = MaxWisForRace(ch); }

		break;

	case APPLY_CON:
		if( (-mod) > GET_CON(ch) )
		{ mod = -GET_CON(ch); }
		GET_CON(ch) += mod;
		if (GET_CON(ch) > MaxConForRace(ch))
		{ GET_CON(ch) =MaxConForRace(ch); }
		break;

	case APPLY_SEX:
		GET_SEX(ch) = (!(ch->player.sex-1))+1;
		break;

	case APPLY_CHR:
		if( (-mod) > GET_CHR(ch) )
		{ mod = -GET_CHR(ch); }
		GET_CHR(ch) += mod;
		if (GET_CHR(ch) > MaxChrForRace(ch))
		{ GET_CHR(ch) = MaxChrForRace(ch); }

		break;

	case APPLY_LEVEL:
		break;

	case APPLY_AGE:
		ch->player.time.birth -= SECS_PER_MUD_YEAR*mod;
		break;

	case APPLY_CHAR_WEIGHT:
		GET_WEIGHT(ch) += mod;
		break;

	case APPLY_CHAR_HEIGHT:
		GET_HEIGHT(ch) += mod;
		break;

	case APPLY_MANA:
		ch->points.max_mana+=mod;
		break;

	case APPLY_HIT:
		ch->points.max_hit+=mod;
		break;

	case APPLY_MOVE:
		ch->points.max_move+=mod;
		break;

	case APPLY_GOLD:
		break;


	case APPLY_EXP:
		break;

	case APPLY_AC:
		GET_AC(ch) += mod;
		break;

	case APPLY_HITROLL:
		GET_HITROLL(ch) += mod;
		break;

	case APPLY_DAMROLL:
		GET_DAMROLL(ch) += mod;
		break;

	/* negatives make saving throws better */

	case APPLY_SAVING_PARA:
		ch->specials.apply_saving_throw[0] += mod;
		break;

	case APPLY_SAVING_ROD:
		ch->specials.apply_saving_throw[1] += mod;
		break;

	case APPLY_SAVING_PETRI:
		ch->specials.apply_saving_throw[2] += mod;
		break;

	case APPLY_SAVING_BREATH:
		ch->specials.apply_saving_throw[3] += mod;
		break;

	case APPLY_SAVING_SPELL:
		ch->specials.apply_saving_throw[4] += mod;
		break;

	case APPLY_SAVE_ALL: {
		for (i=0; i<=4; i++)
		{ ch->specials.apply_saving_throw[i] += mod; }
	}
	break;
	case APPLY_IMMUNE:
		break;
	case APPLY_SUSC:
		break;
	case APPLY_M_IMMUNE:
		break;
	case APPLY_SPELL:
		break;
	case APPLY_HITNDAM:
		GET_HITROLL(ch) += mod;
		GET_DAMROLL(ch) += mod;
		break;
	case APPLY_WEAPON_SPELL:
	case APPLY_EAT_SPELL:
		break;
	case APPLY_BACKSTAB:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_BACKSTAB].learned+ mod) >=100)
		{ ch->skills[SKILL_BACKSTAB].learned=100; }
		else
		{ ch->skills[SKILL_BACKSTAB].learned += mod; }
		break;
	case APPLY_KICK:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_KICK].learned+ mod) >=100)
		{ ch->skills[SKILL_KICK].learned=100; }
		else
		{ ch->skills[SKILL_KICK].learned += mod; }
		break;
	case APPLY_SNEAK:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_SNEAK].learned+ mod) >=100)
		{ ch->skills[SKILL_SNEAK].learned=100; }
		else
		{ ch->skills[SKILL_SNEAK].learned += mod; }
		break;
	case APPLY_HIDE:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_HIDE].learned+ mod) >=100)
		{ ch->skills[SKILL_HIDE].learned=100; }
		else
		{ ch->skills[SKILL_HIDE].learned += mod; }
		break;
	case APPLY_BASH:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_BASH].learned+ mod) >=100)
		{ ch->skills[SKILL_BASH].learned=100; }
		else
		{ ch->skills[SKILL_BASH].learned += mod; }
		break;
	case APPLY_PICK:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_PICK_LOCK].learned+ mod) >=100)
		{ ch->skills[SKILL_PICK_LOCK].learned=100; }
		else
		{ ch->skills[SKILL_PICK_LOCK].learned += mod; }
		break;
	case APPLY_STEAL:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_STEAL].learned+ mod) >=100)
		{ ch->skills[SKILL_STEAL].learned=100; }
		else
		{ ch->skills[SKILL_STEAL].learned += mod; }
		break;
	case APPLY_TRACK:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_HUNT].learned+ mod) >=100)
		{ ch->skills[SKILL_HUNT].learned=100; }
		else
		{ ch->skills[SKILL_HUNT].learned += mod; }
		break;

	case APPLY_SPELLFAIL:
		ch->specials.spellfail += mod;
		break;

	case APPLY_HASTE:
		if (mod > 0) {
			if (WizLock)
			{ fprintf(stderr, "current mult = %f\n", ch->mult_att); }
			ch->mult_att = ch->mult_att * 2.0;
			if (WizLock)
			{ fprintf(stderr, "new mult = %f\n", ch->mult_att); }
		}
		else if (mod < 0) {
			ch->mult_att = ch->mult_att / 2.0;
		}
		break;

	case APPLY_SLOW:
		if (mod > 0)
		{ ch->mult_att /= 2.0; }
		else if (mod < 0)
		{ ch->mult_att *= 2.0; }
		break;

	case APPLY_ATTACKS:
		break;

	case APPLY_FIND_TRAPS:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_FIND_TRAP].learned+ mod) >=100)
		{ ch->skills[SKILL_FIND_TRAP].learned=100; }
		else
		{ ch->skills[SKILL_FIND_TRAP].learned += mod; }
		break;

	case APPLY_RIDE:
		if (!ch->skills)
		{ return; }
		if ((int)(ch->skills[SKILL_RIDE].learned+ mod) >=100)
		{ ch->skills[SKILL_RIDE].learned=100; }
		else
		{ ch->skills[SKILL_RIDE].learned += mod; }
		break;

	case APPLY_RACE_SLAYER:
	case APPLY_ALIGN_SLAYER:
		break;

	case APPLY_MANA_REGEN:
		SECURE_INCR(ch->points.mana_gain,mod);
		break;

	case APPLY_HIT_REGEN:
		SECURE_INCR(ch->points.hit_gain,mod);
		break;

	case APPLY_MOVE_REGEN:
		SECURE_INCR(ch->points.move_gain,mod);
		break;


	/* set hunger/thirst/drunk to MOD value */

	case APPLY_MOD_THIRST:
		ch->specials.conditions[THIRST] = mod;
		break;
	case APPLY_MOD_HUNGER:
		ch->specials.conditions[FULL] =mod;
		break;
	case APPLY_MOD_DRUNK:
		ch->specials.conditions[DRUNK] =mod;
		break;


	/* set these attribs to the mod givin the attrib setting the VALUE,
	 * not mod */
	case APPLY_T_STR:
	case APPLY_T_INT:
	case APPLY_T_DEX:
	case APPLY_T_WIS:
	case APPLY_T_CON:
	case APPLY_T_CHR:
	case APPLY_T_HPS:
	case APPLY_T_MOVE:
	case APPLY_T_MANA:
		break;

	default:

		mudlog( LOG_SYSERR,
				"Unknown apply adjust attempt for %s (handler.c, affect_modify).",
				GET_NAME( ch ) );
		break;

	} /* switch */

}


/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data* ch) {
	struct affected_type* af;
	int i,j;
	if (!GET_NAME(ch)) {
		return;
	}


	if( !HasClass( ch, CLASS_MONK ) ) {
		GET_AC(ch) -= dex_app[ (int)GET_DEX(ch) ].defensive;
	}

	for(i=0; i<MAX_WEAR; i++) {
		if (ch->equipment[i]) {
			for(j=0; j<MAX_OBJ_AFFECT; j++)
				affect_modify(ch, ch->equipment[i]->affected[j].location,
							  ch->equipment[i]->affected[j].modifier,
							  ch->equipment[i]->obj_flags.bitvector, FALSE);
			if( GET_ITEM_TYPE( ch->equipment[ i ] ) == ITEM_ARMOR )
			{ GET_AC(ch) += apply_ac( ch, i ); }
		}
	}

	for(af = ch->affected; af; af=af->next)
		affect_modify(ch, af->location,  af->modifier, af->bitvector,
					  FALSE);

	ch->tmpabilities = ch->abilities;
	if( IS_PC( ch ) ) {
		GET_AC( ch ) = 100;
		if (HasClass(ch, CLASS_MONK)) {
			GET_AC(ch) -= MIN(150, (GET_LEVEL(ch, MONK_LEVEL_IND)*5));
		}
	}

	for(i=0; i<MAX_WEAR; i++) {
		if (ch->equipment[i]) {
			for(j=0; j<MAX_OBJ_AFFECT; j++)
				affect_modify(ch, ch->equipment[i]->affected[j].location,
							  ch->equipment[i]->affected[j].modifier,
							  ch->equipment[i]->obj_flags.bitvector, TRUE);
			if( GET_ITEM_TYPE( ch->equipment[ i ] ) == ITEM_ARMOR )
			{ GET_AC(ch) -= apply_ac( ch, i ); }
		}
	}

	for(af = ch->affected; af; af=af->next)
		affect_modify(ch, af->location, af->modifier,
					  af->bitvector, TRUE);

	if( !HasClass( ch, CLASS_MONK ) ) {
		GET_AC(ch) += dex_app[ (int)GET_DEX(ch) ].defensive;
	}
#if 1
	/* this I think worked, wanna test it without and see what happens */
	/* I think dex items cause this to wack peoples AC out... msw */

	/* Infatti i "dex items" creavano problemi. Ma la soluzione e` nel
	 * rimettere a 0 l'AC quando si tolgono tutti gli affect.
	 * Questo viene fatto in affect_total(). Benem
	 */

	if( GET_AC(ch) < -100 )
	{ GET_AC(ch) = -100; }
	if( GET_AC(ch) > 100 )
	{ GET_AC(ch) = 100; }
#endif

	/* Make certain values are between 0..25, not < 0 and not > 25! */

	i = (IS_NPC(ch) || GetMaxLevel(ch) >= MAESTRO_DEI_CREATORI ? 25 :18);

	if (IS_NPC(ch) || IS_IMMORTAL(ch)) {
		GET_DEX(ch) = MAX(3,MIN(GET_DEX(ch), 25));
		GET_INT(ch) = MAX(3,MIN(GET_INT(ch), 25));
		GET_WIS(ch) = MAX(3,MIN(GET_WIS(ch), 25));
		GET_CON(ch) = MAX(3,MIN(GET_CON(ch), 25));
		GET_STR(ch) = MAX(3,GET_STR(ch));
		GET_CHR(ch) = MAX(3,MIN(GET_CHR(ch), 25));
	}
	else {
		GET_DEX(ch) = MAX(3,MIN(GET_DEX(ch), MaxDexForRace(ch)));
		GET_INT(ch) = MAX(3,MIN(GET_INT(ch), MaxIntForRace(ch)));
		GET_WIS(ch) = MAX(3,MIN(GET_WIS(ch), MaxWisForRace(ch)));
		GET_CON(ch) = MAX(3,MIN(GET_CON(ch), MaxConForRace(ch)));
		GET_STR(ch) = MAX(3,GET_STR(ch));
		GET_CHR(ch) = MAX(3,MIN(GET_CHR(ch), MaxChrForRace(ch)));
	}
	if(IS_NPC(ch) || GetMaxLevel(ch) >= MAESTRO_DEI_CREATORI) {
		GET_STR(ch) = MIN(GET_STR(ch), i);
	}
	else {
		if (GET_STR(ch) > 18) {
			i = GET_ADD(ch) + ((GET_STR(ch)-18)*10);
			GET_ADD(ch) = MIN(i, 100);
			if (GET_STR(ch) > MaxStrForRace(ch))
			{ GET_STR(ch) = MaxStrForRace(ch); }
		}
	}
	/* update regen rates (for age) */
	alter_hit(ch, 0);
	alter_mana(ch, 0);
	alter_move(ch, 0);
}

/* Insert an affect_type in a char_data structure
 * Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data* ch, struct affected_type* af ) {
	struct affected_type* affected_alloc;


	CREATE(affected_alloc, struct affected_type, 1);
	MARK;
	*affected_alloc = *af;
	affected_alloc->next = ch->affected;
	ch->affected = affected_alloc;
	affect_modify(ch, af->location,af->modifier,af->bitvector, TRUE);
	affect_total(ch);
	MARK;
}



/* Remove an affected_type structure from a char (called when duration
						  * reaches zero). Pointer *af must never be NIL! Frees mem and calls
 * affect_location_apply                                                */

void affect_remove( struct char_data* ch, struct affected_type* af ) {
	struct affected_type* hjp;


	if( !ch->affected ) {
		mudlog( LOG_SYSERR, "affect removed from char %s without affect",
				GET_NAME( ch ) );
		return;
	}

	/* Curious to see if af is really non NULL */
	if( !af ) {
		mudlog( LOG_SYSERR, "af is NULL! in affect_remove()");
		return;
	}

	affect_modify( ch, af->location, af->modifier, af->bitvector, FALSE );


	/* remove structure *af from linked list */

	if (ch->affected == af) {
		/* remove head of list */
		ch->affected = af->next;
	}
	else {
		for( hjp = ch->affected; (hjp) && (hjp->next) && (hjp->next != af);
				hjp = hjp->next);

		if (hjp->next != af) {
			mudlog( LOG_SYSERR, "Could not locate affected_type in ch->affected. "
					"(handler.c, affect_remove)" );
			return;
		}
		hjp->next = af->next; /* skip the af element */
	}

	free ( af );
	PushStatus("affect_total");
	affect_total(ch);
	PopStatus();
}

/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data* ch, short skill) {
	struct affected_type* hjp, *pNext;

	for( hjp = ch->affected; hjp; hjp = pNext ) {
		pNext = hjp->next;
		if( hjp->type == skill )
		{ affect_remove( ch, hjp ); }
	}
}
/* Rimuove UNA spell di tipo skill */
void one_affect_from_char( struct char_data* ch, short skill) {
	struct affected_type* hjp, *pNext;

	for( hjp = ch->affected; hjp; hjp = pNext ) {
		pNext = hjp->next;
		if( hjp->type == skill ) {
			affect_remove( ch, hjp );
			return;
		}
	}
	return;
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates
 *  not affected
 * */
int how_many_spell(struct char_data* ch, short skill) {
	struct affected_type* hjp;
	int howmany=0;
	for (hjp = ch->affected; hjp; hjp = hjp->next)
		if ( hjp->type == skill )
		{ howmany++; }

	return( howmany );
}


bool affected_by_spell( struct char_data* ch, short skill ) {
	struct affected_type* hjp;

	for (hjp = ch->affected; hjp; hjp = hjp->next) {
		if ((int) hjp < 1000) {
			mudlog(LOG_SYSERR,"Invalid affected address for %s",GET_NAME(ch));
			continue;
		}
		if ( hjp->type == skill )
		{ return( TRUE ); }
	}
	return( FALSE );
}



void affect_join( struct char_data* ch, struct affected_type* af,
				  bool avg_dur, bool avg_mod ) {
	struct affected_type* hjp, *pNext = NULL;
	bool found = FALSE;

	for( hjp = ch->affected; !found && hjp; hjp = pNext ) {
		pNext = hjp->next;
		if( hjp->type == af->type ) {
			af->duration += hjp->duration;
			if(avg_dur)
			{ af->duration /= 2; }

			af->modifier += hjp->modifier;
			if(avg_mod)
			{ af->modifier /= 2; }

			affect_remove(ch, hjp);
			affect_to_char(ch, af);
			found = TRUE;
		}
	}
	if (!found)
	{ affect_to_char(ch, af); }
}


/* move a player out of a room */
void char_from_room(struct char_data* ch) {
	struct char_data* i;
	struct room_data* rp;

	if( ch->in_room == NOWHERE ) {
		mudlog( LOG_SYSERR, "%s is in room NOWHERE (handler.c, char_from_room)",
				GET_NAME_DESC( ch ) );
		return;
	}
	/* Alar: anticipato il check sulla validit� della stanza */
	rp = real_roomp( ch->in_room );
	if( rp == NULL ) {
		mudlog( LOG_SYSERR, "%s is not in a valid room %ld (char_from_room)",
				GET_NAME_DESC( ch ), ch->in_room );
		return;
	}
	if( ch->equipment[ WEAR_LIGHT ] )
		if( ch->equipment[ WEAR_LIGHT ]->obj_flags.type_flag == ITEM_LIGHT )
			if( ch->equipment[ WEAR_LIGHT ]->obj_flags.value[ 2 ] ) /* Light is ON */
			{ rp->light--; }


	if (ch == rp->people)  /* head of list */
	{ rp->people = ch->next_in_room; }
	else {
		/* locate the previous element */
		for( i = rp->people; i && i->next_in_room != ch; i = i->next_in_room ) ;
		if( i )
		{ i->next_in_room = ch->next_in_room; }
		else
			mudlog( LOG_SYSERR, "%s was not in people list of his room %ld "
					"(char_from_room)", GET_NAME_DESC( ch ),
					ch->in_room );
	}

	ch->in_room = NOWHERE;
	ch->next_in_room = 0;
}


/* place a character in a room */
void char_to_room(struct char_data* ch, long room) {
	struct room_data* rp;

	rp = real_roomp( room );
	if( !rp ) {
		room = 0;
		rp = real_roomp(room);
		if( !rp )
		{ assert( ("Stanza zero inesistente",0) ); }
	}
	ch->next_in_room = rp->people;
	rp->people = ch;
	ch->in_room = room;

	if( ch->equipment[ WEAR_LIGHT ] ) {
		if( ch->equipment[ WEAR_LIGHT ]->obj_flags.type_flag == ITEM_LIGHT ) {
			if( rp->sector_type != SECT_UNDERWATER ) {
				if( ch->equipment[ WEAR_LIGHT ]->obj_flags.value[ 2 ] ) {
					rp->light++; /* Light is ON */
					if( rp->light < 1 )
					{ rp->light = 1; }
				}
			}
			else {
				if( ch->equipment[ WEAR_LIGHT ]->obj_flags.value[ 2 ] > 0 ) {
					send_to_char("Your light source is extinguished instantyl!\n\r", ch);
					ch->equipment[ WEAR_LIGHT ]->obj_flags.value[ 2 ] = 0;
				}
				else {
					rp->light++;
					if( rp->light < 1 )
					{ rp->light = 1; }
				}
			}
		}
	}

	if( IS_SET( TELE_COUNT, rp->tele_mask ) && rp->tele_cnt > 0 ) {
		/* this is a teleport countdown room */
		ch->nTeleCount = rp->tele_cnt;
	}

	if( IS_PC( ch ) ) {
		if( zone_table[ rp->zone ].start == 0 ) {
			mudlog( LOG_CHECK, "Zone inizializzata da %s.", GET_NAME_DESC( ch ) );
			/* start up the zone.  */
			reset_zone(rp->zone);
		}
	}
}


/* give an object to a char   */
void obj_to_char( struct obj_data* object, struct char_data* ch ) {
#if 1
	if (!object) {
		mudlog( LOG_SYSERR, "!object in obj_to_char!");
		return;
	}
	if (!ch) {
		mudlog( LOG_SYSERR, "!ch in obj_to_char for object <%s>, could be bogus "
				"maximum for the obj in the zone file", object->name );
		return;
	}
#endif

	if (!(!object->in_obj && !object->carried_by && !object->equipped_by &&
			object->in_room == NOWHERE)) {
		/*  Probabilmente non e stata chiamata obj_from_char */
		/*  oppure e' stata chiamata da Oload, in questo caso obj_from_char
		    fa crashare il mud! Gaia 2001 */

		mudlog( LOG_SYSERR,
				"Oggetto %s: per darlo a %s lo tolgo al proprietario.",
				object->name,
				GET_NAME(ch));
		obj_from_char(object);

	}


	if (ch->carrying)
	{ object->next_content = ch->carrying; }
	else
	{ object->next_content = 0; }

	ch->carrying = object;
	object->carried_by = ch;
	object->in_room = NOWHERE;
	object->equipped_by = 0;
	object->in_obj = 0;
	IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
	IS_CARRYING_N(ch)++;

}


/* take an object from a char */
void obj_from_char(struct obj_data* object) {
	struct obj_data* tmp;

	if( !object ) {
		mudlog( LOG_SYSERR, "object == NULL in obj_from_char (handler.c).");
		return;
	}


	if( !object->carried_by ) {
		mudlog( LOG_SYSERR, "Object '%s' is not carried by anyone",
				object->short_description );
		return;
	}

	if( !object->carried_by->carrying ) {
		mudlog( LOG_SYSERR,
				"The object->carried_by (of object %s) character is carrying "
				"anything.", object->short_description );
		return;
	}

	if( object->in_obj || object->equipped_by ) {
		mudlog( LOG_SYSERR, "Object %s in more than one place.",
				object->short_description );
		return;
	}

	if (object->carried_by->carrying == object)   /* head of list */
	{ object->carried_by->carrying = object->next_content; }
	else {
		for( tmp = object->carried_by->carrying;
				tmp && (tmp->next_content != object);
				tmp = tmp->next_content); /* locate previous */

		if (!tmp) {
			mudlog( LOG_SYSERR, "Couldn't find object %s on character %s",
					object->short_description, GET_NAME_DESC( object->carried_by ) );
			return;
		}

		tmp->next_content = object->next_content;
	}

	IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
	IS_CARRYING_N(object->carried_by)--;
	object->carried_by = 0;
	object->equipped_by = 0; /* should be unnecessary, but, why risk it */
	object->next_content = 0;
	object->in_obj = 0;
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data* ch, int eq_pos) {
	assert(ch->equipment[eq_pos]);

	if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
	{ return 0; }

	switch (eq_pos) {
	case WEAR_BODY:
		return (3*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 30% */
	case WEAR_HEAD:
		return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
	case WEAR_LEGS:
		return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
	case WEAR_FEET:
		return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
	case WEAR_HANDS:
		return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
	case WEAR_ARMS:
		return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
	case WEAR_SHIELD:
		return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
	case WEAR_BACK:
		return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
	case WEAR_ABOUT:
		return (3*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 30% */
	}
	return 0;
}



void equip_char(struct char_data* ch, struct obj_data* obj, int pos) {
#if 0
	int GoodBlade( struct char_data *ch, int cmd, char* arg,
				   struct obj_data *tobj, int type);
	int NeutralBlade( struct char_data *ch, int cmd, char* arg,
					  struct obj_data *tobj, int type);
#endif

	int j;

	if( pos < 0 || pos > MAX_WEAR ) {
		mudlog( LOG_SYSERR, "pos < 0 || pos > MAX_WEAR (equip_char)" );
		return;
	}
	if( ch->equipment[ pos ] ) {
		mudlog( LOG_SYSERR, "ch->equipment[ %d ] of %s not empty (equip_char)", pos,
				GET_NAME( ch ) );
		return;
	}
	if( obj == NULL ) {
		mudlog( LOG_SYSERR, "obj == NULL in (equip_char)" );
		return;
	}
	assert(!obj->carried_by);
	assert(obj->in_room==NOWHERE) ;

	if( ch->in_room != NOWHERE ) {

		/* no checks on super ego items, they do it already */
		if( obj->item_number >= 0 &&
				( obj_index[obj->item_number].func == EvilBlade ||
				  obj_index[obj->item_number].func == NeutralBlade ||
				  obj_index[obj->item_number].func == GoodBlade ) ) {
			/* do nothing */
		}
		else {
			if( !CheckEgoEquip( ch, obj ) ) {
				return;
			}
		}                         /* end item ego checks */


		if( !CheckGetBarbarianOK(ch,obj)) {
			if (ch->in_room != NOWHERE) {
				obj_to_room(obj, ch->in_room);
				do_save(ch,"",0);
			}
			else
				mudlog( LOG_SYSERR,
						"Ch->in_room = NOWHERE on anti-barb item! (equip_char)");
			return;
		}

		if( ItemAlignClash(ch, obj) && (GetMaxLevel(ch) < MAESTRO_DEI_CREATORI)) {
			if (ch->in_room != NOWHERE) {
				act( "You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0,
					 TO_CHAR);
				act( "$n is zapped by $p and instantly drop it.",
					 FALSE, ch, obj, 0, TO_ROOM);
				obj_to_room(obj, ch->in_room);
#if 0
				/*equip_char puo essere chiamata da do_save... meglio evitare
				 * 	  ricorsioni
				 * */
				do_save(ch,"",0);
#endif
				return;
			}
			else {
				mudlog( LOG_SYSERR,
						"ch->in_room = NOWHERE when zapping eq char (equip_char)");
				assert(0);
			}
		}
	}
	if (IS_AFFECTED(ch, AFF_SNEAK) &&
			IsRestricted(obj, CLASS_THIEF))
	{ affect_from_char(ch, SKILL_SNEAK); }

	ch->equipment[pos] = obj;
	obj->equipped_by = ch;
	obj->eq_pos = pos;
#if EQPESANTE
	IS_CARRYING_W(obj->equipped_by) += GET_OBJ_WEIGHT(obj);
	if (pos == WEAR_BACK) { IS_CARRYING_W(obj->equipped_by) -= GET_OBJ_WEIGHT(obj); } // SALVO controllo se borsa per il peso
#endif
	if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	{ GET_AC(ch) -= apply_ac(ch, pos); }

	for(j=0; j<MAX_OBJ_AFFECT; j++)
		affect_modify(ch, obj->affected[j].location,
					  obj->affected[j].modifier,
					  obj->obj_flags.bitvector, TRUE);

	if (GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
		/* some nifty manuevering for strength */
		if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF))
		{ GiveMinStrToWield(obj, ch); }
	}

	affect_total(ch);
}


int GiveMinStrToWield(struct obj_data* obj, struct char_data* ch) {
	int str=0;

	/*
	 * will have a problem with except. str, that i do not care to solve
	 */

	while( GET_OBJ_WEIGHT( obj ) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	{ GET_STR(ch)++; }

	GET_RSTR( ch ) = GET_STR( ch );

	return(str);

}

struct obj_data* unequip_char(struct char_data* ch, int pos) {
	int j;
	struct obj_data* obj;

	if (pos > MAX_WEAR || pos < 0) {
		mudlog( LOG_SYSERR, "pos > MAX_WEAR || pos < 0 in handler.c unequip");
		return NULL;
	}

	assert(ch->equipment[pos]);

	obj = ch->equipment[pos];

	assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);

	if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	{ GET_AC(ch) += apply_ac(ch, pos); }

#if EQPESANTE
	IS_CARRYING_W(obj->equipped_by) -= GET_OBJ_WEIGHT(obj);
	if (pos == WEAR_BACK) { IS_CARRYING_W(obj->equipped_by) += GET_OBJ_WEIGHT(obj); } // SALVO controllo se borsa per il peso
#else
	IS_CARRYING_W(obj->equipped_by) += GET_OBJ_WEIGHT(obj);
#endif
	ch->equipment[pos] = 0;
	obj->equipped_by = 0;
	obj->eq_pos = -1;

	for(j=0; j<MAX_OBJ_AFFECT; j++)
		affect_modify(ch, obj->affected[j].location,
					  obj->affected[j].modifier,
					  obj->obj_flags.bitvector, FALSE);

	affect_total(ch);

	return(obj);
}


int get_number(char** name) {

	int i;
	char* ppos;
	char number[MAX_INPUT_LENGTH];

	number[0] = 0;

	if( ( ppos = (char*)index(*name, '.') ) && ppos[1] ) {
		*ppos++ = '\0';
		strcpy(number,*name);
		strcpy(*name, ppos);

		for(i=0; *(number+i); i++)
			if (!isdigit(*(number+i)))
			{ return(0); }

		return(atoi(number));
	}

	return(1);
}

/* Search a given list for an object, and return a pointer to that object */
struct obj_data* get_obj_in_list(char* name, struct obj_data* list) {
	struct obj_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;


	if (!(number = get_number(&tmp)))
	{ return(0); }

	for (i = list, j = 1; i && (j <= number); i = i->next_content)
		if (isname(tmp, i->name)) {
			if (j == number)
			{ return(i); }
			j++;
		}

	for (i = list, j = 1; i && (j <= number); i = i->next_content)
		if (isname2(tmp, i->name)) {
			if (j == number)
			{ return(i); }
			j++;
		}

	return(0);
}


/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data* get_obj_in_list_num(int num, struct obj_data* list) {
	struct obj_data* i;

	for (i = list; i; i = i->next_content)
		if (i->item_number == num)
		{ return(i); }

	return(0);
}




/*search the entire world for an object, and return a pointer  */
struct obj_data* get_obj(char* name) {
	struct obj_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	for (i = object_list, j = 1; i && (j <= number); i = i->next)
		if (isname(tmp, i->name)) {
			if( j == number )
			{ return(i); }
			j++;
		}

	for (i = object_list, j = 1; i && (j <= number); i = i->next)
		if (isname2(tmp, i->name)) {
			if (j == number)
			{ return(i); }
			j++;
		}

	return(0);
}

/*search the entire world for an object number, and return a pointer  */
struct obj_data* get_obj_num(int nr) {
	struct obj_data* i;

	for (i = object_list; i; i = i->next)
		if (i->item_number == nr)
		{ return(i); }

	return(0);
}




/* search a room for a char, and return a pointer if found..  */
struct char_data* get_char_room(char* name, int room) {
	struct char_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	for( i = real_roomp(room)->people, j = 1; i && (j <= number);
			i = i->next_in_room ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR, "Invalid char %s in room %d (ADDR: %p, magic: %d).",
					GET_NAME_DESC( i ), room, i, i->nMagicNumber );
		}
		if (isname(tmp, GET_NAME(i))) {
			if( j == number )
			{ return(i); }
			j++;
		}
	}
	for( i = real_roomp(room)->people, j = 1;
			i && (j <= number); i = i->next_in_room ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR, "Invalid char %s in room %d (ADDR: %p, magic: %d).",
					GET_NAME_DESC( i ), room, i, i->nMagicNumber );
		}
		if( isname2( tmp, GET_NAME(i) ) ) {
			if( j == number )
			{ return(i); }
			j++;
		}
	}
	return NULL;
}




/* search all over the world for a char, and return a pointer if found */
struct char_data* get_char(char* name) {
	struct char_data* i;
	int j, number, nPosInList;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	for( i = character_list, j = 1, nPosInList = 0; i && j <= number;
			i = i->next, nPosInList++ ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR,
					"Invalid char %s in character_list (ADDR:%p, magic:%d, "
					"pos:%d on %ld).", GET_NAME_DESC( i ), i, i->nMagicNumber,
					nPosInList, mob_count );
			raw_force_all( "save" );
			assert( 0 );
		}
		if( isname( tmp, GET_NAME(i) ) || isname2( tmp, GET_NAME(i) ) ) {
			if( j == number )
			{ return(i); }
			j++;
		}
	}
	for( i = character_list, j = 1; i && j <= number; i = i->next ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR,
					"Invalid char %s in character_list (ADDR: %p, magic: %d).",
					GET_NAME_DESC( i ), i, i->nMagicNumber );
		}
		if( isname2( tmp, GET_NAME(i) ) ) {
			if( j == number )
			{ return(i); }
			j++;
		}
	}
	return NULL;
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data* get_char_num(int nr) {
	struct char_data* i;
	int nPosInList;

	for( i = character_list, nPosInList = 0; i; i = i->next, nPosInList++ ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR,
					"Invalid char %s in character_list (ADDR:%p, magic:%d, "
					"pos:%d on %ld).", GET_NAME_DESC( i ), i, i->nMagicNumber,
					nPosInList, mob_count );
			raw_force_all( "save" );
			assert( 0 );
		}
		if (i->nr == nr)
		{ return(i); }
	}

	return(0);
}




/* put an object in a room */
void obj_to_room(struct obj_data* object, long room) {

	if (room == -1)
	{ room = 4; }

	if( object == NULL ) {
		mudlog( LOG_SYSERR, "object == NULL in obj_to_room (handler.c)." );
		return;
	}

	assert(!object->equipped_by && object->eq_pos == -1);

	if (object->in_room > NOWHERE) {
		obj_from_room(object);
	}

	object->next_content = real_roomp(room)->contents;
	real_roomp(room)->contents = object;
	object->in_room = room;
	object->carried_by = 0;
	object->equipped_by = 0; /* should be unnecessary */
	if( !IS_SET( real_roomp( room )->room_flags, DEATH ) &&
			IS_SET( real_roomp( room )->room_flags, SAVE_ROOM ) )
	{ save_room(room); }
}

void obj_to_room2(struct obj_data* object, long room) {

	if (room == -1)
	{ room = 4; }

	assert(!object->equipped_by && object->eq_pos == -1);

	if (object->in_room > NOWHERE) {
		obj_from_room(object);
	}

	object->next_content = real_roomp(room)->contents;
	real_roomp(room)->contents = object;
	object->in_room = room;
	object->carried_by = 0;
	object->equipped_by = 0; /* should be unnecessary */
}


/* Take an object from a room */
void obj_from_room(struct obj_data* object) {
	struct obj_data* i;
	struct char_data* ch;

	/* remove object from room */
	if (!object) { return; }
	if( object->in_room <= NOWHERE ) {
		if( object->carried_by || object->equipped_by ) {
			if (object->carried_by) {
				ch=object->carried_by;
				obj_from_char(object);
			}
			else {
				ch=object->equipped_by;
				do_remove(ch,object->name,0);
			}

			mudlog( LOG_SYSERR,
					"Object %s was taken from a char, instead of a room. Sorry for %s",
					object->name,GET_NAME(ch));
		}
		return;
	}

	if( object == real_roomp( object->in_room )->contents ) /* head of list */
	{ real_roomp( object->in_room )->contents = object->next_content; }

	else {   /* locate previous element in list */
		for( i = real_roomp(object->in_room)->contents; i &&
				i->next_content != object; i = i->next_content );

		if( i ) {
			i->next_content = object->next_content;
		}
	}

	if( !IS_SET( real_roomp( object->in_room )->room_flags, DEATH ) &&
			IS_SET( real_roomp( object->in_room )->room_flags, SAVE_ROOM ) )
	{ save_room(object->in_room); }
	object->in_room = NOWHERE;
	object->next_content = 0;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data* obj, struct obj_data* obj_to) {
	struct obj_data* tmp_obj;

	if( obj == NULL || obj_to == NULL ) {
		mudlog( LOG_SYSERR,
				"obj == NULL || obj_to == NULL in obj_to_obj (handler.c)." );
		return;
	}

	obj->next_content = obj_to->contains;
	obj_to->contains = obj;
	obj->in_obj = obj_to;
	if( obj->carried_by != NULL || obj->equipped_by != NULL )
		mudlog( LOG_SYSERR,
				"obj->carried_by != NULL || obj->equipped_by != NULL in "
				"obj_to_obj" );
	/*
	 * (jdb)  hopefully this will fix the object problem
	 */
	obj->carried_by = 0;
	obj->equipped_by = 0;

#if 0
	for( tmp_obj = obj->in_obj; tmp_obj; tmp_obj = tmp_obj->in_obj )
	{ GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj); }
#else

	/* Subtract weight from containers container */
	for( tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj )
	{ GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj); }

	GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

	/* Subtract weight from char that carries the object */
	if( tmp_obj->carried_by )
	{ IS_CARRYING_W( tmp_obj->carried_by ) += GET_OBJ_WEIGHT(obj); }
#endif


	if( obj_to->in_room != NOWHERE &&
			!IS_SET( real_roomp( obj_to->in_room )->room_flags, DEATH ) &&
			IS_SET( real_roomp( obj_to->in_room )->room_flags, SAVE_ROOM ) )
	{ save_room( obj_to->in_room ); }
}


/* remove an object from an object */
void obj_from_obj(struct obj_data* obj) {
	struct obj_data* tmp, *obj_from;

	if( obj->carried_by ) {
		mudlog( LOG_SYSERR, "%s carried by %s in obj_from_obj\n", obj->name,
				obj->carried_by->player.name);
	}
	if (obj->equipped_by) {
		mudlog( LOG_SYSERR, "%s equipped by %s in obj_from_obj\n", obj->name,
				obj->equipped_by->player.name);
	}
	if (obj->in_room != NOWHERE) {
		mudlog( LOG_SYSERR, "%s in room %d in obj_from_obj\n", obj->name,
				obj->in_room);
	}

	assert(!obj->carried_by && !obj->equipped_by && obj->in_room == NOWHERE);

	if (obj->in_obj) {
		obj_from = obj->in_obj;
		if (obj == obj_from->contains)   /* head of list */
		{ obj_from->contains = obj->next_content; }
		else {
			for( tmp = obj_from->contains;
					tmp && ( tmp->next_content != obj );
					tmp = tmp->next_content ); /* locate previous */

			if (!tmp) {
				mudlog( LOG_SYSERR, "Fatal error in object structures." );
				assert(0);
			}

			tmp->next_content = obj->next_content;
		}

		/* Subtract weight from containers container */
		for( tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj ) {
			GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
			if( GET_OBJ_WEIGHT(tmp) < 0 )
			{ GET_OBJ_WEIGHT(tmp) = 0; }
		}

		GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
		if( GET_OBJ_WEIGHT(tmp) < 0 )
		{ GET_OBJ_WEIGHT(tmp) = 0; }

		/* Subtract weight from char that carries the object */
		if( tmp->carried_by ) {
			IS_CARRYING_W( tmp->carried_by ) -= GET_OBJ_WEIGHT(obj);
			if( IS_CARRYING_W( tmp->carried_by ) < 0 )
			{ IS_CARRYING_W( tmp->carried_by ) = 0; }
		}


		obj->in_obj = 0;
		obj->next_content = 0;

		if( obj_from->in_room != NOWHERE &&
				!IS_SET( real_roomp( obj_from->in_room )->room_flags, DEATH ) &&
				IS_SET( real_roomp( obj_from->in_room )->room_flags, SAVE_ROOM ) )
		{ save_room( obj_from->in_room ); }
	}
	else {
		perror("Trying to object from object when in no object.");
		assert(0);
	}

}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data* list, struct char_data* ch) {
	if (list) {
		object_list_new_owner(list->contains, ch);
		object_list_new_owner(list->next_content, ch);
		list->carried_by = ch;
	}
}


/* Extract an object from the world */
void extract_obj(struct obj_data* obj) {
	struct obj_data* temp1, *temp2;
	extern long obj_count;
	int i;

	if( obj == NULL ) {
		mudlog( LOG_SYSERR, "obj == NULL in extract_obj (handler.c)" );
		return;
	}
	if(obj->in_room != NOWHERE)
	{ obj_from_room(obj); }
	else if(obj->carried_by)
	{ obj_from_char(obj); }
	else if (obj->equipped_by) {
		if (obj->eq_pos > -1) {
			/*
			 **  set players equipment slot to 0; that will avoid the garbage items.
			 */
			obj->equipped_by->equipment[ (int)obj->eq_pos ] = 0;

		}
		else {
			mudlog( LOG_SYSERR, "Extract on equipped item %s in slot -1 of %s "
					"in extract_obj (handler.c).",
					obj->name, obj->equipped_by->player.name );

			for( i = 0; i < MAX_WEAR; i++ )
				if( obj->equipped_by->equipment[ i ] == obj )
				{ obj->equipped_by->equipment[ i ] = 0; }
		}
	}
	else if(obj->in_obj) {
		temp1 = obj->in_obj;
		if(temp1->contains == obj)   /* head of list */
		{ temp1->contains = obj->next_content; }
		else {
			for( temp2 = temp1->contains ; temp2 && (temp2->next_content != obj);
					temp2 = temp2->next_content );

			if(temp2) {
				temp2->next_content = obj->next_content;
			}
		}
	}

	for( ; obj->contains; extract_obj( obj->contains ) );
	/* leaves nothing ! */

	if( object_list == obj )       /* head of list */
	{ object_list = obj->next; }
	else {
		for( temp1 = object_list;
				temp1 && (temp1->next != obj);
				temp1 = temp1->next);

		if( temp1 ) {
			temp1->next = obj->next;
		}
		else {
			mudlog( LOG_SYSERR,
					"Couldn't find object %s in object list in extract_obj "
					"(handler.c).", obj->name );
			obj_count = 0;
			for( i = 0; i < top_of_objt; i++ )
			{ obj_index[ i ].number = 0; }
			for( temp1 = object_list; temp1; temp1 = temp1->next ) {
				if( temp1->item_number >= 0 && temp1->item_number < top_of_objt ) {
					(obj_index[ temp1->item_number ].number)++;
					obj_count++;
				}
			}
			free_obj(obj);
			return;
		}
	}

	if( obj->item_number >= 0 && obj->item_number < top_of_objt ) {
		(obj_index[obj->item_number].number)--;
		obj_count--;
	}
	free_obj(obj);

}

void update_object( struct obj_data* obj, int use) {

	if (obj->obj_flags.timer > 0)        { obj->obj_flags.timer -= use; }
	if (obj->contains) { update_object(obj->contains, use); }
	if (obj->next_content)
		if (obj->next_content != obj)
		{ update_object(obj->next_content, use); }
}

void update_char_objects( struct char_data* ch ) {

	int i;

	if (ch->equipment[WEAR_LIGHT])
		if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
			if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
			{ (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--; }

	for(i = 0; i < MAX_WEAR; i++)
		if (ch->equipment[i])
		{ update_object(ch->equipment[i],2); }

	if (ch->carrying)
	{ update_object(ch->carrying,1); }
}


void extract_char(struct char_data* ch) {
	extract_char_smarter(ch, NOWHERE);
}

void CheckCharList() {
	int nPosInList;
	struct char_data* k;

	for( k = character_list, nPosInList = 0; k; k = k->next, nPosInList++ ) {
		if( k->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR,
					"Invalid char %s in character_list. ADDR:%p, magic:%d, pos:%d "
					"on %ld (ChakCHarList - handler.c).", GET_NAME_DESC( k ), k,
					k->nMagicNumber, nPosInList, mob_count );
		}
	}
}

/* Extract a ch completely from the world, and leave his stuff behind */

void extract_char_smarter(struct char_data* ch, long save_room) {
	struct obj_data* i;
	struct char_data* k, *next_char;
	struct descriptor_data* t_desc;
	int l, was_in, j, w;

	extern long mob_count;
	extern struct char_data* combat_list;

#ifndef NOEVENTS
	mudlog(LOG_SILENT,"Cancelling EVENTS in extract_char");
	mudlog(LOG_SILENT,"Eventi per %s. Eventi %d,%d,%d",
		   GET_NAME(ch),
		   GET_POINTS_EVENT(ch,0),
		   GET_POINTS_EVENT(ch,1),
		   GET_POINTS_EVENT(ch,2));
	/* cancel point updates */
	for (w = 0; w < 3; w++)
		if (GET_POINTS_EVENT(ch, w)) {
			mudlog(LOG_SILENT,"Cancellazione evento %d. Stato: %d",
				   w,
				   GET_POINTS_EVENT(ch,w));
			event_cancel(GET_POINTS_EVENT(ch, w));
			GET_POINTS_EVENT(ch, w) = NULL;
		}
	mudlog(LOG_SILENT,"Cancelled EVENTS in extract_char");
#endif
	if( ch == NULL ) {
		mudlog( LOG_SYSERR, "ch == NULL in extract_char_smarter (handler.c)" );
		return;
	}
	else if( ch->nMagicNumber != CHAR_VALID_MAGIC ) {
		mudlog( LOG_SYSERR, "Invalid ch pointer in extract_char_smarter "
				"(handler.c)" );
		return;
	}


	mudlog( LOG_CHECK | LOG_SILENT, "Extracting char %s (ADDR: %p, magic %d)",
			GET_NAME_DESC( ch ), ch, ch->nMagicNumber );

	if( !IS_NPC( ch ) && !ch->desc ) {
		for( t_desc = descriptor_list; t_desc; t_desc = t_desc->next )
			if( t_desc->original == ch )
			{ force_return( t_desc->character, "", 0 ); }
	}

	if( ch->in_room == NOWHERE ) {
		/* problem from linkdeath */
		mudlog( LOG_SYSERR, "%s is in NOWHERE (extract_char)",
				GET_NAME_DESC( ch ) );
		char_to_room(ch, 4);  /* 4 == all purpose store */
	}

	if( ch->followers || ch->master )
	{ die_follower( ch ); }

	if( ch->desc ) {
		/* Forget snooping */
		if( ch->desc->snoop.snooping && ch->desc->snoop.snooping->desc )
		{ ch->desc->snoop.snooping->desc->snoop.snoop_by = 0; }

		if( ch->desc->snoop.snoop_by ) {
			send_to_char( "La tua vittima non e` piu` fra noi.\n\r",
						  ch->desc->snoop.snoop_by );
			if( ch->desc->snoop.snoop_by->desc )
			{ ch->desc->snoop.snoop_by->desc->snoop.snooping = 0; }
		}

		ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
	}

	if( ch->carrying ) {
		/* transfer ch's objects to room */
		if( !IS_IMMORTAL( ch ) ) {
			while( ch->carrying ) {
				i = ch->carrying;
				obj_from_char( i );
				obj_to_room( i, ch->in_room );
				check_falling_obj( i, ch->in_room );
			}
		}
		else {
			send_to_char( "Hai lasciato della roba, qui. Ci penso io a "
						  "sbarazzarmene.\n\r", ch );
			/* equipment too */
			for (j=0; j<MAX_WEAR; j++) {
				if( ch->equipment[ j ] )
				{ obj_to_char( unequip_char( ch, j ), ch ); }
			}
			while( ch->carrying ) {
				i = ch->carrying;
				obj_from_char( i );
				extract_obj( i );
			}
		}
	}

	if( ch->specials.fighting )
	{ stop_fighting( ch ); }

	for( k = combat_list; k ; k = next_char ) {
		next_char = k->next_fighting;
		if( k->specials.fighting == ch )
		{ stop_fighting( k ); }
	}

	if( MOUNTED( ch ) )
	{ Dismount( ch, MOUNTED( ch ), POSITION_STANDING ); }

	if( RIDDEN( ch ) )
	{ Dismount( RIDDEN( ch ), ch, POSITION_STANDING ); }
	/* Must remove from room before removing the equipment! )++;
	*/
	was_in = ch->in_room;

	char_from_room( ch );

	/* clear equipment_list */
	for( l = 0; l < MAX_WEAR; l++ )
		if( ch->equipment[ l ] )
		{ obj_to_room( unequip_char( ch, l ), was_in ); }


	if (IS_NPC(ch)) {
		for( k = character_list; k; k = k->next ) {
			if( k->specials.hunting )
				if( k->specials.hunting == ch )
				{ k->specials.hunting = 0; }

			if( Hates( k, ch ) )
			{ RemHated( k, ch ); }

			if( Fears( k, ch ) )
			{ RemFeared( k, ch ); }

			if( k->orig == ch )
			{ k->orig = 0; }
		}
	}
	else {
		for( k = character_list; k; k = k->next ) {
			if( k->specials.hunting )
				if( k->specials.hunting == ch )
				{ k->specials.hunting = 0; }

			if( Hates( k, ch ) )
			{ ZeroHatred( k, ch ); }

			if( Fears( k, ch ) )
			{ ZeroFeared( k, ch ); }

			if( k->orig == ch )
			{ k->orig = 0; }
		}
	}

	CheckCharList();

	/* pull the char from the list */
	if( ch == character_list )
	{ character_list = ch->next; }
	else {
		int nPosInList;
		for( k = character_list, nPosInList = 0; k && k->next != ch;
				k = k->next, nPosInList++ ) {
			if( k->nMagicNumber != CHAR_VALID_MAGIC ) {
				mudlog( LOG_SYSERR,
						"Invalid char %s in character_list. ADDR:%p, magic:%d, pos:%d "
						"on %ld (extract_char - handler.c).", GET_NAME_DESC( k ), k,
						k->nMagicNumber, nPosInList, mob_count );
				raw_force_all( "save" );
				assert( k->nMagicNumber == CHAR_VALID_MAGIC );
			}
		}
		if( k )
		{ k->next = ch->next; }
		else {
			mudlog( LOG_SYSERR, "Character %s not found in character_list in "
					"extract_char (handler.c).", GET_NAME_DESC( ch ) );
			raw_force_all( "save" );
			assert( 0 );
		}
	}

	if( ch->specials.group_name )
	{ free( ch->specials.group_name ); }
	ch->specials.group_name = NULL;

	GET_AC( ch ) = 100;

	if( ch->desc ) {
		if( ch->desc->original )
		{ do_return( ch, "", 0 ); }

		save_char( ch, save_room, 0 );
	}


	t_desc = ch->desc;

	if( ch->term ) {
		ScreenOff( ch );
		ch->term = 0;
	}

	if( IS_NPC( ch ) ) {
		if( ch->nr > -1 ) /* if mobile */
		{ mob_index[ ch->nr ].number--; }
		FreeHates( ch );
		FreeFears( ch );
		mob_count--;
		free_char( ch );
	}

	if( t_desc ) {
		t_desc->connected = CON_SLCT;
		SEND_TO_Q( MENU, t_desc );
	}
}


/************************************************************************
* Here follows high-level versions of some earlier routines, ie functionst
* which incorporate the actual player-data.
*********************************************************************** */

struct char_data* get_char_near_room_vis(struct char_data* ch, char* name, long next_room) {
	long store_room;
	struct char_data* i;

	store_room = ch->in_room;
	ch->in_room = next_room;
	i= get_char_room_vis(ch,name);
	ch->in_room = store_room;

	return(i);
}

struct char_data* get_char_room_vis(struct char_data* ch, char* name) {
	struct char_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	for( i = real_roomp(ch->in_room)->people, j = 1;
			i && j <= number; i = i->next_in_room ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR, "Invalid char %s in room %ld (ADDR: %p, magic: %d).",
					GET_NAME_DESC( ch ), ch->in_room, ch, ch->nMagicNumber );
		}
		if( isname( tmp, GET_NAME(i) ) ) {
			if (CAN_SEE(ch, i)) {
				if( j == number )
				{ return(i); }
				j++;
			}
		}
	}
	for( i = real_roomp(ch->in_room)->people, j = 1;
			i && j <= number; i = i->next_in_room ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR, "Invalid char %s in room %ld (ADDR: %p, magic: %d).",
					GET_NAME_DESC( ch ), ch->in_room, ch, ch->nMagicNumber );
		}
		if (isname2(tmp, GET_NAME(i))) {
			if (CAN_SEE(ch, i)) {
				if (j == number)
				{ return(i); }
				j++;
			}
		}
	}

	return(0);
}


/* get a character from anywhere in the world, doesn't care much about
   being in the same room... */
struct char_data* get_char_vis_world(struct char_data* ch, char* name,int* count) {
	struct char_data* i;
	int j, number, nPosInList;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if( !( number = get_number( &tmp ) ) )
	{ return NULL; }

	j = count ? *count : 1;
	for( i = character_list, nPosInList = 0; i && j <= number;
			i = i->next, nPosInList++ ) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR,
					"Invalid char %s in character_list (ADDR:%p, magic:%d, "
					"pos:%d on %ld).", GET_NAME_DESC( i ), i, i->nMagicNumber,
					nPosInList, mob_count );
			raw_force_all( "save" );
			assert( 0 );
		}
		if (isname(tmp, GET_NAME(i))) {
			if (CAN_SEE(ch, i)) {
				if (j == number)
				{ return(i); }
				j++;
			}
		}
	}
	j = count ? *count : 1;
	for (i = character_list; i && j <= number; i = i->next) {
		if( i->nMagicNumber != CHAR_VALID_MAGIC ) {
			mudlog( LOG_SYSERR,
					"Invalid char %s in character_list (ADDR: %p, magic: %d).",
					GET_NAME_DESC( i ), i, i->nMagicNumber );
		}
		if (isname2(tmp, GET_NAME(i))) {
			if (CAN_SEE(ch, i)) {
				if (j == number)
				{ return(i); }
				j++;
			}
		}
	}
	if( count )
	{ *count = j; }
	return NULL;
}


struct char_data* get_char_vis( struct char_data* ch, char* name ) {
	struct char_data* i;

	/* check location */
	if( ( i = get_char_room_vis( ch, name ) ) )
	{ return(i); }

	return get_char_vis_world( ch,name, NULL );
}



struct obj_data* get_obj_in_list_vis(struct char_data* ch, char* name,struct obj_data* list) {
	struct obj_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	for (i = list, j = 1; i && (j <= number); i = i->next_content)
		if (isname(tmp, i->name))
			if (CAN_SEE_OBJ(ch, i)) {
				if (j == number)
				{ return(i); }
				j++;
			}

	for (i = list, j = 1; i && (j <= number); i = i->next_content)
		if (isname2(tmp, i->name))
			if (CAN_SEE_OBJ(ch, i)) {
				if (j == number)
				{ return(i); }
				j++;
			}

	return(0);
}

struct obj_data* get_obj_vis_world(struct char_data* ch, char* name,
								   int* count) {
	struct obj_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	j = count ? *count : 1;

	/* ok.. no luck yet. scan the entire obj list   */
	for (i = object_list; i && (j <= number); i = i->next)
		if (isname(tmp, i->name))
			if (CAN_SEE_OBJ(ch, i)) {
				if (j == number)
				{ return(i); }
				j++;
			}

	j = count ? *count : 1;

	/* ok.. no luck yet. scan the entire obj list   */
	for (i = object_list; i && (j <= number); i = i->next)
		if (isname2(tmp, i->name))
			if( CAN_SEE_OBJ( ch, i ) ) {
				if (j == number)
				{ return(i); }
				j++;
			}

	if (count)
	{ *count = j; }

	return(0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data* get_obj_vis( struct char_data* ch, char* name ) {
	struct obj_data* i;

	/* scan items carried */
	if( ( i = get_obj_in_list_vis( ch, name, ch->carrying ) ) )
	{ return(i); }

	/* scan room */
	if( ( i = get_obj_in_list_vis( ch, name,
								   real_roomp( ch->in_room)->contents ) ) )
	{ return(i); }

	return get_obj_vis_world(ch, name, NULL);
}

struct obj_data* get_obj_vis_accessible(struct char_data* ch, char* name) {
	struct obj_data* i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char* tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
	{ return(0); }

	/* scan items carried */
	for (i = ch->carrying, j=1; i && j<=number; i = i->next_content)
		if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
			if (j == number)
			{ return(i); }
			else
			{ j++; }

	for( i = real_roomp(ch->in_room)->contents; i && j<=number;
			i = i->next_content )
		if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
			if (j==number)
			{ return(i); }
			else
			{ j++; }

	/* scan items carried */
	for (i = ch->carrying, j=1; i && j<=number; i = i->next_content)
		if (isname2(tmp, i->name) && CAN_SEE_OBJ(ch, i))
			if (j == number)
			{ return(i); }
			else
			{ j++; }

	for( i = real_roomp(ch->in_room)->contents; i && j<=number;
			i = i->next_content )
		if (isname2(tmp, i->name) && CAN_SEE_OBJ(ch, i))
			if (j==number)
			{ return(i); }
			else
			{ j++; }

	return 0;
}




struct obj_data* create_money( int amount ) {
	struct obj_data* obj;
	struct extra_descr_data* new_descr;
	char buf[80];

	/*  char *strdup(char *str); */

	if( amount < 0 ) {
		LPUSH;
		mudlog( LOG_SYSERR, "Try to create negative money. Changed to 1");
		amount=1;
	}

	CREATE( obj, struct obj_data, 1 );
	clear_object( obj );

	CREATE( new_descr, struct extra_descr_data, 1 );
	new_descr->nMagicNumber = EXDESC_VALID_MAGIC;

	if( amount == 1 ) {
		obj->name = strdup( "coin gold" );
		obj->short_description = strdup( "a gold coin" );
		obj->description = strdup( "One miserable gold coin." );

		new_descr->keyword = strdup( "coin gold" );
		new_descr->description = strdup( "One miserable gold coin." );
	}
	else {
		obj->name = strdup( "coins gold" );
		obj->short_description = strdup( "gold coins" );
		obj->description = strdup( "A pile of gold coins." );

		new_descr->keyword = strdup( "coins gold" );
		if( amount < 10 ) {
			sprintf( buf, "There is %d coins.", amount );
			new_descr->description = strdup(buf);
		}
		else if( amount < 100 ) {
			sprintf( buf,"There is about %d coins", 10 * ( amount / 10 ) );
			new_descr->description = strdup(buf);
		}
		else if( amount < 1000 ) {
			sprintf( buf,"It looks like something round %d coins",100*(amount/100));
			new_descr->description = strdup(buf);
		}
		else if( amount < 100000 ) {
			sprintf( buf, "You guess there is %d coins",
					 1000*((amount/1000)+ number(0,(amount/1000))));
			new_descr->description = strdup(buf);
		}
		else
		{ new_descr->description = strdup( "There is A LOT of coins" ); }
	}

	new_descr->next = 0;
	obj->ex_description = new_descr;

	obj->obj_flags.type_flag = ITEM_MONEY;
	obj->obj_flags.wear_flags = ITEM_TAKE;
	obj->obj_flags.value[0] = amount;
	obj->obj_flags.cost = amount;
	obj->item_number = -1;

	obj->next = object_list;
	object_list = obj;

	return( obj );
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(const char* arg, int bitvector, struct char_data* ch,struct char_data** tar_ch, struct obj_data** tar_obj) {
	static char* ignore[] = {
		"the",
		"in",
		"on",
		"at",
		"\n"
	};

	int i;
	char name[256];
	bool found;

	found = FALSE;

	/* Eliminate spaces and "ignore" words */
	while( *arg && !found ) {
		for(; *arg == ' '; arg++)   ;

		for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)   ;
		name[i] = 0;
		arg+=i;
		if( search_block( name, ignore, TRUE ) > -1 )
		{ found = TRUE; }
	}

	if( !name[ 0 ] )
	{ return( 0 ); }

	*tar_ch  = 0;
	*tar_obj = 0;

	if( IS_SET( bitvector, FIND_CHAR_ROOM ) ) {
		/* Find person in room */
		if( ( *tar_ch = get_char_room_vis(ch, name) ) )
		{ return(FIND_CHAR_ROOM); }
	}

	if( IS_SET( bitvector, FIND_CHAR_WORLD ) ) {
		if( ( *tar_ch = get_char_vis( ch, name ) ) )
		{ return( FIND_CHAR_WORLD ); }
	}

	if( IS_SET( bitvector, FIND_OBJ_EQUIP ) ) {
		for( found = FALSE, i = 0; i < MAX_WEAR && !found; i++ ) {
			if( ch->equipment[i] && str_cmp( name, ch->equipment[i]->name ) == 0 ) {
				*tar_obj = ch->equipment[i];
				found = TRUE;
			}
		}
		if (found)
		{ return(FIND_OBJ_EQUIP); }
	}

	if (IS_SET(bitvector, FIND_OBJ_INV)) {
		if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
			if( ( *tar_obj = get_obj_vis_accessible( ch, name ) ) )
			{ return(FIND_OBJ_INV); }
		}
		else {
			if( ( *tar_obj = get_obj_in_list_vis( ch, name, ch->carrying ) ) )
			{ return(FIND_OBJ_INV); }
		}
	}

	if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
		if( ( *tar_obj = get_obj_in_list_vis( ch, name,
											  real_roomp(ch->in_room)->contents ) ) )
		{ return(FIND_OBJ_ROOM); }
	}

	if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
		if( ( *tar_obj = get_obj_vis( ch, name ) ) )
		{ return(FIND_OBJ_WORLD); }
	}

	return(0);
}

void AddAffects( struct char_data* ch, struct obj_data* o) {
	int i;

	for (i=0; i<MAX_OBJ_AFFECT; i++) {
		if (o->affected[i].location != APPLY_NONE) {
			affect_modify(ch, o->affected[i].location,
						  o->affected[i].modifier,
						  o->obj_flags.bitvector, TRUE);
		}
		else {
			return;
		}
	}
}
