/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: gilde.c,v 1.2 2002/02/13 12:30:58 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdint>
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
#include "gilde.hpp"
#include "act.comm.hpp"
#include "act.move.hpp"
#include "act.obj2.hpp"
#include "act.social.hpp"
#include "charlist.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "skills.hpp"
namespace Alarmud {


typedef struct structSocioGilda {
	char szNomeSocio[ 21 ];
	struct structSocioGilda* pNext;
} TSocioGilda;

typedef struct {
	int nGuardia;
	int nGuardiaRoom;
	int nDir;
	int nBanchiere;
	int nBanca;
	int nBanchiereXP;
	int nBancaXP;
	char szBaseFileName[ 81 ];
	int nLibroSoci;
	char szNomeCapo[ 21 ];
	TSocioGilda* pListaSoci;
} TDatiGilda;


TDatiGilda* pDatiGilde = NULL;

#define GUILD_DIR "gilde"
#define GUILD_GOLD_TOT ".gold.tot"
#define GUILD_GOLD_LOG ".gold.log"
#define GUILD_EXP_TOT  ".exp.tot"
#define GUILD_EXP_LOG  ".exp.log"
#define GUILD_MEMBER   ".member"
#define GUILD_DAT      "ombra.gui"

#define GUILD_MAXXP_DEP 100000
#define GUILD_MAXXP_PRE 5000000


/*****************************************************************************
 * Le funzioni che seguono servono per implementare la lista dei membri di
 * una gilda. Mi piacerebbe convertire tutto in C++...
 * **************************************************************************/



/*****************************************************************************
 * Inizializza la lista dei nomi dei membri della gilda leggendoli dal file
 * appropriato. Il parametro nIndex e' l'indice dell'array pDatiGilda
 * corrispondente alla gilda desiderata.
 * **************************************************************************/

void InitializeMemberList( int nIndex ) {
	char szFileName[ 256 ];
	char szDummy[ 161 ];
	FILE* pfMemberList;

	sprintf( szFileName, "%s/%s%s", GUILD_DIR,
			 pDatiGilde[ nIndex ].szBaseFileName, GUILD_MEMBER );

	if( ( pfMemberList = fopen( szFileName, "r+" ) ) != NULL ) {
		szDummy [ 0 ] = 0;
		fscanf( pfMemberList, " %160s \n", szDummy );
		strncpy( pDatiGilde[ nIndex ].szNomeCapo, szDummy,
				 sizeof( pDatiGilde[ nIndex ].szNomeCapo ) - 1 );

		while( !feof( pfMemberList ) ) {
			szDummy [ 0 ] = 0;
			fscanf( pfMemberList, " %160s \n", szDummy );
			if( strlen( szDummy ) ) {
				TSocioGilda* pSocio = (TSocioGilda*)calloc( 1,
									  sizeof( TSocioGilda ) );
				if( pSocio ) {
					strncpy( pSocio->szNomeSocio, szDummy,
							 sizeof( pSocio->szNomeSocio ) - 1 );
					pSocio->pNext = pDatiGilde[ nIndex ].pListaSoci;
					pDatiGilde[ nIndex ].pListaSoci = pSocio;
				}
				else {
					mudlog( LOG_ERROR,
							"Cannot alloc memory in InitializeMemberList (gilde.c)." );
				}
			}
		}

		fclose( pfMemberList );
	}
}

void UpdateGuildListFile( int nIndex ) {
	char szFileName[ 256 ];
	FILE* pfMemberList;

	sprintf( szFileName, "%s/%s%s", GUILD_DIR,
			 pDatiGilde[ nIndex ].szBaseFileName, GUILD_MEMBER );

	if( ( pfMemberList = fopen( szFileName, "w" ) ) != NULL ) {
		TSocioGilda* pSocio;

		fprintf( pfMemberList, "%s\n", pDatiGilde[ nIndex ].szNomeCapo );
		for( pSocio = pDatiGilde[ nIndex ].pListaSoci; pSocio;
				pSocio = pSocio->pNext ) {
			fprintf( pfMemberList, "%s\n", pSocio->szNomeSocio );
		}
		fclose( pfMemberList );
	}
	else {
		mudlog( LOG_ERROR,
				"Cannot create file %s in UpdateGuildListFile (gilde.c).",
				szFileName );
	}
}



int IsInGuildList( int nIndex, const char* szNomeMembro ) {
	TSocioGilda* pSocio;

	for( pSocio = pDatiGilde[ nIndex ].pListaSoci; pSocio;
			pSocio = pSocio->pNext ) {
		if( strcasecmp( pSocio->szNomeSocio, szNomeMembro ) == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}

void AppendToGuildList( int nIndex, const char* szNomeMembro ) {
	TSocioGilda* pSocio = (TSocioGilda*)calloc( 1, sizeof( TSocioGilda ) );
	if( pSocio ) {
		strncpy( pSocio->szNomeSocio, szNomeMembro,
				 sizeof( pSocio->szNomeSocio ) - 1 );
		pSocio->pNext = pDatiGilde[ nIndex ].pListaSoci;
		pDatiGilde[ nIndex ].pListaSoci = pSocio;

		UpdateGuildListFile( nIndex );
	}
	else {
		mudlog( LOG_ERROR,
				"Cannot allocate memory in AppendToGuildList (gilde.c)." );
	}
}

void RemoveFromGuildList( int nIndex, const char* szNomeMembro ) {
	TSocioGilda** ppSocio;
	int bRemoved = FALSE;

	for( ppSocio = &pDatiGilde[ nIndex ].pListaSoci; *ppSocio;
			ppSocio = &(*ppSocio)->pNext ) {
		if( strcasecmp( (*ppSocio)->szNomeSocio, szNomeMembro ) == 0 ) {
			TSocioGilda* pSocio = *ppSocio;
			*ppSocio = (*ppSocio)->pNext;
			free( pSocio );
			bRemoved = TRUE;
			break;
		}
	}

	if( bRemoved ) {
		UpdateGuildListFile( nIndex );
	}
}

/*************************************/
/* predicates for find_path function */

int IsGuildGuardRoomFP( int room, void* tgt_room ) {
	return abs(room) == reinterpret_cast<intptr_t>(tgt_room);
}

/****************************************************************************
 * PlayersGuildGuard va assegnata ai guardiani delle gilde dei giocatori.
 * Il guardiano blocchera` tutti quelli che tentano di entrare nella gilda,
 * ma non sono iscritti nel libro.
 * E` possibile dare una chiave da tenere (HOLD) alla guardia, in modo che
 * in caso di perdita, il capo della gilda la puo` avere.
 * Deve dare il comando 'ask guardia chiave' e la guardia dara` la chiave al
 * capo (e solo al capo). In caso di morte della guardia, questa distruggera`
 * la chiave tenuta.
 ****************************************************************************/

MOBSPECIAL_FUNC(PlayersGuildGuard) {
	int nIndex;

	if( !mob || !ch || mob->nr < 0 ) {
		mudlog( LOG_SYSERR,"!mob || !ch || mob->nr < 0" );
		return FALSE;
	}

	nIndex = mob->generic;

	if( !nIndex ) {

		for( nIndex = 0; pDatiGilde[ nIndex ].nGuardia &&
				pDatiGilde[ nIndex ].nGuardia !=
				mob_index[ mob->nr ].iVNum; nIndex++ );

		if( !pDatiGilde[ nIndex ].nGuardia ) {
			mudlog( LOG_ERROR,
					"PlayersGuildMaster assigned to wrong mob '%s' (%ld).",
					GET_NAME( mob ), mob->nr );
			mob_index[ mob->nr ].func = NULL;
		}
		mob->generic = nIndex + 1;
	}
	else {
		nIndex--;
	}

	if( type == EVENT_COMMAND && AWAKE( mob ) ) {
		if( cmd >= CMD_NORTH && cmd <= CMD_DOWN ) {
			if( mob->in_room == pDatiGilde[ nIndex ].nGuardiaRoom &&
					pDatiGilde[ nIndex ].nDir == cmd - 1 ) {
				if( !IsInGuildList( nIndex, GET_NAME( ch ) ) &&
						strcasecmp( GET_NAME( ch ),
									pDatiGilde[ nIndex ].szNomeCapo ) != 0 &&
						( GetMaxLevel( ch ) < MAESTRO_DEI_CREATORI ||
						  !IS_SET( ch->specials.act, PLR_NOHASSLE ) ) ) {
					do_action( mob, "", CMD_SHAKE );
					act( "$c0015[$c0005$N$c0015] ti dice 'Dove credi di andare tu ? "
						 "L'ingresso e` riservato!'", FALSE, ch, 0, mob, TO_CHAR );
					act( "$c0015[$c0005$N$c0015] dice a $n 'Dove credi di andare tu ? "
						 "L'ingresso e` riservato!'", FALSE, ch, 0, mob, TO_ROOM );
					return TRUE;
				}
			} /* O la stanza o la direzione non sono quelle controllate. */
		}
		else if( cmd == CMD_DOORBASH ) {
			do_action( mob, GET_NAME( ch ), CMD_GLARE );
			act( "$c0015[$c0005$N$c0015] ti dice 'Non ci pensare neppure.'",
				 FALSE, ch, 0, mob, TO_CHAR );
			act( "$c0015[$c0005$N$c0015] dice a $n 'Non ci pensare neppure.'",
				 FALSE, ch, 0, mob, TO_ROOM );
			return TRUE;
		}
		else if( cmd == CMD_ASK ) {
			char szBuffer[ 180 ];
			one_argument( arg, szBuffer );
			if( isname( szBuffer, GET_NAME( mob ) ) ) {
				do_ask( ch, arg, CMD_ASK );
				arg = one_argument( arg, szBuffer );
				if( isname( pDatiGilde[ nIndex ].szNomeCapo, GET_NAME( ch ) ) ) {
					char chEorA = ( GET_SEX( ch ) == SEX_FEMALE ? 'a' : 'e' );
					if( mob->equipment[ HOLD ] &&
							isname2( arg, mob->equipment[ HOLD ]->name ) ) {
						sprintf( szBuffer, "%s Certo signor%c, subito signor%c",
								 GET_NAME( ch ), chEorA, chEorA );
						do_tell( mob, szBuffer, CMD_TELL );
						act( "$n da` $p a $N", TRUE, mob, mob->equipment[ HOLD ], ch,
							 TO_NOTVICT );
						act( "$n ti da` $p", TRUE, mob, mob->equipment[ HOLD ], ch,
							 TO_VICT );
						obj_to_char( unequip_char( mob, HOLD ), ch );
					}
					else {
						sprintf( szBuffer, "%s Mi dispiace signor%c, ma non ce l'ho",
								 GET_NAME( ch ), chEorA );
						do_tell( mob, szBuffer, CMD_TELL );
					}
				}
				else {
					act( "$N continua la sua guardia, ignorando le tue richieste.",
						 TRUE, ch, NULL, mob, TO_CHAR );
					act( "$N continua la sua guardia, ignorando le richieste di $n.",
						 TRUE, ch, NULL, mob, TO_ROOM );
				}
				return TRUE;
			}
		}
	}
	else if( type == EVENT_DEATH ) {
		if( mob->equipment[ HOLD ] ) {
			struct obj_data* pObj = unequip_char( mob, HOLD );
			act( "$n, con un ultimo sforzo disperato, distrugge $p.",
				 TRUE, mob, pObj, 0, TO_ROOM );
			extract_obj( pObj );
		}
	}
	else if( type == EVENT_TICK ) {
		if( !AWAKE( mob ) ) {
			if( !IS_AFFECTED( mob, AFF_SLEEP ) ) {
				do_wake( mob, "", 0 );
				return TRUE;
			}
		}
		else if( GET_POS( mob ) > POSITION_SLEEPING &&
				 GET_POS( mob ) < POSITION_FIGHTING ) {
			do_stand( mob, "", 0 );
			return TRUE;
		}
		else if( mob->in_room != pDatiGilde[ nIndex ].nGuardiaRoom ) {
			int iDir;

			iDir = find_path( mob->in_room, IsGuildGuardRoomFP,
							  reinterpret_cast<void*>(pDatiGilde[ nIndex ].nGuardiaRoom), -5000, 0 );
			if( iDir >= 0 ) {
				go_direction( mob, iDir );
				return TRUE;
			}
		}
		else if( !mob->equipment[HOLD] && mob->carrying ) {
			struct obj_data* pObj;
			for( pObj = mob->carrying; pObj; pObj = pObj->next_content ) {
				if( CAN_WEAR( pObj, ITEM_HOLD ) ) {
					perform_wear( mob, pObj, 13 );
					obj_from_char( pObj );
					equip_char( mob, pObj, HOLD );
					return TRUE;
				}
			}
		}
		else {
			struct char_data* pCapo;
			pCapo = get_char_room_vis( mob, pDatiGilde[ nIndex ].szNomeCapo );

			if( pCapo != NULL ) {
				if( !number( 0, 10 ) ) {
					do_action( mob, GET_NAME( pCapo ), CMD_KNEEL );
					if( GET_SEX( pCapo ) == SEX_MALE ) {
						act( "$c0015[$c0005$N$c0015] ti dice 'Ai suoi ordini signore, "
							 "benvenuto.'", FALSE, pCapo, 0, mob, TO_CHAR );
						act( "$c0015[$c0005$N$c0015] dice a $n 'Ai suoi ordini signore, "
							 "benvenuto.'", FALSE, pCapo, 0, mob, TO_ROOM );
					}
					else if( GET_SEX( pCapo ) == SEX_FEMALE ) {
						act( "$c0015[$c0005$N$c0015] ti dice 'Ai suoi ordini signora, "
							 "benvenuta.'", FALSE, pCapo, 0, mob, TO_CHAR );
						act( "$c0015[$c0005$N$c0015] dice a $n 'Ai suoi ordini signora, "
							 "benvenuta.'", FALSE, pCapo, 0, mob, TO_ROOM );
					}
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/****************************************************************************
 * Procedura di servizio per GuildBanker.
 ****************************************************************************/

void GuildDeposit( struct char_data* ch, struct char_data* mob,const char* arg, int nIndex ) {

	char szBuffer[ 256 ];

	long lGold = -1;

	sscanf( arg, "%ld", &lGold );
	if( lGold > 0 ) {
		if( lGold <= GET_GOLD( ch ) ) {
			FILE* pfTotal;
			char szFileName[ 256 ];

			sprintf( szFileName, "%s/%s%s", GUILD_DIR,
					 pDatiGilde[ nIndex ].szBaseFileName, GUILD_GOLD_TOT );
			if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL ) {
				long lTotalGold = 0;
				rewind( pfTotal );
				fscanf( pfTotal, "%ld\n", &lTotalGold );
				rewind( pfTotal );
				lTotalGold += lGold;
				fprintf( pfTotal, "%ld\n", lTotalGold );
				GET_GOLD( ch ) -= lGold;
				fclose( pfTotal );

				act( "$c0013[$c0015$N$c0013] ti dice 'Grazie di aver "
					 "contribuito alla prosperita` della gilda'", FALSE, ch,
					 NULL, mob, TO_CHAR );
				sprintf( szBuffer, "$n deposita %ld monete nelle casse della gilda.",
						 lGold );
				act( szBuffer, TRUE, ch, NULL, NULL, TO_ROOM );

			}
			else {
				act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
					 FALSE, ch, NULL, mob, TO_CHAR );
				mudlog( LOG_ERROR, "Cannot open file %s in GuildBank",
						szFileName );
			}
		}
		else {
			act( "$c0013[$c0015$N$c0013] ti dice 'Non mi sembra che "
				 "tu abbia tutte quelle monete'", FALSE, ch, NULL, mob,
				 TO_CHAR );
		}
	}
	else {
		act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
			 "quante monete indende versare. Puo` ripetere ?'", FALSE, ch,
			 NULL, mob, TO_CHAR );
	}
}

/****************************************************************************
 * Procedura di servizio per GuildBanker.
 ****************************************************************************/

void GuildBalance( struct char_data* pCh, struct char_data* pMob,int nIndex ) {
	if( ( isname( pDatiGilde[ nIndex ].szNomeCapo, GET_NAME( pCh ) ) ||
			GetMaxLevel( pCh ) >= MAESTRO_DEI_CREATORI ) ) {
		FILE* pfTotal;
		char szFileName[ 256 ];

		sprintf( szFileName, "%s/%s%s", GUILD_DIR,
				 pDatiGilde[ nIndex ].szBaseFileName, GUILD_GOLD_TOT );
		if( ( pfTotal = fopen( szFileName, "a+" ) ) != NULL ) {
			char szBuffer[ 256 ];
			long lTotalGold = 0;

			rewind( pfTotal );
			fscanf( pfTotal, "%ld\n", &lTotalGold );
			fclose( pfTotal );
			sprintf( szBuffer, "$c0013[$c0015$N$c0013] ti dice '"
					 "Nelle casse della gilda ci sono %ld monete d'oro'",
					 lTotalGold );
			act( szBuffer, FALSE, pCh, NULL, pMob, TO_CHAR );
		}
		else {
			act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
				 FALSE, pCh, NULL, pMob, TO_CHAR );
			mudlog( LOG_ERROR, "Cannot open file %s in GuildBank",
					szFileName );
		}
	}
	else {
		act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma sono "
			 "informazioni riservate'", FALSE, pCh, NULL, pMob, TO_CHAR );
	}
}

/****************************************************************************
 * Procedura di servizio per GuildBanker.
 ****************************************************************************/

void GuildWithdraw( struct char_data* pCh, struct char_data* pMob,const char* pArg, int nIndex ) {
	struct char_data* pGuildMaster;

	if( ( ( pGuildMaster = get_char_room( pDatiGilde[ nIndex ].szNomeCapo,
										  pCh->in_room ) ) != NULL &&
			!IS_LINKDEAD( pGuildMaster ) ) ) {
		long lGold = -1;
		sscanf( pArg, "%ld", &lGold );
		if( lGold > 0 ) {
			FILE* pfTotal;
			char szFileName[ 256 ];

			sprintf( szFileName, "%s/%s%s", GUILD_DIR,
					 pDatiGilde[ nIndex ].szBaseFileName, GUILD_GOLD_TOT );
			if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL ) {
				long lTotalGold = 0;
				rewind( pfTotal );
				fscanf( pfTotal, "%ld\n", &lTotalGold );
				if( lGold <= lTotalGold ) {
					char szBuffer[ 256 ];
					rewind( pfTotal );
					lTotalGold -= lGold;
					fprintf( pfTotal, "%ld\n", lTotalGold );
					GET_GOLD( pCh ) += lGold;
					act( "$c0013[$c0015$N$c0013] ti dice 'Ecco i soldi richiesti'",
						 FALSE, pCh, NULL, pMob, TO_CHAR );
					sprintf( szBuffer, "$n preleva %ld monete dalle casse della gilda.",
							 lGold );
					act( szBuffer, TRUE, pCh, NULL, NULL, TO_ROOM );

				}
				else {
					act( "$c0013[$c0015$N$c0013] ti dice 'La banca non ha "
						 "tutte quelle monete'", FALSE, pCh, NULL, pMob, TO_CHAR );
				}
				fclose( pfTotal );

			}
			else {
				act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
					 FALSE, pCh, NULL, pMob, TO_CHAR );
				mudlog( LOG_ERROR, "Cannot open file %s in GuildBank",
						szFileName );
			}
		}
		else {
			act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
				 "quante monete indende versare. Puo` ripetere ?'", FALSE, pCh,
				 NULL, pMob, TO_CHAR );
		}
	}
	else {
		char pMsg[] = "$c0015[$c0005$N$c0015] dice 'Il prelievo puo` essere "
					  "fatto solo in presenza del capo della gilda'";
		act( pMsg, FALSE, pCh, NULL, pMob, TO_CHAR );
		act( pMsg, FALSE, pCh, NULL, pMob, TO_ROOM );
	}
}

