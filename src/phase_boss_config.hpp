/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PHASE_BOSS_CONFIG_HPP
#define __PHASE_BOSS_CONFIG_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace Alarmud {

/**
 * Dati PhaseBoss (profili, ability, pesi).
 * Nuovo boss: aggiungi un PhaseBossProfile in phase_boss_config.cpp e
 * registralo in kAll[]; myst.spe → M <vnum> PhaseBoss <id>
 */

enum class PhaseBossInterrupt : unsigned {
	None       = 0,
	ClassBreak = 1u << 0 /* skill interrupt 1x classe */
};

enum class PhaseBossDmg : std::uint8_t { Lightning = 0, Bludgeon };

enum class PhaseBossTarget : std::uint8_t { Tank = 0, Aoe };

[[nodiscard]] constexpr PhaseBossInterrupt operator|(PhaseBossInterrupt a,
													 PhaseBossInterrupt b) noexcept {
	return static_cast<PhaseBossInterrupt>(static_cast<unsigned>(a) |
										   static_cast<unsigned>(b));
}

[[nodiscard]] constexpr PhaseBossInterrupt operator&(PhaseBossInterrupt a,
													 PhaseBossInterrupt b) noexcept {
	return static_cast<PhaseBossInterrupt>(static_cast<unsigned>(a) &
										   static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool phase_boss_any(PhaseBossInterrupt mask,
											PhaseBossInterrupt bit) noexcept {
	return (mask & bit) != PhaseBossInterrupt::None;
}

/** Una ability del boss (telegraph → resolve). Catalogo in abilities[]. */
struct PhaseBossAbility {
	std::string_view key;              /* id usato nei pesi (es. "bolt") */
	int windup = 0;                    /* tick di caricamento (≈4s l'uno) */
	int cast_hp = 0;                   /* "vita" del cast; interrupt la scalano */
	PhaseBossInterrupt mask = PhaseBossInterrupt::None; /* None = non interrompibile */
	PhaseBossTarget target = PhaseBossTarget::Tank;     /* Tank o AoE room */
	PhaseBossDmg dmg = PhaseBossDmg::Lightning;         /* tipo per damage() */
	int dam_base = 0;                  /* grezzo = base + dice_n d dice_s */
	int dice_n = 0;
	int dice_s = 0;
	/** Pavimento soft: danno ∝ cast_hp rimasti, non sotto soft_num/soft_den. */
	int soft_num = 2;
	int soft_den = 5;
	int min_phase = 0;                 /* 0 Idle / 1 Rage / 2 Wrath: non prima */
	int min_fight_ticks = 0;           /* gate (es. ultimate dopo N tick fight) */
	const char* emote_start = nullptr; /* inizio telegraph */
	const char* emote_hint = nullptr;  /* hint interrupt (opz.) */
	const char* emote_tick = nullptr;  /* durante windup */
	const char* emote_full = nullptr;  /* resolve senza interrupt */
	const char* emote_soft = nullptr;  /* resolve dopo soft */
};

/** Una delle 3 fasi HP: [0]=Idle, [1]=Rage, [2]=Wrath. */
struct PhaseBossPhase {
	int enter_hp = 100;                /* entra a ≤ questo HP% (Idle=100) */
	const char* enter_emote = nullptr;
	const char* enter_emote2 = nullptr;
	int add_vnum = 0;                  /* summon all'ingresso (0=niente) */
	int add_max = 0;
	const char* add_emote = nullptr;
	int cd_after_cast = 3;             /* tick di pausa dopo un cast in questa fase */
};

/** Entry tabella pesi: chance assoluta 1..100, in ordine (primo match vince). */
struct PhaseBossWeight {
	std::string_view ability_key;
	int chance = 0;
};

/**
 * Overlay opzionale enrage (non e' una 4a fase kit).
 * Sotto enter_hp: bonus su melee + ability.
 * bonus% = base_bonus_pct + per_hp_pct * (enter_hp - hp%).
 * enter_hp <= 0 = disattivo.
 */
struct PhaseBossEnrage {
	int enter_hp = 0;                  /* soglia HP%; 0 = off */
	int base_bonus_pct = 10;           /* bonus % esatto a enter_hp */
	int per_hp_pct = 3;                /* +N% per ogni 1% HP sotto enter */
	const char* enter_emote = nullptr; /* una volta all'ingresso */
};

/**
 * Profilo completo di un boss PhaseBoss.
 *
 * Flusso engine: a EVENT_TICK in fight aggiorna fase HP → eventualmente enrage →
 * avanza windup o sceglie ability dai weights della fase corrente.
 * Interrupt di classe: phase_boss_try_interrupt (soft proporzionale / hard).
 */
struct PhaseBossProfile {
	/** Nome spe / lookup: M <vnum> PhaseBoss <id> */
	std::string_view id;
	/** Se >0, bind automatico a quel mob vnum (oltre al token spe). 0 = solo per id. */
	int bind_vnum = 0;
	/**
	 * Soft-enrage temporale: dopo N fight tick forza fase Wrath anche sopra
	 * la soglia HP Wrath (pressione se il fight dura troppo).
	 */
	int soft_enrage_ticks = 120;
	/** Fasi Idle / Rage / Wrath (soglie, emote, add, cd post-cast). */
	std::array<PhaseBossPhase, 3> phases{};
	/** Catalogo ability (fino a 8); vedi ability_count. */
	std::array<PhaseBossAbility, 8> abilities{};
	/** Quante entry in abilities[] sono valide. */
	int ability_count = 0;
	/**
	 * Pesi per fase: weights[fase][i] = {key, chance}.
	 * Roll 1..100; prima entry con roll<=chance (e gate ok) parte.
	 */
	std::array<std::array<PhaseBossWeight, 6>, 3> weights{};
	/** Quante weight entry usate per Idle / Rage / Wrath. */
	std::array<int, 3> weight_count{};
	/** Emote a EVENT_DEATH (oltre extract adds). */
	const char* death_emote = nullptr;
	/** Enrage HP opzionale (danno melee+special); enter_hp=0 disattiva. */
	PhaseBossEnrage enrage{};
	/** Msg al PG se riprova interrupt su cast gia' hard-broken. */
	const char* already_broken = "L'azione del boss e' gia' stata spezzata.\n\r";
};

[[nodiscard]] const PhaseBossProfile* const* phase_boss_profiles(std::size_t& count) noexcept;
[[nodiscard]] const PhaseBossProfile* phase_boss_find_by_id(std::string_view id) noexcept;
[[nodiscard]] const PhaseBossProfile* phase_boss_find_by_vnum(int vnum) noexcept;
[[nodiscard]] const PhaseBossAbility* phase_boss_find_ability(const PhaseBossProfile& profile,
															 std::string_view key) noexcept;

extern const PhaseBossProfile kPhaseBossZeus;

} // namespace Alarmud

#endif // __PHASE_BOSS_CONFIG_HPP
