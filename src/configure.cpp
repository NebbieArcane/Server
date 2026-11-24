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
#include "edit_pool_balance.hpp"
#include "modify.hpp"
#include "procarea_balance.hpp"
#include "service_balance.hpp"
#include "utility.hpp"
#include <algorithm>
#include <cctype>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace Alarmud {

namespace {

[[nodiscard]] bool iequals(std::string_view a, std::string_view b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), [](char l, char r) {
			   return std::tolower(static_cast<unsigned char>(l)) ==
					  std::tolower(static_cast<unsigned char>(r));
		   });
}

/** Prefisso case-insensitive: arg abbrevia word (come is_abbrev). */
[[nodiscard]] bool iprefix(std::string_view arg, std::string_view word) {
	if(arg.empty() || arg.size() > word.size()) {
		return false;
	}
	return std::equal(arg.begin(), arg.end(), word.begin(), [](char l, char r) {
		return std::tolower(static_cast<unsigned char>(l)) ==
			   std::tolower(static_cast<unsigned char>(r));
	});
}

/** Primo token e resto. */
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

enum class ConfigTopic {
	Dimensione,
	Dispel,
	Editpool,
	Derent,
	Disincanta,
	Vault,
	Reboot,
};

struct TopicAlias {
	const char* alias;
	ConfigTopic id;
	const char* canon;
};

constexpr TopicAlias kTopicAliases[] = {
	{"dimensione", ConfigTopic::Dimensione, "dimensione"},
	{"dimension", ConfigTopic::Dimensione, "dimensione"},
	{"procarea", ConfigTopic::Dimensione, "dimensione"},
	{"istanza", ConfigTopic::Dimensione, "dimensione"},
	{"dispel", ConfigTopic::Dispel, "dispel"},
	{"dispelconf", ConfigTopic::Dispel, "dispel"},
	{"spell_balance", ConfigTopic::Dispel, "dispel"},
	{"editpool", ConfigTopic::Editpool, "editpool"},
	{"edit_pool", ConfigTopic::Editpool, "editpool"},
	{"pool", ConfigTopic::Editpool, "editpool"},
	{"derent", ConfigTopic::Derent, "derent"},
	{"rent", ConfigTopic::Derent, "derent"},
	{"renteditor", ConfigTopic::Derent, "derent"},
	{"disincanta", ConfigTopic::Disincanta, "disincanta"},
	{"disenchant", ConfigTopic::Disincanta, "disincanta"},
	{"affectslot", ConfigTopic::Disincanta, "disincanta"},
	{"affect_slot", ConfigTopic::Disincanta, "disincanta"},
	{"vault", ConfigTopic::Vault, "vault"},
	{"vaultplan", ConfigTopic::Vault, "vault"},
	{"cassette", ConfigTopic::Vault, "vault"},
	{"cassetta", ConfigTopic::Vault, "vault"},
	{"reboot", ConfigTopic::Reboot, "reboot"},
	{"reboottime", ConfigTopic::Reboot, "reboot"},
	{"riavvio", ConfigTopic::Reboot, "reboot"},
};

struct TopicResolve {
	enum class Kind { Ok, Ambiguous, Unknown } kind{Kind::Unknown};
	ConfigTopic id{ConfigTopic::Dimensione};
	std::vector<const char*> ambiguous;
};

[[nodiscard]] TopicResolve resolve_topic(std::string_view token) {
	TopicResolve out;

	for(const TopicAlias& a : kTopicAliases) {
		if(iequals(token, a.alias)) {
			out.kind = TopicResolve::Kind::Ok;
			out.id = a.id;
			return out;
		}
	}

	/* Prefisso: un canon una sola volta. */
	for(const TopicAlias& a : kTopicAliases) {
		if(!iprefix(token, a.alias)) {
			continue;
		}
		bool already = false;
		for(const char* c : out.ambiguous) {
			if(iequals(c, a.canon)) {
				already = true;
				break;
			}
		}
		if(!already) {
			out.ambiguous.push_back(a.canon);
			out.id = a.id;
		}
	}

	if(out.ambiguous.size() == 1) {
		out.kind = TopicResolve::Kind::Ok;
		out.ambiguous.clear();
		return out;
	}
	if(out.ambiguous.size() > 1) {
		out.kind = TopicResolve::Kind::Ambiguous;
		return out;
	}
	out.kind = TopicResolve::Kind::Unknown;
	return out;
}

