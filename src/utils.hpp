/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __UTILS__HPP
#define __UTILS__HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "cmdid.hpp"
#include "snew.hpp"
#include "hash.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
namespace Alarmud {

#define RM_BLOOD(rm)   (real_roomp(rm)->blood)

//#define TRUE  1

//#define FALSE 0

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')

#define IS_WEAPON(o) (o->obj_flags.type_flag == ITEM_WEAPON)

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)))

#define CREATE(result, type, number)  do {\
		if (!((result) = static_cast<type *> (calloc ((number), sizeof(type)))))\
		{ SetStatus("calloc"); abort(); }\
	} while(0)

#define RECREATE(result,type,number) do {\
		if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ SetStatus("realloc"); abort(); } \
	} while(0)


#define IS_SET(flag,bit)  ((flag) & (bit))


#define SWITCH(a,b) { (a) ^= (b); \
		(b) ^= (a); \
		(a) ^= (b); }

#define IS_AFFECTED(ch,skill)  (IS_SET((ch)->specials.affected_by, (skill)))
#define IS_AFFECTED2(ch,skill) (IS_SET((ch)->specials.affected_by2,(skill)))

#define IS_RARE( obj ) ( obj->obj_flags.cost >= LIM_ITEM_COST_MIN ) // Gaia 2001


#define IS_DARK_P(proom) ( !IS_SET( (proom)->room_flags, BRIGHT ) && \
						   (proom)->light <= 0 && \
						   ( IS_SET( (proom)->room_flags, DARK ) || IsDarkOutside( (proom) ) ) )

#define IS_LIGHT_P(proom) ( IS_SET( (proom)->room_flags, BRIGHT ) || \
							(proom)->light>0 || \
							( !IS_SET( (proom)->room_flags, DARK ) || \
							  !IsDarkOutside( (proom) ) ) )

#define IS_DARK(room) ( !IS_SET( real_roomp(room)->room_flags, BRIGHT ) && \
						real_roomp(room)->light <= 0 && \
						( IS_SET( real_roomp(room)->room_flags, DARK ) ||  \
						  IsDarkOutside( real_roomp(room) ) ) )

#define IS_LIGHT(room) ( IS_SET( real_roomp(room)->room_flags, BRIGHT ) || \
						 real_roomp(room)->light>0 || \
						 ( !IS_SET( real_roomp(room)->room_flags, DARK ) || \
						   !IsDarkOutside( real_roomp(room) ) ) )

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define RM_FLAGS(i)  ((real_roomp(i))?real_roomp(i)->room_flags:0)

#define GET_LEVEL(ch, i)   ((ch)->player.level[(i)])
#define GET_LEVEL_CASTER(ch, i)   ((ch)->player.level[(i)]<11? \
								   (ch)->player.level[(i)]: \
								   ((ch)->player.level[(i)] - HowManyClasses((ch))+1))

#define GET_CLASS_TITLE(ch, iClass, lev)   ((ch)->player.sex ?  \
		(((ch)->player.sex == 1) ? titles[(iClass)][(lev)].title_m : \
		 titles[(iClass)][(lev)].title_f) : titles[(iClass)][(lev)].title_m)

#define GET_REQ(i) (i<2  ? "Pessimo" :(i<4  ? "Maluccio"     :(i<7  ? "Scarso"      :\
									   (i<10 ? "Medio" :(i<14 ? "Discreto"    :(i<20 ? "Bene"    :(i<24 ? "Molto bene" :\
											   "Superbo" )))))))

#define HSHR(ch) ((ch)->player.sex ?                                        \
				  (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?                                        \
				  (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ?                                         \
				  (((ch)->player.sex == 1) ? "him" : "her") : "it")

#define LEGLI(ch) ((ch)->player.sex ?                                   \
				   (((ch)->player.sex == 1) ? "gli" : "le") : "gli")

#define SSLF(ch) ((ch)->player.sex ?                                    \
				  (((ch)->player.sex == 1) ? "o" : "a") : "o")

#define UNUNA(ch) ((ch)->player.sex ?                                   \
				   (((ch)->player.sex == 1) ? "un" : "una") : "un")

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

/* GGBEG */
#define IS_PKILLER(ch)   (IS_SET((ch)->player.user_flags,RACE_WAR))

#define GET_EMAIL(ch)     ((ch)->specials.email)

#define GET_REALNAME(ch)     ((ch)->specials.realname)

#define GET_AUTHCODE(ch)   ((ch)->specials.authcode)

#define GET_AUTHBY(ch)   ((ch)->specials.AuthorizedBy)
#define GET_TEMPO_IN(ch,pos) ((ch)->specials.TempoPassatoIn[pos])
#define GET_POS_PREV(ch)   ((ch)->specials.PosPrev)
/* GGPATCH */

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_NAME_DESC( ch ) ( IS_NPC( ch ) ? (ch)->player.short_descr : \
							  (ch)->player.name )

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_CLASS(ch)   ((ch)->player.iClass)

