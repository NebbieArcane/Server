/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: snew.c,v 1.3 2002/03/23 16:43:20 Thunder Exp $
*/
/***************************  System  include ************************************/
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
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
#include "snew.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
namespace Alarmud {
#define KEYLIB "keydir"
#define BUFLEN 128
bool TestMode=false;
char hname[128];
char* ggdup(const char* s) {
	char* p=NULL;
	if (s) {
		p=(char*)calloc(strlen(s)+1,sizeof(char));
		if (p) { strcpy(p,s); }
	}
	return(p);

}

char* HostName() {
	if (!*hname) { gethostname(hname,127); }
	return hname;
}
bool IsTest() {
	return TestMode ;
}
bool SetTest(bool value) {
	TestMode=value;
	return TestMode;
}


char* GetKey(char* db,char* chiave) {
	char buf[BUFLEN];
	FILE* ft;
	sprintf(buf,"%s/%s%s",KEYLIB,lower(db),lower(chiave));
	if ((ft=fopen(buf,"r")) ) {
		fclose(ft);
		return(ggdup(lower(buf)));
	}
	else {
		return (char*)NULL;
	}
}
int PutKey(char* db,char* chiave, char* value) {
	char buf[BUFLEN];
	int rc=1;
	FILE* ft;
	sprintf(buf,"%s/%s%s",KEYLIB,lower(db),lower(chiave));
	if ((ft=fopen(buf,"w")) ) {
		rc=fputs(lower(value),ft);
	}
	fclose(ft);
	return(rc>=0);
}
void DelKey(char* db,char* chiave) {
	char buf[BUFLEN];
	sprintf(buf,"%s/%s%s",KEYLIB,lower(db),lower(chiave));
	unlink(buf);
	return;
}
int GetStat(struct char_data* ch, int stat) {
	switch(stat) {
	case STR:
		return(GET_RSTR(ch));
	case INT:
		return(GET_RINT(ch));
	case WIS:
		return(GET_RWIS(ch));
	case DEX:
		return(GET_RDEX(ch));
	case CON:
		return(GET_RCON(ch));
	case CHR:
		return(GET_RCHR(ch));
	default:
		mudlog(LOG_SYSERR,"Bogus stat in get stat %d on %s",stat,GET_NAME(ch));
	}
	return 0;
}

long StatCost(struct char_data* ch, int stat) // SALVO meglio se long, modificate anche righe sotto
/* Calcola il costo. */
{
	int vecchia=0;
	long xpcost=0L;
	vecchia=GetStat(ch,stat);
	switch(vecchia) {
	case 25:
		xpcost+=10L; //125
	/* no break */
	case 24:
		xpcost+=10L; //115
	/* no break */
	case 23:
		xpcost+=10L; //105
	/* no break */
	case 22:
		xpcost+=10L; //95
	/* no break */
	case 21:
		xpcost+=10L; //85
	/* no break */
	case 20:
		xpcost+=10L; //75
	/* no break */
	case 19:
		xpcost+=10L; //65
	/* no break */
	case 18:
		xpcost+=10L; //55
	/* no break */
	case 17:
		xpcost+=10L; //45
	/* no break */
	case 16:
		xpcost+=5L;  //35
	/* no break */
	case 15:
		xpcost+=5L;  //30
	/* no break */
	case 14:
		xpcost+=5L;  //25
	/* no break */
	case 13:
		xpcost+=4L;  //20
	/* no break */
	case 12:
		xpcost+=3L;  //16
	/* no break */
	case 11:
		xpcost+=2L;  //13
	/* no break */
	case 10:
		xpcost+=1L;  //11
	/* no break */
	default:
		xpcost+=10L; //10
	}

	switch((long)(GetMaxLevel(ch)/10)) {
	case 0:
		xpcost*=2500;
		break;
	case 1:
		xpcost*=6800;
		break;
	case 2:
		xpcost*=35000;
		break;
	case 3:
		xpcost*=93000;
		break;
	case 4:
		xpcost*=400000;
		break;
	case 5:
		xpcost*=500000;
		break;
	default:
		xpcost*=500000;
		break;
	}
	//xpcost*=(long)(GetMaxLevel(ch)/10);
	return xpcost;
}

int MaxStat(struct char_data* ch,int stat) {
	switch(stat) {
	case STR:
		return(MaxStrForRace(ch));
	case INT:
		return(MaxIntForRace(ch));
	case WIS:
		return(MaxWisForRace(ch));
	case DEX:
		return(MaxDexForRace(ch));
	case CON:
		return(MaxConForRace(ch));
	case CHR:
		return(MaxChrForRace(ch));
	default:
		return(0);
	}
	return(0);
}


int SetStat(struct char_data* ch,int stat,int value)
/* Modifica la real stat, tenendo conto dei massimi razziali
 **/
{
	int nuova=0;
	int xpcost=1;
	nuova=value;
	if (nuova>MaxStat(ch,stat) ) {
		return(0);
	}

	switch(stat) {
	case STR :
		GET_RSTR(ch)=nuova;
		break;
	case INT :
		GET_RINT(ch)=nuova;
		break;
	case WIS :
		GET_RWIS(ch)=nuova;
		break;
	case DEX :
		GET_RDEX(ch)=nuova;
		break;
	case CON :
		GET_RCON(ch)=nuova;
		break;
	case CHR :
		GET_RCHR(ch)=nuova;
		break;
	default:
		mudlog(LOG_SYSERR,"Bogus stat in set stat %d on %s",stat,GET_NAME(ch));
		xpcost=0;
	}
	return xpcost;
}

int GetTargetType(struct char_data* ch,struct char_data* target,int ostility) {
	struct char_data* t = target;
	char* name=nullptr;
	int tt=gtt_IS_NONE;
	if (!ch || !t) { return(tt); }
	if (ch == t) { return(gtt_IS_SELF); }
	if (in_group(ch,t)) { tt=gtt_IS_FRIEND; }
	else if (IS_POLY(t)) { tt=gtt_IS_POLY; }
	else if (IS_PC(t)) { tt=gtt_IS_PLAYER; }
	if (IS_NPC(t)) {
		name=t->player.short_descr;
	}
	else {
		name=t->player.name;
	}
	if (name) {
		if (ch->specials.supporting && *ch->specials.supporting) {
			if (!(strcasecmp(ch->specials.supporting,name))) {
				tt=gtt_IS_SUPPORTED;
			}
		}

		if (ch->specials.bodyguarding && *ch->specials.bodyguarding) {
			if (!(strcasecmp(ch->specials.bodyguarding,name))) {
				tt=gtt_IS_BODYGUARDED;
			}
		}
	}
	if (in_clan(ch,t)) { tt=gtt_IS_CLAN; }
	if (ch->specials.fighting && ch->specials.fighting==t) {
		if(tt < gtt_IS_FRIEND)
		{ tt=gtt_IS_ENEMY; }
		else
		{ tt=gtt_IS_PLAYER; }
	}
	return(tt);
}
int modifier[]= {
	1,
	0,
	1,
	1,
	3,
	5,
	8,
	10,
	30,
	30
};
const char* targets[]= {
	"none",
	"self",
	"enemy",
	"poly",
	"player",
	"friend",
	"supported",
	"bodyguarded",
	"clan",
	"ullalla!"
};
void AlignMod(struct char_data* ch,struct char_data* victim,int ostility) {
	long   alignmod;
	long targettype;
	long ftargettype=0;
	struct char_data* fighted;
	if (!ch ) { return; }
	if (!IS_PC(ch)) { return; }
	if (!victim) { victim=ch; }
	fighted=victim->specials.fighting;

	if (ch == victim) { return; } /* ACIDUS 2003 le skill su se stessi non cambiano align */

	if (ch != victim) {
		targettype=GetTargetType(ch,victim,ostility);
		ftargettype=GetTargetType(ch,fighted,ostility);
	}
	else {
		targettype=gtt_IS_SELF;
		ftargettype=gtt_IS_SELF;
	}
	if (victim and victim->nMagicNumber!= CHAR_VALID_MAGIC) return;
	if (fighted and fighted->nMagicNumber!= CHAR_VALID_MAGIC) return;
	if (ch and ch->nMagicNumber!= CHAR_VALID_MAGIC) return;

	/* Verifica eventuale necessita' di inversione
	 * L'inversione viene valutata solo se ostility < 0
	 * In caso di azioni aggressive conta sempre il target reale
	 * */
	if (ostility >0) { ftargettype=targettype; }
	if (ftargettype>targettype) {

		/* Sto facendo qualcosa a qualcuno che combatte contro un personaggio
		 * cui sono legato da vincoli superiori.
		 * Inverto l'ostilita' e uso ftargettype
		 * */
		ostility=-ostility;
		targettype=ftargettype;
	}
	else if (targettype>ftargettype) {
		/* Sto facendo qualcosa a qualcuno che combatte contro un personaggio
		 * cui sono legato da vincoli di amicizia inferiori
		 * Aggiusto targettype - se due personaggi nel mio gruppo combattono
		 * fra loro non est facile decidere chi aiutare
		 * */
		targettype-=ftargettype;
		targettype=MAX(targettype,0); /* Non deve mai diventare 0 */
	}
	else {
		/* Sono legato ai due indifferentemente
		 * Questa branca c'e' solo per poter mettere il commento
		 * */
	}

	if (ostility < 0 )
	{ alignmod=ostility; }
	else
	{ alignmod=modifier[targettype]*((ostility>0)?+1:-1); }
	/*   alignmod+=ostility;*/
	if (!alignmod) { return; }
	if (targettype > gtt_IS_NONE) {
		if (number(1,3)==2) {
			if (alignmod<0)
			{ send_to_char("Ti senti meritevole\n",ch); }
			else
			{ send_to_char("Ti senti una Jena\n",ch); }
		}
	}

#ifdef NEW_ALIGN
	GET_ALIGNMENT(ch)-=alignmod;
#endif
	mudlog(LOG_CHECK,"Char: %s alignmod by %5d",GET_NAME(ch),-alignmod);
}
void ActionAlignMod(struct char_data* ch,struct char_data* victim,int cmd) {
	int ostility=0;
	switch( cmd) {

	case CMD_BACKSTAB:
		ostility=5;
		mudlog(LOG_CHECK,"Act: backstab ostility=%d",ostility);
		break;
	case CMD_RESCUE:
		ostility=-5;
		mudlog(LOG_CHECK,"Act: rescue ostility=%d",ostility);
		break;
	case CMD_DISARM:
		ostility=10;
		mudlog(LOG_CHECK,"Act: disarm ostility=%d",ostility);
		break;
	case CMD_BASH:
		ostility=1;
		mudlog(LOG_CHECK,"Act: bash ostility=%d",ostility);
		break;
	case CMD_KICK:
		ostility=-1;
		mudlog(LOG_CHECK,"Act: kick ostility=%d",ostility);
		break;
	case CMD_FINGER:
		ostility=5;
		mudlog(LOG_CHECK,"Act: finger ostility=%d",ostility);
		break;
	default:
		ostility=0;
		break;
	}

	AlignMod(ch,victim,ostility);

}

const char* GetTargetTypeString(int target) {
	if (target > gtt_LAST) { target=0; }
	return(targets[target]);
}

int GetNum(struct char_data* pMob,int pos) {
	const char* p;
	char appo[256];
	for(p=mob_index[pMob->nr].specparms; pos>0; pos--) {
		p=one_argument(p,appo);
	}
	return(atoi(appo));
}
int GetNumR(struct room_data* pRoom,int pos) {
	const char* p;
	char appo[256];
	for(p=real_roomp(pRoom->number)->specparms; pos>0; pos--) {
		p=one_argument(p,appo);
	}
	return(atoi(appo));
}


struct char_data* CloneChar(struct char_data* ch, long nroom) {

