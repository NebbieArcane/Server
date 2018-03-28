/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD */
/* $Id: magic3.c,v 1.4 2002/03/14 21:48:56 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <ctime>
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
#include "magic3.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.off.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magicutils.hpp"
#include "modify.hpp"
#include "regen.hpp"
#include "spell_parser.hpp"

namespace Alarmud {

#define STATE(d) ((d)->connected)
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

/* druid spells */

void spell_tree_travel(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if (!affected_by_spell(ch, SPELL_TREE_TRAVEL)) {
		af.type      = SPELL_TREE_TRAVEL;

		af.duration  = 24;
		af.modifier  = -5;
		af.location  = APPLY_AC;
		af.bitvector = AFF_TREE_TRAVEL;
		affect_to_char(victim, &af);

		send_to_char("Evochi il potere di viaggiare attraverso gli alberi!\n\r", victim);
	}
	else {
		send_to_char("Non succede nulla di nuovo.\n\r", ch);
	}

}

void spell_transport_via_plant(byte level, struct char_data* ch,
							   struct char_data* victim, struct obj_data* obj) {

	/*struct room_data *rp;
	  struct obj_data *o;

	rp = real_roomp(ch->in_room);
	for (o = rp->contents; o; o = o->next_content) {
	  if (ITEM_TYPE(o) == ITEM_TREE)
	    break;
	}

	le righe in commento cercavano l'albero di partenza */

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r", ch);
		return;
	}

	if (ITEM_TYPE(obj) != ITEM_TREE) {
		send_to_char("Non e' un albero!\n\r", ch);
		return;
	}

	if (obj->in_room < 0) {
		send_to_char("Non trovi quell'albero da nessuna parte\n\r", ch);
		return;
	}

	if (!real_roomp(obj->in_room)) {
		send_to_char("Non trovi quell'albero da nessuna parte\n\r", ch);
		return;
	}

	act("$c0010$n tocca il suolo per percepire $p, e viene assorbito dalla terra!", FALSE, ch, obj, 0, TO_ROOM);
	act("$c0010Percepisci $p, e come acqua ti lasci guidare dalla terra alle sue radici.", FALSE, ch, obj, 0, TO_CHAR);
	char_from_room(ch);
	char_to_room(ch, obj->in_room);
	act("$c0010$p si squote leggermente, e $n appare magicamente dal suo interno!", FALSE, ch, obj, 0, TO_ROOM);
	act("$c0010Vieni trasportat$b istantaneamente da $p!", FALSE, ch, obj, 0, TO_CHAR);
	do_look(ch, "\0", 15);

}

void spell_speak_with_plants(byte level, struct char_data* ch,
							 struct char_data* victim, struct obj_data* obj) {
	char buffer[128];

	assert(ch && obj);

	if (ITEM_TYPE(obj) != ITEM_TREE) {
		send_to_char("Mi spiace ma non puoi parlare con quella cosa..\n\r", ch);
		return;
	}

	sprintf(buffer, "$c0013%s ti dice 'Ciao $n, come va' oggi?'",
			fname(obj->name));
	act(buffer, FALSE, ch, obj, 0, TO_CHAR);
	act("$p si squote leggermente.", FALSE, ch, obj, 0, TO_ROOM);

}

#define TREE 6110

void spell_changestaff(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	struct obj_data* s;
	struct char_data* t;

	/* player must be holding staff at the time */

	if (!ch->equipment[HOLD]) {
		send_to_char("Devi avere in mano un bastone!\n\r", ch);
		return;
	}

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char("Non puoi lanciare l'incantesimo al chiuso!\n\r",ch);
		return;
	}

	if( ( s = unequip_char( ch, HOLD ) ) == NULL )
	{ return; }

	if (ITEM_TYPE(s) != ITEM_STAFF) {
		act("$p non e' sufficiente a colmpletare l'incantesimo",
			FALSE, ch, s, 0, TO_CHAR);
		extract_obj(s);
		return;
	}

	if (!s->obj_flags.value[2]) {
		act("$p non e' abbastanza potente per completare l'incantesimo",
			FALSE, ch, s, 0, TO_CHAR);
		extract_obj(s);
		return;
	}


	act("$p svanisce in una vampata di fuoco!", FALSE, ch, s, 0, TO_ROOM);
	act("$p svanisce in una vampata di fuoco!", FALSE, ch, s, 0, TO_CHAR);

	t = read_mobile(TREE, VIRTUAL);
	char_to_room(t, ch->in_room);
	GET_EXP(t)=0;

	act("$n spunta davanti a te!", FALSE, t, 0, 0, TO_ROOM);

	if(too_many_followers(ch)) {
		act("$N da un'occhiata alla quantita' di gente che ti segue e si rifuta di aggregarsi!",
			TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
		act("$N da un'occhiata alla quantita' di gente che segue $n e si rifiuta di aggregarsi!",
			TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
	}
	else {

		af.type      = SPELL_CHARM_PERSON;

		if (IS_PC(ch) || ch->master) {
			af.duration  = follow_time(ch);

			af.duration += s->obj_flags.value[2]; /* num charges */

			af.modifier  = 0;
			af.location  = 0;
			af.bitvector = AFF_CHARM;
			affect_to_char(t, &af);
		}
		else {
			SET_BIT(t->specials.affected_by, AFF_CHARM);
		}
		add_follower(t, ch);

		extract_obj(s);
	}
}

/* mage spells */
void spell_pword_kill(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	int max = 80;

	max += level;
	max += level/2;

	if (GET_MAX_HIT(victim) <= max || GetMaxLevel(ch) > 53 ) {
		damage(ch, victim, GET_MAX_HIT(victim)*12, SPELL_PWORD_KILL, 5);
	}
	else {
		send_to_char("E' troppo potente per morire con una parola.\n\r", ch);
	}

}

void spell_pword_blind(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {

	if (GET_MAX_HIT(victim) <= 100 || GetMaxLevel(ch) > 53) {
		SET_BIT(victim->specials.affected_by, AFF_BLIND);
	}
	else {
		send_to_char("E' troppo potente per perdere la vista con una parola.\n\r", ch);
	}

}


void spell_chain_lightn(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int lev = level;
	struct char_data* t, *next;
	/* Non e' piu' obbligatorio il target.
	 * Se presente, il target subisce il primo attacco
	 * altrimenti, si comporta come una normale area spell
	 *
	 * victim = levd6 damage
	 */
	if (victim)
		if( damage(ch, victim, dice(lev,6), SPELL_LIGHTNING_BOLT, 5) != SubjectDead )
		{ lev--; }

	for (t = real_roomp(ch->in_room)->people; t; t=next) {
		next = t->next_in_room;
		if (!in_group(ch, t) && t != victim && !IS_IMMORTAL(t)) {
			damage(ch, t, dice(lev,6), SPELL_LIGHTNING_BOLT, 5);
			lev--;
		}
	}
}

void spell_scare(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	int i=1;
	if (level - GetMaxLevel(victim) >= 5) {
		level/=2;
		for (i=1; i<level; i++)
		{ do_flee(victim, "\0", 0); }
	}
}

void spell_haste(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if (affected_by_spell(victim, SPELL_HASTE)) {
		act("$N si muove gia piu' velocemente!",FALSE,ch,0,victim,TO_CHAR);
		return;
	}

	if (IS_NPC(victim)) {
		send_to_char("Non sembra funzionare\n", ch);
		return;
	}

	if (IS_IMMUNE(victim, IMM_HOLD)) {
		act("$N ignora l'incantesimo.", FALSE, ch, 0, victim, TO_CHAR);
		act("$n ha appena provato ad aumentare la tua velocita', ma non ci fai caso.", FALSE, ch, 0,
			victim, TO_VICT);
		if (!in_group(ch, victim)) {
			if (!IS_PC(ch))
			{ hit(victim, ch, TYPE_UNDEFINED); }
		}
		return;
	}


	af.type      = SPELL_HASTE;
    
    if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER)) {
	af.duration  = 4;
    } else {
        af.duration  = 1;
    }
    
	af.modifier  = 1;
	af.location  = APPLY_HASTE;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	send_to_char("Ti senti veloce.... e nulla piu'!\n\r", victim);
	send_to_char("Perdi conoscenza\n\r",victim);
	GET_POS(victim) = POSITION_STUNNED;

	if (!in_group(ch, victim)) {
		if (!IS_PC(ch))
		{ hit(victim, ch, TYPE_UNDEFINED); }
	}

}

void spell_slow(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if (affected_by_spell(victim, SPELL_SLOW)) {
		act("$N e' gia' rallentat$b.",FALSE, ch,0,victim,TO_CHAR);
		return;
	}

	if (IS_IMMUNE(victim, IMM_HOLD)) {
		act("$N sembra ignorare l'incantesimo", FALSE, ch, 0, victim, TO_CHAR);
		act("$n ha appena cercato di rallentarti, ma non ci fai caso.", FALSE, ch, 0,
			victim, TO_VICT);
		if (!in_group(ch, victim)) {
			if (!IS_PC(ch))
			{ hit(victim, ch, TYPE_UNDEFINED); }
		}
		return;
	}

	af.type      = SPELL_SLOW;
	af.duration  = level;
	af.modifier  = -1;
	af.location  = APPLY_HASTE;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	send_to_char("Ti muovi piu' lentamente!\n\r", victim);

	if (!IS_PC(victim) && !IS_SET(victim->specials.act,ACT_POLYSELF))
	{ hit(victim, ch, TYPE_UNDEFINED); }

}

