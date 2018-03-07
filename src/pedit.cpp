/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: pedit.c,v 2.4 2002/06/03 22:53:09 Thunder Exp $
* */
/***************************  System  include ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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
#include "pedit.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"
namespace Alarmud {

/*
PER AGGANCIARE LA PROCEDURA:
mettere in lib/myst.spe <M numerodelmob EditMaster>, possibilmente il
  mob dovrebbe stare in una stanza privata a 2 e nosummon,
mettere in specass2.h il prototipo dell'EditMaster
  <int EditMaster( struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)>,
mettere in otherproc.h <{ "EditMaster", EditMaster },>,

TODOLIST:
controllare se deve essere possibile mettere resistenza o immunita' ad un
  oggetto che e' suscettibile, e altri controlli su modifiche doppie vedi
  sempre immunita' e resistenze, armi brittle e qualcosina d'altro,

QUESTI NON LI HO ANCORA MESSI PER EFFETTUARE I TEST PIU' VELOCEMENTE:
il controllo sul prince,

QUESTO OCCORRE DEFINIRE UNA STRATEGIA CHIARA:
consentire la creazione di oggetti e armi con i vnum assegnati,
*/
#define	ARM	1
#define	OGG	2
#define	RES	3
#define	IMM	4


struct lista_comandi {
	int	cmd;			// num comando
	char	com[20];		// stringa comando
	int	tipo;			// oggetto o arma
	int	azione;			// BLUNT, SPELL_ ...
	long	costoxp;			// costo * M xp
	int	costopq;		// costo * PQuest
	int	add;			// incrementro a modifica
	int	max;			// il massimo a modifiche consentite
};

static lista_comandi comandi[] = {
	{0, "fine", 			0, 	0,			0L,0, 	0, 	0},
	{1, "aiuto", 			0, 	0,			0L,0, 	0, 	0},
	{2, "move", 			OGG, 	APPLY_MOVE, 		10L,1, 	10,	30},
	{3, "mana", 			OGG, 	APPLY_MANA, 		5L,1, 	2, 	10},
	{4, "hp", 			OGG, 	APPLY_HIT, 		5L,1,   2, 	10},
	{5, "hitroll", 		OGG, 	APPLY_HITROLL, 		40L,8,  1, 	1},
	{6, "damroll", 		OGG, 	APPLY_DAMROLL, 		50L,9,  1, 	1},
	{7, "armor", 			OGG, 	APPLY_AC, 		5L,1,  -5,      0},
	{8, "move_regen", 		OGG, 	APPLY_MOVE_REGEN, 	10L,1,  10, 	10},
	{9, "mana_regen", 		OGG, 	APPLY_MANA_REGEN, 	6L,1,   1, 	5},
	{10, "hp_regen", 		OGG, 	APPLY_HIT_REGEN, 	6L,1,   2, 	5},
	{11, "str", 			OGG, 	APPLY_STR, 		9L,2,   1, 	1},
	{12, "dex", 			OGG, 	APPLY_DEX, 		10L,2,  1, 	2},
	{13, "con", 			OGG, 	APPLY_CON, 		5L,1,   1, 	2},
	{14, "wis", 			OGG, 	APPLY_WIS, 		10L,2,  1, 	2},
	{15, "int", 			OGG, 	APPLY_INT, 		8L,2,   1, 	2},
	{16, "chr", 			OGG, 	APPLY_CHR, 		3L,1,   3, 	6},
	{17, "spellfail",		OGG, 	APPLY_SPELLFAIL,	6L,1,  -2,      0},
};

#define	MAXCOM		18

