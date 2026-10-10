/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
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
#include "spec_nebbiearcane.hpp"
#include "act.comm.hpp"
#include "act.off.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magic3.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spell_parser.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "utility.hpp"
namespace Alarmud {

/* Nebbie Arcane: Thanatos, Signora Bambole, Nightmare, BossKill, LibroEroi. */

/* Piccola macro per non riempire il codice di if facendo degli strcmp */
#define STRSWITCH     if( false ){
#define CHECK(X,Y)    } else if ( strcmp((X),(Y)) == 0 ) {
#define END           }

MOBSPECIAL_FUNC(BiosKaiThanatos) {
#define MIN_WEARING 10
	struct char_data* god;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	god = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(BiosKaiThanatos));

	if(!god) {
		return(FALSE);
	}
	switch(cmd) {
	case CMD_SACRIFICE :
		break;
	case CMD_GIVE :
	case CMD_DROP :
	case CMD_REMOVE :
		if(!IS_MAESTRO_DEL_CREATO(ch)) {
			act("Non davanti a $N!", FALSE,
				ch, 0, god, TO_CHAR);
			return(TRUE);
		}
		else {
			return(FALSE);
		}
	default:
		break;
	}

	if(cmd != CMD_SACRIFICE) {
		return(FALSE);
	}