#define KITTEN  3090
#define PUPPY   3091
#define BEAGLE  3092
#define ROTT    3093
#define WOLF    3094

void spell_familiar(byte level, struct char_data* ch,
					struct char_data** victim, struct obj_data* obj) {

	struct affected_type af;
	struct char_data* f;

	if (affected_by_spell(ch, SPELL_FAMILIAR)) {
		send_to_char("Non puoi avere piu' di un animale domestico al giorno\n\r",ch);
		return;
	}

	/*
	  depending on the level, one of the pet shop kids.
	  */

	if (level < 2)
	{ f = read_mobile(KITTEN, VIRTUAL); }
	else if (level < 4)
	{ f = read_mobile(PUPPY, VIRTUAL); }
	else if (level < 6)
	{ f = read_mobile(BEAGLE, VIRTUAL); }
	else if (level < 8)
	{ f = read_mobile(ROTT, VIRTUAL); }
	else
	{ f = read_mobile(WOLF, VIRTUAL); }

	char_to_room(f, ch->in_room);


	af.type      = SPELL_FAMILIAR;
	af.duration  = 24;
	af.modifier  = -1;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	act("$n appare con un lampo di luce!\n\r", FALSE, f, 0, 0, TO_ROOM);

	SET_BIT(f->specials.affected_by, AFF_CHARM);
	GET_EXP(f) = 0;
	add_follower(f, ch);
	IS_CARRYING_W(f) = 0;
	IS_CARRYING_N(f) = 0;

	*victim = f;

}

/* cleric */

void spell_aid(byte level, struct char_data* ch,
			   struct char_data* victim, struct obj_data* obj) {
	/* combo bless, cure light woundsish */
	struct affected_type af;

	if (affected_by_spell(victim, SPELL_AID)) {
		send_to_char("Senti che l'aiuto Divino e' gia' presente\n\r", ch);
		return;
	}

	GET_HIT(victim)+=number(1,8);

	update_pos(victim);

	act("$n sembra aiutat$b dagli Dei", FALSE, victim, 0, 0, TO_ROOM);
	send_to_char("Ti senti meglio!\n\r", victim);

	af.type      = SPELL_AID;
	af.duration  = 10;
	af.modifier  = 1;
	af.location  = APPLY_HITROLL;
	af.bitvector = 0;
	affect_to_char(victim, &af);

}

void spell_holyword(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	int lev, t_align;
	struct char_data* t, *next;

	if (level > 0)
	{ t_align = -300; }
	else {
		level = -level;
		t_align = 300;
	}

	for (t = real_roomp(ch->in_room)->people; t; t=next) {
		next = t->next_in_room;
		if (!IS_IMMORTAL(t) && !IS_AFFECTED(t, AFF_SILENCE)) {
			if (level > 0) {
				if (GET_ALIGNMENT(t) <= t_align) {
					if ((lev = GetMaxLevel(t)) <= 4) {
						damage(ch, t, GET_MAX_HIT(t)*20, SPELL_HOLY_WORD, 5);
					}
					else if (lev <= 8) {
						damage(ch, t, 1, SPELL_HOLY_WORD, 5);
						spell_paralyze(level, ch, t, 0);
					}
					else if (lev <= 12) {
						damage(ch, t, 1, SPELL_HOLY_WORD, 5);
						spell_blindness(level, ch, t, 0);
					}
					else if (lev <= 16) {
						damage(ch, t, 0, SPELL_HOLY_WORD, 5);
						GET_POS(t) = POSITION_STUNNED;
					}
				}
			}
			else {
				if (GET_ALIGNMENT(t) >= t_align) {
					if ((lev = GetMaxLevel(t)) <= 4) {
						damage(ch, t, GET_MAX_HIT(t)*20, SPELL_UNHOLY_WORD, 5);
					}
					else if (lev <= 8) {
						damage(ch, t, 1, SPELL_UNHOLY_WORD, 5);
						spell_paralyze(level, ch, t, 0);
					}
					else if (lev <= 12) {
						damage(ch, t, 1, SPELL_UNHOLY_WORD, 5);
						spell_blindness(level, ch, t, 0);
					}
					else if (lev <= 16) {
						damage(ch, t, 1, SPELL_UNHOLY_WORD, 5);
						GET_POS(t) = POSITION_STUNNED;
					}
				}
			}
		}
	}
}

#define GOLEM 38

void spell_golem(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	int count=0;
	int armor;
	struct char_data* gol;
	struct obj_data* helm=0,*jacket=0,*leggings=0,*sleeves=0,*gloves=0,
						 *boots=0,*o;
	struct room_data* rp;

	/* you need:  helm, jacket, leggings, sleeves, gloves, boots */

	rp = real_roomp(ch->in_room);
	if (!rp) { return; }

	for (o = rp->contents; o; o = o->next_content) {
		if (ITEM_TYPE(o) == ITEM_ARMOR) {
			if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HEAD)) {
				if (!helm) {
					count++;
					helm = o;
					continue;  /* next item */
				}
			}
			if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET)) {
				if (!boots) {
					count++;
					boots = o;
					continue;  /* next item */
				}
			}
			if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_BODY)) {
				if (!jacket) {
					count++;
					jacket = o;
					continue;  /* next item */
				}
			}
			if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS)) {
				if (!leggings) {
					count++;
					leggings = o;
					continue;  /* next item */
				}
			}
			if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS)) {
				if (!sleeves) {
					count++;
					sleeves = o;
					continue;  /* next item */
				}
			}
			if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS)) {
				if (!gloves) {
					count++;
					gloves = o;
					continue;  /* next item */
				}
			}
		}
	}

	if (count < 6) {
		send_to_char("Non hai tutti i pezzi che ti servono!\n\r", ch);
		return;
	}

	if (count > 6) {
		send_to_char("C'e' qualcosa che non torna!\n\r", ch);
		return;
	}

	if (!boots || !sleeves || !gloves || !helm || !jacket || !leggings) {
		/* shouldn't get this far */
		send_to_char("Non hai tutti i pezzi giusti!\n\r", ch);
		return;
	}

	gol = read_mobile(GOLEM, VIRTUAL);
	char_to_room(gol, ch->in_room);

	/* add up the armor values in the pieces */
	armor = boots->obj_flags.value[0];
	armor += helm->obj_flags.value[0];
	armor += gloves->obj_flags.value[0];
	armor += (leggings->obj_flags.value[0]*2);
	armor += (sleeves->obj_flags.value[0]*2);
	armor += (jacket->obj_flags.value[0]*3);

	GET_AC(gol) -= armor;

	gol->points.max_hit = dice( (armor/6), 10) + GetMaxLevel(ch);
	GET_HIT(gol) = GET_MAX_HIT(gol);

	GET_LEVEL(gol, WARRIOR_LEVEL_IND) = (armor/6);

	SET_BIT(gol->specials.affected_by, AFF_CHARM);
	GET_EXP(gol) = 0;
	IS_CARRYING_W(gol) = 0;
	IS_CARRYING_N(gol) = 0;

	gol->player.iClass = CLASS_WARRIOR;

	if (GET_LEVEL(gol, WARRIOR_LEVEL_IND) > 10)
	{ gol->mult_att+=0.5; }

	/* add all the effects from all the items to the golem */
	AddAffects(gol,boots);
	AddAffects(gol,gloves);
	AddAffects(gol,jacket);
	AddAffects(gol,sleeves);
	AddAffects(gol,leggings);
	AddAffects(gol,helm);

	act("$n esegue un rituale sul mucchio di armature", FALSE, ch, 0, 0,
		TO_ROOM);
	act("Muovi le tue mani sulla pila di armature eseguendo un antico rituale.", FALSE, ch, 0, 0, TO_CHAR);

	act("I pezzi di armatura volano per la stanza e si uniscono in una figura umanoide!", FALSE, ch, 0, 0,
		TO_ROOM);

	act("$N si forma rapidamente con i vari pezzi a disposizione", FALSE, ch, 0, gol, TO_CHAR);

	add_follower(gol, ch);

	extract_obj(helm);
	extract_obj(boots);
	extract_obj(gloves);
	extract_obj(leggings);
	extract_obj(sleeves);
	extract_obj(jacket);

}


/***************/


void spell_feeblemind(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	int t,i;

	if (!affected_by_spell(victim,SKILL_MINDBLANK) ||
			!saves_spell(victim, SAVING_SPELL)) {
		if(affected_by_spell(victim, SPELL_FEEBLEMIND)) {
			send_to_char("E' gia' abbastanza stupido cosi'!\n\r", ch);
			return;
		}

		send_to_char("Ti senti molto stupido\n\r", victim);

		af.type      = SPELL_FEEBLEMIND;
		af.duration  = 24;
		af.modifier  = -5;
		af.location  = APPLY_INT;
		af.bitvector = 0;
		affect_to_char(victim, &af);

		af.type      = SPELL_FEEBLEMIND;
		af.duration  = 24;
		af.modifier  = 70;
		af.location  = APPLY_SPELLFAIL;
		af.bitvector = 0;
		affect_to_char(victim, &af);

		/*
		 * last, but certainly not least
		 */

		if (!victim->skills)
		{ return; }

		t = number(1,100);

		while (1) {
			for (i=0; i<MAX_SKILLS; i++) {
				if (victim->skills[i].learned)
				{ t--; }
				if (t==0) {
					victim->skills[i].learned = 0;
					victim->skills[i].flags = 0;
					break;
				}
			}
			return;
		}

	}
	else if (!victim->specials.fighting) {
		/* they saved */
		set_fighting(victim,ch);
	}
}


