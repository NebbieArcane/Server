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
 * pieni fuori fight; in fight la corte resta congelata.
 */
inline constexpr int kOlympZeusRoom = 13784;
inline constexpr int kOlympZeusKeyVnum = 13893;
inline constexpr int kOlympArtemisKeyVnum = 13881;
inline constexpr int kOlympAttendantsMax = 3;

/** Dei della corte Olimpo (no Efesto / attendant / erranti). */
[[nodiscard]] bool is_olympian_god_vnum(int vnum) noexcept;

/** Spawn/despawn attendants per la sala (chiamare dal tick del dio). */
void olympian_maintain_court_room(struct room_data* rp);

/** Solo spawn (niente extract): sicuro da EVENT_COMMAND / RawMove. */
void olympian_spawn_court_room(struct room_data* rp);

/** Despawn corte alla morte del dio (Zeus / KeyDrop). */
void olympian_clear_court_on_god_death(struct char_data* god);

MOBSPECIAL_FUNC(OlympianKeyDrop);
MOBSPECIAL_FUNC(OlympianWanderer);
MOBSPECIAL_FUNC(OlympianAttendant);
OBJSPECIAL_FUNC(OlympianKey);
ROOMSPECIAL_FUNC(OlympianCourt);

} // namespace Alarmud
#endif // __SPEC_MOUNT_OLYMP_HPP