/**************************************************************************
 * GuildBank permette di realizzare le banche delle gilde. A differenza
 * delle banche normali, i soldi versati, rimangono nella banca.
 *
 * Possono depositare nelle banche solo gli appartenenti alla gilda iscritti
 * nel libro della gilda.
 *
 * Il bilancio della banca lo puo` leggere solo il capogilda, mentre
 * possono prelevare dalla banca solo gli appartenenti alla gilda che
 * tengono (HOLD) il simbolo della gilda e solo in presenza del capogilda.
 *
 *************************************************************************/

MOBSPECIAL_FUNC(GuildBanker) {

	if( !mob || !ch ) {
		mudlog( LOG_SYSERR, "pCh or pMob == NULL in GuildBank" );
		return FALSE;
	}

	if( type == EVENT_COMMAND &&
			( cmd == CMD_DEPOSIT || cmd == CMD_WITHDRAW || cmd == CMD_BALANCE ) ) {
		int nIndex;
		for( nIndex = 0; pDatiGilde[ nIndex ].nBanchiereXP &&
				pDatiGilde[ nIndex ].nBanchiereXP !=
				mob_index[ mob->nr ].iVNum; nIndex++ );

		if( pDatiGilde[ nIndex ].nBanchiere ) {

			if( pDatiGilde[ nIndex ].nBanca &&
					mob->in_room != pDatiGilde[ nIndex ].nBanca )
			{ return FALSE; }

			if( IS_PC( ch ) ) {

				if( IsInGuildList( nIndex, GET_NAME( ch ) ) ||
						strcasecmp( GET_NAME( ch ),
									pDatiGilde[ nIndex ].szNomeCapo ) == 0 ||
						( GetMaxLevel( ch ) >= MAESTRO_DEI_CREATORI &&
						  IS_SET( ch->specials.act, PLR_NOHASSLE ) ) ) {
					switch( cmd ) {
					case CMD_DEPOSIT:
						GuildDeposit( ch, mob, arg, nIndex );
						break;

					case CMD_BALANCE:
						GuildBalance( ch, mob, nIndex );
						break;

					case CMD_WITHDRAW:
						GuildWithdraw( ch, mob, arg, nIndex );
						break;
					}
				}
				else {
					char pMsg[] = "$c0015[$c0005$N$c0015] dice 'Mi dispiace, ma solo i "
								  "membri della gilda possono usare la banca'";
					act( pMsg, FALSE, ch, NULL, mob, TO_CHAR );
					act( pMsg, FALSE, ch, NULL, mob, TO_ROOM );
				}
			}
			else {
				act( "Solo i giocatori possono usare la banca.", FALSE, ch,
					 NULL, NULL, TO_CHAR );
			}

			return TRUE;
		}
		else {
			mudlog( LOG_ERROR,
					"GuildXPBanker assigned to wrong mob '%s' (%ld).",
					GET_NAME( mob ), mob->nr );
		}
	}

	return FALSE;
}

