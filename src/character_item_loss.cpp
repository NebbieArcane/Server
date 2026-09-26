/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* character_item_loss.cpp - audit perdite oggetti dal PG. */
#include "character_item_loss.hpp"

#include "structs.hpp"
#include "utils.hpp"

#include <string>

namespace Alarmud {

std::string item_loss_pc_name(const char_data* ch) {
	if(ch == nullptr) {
		return "?";
	}
	const char* name = GET_NAME(ch);
	return (name != nullptr && name[0] != '\0') ? std::string(name) : "?";
}

} // namespace Alarmud

#if USE_MYSQL

#include "Sql.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "logging.hpp"
#include "modify.hpp"
#include "odb/account-odb.hxx"
#include "toon_migration.hpp"
#include "utility.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/mysql/database.hxx>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Alarmud {
namespace {

using toonQuery = odb::query<toon>;
using LossQ = odb::query<character_item_loss>;

[[nodiscard]] std::string ascii_lower(std::string_view in) {
	std::string out(in);
	std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return out;
}

[[nodiscard]] std::string truncate_utf8_bytes(std::string s, std::size_t maxn) {
	if(s.size() > maxn) {
		s.resize(maxn);
	}
	return s;
}

[[nodiscard]] char_data* loss_real_pc(char_data* ch) {
	if(ch == nullptr) {
		return nullptr;
	}
	if(IS_POLY(ch) && ch->desc != nullptr && ch->desc->original != nullptr) {
		return ch->desc->original;
	}
	return ch;
}

[[nodiscard]] unsigned int loss_item_vnum(const obj_data* obj) {
	if(obj == nullptr || obj->item_number < 0) {
		return 0;
	}
	return static_cast<unsigned int>(obj_index[obj->item_number].iVNum);
}

[[nodiscard]] std::string loss_short_desc(const obj_data* obj) {
	if(obj == nullptr || obj->short_description == nullptr) {
		return {};
	}
	std::string desc = obj->short_description;
	if(GET_ITEM_TYPE(obj) == ITEM_MONEY) {
		const int amount = obj->obj_flags.value[0];
		desc += " (";
		desc += std::to_string(amount);
		desc += ")";
	}
	return truncate_utf8_bytes(std::move(desc), 128);
}

void fill_loss_row(character_item_loss& row, char_data* loser, obj_data* obj,
				   std::string_view kind, std::string_view detail, unsigned long long toon_id) {
	row = character_item_loss{};
	row.at = boost::posix_time::second_clock::local_time();
	row.toon_id = toon_id;
	if(loser != nullptr) {
		row.toon_name = item_loss_pc_name(loser);
	}
	row.kind = kind.empty() ? std::string(kItemLossOther) : std::string(kind);
	row.item_number = loss_item_vnum(obj);
	if(obj != nullptr && obj->db_instance_id != 0) {
		row.instance_id = obj->db_instance_id;
	}
	row.short_desc = loss_short_desc(obj);
	if(loser != nullptr && loser->in_room != NOWHERE) {
		row.room_vnum = static_cast<long long>(loser->in_room);
	}
	if(!detail.empty()) {
		row.detail = truncate_utf8_bytes(std::string(detail), 256);
	}
}

[[nodiscard]] unsigned long long resolve_toon_id(DB* db, std::string_view name) {
	const std::string key(name);
	for(const toon& row : db->query<toon>(toonQuery::name == key)) {
		return row.id;
	}
	return 0;
}

[[nodiscard]] bool can_log_loss(char_data* loser) {
	loser = loss_real_pc(loser);
	if(loser == nullptr || !IS_PC(loser)) {
		return false;
	}
	if(IS_IMMORTAL(loser)) {
		return false;
	}
	const std::string name = item_loss_pc_name(loser);
	if(name == "?" || name.empty()) {
		return false;
	}
	return toon_is_migrated_by_name(name.c_str());
}

} // namespace

