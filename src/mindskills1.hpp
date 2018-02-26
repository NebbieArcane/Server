#ifndef __MINDSKILLS1_HPP
#define __MINDSKILLS1_HPP
#include "config.hpp"
#include "typedefs.hpp"
void mind_burn(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_cell_adjustment(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_chameleon( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_clairvoyance(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_danger_sense( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_disintegrate( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_domination(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_intensify(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_levitation(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_mind_over_body(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_mind_wipe( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_mindblank( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_probability_travel(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_psi_strength( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_psychic_crush( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_psychic_impersonation( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_telekinesis( byte level, struct char_data* ch,struct char_data* victim, int dir_num );
void mind_teleport(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_tower_iron_will(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
void mind_ultra_blast( byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj);
#endif // __MINDSKILLS1_HPP
