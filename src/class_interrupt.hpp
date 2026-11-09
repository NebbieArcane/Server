/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __CLASS_INTERRUPT_HPP
#define __CLASS_INTERRUPT_HPP

namespace Alarmud {

/** Melee / ranger / psi — comandi. */
ACTION_FUNC(do_hilt_smash);
ACTION_FUNC(do_fury_howl);
ACTION_FUNC(do_pressure_point);
ACTION_FUNC(do_cheap_shot);
ACTION_FUNC(do_tactical_strike);
ACTION_FUNC(do_mind_spike);

/** Caster / paladin — via cast '…'. */
void cast_holy_rebuke(byte level, struct char_data* ch, const char* arg, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_spellshatter(byte level, struct char_data* ch, const char* arg, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_divine_rebuke(byte level, struct char_data* ch, const char* arg, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_thornbind(byte level, struct char_data* ch, const char* arg, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj);

void spell_holy_rebuke(byte level, struct char_data* ch, struct char_data* victim,
					   struct obj_data* obj);
void spell_spellshatter(byte level, struct char_data* ch, struct char_data* victim,
						struct obj_data* obj);
void spell_divine_rebuke(byte level, struct char_data* ch, struct char_data* victim,
						 struct obj_data* obj);
void spell_thornbind(byte level, struct char_data* ch, struct char_data* victim,
					 struct obj_data* obj);

[[nodiscard]] bool is_class_interrupt_id(int id) noexcept;

/** Rifiuta le spell interrupt (CD, classe, livello) prima di mana e lag. */
[[nodiscard]] bool class_interrupt_refuse_cast(struct char_data* ch, int spell_id);

/** Pulisce il CD interrupt (extract_char). */
void clear_class_interrupt_cooldown(struct char_data* ch);

} // namespace Alarmud

#endif // __CLASS_INTERRUPT_HPP