#define GET_HOME(ch)        ((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)

#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)

#define GET_DEX(ch)     ((ch)->tmpabilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)

#define GET_CHR(ch)     ((ch)->tmpabilities.chr)


#define GET_RSTR(ch)     ((ch)->abilities.str)

#define GET_RADD(ch)     ((ch)->abilities.str_add)

#define GET_RDEX(ch)     ((ch)->abilities.dex)

#define GET_RINT(ch)     ((ch)->abilities.intel)

#define GET_RWIS(ch)     ((ch)->abilities.wis)

#define GET_RCON(ch)     ((ch)->abilities.con)

#define GET_RCHR(ch)     ((ch)->abilities.chr)

#define STRENGTH_APPLY_INDEX(ch) \
	( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
	  (GET_ADD(ch) <= 50) ? 26 :( \
								  (GET_ADD(ch) <= 75) ? 27 :( \
										  (GET_ADD(ch) <= 90) ? 28 :( \
												  (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
	)

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)
#define GET_MHIT(ch)     ((ch)->points.max_hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)
#define GET_MMOVE(ch)    ((ch)->points.max_move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->points.mana)
#define GET_MMANA(ch)    ((ch)->points.max_mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)

//#define GET_PQUEST(ch)  ((ch)->points.pQuest)
#define GET_RUNEDEI(ch)  ((ch)->points.pRuneDei)

#define GET_BANK(ch)    ((ch)->points.bankgold)

#define GET_ZONE(ch)    ((ch)->specials.permissions)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_HEIGHT(ch)  ((ch)->player.height)

#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_RACE(ch)     ((ch)->race)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define GET_SPECFLAGS(ch) ((ch)->player.user_flags)

#define GET_POINTS_EVENT(ch, i) ((ch)->points_event[i])
#define GET_DAMAGE_EVENTS(ch)   ((ch)->damage_events)


#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING && \
				   !IS_AFFECTED(ch, AFF_PARALYSIS) )

#if 1 // se messo a 0 si elimina il lag
#if not LAG_MOBILES
#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = ((GetMaxLevel(ch) >= DEMIGOD) ? (0) : (cycle)) : 0)
#else
#define WAIT_STATE(ch, cycle)  if((ch)->desc) (ch)->desc->wait = ((GetMaxLevel(ch) >= DEMIGOD) ? (0) : (cycle)); else ch->specials.tick_to_lag = (cycle);
#endif
#else
#define WAIT_STATE(ch, cycle)  if((ch)->desc) (ch)->desc->wait = 0; else ch->specials.tick_to_lag = (cycle);
#endif

#define GET_WAIT_STATE( ch ) \
	( (ch)->desc ? (ch)->desc->wait : (ch)->specials.tick_to_lag )

#define IS_WAITING( ch ) \
	( (ch)->desc ? (ch)->desc->wait > 1 : (ch)->specials.tick_to_lag > 0 )



/* Object And Carry related macros */

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GetMaxLevel(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
	(((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
	 ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
	(CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
	 CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (                                          \
		CAN_SEE(vict, ch) ?                                                                                    \
		(!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) :        \
		"qualcuno")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
						 (obj)->short_description  : "qualcosa")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
						 fname((obj)->name) : "qualcosa")

#define OUTSIDE(ch) (!IS_SET(real_roomp((ch)->in_room)->room_flags,INDOORS))

#define GET_PRINCE(ch)    ((ch)->player.vassallodi)
#define IS_VASSALLOOF(ch,nome) (GET_PRINCE(ch) && !strcasecmp(GET_PRINCE(ch),nome))
#define IS_PRINCEOF(nome,ch) (IS_VASSALLOOF(ch,nome))
#define HAS_PRINCE(ch) ((ch) && (ch)->player.vassallodi)
/*#define IS_PRINCE(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch))==PRINCIPE&&(GET_EXP(ch)>=PRINCEEXP)) */
#define IS_PRINCE(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)==PRINCIPE))
#define HAS_BODYGUARD(ch) ((ch) && (ch)->specials.bodyguard)
#define GET_BODYGUARD(ch)    ((ch)->specials.bodyguard)

#define IS_IMMORTAL(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=LOW_IMMORTAL))
#define IS_IMMORTALE(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=IMMORTALE))
#define IS_DIO(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=DIO))
#define IS_DIO_MINORE(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=DIO_MINORE))
#define IS_QUESTMASTER(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=QUESTMASTER))
#define IS_MAESTRO(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=MAESTRO))
#define IS_DEMIURGO(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=DEMIURGO))
#define IS_CREATORE(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=CREATORE))
#define IS_MAESTRO_DEL_CREATO(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=MAESTRO_DEL_CREATO))
#define IS_MAESTRO_DEGLI_DEI(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=MAESTRO_DEGLI_DEI))
#define IS_BIG_GUY(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=BIG_GUY))
#define IS_IMMENSO(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)>=IMMENSO))
#define NO_HASSLE( ch ) (IS_PC( ch ) && \
						 IS_SET( (ch)->specials.act, PLR_NOHASSLE ) )

#define GET_OBJ_VNUM(o)    (obj_index[o->item_number].iVNum)
#define GET_MOB_VNUM(m)    (mob_index[m->nr].iVNum)
#define IS_POLICE(ch) ((GET_MOB_VNUM(ch) == 3060) || \
					   (GET_MOB_VNUM(ch) == 3069) || \
					   (GET_MOB_VNUM(ch) == 3067))
#define IS_CORPSE(obj) (GET_ITEM_TYPE((obj))==ITEM_CONTAINER && \
						(obj)->obj_flags.value[3] && \
						(isname("corpse",(obj)->name)|| \
						 isname("corpo",(obj)->name)))

#define EXIT(ch, door)  (real_roomp((ch)->in_room)->dir_option[door])

#define EXIT_NUM(room_num, door)  (real_roomp(room_num)->dir_option[door])


#define CAN_GO(ch, door) (EXIT(ch,door)&&real_roomp(EXIT(ch,door)->to_room) \
						  && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door) (EXIT(ch,door) && \
								real_roomp(EXIT(ch,door)->to_room) \
								&& !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))
