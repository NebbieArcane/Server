/* src/skills.cpp */
#ifndef __SKILLS__HPP
#define __SKILLS__HPP
#include "config.hpp"
enum e_find_result {
	FOUND_WATER= 13,  /* obj found when water found */
	FOUND_FOOD = 21,  /* obj that is found if they made it! */
	FOUND_FOOD0=1300,
	FOUND_FOOD1=1301,
	FOUND_FOOD2=1302,
	FOUND_FOOD3=1303,
	FOUND_FOOD4=1304,
	FOUND_FOOD5=1305,
	FOUND_FOOD6=1306,
	FOUND_FOOD7=1307,
	FOUND_FOOD8=1308,
	FOUND_FOOD9=1309,
	FOUND_FOOD10=1310,
	FOUND_FOOD11=1311,
	FOUND_FOOD12=1312,
	FOUND_FOOD13=1313,
	FOUND_FOOD14=1314,
	FOUND_FOOD15=1315,
	FOUND_FOOD16=1316,
	FOUND_FOOD17=1317,
	FOUND_FOOD18=1318,
	FOUND_FOOD19=1319,
	FOUND_FOOD20=1320,
	FOUND_FOOD21=1321,
	FOUND_FOOD22=1322,
	FOUND_FOOD23=1323,
	FOUND_FOOD24=1324,
	FOUND_FOOD25=1325,
	FOUND_FOOD26=1326,
	FOUND_FOOD27=1327,
	FOUND_FOOD28=1328,
	FOUND_FOOD29=1329,
	FOUND_FOOD30=1330,
	FOUND_FOOD31=1331,
	FOUND_FOOD32=1332,
	FOUND_FOOD33=1333,
	FOUND_FOOD34=1334,
	FOUND_FOOD35=1335,
	FOUND_FOOD36=1336,
	FOUND_FOOD37=1337,
	FOUND_FOOD38=1338,
	FOUND_FOOD39=1339,
	FOUND_FOOD40=1340,
	FOUND_FOOD41=1341,
	FOUND_FOOD42=1342,
	FOUND_FOOD43=1343,
	FOUND_FOOD44=1344,
	FOUND_FOOD45=1345,
	FOUND_FOOD46=1346,
	FOUND_FOOD47=1347,
	FOUND_FOOD48=1348,
	FOUND_FOOD49=1349,
	FOUND_FOOD50=1350,
	FOUND_FOOD51=1351,
	FOUND_FOOD52=1352,
	FOUND_FOOD53=1353,
	FOUND_FOOD54=1354,
	FOUND_FOOD55=1355,
	FOUND_FOOD56=1356,
	FOUND_FOOD57=1357,
	FOUND_FOOD58=1358,
	FOUND_FOOD59=1359,
	FOUND_FOOD60=1360,
	FOUND_FOOD61=1361,
	FOUND_FOOD62=1362,
	FOUND_FOOD63=1363,
	FOUND_FOOD64=1364,
	FOUND_FOOD65=1365,
	FOUND_FOOD66=1366,
	FOUND_FOOD67=1367,
	FOUND_FOOD68=1368,
	FOUND_FOOD69=1369
};
enum e_tan_result {
	TAN_SHIELD=67,
	TAN_JACKET=68,
	TAN_BOOTS=69,
	TAN_GLOVES=70,
	TAN_LEGGINGS=71,
	TAN_SLEEVES=72,
	TAN_HELMET=73,
	TAN_BAG=14,
	TAN_ARMOR=9602,
};
void ChangeAlignmentDoorBash( struct char_data* pChar, int nAmount ) ;
void ForgeGraphic( struct char_data* ch, int urka) ;
void ForgeString(struct char_data* ch, char* arg, int type) ;
int IS_FOLLOWING(struct char_data* tch, struct char_data* person) ;
int SpyCheck( struct char_data* ch ) ;
int choose_exit_global(int in_room, int tgt_room, int depth) ;
int choose_exit_in_zone(int in_room, int tgt_room, int depth) ;
int dir_track( struct char_data* ch, struct char_data* vict) ;
void do_adrenalize( struct char_data* ch, char* argument, int cmd) ;
void do_aura_sight( struct char_data* ch, char* argument, int cmd) ;
void do_bellow( struct char_data* ch, char* arg, int cmd) ;
void do_blast( struct char_data* ch, char* argument, int cmd) ;
void do_blessing(struct char_data* ch, char* argument, int cmd) ;
void do_brew( struct char_data* ch, char* argument, int cmd) ;
void do_canibalize( struct char_data* ch, char* argument, int cmd) ;
void do_carve( struct char_data* ch, char* argument, int cmd) ;
void do_climb( struct char_data* ch, char* arg, int cmd) ;
void do_daimoku( struct char_data* ch, char* arg, int cmd) ;
void do_disarm(struct char_data* ch, char* argument, int cmd) ;
void do_disguise(struct char_data* ch, char* argument, int cmd) ;
void do_doorbash( struct char_data* ch, char* arg, int cmd) ;
void do_doorway( struct char_data* ch, char* argument, int cmd) ;
void do_esp( struct char_data* ch, char* argument, int cmd) ;
void do_feign_death( struct char_data* ch, char* arg, int cmd) ;
void do_find( struct char_data* ch, char* arg, int cmd) ;
void do_find_food( struct char_data* ch, char* arg, int cmd) ;
void do_find_food_old( struct char_data* ch, char* arg, int cmd) ;
void do_find_traps( struct char_data* ch, char* arg, int cmd) ;
void do_find_water( struct char_data* ch, char* arg, int cmd) ;
void do_finger(struct char_data* ch, char* argument, int cmd) ;
void do_first_aid( struct char_data* ch, char* arg, int cmd ) ;
void do_flame_shroud( struct char_data* ch, char* argument, int cmd) ;
void do_forge( struct char_data* ch, char* arg, int cmd) ;
void do_great_sight( struct char_data* ch, char* argument, int cmd) ;
void do_heroic_rescue( struct char_data* ch, char* arguement, int command ) ;
void do_holy_warcry (struct char_data* ch, char* argument, int cmd) ;
void do_hypnosis( struct char_data* ch, char* argument, int cmd) ;
void do_immolation(struct char_data* ch, char* argument, int cmd) ;
void do_invisibililty( struct char_data* ch, char* argument, int cmd) ;
void do_lay_on_hands (struct char_data* ch, char* argument, int cmd) ;
void do_mantra( struct char_data* ch, char* arg, int cmd) ;
void do_meditate( struct char_data* ch, char* argument, int cmd) ;
void do_mindsummon( struct char_data* ch, char* argument, int cmd) ;
void do_miner( struct char_data* ch ) ;
void do_psi_portal( struct char_data* ch, char* argument, int cmd) ;
void do_psi_shield( struct char_data* ch, char* argument, int cmd) ;
void do_scry( struct char_data* ch, char* argument, int cmd) ;
void do_sending( struct char_data* ch, char* argument, int cmd) ;
void do_spy( struct char_data* ch, char* arg, int cmd) ;
void do_swim( struct char_data* ch, char* arg, int cmd) ;
void do_tan( struct char_data* ch, char* arg, int cmd) ;
void do_tan_old( struct char_data* ch, char* arg, int cmd) ;
void do_track(struct char_data* ch, char* argument, int cmd) ;
void donothing( void* pDummy ) ;
int find_path( int in_room, int (*predicate)( int, void* ), void* c_data, int depth, int in_zone);
void go_direction(struct char_data* ch, int dir) ;
int in_miniera( struct char_data* ch ) ;
int named_mobile_in_room(int room, struct hunting_data* c_data) ;
int remove_trap( struct char_data* ch, struct obj_data* trap) ;
void slam_into_wall( struct char_data* ch, struct room_direction_data* exitp) ;
void slip_in_climb(struct char_data* ch, int dir, int room) ;
int track( struct char_data* ch, struct char_data* vict) ;
#endif // __SKILLS_HPP
