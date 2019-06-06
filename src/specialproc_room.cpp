/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "specialproc_room.hpp"
#include "nilmys.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spec_procs3.hpp"
#include "speciali.hpp"
#include "lucertole.hpp"
namespace Alarmud {
struct RoomSpecialProcEntry roomproc[] = {
	{ "BlockWay", BlockWay },
	{ "CapannaVillaggio", CapannaVillaggio },
	{ "ChurchBell", ChurchBell },
	{ "ColloSerpente", ColloSerpente },
	{ "Donation", Donation },
	{ "Fountain", Fountain },
	{ "House", House },
	{ "Magic_Fountain", Magic_Fountain },
	{ "Rampicante", Rampicante },
	{ "bank", bank },
	{ "druid_challenge_prep_room", druid_challenge_prep_room },
	{ "druid_challenge_room", druid_challenge_room },
	{ "dump", dump },
	{ "monk_challenge_prep_room", monk_challenge_prep_room },
	{ "monk_challenge_room", monk_challenge_room },
	{ "pet_shops", pet_shops },
//  Quest Nilmys
    { "gonhag_block", gonhag_block },
    { "gonhag_chain", gonhag_chain },
    { "portale_ombra", portale_ombra },
//  fine Quest Nilmys
	{ "pray_for_items", pray_for_items },
	{ "BlockAlign", BlockAlign },
	{ "MobKillInRoom", MobKillInRoom },   // SALVO 2006 Quest fisse
	{ "zFineprocedure", NULL },
};
} // namespace Alarmud

