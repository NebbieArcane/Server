/*$Id: protos.h,v 1.3 2002/03/11 11:33:34 Thunder Exp $
*/
#ifndef __PROTOS
#define __PROTOS 1
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "hash.hpp"
#include "mail.hpp"
#include "maximums.hpp"
#include "parser.hpp"
#include "poly.hpp"
#include "race.hpp"
#include "script.hpp"
#include "spells.hpp"
#include "structs.hpp"
#include "trap.hpp"
#include "utils.hpp"
#include "vt100c.hpp"
#include "wizlist.hpp"
#ifdef KLUDGE_STRING
char* strdup(char*);
#else
#endif
/* From Fight.c */
bool CheckMirror(struct char_data* victim);
bool CheckEquilibrium(struct char_data* ch);

/* From Heap.c */

void SmartStrCpy(char* s1, const char* s2);
void StringHeap(char* string, struct StrHeap* Heap);
struct StrHeap* InitHeap();
void DisplayStringHeap(struct StrHeap* Heap, struct char_data* ch, int type, int d);

/* From regen.c */
void    alter_hit(struct char_data* ch, int amount);
void    alter_mana(struct char_data* ch, int amount);
void    alter_move(struct char_data* ch, int amount);


/* From Opinion.c */
void FreeHates( struct char_data* ch);
void FreeFears( struct char_data* ch);
int RemHated( struct char_data* ch, struct char_data* pud);
int AddHated( struct char_data* ch, struct char_data* pud);
int AddHatred( struct char_data* ch, int parm_type, int parm);
void RemHatred( struct char_data* ch, unsigned short bitv);
int Hates( struct char_data* ch, struct char_data* v);
int Fears( struct char_data* ch, struct char_data* v);
int RemFeared( struct char_data* ch, struct char_data* pud);
int AddFeared( struct char_data* ch, struct char_data* pud);
int AddFears( struct char_data* ch, int parm_type, int parm);
struct char_data* FindAHatee( struct char_data* ch);
struct char_data* FindAFearee( struct char_data* ch);
void ZeroHatred(struct char_data* ch, struct char_data* v);
void ZeroFeared(struct char_data* ch, struct char_data* v);
void DeleteHatreds( struct char_data* ch );
void DeleteFears(struct char_data* ch);


/* From Sound.c */

#include "sound.hpp"

/* From Trap.c */

void do_settrap( struct char_data* ch, char* arg, int cmd);
int CheckForMoveTrap(struct char_data* ch, int dir);
int CheckForInsideTrap(struct char_data* ch, struct obj_data* i);
int CheckForAnyTrap(struct char_data* ch, struct obj_data* i);
int CheckForGetTrap(struct char_data* ch, struct obj_data* i);
int TriggerTrap( struct char_data* ch, struct obj_data* i);
void FindTrapDamage( struct char_data* v, struct obj_data* i);
void TrapDamage(struct char_data* v, int damtype, int amnt, struct obj_data* t);
void TrapDam(struct char_data* v, int damtype, int amnt, struct obj_data* t);
void TrapTeleport(struct char_data* v);
void TrapSleep(struct char_data* v);
void InformMess( struct char_data* v);


