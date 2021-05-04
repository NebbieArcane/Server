/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __UTILITY_HPP
#define __UTILITY_HPP
/***************************  System  include ************************************/
#include <cstdio>
#include <boost/lexical_cast.hpp>
/***************************  Local    include ************************************/
namespace Alarmud {

bool isNullChar(struct char_data* ch);
int LoadZoneFile(FILE* fl, int zon);
FILE* OpenZoneFile(struct char_data* c, int zone);
int SaveZoneFile(FILE* fp, int start_room, int end_room) ;
void mail_to_god(struct char_data* ch, const char* god, const char* message);

    // Achievements stuff
char* spamAchie(struct char_data* ch, const char *titolo, int valore, const char *stringa);
bool CheckMercyTable(struct char_data* ch, int quest, int amount);
int CheckMobQuest(int vnumber);
void RewardAll(struct char_data* ch, int achievement_type, int achievement_class, int achievement_level);
void CheckQuestFail(struct char_data* ch);
void AssignMobQuestToToon(struct char_data* ch, int quest, int vnumber);
bool CheckQuest(struct char_data* ch, int quest_number);
int maxAchievements(struct char_data* ch);
int MaxValueAchievement(int achievement_class, int achievement_type, int achievement_level);
int n_bosskill(int vnumber, int achievement_class);
bool hasAchievement(struct char_data* ch, int achievement_class, int display);
std::string bufferAchie(struct char_data* ch, int achievement_type, int achievement_class, int lvl, int num, bool formato, int check);
int race_achievement(int race);
void restringReward(struct obj_data* obj, int obj_slot_number, int max_name, int val_random);
void CheckAchie(struct char_data* ch, int achievement_type, int achievement_class, int amount = 1);
struct LivelloAchie StringaAchie(int valore, int achievement_type, int achievement_class);

FILE* MakeZoneFile(struct char_data* c, int zone);
bool CheckPrac(int classe, int id, int liv);
struct time_info_data real_time_passed(time_t t2, time_t t1);
bool ThereIsPCInRoom(long lRoom);
bool CanSeePCInRoom(struct char_data* pMob);
int HowManyCanSeePCInRoom(struct char_data* pMob);
void fake_setup_dir(FILE* fl, long room, int dir);
char getall(char* name, char* newname);
char in_clan(struct char_data* ch1, struct char_data* ch2);
char in_group_internal(struct char_data* ch1, struct char_data* ch2, int strict);
char in_group_strict(struct char_data* ch1, struct char_data* ch2);
char in_group(struct char_data* ch1, struct char_data* ch2);
char* lower(const char* s);
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
int CheckForBlockedMove(struct char_data* ch, int cmd, const char* arg, int room, int dir, int iClass);
int CheckGetBarbarianOK(struct char_data* ch, struct obj_data* obj_object);
int CheckGiveBarbarianOK(struct char_data* ch, struct char_data* vict, struct obj_data* obj);
int check_nomagic(struct char_data* ch, const char* msg_ch, const char* msg_rm);
int check_nomind(struct char_data* ch, const char* msg_ch, const char* msg_rm);
int check_soundproof(struct char_data* ch);
int CountLims(struct obj_data* obj);
int ContaOggetti(struct obj_data* obj);
int DetermineExp(struct char_data* mob, int exp_flags);
int dice(int number, int size);
const char* zonename_by_room(int room);
int RandomRoomByLevel(int level);
int EgoBladeSave(struct char_data* ch);
int EgoSave(struct char_data* ch);
int EqWBits(struct char_data* ch, const unsigned int bits);
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
int scan_number(const char* text, int* rval);
int StandUp(struct char_data* ch);
int str_cmp2(const char* arg1, const char* arg2);
int str_cmp(const char* arg1, const char* arg2);
int strn_cmp(const char* arg1, const char* arg2, int n);
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
void CallForGuard(struct char_data* ch, struct char_data* vict, int lev, int area);
void CallForMobs(struct char_data* pChar, struct char_data* pVict, int iLevel, int iMobToCall);
void CleanZone(int zone);
void DevelopHatred(struct char_data* ch, struct char_data* v);
void DoNothing(void* pDummy);
bool inRange(int low, int high, int x);
void down_river(unsigned long pulse);
void FallOffMount(struct char_data* ch, struct char_data* h);
void FighterMove(struct char_data* ch);
void FORGET(struct char_data* ch, int spl);
void IncrementZoneNr(int nr);
void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max);
void MakeNiftyAttack(struct char_data* ch);
void MonkMove(struct char_data* ch);
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
void sprintbit(unsigned long vektor, const char* names[], char* result);
void sprintbit2(unsigned long vektor, const char* names[], unsigned long vektor2, const char* names2[], char* result);
void sprinttype(int type, const char* names[], char* result);
void TeleportPulseStuff(unsigned long pulse);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
void mud_time_passed2(time_t t2, time_t t1, struct time_info_data* t);
char RandomChar() ;
char RemColorString(char * buffer) ;
const char* RandomWord() ;
template <typename T>
T tonumber(std::string source,T fallback) {
	try {
		return boost::lexical_cast<T>(source);
	}
	catch(...) {

	}
	return fallback;
}

} // namespace Alarmud
#endif
