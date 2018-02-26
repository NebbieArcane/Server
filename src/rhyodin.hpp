/*$Id: rhyodin.h,v 1.2 2002/02/13 12:30:59 root Exp $
*/
/***************************************************************************
  rhyodin.c contiene per procedure speciale per la zona di Rhyodin e la
  sua quest.
***************************************************************************/
#if !defined(_RHYODIN_H)
#define _RHYODIN_H
#include "config.hpp"

int keystone( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			  int type );
int ghostsoldier( struct char_data* ch, int cmd, char* arg,
				  struct char_data* mob, int type );
int Valik( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
		   int type );
int guardian( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			  int type );
int lattimore( struct char_data* pChar, int mCmd, char* szArg,
			   struct char_data* pMob, int nType );
int coldcaster( struct char_data* ch, int cmd, char* arg,
				struct char_data* mob, int type );
int behir( struct char_data* ch, int cmd, char* arg,
		   struct char_data* mob, int type );
int trapper( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			 int type );
int trogcook( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			  int type );
int shaman( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			int type );
int golgar( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			int type );
int troguard( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
			  int type );
int web_slinger( struct char_data* ch, int cmd, char* arg,
				 struct char_data* mob, int type );
int ragno_intermittente( struct char_data* ch, int cmd, char* arg,
						 struct char_data* mob, int type );
int hit_sucker( struct char_data* ch, int cmd, char* arg,
				struct char_data* mob, int type );
int Ankheg( struct char_data* ch, int cmd, char* arg,
			struct char_data* mob, int type );
int Orso_Bianco( struct char_data* ch, int cmd, char* arg,
				 struct char_data* mob, int type );
int Moribondo( struct char_data* pChar, int nCmd, char* szArg,
			   struct char_data* pMob, int nType );

#endif