void spell_shillelagh( byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj ) {
	int i;
	int count = 0;

	assert( ch && obj );
	assert( MAX_OBJ_AFFECT >= 2 );

	if( GET_ITEM_TYPE(obj) == ITEM_WEAPON &&
			!IS_SET( obj->obj_flags.extra_flags, ITEM_MAGIC ) ) {

		if( !isname( "club", obj->name ) && !isname( "clava", obj->name ) ) {
			send_to_char( "Non e` una clava!\n\r", ch );
			return;
		}

		for( i = 0; i < MAX_OBJ_AFFECT; i++ ) {
			if( obj->affected[i].location == APPLY_NONE )
			{ count++; }
			if (obj->affected[i].location == APPLY_HITNDAM ||
					obj->affected[i].location == APPLY_HITROLL ||
					obj->affected[i].location == APPLY_DAMROLL) {
				act( "Non riesci ad incantare $p.", FALSE, ch, obj, NULL, TO_CHAR );
				return;
			}
		}

		if( count < 2 ) {

			act( "Non riesci ad incantare $p.", FALSE, ch, obj, NULL, TO_CHAR );
			return;
		}

		SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

		/*  find the slots */
		i = getFreeAffSlot( obj );

		obj->affected[i].location = APPLY_HITROLL;
		obj->affected[i].modifier = 2;

		i = getFreeAffSlot( obj );

		obj->affected[i].location = APPLY_DAMROLL;
		obj->affected[i].modifier = 1;

		obj->obj_flags.value[1] = 3;
		obj->obj_flags.value[2] = 4;

		act( "Un alone giallo si forma per un momento intorno a $p.", FALSE, ch,
			 obj, 0, TO_CHAR );
		act( "Un alone giallo si forma per un momento intorno a $p.", TRUE, ch,
			 obj, 0, TO_ROOM );
		SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL);

	}
	else {
		act( "Non riesci ad incantare $p.", FALSE, ch, obj, NULL, TO_CHAR );
	}
}


void spell_goodberry(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct obj_data* tmp_obj;

	assert(ch);
	assert((level >= 0) && (level <= ABS_MAX_LVL));


	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char("Questa magia deve essere usata all'aperto.\n\r",ch);
		return;
	}


	CREATE(tmp_obj, struct obj_data, 1);
	clear_object(tmp_obj);

	tmp_obj->name = strdup("berry mirtillo");
	tmp_obj->short_description = strdup( "un grosso mirtillo" );
	tmp_obj->description = strdup("C'e` un delizioso mirtillo qui in terra.");

	tmp_obj->obj_flags.type_flag = ITEM_FOOD;
	tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
	tmp_obj->obj_flags.value[0] = 10;
	tmp_obj->obj_flags.weight = 1;
	tmp_obj->obj_flags.cost = 10;
	tmp_obj->obj_flags.cost_per_day = 1;

	/*
	 * give it a cure light wounds spell effect
	 */

	SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_MAGIC);

	tmp_obj->affected[0].location = APPLY_EAT_SPELL;
	tmp_obj->affected[0].modifier = SPELL_CURE_SERIOUS;


	tmp_obj->next = object_list;
	object_list = tmp_obj;

	obj_to_char(tmp_obj,ch);

	tmp_obj->item_number = -1;

	act("$p appare nelle mani di $n.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("$p appare nelle tue mani.",TRUE,ch,tmp_obj,0,TO_CHAR);
}




void spell_elemental_blade(byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj) {
	int blade_element;
	struct obj_data* tmp_obj;

	assert(ch);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

	if (ch->equipment[WIELD]) {
		send_to_char("Non puoi farlo mentre impugni un'arma.\n\r", ch);
		return;
	}

	if ((ch->equipment[WEAR_LIGHT] && ch->equipment[HOLD])) { // SALVO controllo 2 mani
		send_to_char("Hai solo due mani.\n\r", ch);
		return;
	}

	blade_element = number(0,2);

	CREATE(tmp_obj, struct obj_data, 1);
	clear_object(tmp_obj);

	switch(blade_element) { /* REQUIEM 2018 new elements for druid blade */
	case 0:
		tmp_obj->name = strdup("blade flame");
		tmp_obj->short_description = strdup("una $c0009flame $c0007blade");
		tmp_obj->description = strdup("Una lama fiammeggiante splende qui in terra.");
		tmp_obj->affected[0].location = APPLY_WEAPON_SPELL;
		tmp_obj->affected[0].modifier = 57;
		break;
	case 1:
		tmp_obj->name = strdup("blade frost");
		tmp_obj->short_description = strdup("una $c0014Frost $c0007blade");
		tmp_obj->description = strdup("Noti una lunga lama di ghiaccio.");
		tmp_obj->affected[0].location = APPLY_WEAPON_SPELL;
		tmp_obj->affected[0].modifier = 8;
		break;
	case 2:
		tmp_obj->name = strdup("blade thunder");
		tmp_obj->short_description = strdup("una $c0012Thunder $c0007blade");
		tmp_obj->description = strdup("Un fascio si fulmini e saette e' qui ai tuoi piedi.");
		tmp_obj->affected[0].location = APPLY_WEAPON_SPELL;
		tmp_obj->affected[0].modifier = 30;
		break;
	}


	tmp_obj->obj_flags.type_flag = ITEM_WEAPON;
	tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_WIELD;
	tmp_obj->obj_flags.value[0] = 0;
	tmp_obj->obj_flags.value[1] = 3;
	tmp_obj->obj_flags.value[2] = 4;
	tmp_obj->obj_flags.value[3] = 3;
	tmp_obj->obj_flags.weight = 1;
	tmp_obj->obj_flags.cost = 10;
	tmp_obj->obj_flags.cost_per_day = -1;

	SET_BIT( tmp_obj->obj_flags.extra_flags,
			 ITEM_MAGIC | ITEM_GLOW );

	if( ( IS_PC( ch ) && HasClass( ch, CLASS_DRUID ) ) ||
			( !IS_PC( ch ) && IS_SET( ch->specials.act, ACT_DRUID ) ) ) {
		SET_BIT( tmp_obj->obj_flags.extra_flags,
				 ITEM_ANTI_EVIL | ITEM_ANTI_GOOD |
				 ITEM_ANTI_CLERIC | ITEM_ANTI_MAGE | ITEM_ANTI_THIEF |
				 ITEM_ANTI_FIGHTER | ITEM_ANTI_BARBARIAN | ITEM_ANTI_RANGER |
				 ITEM_ANTI_PALADIN | ITEM_ANTI_PSI | ITEM_ANTI_MONK );
	}

	/* REQUIEM 2018 Reduced damroll of the blade due weapon_spell improvement... */

	tmp_obj->affected[1].location = APPLY_DAMROLL;
	tmp_obj->affected[1].modifier = 3 +
									( IS_PC( ch ) ? GET_LEVEL(ch, DRUID_LEVEL_IND) : GetMaxLevel( ch ) ) / 25;

	/* GAIA 2000 Added hitroll the blade... */

	tmp_obj->affected[2].location = APPLY_HITROLL;
	tmp_obj->affected[2].modifier = 1 +
									( IS_PC( ch ) ? GET_LEVEL(ch, DRUID_LEVEL_IND) : GetMaxLevel( ch ) ) / 10;

	tmp_obj->next = object_list;
	object_list = tmp_obj;

	equip_char(ch, tmp_obj, WIELD);

	tmp_obj->item_number = -1;

	act("$p appare nelle tue mani.",TRUE,ch,tmp_obj,0,TO_CHAR);
	act("$p appare nelle mani di $n.",TRUE,ch,tmp_obj,0,TO_ROOM);
}




void spell_animal_growth(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Questa magia puo` essere lanciata solo all'aperto.\n\r",ch);
		return;
	}


	if (!victim || !IsAnimal(victim)) {
		act( "$N non e` un animale.", FALSE, ch, NULL, victim, TO_CHAR );
		return;
	}

	if( affected_by_spell( victim, SPELL_ANIMAL_GROWTH ) ) {
		act("$N e` gia` stato fatto crescere.", FALSE, ch, 0, victim, TO_CHAR );
		return;
	}

	if( GetMaxLevel(victim)*2 > GetMaxLevel(ch) ) {
		act( "Non puoi fare diventare $N piu` potente di te.", FALSE, ch,
			 NULL, victim, TO_CHAR );
		return;
	}

	if (IS_PC(victim)) {
		send_to_char( "Non ti sembra di cattivo gusto farlo su un giocatore ?",
					  ch );
		return;
	}

	act("$n raddoppia le sue dumensioni!", FALSE, victim, 0,0, TO_ROOM);
	act("Raddoppi le tue dimensioni!", FALSE,victim,0,0,TO_CHAR);

	af.type      = SPELL_ANIMAL_GROWTH;
	af.duration  = 12;
	af.modifier  = GET_MAX_HIT(victim);
	af.location  = APPLY_HIT;
	af.bitvector = AFF_GROWTH;
	affect_to_char(victim, &af);

	af.type      = SPELL_ANIMAL_GROWTH;
	af.duration  = 12;
	af.modifier  = 5;
	af.location  = APPLY_HITNDAM;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	af.type      = SPELL_ANIMAL_GROWTH;
	af.duration  = 12;
	af.modifier  = 3;
	af.location  = APPLY_SAVE_ALL;
	af.bitvector = 0;
	affect_to_char(victim, &af);

#if 0
	GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
#endif

}

