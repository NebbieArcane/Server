/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_PROCS4_HPP
#define __SPEC_PROCS4_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
// procedure speciali
MOBSPECIAL_FUNC(quest_item_shop);

// funzioni
void IdentifyObj(struct char_data* ch, struct char_data* keeper, int quest, int numero);
void ListaOggetti(struct char_data* ch, int quest);
void SellObj(struct char_data* ch, struct char_data* keeper, int quest, int numero);
std::string get_last_word(const std::string& s);
} // namespace Alarmud
#endif // __SPEC_PROCS4_HPP