/****************************************************************************
 * Procedura di servizio per GuildXPBanker.
 ****************************************************************************/

int GuildBalanceXP( struct char_data* pChar, struct char_data* pMob,int nIndex ) {
	if( ( isname( pDatiGilde[ nIndex ].szNomeCapo, GET_NAME( pChar ) ) ||
			GetMaxLevel( pChar ) >= MAESTRO_DEI_CREATORI ) ) {
		CharElem* pCurr;
		char szBuffer[ 128 ];
		struct string_block sb;
		FILE* pfTotal;
		char szFileName[ 256 ];


		init_string_block( &sb );
		sprintf( szBuffer,
				 "$c0013[$c0015%.20s$c0013] ti dice 'Ecco l'estratto conto:'"
				 "$c0007\n\r\n\r", GET_NAME_DESC( pMob ) );
		append_to_string_block( &sb, szBuffer );
		append_to_string_block( &sb, "Versamenti nelle ultime 24 ore:\n\r\n\r" );

		for( pCurr = (CharElem*)pMob->act_ptr; pCurr;
				pCurr = pCurr->pNext ) {
			if( pCurr->nIntData > 0 ) {
				sprintf( szBuffer, "%-20.20s %7d\n\r",
						 GET_NAME( (struct char_data*)pCurr->pWho ),
						 pCurr->nIntData );
				append_to_string_block( &sb, szBuffer );
			}
		}
		append_to_string_block( &sb, "                     -------\n\r" );
		sprintf( szBuffer, "Totale               %7d\n\r",
				 SumIntDataPos( (CharElem*)pMob->act_ptr ) );
		append_to_string_block( &sb, szBuffer );

		append_to_string_block( &sb,
								"\n\rPrelevamenti negli ultimi 3 giorni:\n\r\n\r" );

		for( pCurr = (CharElem*)pMob->act_ptr; pCurr;
				pCurr = pCurr->pNext ) {
			if( pCurr->nIntData < 0 ) {
				sprintf( szBuffer, "%-20.20s %7d\n\r",
						 GET_NAME( (struct char_data*)pCurr->pWho ),
						 -pCurr->nIntData );
				append_to_string_block( &sb, szBuffer );
			}
		}
		append_to_string_block( &sb, "                     -------\n\r" );
		sprintf( szBuffer, "Totale               %7d\n\r",
				 SumIntDataNeg( (CharElem*)pMob->act_ptr ) );
		append_to_string_block( &sb, szBuffer );

		sprintf( szFileName, "%s/%s%s", GUILD_DIR,
				 pDatiGilde[ nIndex ].szBaseFileName, GUILD_EXP_TOT );
		if( ( pfTotal = fopen( szFileName, "a+" ) ) != NULL ) {
			long lTotalXP = 0;

			rewind( pfTotal );
			fscanf( pfTotal, "%ld\n", &lTotalXP );
			fclose( pfTotal );
			sprintf( szBuffer,
					 "\n\rLa gilda ha %ld punti esperienza a disposizione.\n\r",
					 lTotalXP );
			append_to_string_block( &sb, szBuffer );

		}
		else {
			mudlog( LOG_ERROR, "Cannot open file %s in GuildListXP (gilde.c)",
					szFileName );
			act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
				 FALSE, pChar, NULL, pMob, TO_CHAR );
		}

		page_string_block( &sb, pChar );
		destroy_string_block( &sb );
	}
	else {
		act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
			 "ma sono informazioni riservate'", FALSE, pChar, NULL,
			 pMob, TO_CHAR );
	}
	return TRUE;
}

