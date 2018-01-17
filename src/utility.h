/*$Id: utility.h,v 1.2 2002/02/13 12:30:59 root Exp $
*/
#if !defined( _UTILITY_H )

bool ThereIsPCInRoom( long lRoom );
bool CanSeePCInRoom( struct char_data *pMob );
int HowManyPCInRoom( long lRoom );
int HowManyCanSeePCInRoom( struct char_data *pMob );

#define _UTILITY_H
#endif
