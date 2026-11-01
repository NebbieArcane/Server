/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_PC_SERVICES_HPP
#define __SPEC_PC_SERVICES_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* Servizi a pagamento sul PG. MobIdent/Esattore/… restano nei file storici. */

/** Deposito inventorio oltre MAX_OBJ_SAVE: list / buy / value. */
MOBSPECIAL_FUNC(OverflowClaim);

} // namespace Alarmud
#endif // __SPEC_PC_SERVICES_HPP