/****************************************************************************
 * Procedura di servizio per GuildXPBanker.
 ****************************************************************************/

int GuildDepositXP( struct char_data* pChar,  const char* szArg,struct char_data* pMob, int nIndex ) {
	int iAlreadyDep = GetIntData( (CharElem*)pMob->act_ptr, pChar );
	if( iAlreadyDep < GUILD_MAXXP_DEP ) {
		long lXP = -1;
		sscanf( szArg, "%ld", &lXP );
		if( lXP > 0 ) {
			int i, nMaxXPLevNeed = 0;
			for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ )
				nMaxXPLevNeed = MAX( nMaxXPLevNeed,
									 titles[ i ][ (int)GET_LEVEL( pChar, i ) ].exp );
			if( GET_EXP( pChar ) > ( nMaxXPLevNeed +
									 ( lXP / HowManyClasses(pChar ) ) ) &&
					iAlreadyDep + lXP <= GUILD_MAXXP_DEP ) {
				FILE* pfTotal;
				char szFileName[ 256 ];

				sprintf( szFileName, "%s/%s%s", GUILD_DIR,
						 pDatiGilde[ nIndex ].szBaseFileName, GUILD_EXP_TOT );
				if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL ) {
					char szBuffer[ 160 ];
					long lTotalXP = 0;

					rewind( pfTotal );
					fscanf( pfTotal, "%ld\n", &lTotalXP );
					rewind( pfTotal );
					lTotalXP += lXP;
					fprintf( pfTotal, "%ld\n", lTotalXP );
					GET_EXP( pChar ) -= lXP / HowManyClasses( pChar );
					fclose( pfTotal );
					InsertInListInt( (CharElem**)&pMob->act_ptr, pChar,
									 ( SECS_PER_MUD_HOUR * 4 * 24 ) / PULSE_MOBILE,
									 (int)lXP );

					act( "$c0013[$c0015$N$c0013] ti dice 'Grazie di aver "
						 "contribuito alla prosperita` della gilda'", FALSE,
						 pChar, NULL, pMob, TO_CHAR );
					sprintf( szBuffer, "$n deposita %ld XP nelle casse della gilda.",
							 lXP );
					act( szBuffer, TRUE, pChar, NULL, NULL, TO_ROOM );
				}
				else {
					mudlog( LOG_ERROR, "Cannot open file %s in GuildDepositXP (gilde.c)",
							szFileName );
					act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
						 FALSE, pChar, NULL, pMob, TO_CHAR );
					return FALSE;
				}
			}
			else {
				if( ( GET_EXP( pChar ) - nMaxXPLevNeed ) > 0 ) {
					char szBuffer[ 160 ];
					sprintf( szBuffer,
							 "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
							 "ma puoi versare al massimo %d XP.'",
							 MIN( ( GET_EXP( pChar ) - nMaxXPLevNeed ) *
								  HowManyClasses( pChar ),
								  GUILD_MAXXP_DEP - iAlreadyDep ) );
					act( szBuffer, FALSE, pChar, NULL, pMob, TO_CHAR );
				}
				else {
					act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
						 "ma non hai XP da versare.'", FALSE, pChar, NULL,
						 pMob, TO_CHAR );
				}
			}
		}
		else {
			act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
				 "quanti XP indendi versare. Puoi ripetere ?'", FALSE, pChar,
				 NULL, pMob, TO_CHAR );
		}
	}
	else {
		act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
			 "ma non puoi effettuare altri versamenti per il momento.'", FALSE,
			 pChar, NULL, pMob, TO_CHAR );
	}

	return TRUE;
}