	int i,j;
	struct char_data* mob;
	struct obj_data* ocopy;
	char buf[512];

	CREATE(mob, struct char_data, 1);

	if (!mob) {
		mudlog( LOG_SYSERR, "Cannot create mob?! clonechar");
		return nullptr;
	}

	clear_char(mob);

	mob->specials.last_direction = -1;  /* this is a fix for wander */

	/***** String data *** */
	mob->player.name = strdup(GET_NAME(ch));
	sprintf(buf,"%s %s",GET_NAME(ch),ch->player.title);
	mob->player.short_descr = strdup(buf);
	mob->player.long_descr = strdup(GET_NAME(ch));
	sprintf(buf,"Sembra l'ombra di %s\n",GET_NAME(ch));
	mob->player.description = strdup(buf);
	mob->player.title=strdup(ch->player.title);

	/* *** Numeric data *** */

	mob->mult_att = ch->mult_att;
	mob->specials.spellfail = ch->specials.spellfail;

	SET_BIT(mob->specials.act, ACT_ISNPC);

	mob->specials.affected_by = ch->specials.affected_by;;

	mob->specials.alignment = ch->specials.alignment*-1;

	mob->player.iClass =ch->player.iClass;
	for (i=0; i<ABS_MAX_CLASS; i++)
	{ (mob)->player.level[(i)]=(ch)->player.level[i]; }