/*GGPATCH */
#define IS_SINGLE(ch) (HowManyClasses(ch)==1)
#define IS_CASTER(ch)  (HasClass(ch,CLASS_MAGIC_USER | \
								 CLASS_SORCERER|\
								 CLASS_RANGER|\
								 CLASS_CLERIC|\
								 CLASS_PALADIN|\
								 CLASS_DRUID|\
								 CLASS_PSI\
								)\
					   )
#define IS_SPY(ch)  (HasClass(ch,CLASS_MONK | \
							  CLASS_BARBARIAN|\
							  CLASS_RANGER|\
							  CLASS_THIEF\
							 )\
					)
#define IS_FIGHTER(ch)  (HasClass(ch,CLASS_WARRIOR | \
								  CLASS_BARBARIAN| \
								  CLASS_RANGER|    \
								  CLASS_DRUID|     \
								  CLASS_MONK|     \
								  CLASS_PALADIN    \
								 )\
						)
#define IS_THIEF(ch)    (HasClass(ch,CLASS_THIEF))
#define IS_MONK(ch)    (HasClass(ch,CLASS_MONK))
/* GGPATCHEND */
#define ITEM_TYPE(obj)  (static_cast<int>((obj)->obj_flags.type_flag))

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))
#define IS_PC(ch) (!IS_NPC((ch)) || IS_SET((ch)->specials.act, ACT_POLYSELF))
#define IS_POLY(ch) (IS_NPC(ch) && IS_SET((ch)->specials.act,ACT_POLYSELF))


#define GET_AVE_LEVEL(ch) ((GetAverageLevel(ch)))
#define GET_SPEC_PARM(mob) (mob_index[(mob)->nr].specparms)

#define GET_ALIAS(ch, num) ((ch)->specials.A_list->com[(num)])

#define MOUNTED(ch) ((ch)->specials.mounted_on)
#define RIDDEN(ch) ((ch)->specials.ridden_by)

#if 0
#define isdigit(ch) (ch >= '0' && ch <= '9')
#endif

inline struct room_data* real_roomp( long lVNum ) {
#if HASH
	extern struct hash_header      room_db;
	return hash_find( &room_db, lVNum );
#else
	extern struct room_data*        room_db[WORLD_SIZE];
	return( ( lVNum < WORLD_SIZE && lVNum > -1 ) ? room_db[ lVNum ] : 0 );
#endif
}

#define USING( ch )  ( ( ch )->equipment[ HOLD ] )
#define WIELDING( ch ) ( ( ch )->equipment[ WIELD ] )
#define WEARING( ch, where ) ( ( where ) > MAX_WEAR_POS ? NULL : \
							   ( ch )->equipment[ where ] )
#define WEARING_N(ch, dummy, result) \
	for (dummy=result=0;dummy<MAX_WEAR_POS;dummy++) \
	{ \
		if WEARING(ch,dummy) result++; \
	}
#define REMOVE_FROM_LIST(item, head, next)  \
	if ((item) == (head))        \
		head = (item)->next;      \
	else {               \
		temp = head;          \
		while (temp && (temp->next != (item))) \
			temp = temp->next;     \
		if (temp)             \
			temp->next = (item)->next; \
	}                    \

#define safe_sprintf(a,...) {snprintf(a,sizeof a -1,__VA_ARGS__) ; a[sizeof a -1]='\0';}

} // namespace Alarmud
#endif

