/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_MOUNT_OLYMP_HPP
#define __SPEC_MOUNT_OLYMP_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {

/**
 * Monte Olimpo: chiavi per porta, drop random (Zeus ultima), erranti,
 * corte di attendants: spawn solo se la divinita' e' ferita; despawn a HP
 * pieni fuori fight; in fight niente refill dei morti, ma tiri di rinforzo
 * (30-21% e 20-5%, +2%/punto da 10%) fino a 1 spawn per fascia.
 */
inline constexpr int kOlympZeusRoom = 13784;
inline constexpr int kOlympZeusVnum = 13784;
inline constexpr int kOlympZeusKeyVnum = 13893;
inline constexpr int kOlympTreasureRoom = 13798;
inline constexpr int kOlympTreasureKeyVnum = 13796;
inline constexpr int kOlympArtemisKeyVnum = 13881;
inline constexpr int kOlympEfestoVnum = 13873;
inline constexpr int kOlympAttendantsMax = 3;
/** Move post-NewMobMov: 360 a lvl 60 non regge un fight lungo (stop_fighting sotto -10). */
inline constexpr int kOlympGodMaxMove = 5000;
/**
 * Due zone Olimpo (non contigue): in mezzo Graecian Quest 13799-13860.
 * Zone1 top 13798, zone2 13861-13899.
 */
inline constexpr int kOlympZone1VnumMin = 13780;
inline constexpr int kOlympZone1VnumMax = 13798;
inline constexpr int kOlympZone2VnumMin = 13861;
inline constexpr int kOlympZone2VnumMax = 13899;

/** Vnum in una delle due zone Olimpo (non Graecian Quest). */
[[nodiscard]] bool is_olympus_area_vnum(int vnum) noexcept;

/** NPC con vnum Olimpo (dei / attendant / erranti / altri mob area). */
[[nodiscard]] bool is_olympus_npc(const struct char_data* ch) noexcept;

/** Dei della corte Olimpo (no Efesto / attendant / erranti). */
[[nodiscard]] bool is_olympian_god_vnum(int vnum) noexcept;

/**
 * Una volta per spawn: max_move + roll immunita' danno (fisici/magici per lvl).
 * Ogni tick in fight: se il tank ha fireshield e il dio non e' immune al fuoco,
 * il dio prende AFF_FIRESHIELD (senza annunci).
 */
void olympian_ensure_god_stamina(struct char_data* god);

/** Spawn/despawn attendants per la sala (chiamare dal tick del dio). */
void olympian_maintain_court_room(struct room_data* rp);

/** Solo spawn (niente extract): sicuro da EVENT_COMMAND / RawMove. */
void olympian_spawn_court_room(struct room_data* rp);

/** Despawn corte alla morte del dio (Zeus / KeyDrop). */
void olympian_clear_court_on_god_death(struct char_data* god);

/**
 * Morte Zeus: carica in Sala del Tesoro un premio dal pool QuestNebbie Olimpo
 * (pesi uguali; armi/scudi a peso dimezzato).
 */
void olympian_zeus_death_treasure(struct char_data* god);

/**
 * Morte NPC Olimpo (no attendants): chance (lv-50)*10 [*2 se RACE_GOD],
 * qty (lv-50)*100 [*2 se god], ±10%; split tra PC in room.
 */
void olympian_try_rune_fragment_drop(struct char_data* victim);

MOBSPECIAL_FUNC(OlympianKeyDrop);
MOBSPECIAL_FUNC(OlympianWanderer);
MOBSPECIAL_FUNC(OlympianAttendant);
OBJSPECIAL_FUNC(OlympianKey);
ROOMSPECIAL_FUNC(OlympianCourt);

} // namespace Alarmud
#endif // __SPEC_MOUNT_OLYMP_HPP