/****************************************************************************
 * Procedura di servizio per GuildXPBanker.
 ****************************************************************************/

int GuildWithdrawXP( struct char_data* pChar,  const char* szArg,struct char_data* pMob, int nIndex ) {
	long lXP = -1;
	sscanf( szArg, "%ld", &lXP );
	if( lXP > 0 ) {
		if( get_char_room( pDatiGilde[ nIndex ].szNomeCapo, pChar->in_room ) !=
				NULL ) {
			FILE* pfTotal;
			char szFileName[ 256 ];

			sprintf( szFileName, "%s/%s%s", GUILD_DIR,
					 pDatiGilde[ nIndex ].szBaseFileName, GUILD_EXP_TOT );
			if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL ) {
				long lTotalXP = 0;

				rewind( pfTotal );
				fscanf( pfTotal, "%ld\n", &lTotalXP );

				if( ( lXP + SumIntDataNeg( (CharElem*)pMob->act_ptr, pChar ) ) <=
						GUILD_MAXXP_PRE && lXP <= lTotalXP ) {
					long lOldXP = GET_EXP( pChar );

					gain_exp( pChar, lXP );
					lXP = ( GET_EXP( pChar ) - lOldXP ) * HowManyClasses( pChar );
					if( lXP > 0 ) {
						char szBuffer[ 256 ];

						rewind( pfTotal );
						lTotalXP -= lXP;
						fprintf( pfTotal, "%ld\n", lTotalXP );
						sprintf( szBuffer,
								 "$c0013[$c0015$N$c0013] ti dice 'Hai prelevato %ld "
								 "punti esperienza.'", lXP );
						act( szBuffer, FALSE, pChar, NULL, pMob, TO_CHAR );
						sprintf( szBuffer,
								 "$n preleva %ld punti esperienza dalle casse della "
								 "gilda.", lXP );
						act( szBuffer, TRUE, pChar, NULL, NULL, TO_ROOM );
						InsertInListInt( (CharElem**)&pMob->act_ptr, pChar,
										 ( SECS_PER_MUD_HOUR * 4 * 24 * 3 ) / PULSE_MOBILE,
										 (int)( -lXP ) );
					}
					else {
						act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma non posso "
							 "darti punti esperienza.'", FALSE, pChar, NULL, pMob,
							 TO_CHAR );
					}
					fclose( pfTotal );
				}
				else {
					long lMaxToPrel = MIN( GUILD_MAXXP_PRE -
										   SumIntDataNeg( (CharElem*)pMob->act_ptr,
														  pChar ),
										   lTotalXP );
					if( lMaxToPrel <= 0 ) {
						act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma non si "
							 "possono prelevare altri XP per il momento.'", FALSE, pChar,
							 NULL, pMob, TO_CHAR );
					}
					else {
						char szBuffer[ 256 ];
						sprintf( szBuffer,
								 "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma puoi "
								 "prelevare al massimo %ld punti per il momento.'",
								 lMaxToPrel );
						act( szBuffer, FALSE, pChar, NULL, pMob, TO_CHAR );
					}
				}
			}
			else {
				mudlog( LOG_ERROR,
						"Cannot open file %s in GuildWithdrawXP (gilde.c).",
						szFileName );
				act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
					 FALSE, pChar, NULL, pMob, TO_CHAR );
			}
		}
		else {
			act( "$c0013[$c0015$N$c0013] ti dice 'E` possibile prelevare solo in "
				 "presenza del capogilda.'", FALSE, pChar, NULL, pMob, TO_CHAR );
		}
	}
	else {
		act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
			 "quanti XP indendi prelevare. Puoi ripetere ?'", FALSE, pChar,
			 NULL, pMob, TO_CHAR );
	}
	return TRUE;
}


