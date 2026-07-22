/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __MAGICUTILS_HPP
#define __MAGICUTILS_HPP
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
void FailCalm(struct char_data* victim, struct char_data* ch) ;
void FailCharm(struct char_data* victim, struct char_data* ch) ;
void FailPara(struct char_data* victim, struct char_data* ch) ;
void FailPoison(struct char_data* victim, struct char_data* ch) ;
void FailSleep(struct char_data* victim, struct char_data* ch) ;
void FailSnare(struct char_data* victim, struct char_data* ch) ;
void SwitchStuff(struct char_data* giver, struct char_data* taker) ;
void RelateMobToCaster(struct char_data* ch, struct char_data* mob) ;

/** Absorption shields (SPELL_MANASHIELD / SPELL_MIND_OVER_MATTER). */
[[nodiscard]] bool IsAbsorptionShieldSpell(int spell_type);
int ComputeAbsorptionShieldPool(struct char_data* ch, int mana);
bool ApplyAbsorptionShield(struct char_data* ch, int spell_type);
/** Absorb after sanct; efficiency depends on attack type vs shield kind. */
int AbsorbAbsorptionShieldDamage(struct char_data* victim, int dam, int attack_type);
void RefundAbsorptionShield(struct char_data* ch, int spell_type);
struct affected_type* FindAbsorptionShieldAffect(struct char_data* ch);
int GetAbsorptionShieldResidual(struct char_data* ch);
int GetAbsorptionShieldSpellType(struct char_data* ch);

/** Compatibility wrappers for manashield. */
bool ApplyManaShield(struct char_data* ch);
int AbsorbManaShieldDamage(struct char_data* victim, int dam, int attack_type);
void RefundManaShield(struct char_data* ch);
int GetManaShieldResidual(struct char_data* ch);
} // namespace Alarmud
#endif // __MAGICUTILS_HPP