void spell_insect_growth(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;


	if (!victim ||GET_RACE(victim)!=RACE_INSECT) {
		send_to_char("Non e` un insetto.\n\r", ch);
		return;
	}

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char("Questa magia puo` essere lanciata solo all'aperto.\n\r",ch);
		return;
	}


	if (affected_by_spell(victim, SPELL_INSECT_GROWTH)) {
		act("$N  e` gia` stato fatto crescere.",FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if (GetMaxLevel(victim)*2 > GetMaxLevel(ch)) {
		act( "Non puoi fare diventare $N piu` potente di te.", FALSE, ch,
			 NULL, victim, TO_CHAR );
		return;
	}

	if (IS_PC(victim)) {
		send_to_char( "Non ti sembra di cattivo gursto farlo su un giocatore ?",
					  ch );
		return;
	}

	act("$n raddoppia le sue dimensioni!", TRUE, victim, 0,0, TO_ROOM);
	act("Raddoppi le tue dimensioni!", FALSE,victim,0,0,TO_CHAR);

	af.type      = SPELL_INSECT_GROWTH;
	af.duration  = 12;
	af.modifier  = GET_MAX_HIT(victim);
	af.location  = APPLY_HIT;
	af.bitvector = AFF_GROWTH;
	affect_to_char(victim, &af);

	af.type      = SPELL_INSECT_GROWTH;
	af.duration  = 12;
	af.modifier  = 5;
	af.location  = APPLY_HITNDAM;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	af.type      = SPELL_INSECT_GROWTH;
	af.duration  = 12;
	af.modifier  = 3;
	af.location  = APPLY_SAVE_ALL;
	af.bitvector = 0;
	affect_to_char(victim, &af);

#if 0
	GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
#endif

}

#define CREEPING_DEATH 39

void spell_creeping_death(byte level, struct char_data* ch,
						  struct char_data* victim, int dir) {
	struct affected_type af;
	struct char_data* cd;

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char("L'incantesimo va lanciato all'aperto!\n\r",ch);
		return;
	}


#if 0
	if (GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
		send_to_char("Spell disabled.",ch);
		return;
	}
#endif



	/* obj is really the direction that the death wishes to travel in */

	cd = read_mobile( CREEPING_DEATH, VIRTUAL );
	if (!cd) {
		send_to_char("Non c'e' nulla di simile\n\r", ch);
		return;
	}

	char_to_room(cd, ch->in_room);
	cd->points.max_hit += (number(1,4)*100)+600;

	cd->points.hit = cd->points.max_hit;

	act("$n emette un terribile suono dalla bocca...", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("Hai una sensazione orribile...\n\r", ch);

	act("Una massa enorme di insetti mortali esce dalla bocca di $n!",
		FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("Una massa enorme di insetti esce dalla tua bocca!\n\r",ch);

	act("Gli insetti si fondono in una mass solida - $n", FALSE, ch, 0, 0, TO_ROOM);

	cd->generic = dir;
	cd->act_ptr = ch;

	/* move the creeping death in the proper direction */

	do_move(cd, "\0", dir);

	if (GetMaxLevel(ch) < IMMORTALE) {
		GET_POS(ch) = POSITION_STUNNED;
		WAIT_STATE( ch, 3*PULSE_VIOLENCE ); // creeping
	}

	af.type      = SPELL_CREEPING_DEATH;
	af.duration  = 3;
	af.modifier  = 10500;
	af.location  = APPLY_SPELLFAIL;
	af.bitvector = 0;
	affect_to_char(ch, &af);

}


void spell_commune(byte level, struct char_data* ch,
				   struct char_data* victim, struct obj_data* obj) {
	struct char_data* c;
	struct room_data* rp, *dp;
	char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


	/* look up the creatures in the mob list, find the ones in
	 * this zone, in rooms that are outdoors, and tell the
	 * caster about them */

	buffer[0] = '\0';
	buf[0] = '\0';

	dp = real_roomp(ch->in_room);
	if (!dp)
	{ return; }

	if (IS_SET(dp->room_flags, INDOORS)) {
		send_to_char("L'incantesimo va lanciato all'aperto!\n\r", ch);
		return;
	}

	for (c = character_list; c; c = c->next) {
		rp = real_roomp(c->in_room);
		if (!rp) { return; }

		if (rp->zone == dp->zone) {
			if (!IS_SET(rp->room_flags, INDOORS)) {
				sprintf( buf, "%s is in %s\n\r",
						 IS_NPC(c) ? c->player.short_descr : GET_NAME(c), rp->name );
				if (strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2)
				{ break; }
				strcat(buffer, buf);
				strcat(buffer, "\r");
			}
		}
	}

	page_string(ch->desc, buffer, 1);

}

#define ANISUM1  72
#define ANISUM2  78
#define ANISUM3  84

void spell_animal_summon(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	struct char_data* mob;
	int num, i;
	struct room_data* rp;

	/* load in a monster of the correct type, determined by
	 * level of the spell */

	/* really simple to start out with */

	if ((rp = real_roomp(ch->in_room)) == NULL)
	{ return; }

	if (IS_SET(rp->room_flags, TUNNEL)) {
		send_to_char( "Non c'e` abbastanza spazio qui.\n\r", ch);
		return;
	}

	if (IS_SET(rp->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r", ch);
		return;
	}


	if (affected_by_spell(ch, SPELL_ANIMAL_SUM_1)) {
		send_to_char( "Puoi farlo solo ogni 48 ore.\n\r", ch);
		return;
	}


	switch(level) {
	case 1:
		num = ANISUM1;
		break;
	case 2:
		num = ANISUM2;
		break;
	case 3:
		num = ANISUM3;
		break;
	default:
		num = -1;
		break;
	}

	if( num < 0 )
	{ return; }

	act("$n esegue uno strano rituale.", TRUE, ch, 0, 0, TO_ROOM);
	act("Esegui il rituale dell'evocazione.", TRUE, ch, 0, 0, TO_CHAR);

	for( i = 0; i < 4; i++ ) {
		mob = read_mobile( num + number( 0, 5 ), VIRTUAL );

		if( !mob )
		{ continue; }

		char_to_room(mob, ch->in_room);
		act( "$n arriva improvvisamente.", FALSE, mob, 0, 0, TO_ROOM);
		if( too_many_followers( ch ) ) {
			act( "$N guarda alla dimensione del tuo gruppo e si rifiuta di "
				 "seguirti!", TRUE, ch, 0, victim, TO_CHAR);
			act( "$N guarda alla dimensione del gruppo di $n e si rifiuta di "
				 "seguirlo!", TRUE, ch, 0, victim, TO_ROOM);
		}
		else {
			/* charm them for a while */
			if( mob->master )
			{ stop_follower(mob); }

			add_follower(mob, ch);

			af.type      = SPELL_CHARM_PERSON;
			if (IS_PC(ch) || ch->master) {
				af.duration  = GET_CHR(ch);
				af.modifier  = 0;
				af.location  = 0;
				af.bitvector = AFF_CHARM;
				affect_to_char(mob, &af);

			}
			else {
				SET_BIT(mob->specials.affected_by, AFF_CHARM);
			}
		}
		if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
			REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
		}
		if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
			SET_BIT(mob->specials.act, ACT_SENTINEL);
		}

	}

	af.type =      SPELL_ANIMAL_SUM_1;
	af.duration  = 48;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}

#define FIRE_ELEMENTAL  40

void spell_elemental_summoning(byte level, struct char_data* ch,
							   struct char_data* victim, int spell) {
	int vnum;
	struct char_data* mob;
	struct affected_type af;

	if (affected_by_spell(ch, spell)) {
		send_to_char( "Puoi farlo solo ogni 24 ore.\n\r", ch);
		return;
	}

	vnum = spell - SPELL_FIRE_SERVANT;
	vnum += FIRE_ELEMENTAL;

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r", ch );
		return;
	}

	mob = read_mobile(vnum, VIRTUAL);

	if (!mob) {
		send_to_char("None available\n\r", ch);
		return;
	}

	act("$n esegue uno strano rituale.", TRUE, ch, 0, 0, TO_ROOM);
	act("Esegui il rituale dell'evocazione.", TRUE, ch, 0, 0, TO_CHAR);

	char_to_room(mob, ch->in_room);
	act( "$n appare attraverso una momentanea smagliatura del tessuto "
		 "spazio-tempo.", FALSE, mob, 0, 0, TO_ROOM);
	if (too_many_followers(ch)) {
		act( "$c0015[$c0005$n$c0015] dice 'Non ho nessuna intenzione di "
			 "confondermi con quella folla!'",
			 TRUE, mob, NULL, NULL, TO_ROOM);
	}
	else {

		/* charm them for a while */
		if (mob->master)
		{ stop_follower(mob); }

		add_follower(mob, ch);

		af.type      = SPELL_CHARM_PERSON;

		if (IS_PC(ch) || ch->master) {
			af.duration  = 24;
			af.modifier  = 0;
			af.location  = 0;
			af.bitvector = AFF_CHARM;
			affect_to_char(mob, &af);

		}
		else {
			SET_BIT(mob->specials.affected_by, AFF_CHARM);
		}
	}

	af.type =      spell;
	af.duration  = 24;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	/*
	 * adjust the bits...
	 */

	/*
	 * get rid of aggressive, add sentinel
	 */

	if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
	}
	if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
		SET_BIT(mob->specials.act, ACT_SENTINEL);
	}
}