/****************************************************************************
 * GuildXPBanker va assegnata al banchiere della banca di XP delle gilde dei
 * giocatori.
 *
 * I comandi ammessi sono:
 *
 * deposit puo` essere dato da chiunque sia iscritto nel libro della gilda.
 *         E` possibile depositare al massimo 100.000 XP al giorno e solo la
 *         quantita` di XP che supera quella necessaria per il livello a cui
 *         e` il giocatore.
 * withdraw puo` essere dato da chiunque sia iscritto nel libro della gilda
 *          e solo inpresenza del capogilda.
 *          E` possibile prelevare un massimo di 5.000.000 ogni 3 giorni ed
 *          al massimo il quantitativo di XP necessario a due livelli superiori
 *          all'attuale.
 * balance puo` essere dato solo dal capo della gilda. Permette di vedere la
 *         quantita` di XP presenti nella banca ed una lista
 *         dei giocatori che hanno versato XP nelle ultime ventiquattrore o
 *         prelevato negli ultimi 3 giorni.
 ****************************************************************************/

MOBSPECIAL_FUNC(GuildXPBanker) {
	if( mob == NULL ) {
		mudlog( LOG_SYSERR, "pMob == NULL in GuildMemberBook (gilde.c)" );
		return TRUE;
	}
	else if( mob->nr >= 0 ) {
		if( type == EVENT_COMMAND &&
				( cmd == CMD_DEPOSIT || cmd == CMD_WITHDRAW ||
				  cmd == CMD_BALANCE ) ) {
			int i;
			for( i = 0; pDatiGilde[ i ].nBanchiereXP &&
					pDatiGilde[ i ].nBanchiereXP !=
					mob_index[ mob->nr ].iVNum; i++ );

			if( pDatiGilde[ i ].nBanchiereXP ) {

				if( pDatiGilde[ i ].nBancaXP &&
						mob->in_room != pDatiGilde[ i ].nBancaXP )
				{ return FALSE; }

				if( IS_PC( ch ) ) {

					if( IsInGuildList( i, GET_NAME( ch ) ) ||
							strcasecmp( GET_NAME( ch ),
										pDatiGilde[ i ].szNomeCapo ) == 0 ||
							( GetMaxLevel( ch ) >= MAESTRO_DEI_CREATORI &&
							  IS_SET( ch->specials.act, PLR_NOHASSLE ) ) ) {
						switch( cmd ) {
						case CMD_BALANCE:
							GuildBalanceXP( ch, mob, i );
							break;
						case CMD_DEPOSIT:
							GuildDepositXP( ch,  arg, mob, i );
							break;
						case CMD_WITHDRAW:
							GuildWithdrawXP( ch,  arg, mob, i );
							break;
						}
					}
					else {

						act( "$c0013[$c0015$N$c0013] dice 'Mi dispiace, ma solo i "
							 "membri della gilda possono usare la banca'", FALSE, ch,
							 NULL, mob, TO_CHAR );
					}
				}
				else {
					act( "Solo i giocatori possono usare la banca.", FALSE, ch,
						 NULL, NULL, TO_CHAR );
				}

				return TRUE;
			}
			else {
				mudlog( LOG_ERROR,
						"GuildXPBanker assigned to wrong mob '%s' (%ld).",
						GET_NAME( mob ), mob->nr );
			}

		}
		else if( type == EVENT_TICK ) {
			UpdateList( (CharElem**)&mob->act_ptr );
		}
		else if( type == EVENT_DEATH ) {
			FreeList( (CharElem**)&mob->act_ptr );
		}
	}

	return FALSE;
}

