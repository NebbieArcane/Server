/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "comm.hpp"
#include "cmdid.hpp"
#include "configure.hpp"
#include "act.wizard.hpp"
#include "dispel_balance.hpp"
#include "modify.hpp"
#include "procarea_balance.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace Alarmud {

namespace {

[[nodiscard]] bool iequals(std::string_view a, std::string_view b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), [](char l, char r) {
			   return std::tolower(static_cast<unsigned char>(l)) ==
					  std::tolower(static_cast<unsigned char>(r));
		   });
}

/** Primo token e resto (C++17, niente buffer C / one_argument). */
struct ArgSplit {
	std::string_view first;
	std::string_view rest;
};

[[nodiscard]] ArgSplit split_arg(std::string_view s) {
	const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
	while(!s.empty() && is_space(static_cast<unsigned char>(s.front()))) {
		s.remove_prefix(1);
	}
	if(s.empty()) {
		return {};
	}
	std::size_t i = 0;
	while(i < s.size() && !is_space(static_cast<unsigned char>(s[i]))) {
		++i;
	}
	const std::string_view first = s.substr(0, i);
	std::string_view rest = s.substr(i);
	while(!rest.empty() && is_space(static_cast<unsigned char>(rest.front()))) {
		rest.remove_prefix(1);
	}
	return {first, rest};
}

void configure_usage(char_data* ch) {
	send_to_char(
		"$c0014=== configure ===$c0007\n\r"
		"Topic di balance/listino e orari di reboot.\n\r"
		"\n\r"
		"  $c0014configure dimensione densita|premi|livelli|kit|scorta$c0007\n\r"
		"  $c0014configure dispel$c0007\n\r"
		"  $c0014configure vault$c0007   (QUESTMASTER+)\n\r"
		"  $c0014configure reboot$c0007  (CREATORE+)\n\r"
		"\n\r"
		"Sotto ogni topic: dump, $c0015set$c0007, $c0015reset$c0007 / $c0015help$c0007 "
		"(vault: $c0015list$c0007 / $c0015set$c0007 / $c0015reload$c0007; "
		"reboot: $c0015set HH:MM …$c0007 / $c0015clear$c0007 / $c0015off$c0007).\n\r"
		"Alias: $c0015dimensione densita|…$c0007, $c0015dispelconf$c0007, "
		"$c0015vaultplan$c0007, $c0015reboottime$c0007.\n\r",
		ch);
}

void configure_dimensione_menu(char_data* ch) {
	send_to_char(
		"$c0014configure dimensione$c0007 — sottocomandi:\n\r"
		"  $c0015densita$c0007 | $c0015premi$c0007 | $c0015livelli$c0007 | "
		"$c0015kit$c0007 | $c0015scorta$c0007\n\r"
		"Esempio: $c0014configure dimensione densita$c0007\n\r"
		"         $c0014configure dimensione premi set frag_per_rune 1000$c0007\n\r",
		ch);
}

} // namespace

ACTION_FUNC(do_configure) {
	(void)cmd;
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	const ArgSplit topic_split = split_arg(arg != nullptr ? arg : "");
	const std::string_view topic = topic_split.first;
	const std::string_view rest = topic_split.rest;

	if(topic.empty() || iequals(topic, "help") || iequals(topic, "?")) {
		configure_usage(ch);
		return;
	}

	if(iequals(topic, "dimensione") || iequals(topic, "dimension") ||
	   iequals(topic, "procarea") || iequals(topic, "istanza")) {
		const ArgSplit sub = split_arg(rest);
		if(sub.first.empty()) {
			configure_dimensione_menu(ch);
			return;
		}
		/* procarea_try_balance_wiz_command vuole C-string: copie locali. */
		const std::string subcmd(sub.first);
		const std::string sub_rest(sub.rest);
		if(!procarea_try_balance_wiz_command(ch, subcmd.c_str(), sub_rest.c_str())) {
			configure_dimensione_menu(ch);
		}
		return;
	}

	if(iequals(topic, "dispel") || iequals(topic, "dispelconf") ||
	   iequals(topic, "spell_balance")) {
		const std::string rest_str(rest);
		do_dispelconf(ch, rest_str.c_str(), CMD_DISPELCONF);
		return;
	}

	if(iequals(topic, "vault") || iequals(topic, "vaultplan") ||
	   iequals(topic, "cassette") || iequals(topic, "cassetta")) {
		if(GetMaxLevel(ch) < QUESTMASTER) {
			send_to_char("Il listino cassette richiede livello QUESTMASTER.\n\r", ch);
			return;
		}
		const std::string rest_str(rest);
		do_vaultplan(ch, rest_str.c_str(), CMD_VAULTPLAN);
		return;
	}

	if(iequals(topic, "reboot") || iequals(topic, "reboottime") ||
	   iequals(topic, "riavvio")) {
		if(GetMaxLevel(ch) < CREATORE) {
			send_to_char("Gli orari di reboot richiedono livello CREATORE.\n\r", ch);
			return;
		}
		const std::string rest_str(rest);
		do_reboottime(ch, rest_str.c_str(), CMD_REBOOTTIME);
		return;
	}

	send_to_char("Topic sconosciuto. Digita $c0014configure$c0007 per l'elenco.\n\r", ch);
}

} // namespace Alarmud