void character_item_loss_log(char_data* loser, obj_data* obj, std::string_view kind,
							 std::string_view detail) {
	loser = loss_real_pc(loser);
	if(!can_log_loss(loser) || obj == nullptr) {
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		const unsigned long long toon_id = resolve_toon_id(db, item_loss_pc_name(loser));
		if(toon_id == 0) {
			t.commit();
			return;
		}
		character_item_loss row {};
		fill_loss_row(row, loser, obj, kind, detail, toon_id);
		db->persist(row);
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "character_item_loss_log: %s", e.what());
	}
}

void character_item_loss_log_list(char_data* loser, const std::vector<obj_data*>& objs,
								  std::string_view kind, std::string_view detail) {
	loser = loss_real_pc(loser);
	if(!can_log_loss(loser) || objs.empty()) {
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		const unsigned long long toon_id = resolve_toon_id(db, item_loss_pc_name(loser));
		if(toon_id == 0) {
			t.commit();
			return;
		}
		for(obj_data* obj : objs) {
			if(obj == nullptr) {
				continue;
			}
			character_item_loss row {};
			fill_loss_row(row, loser, obj, kind, detail, toon_id);
			db->persist(row);
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "character_item_loss_log_list: %s", e.what());
	}
}

namespace {

struct LossEventSummary {
	boost::posix_time::ptime at;
	std::string kind;
	std::size_t count = 0;
	std::size_t types = 0;
	odb::nullable<long long> room_vnum;
	std::string single_desc;
	unsigned int single_vnum = 0;
};

struct LossAggKey {
	boost::posix_time::ptime at;
	std::string kind;
	unsigned int item_number = 0;
	std::string short_desc;
	bool operator<(const LossAggKey& o) const {
		if(at != o.at) {
			return at > o.at;
		}
		if(kind != o.kind) {
			return kind < o.kind;
		}
		if(item_number != o.item_number) {
			return item_number < o.item_number;
		}
		return short_desc < o.short_desc;
	}
};

struct LossAggRow {
	LossAggKey key;
	std::size_t count = 0;
	odb::nullable<unsigned long long> instance_id;
	odb::nullable<long long> room_vnum;
	odb::nullable<std::string> detail;
};

[[nodiscard]] bool at_matches_filter(const boost::posix_time::ptime& at,
									 std::string_view filter) {
	if(filter.empty()) {
		return true;
	}
	const std::string stamp = boost::posix_time::to_simple_string(at);
	return stamp.find(filter) != std::string::npos;
}

[[nodiscard]] std::vector<LossEventSummary>
build_loss_event_summaries(const std::vector<character_item_loss>& rows) {
	std::map<std::pair<boost::posix_time::ptime, std::string>, std::vector<const character_item_loss*>>
		groups;
	for(const character_item_loss& row : rows) {
		groups[{row.at, row.kind}].push_back(&row);
	}
	std::vector<LossEventSummary> out;
	out.reserve(groups.size());
	for(const auto& kv : groups) {
		LossEventSummary ev;
		ev.at = kv.first.first;
		ev.kind = kv.first.second;
		ev.count = kv.second.size();
		std::set<std::pair<unsigned int, std::string>> type_keys;
		for(const character_item_loss* r : kv.second) {
			type_keys.insert({r->item_number, r->short_desc});
			if(ev.room_vnum.null() && !r->room_vnum.null()) {
				ev.room_vnum = r->room_vnum;
			}
		}
		ev.types = type_keys.size();
		if(ev.count == 1 && !kv.second.empty()) {
			ev.single_desc = kv.second.front()->short_desc;
			ev.single_vnum = kv.second.front()->item_number;
		}
		out.push_back(std::move(ev));
	}
	std::sort(out.begin(), out.end(),
			  [](const LossEventSummary& a, const LossEventSummary& b) {
				  if(a.at != b.at) {
					  return a.at > b.at;
				  }
				  return a.kind < b.kind;
			  });
	return out;
}

[[nodiscard]] std::vector<LossAggRow>
build_loss_agg_rows(const std::vector<character_item_loss>& rows) {
	std::map<LossAggKey, LossAggRow> agg;
	for(const character_item_loss& row : rows) {
		LossAggKey key {row.at, row.kind, row.item_number, row.short_desc};
		LossAggRow& slot = agg[key];
		if(slot.count == 0) {
			slot.key = key;
			slot.instance_id = row.instance_id;
			slot.room_vnum = row.room_vnum;
			slot.detail = row.detail;
		}
		else {
			/* piu' pezzi: niente instance/detail singoli */
			slot.instance_id = odb::nullable<unsigned long long>();
			if(!slot.detail.null() &&
			   (row.detail.null() || slot.detail.get() != row.detail.get())) {
				slot.detail = odb::nullable<std::string>();
			}
		}
		++slot.count;
	}
	std::vector<LossAggRow> out;
	out.reserve(agg.size());
	for(auto& kv : agg) {
		out.push_back(std::move(kv.second));
	}
	std::sort(out.begin(), out.end(),
			  [](const LossAggRow& a, const LossAggRow& b) {
				  if(a.key.at != b.key.at) {
					  return a.key.at > b.key.at;
				  }
				  if(a.key.kind != b.key.kind) {
					  return a.key.kind < b.key.kind;
				  }
				  if(a.key.item_number != b.key.item_number) {
					  return a.key.item_number < b.key.item_number;
				  }
				  return a.key.short_desc < b.key.short_desc;
			  });
	return out;
}

void format_loss_summary(std::ostringstream& out, const std::vector<LossEventSummary>& events) {
	constexpr std::size_t kMaxEvents = 200;
	const std::size_t limit = std::min(events.size(), kMaxEvents);
	for(std::size_t i = 0; i < limit; ++i) {
		const LossEventSummary& ev = events[i];
		out << "$c0008" << boost::posix_time::to_simple_string(ev.at) << "$c0007 "
			<< "$c0011" << ev.kind << "$c0007 ";
		if(ev.count == 1) {
			out << (ev.single_desc.empty() ? "(senza desc)" : ev.single_desc);
			out << " vnum=" << ev.single_vnum;
		}
		else {
			out << ev.count << " obj (" << ev.types
				<< (ev.types == 1 ? " tipo)" : " tipi)");
		}
		if(!ev.room_vnum.null()) {
			out << " room=" << ev.room_vnum.get();
		}
		out << "\n\r";
	}
	if(events.size() > kMaxEvents) {
		out << "... (troncato a " << kMaxEvents << " eventi; restringi i giorni)\n\r";
	}
}

void format_loss_detail(std::ostringstream& out, const std::vector<LossAggRow>& aggs) {
	constexpr std::size_t kMaxLines = 1000;
	const std::size_t limit = std::min(aggs.size(), kMaxLines);
	for(std::size_t i = 0; i < limit; ++i) {
		const LossAggRow& row = aggs[i];
		out << "$c0008" << boost::posix_time::to_simple_string(row.key.at) << "$c0007 "
			<< "$c0011" << row.key.kind << "$c0007 ";
		if(row.count > 1) {
			out << "x" << row.count << " ";
		}
		out << (row.key.short_desc.empty() ? "(senza desc)" : row.key.short_desc);
		out << " vnum=" << row.key.item_number;
		if(row.count == 1 && !row.instance_id.null()) {
			out << " inst=" << row.instance_id.get();
		}
		if(!row.room_vnum.null()) {
			out << " room=" << row.room_vnum.get();
		}
		if(row.count == 1 && !row.detail.null() && !row.detail.get().empty()) {
			out << " [" << row.detail.get() << "]";
		}
		out << "\n\r";
	}
	if(aggs.size() > kMaxLines) {
		out << "... (troncato a " << kMaxLines << " righe aggregate)\n\r";
	}
}

} // namespace