/****************************************************************************
 * Questa funzione viene attaccata al libro dei membri della gilda. Il capo
 * gilda puo` inserire nuovi membri (che devono essere presenti) con
 * write <nomemembro> <nomelibro>, cancellarli con
 * remove <nomemembro> <nomelibro> e visualizzarne la lista con
 * read <nomelibro> (quest'ultimo comando non e` limitato al solo capogilda).
 * *************************************************************************/

OBJSPECIAL_FUNC(GuildMemberBook) {
	if( obj == NULL ) {
		mudlog( LOG_SYSERR, "pObj == NULL in GuildMemberBook (gilde.c)" );
		return TRUE;
	}
	else if( obj->item_number >= 0 ) {
		if( type == EVENT_COMMAND && ( cmd == CMD_READ || cmd == CMD_WRITE ||
										cmd == CMD_REMOVE ) ) {
			int i;
			for( i = 0; pDatiGilde[ i ].nLibroSoci &&
					pDatiGilde[ i ].nLibroSoci !=
					obj_index[ obj->item_number ].iVNum; i++ );

			if( pDatiGilde[ i ].nLibroSoci ) {
				if( cmd == CMD_READ ) {
					char szArgument[ 256 ];
					one_argument( arg, szArgument );
					if( get_obj_vis_accessible( ch, szArgument ) == obj ) {
						char szBuffer[ 256 ];
						TSocioGilda* pSocio;
						act( "Questo e` $p", FALSE, ch, obj, NULL, TO_CHAR );
						sprintf( szBuffer, "Il capo della gilda e` %c%s.",
								 UPPER( pDatiGilde[ i ].szNomeCapo[ 0 ] ),
								 pDatiGilde[ i ].szNomeCapo + 1 );
						act( szBuffer, FALSE, ch, NULL, NULL, TO_CHAR );
						act( "I membri della gilda sono:", FALSE, ch, NULL,
							 NULL, TO_CHAR );
						act( "$n esamina $p", TRUE, ch, obj, NULL, TO_ROOM );

						for( pSocio = pDatiGilde[ i ].pListaSoci; pSocio;
								pSocio = pSocio->pNext ) {
							act( pSocio->szNomeSocio, FALSE, ch, NULL, NULL,  TO_CHAR );
						}
						return TRUE;
					}
				}
				else if( cmd == CMD_WRITE ) {
					char szNomeMembro[ 256 ];
					char szNomeLibro[ 256 ];
					struct char_data* pMembro;

					argument_interpreter( arg, szNomeMembro, szNomeLibro );

					if( get_obj_vis( ch, szNomeLibro ) == obj ) {
						if( strcasecmp( GET_NAME( ch ),
										pDatiGilde[ i ].szNomeCapo ) == 0 ) {
							if( ( pMembro = get_char_room_vis( ch, szNomeMembro ) ) !=
									NULL ) {
								if( pMembro != ch ) {
									if( IS_PC( pMembro ) ) {
										if( !IsInGuildList( i, GET_NAME( pMembro ) ) ) {
											AppendToGuildList( i, GET_NAME( pMembro ) );
											act( "$n scrive il nome di $N su $p.", TRUE, ch,
												 obj, pMembro, TO_NOTVICT );
											act( "$n scrive il tuo nome su $p.", TRUE, ch, obj,
												 pMembro, TO_VICT );
											act( "Scrivi il nome di $N su $p.", TRUE, ch, obj,
												 pMembro, TO_CHAR );
										}
										else {
											act( "$N e` gia` membro della gilda.", TRUE, ch, NULL,
												 pMembro, TO_CHAR );
										}
									}
									else {
										act( "Puoi iscrivere solo giocatori nella tua gilda.",
											 TRUE, ch, NULL, NULL, TO_CHAR );
									}
								}
								else {
									act( "Ma tu sei gia` il capo.", TRUE, ch, NULL, NULL,
										 TO_CHAR );
								}
							}
							else {
								act( "Il nuovo membro deve essere presente durante "
									 "l'iscrizione.", FALSE, ch, NULL, NULL, TO_CHAR );
							}
						}
						else {
							act( "Mi dispiace, ma solo il capo della gilda puo` aggiungere "
								 "nuovi membri.", FALSE, ch, NULL, NULL, TO_CHAR );
						}
						return TRUE;
					}
				}
				else if( cmd == CMD_REMOVE ) {
					char szNomeMembro[ 256 ];
					char szNomeLibro[ 256 ];

					argument_interpreter( arg, szNomeMembro, szNomeLibro );

					if( get_obj_vis( ch, szNomeLibro ) == obj ) {
						if( strcasecmp( GET_NAME( ch ),
										pDatiGilde[ i ].szNomeCapo ) == 0 ) {
							if( IsInGuildList( i, szNomeMembro ) ) {
								if( strcasecmp( szNomeMembro, GET_NAME( ch ) ) != 0 ) {
									RemoveFromGuildList( i, szNomeMembro );
									act( "$n cancella un nome da $p", TRUE, ch, obj, NULL,
										 TO_ROOM );
									act( "Fatto.", FALSE, ch, NULL, NULL, TO_CHAR );
								}
								else {
									act( "Non puoi cancellarti dalla lista. Tu sei il capo!",
										 FALSE, ch, NULL, NULL, TO_CHAR );
								}
							}
							else {
								act( "Quel nome non e` presente nella lista.", FALSE, ch,
									 NULL, NULL, TO_CHAR );
							}
						}
						else {
							act( "Mi dispiace, ma solo il capo della gilda puo` cancellare "
								 "i membri.", FALSE, ch, NULL, NULL, TO_CHAR );
						}
						return TRUE;
					}
				}
			}
			else {
				mudlog( LOG_SYSERR, "GuildMemberBook assigned to obj not in "
						"pDatiGilde (gilde.c)" );
			}
		}
	}

	return FALSE;
}


