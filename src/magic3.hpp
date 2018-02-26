/* src/magic3.cpp */
#ifndef __MAGIC3_HPP
#define __MAGIC3_HPP
#include "config.hpp"
#include "typedefs.hpp"
void spell_tree_travel(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_transport_via_plant(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_speak_with_plants(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_changestaff(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_pword_kill(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_pword_blind(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_chain_lightn(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_scare(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_haste(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_slow(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_familiar(byte level, struct char_data* ch, struct char_data** victim, struct obj_data* obj);
void spell_aid(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_holyword(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_golem(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_feeblemind(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_shillelagh(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_goodberry(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_elemental_blade(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_animal_growth(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_insect_growth(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_creeping_death(byte level, struct char_data* ch, struct char_data* victim, int dir);
void spell_commune(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_animal_summon(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_elemental_summoning(byte level, struct char_data* ch, struct char_data* victim, int spell);
void spell_reincarnate(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_charm_veggie(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_veggie_growth(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_tree(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_animate_rock(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_travelling(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_animal_friendship(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_invis_to_animals(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_slow_poison(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_snare(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_entangle(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_barkskin(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_gust_of_wind(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_silence(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_warp_weapon(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_heat_stuff(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_dust_devil(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_sunray(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_know_monster(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_find_traps(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_firestorm(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_teleport_wo_error(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_portal(byte level, struct char_data* ch, struct char_data* tmp_ch, struct obj_data* obj);
void spell_mount(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void spell_dragon_ride(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
#endif
