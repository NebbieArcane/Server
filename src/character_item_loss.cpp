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
#include <sstream>
#include <string>
#include <string_view>
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
	return truncate_utf8_bytes(obj->short_description, 128);
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

void character_item_loss_show(char_data* ch, std::string_view name, int days) {
	if(ch == nullptr) {
		return;
	}
	if(name.empty()) {
		send_to_char("Uso: show loss <nome> [giorni]\n\r", ch);
		return;
	}
	days = std::clamp(days, 1, kItemLossShowMaxDays);

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

		std::sort(rows.begin(), rows.end(),
				  [](const character_item_loss& a, const character_item_loss& b) {
					  return a.at > b.at;
				  });

		std::ostringstream out;
		out << "$c0014Perdite oggetti$c0007 per $c0015" << display_name << "$c0007 (ultimi "
			<< days << " giorni):\n\r";
		if(rows.empty()) {
			out << "(nessuna perdita registrata)\n\r";
		}
		else {
			constexpr std::size_t kMaxRows = 200;
			const std::size_t limit = std::min(rows.size(), kMaxRows);
			for(std::size_t i = 0; i < limit; ++i) {
				const character_item_loss& row = rows[i];
				out << "$c0008" << boost::posix_time::to_simple_string(row.at) << "$c0007 "
					<< "$c0011" << row.kind << "$c0007 ";
				out << (row.short_desc.empty() ? "(senza desc)" : row.short_desc);
				out << " vnum=" << row.item_number;
				if(!row.instance_id.null()) {
					out << " inst=" << row.instance_id.get();
				}
				if(!row.room_vnum.null()) {
					out << " room=" << row.room_vnum.get();
				}
				if(!row.detail.null() && !row.detail.get().empty()) {
					out << " [" << row.detail.get() << "]";
				}
				out << "\n\r";
			}
			if(rows.size() > kMaxRows) {
				out << "... (troncato a 200 righe)\n\r";
			}
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
