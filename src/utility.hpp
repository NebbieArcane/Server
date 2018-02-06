/*$Id: utility.h,v 1.2 2002/02/13 12:30:59 root Exp $
*/
#if !defined( _UTILITY_H )
#define _UTILITY_H
#include <stdio.h>
bool ThereIsPCInRoom( long lRoom );
bool CanSeePCInRoom( struct char_data* pMob );
int HowManyPCInRoom( long lRoom );
int HowManyCanSeePCInRoom( struct char_data* pMob );
void _mudlog( const char* const file,int line,unsigned uType,const char* const szString, ... );
void fake_setup_dir(FILE* fl, long room, int dir);
char* fread_string(FILE* fl);
int check_falling(struct char_data* ch);
void NailThisSucker( struct char_data* ch);
#define mudlog(...) _mudlog(__FILE__,__LINE__,__VA_ARGS__)
char getall(char* name, char* newname);
char in_clan(struct char_data* ch1, struct char_data* ch2);
char in_group_internal(struct char_data* ch1, struct char_data* ch2, int strict);
char in_group_strict(struct char_data* ch1, struct char_data* ch2);
char in_group(struct char_data* ch1, struct char_data* ch2);
char* lower(char* s);
char* replace(char* s, char vecchio, char nuovo);
int anti_barbarian_stuff(struct obj_data* obj_object);
int apply_soundproof(struct char_data* ch);
int CanFightEachOther(struct char_data* ch, struct char_data* ch2);
int CAN_SEE_OBJ(struct char_data* ch, struct obj_data* obj);
int CAN_SEE(struct char_data* s, struct char_data* o);
int CanSeeTrap(int num, struct char_data* ch);
int CheckEgoEquip(struct char_data* ch, struct obj_data* obj);
int CheckEgoGet(struct char_data* ch, struct obj_data* obj);
int CheckEgoGive(struct char_data* ch, struct char_data* vict, struct obj_data* obj);
int CheckForBlockedMove(struct char_data* ch, int cmd, char* arg, int room, int dir, int iClass);
int CheckGetBarbarianOK(struct char_data* ch, struct obj_data* obj_object);
int CheckGiveBarbarianOK(struct char_data* ch, struct char_data* vict, struct obj_data* obj);
int check_nomagic(struct char_data* ch, char* msg_ch, char* msg_rm);
int check_nomind(struct char_data* ch, char* msg_ch, char* msg_rm);
int check_soundproof(struct char_data* ch);
int CountLims(struct obj_data* obj);
int DetermineExp(struct char_data* mob, int exp_flags);
int dice(int number, int size);
int EgoBladeSave(struct char_data* ch);
int EgoSave(struct char_data* ch);
int EqWBits(struct char_data* ch, int bits);
int exist(char* s);
int exit_ok(struct room_direction_data* exit, struct room_data** rpp);
int fighting_in_room(int room_n);
int FindZone(int zone);
int follow_time(struct char_data* ch);
int getabunch(char* name, char* newname);
int GetApprox(int num, int perc);
int getFreeAffSlot(struct obj_data* obj);
int GetNewRace(struct char_file_u* s);
int GetSumRaceMaxLevInRoom(struct char_data* ch);
int HasHands(struct char_data* ch);
int HasObject(struct char_data* ch, int ob_num);
int HasWBits(struct char_data* ch, int bits);
int HowManyPCInRoom(long lRoom);
int HowManySpecials(struct char_data* ch);
int InvWBits(struct char_data* ch, int bits);
int IsAnimal(struct char_data* ch);
int IsBadSide(struct char_data* ch);
int IsDarkOutside(struct room_data* rp);
int IsDiabolic(struct char_data* ch);
int IsDragon(struct char_data* ch);
int IsExtraPlanar(struct char_data* ch);
int IsGiantish(struct char_data* ch);
int IsGiant(struct char_data* ch);
int IsGodly(struct char_data* ch);
int IsGoodSide(struct char_data* ch);
int IsHumanoid(struct char_data* ch);
int IS_LINKDEAD(struct char_data* ch);
int IsLycanthrope(struct char_data* ch);
int IsMagicSpell(int spell_num);
int IS_MURDER(struct char_data* ch);
int IsOnPmp(int room_nr);
int IsOther(struct char_data* ch);
int IsPerson(struct char_data* ch);
int IsReptile(struct char_data* ch);
int IsRideable(struct char_data* ch);
int IsRoomDistanceInRange(int nFirstRoom, int nSecondRoom, int nRange);
int IsSmall(struct char_data* ch);
int IsSpecialized(int sk_num);
int IS_STEALER(struct char_data* ch);
int IsUndead(struct char_data* ch);
int IS_UNDERGROUND(struct char_data* ch);
int IsVeggie(struct char_data* ch);
int ItemAlignClash(struct char_data* ch, struct obj_data* obj);
int ItemEgoClash(struct char_data* ch, struct obj_data* obj, int bon);
int LimObj(struct char_data* ch);
int MaxCanMemorize(struct char_data* ch, int spell);
int MaxChrForRace(struct char_data* ch);
int MaxConForRace(struct char_data* ch);
int MaxDexForRace(struct char_data* ch);
int MAX(int a, int b);
int MaxIntForRace(struct char_data* ch);
int MaxLimited(int lev);
int MAX_SPECIALS(struct char_data* ch);
int MaxStrForRace(struct char_data* ch);
int MaxWisForRace(struct char_data* ch);
int MEMORIZED(struct char_data* ch, int spl);
int MIN(int a, int b);
int MobCountInRoom(struct char_data* list);
int MobVnum(struct char_data* c);
int MountEgoCheck(struct char_data* ch, struct char_data* horse);
int NoSummon(struct char_data* ch);
int number(int from, int to);
int NumCharmedFollowersInRoom(struct char_data* ch);
int ObjVnum(struct obj_data* o);
int RecCompObjNum(struct obj_data* o, int obj_num);
int RideCheck(struct char_data* ch, int mod);
int room_of_object(struct obj_data* obj);
int scan_number(char* text, int* rval);
int SiteLock(char* site);
int StandUp(struct char_data* ch);
int str_cmp2(char* arg1, char* arg2);
int str_cmp(char* arg1, char* arg2);
int strn_cmp(char* arg1, char* arg2, int n);
int too_many_followers(struct char_data* ch);
int WeaponImmune(struct char_data* ch);
struct char_data* char_holding(struct obj_data* obj);
struct char_data* FindMobDiffZoneSameRace(struct char_data* ch);
struct time_info_data age(struct char_data* ch);
unsigned int GetItemClassRestrictions(struct obj_data* obj);
unsigned IsImmune(struct char_data* ch, int bit);
unsigned IsResist(struct char_data* ch, int bit);
unsigned IsSusc(struct char_data* ch, int bit);
void age2(struct char_data* ch, struct time_info_data* g);
void age3(struct char_data* ch, struct time_info_data* g);
void buglog(unsigned uType, char* szString, ...);
void CallForGuard(struct char_data* ch, struct char_data* vict, int lev, int area);
void CallForMobs(struct char_data* pChar, struct char_data* pVict, int iLevel, int iMobToCall);
void CleanZone(int zone);
void DevelopHatred(struct char_data* ch, struct char_data* v);
void DoNothing(void* pDummy);
void down_river(unsigned long pulse);
void do_WorldSave(struct char_data* ch, char* argument, int cmd);
void FallOffMount(struct char_data* ch, struct char_data* h);
void FighterMove(struct char_data* ch);
void FORGET(struct char_data* ch, int spl);
void IncrementZoneNr(int nr);
void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max);
void MakeNiftyAttack(struct char_data* ch);
void MonkMove(struct char_data* ch);
void _mudlog(const char* const file, int line, unsigned uType, const char* const szString, ...);
void* Mymalloc(long size);
void RecurseRoom(long lInRoom, int iLevel, int iMaxLevel, unsigned char* achVisitedRooms);
void RemAllAffects(struct char_data* ch);
void RestoreChar(struct char_data* ch);
void RiverPulseStuff(unsigned long pulse);
void RoomLoad(struct char_data* ch, int start, int end);
void RoomSave(struct char_data* ch, long start, long end);
void SetDefaultLang(struct char_data* ch);
void SetHunting(struct char_data* ch, struct char_data* tch);
void SetRacialStuff(struct char_data* mob);
void SpaceForSkills(struct char_data* ch);
void sprintbit(unsigned long vektor, char* names[], char* result);
void sprinttype(int type, char* names[], char* result);
void TeleportPulseStuff(unsigned long pulse);
#endif
