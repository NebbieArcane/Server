/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: lucertole.h,v 1.2 2002/02/13 12:31:00 root Exp $
*/
#ifndef _LUCERTOLE_HPP
#define _LUCERTOLE_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/*************************************************************************
 * lucertole.c contiene le procedure speciali per la foresta delle
 * lucertole di Benem
 * **********************************************************************/

MOBSPECIAL_FUNC(Lizardman);
MOBSPECIAL_FUNC(lizardman_shaman);
MOBSPECIAL_FUNC(village_woman);
MOBSPECIAL_FUNC(snake_avt);
MOBSPECIAL_FUNC(snake_avt2);
MOBSPECIAL_FUNC(virgin_sac);
MOBSPECIAL_FUNC(snake_guardian);
ROOMSPECIAL_FUNC(CapannaVillaggio);
ROOMSPECIAL_FUNC(ColloSerpente);
ROOMSPECIAL_FUNC(Rampicante);


#endif
} // namespace Alarmud