	mob->abilities   =  ch->abilities;


	mob->points=ch->points;
	mob->points.max_hit=ch->points.hit*10;
	mob->points.hit=mob->points.max_hit;
	mob->specials.position = POSITION_STANDING;

	mob->specials.default_pos = POSITION_STANDING;
	mob->player.sex = ch->player.sex;
	mob->immune = ch->immune;
	mob->M_immune = ch->M_immune;
	mob->susc = ch->susc;
	mob->player.time = ch->player.time;

	for (i = 0; i < 3; i++)
	{ GET_COND(mob, i) = -1; }

	for (i = 0; i < 5; i++)
		mob->specials.apply_saving_throw[i] =
			MAX(20-50, 2);
	mob->points.gold = 0;
	GET_EXP(mob) = (long)GET_EXP(ch)/100;
	GET_RACE(mob) = GET_RACE(ch);
	mob->player.weight = ch->player.weight;

	mob->player.height = ch->player.height;
	/* Set the damage as some standard 1d4 */
	mob->points.damroll = 0;
	mob->specials.damnodice = GetMaxLevel(mob);
	mob->specials.damsizedice = 2;


	mob->tmpabilities = mob->abilities;

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
	{ mob->equipment[i] = 0; }
	/* clone EQ equiped */
	if (ch->equipment) {
		for (j=0; j<MAX_WEAR; j++) {
			if (ch->equipment[j]) {
				/* clone mob->equipment[j] */
				if( ( ocopy = (struct obj_data*)clone_obj(ch->equipment[j] ) ) != NULL ) {
					if(ch->equipment[j]->contains) {
						clone_container_obj(ocopy,ch->equipment[j]);
					}
					equip_char(mob,ocopy,j);
				}
			}
		}
	}
	mob->nr = -1;