void configure_usage(char_data* ch) {
	send_to_char(
		"$c0014=== configure ===$c0007\n\r"
		"Topic di balance/listino e orari di reboot.\n\r"
		"I topic si possono abbreviare (prefisso univoco).\n\r"
		"Valori interi: punti migliaia o suffisso k/m (es. 300k, 2m, -1).\n\r"
		"\n\r"
		"  $c0014configure dimensione densita|premi|livelli|kit|scorta$c0007\n\r"
		"  $c0014configure dispel$c0007\n\r"
		"  $c0014configure editpool$c0007\n\r"
		"  $c0014configure derent$c0007\n\r"
		"  $c0014configure disincanta$c0007\n\r"
		"  $c0014configure vault$c0007\n\r"
		"  $c0014configure reboot$c0007\n\r"
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

void configure_ambiguous(char_data* ch, const std::vector<const char*>& names) {
	std::ostringstream os;
	os << "Topic ambiguo. Specifica meglio: ";
	for(std::size_t i = 0; i < names.size(); ++i) {
		if(i != 0) {
			os << ", ";
		}
		os << "$c0014" << names[i] << "$c0007";
	}
	os << ".\n\r";
	send_to_char(os.str().c_str(), ch);
}

} // namespace

bool configure_parse_ll(std::string_view s, long long& out) {
	const std::optional<long long> parsed = parse_amount(s);
	if(!parsed.has_value()) {
		return false;
	}
	out = *parsed;
	return true;
}

bool configure_parse_long(std::string_view s, long& out) {
	long long ll = 0;
	if(!configure_parse_ll(s, ll)) {
		return false;
	}
	if(ll < static_cast<long long>(std::numeric_limits<long>::min()) ||
	   ll > static_cast<long long>(std::numeric_limits<long>::max())) {
		return false;
	}
	out = static_cast<long>(ll);
	return true;
}

bool configure_parse_int(std::string_view s, int& out) {
	long long ll = 0;
	if(!configure_parse_ll(s, ll)) {
		return false;
	}
	if(ll < static_cast<long long>(std::numeric_limits<int>::min()) ||
	   ll > static_cast<long long>(std::numeric_limits<int>::max())) {
		return false;
	}
	out = static_cast<int>(ll);
	return true;
}

void configure_tell_bad_amount(char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	send_to_char(
		"Valore non valido (cifre, punti migliaia, suffisso k/m, segno opzionale).\n\r",
		ch);
}

std::string cfg_k(std::string_view key) {
	return std::string("$c0015") + std::string(key) + "$c0007";
}

std::string cfg_v(std::string_view val) {
	return std::string("$c0011") + std::string(val) + "$c0007";
}

std::string cfg_v(long long n) {
	return std::string("$c0011") + format_thousands(n) + "$c0007";
}

std::string cfg_kv(std::string_view key, std::string_view val) {
	return cfg_k(key) + "=" + cfg_v(val);
}

std::string cfg_kv(std::string_view key, long long n) {
	return cfg_k(key) + "=" + cfg_v(n);
}

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

	const TopicResolve resolved = resolve_topic(topic);
	if(resolved.kind == TopicResolve::Kind::Ambiguous) {
		configure_ambiguous(ch, resolved.ambiguous);
		return;
	}
	if(resolved.kind == TopicResolve::Kind::Unknown) {
		send_to_char("Topic sconosciuto. Digita $c0014configure$c0007 per l'elenco.\n\r",
					 ch);
		return;
	}

	const std::string rest_str(rest);

	switch(resolved.id) {
	case ConfigTopic::Dimensione: {
		const ArgSplit sub = split_arg(rest);
		if(sub.first.empty()) {
			configure_dimensione_menu(ch);
			return;
		}
		const std::string subcmd(sub.first);
		const std::string sub_rest(sub.rest);
		if(!procarea_try_balance_wiz_command(ch, subcmd.c_str(), sub_rest.c_str())) {
			configure_dimensione_menu(ch);
		}
		return;
	}
	case ConfigTopic::Dispel:
		do_dispelconf(ch, rest_str.c_str(), CMD_DISPELCONF);
		return;
	case ConfigTopic::Vault:
		do_vaultplan(ch, rest_str.c_str(), CMD_VAULTPLAN);
		return;
	case ConfigTopic::Reboot:
		do_reboottime(ch, rest_str.c_str(), CMD_REBOOTTIME);
		return;
	case ConfigTopic::Editpool:
		edit_pool_balance_wiz(ch, rest_str.c_str());
		return;
	case ConfigTopic::Derent:
		derent_balance_wiz(ch, rest_str.c_str());
		return;
	case ConfigTopic::Disincanta:
		disincanta_balance_wiz(ch, rest_str.c_str());
		return;
	}
}

} // namespace Alarmud
