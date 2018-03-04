#include "config.hpp"
#include "specialproc_other.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spec_procs3.hpp"
#include "lucertole.hpp"
struct special_proc_entry otherproc[] = {
	{ "AGGRESSIVE",(special_proc) AGGRESSIVE },
	{ "AbbarachDragon",(special_proc)AbbarachDragon },
	{ "AbyssGateKeeper",(special_proc) AbyssGateKeeper },
	{ "Ankheg",(special_proc)(special_proc) Ankheg },
	{ "banshee_lorelai",(special_proc) banshee_lorelai },
	{ "Beholder",(special_proc) Beholder },
	{ "BerserkerItem",(special_proc) BerserkerItem },
	{ "BiosKaiThanatos",(special_proc) BiosKaiThanatos },
	{ "BreathWeapon",(special_proc) BreathWeapon },
	{ "Capo_Fucina",(special_proc) Capo_Fucina },
	{ "camino",(special_proc) camino },
	{ "CaravanGuildGuard",(special_proc) CaravanGuildGuard },
	{ "CarrionCrawler",(special_proc) CarrionCrawler },
	{ "ClericGuildMaster",(special_proc) ClericGuildMaster },
	{ "Cockatrice",(special_proc) Cockatrice },
	{ "Demon",(special_proc) Demon },
	{ "DemonTeacher",(special_proc) DemonTeacher },
	{ "Devil",(special_proc) Devil },
	{ "DogCatcher",(special_proc) DogCatcher },
	{ "DragonHunterLeader",(special_proc) DragonHunterLeader },
	{ "Drow",(special_proc) Drow },
	{ "DruidChallenger",(special_proc) DruidChallenger },
	{ "DruidGuildMaster",(special_proc) DruidGuildMaster },
	{ "DwarvenMiners",(special_proc) DwarvenMiners },
	#if EGO
	{ "EvilBlade",(special_proc) EvilBlade },
	{ "GoodBlade",(special_proc) GoodBlade },
	{ "NeutralBlade",(special_proc) NeutralBlade },
	#endif
	{ "EditMaster",(special_proc)EditMaster},
	{ "Esattore",(special_proc)Esattore},
	{ "ForceMobToAction",(special_proc) ForceMobToAction },
	{ "FireBreather",(special_proc) FireBreather },
	{ "HuntingMercenary",(special_proc) HuntingMercenary },
	{ "Interact",(special_proc) Interact },
	{ "Keftab",(special_proc) Keftab },
	{ "LegionariV",(special_proc) LegionariV },
	{ "LightningBreather",(special_proc) LightningBreather },
	{ "Lizardman",(special_proc) Lizardman },
	{ "MageGuildMaster",(special_proc) MageGuildMaster },
	{ "MidgaardCityguard",(special_proc) MidgaardCityguard },
	{ "MobBlockWay",(special_proc) MobBlockWay },
	{ "ModHit",(special_proc) ModHit },
	{ "MonkChallenger",(special_proc) MonkChallenger },
	{ "MordGuard",(special_proc) MordGuard },
	{ "MordGuildGuard",(special_proc) MordGuildGuard },
	{ "NewThalosGuildGuard",(special_proc) NewThalosGuildGuard },
	{ "NewThalosMayor",(special_proc) NewThalosMayor },
	{ "NudgeNudge",(special_proc) NudgeNudge },
	{ "Orso_Bianco",(special_proc) Orso_Bianco },
	{ "PaladinGuildGuard",(special_proc) PaladinGuildGuard },
	{ "PaladinGuildmaster",(special_proc) PaladinGuildmaster },
	{ "PostMaster",(special_proc) PostMaster },
	{ "PrimoAlbero",(special_proc) PrimoAlbero },
	{ "PrisonGuard",(special_proc) PrisonGuard },
	{ "PrydainGuard",(special_proc) PrydainGuard },
	{ "PsiGuildmaster",(special_proc) PsiGuildmaster },
	{ "Pungiglione",(special_proc) Pungiglione },
	{ "Pungiglione_maggiore",(special_proc) Pungiglione_maggiore },
	{ "RangerGuildmaster",(special_proc) RangerGuildmaster },
	{ "RepairGuy",(special_proc) RepairGuy },
	{ "Ringwraith",(special_proc) Ringwraith },
	{ "RustMonster",(special_proc) RustMonster },
	{ "Samah",(special_proc) Samah },
	{ "Slavalis",(special_proc) Slavalis },
	{ "SlotMachine",(special_proc) SlotMachine },
	{ "spGeneric",(special_proc) spGeneric },
	{ "SporeCloud",(special_proc) SporeCloud },
	{ "ChangeDam",(special_proc) ChangeDam },
	{ "LibroEroi",(special_proc) LibroEroi },
	{ "MobBlockAlign",(special_proc) MobBlockAlign },
	{ "LadroOfferte",(special_proc) LadroOfferte },
	{ "Vampire_Summoner",(special_proc) Vampire_Summoner },
	{ "Nightmare",(special_proc) Nightmare },
	{ "SputoVelenoso",(special_proc) SputoVelenoso },
	{ "StatTeller",(special_proc) StatTeller },
	{ "StatMaster",(special_proc)StatMaster  },
	{ "StormGiant",(special_proc) StormGiant },
	{ "SultanGuard",(special_proc) SultanGuard },
	{ "ThiefGuildMaster",(special_proc) ThiefGuildMaster },
	{ "thion_loader",(special_proc) thion_loader },
	{ "ThrowerMob",(special_proc) ThrowerMob },
	{ "trap_obj",(special_proc) trap_obj },
	{ "TreeThrowerMob",(special_proc) TreeThrowerMob },
	{ "TrueDam",(special_proc) TrueDam },
	{ "Tsuchigumo",(special_proc) Tsuchigumo },
	{ "Tyrannosaurus_swallower",(special_proc) Tyrannosaurus_swallower },
	{ "Tytan",(special_proc) Tytan },
	{ "Valik",(special_proc) Valik },
	{ "WarriorGuildMaster",(special_proc) WarriorGuildMaster },
	{ "XpMaster",(special_proc) XpMaster },
	{ "acid_monster",(special_proc) acid_monster },
	{ "acid_monster/*",(special_proc) acid_monster },
	{ "andy_wilcox",(special_proc) andy_wilcox },
	{ "archer_instructor",(special_proc) archer_instructor },
	{ "astral_portal",(special_proc) astral_portal },
	{ "attack_rats",(special_proc) attack_rats },
	{ "avatar_celestian",(special_proc) avatar_celestian },
	{ "baby_bear",(special_proc) baby_bear },
	{ "banana",(special_proc) banana },
	{ "banshee",(special_proc) banshee },
	{ "barbarian_guildmaster",(special_proc) barbarian_guildmaster },
	{ "behir",(special_proc) behir },
	{ "blink",(special_proc) blink },
	{ "board",(special_proc) board },
	#if !GCC27
	{ "chess_game",(special_proc) chess_game },
	#endif
	{ "coldcaster",(special_proc) coldcaster },
	{ "creceptionist",(special_proc) creceptionist },
	{ "creeping_death",(special_proc) creeping_death },
	{ "death_knight",(special_proc) death_knight },
	{ "delivery_beast",(special_proc) delivery_beast },
	{ "delivery_elf",(special_proc) delivery_elf },
	{ "druid_protector",(special_proc) druid_protector },
	{ "enter_obj",(special_proc) enter_obj },
	{ "eric_johnson",(special_proc) eric_johnson },
	{ "fido",(special_proc) fido },
	{ "fighter_mage",(special_proc) fighter_mage },
	{ "flame",(special_proc) flame },
	{ "geyser",(special_proc) geyser },
	{ "ghost",(special_proc) ghost },
	{ "ghostsoldier",(special_proc) ghostsoldier },
	{ "ghoul",(special_proc) ghoul },
	{ "goblin_sentry",(special_proc) goblin_sentry },
	{ "golgar",(special_proc) golgar },
	{ "green_slime",(special_proc) green_slime },
	{ "guardian",(special_proc) guardian },
	{ "guild_guard",(special_proc) guild_guard },
	{ "hit_sucker",(special_proc) hit_sucker },
	{ "hunter",(special_proc) hunter },
	{ "miner_teacher",(special_proc) miner_teacher },
	{ "forge_teacher",(special_proc) forge_teacher },
	{ "determine_teacher",(special_proc) determine_teacher },
	{ "equilibrium_teacher",(special_proc) equilibrium_teacher },
	{ "jabberwocky",(special_proc) jabberwocky },
	{ "janitor",(special_proc) janitor },
	{ "jive_box",(special_proc) jive_box },
	{ "jugglernaut",(special_proc) jugglernaut },
	{ "keystone",(special_proc) keystone },
	{ "lattimore",(special_proc) lattimore },
	{ "lich_church",(special_proc) lich_church },
	{ "lizardman_shaman",(special_proc) lizardman_shaman },
	{ "loremaster",(special_proc) loremaster },
	{ "mad_cyrus",(special_proc) mad_cyrus },
	{ "mad_gertruda",(special_proc) mad_gertruda },
	{ "mage_specialist_guildmaster",(special_proc) mage_specialist_guildmaster },
	{ "mayor",(special_proc) mayor },
	{ "medusa",(special_proc) medusa },
	{ "monk_master",(special_proc) monk_master },
	{ "Moribondo",(special_proc) Moribondo },
	{ "msg_obj",(special_proc) msg_obj },
	{ "ninja_master",(special_proc) ninja_master },
	{ "nodrop",(special_proc) nodrop },
	{ "paramedics",(special_proc) paramedics },
	{ "portal",(special_proc) portal },
	{ "puff",(special_proc) puff },
	{ "ragno_intermittente",(special_proc) ragno_intermittente },
	{ "Rakda",(special_proc) Rakda },
	{ "raven_iron_golem",(special_proc) raven_iron_golem },
	{ "real_fox",(special_proc) real_fox },
	{ "real_rabbit",(special_proc) real_rabbit },
	{ "receptionist",(special_proc) receptionist },
	{ "regenerator",(special_proc) regenerator },
	{ "replicant",(special_proc) replicant },
	{ "sailor",(special_proc) sailor },
	{ "scraps",(special_proc) scraps },
	{ "shadow",(special_proc) shadow },
	{ "shaman",(special_proc) shaman },
	{ "sisyphus",(special_proc) sisyphus },
	{ "snake",(special_proc) snake },
	{ "snake_plus",(special_proc) snake_plus },
	{ "snake_avt",(special_proc) snake_avt },
	{ "snake_avt2",(special_proc) snake_avt2 },
	{ "snake_guardian",(special_proc) snake_guardian },
	{ "soap",(special_proc) soap },
	{ "strahd_vampire",(special_proc) strahd_vampire },
	{ "strahd_zombie",(special_proc) strahd_zombie },
	{ "temple_labrynth_liar",(special_proc) temple_labrynth_liar },
	{ "temple_labrynth_sentry",(special_proc) temple_labrynth_sentry },
	{ "timnus",(special_proc) timnus },
	{ "tormentor",(special_proc) tormentor },
	{ "trapper",(special_proc) trapper },
	{ "trogcook",(special_proc) trogcook },
	{ "troguard",(special_proc) troguard },
	{ "vampire",(special_proc) vampire },
	{ "village_woman",(special_proc) village_woman },
	{ "virgin_sac",(special_proc) virgin_sac },
	{ "web_slinger",(special_proc) web_slinger },
	{ "winger",(special_proc) winger },
	{ "wraith",(special_proc) wraith },
	{ "zone_obj",(special_proc) zone_obj },
	{ "zombie_master",(special_proc) zombie_master },
	{ "ItemGiven",(special_proc) ItemGiven },  // SALVO 2006 Quest fisse
	{ "ItemPut",(special_proc)ItemPut },  // SALVO 2006 Quest fisse
	{ "zFineprocedure", NULL },
};