const char* aiuto_modifica[] = {
	"$c0015Da questo momento fino al termine devi utilizzare sempre il comando $c0010ASK$c0015:",
	"$c0015Parametri del comando $c0010MODIFICA$c0015 <$c0011CAMPO$c0015> <$c0011VALORE$c0015>",
	"$c0015I principi possono specificare il tipo di pagamento, xp o pq$c0015",
	"$c0015Un punto quest e' comunque richiesto per effettuare qualunque modifica$c0015",
	"$c0015per tutti gli altri il pagamento e' in soli pq $c0015",
	"$c0015Lista dei campi :",
	"$c0011pago$c0015   = pq o xp (default pq)",
	"$c0011stato$c0015  = i valori attuali",
	"$c0011aiuto$c0015  = Queste informazioni.",
	"$c0011fine$c0015   = Termina le modifiche.",
	" Solo per oggetti di tipo armor:",
	"$c0011CARATTERISTICA $c0015 BONUS MAX      $c0014[MXP/PQ]$c0015",
	"$c0011move           $c0015 = +10 max 30   $c0014[ 10/1]$c0015| $c0011move_regen     $c0015 = +10 max 10    $c0014[ 10/1]$c0015",
	"$c0011mana           $c0015 = +2  max 10   $c0014[  4/1]$c0015| $c0011mana_regen     $c0015 = +1 max 5     $c0014[  6/1]$c0015",
	"$c0011hp             $c0015 = +2  max 10   $c0014[  4/1]$c0015| $c0011hp_regen       $c0015 = +2 max 5     $c0014[  6/2]$c0015",
	"$c0011hitroll        $c0015 = +1  max 1    $c0014[ 40/8]$c0015| $c0011damroll        $c0015 = +1 max 1     $c0014[ 50/9]$c0015",
	"$c0011armor          $c0015 = -5  max -20  $c0014[  5/1]$c0015| $c0011str            $c0015 = +1 max 1     $c0014[  9/2]$c0015",
	"$c0011dex            $c0015 = +1  max 2    $c0014[ 10/2]$c0015| $c0011con            $c0015 = +1 max 2     $c0014[  5/1]$c0015",
	"$c0011wis            $c0015 = +1  max 2    $c0014[ 10/2]$c0015| $c0011int            $c0015 = +1 max 2     $c0014[  8/2]$c0015",
	"$c0011chr            $c0015 = +3  max 6    $c0014[  3/1]$c0015| $c0011spellfail      $c0015 = -2 max -10   $c0014[  6/1]$c0015",
	"P.S.",
	"Tra [] e' il costo in Mega xp per una modifica, ricordati che di base hai gia'",
	"  speso 1 pq, se non effettui nessuna modifica ti sara' restituito il pq.",
	"\n"
};

void SayMenu(struct char_data* pCh, const char* apchMenu[]) {
	int i;
	char buf[200];

	for (i = 0; *apchMenu[i] != '\n'; i++) {
		sprintf(buf, "%s\n\r", apchMenu[i]);
		send_to_char(buf, pCh);
	}
}

bool ha_modificato = FALSE;
long tot_costoxp = 0L;
long tot_costopq = 0L;
bool ok_costo = FALSE;
int modifica = FALSE;
int pagamento = 0; // 0=pq  1=xp
char modifica_obj[40] = "";

static struct char_data* find_editman(struct char_data* ch) {
	struct char_data* editman;

	editman = FindMobInRoomWithFunction(ch->in_room, EditMaster);

	if (!editman)
	{ send_to_char("Whoa!  Non c'e' la persona giusta.\n\r", ch); }

	return editman;
}

int calc_costoxp(int i, int p) {
	int  tot;

	tot = MAX(0,static_cast<int>(comandi[i].costoxp * p/comandi[i].add));
	return tot*1000000L;
}
int calc_costopq(int i, int p) {
	int  totpq;

	totpq = MAX(0,static_cast<int>(comandi[i].costopq * p/comandi[i].add));
	return totpq;
}

int EditMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob,int type) {
	char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
	int cost, ave;
	struct char_data* vict;
	struct char_data* editman;
	struct obj_data* obj;
	char field[20], comando[20], parmstr[MAX_STRING_LENGTH];
	char parmstr2[MAX_STRING_LENGTH];
	int iVNum, iCom, iSpell, iMagie, ciclo, temp;

	if (!AWAKE(ch))
	{ return (FALSE); }

	if ( MobCountInRoom( real_roomp(mob->in_room)->people ) < 2 && modifica == TRUE ) {
		mudlog(LOG_PLAYERS,"Il PG e' andato via in EditMaster" );
		modifica = FALSE;
		return (FALSE);
	}

	if (!(editman = find_editman(ch)))
	{ return (FALSE); }

	if (IS_NPC(ch)) {
		if (cmd == CMD_GIVE || cmd == CMD_ASK) {
			arg = one_argument(arg, obj_name);
			if (!*obj_name)
			{ return (FALSE); }
			if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
			{ return (FALSE); }
			arg = one_argument(arg, vict_name);
			if (!*vict_name)
			{ return (FALSE); }
			if (!(vict = get_char_room_vis(ch, vict_name)))
			{ return (FALSE); }
			if (editman) {
				act("$N ti dice 'Che cosa fai qui?? non farmi perdere tempo.'", FALSE,
					ch, 0, editman, TO_CHAR);
				return (TRUE);
			}
		}
		else
		{ return (FALSE); }
	}

	if (cmd == CMD_GIVE && !modifica) {
		arg = one_argument(arg, obj_name);
		if (!*obj_name) {
			act("$N ti dice 'Che cosa vorresti darmi?'", FALSE,
				ch, 0, editman, TO_CHAR);
			return (TRUE);
		}
		if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
			act("$N ti dice 'Non vedo cosa vorresti darmi.'", FALSE,
				ch, 0, editman, TO_CHAR);
			return (TRUE);
		}
		arg = one_argument(arg, vict_name);
		if (!*vict_name) {
			send_to_char("A chi?\n\r", ch);
			return (TRUE);
		}
		if (!(vict = get_char_room_vis(ch, vict_name))) {
			send_to_char("A chi?\n\r", ch);
			return (TRUE);
		}

		if (vict->specials.fighting) {
			act("$N ti dice 'Non vedi che sto combattendo!?'", FALSE,
				ch, 0, editman, TO_CHAR);
			return (TRUE);
		}

		if (!IS_NPC(vict))
		{ return (FALSE); }

		if ((ITEM_TYPE(obj) == ITEM_ARMOR || ITEM_TYPE(obj) == ITEM_WEAPON)) {
			if(GET_RUNEDEI(ch)<1) {
				act("$N ti dice 'Ci lavorerei volentieri, ma tu non puoi permetterti nessuna modifica!'", FALSE,
					ch, 0, editman, TO_CHAR);
				return (TRUE);
			}

			act( "Dai $p a $N.", TRUE, ch, obj, editman, TO_CHAR );
			act( "$n da` $p a $N.", TRUE, ch, obj, editman, TO_ROOM );

			obj_from_char( obj );
			obj_to_char( obj, editman );


			act("$N ti dice 'Ok, vediamo un po' che si puo' fare ...'", FALSE,
				ch, 0, editman, TO_CHAR);
			mudlog(LOG_PLAYERS,"%s inizia modifica %s con %d xp e %d pq", GET_NAME(ch),obj->name,GET_EXP(ch), GET_RUNEDEI(ch));
			modifica = TRUE;
			/*    GET_RUNEDEI(ch) -= 1;  Lo sposto dopo per evitare che in crash si perdano PQ
			      mudlog(LOG_PLAYERS,"a %s viene preso 1 pq per inizio modifica su %s", GET_NAME(ch),obj->name);  */
			strcpy(modifica_obj, obj_name);
			SayMenu(ch, aiuto_modifica);
			return (TRUE);
		}
		else
			act("$N ti dice 'Mi spiace, ma io lavoro solo su armi e armature!'", FALSE,
				ch, 0, editman, TO_CHAR);
		return (TRUE);
	}
	else if (cmd == CMD_ASK && modifica) {

		arg = one_argument(arg, comando);

		if (!*arg || !*comando || strcmp(comando, "modifica")) {
			send_to_char("Prova a usare 'Ask $c0010modifica$c0007 $c0011aiuto$c0007'.\n\r", ch);
			return (TRUE);
		}

		arg = one_argument(arg, field);

		if (!*field) {
			send_to_char ("Modificare che cosa?!? Usa '$c0010modifica$c0015 $c0011aiuto$c0015'.\n\r", ch);
			return (TRUE);
		}

		if (IS_PRINCE(ch) && !strcmp(field, "pago")) {
			SetStatus("EditMaster", field);
			arg = one_argument(arg, parmstr);
			if (!strcmp(parmstr, "xp")) {
				if (ha_modificato) {
					if ((GET_EXP(ch) - 400000000) < tot_costoxp ) {
						act("$N ti dice 'Non hai abbastanza esperienza per pagare il mio prezzo!'", FALSE,
							ch, 0, editman, TO_CHAR);
						return (TRUE);
					}
				}
				pagamento = 1;
				act("$N ti dice 'Va bene, visto che sei tu accetto il pagamento in XP'", FALSE,
					ch, 0, editman, TO_CHAR);
			}
			else if (!strcmp(parmstr, "pq")) {
				if (ha_modificato) {
					if (GET_RUNEDEI(ch) <  tot_costopq + 1) {
						act("$N ti dice 'Non hai abbastanza punti quest per pagarmi!'", FALSE,
							ch, 0, editman, TO_CHAR);
						return (TRUE);
					}
				}
				pagamento = 0;
				act("$N ti dice 'Va bene, accetto i tuoi punti quest come pagamento.'", FALSE,
					ch, 0, editman, TO_CHAR);
			}
			else
				act("$N ti dice 'Non ho capito bene in che modo vorresti pagare. xp o pq?'", FALSE,
					ch, 0, editman, TO_CHAR);
			return (TRUE);
		}

		if (!strcmp(field, "aiuto")) {
			SetStatus("EditMaster", field);
			SayMenu(ch, aiuto_modifica);
			return (TRUE);
		}

		SetStatus("EditMaster", "get_obj_in_list_vis");

		if (!(obj = get_obj_in_list_vis(editman, modifica_obj, editman->carrying))) {
			send_to_char("OPS!! non trovo piu' l'oggetto?!?\n\r", ch);
			modifica = FALSE;
			return (TRUE);
		}
		SetStatus("EditMaster", obj->name);
		if (obj) {
			iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;

			if (!iVNum) {
				send_to_char("Quale sarebbe l'oggetto?!?\n\r", ch);
				return (TRUE);
			}

			if (!strcmp(field, "stato")) {
				char scom[256];

				sprintf(scom, "$c0015Nome:$c0007 %s  $c0015Descr:$c0007 %s \n\r",
						obj->name, obj->short_description);
				send_to_char(scom, ch);
				if (ITEM_TYPE(obj) == ITEM_WEAPON) {
					sprintf(scom, "$c0015Arma:$c0007 %dD%d \n\r",
							obj->obj_flags.value[1], obj->obj_flags.value[2]);
					send_to_char(scom, ch);
					for (temp = 0; temp < MAX_OBJ_AFFECT; temp++)
						if (obj->affected[temp].location == APPLY_WEAPON_SPELL) {
							sprintf(scom, "$c0015Magia:$c0007 %s \n\r",
									spells[obj->affected[temp].modifier - 1]);
							send_to_char(scom, ch);
						}

				}
				else if (ITEM_TYPE(obj) == ITEM_ARMOR) {
					for (temp = 0; temp < MAX_OBJ_AFFECT; temp++)
						if (obj->affected[temp].location) {
							char buf2[256];

							if (obj->affected[temp].location == APPLY_SPELL) {
								sprintbit(obj->affected[temp].modifier, affected_bits, buf2);
								sprintf(scom, "$c0015Magia:$c0007 %s da %s \n\r",
										apply_types[obj->affected[temp].location], buf2);
							}
							else if (obj->affected[temp].location == APPLY_IMMUNE
									 || obj->affected[temp].location == APPLY_M_IMMUNE) {
								sprintbit(obj->affected[temp].modifier, immunity_names, buf2);
								sprintf(scom, "Protezione tipo: $c0015%s$c0007 %s \n\r",
										apply_types[obj->affected[temp].location], buf2);
							}
							else
								sprintf(scom, "Applica a $c0015%s$c0007 %d \n\r",
										apply_types[obj->affected[temp].location],
										obj->affected[temp].modifier);
							send_to_char(scom, ch);
						}
				}
				sprintf(scom, "Pagamento in %s.\n\r", (pagamento) ? "xp" : "pq");
				send_to_char(scom, ch);
				if (ha_modificato) {
					if (pagamento)
					{ sprintf(scom, "Spesa modifiche %ld xp.\n\r", tot_costoxp); }
					else {
						sprintf(scom, "Spesa modifiche %ld pq.\n\r", tot_costopq + 1);
					}
					send_to_char(scom, ch);
				}
				sprintf(scom, "Hai un totale di $c0015%d$c0007 xp e $c0015%d$c0007 pq.\n\r", GET_EXP(ch), GET_RUNEDEI(ch));
				send_to_char(scom, ch);
				return (TRUE);
			}

			for (iCom = 0; iCom < MAXCOM; iCom++)
				if (!strcmp(field, comandi[iCom].com))
				{ break; }

			if (iCom == MAXCOM) {
				act("$N ti dice 'Non capisco. Cosa stai cercando di dirmi?!?'", FALSE,
					ch, 0, editman, TO_CHAR);
				return (TRUE);
			}

			switch (comandi[iCom].cmd) {
			case 0:			// fine
				if (!ha_modificato) {
					struct extra_descr_data* new_descr, *tmp_descr;

					mudlog(LOG_PLAYERS,"%s non ha modificato %s", GET_NAME(ch), obj->name);
					/*		  CREATE(new_descr, struct extra_descr_data, 1); // SALVO vedere perche' non funziona
							  new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
					 		  new_descr->keyword = strdup("EditMaster");
							  new_descr->description = strdup("Oggetto modificato");
							  new_descr->next = 0;
							  obj->ex_description = new_descr; */
				}
				else if (pagamento) {
					GET_RUNEDEI(ch) -= 1;
					GET_EXP(ch) -= MAX(0,tot_costoxp);
					mudlog(LOG_PLAYERS,"%s fine modifica in XP %s con %d xp e %d pq totale costo %ld.", GET_NAME(ch),obj->name,GET_EXP(ch),GET_RUNEDEI(ch),tot_costoxp);
				}
				else {
					GET_RUNEDEI(ch) -= 1+tot_costopq;
					mudlog(LOG_PLAYERS,"%s fine modifica in PQ %s con %d xp e %d pq totale costo %ld.", GET_NAME(ch),obj->name,GET_EXP(ch),GET_RUNEDEI(ch),tot_costopq);
				}
				tot_costoxp = 0L;
				tot_costopq = 0L;
				ok_costo = FALSE;
				ha_modificato = FALSE;
				strcpy(modifica_obj, "");
				modifica = FALSE;
				act( "$N da` $p a $n.", TRUE, ch, obj, editman, TO_ROOM );
				act( "$N ti da` $p.", TRUE, ch, obj, editman, TO_CHAR );
				act("$N ti dice 'Ok. Ecco fatto! Buona fortuna!'", FALSE,
					ch, 0, editman, TO_CHAR);
				obj_from_char( obj );
				obj_to_char( obj, ch );
				return (TRUE);
				break;
			case 1:			// aiuto
				SayMenu(ch, aiuto_modifica);
				return (TRUE);
				break;
			}

			if (ITEM_TYPE(obj) == ITEM_ARMOR || ITEM_WEAPON ) {

				if (comandi[iCom].tipo != 0 && comandi[iCom].tipo != OGG
						&& comandi[iCom].tipo != RES && comandi[iCom].tipo != IMM) {
					act("$N ti dice 'Spiacente, non sono in grado di fare quello che mi chiedi.'", FALSE,
						ch, 0, editman, TO_CHAR);
					return (TRUE);
				}

				switch (comandi[iCom].cmd) {
				case 2:		// move
				case 3:		// mana
				case 4:		// hp
				case 5:		// hitroll
				case 6:		// damroll
				case 7:		// armor
				case 8:		// move_regen
				case 9:		// mana_regen
				case 10:	// hp_regen
				case 11:	// str
				case 12:	// dex
				case 13:	// con
				case 14:	// wis
				case 15:	// int
				case 16:	// chr
				case 17:	// spellfail
					for (ciclo = MAX_OBJ_AFFECT - 1, iSpell = iMagie = 0, temp = -1; ciclo >= 0; ciclo--)
						if (obj->affected[ciclo].location) {
							iSpell++;
							if (obj->affected[ciclo].location == comandi[iCom].azione)
							{ iMagie++; }
						}
						else
						{ temp = ciclo; }
					if (temp<0) {
						act("$N ti dice 'Non capisco'", FALSE,
							ch, 0, editman, TO_CHAR);
						return true;
					}
					arg = one_argument(arg, parmstr);
					if (atol(parmstr) <= 0L && !(comandi[iCom].cmd == 7 || comandi[iCom].cmd == 17)) {
						act("$N ti dice 'Non e' che mi diresti anche il valore?'", FALSE,
							ch, 0, editman, TO_CHAR);
						return (TRUE);
					}
					else if (atol(parmstr) >= 0L && (comandi[iCom].cmd == 7 || comandi[iCom].cmd == 17)) {
						act("$N ti dice 'Non e' che mi diresti anche il valore?'", FALSE,
							ch, 0, editman, TO_CHAR);
						return (TRUE);
					}
					if (iSpell < MAX_OBJ_AFFECT && iMagie < 1
							&& atol(parmstr) <= comandi[iCom].max ) {
						if (pagamento) {
							if ((GET_EXP(ch)-400000000L) < tot_costoxp + static_cast<int>(calc_costoxp(iCom, atoi(parmstr)))) {
								act("$N ti dice 'Spiacente, non hai abbastanza XP.'", FALSE,
									ch, 0, editman, TO_CHAR);
								return (TRUE);
							}
						}
						else {
							if (GET_RUNEDEI(ch)-1 < tot_costopq + static_cast<int>(calc_costopq(iCom, atoi(parmstr)))) {
								act("$N ti dice 'Spiacente, non hai abbastanza punti quest.'", FALSE,
									ch, 0, editman, TO_CHAR);
								return (TRUE);
							}
						}
						act("$N si mette al lavoro.", FALSE,
							ch, 0, editman, TO_CHAR);
						obj->affected[temp].location = comandi[iCom].azione;
						/*			obj->affected[temp].modifier = (atol(parmstr) * comandi[iCom].add); */
						obj->affected[temp].modifier = atol(parmstr) ;
						tot_costoxp +=static_cast<int>(calc_costoxp(iCom, atoi(parmstr)));
						tot_costopq +=static_cast<int>(calc_costopq(iCom, atoi(parmstr)));
						mudlog(LOG_PLAYERS,"%s modifica %s %s su %s e paga %d xp o %d pq", GET_NAME(ch),comandi[iCom].com,parmstr,obj->name,static_cast<int>(calc_costoxp(iCom,atoi(parmstr))),
							   static_cast<int>(calc_costopq(iCom,atoi(parmstr))));
						ha_modificato = TRUE;
					}
					else {
						act("$N ti dice 'Spiacente, non posso proprio.'", FALSE,
							ch, 0, editman, TO_CHAR);
					}
					return (TRUE);
					break;
				}
			}
		}
		else
			act("$N ti dice 'Quale sarebbe l'oggetto in questione?!?'", FALSE,
				ch, 0, editman, TO_CHAR);
		return (TRUE);
	}
	else if (modifica) {
		return (TRUE);
	}
	return (FALSE);
}
} // namespace Alarmud