void spell_reincarnate(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct char_data* newch;
	struct char_file_u st;
	struct descriptor_data* d;
	FILE* fl,*fdeath;
	char szFileName[ 40 ];

	if (!obj)
	{ return; }

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r",ch);
		return;
	}

	if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
		send_to_char("Puoi lanciare questa magia solo in una foresta.\n\r", ch);
		return;
	}

	if (IS_CORPSE(obj)) {
		if (obj->char_vnum) {
			send_to_char( "Questa magia funziona solo sui giocatori.\n\r", ch);
			return;
		}

		sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( obj->oldfilename ) );
		if( ( fl = fopen( szFileName, "r+" ) ) == NULL ) {
			mudlog( LOG_SYSERR, "Cannot find player file %s in reincarnate.",
					szFileName );
			send_to_char( "Problemi con il file del giocatore da reincarnare.\n\r",
						  ch );
			send_to_char( "Contattare un Dio.\n\r", ch );
			return;
		}

#if DEATH_FIX
		sprintf( szFileName, "%s/%s.dead", PLAYERS_DIR, lower( obj->oldfilename ) );
		if( ( fdeath = fopen( szFileName, "r+" ) ) == NULL ) {
			mudlog( LOG_SYSERR, "Cannot find dead file %s in resurrect.",
					szFileName );
			send_to_char( "Problemi con il file del giocatore da resuscitare.\n\r",
						  ch );
			send_to_char( "Contattare un Dio.\n\r", ch );
			return;
		}
#endif
		fread(&st, sizeof(struct char_file_u), 1, fl);
		if( !get_char(st.name) && st.abilities.con > 3 ) {
#if DEATH_FIX
			long xp;
			long ora;
			fscanf(fdeath,"%ld : %ld",&xp,&ora);
			st.points.exp=xp;
			ora=(long)(time(0)-ora);
			mudlog( LOG_PLAYERS, "%s resuscitato dopo %ld secondi",
					obj->oldfilename,ora);

#else
			st.points.exp *= 5;
			st.points.exp /= 4;
#endif
			if( number(1,5) < 4 ) { st.abilities.con -= 1; }

			st.race = GetNewRace(&st);

			act( "La foresta si anima di suoni d'uccelli e d'animali...",
				 TRUE, ch, 0, 0, TO_CHAR);
			act( "La foresta si anima di suoni d'uccelli e d'animali...",
				 TRUE, ch, 0, 0, TO_ROOM);
			act( "... e $p scompare in un batter d'occhio.",
				 TRUE, ch, obj, 0, TO_CHAR);
			act( "... e $p scompare in un batter d'occhio.",
				 TRUE, ch, obj, 0, TO_ROOM);
			GET_MANA(ch) = 1;
			alter_mana(ch,0);
			GET_MOVE(ch) = 1;
			alter_move(ch,0);
			GET_HIT(ch) = 1;
			alter_hit(ch,0);
			GET_POS(ch) = POSITION_SITTING;
			act( "$n crolla a terra esausto!",TRUE, ch, 0, 0, TO_ROOM);
			send_to_char( "Crolli a terra esausto!\n\r",ch);

			rewind( fl );
			fwrite(&st, sizeof(struct char_file_u), 1, fl);
			ObjFromCorpse(obj);

			CREATE(newch, struct char_data, 1);
			clear_char(newch);

			store_to_char(&st, newch);

			reset_char(newch);

			newch->next = character_list;
			character_list = newch;

			char_to_room(newch, ch->in_room);
			newch->invis_level = 51;

			set_title(newch);
			GET_HIT(newch) = 1;
			alter_hit(newch,0);
			GET_MANA(newch) = 1;
			alter_mana(newch,0);
			GET_MOVE(newch) = 1;
			alter_move(newch,0);
			GET_POS(newch) = POSITION_SITTING;
			save_char(newch, AUTO_RENT, 0);

			/* if they are in the descriptor list, suck them into the game */

			for (d = descriptor_list; d; d=d->next) {
				if (d->character && (strcmp(GET_NAME(d->character),
											GET_NAME(newch))==0)) {
					if (STATE(d) != CON_PLYNG) {
						free_char(d->character);
						d->character = newch;
						STATE(d) = CON_PLYNG;
						newch->desc = d;
						send_to_char( "Ti svegli e ti senti diverso.\n\r",
									  newch);
						break;
					}
				}
			}
		}
		else {
			send_to_char( "Lo spirito non ha abbastanza forza per "
						  "reincarnarsi.\n\r", ch);
		}
		fclose(fl);
	}
}

void spell_charm_veggie(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if (victim == ch) {
		send_to_char( "Ami te stesso piu` di prima.\n\r", ch);
		return;
	}

	if (!IsVeggie(victim)) {
		send_to_char( "Puoi farlo solo sulle piante!\n\r", ch);
		return;
	}

	if (GetMaxLevel(victim) > GetMaxLevel(ch) + 10) {
		FailCharm(victim, ch);
		return;
	}

	if (too_many_followers(ch)) {
		act( "$N guarda alla dimensione del tuo gruppo e si rifiuta di "
			 "seguirti!", TRUE, ch, 0, victim, TO_CHAR);
		act( "$N guarda alla dimensione del gruppo di $n e si rifiuta di "
			 "seguirlo!", TRUE, ch, 0, victim, TO_ROOM);
		return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
		if (circle_follow(victim, ch)) {
			send_to_char( "Mi spiace ma seguirsi l'un l'altro non e` permesso.\n\r",
						  ch );
			return;
		}
		if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
			FailCharm(victim,ch);
			return;
		}
		if (IsResist(victim, IMM_CHARM)) {
			if (saves_spell(victim, SAVING_PARA)) {
				FailCharm(victim,ch);
				return;
			}

			if (saves_spell(victim, SAVING_PARA)) {
				FailCharm(victim,ch);
				return;
			}
		}
		else {
			if (!IsSusc(victim, IMM_CHARM)) {
				if (saves_spell(victim, SAVING_PARA)) {
					FailCharm(victim,ch);
					return;
				}
			}
		}

		if (victim->master)
		{ stop_follower(victim); }

		add_follower(victim, ch);

		af.type      = SPELL_CHARM_PERSON;

		if (GET_INT(victim))
		{ af.duration  = 24*GET_CHR(ch)/GET_INT(victim); }
		else
		{ af.duration  = 24*18; }

		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_CHARM;
		affect_to_char(victim, &af);

		act( "Ti senti attratt$b da $n.",FALSE,ch,0,victim,TO_VICT);
	}
}


void spell_veggie_growth(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if (!victim || !IsVeggie(victim)) {
		send_to_char( "Puoi farlo solo sulle piante.\n\r", ch);
		return;
	}

	if (affected_by_spell(victim, SPELL_VEGGIE_GROWTH)) {
		act( "$N e` gia` stato fatto crescere.",FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if (GetMaxLevel(victim)*2 > GetMaxLevel(ch)) {
		send_to_char( "Non puoi farlo piu` potente di te.\n\r", ch);
		return;
	}

	if (IS_PC(victim)) {
		send_to_char( "Non ti sembra di cattivo gursto farlo su un giocatore ?",
					  ch );
		return;
	}

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r",ch);
		return;
	}

	act("$n raddoppia le sue dimensioni!", TRUE, victim, 0,0, TO_ROOM);
	act("Raddoppi le tue dimensioni!", FALSE,victim,0,0,TO_CHAR);

	af.type      = SPELL_VEGGIE_GROWTH;
	af.duration  = 2*level;
	af.modifier  = GET_MAX_HIT(victim);
	af.location  = APPLY_HIT;
	af.bitvector = AFF_GROWTH;
	affect_to_char(victim, &af);

	af.type      = SPELL_VEGGIE_GROWTH;
	af.duration  = 2*level;
	af.modifier  = 5;
	af.location  = APPLY_HITNDAM;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	af.type      = SPELL_VEGGIE_GROWTH;
	af.duration  = 2*level;
	af.modifier  = 3;
	af.location  = APPLY_SAVE_ALL;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);

}

#define SAPLING  45

void spell_tree(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {
	struct char_data* mob;
	int mobn;


	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r", ch);
		return;
	}


	mobn = SAPLING;
	if (level > 20) {
		mobn++;
	}
	if (level > 30) {
		mobn++;
	}
	if (level > 40) {
		mobn++;
	}
	if (level > 48) {
		mobn++;
	}
	mob = read_mobile(mobn, VIRTUAL);
	if (mob) {
		spell_poly_self(level, ch, mob, 0);
	}
	else {
		send_to_char( "Non riesci ad evocare un'immagine della creatura.\n\r", ch);
	}
	return;

}

#define LITTLE_ROCK  50

void spell_animate_rock(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct char_data* mob;
	struct affected_type af;
	int mobn=LITTLE_ROCK;

	if (ITEM_TYPE(obj) != ITEM_ROCK) {
		send_to_char( "Non e` il giusto tipo di roccia.\n\r", ch);
		return;
	}

	/* get the weight of the rock, make the follower based on the weight */

	if (GET_OBJ_WEIGHT(obj) > 20)
	{ mobn++; }
	if (GET_OBJ_WEIGHT(obj) > 40)
	{ mobn++; }
	if (GET_OBJ_WEIGHT(obj) > 80)
	{ mobn++; }
	if (GET_OBJ_WEIGHT(obj) > 160)
	{ mobn++; }
	if (GET_OBJ_WEIGHT(obj) > 320)
	{ mobn++; }

	mob = read_mobile(mobn, VIRTUAL);
	if (mob) {
		char_to_room(mob, ch->in_room);
		/* charm them for a while */
		if (mob->master)
		{ stop_follower(mob); }

		add_follower(mob, ch);

		af.type      = SPELL_ANIMATE_ROCK;
		af.duration  = 24;
		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_CHARM;
		affect_to_char(mob, &af);

		af.type      = SPELL_CHARM_PERSON;

		if (IS_PC(ch) || ch->master) {
			af.duration  = 24;
			af.modifier  = 0;
			af.location  = 0;
			af.bitvector = AFF_CHARM;
			affect_to_char(mob, &af);

		}
		else {
			SET_BIT(mob->specials.affected_by, AFF_CHARM);
		}

		/*
		 * get rid of aggressive, add sentinel
		 */

		if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
			REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
		}
		if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
			SET_BIT(mob->specials.act, ACT_SENTINEL);
		}

		extract_obj(obj);

	}
	else {
		send_to_char( "Mi dispiace, ma questa magia non funziona oggi.\n\r", ch);
		return;
	}

}