	if(IS_NPC(ch)) {
		act("$N ti dice 'Non hai dunque il coraggio di mostrare il tuo vero volto?'",
			FALSE,ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		return(TRUE);
	}
	int dummy,result=0;
	WEARING_N(ch,dummy,result);
	if(result<MIN_WEARING) {
		act("$N ti dice 'Torna da me vestito dei tuoi abiti migliori'",
			FALSE,ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		return(TRUE);
	}
	long snap_exp = 0;
	long snap_at = 0;
	if(!death_snapshot_load(GET_NAME(ch), snap_exp, snap_at)) {
		act("$N ti dice 'Impostore! Vattene via subito!'", FALSE,
			ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		mudlog(LOG_CHECK, "No dead snapshot for %s", GET_NAME(ch));
		return(TRUE);
	}

	if(((long)time(0) - snap_at) > (4 * 60 * 60)) {

		act("$N ti dice 'Troppo tardi! Il tuo karma si e' compiuto.'", FALSE,
			ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		mudlog(LOG_CHECK, "%s: to late on sacrifice", GET_NAME(ch));
		return(TRUE);
	}
	if((GET_RCON(ch)<=3) && (number(1,100)<90)) {

		act("$N ti dice 'Troppe volte hai danzato!'", FALSE,
			ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		mudlog(LOG_CHECK, "%s: no CON on sacrifice", GET_NAME(ch));
		return(TRUE);
	}
	const int restored_exp = static_cast<int>(snap_exp);
	GET_RCON(ch)=MAX(GET_RCON(ch)-1,3);
	act("$N ti dice 'Il karma e' stato benevolo con te!'", FALSE,
		ch, 0, god, TO_CHAR);
	act("$N ti dice 'Il tuo sacrificio e' stato accettato!'", FALSE,
		ch, 0, god, TO_CHAR);
	act("$N dice qualcosa a $n", FALSE,
		ch, 0, god, TO_ROOM);
	GET_EXP(ch)=restored_exp;

	act("$c0001Senti le viscere rivoltarsi, mentre energie sconosciute ti strappano l'anima", FALSE,
		ch, 0, god, TO_CHAR);
	act("$n si solleva da terra e inizia a brillare.", FALSE,
		ch, 0, god, TO_ROOM);
	for(int dmg_type = TYPE_GENERIC_FIRST; dmg_type <= TYPE_GENERIC_LAST; dmg_type++) {
#if NOSCRAP
		DamageStuff(ch, dmg_type, 200, 5);
		DamageStuff(ch, dmg_type, 200, 5);
		DamageStuff(ch, dmg_type, 200, 5);
#endif
		DamageStuff(ch, dmg_type, 200, 5);
	}
	act("$c0001L'urlo di mille gole sgozzate ti assorda", FALSE,
		ch, 0, god, TO_CHAR);
	act("$n si contorce come in agonia, mentre un vento impetuoso si alza dal nulla.", FALSE,
		ch, 0, god, TO_ROOM);
	GET_HIT(ch)=MIN(GET_HIT(ch),10);
	alter_hit(ch,0);
	for(int dmg_type = TYPE_GENERIC_FIRST; dmg_type <= TYPE_GENERIC_LAST; dmg_type++) {
#if NOSCRAP
		DamageStuff(ch, dmg_type, 200, 5);
		DamageStuff(ch, dmg_type, 200, 5);
		DamageStuff(ch, dmg_type, 200, 5);
#endif
		DamageStuff(ch, dmg_type, 200, 5);
	}
	act("Alla fine, giaci a terra spossat$b", FALSE,
		ch, 0, god, TO_CHAR);
	act("$n cade a terra spossato.", FALSE,
		ch, 0, god, TO_ROOM);
	GET_POS(ch)=POSITION_STUNNED;
	mudlog(LOG_CHECK, "%s: sacrifice accepted", GET_NAME(ch));
	return TRUE;
}

MOBSPECIAL_FUNC(BossKill) {
	if(type == EVENT_DEATH) {
		struct char_data* p;
        struct follow_type* f;
        struct char_data* k;
        char buf[MAX_STRING_LENGTH];
        int premio = ((GET_LEVEL(ch, WARRIOR_LEVEL_IND) - 50)/3)+1;

        if(premio <= 0) {
            sprintf(buf,"Il liv di %s[%d] e' troppo basso per pagare in rune.",GET_NAME(ch),ch->nr);
            mudlog(LOG_SYSERR, buf);
            return FALSE;
        }

        if(!real_roomp(ch->in_room)) {
            return FALSE;
        }

        for(p = real_roomp(ch->in_room)->people; p; p=p->next_in_room) {
            if(p->lastmkill != NULL && strstr(p->lastmkill, GET_NAME(ch))) {

                if(p->master) {
                    k = p->master;
                }
                else {
                    k = p;
                }

                if(premio > 1) {
                    sprintf(buf,"\n\r$c0011Ottieni %d rune degli Dei per la tua impresa!$c0007\n\r", premio);
                } else {
                    sprintf(buf,"\n\r$c0011Ottieni una runa degli Dei per la tua impresa!$c0007\n\r");
                }

                GET_RUNEDEI(k) += premio;
                send_to_char(buf, k);

                if(k->followers) {
                    for(f=k->followers; f; f=f->next) {
                        if(IS_AFFECTED(f->follower, AFF_GROUP)) {
                            if(!f->follower->desc) {
                                /* link dead */
                            }
                            else {
                                GET_RUNEDEI(f->follower) += premio;
                                send_to_char(buf, f->follower);
                            }
                        }
                    }
                }
                return TRUE;
            }
        }
    }
	return FALSE;
}

MOBSPECIAL_FUNC(LibroEroi) {
	char par2[256], runa[256], buf[128], mail[]=STAFF_EMAIL;
	struct obj_data* obj;
	int i,number,num2,xp,gold, max,xpcum,k,trueGain;
	short chrace;

	/*if( GetMaxLevel(ch)>51)
	 return true;
	*/
	num2=0;

	if(type == EVENT_COMMAND && cmd == CMD_SAY) {
		const auto [runaStr, par2Str] = chop_argument(arg, sizeof(runa) - 1, sizeof(par2) - 1);
		std::strncpy(runa, runaStr.c_str(), sizeof(runa) - 1);
		runa[sizeof(runa) - 1] = '\0';
		std::strncpy(par2, par2Str.c_str(), sizeof(par2) - 1);
		par2[sizeof(par2) - 1] = '\0';
		if(isdigit(static_cast<unsigned char>(par2[0]))) {
			num2=atoi(par2);
		}


		STRSWITCH
		CHECK("ael",runa)
		if(GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> SANCTUARY");
			act("Reciti solennemente la parole del Potere \"AEL\".\r\nLe rune che la compongono si illuminano mentre cominciano a bruciare\r\nsulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("Una colonna di luce ti colpisce! Senti un piacevole calore pervadere il tuo corpo!\r\nLentamente la colonna di luce si dissolve, sbiadendo fino a lasciarti un aura bianchissima\r\nattorno al corpo: sai di essere sotto la protezione della luce degli dei!\r\n",
				false,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"AEL\". La sua carne sembra bruciare,\r\nmentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("$N viene avvolt$b da una colonna di luce accecante. Mentre la luce sbiadisce\r\n e riesci a rimettere a fuoco la stanza, vedi che $N e' circondat$b da una intensa aura bianca.\r\n",FALSE,ch,0,ch,
				TO_ROOM);
			spell_sanctuary(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 4;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 4\"| mail -s \"ESECUZIONE RUNE --> Sanctuary\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,mob,0,ch,TO_NOTVICT);
			return true;
		}
		CHECK("inen",runa)
		if(GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> IDENTIFY");
			act("Reciti solennemente la parole del Potere \"INEN\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono\r\nfino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"INEN\". La sua carne sembra bruciare,\r\nmentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			for(i=0; i<3; i++) {
				number = real_object(32992);
				obj = read_object(number, REAL);
				obj_to_room(obj,ch->in_room);
			}
			act("Una esplosione di luce invade la stanza, quando si disperde\r\n vedi che per terra sono comparsi tre strani oggetti simili all'occhio di un drago...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Una esplosione di luce invade la stanza, quando si disperde\r\n vedi che per terra sono comparsi tre strani oggetti simili all'occhio di un drago...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Il Sacerdote parla: \"Usa l'Occhio del Drago per conoscere le virtu' di un oggetto!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Il Sacerdote parla: \"Usa l'Occhio del Drago per conoscere le virtu' di un oggetto!\"\r\n",FALSE,ch,0,ch,TO_ROOM);
			GET_RUNEDEI(ch) -= 4;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 4\"| mail -s \"ESECUZIONE RUNE --> Identify\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("ghia",runa)
		if(GET_RUNEDEI(ch)>=1) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> CREATE FOOD");
			act("Reciti solennemente la parole del Potere \"GHIA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GHIA\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			for(i=0; i<5; i++) {
				number = real_object(32991);
				obj = read_object(number, REAL);
				obj_to_room(obj,ch->in_room);
			}
			act("Una esplosione di luce invade la stanza, quando si disperde vedi che per terra\r\nsono comparse cinque coppe colme di un liquido denso e profumato...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Una esplosione di luce invade la stanza, quando si disperde vedi che per terra\r\nsono comparse cinque coppe colme di un liquido denso e profumato...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Il Sacerdote parla: \"Quando sarai affamato, potrei nutrirti col nettare degli dei\r\ncontenuto nelle coppe!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Il Sacerdote parla: \"Quando sarai affamato, potrei nutrirti col nettare degli dei\r\ncontenuto nelle coppe!\"\r\n",FALSE,ch,0,ch,TO_ROOM);
			GET_RUNEDEI(ch) -= 1;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 1\"| mail -s \"ESECUZIONE RUNE --> Create Food\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("zir",runa)
		if(GET_RUNEDEI(ch)>=20) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> FIRESHIELD");
			act("Reciti solennemente la parole del Potere \"ZIR\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ZIR\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Cominci a bruciare come una torcia, ma i lembi di fuoco che ti circondano non ti provocano\r\nalcun dolore, anzi, ti danno un gran senso di protezione!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N viene avvolt$b da possenti fiamme, ma sembra essere in grado di controllarle!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_fireshield(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 20;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 20\"| mail -s \"ESECUZIONE RUNE --> FireShield\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("sidamishida",runa)
		if(GET_RUNEDEI(ch)>=8) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> TREETRAVEL");
			act("Reciti solennemente la parole del Potere \"SIDAMISHIDA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"SIDAMISHIDA\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Ti senti in completa comunione con la natura: ora sai di poter viaggiare utilizzando\r\nle sacre vie dei druidi!\r\n",FALSE,mob,0,ch,TO_VICT);
			spell_tree_travel(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 8;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 8\"| mail -s \"ESECUZIONE RUNE --> TreeTravel\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("elu",runa)
		if(GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> WATER BREATH");
			act("Reciti solennemente la parole del Potere \"ELU\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ELU\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Per un attimo ti senti strozzare...\n\r...ti manca il fiato...\r\n...pensi di morire...\r\n ma ad un tratto tutto passa e senti di poter respirare ovunque!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N Strabuzza gli occhi e si tienen la gola: SEMBRA SOFFOCARE!! Ad un tratto inspira profondamente e tutto sembra passato..\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_water_breath(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 4;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 4\"| mail -s \"ESECUZIONE RUNE --> Water Breath\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("tide",runa)
		if(GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> STRENGTH");
			act("Reciti solennemente la parole del Potere \"TIDE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"TIDE\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Ti senti pieno di vigore. I tuoi muscoli si gonfiano e tutto quello che porti ti sembra piu' leggero!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N si erge in tutta la sua potenza ed i suoi muscoli si gonfiano!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_strength(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 4;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 4\"| mail -s \"ESECUZIONE RUNE --> Strength\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("fuel",runa)
		if(GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> MINOR TRACK");
			act("Reciti solennemente la parole del Potere \"FUEL\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"FUEL\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti una voce: \"Concentrati sulla tua preda e questa non ti potra' sfuggire!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Gli occhi di $N sono attraversati da un lampo di luce!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_track(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 4;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 4\"| mail -s \"ESECUZIONE RUNE --> Minor Track\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("beio",runa)
		if(GET_RUNEDEI(ch)>=6) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> MAJOR TRACK");
			act("Reciti solennemente la parole del Potere \"BEIO\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"BEIO\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti una voce: \"Concentrati sulla tua preda e questa non ti potra' sfuggire!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Gli occhi di $N sono attraversati da un lampo di luce!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_track(52,ch,ch,NULL);
			GET_RUNEDEI(ch) -= 6;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 6\"| mail -s \"ESECUZIONE RUNE --> Major Track\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("aelgud",runa)
		if(GET_RUNEDEI(ch)>=3) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT DRAG BREATH");
			act("Reciti solennemente la parole del Potere \"AELGUD\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"AELGUD\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: un possente drago ti soffia addosso tutta la sua rabbia,\r\nma tu non vacilli e passi indenne!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_dragon_breath(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 3;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 3\"| mail -s \"ESECUZIONE RUNE --> Prot Dragon Breath\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("gudorizir",runa)
		if(GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT FIRE");
			act("Reciti solennemente la parole del Potere \"GUDORIZIR\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORIZIR\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: vieni colpito in pieno da una enorme palla infuocata,\r\nma tu non vacilli e passi indenne!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_fire(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 2;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 2\"| mail -s \"ESECUZIONE RUNE --> Prot Fire\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("gudorishaff",runa)
		if(GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT ELECTRICITY");
			act("Reciti solennemente la parole del Potere \"GUDORISHAFF\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORISHAFF\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: un fulmine esplode nel cielo sereno e ti colpisce in pieno,\r\nma tu non vacilli e passi indenne!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_elec(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 2;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 2\"| mail -s \"ESECUZIONE RUNE --> pROT eLECTRICITY\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("gudorilakra",runa)
		if(GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT COLD");
			act("Reciti solennemente la parole del Potere \"GUDORILAKRA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORILAKRA\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: un vento gelido si alza distruggendo\r\ntutto quello che colpisce con lame di ghiaccio e sei nel centro di questa bufera, ma tu non vacilli e passi indenne!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_cold(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 2;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 2\"| mail -s \"ESECUZIONE RUNE --> Prot Cold\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("gudorielei",runa)
		if(GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT ENERGY");
			act("Reciti solennemente la parole del Potere \"GUDORIELEI\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORIELEI\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: una strana nube attraversata da fulmini multicolore avanza verso di te distruggendo tutto quello che incontra fino ad inglobarti al suo interno, ma tu non vacilli e passi indenne!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_energy(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 2;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 2\"| mail -s \"ESECUZIONE RUNE --> Prot Energy\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("iaeelia",runa)
		if(GET_RUNEDEI(ch)>=10) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> TS");
			act("Reciti solennemente la parole del Potere \"IAEELIA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"IAEELIA\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Un dolore lancinante ti attraversa la testa ed hai come l'impressione che i tuoi occhi stiano per esplodere...\r\nMa tutto passa presto e sai che ora nulla sara' celato alla tua Vista!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... ad un tratto tutto passa ed i suoi occhi brillano di una strana luce azzurrina!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_true_seeing(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 10;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 10\"| mail -s \"ESECUZIONE RUNE --> TS\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("elia",runa)
		if(GET_RUNEDEI(ch)>=8) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> DETECT INVI");
			act("Reciti solennemente la parole del Potere \"ELIA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ELIA\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Un dolore lancinante ti attraversa la testa ed hai come l'impressione che i tuoi occhi stiano per esplodere...\r\nMa tutto passa presto e sai che ora la magia non potra' piu' nascodere qualcuno o qualcosa alla tua Vista!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... ad un tratto tutto passa ed i suoi occhi brillano di una strana luce porpora!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_detect_invisibility(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 8;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 8\"| mail -s \"ESECUZIONE RUNE --> TreeTravel\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("ene",runa)
		if(GET_RUNEDEI(ch)>=num2) {

			if(num2<=0||!num2) {
				return true;
			}

			xp=num2*(GetMaxLevel(ch))*10000;
			xpcum=0;
			k=0;
			max=0;

			if(ch->desc && ch->desc->original) {
				chrace = ch->desc->original->race;
			}
			else {
				chrace = GET_RACE(ch);
			}

			/* Calcolo il maxxaggio */
			for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
				if(GET_LEVEL(ch, i) && GET_LEVEL(ch, i) < RacialMax[ chrace ][ i ]) {
					mudlog(LOG_SYSERR,"Non sono al massimo razziale");
					if(GET_LEVEL(ch, i)!=0) {
						k=(titles[i][ GET_LEVEL(ch, i) + 2 ].exp)-1;
						if(xpcum == 0 || k < xpcum) {
							xpcum=k;
							mudlog(LOG_SYSERR,"maxxaggio a %d",k);
						}
					}
				}
			}
			k=HowManyClasses(ch);
			trueGain=GET_EXP(ch)+(xp/k);
			/* Se si tratta di un principe accediamo comunque al gain completo, ignorando il maxxaggio */
			if(trueGain <= xpcum || GET_EXP(ch)>=PRINCEEXP) {
				/* Check sull'owerflow per calcolar ele rune da spendere */
				if((GET_EXP(ch)+(xp/k))<0) {
					max=(MAX_XP-GET_EXP(ch))/(GetMaxLevel(ch)*10000);
					if(max>0) {
						act("Reciti solennemente la parole del Potere \"ENE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
							FALSE,mob,0,ch,TO_VICT);
						act("Il sacerdote ti dice \"Capisco il tuo desiderio di sapere, ma per volere degli ho considerato solo una parte delle Rune che volevi consacrare agli Dei!\"\r\n",FALSE,mob,0,ch,TO_VICT);
						act("$N declama con voce imponente la parola \"ENE\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
						act("Strane immagini vorticano davanti ai tuoi occhi... Stai rivivendo le epiche gesta di Eroi di antico passato!\r\nQuando la tua visione termina, e' come se TU abbia vissuto in prima persona quelle avventure!\r\n",
							FALSE,mob,0,ch,TO_VICT);
						act("$N trema scosso da brividi, in preda ad una strana trance mistica, ma in un lungo istante tutto cio' passa...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
						xp=max*(GetMaxLevel(ch))*10000;
						GET_RUNEDEI(ch) -= max;
						gain_exp(ch, xp);
						mudlog(LOG_PLAYERS, "GAIN PARZIALE PRINCIPI Rune spese (num2)=%d, spendibili=%d, guadagna %d xp",num2,max,xp);
						mudlog(LOG_PLAYERS, "esecuzione rune --> assegno %d PX a %s",xp,GET_NAME(ch));
						sprintf(buf,"echo \"PC: %s RUNE SPESE: %d\"| mail -s \"ESECUZIONE RUNE --> Conv XP\" %s", GET_NAME(ch),max,mail);
						system(buf);
						return true;
					}
					else {
						act("Non puoi apprendere cosi tanto!!\r\n",FALSE,mob,0,ch,TO_VICT);
						act("$N declama le rune, ma non accade nulla!\r\n",FALSE,mob,0,ch,TO_NOTVICT);
						mudlog(LOG_PLAYERS, "esecuzione rune --> %s non pu� convertire rune ulteriormente (overflow principi)",GET_NAME(ch));
						return true;
					}
					return true;
				}
				GET_RUNEDEI(ch) -= num2;
				act("Reciti solennemente la parole del Potere \"ENE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
					FALSE,mob,0,ch,TO_VICT);
				act("$N declama con voce imponente la parola \"ENE\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
				act("Strane immagini vorticano davanti ai tuoi occhi... Stai rivivendo le epiche gesta di Eroi di antico passato!\r\nQuando la tua visione termina, e' come se TU abbia vissuto in prima persona quelle avventure!\r\n",
					FALSE,mob,0,ch,TO_VICT);
				act("$N trema scosso da brividi, in preda ad una strana trance mistica, ma in un lungo istante tutto cio' passa...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
				gain_exp(ch, xp);
				mudlog(LOG_PLAYERS, "GAIN COMPLETO Rune spese (num2)=%d, guadagna %d PX",num2,xp);
				mudlog(LOG_PLAYERS, "esecuzione rune --> assegno %d PX a %s",xp,GET_NAME(ch));
				sprintf(buf,"echo \"PC: %s RUNE SPESE: %d\"| mail -s \"ESECUZIONE RUNE --> Conv XP\" %s", GET_NAME(ch),num2,mail);
				system(buf);
				return true;
			}
			else {
				max=(xpcum-GET_EXP(ch))*k/(GetMaxLevel(ch)*10000);
				if(max>0) {
					mudlog(LOG_PLAYERS, "GAIN PARZIALE Rune spese (num2)=%d Spendibili %d",num2,max);
					xp=max*(GetMaxLevel(ch))*10000;
					act("Reciti solennemente la parole del Potere \"ENE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
						FALSE,mob,0,ch,TO_VICT);
					act("Il sacerdote ti dice \"Capisco il tuo desiderio di sapere, ma per volere degli ho considerato solo una parte delle Rune che volevi consacrare agli Dei!\"\r\n",FALSE,mob,0,ch,TO_VICT);
					act("$N declama con voce imponente la parola \"ENE\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
					act("Strane immagini vorticano davanti ai tuoi occhi... Stai rivivendo le epiche gesta di Eroi di antico passato!\r\nQuando la tua visione termina, � come se TU abbia vissuto in prima persona quelle avventure!\r\n",
						FALSE,mob,0,ch,TO_VICT);
					act("$N trema scosso da brividi, in preda ad una strana trance mistica, ma in un lungo istante tutto cio' passa...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
					GET_RUNEDEI(ch) -= max;
					gain_exp(ch, xp);
					mudlog(LOG_PLAYERS, "GAIN PARZIALE Rune spese (num2)=%d, spendibili=%d, guadagna %d xp",num2,max,xp);
					mudlog(LOG_PLAYERS, "esecuzione rune --> assegno %d PX a %s",xp,GET_NAME(ch));
					sprintf(buf,"echo \"PC: %s RUNE SPESE: %d\"| mail -s \"ESECUZIONE RUNE --> Conv XP\" %s", GET_NAME(ch),max,mail);
					system(buf);
					return true;
				}
				else {
					act("Non puoi apprendere cosi tanto!!\r\n",FALSE,mob,0,ch,TO_VICT);
					act("$N declama le rune, ma non accade nulla!\r\n",FALSE,mob,0,ch,TO_NOTVICT);
					mudlog(LOG_PLAYERS, "esecuzione rune --> %s non pu� convertire rune ulteriormente (maxxaggio classe)",GET_NAME(ch));
					return true;
				}
			}
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,mob,0,ch,TO_NOTVICT);
			mudlog(LOG_PLAYERS, "esecuzione rune --> %s vuole convertire %d rune ma ne ha solo %d",GET_NAME(ch),num2,GET_RUNEDEI(ch));
			return true;
		}
		CHECK("ane",runa)
		if(GET_RUNEDEI(ch)>=num2 && num2>0) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> COINS");
			act("Reciti solennemente la parole del Potere \"ANE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ANE\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Le tue tasche si fanno piu' pesanti, mentre una voce nella tua testa ti dice \"Premio la tua avidita' eroe! Oppure vuoi usare le tue ricchezze per il bene degli altri?\"\r\n",FALSE,mob,0,ch,
				TO_VICT);
			act("Le tasche di $N si gonfiano improvvisamente!\r\n",FALSE,ch,0,ch,TO_ROOM);
			gold=num2*10000;
			mudlog(LOG_PLAYERS, "esecuzione rune --> assegno %d coins a %s",gold,GET_NAME(ch));
			GET_GOLD(ch)+=gold;
			GET_RUNEDEI(ch) -= num2;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: %d\"| mail -s \"ESECUZIONE RUNE --> Conv Coins\" %s", GET_NAME(ch),num2,mail);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("elei",runa)
		if(GET_RUNEDEI(ch)>=1) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> CURE BLINDNESS");
			act("Reciti solennemente la parole del Potere \"ELEI\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ELEI\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Un dolore lancinante ti attraversa la testa ed hai come l'impressione che i tuoi occhi stiano per esplodere...\r\nMa tutto passa presto e quando riapri gli occhi, ti accorgi di poter vedere di nuovo!\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... Un bagliore attraversa i suoi occhi e poi si spegne\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_cure_blind(52, ch, ch,0);
			GET_RUNEDEI(ch) -= 1;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 1\"| mail -s \"ESECUZIONE RUNE --> Cure Blind\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK("itel",runa)
		if(GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> REMOVE PARALYSIS");
			act("Reciti solennemente la parole del Potere \"ITEL\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",
				FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ITEL\". La sua carne sembra bruciare, mentre le rune che aveva tatuate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Il tuo corpo brucia pervaso da un immenso calore... Quando questa sensazione passa, ti accorgi di essere di nuovo padrone dei tuoi movimenti.\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... ad un tratto tutto passa ed il suo corpo si muove di nuovo!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_remove_paralysis(52,ch,ch,0);
			GET_RUNEDEI(ch) -= 4;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: 1\"| mail -s \"ESECUZIONE RUNE --> Remove Paral\" %s", GET_NAME(ch),mail);
			mudlog(LOG_PLAYERS,buf);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}

		END

	}

	return false;
}

MOBSPECIAL_FUNC(Vampire_Summoner) {
	const char* p;
	char nmob[256], buf[400], target[100];
	int nummob;
	struct char_data* mobtmp;
    
    p=GET_SPEC_PARM(mob);
    p=one_argument(p,nmob);
    nummob=atoi(nmob);
    
    if(cmd) {
        return(FALSE);
    }
    if(!AWAKE(mob)) {
        return(FALSE);
    }

	if(mob->specials.fighting)
    {
        mobtmp = mob->specials.fighting;
		// Ok, controllo se sta combattendo contro i suoi servitori
        if(IS_MOB(mobtmp))
        {
            if(mob_index[mobtmp->nr].iVNum == nummob)
            {
					// Ok, � una delle bambole che sta combattendo
					// Vediamo di eliminarla allora
                do_say(mob, "Vattene subito serva infedele!", 0);
                act("$c0011$N$c0011 scompare subito dopo aver udito l'ordine di $n$c0011!", FALSE, mob, 0, mobtmp, TO_ROOM);
                extract_char(mobtmp);
                return FALSE;
            }
        }

        switch(number(0,9))
        {
            case 1:
                if(!(IS_SET((mob->specials.fighting)->M_immune, IMM_DRAIN)))
                {
                    do_say(mob, "Voglio la tua energia vitale!!!", 0);
                    cast_energy_drain(50, mob, "", SPELL_TYPE_SPELL, mob->specials.fighting, 0);
                }
                else
                    return(magic_user(mob,cmd,arg,mob,type));
                break;
            case 2:
                return(magic_user(mob,cmd,arg,mob,type));
                break;
            case 3:
                return(cleric(ch,cmd,arg,mob,type));
                break;
            case 4:
                if(!(IS_SET((mob->specials.fighting)->M_immune, IMM_DRAIN)))
                {
                    do_say(mob, "Voglio la tua energia vitale!!!", 0);
                    cast_energy_drain(50, mob, "", SPELL_TYPE_SPELL, mob->specials.fighting, 0);
                }
                else
                    return(magic_user(mob,cmd,arg,mob,type));
                break;
            default:
                // Se ha troppi mob servitori evita di evocarne altri
                if(!too_many_followers(mob))
                {
                    act("$c0013$n$c0013 chiama una bambola in sua difesa!", FALSE, mob, 0, 0, TO_ROOM);
                    act("$c0011Una bambola si anima e corre in soccorso di $n!", FALSE, mob, 0, 0, TO_ROOM);
                    
                    mobtmp = read_mobile(real_mobile(nummob),REAL);
                    mudlog(LOG_MOBILES,"mobtmp %s", GET_NAME(mobtmp));
                    char_to_room(mobtmp, mob->in_room);
                    mudlog(LOG_MOBILES,"mob %s", GET_NAME(mob));

                    add_follower(mobtmp, mob);
                    SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);

                    AddHated(mobtmp,mob->specials.fighting);
                    do_order(mob, "followers guard on", 0);
                    one_argument(GET_NAME(mob->specials.fighting), target);
                    sprintf(buf, "followers kill %s", target);
                    do_order(ch, buf, 0);
                }
        }
        
        return TRUE;
    }
    
	return FALSE;
}

MOBSPECIAL_FUNC(bambola)
{
    struct char_data* tch;
    char target[100];
    
    if(cmd)
    {
        return (FALSE);
    }
    
    if(type == EVENT_DEATH)
    {
        if((ch->master) && ch->in_room == (ch->master)->in_room)
        {
            do_say(ch, "Mia... Signora... Perdonami...", 0);
            act("$N si avvicina $n e $d dice $c0013'Mi hai servito bene!'", FALSE, ch, 0, (ch->master), TO_ROOM);
            act("$c0011$N$c0011 si getta su $n$c0011 ed assorbe le sue ultime energie vitali.\n\r", FALSE, ch, 0, (ch->master), TO_ROOM);
            if(GET_HIT(ch->master) < (GET_MAX_HIT(ch->master)-30))
                (ch->master)->points.hit += dice(1, 51) + 99;
            (ch->master)->points.max_hit += dice(2,5)*(ch->master)->points.max_hit/100;
            if ((ch->master)->points.max_hit > 25000)
                (ch->master)->points.max_hit = 25000;
            alter_hit(ch->master,0);
        }
        return (TRUE);
    }
    
    if(!ch->specials.fighting)
    {
        for(tch = real_roomp(mob->in_room)->people; tch; tch = tch->next_in_room)
        {
            if(IS_MOB(tch) && IS_MOB(ch) && mob_index[tch->nr].iVNum == mob_index[ch->nr].iVNum)
            {
                if(tch->specials.fighting)
                {
                    act("$c0015[$c0013$n$c0015] ruggisce 'Morte ai nemici delle bambole!'", FALSE, ch, 0, 0, TO_ROOM);
                    hit(ch, tch->specials.fighting, TYPE_HIT);
                    return (FALSE);
                }
            }
        }
        
        if((ch->master) && ch->in_room == (ch->master)->in_room)
        {
            if(!ch->skills)
            {
                SpaceForSkills(ch);
            }
            
            if(!HasActiveSanctuary(ch->master))
            {
                if(!ch->skills[SPELL_SANCTUARY].learned)
                {
                    ch->skills[SPELL_SANCTUARY].learned = GetMaxLevel(ch)*3+30;
                }
                act("$n pronuncia le parole, '$c0015Possa la mia aura proteggerti, mia Signora.$c0007'.", 1, ch, 0, 0, TO_ROOM);
                cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->master, 0);
            }
            else if(GET_HIT(ch->master) < (GET_MAX_HIT(ch->master)-30))
            {
                if(!ch->skills[SPELL_HEAL].learned)
                {
                    ch->skills[SPELL_HEAL].learned = GetMaxLevel(ch)*3+30;
                }
                act("$n pronuncia le parole, '$c0015Attenta mia Signora!$c0007'.", TRUE, ch, 0, 0, TO_ROOM);
                cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->master, 0);
            }
            else if((ch->master)->specials.fighting)
            {
                if(!ch->skills[SPELL_HARM].learned)
                {
                    ch->skills[SPELL_HARM].learned = GetMaxLevel(ch)*3+30;
                }
                act("$n pronuncia le parole, '$c0015Fa male, vero?$c0007'.", TRUE, ch, 0, 0, TO_ROOM);
                cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, (ch->master)->specials.fighting, 0);
            }
        }
        
        if(number(0,5) == 0)
        {
            if(!(ch->master))
            {
                act("$c0015$n$c0015 si rimette al suo posto.\n\r", FALSE, ch, 0, 0, TO_ROOM);
                extract_char(ch);
                return (FALSE);
            }
            else if(!((ch->master)->specials.fighting))
            {
                act("$c0013[$c0015$N$c0013] indica $n$c0013 e $d dice 'Puoi andare, non mi servi piu'!'", FALSE, ch, 0, (ch->master), TO_ROOM);
                act("$c0015$n$c0015 fa un inchino a $N$c0015 poi si rimette al suo posto.\n\r", FALSE, ch, 0, (ch->master), TO_ROOM);
                (ch->master)->points.max_hit += dice(2,4)*(ch->master)->points.max_hit/100;
                if ((ch->master)->points.max_hit > 25000)
                    (ch->master)->points.max_hit = 25000;
                alter_hit(ch->master,0);
                extract_char(ch);
                return (FALSE);
            }
            else if((ch->master)->specials.fighting)
            {
                act("$c0013[$c0015$N$c0013] dice 'MORTE ai nemici della mia Signora!'", FALSE, ch, 0, (ch->master), TO_ROOM);
                hit(ch, (ch->master)->specials.fighting, TYPE_HIT);
            }
            else
            {
                act("$c0015$n$c0015 si rimette al suo posto.\n\r", FALSE, ch, 0, 0, TO_ROOM);
                extract_char(ch);
                return (FALSE);
            }
        }

    }
    else
    {
        switch(number(0, 3))
        {
            case 1:
                return(cleric(mob, cmd, arg, mob, type));
                break;
                
            case 2:
                // assegno la skill rescue alla bambola nel caso non l'avesse
                if(!ch->skills)
                {
                    SpaceForSkills(ch);
                }
                
                if(!ch->skills[SKILL_RESCUE].learned)
                {
                    ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch)*3+30;
                }
                
                if((ch->master) && ch->in_room == (ch->master)->in_room)
                {
                    if((ch->master)->attackers > 2)
                    {
                        act("$c0009[$c0015$n$c0009] urla 'Non toccare la mia Signora!'", FALSE, ch, 0, 0, TO_ROOM);
                        do_rescue(ch, "evangeline", 0);
                        return(TRUE);
                    }
                    else if((ch->master)->specials.fighting)
                    {
                        act("$c0009[$c0015$n$c0009] urla 'Come osi!'", FALSE, ch, 0, 0, TO_ROOM);
                        one_argument(GET_NAME(mob->specials.fighting), target);
                        do_bash(ch, target, 0);
                    }
                }
                break;
                
            case 3:
                if((ch->master) && ch->in_room == (ch->master)->in_room)
                {
                    if(!HasActiveSanctuary(ch->master))
                    {
                        act("$c0009[$c0015$n$c0009] urla 'Arrivo mia Signora!", TRUE, ch, 0, 0, TO_ROOM);
                        act("$c0013$n$c0013 rapidamente si avvicina a $N$c0013 e viene da $L assorbit$B.", TRUE, ch, 0, ch->master, TO_ROOM);
                        act("$n e' protett$b da una $c0015aura bianca$c0007!", TRUE, ch->master, 0, 0, TO_ROOM);
                        SET_BIT((ch->master)->specials.affected_by, AFF_SANCTUARY);
                        (ch->master)->points.max_hit += dice(2, 6)*(ch->master)->points.max_hit/100;
                        if ((ch->master)->points.max_hit > 25000)
                            (ch->master)->points.max_hit = 25000;
                        alter_hit(ch->master,0);
                        extract_char(ch);
                        return (FALSE);
                    }
                    else if(GET_HIT(ch->master) < (GET_MAX_HIT(ch->master)-80))
                    {
                        act("$c0013$n$c0013 rapidamente si avvicina a $N$c0013 e viene da $L assorbit$B.", TRUE, ch, 0, ch->master, TO_ROOM);
                        act("$n emette un $c0009ghigno malefico$c0007.", TRUE, ch->master, 0, 0, TO_ROOM);
                        do_say(ch->master, "Ora si che sto bene!", 0);
                        (ch->master)->points.max_hit += dice(2, 6)*(ch->master)->points.max_hit/100;
                        (ch->master)->points.hit += dice(1, 51) + 99;
                        if ((ch->master)->points.max_hit > 25000)
                            (ch->master)->points.max_hit = 25000;
                        if(GET_HIT(ch->master) > GET_MAX_HIT(ch->master))
                            GET_HIT(ch->master) = GET_MAX_HIT(ch->master);
                        alter_hit(ch->master,0);
                        act("\n\r$c0011$n$c0011 e' diventat$b piu' potente.", TRUE, ch->master, 0, 0, TO_ROOM);
                        extract_char(ch);
                        return (FALSE);
                    }
                    else if((ch->master)->specials.fighting)
                    {
                        if(!ch->skills)
                        {
                            SpaceForSkills(ch);
                        }
                        if(!ch->skills[SPELL_HARM].learned)
                        {
                            ch->skills[SPELL_HARM].learned = GetMaxLevel(ch)*3+30;
                        }
                        act("$n pronuncia le parole, '$c0015Fa male, vero?$c0007'.", TRUE, ch, 0, 0, TO_ROOM);
                        cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, (ch->master)->specials.fighting, 0);
                    }
                }
                break;
                
            default:
                return(magic_user(mob, cmd, arg, mob, type));
        }
        return TRUE;
    }
    
    return FALSE;
}

MOBSPECIAL_FUNC(Nightmare) {
	struct affected_type af;

	if(!IS_AFFECTED(mob, AFF_FIRESHIELD)) {
		af.type      = SPELL_FIRESHIELD;
		af.duration  = 24*7;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_FIRESHIELD;
		affect_to_char(mob, &af);
	}
	if(!IS_AFFECTED(mob, AFF_GLOBE_DARKNESS)) {
		af.type      = SPELL_GLOBE_DARKNESS;
		af.duration  = 1;
		af.modifier  = 5;
		af.location  = APPLY_HIDE;
		af.bitvector = AFF_GLOBE_DARKNESS;
		affect_to_char(mob, &af);
	}
	return false;
}

} // namespace Alarmud
