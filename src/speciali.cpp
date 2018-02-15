/* AlarMUD
* $Id: speciali.c,v 1.1.1.1 2002/02/13 11:14:54 root Exp $*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "cmdid.hpp"
#include "fight.hpp"
#include "protos.hpp"
#include "snew.hpp"
#include "specass2.hpp"
#include "utility.hpp"
/* Piccola macro per non riempire il codice di if facendo degli strcmp */
#define STRSWITCH     if( false ){
#define CHECK(X,Y)    } else if ( strcmp((X),(Y)) == 0 ) {
#define END           }



/****************************************************************************
*  Blocca il passaggio in una certa direzione. Room Procedure
****************************************************************************/
int sBlockWay( struct char_data* pChar, int nCmd, char* szArg, struct room_data* pRoom, int nType ) {
	char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=pRoom->specparms;
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if( nType == EVENT_COMMAND ) {
		if( (nCmd != ndir) ||
				( (GetMaxLevel(pChar)>=nlev1) && (GetMaxLevel(pChar)<=nlev2))
				&& !IS_PRINCE( pChar )) { // Gaia 2001
			return(FALSE);
		}
		else {
			if (!msg || !msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}

			sprintf(lev2,"%s\r\n",msg);
			send_to_char(lev2,pChar);
			return TRUE;
		}
	}
	return FALSE;
}
/****************************************************************************
*  Blocca il passaggio in una certa direzione. Mob/Obj Procedure
****************************************************************************/
int sMobBlockWay( struct char_data* pChar, int nCmd, char* szArg,
				  struct char_data* pMob, int nType ) {
	char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=mob_index[pMob->nr].specparms;
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if( nType == EVENT_COMMAND ) {
		if( (nCmd != ndir) ||
				( (GetMaxLevel(pChar)>=nlev1) && (GetMaxLevel(pChar)<=nlev2))) {
			return(FALSE);
		}
		else {
			if (!msg || !msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			sprintf(lev2,"%s\r\n",msg);
			act(msg, FALSE, pMob, 0, pChar, TO_VICT);
			act("$n dice qualcosa a $N.", FALSE, pMob, 0, pChar, TO_NOTVICT);
			return TRUE;
		}
	}
	return FALSE;
}
int sEgoWeapon( struct char_data* pChar, int nCmd, char* szArg,
				struct char_data* pMob, int nType ) {
	char* p;
	char pcname[256];
	p=mob_index[pMob->nr].specparms;
	if (strlen(p)>255)
	{ return FALSE; }
	p=one_argument(p,pcname);
	if( nType == EVENT_COMMAND ) {
		return TRUE;
	}
	return FALSE;
}

/***** FLYP WORK *****/

/***** FENICE START *****/

/***********************************************************************************
*  Il Mobbo cambia il tipo di danno - va passato alla speciale il codice del danno
*  *Flyp*
***********************************************************************************/
int ChangeDam( struct char_data* pChar, int nCmd, char* szArg, struct char_data* pMob, int nType ) {
	char* p;
	char dam[256];
	int damType;

	p=mob_index[pMob->nr].specparms;
	p=one_argument(p,dam);
	damType=atoi(dam);

	pMob->specials.attack_type=damType;

	return FALSE;

}

/***** FENICE END *****/

/***** TEMPLI EROI START *****/

/****************************************************************************
*  Libro degli eroi - Casta lo spell e scala le rune
*  *Flyp*
****************************************************************************/
int LibroEroi(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) {
	char* p;
	char num [8], par2[256], runa[256], buf[128], mail[128]="flyp@e4a.it,nebbiearcane@yahoo.it,elei-mi-shill@munoi-ana-inen.com,gnocera@tin.it";
	struct obj_data* obj;
	int i,number,num2,xp,gold, nalign, tmpalign,max,xp2,xpcum,k,trueGain;
	short chrace;

	/*if( GetMaxLevel(ch)>51)
	 return true;
	*/
	num2=0;

	if( type == EVENT_COMMAND && cmd == CMD_SAY) {
		half_chop(arg,runa,par2);
		if(isdigit(*par2)) { num2=atoi(par2); }


		STRSWITCH
		CHECK ("ael",runa)
		if (GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> SANCTUARY");
			act("Reciti solennemente la parole del Potere \"AEL\".\r\nLe rune che la compongono si illuminano mentre cominciano a bruciare\r\nsulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Una colonna di luce ti colpisce! Senti un piacevole calore pervadere il tuo corpo!\r\nLentamente la colonna di luce si dissolve, sbiadendo fino a lasciarti un aura bianchissima\r\nattorno al corpo: sai di essere sotto la protezione della luce degli dei!\r\n",false,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"AEL\". La sua carne sembra bruciare,\r\nmentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("$N viene avvolt$b da una colonna di luce accecante. Mentre la luce sbiadisce\r\n e riesci a rimettere a fuoco la stanza, vedi che $N e' circondat$b da una intensa aura bianca.\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_sanctuary(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 4;
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
		CHECK ("inen",runa)
		if (GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> IDENTIFY");
			act("Reciti solennemente la parole del Potere \"INEN\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono\r\nfino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"INEN\". La sua carne sembra bruciare,\r\nmentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			for (i=0; i<3; i++) {
				number = real_object(32992);
				obj = read_object(number, REAL);
				obj_to_room(obj,ch->in_room);
			}
			act("Una esplosione di luce invade la stanza, quando si disperde\r\n vedi che per terra sono comparsi tre strani oggetti simili all'occhio di un drago...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Una esplosione di luce invade la stanza, quando si disperde\r\n vedi che per terra sono comparsi tre strani oggetti simili all'occhio di un drago...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Il Sacerdote parla: \"Usa l'Occhio del Drago per conoscere le virtu' di un oggetto!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Il Sacerdote parla: \"Usa l'Occhio del Drago per conoscere le virtu' di un oggetto!\"\r\n",FALSE,ch,0,ch,TO_ROOM);
			GET_RUNEDEI( ch ) -= 4;
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
		CHECK ("ghia",runa)
		if (GET_RUNEDEI(ch)>=1) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> CREATE FOOD");
			act("Reciti solennemente la parole del Potere \"GHIA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GHIA\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			for (i=0; i<5; i++) {
				number = real_object(32991);
				obj = read_object(number, REAL);
				obj_to_room(obj,ch->in_room);
			}
			act("Una esplosione di luce invade la stanza, quando si disperde vedi che per terra\r\nsono comparse cinque coppe colme di un liquido denso e profumato...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Una esplosione di luce invade la stanza, quando si disperde vedi che per terra\r\nsono comparse cinque coppe colme di un liquido denso e profumato...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Il Sacerdote parla: \"Quando sarai affamato, potrei nutrirti col nettare degli dei\r\ncontenuto nelle coppe!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Il Sacerdote parla: \"Quando sarai affamato, potrei nutrirti col nettare degli dei\r\ncontenuto nelle coppe!\"\r\n",FALSE,ch,0,ch,TO_ROOM);
			GET_RUNEDEI( ch ) -= 1;
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
		CHECK ("zir",runa)
		if (GET_RUNEDEI(ch)>=20) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> FIRESHIELD");
			act("Reciti solennemente la parole del Potere \"ZIR\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ZIR\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Cominci a bruciare come una torcia, ma i lembi di fuoco che ti circondano non ti provocano\r\nalcun dolore, anzi, ti danno un gran senso di protezione!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N viene avvolt$b da possenti fiamme, ma sembra essere in grado di controllarle!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_fireshield(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 20;
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
		CHECK ("sidamishida",runa)
		if (GET_RUNEDEI(ch)>=8) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> TREETRAVEL");
			act("Reciti solennemente la parole del Potere \"SIDAMISHIDA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"SIDAMISHIDA\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Ti senti in completa comunione con la natura: ora sai di poter viaggiare utilizzando\r\nle sacre vie dei druidi!\r\n",FALSE,mob,0,ch,TO_VICT);
			spell_tree_travel(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 8;
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
		CHECK ("elu",runa)
		if (GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> WATER BREATH");
			act("Reciti solennemente la parole del Potere \"ELU\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ELU\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Per un attimo ti senti strozzare...\n\r...ti manca il fiato...\r\n...pensi di morire...\r\n ma ad un tratto tutto passa e senti di poter respirare ovunque!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N Strabuzza gli occhi e si tienen la gola: SEMBRA SOFFOCARE!! Ad un tratto inspira profondamente e tutto sembra passato..\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_water_breath(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 4;
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
		CHECK ("tide",runa)
		if (GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> STRENGTH");
			act("Reciti solennemente la parole del Potere \"TIDE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"TIDE\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Ti senti pieno di vigore. I tuoi muscoli si gonfiano e tutto quello che porti ti sembra piu' leggero!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N si erge in tutta la sua potenza ed i suoi muscoli si gonfiano!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_strength(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 4;
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
		CHECK ("fuel",runa)
		if (GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> MINOR TRACK");
			act("Reciti solennemente la parole del Potere \"FUEL\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"FUEL\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti una voce: \"Concentrati sulla tua preda e questa non ti potra' sfuggire!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Gli occhi di $N sono attraversati da un lampo di luce!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_track(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 4;
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
		CHECK ("beio",runa)
		if (GET_RUNEDEI(ch)>=6) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> MAJOR TRACK");
			act("Reciti solennemente la parole del Potere \"BEIO\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"BEIO\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti una voce: \"Concentrati sulla tua preda e questa non ti potra' sfuggire!\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Gli occhi di $N sono attraversati da un lampo di luce!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_track(52,ch,ch,NULL);
			GET_RUNEDEI( ch ) -= 6;
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
		CHECK ("aelgud",runa)
		if (GET_RUNEDEI(ch)>=3) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT DRAG BREATH");
			act("Reciti solennemente la parole del Potere \"AELGUD\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"AELGUD\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: un possente drago ti soffia addosso tutta la sua rabbia,\r\nma tu non vacilli e passi indenne!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_dragon_breath(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 3;
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
		CHECK ("gudorizir",runa)
		if (GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT FIRE");
			act("Reciti solennemente la parole del Potere \"GUDORIZIR\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORIZIR\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: vieni colpito in pieno da una enorme palla infuocata,\r\nma tu non vacilli e passi indenne!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_fire(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 2;
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
		CHECK ("gudorishaff",runa)
		if (GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT ELECTRICITY");
			act("Reciti solennemente la parole del Potere \"GUDORISHAFF\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORISHAFF\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: un fulmine esplode nel cielo sereno e ti colpisce in pieno,\r\nma tu non vacilli e passi indenne!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_elec(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 2;
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
		CHECK ("gudorilakra",runa)
		if (GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT COLD");
			act("Reciti solennemente la parole del Potere \"GUDORILAKRA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORILAKRA\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: un vento gelido si alza distruggendo\r\ntutto quello che colpisce con lame di ghiaccio e sei nel centro di questa bufera, ma tu non vacilli e passi indenne!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_cold(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 2;
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
		CHECK ("gudorielei",runa)
		if (GET_RUNEDEI(ch)>=2) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> PROT ENERGY");
			act("Reciti solennemente la parole del Potere \"GUDORIELEI\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"GUDORIELEI\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Senti la tua pelle bruciare, stirarsi fino all'inverosimile...\r\nUna visione ti attraversa la mente: una strana nube attraversata da fulmini multicolore avanza verso di te distruggendo tutto quello che incontra fino ad inglobarti al suo interno, ma tu non vacilli e passi indenne!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N ha uno sguardo vacuo, perso nel vuoto. Ma subito si scuote e sembra tornre in se...\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_prot_energy(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 2;
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
		CHECK ("iaeelia",runa)
		if (GET_RUNEDEI(ch)>=10) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> TS");
			act("Reciti solennemente la parole del Potere \"IAEELIA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"IAEELIA\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Un dolore lancinante ti attraversa la testa ed hai come l'impressione che i tuoi occhi stiano per esplodere...\r\nMa tutto passa presto e sai che ora nulla sara' celato alla tua Vista!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... ad un tratto tutto passa ed i suoi occhi brillano di una strana luce azzurrina!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_true_seeing(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 10;
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
		CHECK ("elia",runa)
		if (GET_RUNEDEI(ch)>=8) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> DETECT INVI");
			act("Reciti solennemente la parole del Potere \"ELIA\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ELIA\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Un dolore lancinante ti attraversa la testa ed hai come l'impressione che i tuoi occhi stiano per esplodere...\r\nMa tutto passa presto e sai che ora la magia non potra' piu' nascodere qualcuno o qualcosa alla tua Vista!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... ad un tratto tutto passa ed i suoi occhi brillano di una strana luce porpora!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_detect_invisibility(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 8;
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
		CHECK ("ene",runa)
		if (GET_RUNEDEI(ch)>=num2) {

			if (num2<=0||!num2)
			{ return true; }

			xp=num2*(GetMaxLevel(ch))*10000;
			xp2=0;
			xpcum=0;
			k=0;
			max=0;

			if( ch->desc && ch->desc->original )
			{ chrace = ch->desc->original->race; }
			else
			{ chrace = GET_RACE( ch ); }

			/* Calcolo il maxxaggio */
			for (i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
				if( GET_LEVEL( ch, i ) && GET_LEVEL( ch, i ) < RacialMax[ chrace ][ i ] ) {
					mudlog(LOG_SYSERR,"Non sono al massimo razziale");
					if (GET_LEVEL( ch, i )!=0) {
						k=(titles[i][ GET_LEVEL( ch, i ) + 2 ].exp)-1;
						if (xpcum == 0 || k < xpcum) {
							xpcum=k;
							mudlog(LOG_SYSERR,"maxxaggio a %d",k);
						}
					}
				}
			}
			k=HowManyClasses(ch);
			trueGain=GET_EXP(ch)+(xp/k);
			/* Se si tratta di un principe accediamo comunque al gain completo, ignorando il maxxaggio */
			if (trueGain <= xpcum || GET_EXP(ch)>=PRINCEEXP) {
				/* Check sull'owerflow per calcolar ele rune da spendere */
				if ((GET_EXP(ch)+(xp/k))<0) {
					max=(MAX_XP-GET_EXP(ch))/(GetMaxLevel(ch)*10000);
					if (max>0) {
						act("Reciti solennemente la parole del Potere \"ENE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
						act("Il sacerdote ti dice \"Capisco il tuo desiderio di sapere, ma per volere degli ho considerato solo una parte delle Rune che volevi consacrare agli Dei!\"\r\n",FALSE,mob,0,ch,TO_VICT);
						act("$N declama con voce imponente la parola \"ENE\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
						act("Strane immagini vorticano davanti ai tuoi occhi... Stai rivivendo le epiche gesta di Eroi di antico passato!\r\nQuando la tua visione termina, e' come se TU abbia vissuto in prima persona quelle avventure!\r\n",FALSE,mob,0,ch,TO_VICT);
						act("$N trema scosso da brividi, in preda ad una strana trance mistica, ma in un lungo istante tutto cio' passa...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
						xp=max*(GetMaxLevel(ch))*10000;
						GET_RUNEDEI( ch ) -= max;
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
				GET_RUNEDEI( ch ) -= num2;
				act("Reciti solennemente la parole del Potere \"ENE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
				act("$N declama con voce imponente la parola \"ENE\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
				act("Strane immagini vorticano davanti ai tuoi occhi... Stai rivivendo le epiche gesta di Eroi di antico passato!\r\nQuando la tua visione termina, e' come se TU abbia vissuto in prima persona quelle avventure!\r\n",FALSE,mob,0,ch,TO_VICT);
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
				if (max>0) {
					mudlog(LOG_PLAYERS, "GAIN PARZIALE Rune spese (num2)=%d Spendibili %d",num2,max);
					xp=max*(GetMaxLevel(ch))*10000;
					act("Reciti solennemente la parole del Potere \"ENE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
					act("Il sacerdote ti dice \"Capisco il tuo desiderio di sapere, ma per volere degli ho considerato solo una parte delle Rune che volevi consacrare agli Dei!\"\r\n",FALSE,mob,0,ch,TO_VICT);
					act("$N declama con voce imponente la parola \"ENE\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
					act("Strane immagini vorticano davanti ai tuoi occhi... Stai rivivendo le epiche gesta di Eroi di antico passato!\r\nQuando la tua visione termina, � come se TU abbia vissuto in prima persona quelle avventure!\r\n",FALSE,mob,0,ch,TO_VICT);
					act("$N trema scosso da brividi, in preda ad una strana trance mistica, ma in un lungo istante tutto cio' passa...\r\n",FALSE,mob,0,ch,TO_NOTVICT);
					GET_RUNEDEI( ch ) -= max;
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
		CHECK ("ane",runa)
		if (GET_RUNEDEI(ch)>=num2 && num2>0) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> COINS");
			act("Reciti solennemente la parole del Potere \"ANE\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ANE\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Le tue tasche si fanno piu' pesanti, mentre una voce nella tua testa ti dice \"Premio la tua avidita' eroe! Oppure vuoi usare le tue ricchezze per il bene degli altri?\"\r\n",FALSE,mob,0,ch,TO_VICT);
			act("Le tasche di $N si gonfiano improvvisamente!\r\n",FALSE,ch,0,ch,TO_ROOM);
			gold=num2*10000;
			mudlog(LOG_PLAYERS, "esecuzione rune --> assegno %d coins a %s",gold,GET_NAME(ch));
			GET_GOLD(ch)+=gold;
			GET_RUNEDEI( ch ) -= num2;
			sprintf(buf,"echo \"PC: %s RUNE SPESE: %d\"| mail -s \"ESECUZIONE RUNE --> Conv Coins\" %s", GET_NAME(ch),max,mail);
			system(buf);
			return true;
		}
		else {
			act("Le tue rune non sono sufficienti per avere il favore che hai chiesto agli dei!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama le rune, ma non accade nulla!\r\n",FALSE,ch,0,ch,TO_ROOM);
			return true;
		}
		CHECK ("elei",runa)
		if (GET_RUNEDEI(ch)>=1) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> CURE BLINDNESS");
			act("Reciti solennemente la parole del Potere \"ELEI\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ELEI\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Un dolore lancinante ti attraversa la testa ed hai come l'impressione che i tuoi occhi stiano per esplodere...\r\nMa tutto passa presto e quando riapri gli occhi, ti accorgi di poter vedere di nuovo!\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... Un bagliore attraversa i suoi occhi e poi si spegne\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_cure_blind(52, ch, ch,0);
			GET_RUNEDEI( ch ) -= 1;
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
		CHECK ("itel",runa)
		if (GET_RUNEDEI(ch)>=4) {
			mudlog(LOG_PLAYERS, "esecuzione rune --> REMOVE PARALYSIS");
			act("Reciti solennemente la parole del Potere \"ITEL\".\r\n Le rune che la compongono si illuminano mentre cominciano a bruciare sulla tua pelle e lentamente sbiadiscono fino a scomparire. Il dolore passa alla svelta...\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N declama con voce imponente la parola \"ITEL\". La sua carne sembra bruciare, mentre le rune che aveva tautate si infiammano e sbiadiscono...\r\n",FALSE,ch,0,ch,TO_ROOM);
			act("Il tuo corpo brucia pervaso da un immenso calore... Quando questa sensazione passa, ti accorgi di essere di nuovo padrone dei tuoi movimenti.\r\n",FALSE,mob,0,ch,TO_VICT);
			act("$N trema scosso da brividi... ad un tratto tutto passa ed il suo corpo si muove di nuovo!\r\n",FALSE,ch,0,ch,TO_ROOM);
			spell_remove_paralysis(52,ch,ch,0);
			GET_RUNEDEI( ch ) -= 4;
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

/****************************************************************************
*  Guardiano - Blocca gli align diversi
*	 Alla speciale si passa -1000 per far passare evil, 0 neutral, 1000 good
*  *Flyp*
****************************************************************************/
int MobBlockAlign( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) {
	char* p;
	char dir[256];
	char align[256];
	char msg[256];

	int ndir, nalign, tmpalign;



	if( type == EVENT_COMMAND ) {
		p=mob_index[mob->nr].specparms;

		p=one_argument(p,dir);
		p=one_argument(p,align);
		only_argument(p,msg);

		ndir=atoi(dir);
		nalign=atoi(align);


		tmpalign=GET_ALIGNMENT(ch);

		//definiamo gli allineamenti
		if (tmpalign<=-350)
		{ tmpalign=-1000; }
		else if (tmpalign>=350)
		{ tmpalign=1000; }
		else
		{ tmpalign=0; }
		if( (cmd != ndir) || (tmpalign==nalign))
		{ return(FALSE); }
		else {
			if (!msg || !msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			//sprintf(lev2,"%s\r\n",msg);
			act(msg, FALSE, mob, 0, ch, TO_VICT);
			act("$n dice qualcosa a $N.", FALSE, mob, 0, ch, TO_NOTVICT);
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************************
*  Ingresso - Blocca gli align diversi
*	 Alla speciale si passa -1000 per far passare evil, 0 neutral, 1000 good
*  *Flyp*
****************************************************************************/
int BlockAlign( struct char_data* ch, int cmd, char* arg, struct room_data* pRoom, int type ) {
	char* p;
	char dir[256];
	char align[256];
	char msg[256];

	int ndir, nalign, tmpalign;

	if( type == EVENT_COMMAND ) {
		p=pRoom->specparms;
		p=one_argument(p,dir);

		p=one_argument(p,align);
		only_argument(p,msg);

		ndir=atoi(dir);
		nalign=atoi(align);


		tmpalign=GET_ALIGNMENT(ch);

		//definiamo gli allineamenti
		if (tmpalign<=-350)
		{ tmpalign=-1000; }
		else if (tmpalign>=350)
		{ tmpalign=1000; }
		else
		{ tmpalign=0; }

		if( (cmd != ndir) || (tmpalign==nalign))
		{ return(FALSE); }
		else {
			if (!msg || !msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			sprintf(dir,"%s\r\n",msg);
			send_to_char(dir,ch);
			return TRUE;
		}
	}
	return FALSE;
}

int LadroOfferte( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) {
	char buf[256], buf2[256];

	one_argument(arg,buf);
	only_argument(arg,buf2);

	sprintf(arg,buf2);
	if( type == EVENT_COMMAND ) {
		if( cmd == CMD_GET ) {
			if ((strstr(buf,"coins"))||(strstr(buf2,"coins"))) {
				do_kill(mob,GET_NAME(ch),0);
				return FALSE;
			}
		}
	}
	return FALSE;
}

/***** TEMPLI EROI END *****/

/***** NEO ORSHINGAL START *****/

int Vampire_Summoner( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) {
	char* p;
	char nmob[256];
	int check, nummob;
	struct char_data* mobtmp;
	static struct char_data* tmp;

	if ((GET_POS(mob)==POSITION_FIGHTING) && (number(0,9)<6)) {
		// Summon control added by EleiMiShill

		// Allora, mi serve il VNUM
		p=mob_index[mob->nr].specparms;
		p=one_argument(p,nmob);
		nummob=atoi(nmob);

		// Ok, controllo se sta combattendo contro i suoi servitori
		if(mob->specials.fighting) {
			mobtmp=mob->specials.fighting;
			if(IS_NPC(mobtmp)) {
				if(mobtmp->nr==nummob) {
					// Ok, � una delle bambole che sta combattendo
					// Vediamo di eliminarla allora
					do_say(mob, "Vattene serva infedele!", 0);
					extract_char(mobtmp);
					return false;
				}
			}
		}
		// Se ha troppi mob servitori evita di evocarne altri
		if(!too_many_followers(mob)) {

			mobtmp = read_mobile(real_mobile(nummob),REAL);
			mudlog(LOG_SYSERR,"mobtmp %s", GET_NAME(mobtmp));
			char_to_room(mobtmp, ch->in_room);
			mudlog(LOG_SYSERR,"mob %s", GET_NAME(mob));

			add_follower(mobtmp, mob);
			SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
			//mudlog(LOG_SYSERR,"ch %s", mobGET_NAME(ch));

			AddHated(mobtmp,mob->specials.fighting);
			do_order(mob, "followers guard on", 0);
		}
		// Se il MOB o il PG � immune a drain non perdo nemmeno il tempo di tentare
		// il che � bastardo perch� non vedi il messaggio quindi non sa che drena :-)
		if(!(IS_SET((mob)->M_immune, IMM_DRAIN)))
			//IS_IMMUNE(mob,IMM_DRAIN) ARGH!!! IS_IMMUNE qui non � nemmeno definita!
		{
			do_say(mob, "Voglio la tua energia vitale!!!", 0);
			cast_energy_drain( 50, mob, "", SPELL_TYPE_SPELL, mob->specials.fighting, 0);
		}
		return true;
	}

	return false;
}

int Nightmare( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) {
	struct affected_type af;
	int num;

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

	if(RIDDEN(mob)) {
		if(RIDDEN(mob)==ch) {
			GET_HIT(ch)-10;
		}
	}
	return false;
}

/***** NEO ORSHINGAL END *****/