	mob->generic = 0;
	mob->commandp = 0;
	mob->commandp2 = 0;
	mob->waitp = 0;


	/* insert in list */

	mob->next = character_list;
	character_list = mob;


	/* set up things that all members of the race have */
	SetRacialStuff(mob);

	/* set up distributed movement system */

	mob->specials.tick = mob_tick_count++;

	if( mob_tick_count == TICK_WRAP_COUNT )
	{ mob_tick_count=0; }
	mob_count++;
	char_to_room(mob,nroom);
	mudlog( LOG_CHECK,
			"CLoned %s (ADDR: %p, magic %d, next %p, #mobs %ld).",
			GET_NAME( mob ), mob, mob->nMagicNumber, mob->next, mob_count );
	return(mob);
}
#define iif(p1,p2,p3)   (p1?p2:p3)
float GetCharBonusIndex(struct char_data* ch) {
	/* Calcola un indice in base al valore dell'equipaggiamento
	 * */
	long i;
	long j;
	long mod=0;
	long thismod=0;
	float indice=0.0;
	char buf[256];
	struct obj_data* item=NULL;
	for(i=0; i<MAX_WEAR; i++) {
		thismod=0;
		item=ch->equipment[i];
		if (item) {
			for(j=0; j<MAX_OBJ_AFFECT; j++) {

				mod=item->affected[j].modifier;
				sprinttype(item->affected[j].location,apply_types,buf);
				switch(item->affected[j].location) {

				/* File da Xanathon
				Per il resto credo questi valori siano quelli che reputo piu' opportuni:)
				Resta comunque aperta la questione di quanto sia la soglia di valore
				generale per gli eventuali bonus e malus sugli xp: per come era tarato
				sinora il mud io avrei detto statisticamente intorno a un indice di 1500, ma
				con le modifiche apportate non saprei, bisognerebbe tu lo montassi e mi
				dessi qualche giorno per fare le verifiche.
				E poi a quanto dovrebbe essere la "fluttuazione" degli xp? una prima soglia
				del 25% e una seconda del 50% o e' forse troppo?
				*/

				case APPLY_NONE:

				case APPLY_CHAR_WEIGHT :
				case APPLY_CHAR_HEIGHT:

				case APPLY_AFF2:
				case APPLY_RIDE:
				case APPLY_MOD_THIRST:
				case APPLY_MOD_HUNGER:
				case APPLY_MOD_DRUNK:
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
				case APPLY_STR:
					thismod+=mod*iif(IS_FIGHTER(ch),5,3);
					break;
				case APPLY_CON:
					thismod+=mod*iif(IS_FIGHTER(ch),8,5);
					break;
				case APPLY_DEX:
					thismod+=mod*iif(IS_THIEF(ch),10,8);
					break;
				case APPLY_INT:
					thismod+=mod*iif(IS_CASTER(ch),10,7);
					break;
				case APPLY_WIS:
					thismod+=mod*iif(IS_CASTER(ch),10,7);
					break;
				case APPLY_CHR:
					thismod+=mod*2;
					break;
				case APPLY_SEX:
					break;
				case APPLY_LEVEL:
					thismod+=mod*10;
					break;
				case APPLY_AGE:
					thismod-=mod*2;
					break;
				case APPLY_MANA:
					thismod+=mod*iif(IS_CASTER(ch),3,1);
					break;
				case APPLY_HIT:
					thismod+=mod*iif(IS_FIGHTER(ch),8,6);
					break;
				case APPLY_MOVE:
					thismod+=mod*1;
					break;
				case APPLY_GOLD:
					break;
				case APPLY_AC:
					thismod-=mod*2;
					break;
				case APPLY_HITROLL:
					thismod+=iif(IS_FIGHTER(ch),10,8);
					break;
				case APPLY_DAMROLL:
					thismod+=iif(IS_FIGHTER(ch),20,15);
					break;
				case APPLY_SAVING_PARA:
					thismod+=mod*2;
					break;
				case APPLY_SAVING_ROD:
					thismod+=mod*2;
					break;
				case APPLY_SAVING_PETRI:
					thismod+=mod*2;
					break;
				case APPLY_SAVING_BREATH:
					thismod+=mod*2;
					break;
				case APPLY_SAVING_SPELL:
					thismod+=mod*2;
					break;
				case APPLY_SAVE_ALL:
					thismod-=mod*5;
					break;
				case APPLY_IMMUNE:
					thismod+=100;
					break;
				case APPLY_SUSC:
					thismod-=100;
					break;
				case APPLY_M_IMMUNE:
					thismod+=150;
					break;
				case APPLY_SPELL:
					thismod+=1;
					break;
				case APPLY_WEAPON_SPELL:
					thismod+=15;
					break;
				case APPLY_EAT_SPELL:
					thismod+=1;
					break;
				case APPLY_FIND_TRAPS:
				case APPLY_BACKSTAB:
				case APPLY_PICK:
				case APPLY_STEAL:
					thismod+=mod*iif(IS_THIEF(ch),5,0);
					break;
				case APPLY_KICK:
					thismod+=mod*iif(IS_SPY(ch),8,2);
					break;
				case APPLY_BASH:
					thismod+=mod*iif(IS_FIGHTER(ch),10,2);
					break;
				case APPLY_SNEAK:
					thismod+=mod*iif(IS_SPY(ch),5,20);
					break;
				case APPLY_HIDE:
					thismod+=mod*iif(IS_SPY(ch),5,20);
					break;
				case APPLY_TRACK:
					thismod+=5;
					break;
				case APPLY_HITNDAM:
					thismod+=mod*iif(IS_FIGHTER(ch),30,23);
					break;
				case APPLY_SPELLFAIL:
					thismod-=mod*iif(IS_CASTER(ch),5,1);
					break;
				case APPLY_ATTACKS:
				case APPLY_HASTE:
					thismod+=mod*iif(IS_FIGHTER(ch),10,5);
					break;
				case APPLY_SLOW:
					thismod-=mod*iif(IS_FIGHTER(ch),10,2);
					break;
				case APPLY_RACE_SLAYER:
					thismod-=iif(IS_FIGHTER(ch),100,60);
					break;
				case APPLY_ALIGN_SLAYER:
					thismod-=iif(IS_FIGHTER(ch),100,60);
					break;
				case APPLY_MANA_REGEN:
					thismod+=mod*iif(IS_CASTER(ch),10,1);
					break;
				case APPLY_HIT_REGEN:
					thismod+=mod*iif(IS_FIGHTER(ch),8,5);
					break;
				case APPLY_MOVE_REGEN:
					thismod+=mod*3;
					break;
				}
			}
			indice+=thismod;
		}
	}
	return indice;
}
ACTION_FUNC(do_setalign) {
	long oldalign;
	long gold;
	gold=MAX(0,(GetMaxLevel(ch)-1) * 20000 * HowManyClasses(ch));
	if (GET_EXP(ch)< gold) {
		send_to_char("Non hai abbastanza esperienza per cambiare align\n\r",ch);
		send_to_char("Gli allineamenti possibili sono solo Evil, Neutral e Good\n\r",ch);
	}
	else {
		oldalign=GET_ALIGNMENT(ch);
		if (!strcasecmp(arg,"evil"))
		{ GET_ALIGNMENT(ch)=-1000; }
		else if (!strcasecmp(arg,"neutral"))
		{ GET_ALIGNMENT(ch)=0; }
		else if (!strcasecmp(arg,"good"))
		{ GET_ALIGNMENT(ch)=1000; }
		else
		{ send_to_char("Gli allineamenti possibili sono solo Evil, Neutral e Good\n\r",ch); }
		if (oldalign != GET_ALIGNMENT(ch))
		{ GET_EXP(ch)-=gold; }
	}
	return;
}


} // namespace Alarmud