void character_item_loss_show(char_data* ch, std::string_view name,
							  const ItemLossShowOpts& opts) {
	if(ch == nullptr) {
		return;
	}
	if(name.empty()) {
		send_to_char(
			"Uso: show loss <nome> [detail|death] [giorni|orario]\n\r", ch);
		return;
	}
	const int days = std::clamp(opts.days, 1, kItemLossShowMaxDays);

	DB* db = Sql::getMysql();
	if(db == nullptr) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return;
	}

	try {
		odb::transaction t(db->begin());
		const std::string want = ascii_lower(name);

		unsigned long long toon_id = 0;
		std::string display_name;
		for(const toon& row : db->query<toon>()) {
			if(ascii_lower(row.name) == want) {
				toon_id = row.id;
				display_name = row.name;
				break;
			}
		}
		if(toon_id == 0) {
			t.commit();
			send_to_char("Nessun toon con quel nome.\n\r", ch);
			return;
		}

		const boost::posix_time::ptime since =
			boost::posix_time::second_clock::local_time() -
			boost::posix_time::hours(24 * days);

		std::vector<character_item_loss> rows;
		for(const character_item_loss& row :
			db->query<character_item_loss>((LossQ::toon_id == toon_id) && (LossQ::at >= since))) {
			rows.push_back(row);
		}
		t.commit();

		if(!opts.at_filter.empty()) {
			rows.erase(std::remove_if(rows.begin(), rows.end(),
									  [&](const character_item_loss& r) {
										  return !at_matches_filter(r.at, opts.at_filter);
									  }),
					   rows.end());
		}

		std::ostringstream out;
		out << "$c0014Perdite oggetti$c0007 per $c0015" << display_name << "$c0007 (ultimi "
			<< days << " giorni";
		if(opts.view == ItemLossShowView::Detail) {
			out << ", dettaglio";
		}
		else if(opts.view == ItemLossShowView::Death) {
			out << ", morte";
		}
		if(!opts.at_filter.empty()) {
			out << ", at~" << opts.at_filter;
		}
		out << "):\n\r";

		if(rows.empty()) {
			out << "(nessuna perdita registrata)\n\r";
			const std::string page = out.str();
			page_string(ch->desc, page.c_str(), 1);
			return;
		}

		if(opts.view == ItemLossShowView::Summary) {
			const std::vector<LossEventSummary> events = build_loss_event_summaries(rows);
			format_loss_summary(out, events);
			out << "$c0008(detail = pezzi aggregati; death = ultima/filtrata DEATH_CORPSE)$c0007\n\r";
		}
		else if(opts.view == ItemLossShowView::Death) {
			std::vector<character_item_loss> deaths;
			deaths.reserve(rows.size());
			for(const character_item_loss& r : rows) {
				if(r.kind == kItemLossDeathCorpse) {
					deaths.push_back(r);
				}
			}
			if(deaths.empty()) {
				out << "(nessun DEATH_CORPSE in finestra)\n\r";
			}
			else {
				boost::posix_time::ptime target = deaths.front().at;
				for(const character_item_loss& r : deaths) {
					if(r.at > target) {
						target = r.at;
					}
				}
				/* Senza filtro at: solo l'evento morte piu' recente. */
				if(opts.at_filter.empty()) {
					deaths.erase(std::remove_if(deaths.begin(), deaths.end(),
												[&](const character_item_loss& r) {
													return r.at != target;
												}),
								 deaths.end());
					out << "$c0008Evento$c0007 "
						<< boost::posix_time::to_simple_string(target) << "\n\r";
				}
				format_loss_detail(out, build_loss_agg_rows(deaths));
			}
		}
		else {
			format_loss_detail(out, build_loss_agg_rows(rows));
		}

		const std::string page = out.str();
		page_string(ch->desc, page.c_str(), 1);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "character_item_loss_show: %s", e.what());
		send_to_char("Errore lettura character_item_loss.\n\r", ch);
	}
}

void character_item_loss_purge_old() {
	if(ITEM_LOSS_RETENTION_HOURS <= 0) {
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		const boost::posix_time::ptime since =
			boost::posix_time::second_clock::local_time() -
			boost::posix_time::hours(ITEM_LOSS_RETENTION_HOURS);
		odb::transaction t(db->begin());
		const unsigned long long n =
			db->erase_query<character_item_loss>(LossQ::at < since);
		t.commit();
		if(n > 0) {
			mudlog(LOG_CHECK,
				   "character_item_loss: purged %llu rows older than %d hours",
				   n, ITEM_LOSS_RETENTION_HOURS);
		}
		else {
			mudlog(LOG_CHECK,
				   "character_item_loss: purge ok (0 rows older than %d hours)",
				   ITEM_LOSS_RETENTION_HOURS);
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "character_item_loss_purge_old: %s", e.what());
	}
}

} // namespace Alarmud

#endif /* USE_MYSQL */
