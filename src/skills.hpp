/* src/skills.cpp */
#ifndef __SKILLS__HPP
#define __SKILLS__HPP
void do_disarm(struct char_data *ch, char *argument, int cmd);
void do_finger(struct char_data *ch, char *argument, int cmd);
int named_mobile_in_room(int room, struct hunting_data *c_data);
void do_track(struct char_data *ch, char *argument, int cmd);
int track(struct char_data *ch, struct char_data *vict);
int dir_track(struct char_data *ch, struct char_data *vict);
void donothing(void *pDummy);
int find_path(int in_room, int (*predicate)(int, void *), void *c_data, int depth, int in_zone);
int choose_exit_global(int in_room, int tgt_room, int depth);
int choose_exit_in_zone(int in_room, int tgt_room, int depth);
void go_direction(struct char_data *ch, int dir);
void slam_into_wall(struct char_data *ch, struct room_direction_data *exitp);
void ChangeAlignmentDoorBash(struct char_data *pChar, int nAmount);
void do_doorbash(struct char_data *ch, char *arg, int cmd);
void do_swim(struct char_data *ch, char *arg, int cmd);
void do_mantra(struct char_data *ch, char *arg, int cmd);
void do_daimoku(struct char_data *ch, char *arg, int cmd);
int SpyCheck(struct char_data *ch);
void do_spy(struct char_data *ch, char *arg, int cmd);
int remove_trap(struct char_data *ch, struct obj_data *trap);
void do_feign_death(struct char_data *ch, char *arg, int cmd);
void do_first_aid(struct char_data *ch, char *arg, int cmd);
void do_disguise(struct char_data *ch, char *argument, int cmd);
void do_climb(struct char_data *ch, char *arg, int cmd);
void slip_in_climb(struct char_data *ch, int dir, int room);
void do_tan(struct char_data *ch, char *arg, int cmd);
void do_tan_old(struct char_data *ch, char *arg, int cmd);
void do_find_food(struct char_data *ch, char *arg, int cmd);
void do_find_food_old(struct char_data *ch, char *arg, int cmd);
void do_find_water(struct char_data *ch, char *arg, int cmd);
void do_find_traps(struct char_data *ch, char *arg, int cmd);
void do_find(struct char_data *ch, char *arg, int cmd);
void do_bellow(struct char_data *ch, char *arg, int cmd);
void do_carve(struct char_data *ch, char *argument, int cmd);
void do_doorway(struct char_data *ch, char *argument, int cmd);
void do_psi_portal(struct char_data *ch, char *argument, int cmd);
void do_mindsummon(struct char_data *ch, char *argument, int cmd);
void do_immolation(struct char_data *ch, char *argument, int cmd);
void do_canibalize(struct char_data *ch, char *argument, int cmd);
void do_flame_shroud(struct char_data *ch, char *argument, int cmd);
void do_aura_sight(struct char_data *ch, char *argument, int cmd);
void do_great_sight(struct char_data *ch, char *argument, int cmd);
void do_blast(struct char_data *ch, char *argument, int cmd);
void do_hypnosis(struct char_data *ch, char *argument, int cmd);
void do_scry(struct char_data *ch, char *argument, int cmd);
void do_invisibililty(struct char_data *ch, char *argument, int cmd);
void do_adrenalize(struct char_data *ch, char *argument, int cmd);
void do_meditate(struct char_data *ch, char *argument, int cmd);
int IS_FOLLOWING(struct char_data *tch, struct char_data *person);
void do_heroic_rescue(struct char_data *ch, char *arguement, int command);
void do_blessing(struct char_data *ch, char *argument, int cmd);
void do_lay_on_hands(struct char_data *ch, char *argument, int cmd);
void do_holy_warcry(struct char_data *ch, char *argument, int cmd);
void do_psi_shield(struct char_data *ch, char *argument, int cmd);
void do_esp(struct char_data *ch, char *argument, int cmd);
void do_sending(struct char_data *ch, char *argument, int cmd);
void do_brew(struct char_data *ch, char *argument, int cmd);
int in_miniera(struct char_data *ch);
void do_miner(struct char_data *ch);
void ForgeString(struct char_data *ch, char *arg, int type);
void ForgeGraphic(struct char_data *ch, int urka);
void do_forge(struct char_data *ch, char *arg, int cmd);
#endif