#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj1.hpp"
#include "act.obj2.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
/* From board.c */
/* CHECKED */
void board_write_msg(struct char_data* ch, char* arg, int bnum);
int board_display_msg(struct char_data* ch, char* arg, int bnum);
int board_remove_msg(struct char_data* ch, char* arg, int bnum);
void board_save_board();
void board_load_board();
int board_show_board(struct char_data* ch, char* arg, int bnum);
int board(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
char* fix_returns(char* text_string);
int board_check_locks (int bnum, struct char_data* ch);


/* From comm.c */
#include "comm.hpp"
/* From constants.c */
#include "constants.hpp"

/* From db.c */
#include "db.hpp"
/* From handler.c */



/* From hash.c */

void init_hash_table(struct hash_header* ht, int rec_size, int table_size);
void init_world(struct room_data* room_db[]);
void destroy_hash_table(struct hash_header* ht, void (*gman)( void* ));
void _hash_enter(struct hash_header* ht, long key, void* data);
struct room_data* room_find( struct room_data* room_db[], long key);
void* hash_find(struct hash_header* ht, long key);
int room_enter(struct room_data* rb[], long key, struct room_data* rm);
int hash_enter(struct hash_header* ht, long key, void* data);
struct room_data* room_find_or_create(struct room_data* rb[], long key);
void* hash_find_or_create(struct hash_header* ht, long key);
int room_remove(struct room_data* rb[], long key);
void* hash_remove(struct hash_header* ht, long key);
void room_iterate( struct room_data* rb[],
				   void (*func)( int, struct room_data*, void* ),
				   void* cdata );

void hash_iterate( struct hash_header* ht, void (*func)( int, void*, void* ),
				   void* cdata );


/* From interpreter.c */
#include "interpreter.hpp"

/*
int command_search( char *arg, int len, struct command_info *cmd_info,
  char **list);
*/



#include "maximums.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magic3.hpp"

/* From magicutils.c */

void SwitchStuff( struct char_data* giver, struct char_data* taker);
void FailCharm( struct char_data* victim, struct char_data* ch );
void FailSnare( struct char_data* victim, struct char_data* ch );
void FailSleep( struct char_data* victim, struct char_data* ch );
void FailPara( struct char_data* victim, struct char_data* ch );
void FailCalm( struct char_data* victim, struct char_data* ch );
void FailPoison( struct char_data* victim, struct char_data* ch );


/* From mobact.c */
int UseViolentHeldItem(struct char_data* ch) ;
void mobile_guardian(struct char_data* ch);
void mobile_wander(struct char_data* ch);
void MobHunt(struct char_data* ch);
void MobScavenge(struct char_data* ch);
void check_mobile_activity(unsigned long pulse);
void mobile_activity(struct char_data* ch);
int SameRace( struct char_data* ch1, struct char_data* ch2);
int AssistFriend( struct char_data* ch);
void FindABetterWeapon(struct char_data* mob);
int GetDamage(struct obj_data* w, struct char_data* ch);
int GetDamBonus(struct obj_data* w);
int GetHandDamage(struct char_data* ch);
int MobFriend( struct char_data* ch, struct char_data* f);
void PulseMobiles(int type);
void DoScript(struct char_data* ch);
int CommandSearch(char* arg);
void CommandSetup();
void noop(char* arg, struct char_data* ch);
void end2(char* arg, struct char_data* ch);
void sgoto(char* arg, struct char_data* ch);
void do_act(char* arg, struct char_data* ch);
void do_jmp(char* arg, struct char_data* ch);
void do_jsr(char* arg, struct char_data* ch);
void do_rts(char* arg, struct char_data* ch);
void MobHit(struct char_data*, struct char_data*, int);


/* From modify.c */
#include "modify.hpp"



/* From multiclass.c */

int GetClassLevel(struct char_data* ch, int iClass);
int CountBits(int iClass);
int OnlyClass( struct char_data* ch, int iClass);
int HasClass(struct char_data* ch, int iClass);
int HowManyClasses(struct char_data* ch);
int BestFightingClass(struct char_data* ch);
int BestThiefClass(struct char_data* ch);
int BestMagicClass(struct char_data* ch);
int GetSecMaxLev(struct char_data* ch);
int GetALevel(struct char_data* ch, int which);
int GetThirdMaxLev(struct char_data* ch);
int GetMaxLevel(struct char_data* ch);
int GetMaxClass(struct char_data* ch);
int GetHighClass(struct char_data* ch);
int GetTotLevel(struct char_data* ch);
void StartLevels(struct char_data* ch);
int BestClassIND(struct char_data* ch);
int BestClassBIT(struct char_data* ch);


/* From reception.c */

void add_obj_cost(struct char_data* ch, struct char_data* re,
				  struct obj_data* obj, struct obj_cost* cost);

bool recep_offer(struct char_data* ch,        struct char_data* receptionist,
				 struct obj_cost* cost, int forcerent);

void update_file(struct char_data* ch, struct obj_file_u* st);
void obj_store_to_char(struct char_data* ch, struct obj_file_u* st);
void load_char_objs(struct char_data* ch);
void put_obj_in_store(struct obj_data* obj, struct obj_file_u* st);
int contained_weight(struct obj_data* container);
void obj_to_store(struct obj_data* obj, struct obj_file_u* st,
				  struct char_data* ch, int bDelete);
void save_obj(struct char_data* ch, struct obj_cost* cost, int bDelete);
void update_obj_file();
void CountLimitedItems(struct obj_file_u* st);
void PrintLimitedItems();
int receptionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int creceptionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
void zero_rent( struct char_data* ch);
void ZeroRent( char* n);
int ReadObjs( FILE* fl, struct obj_file_u* st);
void WriteObjs( FILE* fl, struct obj_file_u* st);
void load_char_extra(struct char_data* ch);
void write_char_extra( struct char_data* ch);
void obj_store_to_room(int room, struct obj_file_u* st);
void load_room_objs(int room);
void save_room(int room);


/* From security.c */

int SecCheck(char* arg, char* site);


/* From shop.c */

int is_ok(struct char_data* keeper, struct char_data* ch, int shop_nr);
int trade_with(struct obj_data* item, int shop_nr);
int shop_producing(struct obj_data* item, int shop_nr);
void shopping_buy( char* arg, struct char_data* ch,
				   struct char_data* keeper, int shop_nr);
void shopping_sell( char* arg, struct char_data* ch,
					struct char_data* keeper,int shop_nr);
void shopping_value( char* arg, struct char_data* ch,
					 struct char_data* keeper, int shop_nr);
void shopping_list( char* arg, struct char_data* ch,
					struct char_data* keeper, int shop_nr);
int shopping_kill( char* arg, struct char_data* ch,
				   struct char_data* keeper, int shop_nr);
int shop_keeper(struct char_data* ch, int cmd, char* arg, char* mob, int type);
void boot_the_shops();
void assign_the_shopkeepers();


/* From signals.c
int checkpointing();
int shutdown_request();
int hupsig();
int logsig();
*/
void signal_setup();

#include "skills.hpp"

/* From spec_assign.c */
int is_murdervict(struct char_data* ch);
void assign_mobiles();


/* From spec_procs.c */

int is_target_room_p(int room, void* tgt_room);
int named_object_on_ground(int room, void* c_data);
char* how_good(int percent);
int GainLevel(struct char_data* ch, int iClass);
struct char_data* FindMobInRoomWithFunction( int room,
		int (*func)( struct char_data*,
					 int, char*,
					 struct char_data*,
					 int ) );
int MageGuildMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int BiosKaiThanatos(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int ClericGuildMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int ThiefGuildMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int WarriorGuildMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int dump(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int mayor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int andy_wilcox(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int eric_johnson(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
void exec_social(struct char_data* npc, char* cmd, int next_line,
				 int* cur_line, void** thing);
void npc_steal(struct char_data* ch,struct char_data* victim);
int snake(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int snake_plus(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int MidgaardCityguard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int PaladinGuildGuard( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int GameGuard( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int GreyParamedic(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int types);
int AmberParamedic(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int blink( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int MidgaardCitizen(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int ghoul(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int CarrionCrawler(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int WizardGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int vampire(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int wraith(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int shadow(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int geyser(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int green_slime(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int DracoLich(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Drow(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Leader(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int thief(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int guild_guard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Inquisitor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int puff(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int regenerator( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int replicant( struct char_data* ch, int cmd, char* arg, struct char_data* mob1, int type);
int Tytan(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int AbbarachDragon(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int fido(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int janitor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int tormentor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int RustMonster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int temple_labrynth_liar(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int temple_labrynth_sentry(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Whirlwind (struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int NudgeNudge(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int AGGRESSIVE(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int citizen(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Ringwraith( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int WarrenGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int zm_tired(struct char_data* zmaster);
int zm_stunned_followers(struct char_data* zmaster);
void zm_init_combat(struct char_data* zmaster, struct char_data* target);
int zm_kill_fidos(struct char_data* zmaster);
int zm_kill_aggressor(struct char_data* zmaster);
int zombie_master(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int pet_shops(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int Fountain(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int bank (struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int pray_for_items(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int chalice(struct char_data* ch, int cmd, char* arg);
int kings_hall(struct char_data* ch, int cmd, char* arg);
int Donation(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int House(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int SaveRoomContens( struct char_data* pChar, int nCmd, char* pchArg,
					 struct room_data* pRoom, int nEvent );
int sisyphus(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int jabberwocky(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int flame(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int banana(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int paramedics(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int jugglernaut(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int delivery_elf(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int delivery_beast(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Keftab(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int StormGiant(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Manticore(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Kraken(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int fighter(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int NewThalosMayor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int SultanGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int NewThalosCitizen(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int NewThalosGuildGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int magic_user2(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int MordGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int MordGuildGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int CaravanGuildGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int StatTeller(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
void ThrowChar(struct char_data* ch, struct char_data* v, int dir);
int ThrowerMob(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Tyrannosaurus_swallower(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int soap(struct char_data* ch, int cmd, char* arg, struct obj_data* tobj, int type);
int enter_obj(struct char_data* ch, int cmd, char* arg, struct obj_data* tobj, int type);
int zone_obj(struct char_data* ch, int cmd, char* arg, struct obj_data* tobj, int type);
int nodrop(struct char_data* ch, int cmd, char* arg, struct obj_data* tobj, int type);
int Rakda(struct char_data* ch, int cmd, char* arg, struct obj_data* tobj, int type);

/* From spec_procs2.c */
int EvilBlade(struct char_data* ch, int cmd, char* arg,struct obj_data* tobj, int type);
int PrisonGuard( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int acid_monster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int FireBreather(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int FrostBreather(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int AcidBreather(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int GasBreather(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int LightningBreather(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int magic_user_imp(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int RangerGuildmaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int PsiGuildmaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int PaladinGuildmaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);

int death_knight(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int DogCatcher(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int mad_gertruda(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int mad_cyrus(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int raven_iron_golem(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);

int ghost(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int druid_protector(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Magic_Fountain(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int DruidAttackSpells(struct char_data* ch, struct char_data* vict, int level);
int Summoner(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int monk(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
void invert(char* arg1, char* arg2);
int jive_box(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
int magic_user(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int cleric(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int ninja_master(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int barbarian_guildmaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int mage_specialist_guildmaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int RepairGuy( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Samah( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int BitterBlade(struct char_data* ch, int cmd, char* arg,struct obj_data* tobj, int type);
int MakeQuest(struct char_data* ch, struct char_data* gm, int iClass, char* arg, int cmd) ;
int AbyssGateKeeper( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int creeping_death( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
void  Submit(struct char_data* ch, struct char_data* t);
void  SayHello(struct char_data* ch, struct char_data* t);
void GreetPeople(struct char_data* ch);
int GenericCityguardHateUndead(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int GenericCityguard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int PrydainGuard(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int sailor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int loremaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int hunter(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int miner_teacher(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int forge_teacher(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int determine_teacher(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int equilibrium_teacher(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int monk_master(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int DruidGuildMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int Devil(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Demon(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int DemonTeacher(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
void DruidHeal(struct char_data* ch, int level);
void DruidTree(struct char_data* ch);
void DruidMob(struct char_data* ch);
int DruidChallenger(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int druid( struct char_data* ch, int cmd, char* arg, struct char_data* mob,
		   int type );
int MonkChallenger(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int druid_challenge_prep_room(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int druid_challenge_room(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int monk_challenge_room(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int monk_challenge_prep_room(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int portal(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
int scraps(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
int attack_rats(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int DragonHunterLeader(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int HuntingMercenary(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int SlotMachine(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
int astral_portal(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int camino(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int DwarvenMiners(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int real_rabbit(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int real_fox(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int antioch_grenade(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);


/* From spec_procs3.c */
int archer_instructor(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int archer(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Beholder(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int BerserkerItem(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
int AntiSunItem(struct char_data* ch, int cmd, char* arg, struct obj_data* obj, int type);
int ChurchBell(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int Slavalis(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int TreeThrowerMob(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int PostMaster(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
struct room_data* forward_square(struct room_data* room);
struct room_data* back_square(struct room_data* room);
struct room_data* left_square(struct room_data* room);
struct room_data* right_square(struct room_data* room);
struct room_data* forward_left_square(struct room_data* room);
struct room_data* forward_right_square(struct room_data* room);
struct room_data* back_right_square(struct room_data* room);
struct room_data* back_left_square(struct room_data* room);
struct char_data* square_contains_enemy(struct room_data* square);
int square_contains_friend(struct room_data* square);
int square_empty(struct room_data* square);
int chess_game(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int AcidBlob(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int avatar_celestian(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int strahd_vampire(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int strahd_zombie(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int banshee(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int banshee_lorelai(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int baby_bear(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int timnus(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int winger(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);

int Barbarian(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Paladin(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Psionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Ranger(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);

int goblin_sentry(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int lich_church(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int medusa(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int Cockatrice(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);
int GoodBlade(struct char_data* ch, int cmd, char* arg,struct obj_data* tobj, int type);
int NeutralBlade(struct char_data* ch, int cmd, char* arg,struct obj_data* tobj, int type);
int PrimoAlbero( struct char_data* ch, int cmd, char* arg,
				 struct char_data* mob, int type );
int LegionariV( struct char_data* ch, int cmd, char* arg,
				struct char_data* mob, int type );
int BlockWay(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
int MobBlockWay(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type);

/* From spell_parser.c */
void spello( int nr, byte beat, byte pos, byte mlev, byte clev, byte dlev,
			 byte slev, byte plev, byte rlev, byte ilev,
			 ubyte mana, sh_int tar, void (*func)( byte, struct char_data*,
					 char*, int,
					 struct char_data*,
					 struct obj_data* ),
			 sh_int sf );

/*
void clone_char(struct char_data *ch);
void clone_obj(struct obj_data *obj);
*/
bool circle_follow(struct char_data* ch, struct char_data* victim);
bool saves_spell(struct char_data* ch, sh_int save_type);
bool ImpSaveSpell(struct char_data* ch, sh_int save_type, int mod);
void AddCommand( char* name, void (*func)( struct char_data*, char*, int ),
				 int number, int min_pos, int min_lev );


/* From spells1.c */

void cast_burning_hands( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void cast_call_lightning(byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void cast_chill_touch( byte level, struct char_data* ch, char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj );
void cast_shocking_grasp( byte level, struct char_data* ch, char* arg, int type,
						  struct char_data* victim, struct obj_data* tar_obj );
void cast_colour_spray( byte level, struct char_data* ch, char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj );
void cast_earthquake( byte level, struct char_data* ch, char* arg, int type,
					  struct char_data* victim, struct obj_data* tar_obj );
void cast_energy_drain( byte level, struct char_data* ch, char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj );
void cast_fireball( byte level, struct char_data* ch, char* arg, int type,
					struct char_data* victim, struct obj_data* tar_obj );
void cast_harm( byte level, struct char_data* ch, char* arg, int type,
				struct char_data* victim, struct obj_data* tar_obj );
void cast_lightning_bolt( byte level, struct char_data* ch, char* arg, int type,
						  struct char_data* victim, struct obj_data* tar_obj );
void cast_acid_blast( byte level, struct char_data* ch, char* arg, int type,
					  struct char_data* victim, struct obj_data* tar_obj );
void cast_cone_of_cold( byte level, struct char_data* ch, char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj );
void cast_ice_storm( byte level, struct char_data* ch, char* arg, int type,
					 struct char_data* victim, struct obj_data* tar_obj );
void cast_meteor_swarm( byte level, struct char_data* ch, char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj );
void cast_flamestrike( byte level, struct char_data* ch, char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj );
void cast_magic_missile( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void cast_cause_light( byte level, struct char_data* ch, char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj );
void cast_cause_serious( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void cast_cause_critic(byte level, struct char_data* ch, char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj );
void cast_geyser( byte level, struct char_data* ch, char* arg, int type,
				  struct char_data* victim, struct obj_data* tar_obj );
void cast_green_slime( byte level, struct char_data* ch, char* arg, int type,
					   struct char_data* victim, struct obj_data* tar_obj );


/* From spells2.c */
#include "spells2.hpp"

/* from mind_use1.c */

void mind_use_burn( byte level, struct char_data* ch, char* arg, int type,
					struct char_data* victim, struct obj_data* tar_obj );
void mind_use_clairvoyance( byte level, struct char_data* ch, char* arg,
							int type,  struct char_data* victim,
							struct obj_data* tar_obj );
void mind_use_danger_sense( byte level, struct char_data* ch, char* arg,
							int type, struct char_data* victim,
							struct obj_data* tar_obj );
void mind_use_disintegrate( byte level, struct char_data* ch, char* arg,
							int type, struct char_data* victim,
							struct obj_data* tar_obj );
void mind_use_telekinesis( byte level, struct char_data* ch, char* arg,
						   int type, struct char_data* victim,
						   struct obj_data* tar_obj );
void mind_use_levitation( byte level, struct char_data* ch, char* arg,
						  int type,  struct char_data* victim,
						  struct obj_data* tar_obj );
void mind_use_cell_adjustment( byte level, struct char_data* ch, char* arg,
							   int type, struct char_data* victim,
							   struct obj_data* tar_obj );
void mind_use_chameleon( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void mind_use_psi_strength( byte level, struct char_data* ch, char* arg,
							int type, struct char_data* victim,
							struct obj_data* tar_obj );
void mind_use_mind_over_body( byte level, struct char_data* ch, char* arg,
							  int type, struct char_data* victim,
							  struct obj_data* tar_obj );
void mind_use_probability_travel( byte level, struct char_data* ch, char* arg,
								  int type, struct char_data* victim,
								  struct obj_data* tar_obj );
void mind_use_teleport( byte level, struct char_data* ch, char* arg, int type,
						struct char_data* victim, struct obj_data* tar_obj );
void mind_use_domination( byte level, struct char_data* ch, char* arg,
						  int type,  struct char_data* victim,
						  struct obj_data* tar_obj );
void mind_use_mind_wipe( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void mind_use_psychic_crush( byte level, struct char_data* ch, char* arg,
							 int type, struct char_data* victim,
							 struct obj_data* tar_obj );
void mind_use_tower_iron_will( byte level, struct char_data* ch, char* arg,
							   int type, struct char_data* victim,
							   struct obj_data* tar_obj );
void mind_use_mindblank( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void mind_use_psychic_impersonation( byte level, struct char_data* ch,
									 char* arg, int type,
									 struct char_data* victim,
									 struct obj_data* tar_obj );
void mind_use_ultra_blast( byte level, struct char_data* ch, char* arg,
						   int type, struct char_data* victim,
						   struct obj_data* tar_obj );
void mind_use_intensify( byte level, struct char_data* ch, char* arg, int type,
						 struct char_data* victim, struct obj_data* tar_obj );
void mind_teleport( byte level, struct char_data* ch, struct char_data* victim,
					struct obj_data* obj );
void mind_probability_travel( byte level, struct char_data* ch,
							  struct char_data* victim, struct obj_data* obj );
void mind_danger_sense( byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj );
void mind_clairvoyance( byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj );
void mind_disintegrate( byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj );
void mind_telekinesis( byte level, struct char_data* ch,
					   struct char_data* victim, int dir_num );
void mind_levitation( byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj );
void mind_mind_over_body( byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj );
void mind_domination( byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj );
void mind_mind_wipe( byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj );
void mind_psychic_impersonation( byte level, struct char_data* ch,
								 struct char_data* victim,
								 struct obj_data* obj );
void mind_intensify( byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj );
void mind_cell_adjustment( byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj );
void mind_mindblank( byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj );
void mind_tower_iron_will( byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj );
void mind_psi_strength( byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj );
void mind_chameleon( byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj );
void mind_psychic_crush( byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj );
void mind_ultra_blast( byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj );
void mind_burn( byte level, struct char_data* ch, struct char_data* victim,
				struct obj_data* obj );


#include "utility.hpp"
bool isNullChar (struct char_data* ch); // SALVO nuova funzione

void Zwrite (FILE* fp, char cmd, int tf, int arg1, int arg2, int arg3,
			 int arg4, char* desc);
void RecZwriteObj(FILE* fp, struct obj_data* o);
int SaveZoneFile(FILE* fl, int start_room, int end_room);
int LoadZoneFile(FILE* fl, int zon);
FILE* MakeZoneFile( struct char_data* c, int zone);
FILE* OpenZoneFile(struct char_data* c, int zone);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
void mud_time_passed2(time_t t2, time_t t1, struct time_info_data* t);
bool CheckPrac (int classe, int id, int liv); // SALVO nuova funzione

/* From weather.c */

void weather_and_time(int mode);
void another_hour(int mode);
void ChangeSeason(int month);
void weather_change();
void ChangeWeather( int change);
void GetMonth( int month);
void switch_light(byte why);

#include "create.hpp"

#include "ansi_parser.hpp"


/* mail.c */

int        mail_ok(struct char_data* ch);
struct char_data* find_mailman(struct char_data* ch);
void        postmaster_send_mail(struct char_data* ch, int cmd, char* arg);
void        postmaster_check_mail(struct char_data* ch, int cmd, char* arg);
void        postmaster_receive_mail(struct char_data* ch, int cmd, char* arg);
int        scan_mail_file(void);

/* create.*.c */
void do_medit(struct char_data* ch, char* argument, int cmd);
void do_oedit(struct char_data* ch, char* argument, int cmd);
void MobEdit(struct char_data* ch, char* arg);
void ObjEdit(struct char_data* ch, char* arg);

/* All Done! (yay!) */
#include "status.hpp"
#endif