void spell_travelling(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

	if ( affected_by_spell(victim, SPELL_TRAVELLING) )
	{ return; }

	af.type      = SPELL_TRAVELLING;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_TRAVELLING;

	affect_to_char(victim, &af);
	act("$n sembra avere tanti piedi.", TRUE, victim, 0, 0, TO_ROOM);
	act( "Ti senti pien$b di piedi.", FALSE, victim, NULL, NULL, TO_CHAR );
}

void spell_animal_friendship(byte level, struct char_data* ch,
							 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

	if ( affected_by_spell(ch, SPELL_ANIMAL_FRIENDSHIP) ) {
		send_to_char( "Puoi farlo solo una volta al giorno.\n\r", ch);
		return;
	}

	if (IS_GOOD(victim) || IS_EVIL(victim)) {
		send_to_char( "Puoi farlo solo su animali neutrali.\n\r", ch);
		return;
	}

	if (!IsAnimal(victim)) {
		send_to_char( "Puoi farlo solo su animali.\n\r", ch);
		return;
	}

	if( GET_MAX_HIT( victim ) > GetMaxLevel( ch ) * 4 ) {
		act( "Ancora non hai abbastanza forza di volonta` per incantare $N.",
			 FALSE, ch, NULL, victim, TO_CHAR );
		return;
	}

	if (too_many_followers(ch)) {
		act( "$N guarda alla dimensione del tuo gruppo e si rifiuta di "
			 "seguirti!", TRUE, ch, 0, victim, TO_CHAR);
		act( "$N guarda alla dimensione del gruppo di $n e si rifiuta di "
			 "seguirlo!", TRUE, ch, 0, victim, TO_ROOM);
		return;
	}

	if( IsImmune(victim, IMM_CHARM) || saves_spell( victim, SAVING_SPELL ) ) {
		act( "$N sembra ignorare i tuoi tentativi ammaliatori.", FALSE, ch, NULL,
			 victim, TO_CHAR );
		return;
	}

	if (victim->master)
	{ stop_follower(victim); }

	add_follower(victim, ch);

	af.type      = SPELL_ANIMAL_FRIENDSHIP;
	af.duration  = 24;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);


	af.type      = SPELL_ANIMAL_FRIENDSHIP;
	af.duration  = 36;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_CHARM;
	affect_to_char(victim, &af);

	/*
	 * get rid of aggressive, add sentinel
	 */
	REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);

	SET_BIT(victim->specials.act, ACT_SENTINEL);

}

void spell_invis_to_animals(byte level, struct char_data* ch,
							struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if (!affected_by_spell(victim, SPELL_INVIS_TO_ANIMALS)) {
		act( "$n sembra sbiadire per un momento.", TRUE, victim,0,0,TO_ROOM );
		send_to_char( "Scompari... quasi.\n\r", victim);

		af.type      = SPELL_INVIS_TO_ANIMALS;
		af.duration  = 24;
		af.modifier  = 0;
		af.location  = APPLY_AFF2;
		af.bitvector = AFF2_ANIMAL_INVIS;
		affect_to_char(victim, &af);
	}
}


void spell_slow_poison(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if( affected_by_spell( victim, SPELL_POISON ) ) {
		act( "$n sembra migliorare.", TRUE, victim,0,0,TO_ROOM );
		send_to_char( "Ti senti un po' meglio!.\n\r", victim );

		af.type      = SPELL_SLOW_POISON;
		af.duration  = 24;
		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
}


void spell_snare( byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r",ch);
		return;
	}

	if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
		send_to_char( "Puoi farlo solo in una foresta.\n\r", ch);
		return;
	}

	/* if victim fails save, movement = 0 */
	if (!saves_spell(victim, SAVING_SPELL)) {
		act( "Alcune radici e liane vengono fuori dal terreno e si arrotolano "
			 "intorno ai tuoi piedi!", FALSE, victim, 0,0, TO_CHAR);
		act( "Alcune radici e liane vengono fuori dal terreno e si arrotolano "
			 "intorno ai piedi di $n!", FALSE, victim, 0,0, TO_CHAR);
		GET_MOVE(victim)=0;
	}
	else {
		FailSnare(victim, ch);
	}
}

void spell_entangle(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if( IS_SET( real_roomp( ch->in_room )->room_flags, INDOORS ) ) {
		act( "Quest'incantesimo lo puoi fare solo all'aperto!", FALSE, ch, 0, 0,
			 TO_CHAR );
		return;
	}

	if( real_roomp( ch->in_room )->sector_type != SECT_FOREST ) {
		act( "Quest'incantesimo lo puoi fare solo in una foresta.", FALSE, ch,
			 0, 0, TO_CHAR );
		return;
	}

	/* if victim fails save, paralyzed for a very short time */
	if( !saves_spell( victim, SAVING_SPELL ) ) {
		act( "Alcune radice e liane vengono fuori dal terreno e ti avvolgono.",
			 FALSE, victim, 0,0, TO_CHAR);
		act( "Alcune radice e liane vengono fuori dal terreno e avvolgono $n.",
			 FALSE, victim, 0,0, TO_CHAR);

		af.type      = SPELL_ENTANGLE;
		af.duration  = 1;
		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_PARALYSIS;
		affect_to_char( victim, &af );
	}
	else {
		FailSnare(victim, ch);
	}
}

void spell_barkskin(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

	if (!affected_by_spell(victim, SPELL_BARKSKIN) &&
			!affected_by_spell(victim, SPELL_ARMOR)) {

		af.type      = SPELL_BARKSKIN;
		af.duration  = 24;
		af.modifier  = -10 - level/2; /* level dependent Gaia 2001 */
		af.location  = APPLY_AC;
		af.bitvector = 0;
		affect_to_char(victim, &af);

		af.type      = SPELL_BARKSKIN;
		af.duration  = 24;
		af.modifier  = -1 - level/10; /* level dependent Gaia 2001 */
		af.location  = APPLY_SAVE_ALL;
		af.bitvector = 0;
		affect_to_char(victim, &af);

		af.type      = SPELL_BARKSKIN;
		af.duration  = 24;
		af.modifier  = 1 + level/10; /* level dependent Gaia 2001 */
		af.location  = APPLY_SAVING_SPELL;
		af.bitvector = 0;
		affect_to_char(victim, &af);

		if (ch == victim ) {
			if (level > 30) {
				/* resistance to cold */

				af.type      = SPELL_BARKSKIN;
				af.duration  = 24;
				af.modifier  = IMM_COLD;
				af.location  = APPLY_IMMUNE;
				af.bitvector = 0;
				affect_to_char(ch, &af);
			}
			if (level > 40) {
				/* resistance to piercing weapons */

				af.type      = SPELL_BARKSKIN;
				af.duration  = 24;
				af.modifier  = IMM_PIERCE;
				af.location  = APPLY_IMMUNE;
				af.bitvector = 0;
				affect_to_char(ch, &af);
			}
			if (level > 45) {
				/* resistance to acid */

				af.type      = SPELL_BARKSKIN;
				af.duration  = 24;
				af.modifier  = IMM_ACID;
				af.location  = APPLY_IMMUNE;
				af.bitvector = 0;
				affect_to_char(ch, &af);
			}
			if (level > 49) {
				/* resistance to electricity */

				af.type      = SPELL_BARKSKIN;
				af.duration  = 24;
				af.modifier  = IMM_ELEC;
				af.location  = APPLY_IMMUNE;
				af.bitvector = 0;
				affect_to_char(ch, &af);
			}
		}

		send_to_char( "La tua pelle assume un aspetto ruvido e legnoso.\n\r",
					  victim);
		act( "La pelle di $n assume un aspetto ruvido e legnoso.", TRUE, victim, 0, 0,
			 TO_ROOM);

	}
	else {
		send_to_char( "Non sembra succedere nulla di nuovo.\n\r", ch);
	}
}

