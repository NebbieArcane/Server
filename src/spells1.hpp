/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPELLS1_HPP
#define __SPELLS1_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
void cast_acid_blast		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_burning_hands		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_call_lightning	( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_cause_critic		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_cause_light		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_cause_serious		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_chill_touch		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_colour_spray		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_cone_of_cold		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_disintegrate		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_earthquake		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_energy_drain		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_fireball			( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_flamestrike		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_geyser			( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_green_slime		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_harm				( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_ice_storm			( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_incendiary_cloud	( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_lightning_bolt	( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_magic_missile		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_meteor_swarm		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_prismatic_spray	( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_shocking_grasp	( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
void cast_wizard_eye		( byte level, struct char_data* ch, char* arg, int type, struct char_data* victim, struct obj_data* tar_obj );
} // namespace Alarmud
#endif // __SPELLS1_HPP