/****************************************************************************
 * Queste tre funzioni (assignMob, AssignRoom ed AssignObj) servono per
 * assegnare delle procedure speciali a, rispettivamente, un mobile, una
 * stanza ed un oggetto.
 * *************************************************************************/

void AssignMob( int vnum, mobspecial_func proc ) {

	if( vnum > 0 ) {
		int rnum;
		if( ( rnum = real_mobile( vnum ) ) < 0 ) {
			mudlog( LOG_ERROR,"Mobile %d not found in database.",vnum );
		}
		else {
			mob_index[ rnum ].func = reinterpret_cast<genericspecial_func>(proc);
		}
	}
}

void AssignRoom( int vnum, roomspecial_func proc ) {

	if( vnum > 0 ) {
		struct room_data* pRoom;
		if( ( pRoom = real_roomp( vnum ) ) == NULL ) {
			mudlog( LOG_ERROR,"Room %d not found in database.",vnum );
		}
		else {
			pRoom->funct = proc;
		}
	}
}

void AssignObj( int vnum, objspecial_func proc ) {

	if( vnum > 0 ) {
		int rnum;
		if( ( rnum = real_object( vnum ) ) < 0 ) {
			mudlog( LOG_ERROR,
					"Object %d not found in database. AssignObj (gilde.c)",
					vnum );
		}
		else {
			obj_index[ rnum ].func = reinterpret_cast<genericspecial_func>(proc);
		}
	}
}

/***************************************************************************
 * BootGilde si occupa di leggere il file GUILD_DAT ed assegnare le
 * procedure speciali ai mob ed agli oggetti delle gilde.
 * I mob sono la guardia della gilda, il banchiere ed il banchiere degli XP.
 * Poi c'e' il libro dei soci nel quale e possibile inserire i nomi dei
 * soci della gilda.
 **************************************************************************/

void BootGuilds() {
	FILE* pfDatiGilde;
	char szDummy[ 160 ];

	if( ( pfDatiGilde = fopen( GUILD_DAT, "r+" ) ) != NULL ) {
		int nMax = 0;
		while( !feof( pfDatiGilde ) ) {
			fgets( szDummy, sizeof( szDummy ), pfDatiGilde );
			nMax++;
		}

		rewind( pfDatiGilde );

		if( ( pDatiGilde = (TDatiGilda*)calloc( nMax + 1,
												sizeof( TDatiGilda ) ) ) != NULL ) {
			int i;

			for( i = 0; i < nMax && !feof( pfDatiGilde ); i++ ) {
				fscanf( pfDatiGilde, "%80s %d %d %d %d %d %d %d %d\n",
						pDatiGilde[ i ].szBaseFileName, &pDatiGilde[ i ].nGuardia,
						&pDatiGilde[ i ].nGuardiaRoom,   &pDatiGilde[ i ].nDir,
						&pDatiGilde[ i ].nBanchiere,     &pDatiGilde[ i ].nBanca,
						&pDatiGilde[ i ].nBanchiereXP,   &pDatiGilde[ i ].nBancaXP,
						&pDatiGilde[ i ].nLibroSoci );

				AssignMob( pDatiGilde[ i ].nGuardia, PlayersGuildGuard );
				AssignMob( pDatiGilde[ i ].nBanchiere, GuildBanker );
				AssignMob( pDatiGilde[ i ].nBanchiereXP, GuildXPBanker );
				AssignObj( pDatiGilde[ i ].nLibroSoci, GuildMemberBook );
				InitializeMemberList( i );
			}
		}
		else {
			mudlog( LOG_ERROR,"Cannot alloc memory for pDatiGilde" );
		}

		fclose( pfDatiGilde );
	}
}
} // namespace Alarmud