void spell_gust_of_wind(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct char_data* tmp_victim, *temp;

	assert(ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	act( "Muovi le mani ed alzi una raffica di vento!",
		 TRUE, ch, 0, 0, TO_CHAR );
	act( "$n muove le mani ed alza una una raffica di vento!",
		 TRUE, ch, 0, 0, TO_ROOM );

	for( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = temp ) {
		temp = tmp_victim->next_in_room;
		if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if( GetMaxLevel( tmp_victim ) > IMMORTALE )
			{ return; }
#if defined( ALAR )
			if( !in_group( ch, tmp_victim ) &&
					(!saves_spell( tmp_victim, SAVING_SPELL ) ||
					 number(1,60) < GET_LEVEL(ch,BestMagicClass(ch)))
					&& !CheckEquilibrium(tmp_victim)         //Acidus 2003 - skill better equilibrium
			  ) {
				act( "$n perde l'equilibrio per la forte raffica di vento.",
					 TRUE, tmp_victim, 0, 0, TO_ROOM );
				act( "La raffica lanciata da $n ti fa perdere l'equilibrio.",
					 TRUE, ch, 0, tmp_victim, TO_VICT );
				WAIT_STATE( tmp_victim, PULSE_VIOLENCE * 1 ); // gust
				GET_POS( tmp_victim ) = POSITION_SITTING;
			}
			else {
				if( IsHumanoid( tmp_victim ) ) {
					act( "La raffica di vento alzata da $n ti scompiglia i capelli.",
						 TRUE, ch, 0, tmp_victim, TO_VICT );
				}
			}
#else
			if( !in_group( ch, tmp_victim ) &&
					!saves_spell( tmp_victim, SAVING_SPELL ) ) {
				act( "$n perde l'equilibrio per la forte raffica di vento.",
					 TRUE, tmp_victim, 0, 0, TO_ROOM );
				act( "La raffica lanciata da $n ti fa perdere l'equilibrio.",
					 TRUE, ch, 0, tmp_victim, TO_VICT );
				WAIT_STATE( tmp_victim, PULSE_VIOLENCE * 1 ); // gust
				GET_POS( tmp_victim ) = POSITION_SITTING;
			}
			else {
				if( IsHumanoid( tmp_victim ) ) {
					act( "La raffica di vento alzata da $n ti scompiglia i capelli.",
						 TRUE, ch, 0, tmp_victim, TO_VICT );
				}
			}
#endif
		}
	}
}


void spell_silence(byte level, struct char_data* ch,
				   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if (!saves_spell(victim, SAVING_SPELL)) {
		act( "$n smette di fare rumore!", TRUE, victim,0,0,TO_ROOM);
		send_to_char( "Non riesci a sentire piu nulla!.\n\r", victim);

		af.type      = SPELL_SILENCE;
		af.duration  = level;
		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_SILENCE;
		affect_to_char(victim, &af);
	}
	else {
		send_to_char( "Ti senti silenzioso per un momento, ma poi la sensazione "
					  "passa.", victim );
		if( IS_MOB( victim ) && mob_index[victim->nr].func ) {
			if( !(*mob_index[victim->nr].func)( ch, 0, "", victim, EVENT_ATTACK ) ) {
				if( !victim->specials.fighting ) {
					set_fighting(victim, ch);
				}
			}
		}
	}
}

void spell_warp_weapon(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {

	assert(ch && (victim || obj));

	if (!obj) {
		if (!victim->equipment[WIELD]) {
			act("$N non sta impugnando un'arma!", FALSE, ch, 0,
				victim, TO_CHAR);
			return;
		}
		obj = victim->equipment[WIELD];
	}

	act( "$n spunta e deforma $p con la potenza del suo incantesimo.", FALSE,
		 ch, obj, 0, TO_CHAR);
	act( " $p viene spuntata e deformata dalla potenza dell'incantesimo di $n.",
		 FALSE, ch, obj, 0, TO_ROOM);
	DamageOneItem(victim, BLOW_DAMAGE, obj);

	if (!IS_PC(victim))
		if (!victim->specials.fighting)
		{ set_fighting(victim,ch); }
}



void spell_heat_stuff(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if (check_peaceful(victim, "")) // SALVO metto il controllo peaceful
	{ return; }
	if (affected_by_spell(victim, SPELL_HEAT_STUFF)) {
		send_to_char( "Non sembra succedere nulla di nuovo.\n\r", victim);
		return;
	}

	if (HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
		af.type = SPELL_HEAT_STUFF;
		af.duration = level;
		af.modifier = -2;
		af.location = APPLY_DEX;
		af.bitvector = 0;
		affect_to_char(victim, &af);

		af.type = SPELL_HEAT_STUFF;
		af.duration = level;
		af.modifier = 0;
		af.location = APPLY_AFF2;
		af.bitvector = AFF2_HEAT_STUFF;

		affect_to_char(victim, &af);
		send_to_char("La tua armatura inizia a sfrigolare e fumare.\n\r", victim);
		act( "L'armatura di $N inizia a sfrigolare.\n\r", FALSE, ch, 0, victim,
			 TO_CHAR);
		act( "L'armatura di $N inizia a sfrigolare.\n\r", FALSE, ch, 0, victim,
			 TO_NOTVICT);
		GET_HIT(victim) -= 1 ; // Gaia 2001
		alter_hit( victim, 0 ); // Gaia 2001
		if (!IS_PC(victim))
			if (!victim->specials.fighting)
			{ set_fighting(victim,ch); }
	}
}

#define DUST_DEVIL 60

void spell_dust_devil( byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int vnum;
	struct char_data* mob;
	struct affected_type af;

	if (affected_by_spell(ch, SPELL_DUST_DEVIL)) {
		send_to_char( "Puoi farlo solo ogni 24 ore.\n\r", ch);
		return;
	}

	if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char( "Puoi farlo solo all'aperto.\n\r", ch);
		return;
	}


	vnum = DUST_DEVIL;

	mob = read_mobile(vnum, VIRTUAL);

	if (!mob) {
		send_to_char("None available\n\r", ch);
		return;
	}

	act("$n esegue uno strano rituale!", TRUE, ch, 0, 0, TO_ROOM);
	act("Esegui il rituale dell'evocazione.", FALSE, ch, 0, 0, TO_CHAR);

	char_to_room(mob, ch->in_room);
	act( "$n arriva attraverso uno squarcio nel tessuto sapzio-tempo!",
		 FALSE, mob, 0, 0, TO_ROOM);

	/* charm them for a while */
	if (mob->master)
	{ stop_follower(mob); }

	add_follower(mob, ch);

	af.type      = SPELL_CHARM_PERSON;

	if (IS_PC(ch) || ch->master) {
		af.duration  = 24;
		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_CHARM;
		affect_to_char(mob, &af);

	}
	else {
		SET_BIT(mob->specials.affected_by, AFF_CHARM);
	}


	af.type =      SPELL_DUST_DEVIL;
	af.duration  = 24;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	/*
	 * adjust the bits...
	 */

	/*
	 * get rid of aggressive, add sentinel
	 */

	if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
	}
	if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
		SET_BIT(mob->specials.act, ACT_SENTINEL);
	}
}

void spell_sunray(byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	struct char_data* t, *n;
	int dam;

	/*
	 * blind all in room
	 */
	for (t= real_roomp(ch->in_room)->people; t; t=n) {
		n = t->next_in_room;
		if (!in_group(ch, t) && !IS_IMMORTAL(t)) {
			spell_blindness(level, ch, t, obj);
			/*
			 * hit undead target
			 */
			if (t == victim) {
				if( IsUndead(victim) ||
						GET_RACE(victim) == RACE_VEGMAN) {
					dam = dice(6,8);
					if (saves_spell(victim, SAVING_SPELL)&&
							(GET_RACE(victim)!=RACE_VEGMAN))
					{ dam >>= 1; }
					damage(ch, victim, dam, SPELL_SUNRAY, 5);
				}
			}
			else {
				/*
				 * damage other undead in room
				 */
				if( IsUndead(t) ||
						GET_RACE(t) == RACE_VEGMAN) {
					dam = dice(3,6);
					if( saves_spell(t, SAVING_SPELL)&&
							(GET_RACE(t)!=RACE_VEGMAN))
					{ dam = 0; }
					damage(ch, t, dam, SPELL_SUNRAY, 5);
				}
			}
		}
	}
}

void spell_know_monster( byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj ) {
	char buf[256], buf2[256];
	int exp, lev, hits;

	/*
	 * depending on level, give info.. sometimes inaccurate
	 */

	if (!IS_PC(victim)) {
		sprintf(buf,"$N e` un$B %s.", RaceName[GET_RACE(victim)]);
		act(buf,FALSE, ch, 0, victim, TO_CHAR);
		if (level > 5) {
			exp = GetApprox(GET_EXP(victim), 40+level);
			sprintf(buf, "$N ha circa %d punti di esperienza.", exp);
			act(buf,FALSE, ch, 0, victim, TO_CHAR);
		}
		if (level > 10) {
			lev = GetApprox(GetMaxLevel(victim), 40+level);
			sprintf( buf,
					 "$N combatte piu` o meno come un guerriero del %d^ livello.",
					 lev );
			act(buf,FALSE, ch, 0, victim, TO_CHAR);
		}
		if (level > 15) {
			if (IS_SET(victim->hatefield, HATE_RACE)) {
				sprintf( buf, "$N sembra odiare la razza dei %s.",
						 RaceName[victim->hates.race] );
				act(buf,FALSE, ch, 0, victim, TO_CHAR);
			}
			if (IS_SET(victim->hatefield, HATE_CLASS)) {
				sprintbit((unsigned)victim->hates.iClass, pc_class_types, buf2);
				sprintf(buf, "$N sembra odiare la classe %s.", buf2);
				act(buf,FALSE, ch, 0, victim, TO_CHAR);
			}
		}
		if (level > 20) {
			hits = GetApprox(GET_MAX_HIT(victim), 40+level);
			sprintf(buf,"Probabilmente $N ha %d punti vita.", hits);
			act(buf,FALSE, ch, 0, victim, TO_CHAR);
		}
		if (level > 25) {
			if (victim->susc) {
				sprintbit(victim->susc, immunity_names, buf2);
				sprintf(buf, "$N e` suscettibile a %s.", buf2);
				act(buf,FALSE, ch, 0, victim, TO_CHAR);
			}
		}
		if (level > 30) {
			if (victim->immune) {
				sprintbit(victim->immune, immunity_names, buf2);
				sprintf(buf, "$N e` resistente a %s.", buf2);
				act(buf,FALSE, ch, 0, victim, TO_CHAR);
			}
		}
		if (level > 35) {
			if (victim->M_immune) {
				sprintbit(victim->M_immune, immunity_names, buf2);
				sprintf(buf, "$N e` immune a %s.", buf2);
				act(buf,FALSE, ch, 0, victim, TO_CHAR);
			}
		}
		if (level > 40) {
			int att;
			att = GetApprox((int)victim->mult_att, 30+level);
			sprintf(buf,"$N ha circa %d.0 attacchi per turno.", att);
			act(buf,FALSE, ch, 0, victim, TO_CHAR);
		}
		if (level > 45) {
			int no, s;
			no = GetApprox(victim->specials.damnodice, 30+level);
			s = GetApprox(victim->specials.damsizedice, 30+level);

			sprintf(buf,"Ognuno dei quali fa circa %dd%d punti di danno.",
					no, s);
			act(buf,FALSE, ch, 0, victim, TO_CHAR);
		}
	}
	else {
		send_to_char( "Non e` realmente un mostro.\n\r", ch);
		return;
	}
}

void spell_find_traps( byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	/*
	 * raise their detect traps skill
	 */
	if (affected_by_spell(ch, SPELL_FIND_TRAPS)) {
		send_to_char("Stai gia' cercando eventuali trappole.\n\r", ch);
		return;
	}

	af.type =      SPELL_FIND_TRAPS;
	af.duration  = level;
	af.modifier  = 50+level;
	af.location  = APPLY_FIND_TRAPS;
	af.bitvector = 0;
	affect_to_char(ch, &af);

}

void spell_firestorm(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	/*
	 * a-e -    2d8+level
	 */
	int dam;
	struct char_data* tmp_victim, *temp;

	assert(ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(2,8) + level*2 + 1;  /* Potenziato un pochetto
                                     come alternatica al chain Gaia 2001 */

	send_to_char("Le fiamme ti avvolgono!\n\r", ch);
	act("$n crea una tempesta di fuoco che si scatena nella stanza!\n\r",
		FALSE, ch, 0, 0, TO_ROOM);

	for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = temp ) {
		temp = tmp_victim->next_in_room;
		if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if ((GetMaxLevel(tmp_victim)>IMMORTALE) && (!IS_NPC(tmp_victim)))
			{ return; }
			if (!in_group(ch, tmp_victim)) {
				act("Vieni avvolt$b dalle fiamme!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				if ( saves_spell(tmp_victim, SAVING_SPELL) )
				{ dam >>= 1; }
				heat_blind(tmp_victim);
				MissileDamage(ch, tmp_victim, dam, SPELL_BURNING_HANDS, 5);
				spell_fear(level, ch, tmp_victim, 0);
			}
			else {
				act("Riesci a schivare le fiamme!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				heat_blind(tmp_victim);
			}
		}
	}
}


void spell_teleport_wo_error(byte level, struct char_data* ch,
							 struct char_data* victim, struct obj_data* obj) {
	int location;
	struct room_data* rp;

	/* replaces the current functionality of astral walk */
	assert(ch && victim);

	location = victim->in_room;
	rp = real_roomp(location);

	if (GetMaxLevel(victim) > MAX_MORT ||
			!rp ||
			IS_SET(rp->room_flags,  PRIVATE) ||
			IS_SET(rp->room_flags,  NO_SUM) ||
			IS_SET(rp->room_flags,  NO_MAGIC) ||
			(IS_SET(rp->room_flags,  TUNNEL) &&
			 (MobCountInRoom(rp->people) > rp->moblim)))  {
		send_to_char("Non riesci.\n\r", ch);
		return;
	}

	if (!IsOnPmp(location)) {
		send_to_char("E' in un piano extra-dimensionale!\n", ch);
		return;
	}
	if (!IsOnPmp(ch->in_room)) {
		send_to_char("Sei in un piano extra-dimensionale!\n\r", ch);
		return;
	}


	if (dice(1,20) == 20) {
		send_to_char("Sbagli la magia e perdi il controllo!\n\r", ch);
		spell_teleport(level, ch, ch, 0);
		return;
	}
	else {
		act("$n apre un portale attraverso un'altra dimensione e lo attraversa!",
			FALSE,ch,0,0,TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, location);
		act("Vieni acceccat$b per un attimo quando $n appeare in un lampo di luce!",
			FALSE,ch,0,0,TO_ROOM);
		do_look(ch, "",15);
		check_falling(ch);

		if (IS_SET(real_roomp(ch->in_room)->room_flags, DEATH) &&
				GetMaxLevel(ch) < IMMORTALE) {
			NailThisSucker(ch);
			return;
		}
	}
}

#define PORTAL 31

void spell_portal(byte level, struct char_data* ch,
				  struct char_data* tmp_ch, struct obj_data* obj) {
	/* create a magic portal */
	struct obj_data* tmp_obj;
	struct extra_descr_data* ed;
	struct room_data* rp, *nrp;
	char buf[512];

	assert(ch);
	assert((level >= 0) && (level <= ABS_MAX_LVL));


	/*
	 * check target room for legality.
	 */
	rp = real_roomp( ch->in_room );
	/*  tmp_obj = read_object( PORTAL, VIRTUAL );*/
	if( !rp ) {
		send_to_char("L'incantesimo non riesce\n\r", ch);
		return;
	}

	if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
		send_to_char("Un'oscura magia blocca il tuo incantesimo.\n\r", ch);
		return;
	}

	if (IS_SET(rp->room_flags, TUNNEL)) {
		send_to_char("Non c'e' abbastanza spazio per aprire un portale!\n\r", ch);
		return;
	}

	if( !( nrp = real_roomp(tmp_ch->in_room) ) ) {
		mudlog( LOG_SYSERR, "%s not in any room", GET_NAME( tmp_ch ) );
		send_to_char("Non c'e' nulla du simile in giro.\n", ch);
		return;
	}

	if (IS_SET(real_roomp(tmp_ch->in_room)->room_flags, NO_SUM)) {
		send_to_char("Un'antica magia ti blocca.\n\r", ch);
		return;
	}

	if (!IsOnPmp(ch->in_room)) {
		send_to_char("Sei in un piano extra-dimensionale!\n\r", ch);
		return;
	}

	if (!IsOnPmp(tmp_ch->in_room)) {
		send_to_char("E' in un piano extra-dimensionale!\n\r", ch);
		return;
	}

	if (IS_SET(SystemFlags,SYS_NOPORTAL)) {
		send_to_char("I piani dimensionali sono in movimento, non riesci ad aprire un portale!\n",ch);
		return;
	}

	tmp_obj = read_object( PORTAL, VIRTUAL );
	if( !tmp_obj ) {
		mudlog( LOG_SYSERR, "Cannot load PORTAL in spell_portal (magic3.c)." );
		send_to_char("L'incantesimo fallisce\n\r", ch);
		return;
	}

	sprintf( buf, "Attraverso la nebbia che avvolge il portale riesci a vedere %s",
			 nrp->name);

	CREATE(ed, struct extra_descr_data, 1);
	ed->nMagicNumber = EXDESC_VALID_MAGIC;
	ed->next = tmp_obj->ex_description;
	tmp_obj->ex_description = ed;
	if( tmp_obj->name )
	{ ed->keyword = strdup( tmp_obj->name ); }
	else
		mudlog( LOG_SYSERR,
				"Obj PORTAL has no name in spell_portal (magic3.c)." );

	ed->description = strdup(buf);

	tmp_obj->obj_flags.value[0] = MAX((level-10)/5,5);
	tmp_obj->obj_flags.value[1] = tmp_ch->in_room;

	obj_to_room(tmp_obj,ch->in_room);

	act("$p appare improvvisamente dal nulla.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("$p appare improvvisamente dal nulla.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

#define MOUNT_ONE 65
#define MOUNT_GOOD 69
#define MOUNT_EVIL 70
#define MOUNT_NEUT 71

void spell_mount(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	struct char_data* m;
	int mnr;

	/* create a ridable mount, and automatically mount said creature */

	mnr = MOUNT_ONE;
	if (level < 30) {
		if (level < 12)
		{ mnr++; }
		if (level < 18)
		{ mnr++; }
		if (level < 24)
		{ mnr++; }
	}
	else {
		if (IS_EVIL(ch)) {
			mnr = MOUNT_EVIL;
		}
		else if (IS_GOOD(ch)) {
			mnr = MOUNT_GOOD;
		}
		else {
			mnr = MOUNT_NEUT;
		}
	}

	m = read_mobile(mnr, VIRTUAL);
	if (m) {
		char_to_room(m, ch->in_room);
		act("Con un lampo di luce, $N appare", FALSE, ch, 0, m, TO_CHAR);
		act("Con un lampo di luce, $N appare, e $n gli salta in groppa", FALSE,
			ch, 0, m, TO_ROOM);
		send_to_char("Salti in groppa alla tua cavalcatura\n\r", ch);
		MOUNTED(ch) = m;
		RIDDEN(m) = ch;
		GET_POS(ch) = POSITION_MOUNTED;
	}
	else {
		send_to_char("horses aren't in database\n\r", ch);
		return;
	}
}

void spell_dragon_ride(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if (affected_by_spell(ch, SPELL_DRAGON_RIDE)) {
		send_to_char("L'incantesimo e' gia' attivo\n\r", ch);
		return;
	}

	af.type = SPELL_DRAGON_RIDE;
	af.duration = level;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = AFF_DRAGON_RIDE;
	affect_to_char(ch, &af);
}

} // namespace Alarmud

