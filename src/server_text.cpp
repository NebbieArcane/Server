/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* server_text.cpp - news, wiznews, motd, wizmotd from MySQL.
 * */
/***************************  General include ************************************/
#include "config.hpp"
#include "flags.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "server_text.hpp"
#include "act.info.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "interpreter.hpp"
#include "modify.hpp"
#include "snew.hpp"
#include "Sql.hpp"
#include "utility.hpp"
#include "version.hpp"

#if USE_MYSQL
/***************************  System  include ************************************/
#include <mysql/mysql.h>
#include <odb/database.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/transaction.hxx>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
namespace Alarmud {
namespace {

struct ServerTextRow {
  unsigned long long id = 0;
  ServerTextKind kind = ServerTextKind::news;
  ServerTextComponent component = ServerTextComponent::general;
  std::string headline;
  std::string version_str;
  std::string body_long;
  bool has_body_long = false;
  int sort_key = 0;
  int day = 0;
  int month = 0;
  int year = 0;
  bool has_date = false;
  std::string author;
};

struct BodyWritePending {
  unsigned long long entry_id = 0;
  ServerTextKind kind = ServerTextKind::news;
  char* buffer = nullptr;
};

std::vector<ServerTextRow> g_news_rows;
std::vector<ServerTextRow> g_wiznews_rows;
std::unordered_map<struct descriptor_data*, BodyWritePending> g_body_pending;
bool g_table_ready = false;

const char* kCreateServerTextTable =
  "CREATE TABLE IF NOT EXISTS server_text_entry ("
  "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
  "kind TINYINT UNSIGNED NOT NULL,"
  "component TINYINT UNSIGNED NOT NULL DEFAULT 0,"
  "headline TEXT NOT NULL,"
  "version_str VARCHAR(96) NULL,"
  "body_long TEXT NULL,"
  "entry_date DATE NULL,"
  "sort_key INT NOT NULL DEFAULT 0,"
  "active TINYINT(1) NOT NULL DEFAULT 1,"
  "author VARCHAR(32) NOT NULL DEFAULT '',"
  "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
  "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
  "PRIMARY KEY (id),"
  "KEY kind_active_sort_i (kind, active, sort_key),"
  "KEY kind_active_component_i (kind, active, component)"
  ") ENGINE=InnoDB";

std::string st_sql_escape(const char* s) {
  if(!s) {
    return "";
  }
  std::string out;
  out.reserve(std::strlen(s) * 2 + 4);
  for(const char* p = s; *p; ++p) {
    if(*p == '\'' || *p == '\\') {
      out.push_back('\\');
    }
    out.push_back(*p);
  }
  return out;
}

std::string st_sql_literal(const char* s) {
  if(!s || !*s) {
    return "''";
  }
  return "'" + st_sql_escape(s) + "'";
}

bool st_mysql_exec(odb::database* db, const std::string& sql) {
  if(!db) {
    return false;
  }
  try {
    odb::transaction t(db->begin());
    t.tracer(logTracer);
    db->execute(sql.c_str());
    t.commit();
    return true;
  }
  catch(const odb::exception& e) {
    mudlog(LOG_SYSERR, "server_text mysql exec: %s", e.what());
    return false;
  }
}

bool st_mysql_query(odb::database* db, const std::string& sql, MYSQL_RES*& out_res) {
  out_res = nullptr;
  if(!db) {
    return false;
  }
  try {
    odb::connection_ptr cp(db->connection());
    auto& mc = static_cast<odb::mysql::connection&>(*cp);
    MYSQL* h = mc.handle();
    if(mysql_query(h, sql.c_str()) != 0) {
      mudlog(LOG_SYSERR, "server_text query: %s", mysql_error(h));
      return false;
    }
    out_res = mysql_store_result(h);
    return true;
  }
  catch(const odb::exception& e) {
    mudlog(LOG_SYSERR, "server_text query: odb (will retry): %s", e.what());
    try {
      odb::connection_ptr cp(db->connection());
      auto& mc = static_cast<odb::mysql::connection&>(*cp);
      MYSQL* h = mc.handle();
      if(mysql_query(h, sql.c_str()) != 0) {
        mudlog(LOG_SYSERR, "server_text query: %s", mysql_error(h));
        return false;
      }
      out_res = mysql_store_result(h);
      return true;
    }
    catch(const odb::exception& e2) {
      mudlog(LOG_SYSERR, "server_text query: odb (giving up): %s", e2.what());
      return false;
    }
  }
}

unsigned long long st_mysql_scalar(odb::database* db, const std::string& sql) {
  MYSQL_RES* res = nullptr;
  if(!st_mysql_query(db, sql, res)) {
    return 0;
  }
  unsigned long long n = 0;
  if(res) {
    if(MYSQL_ROW row = mysql_fetch_row(res)) {
      if(row[0]) {
        n = std::strtoull(row[0], nullptr, 10);
      }
    }
    mysql_free_result(res);
  }
  return n;
}

bool st_table_exists(odb::database* db) {
  return st_mysql_scalar(db,
                         "SELECT COUNT(*) FROM information_schema.TABLES "
                         "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = "
                         "'server_text_entry'") > 0;
}

bool st_column_exists(odb::database* db, const char* column) {
  if(!column || !*column) {
    return false;
  }
  std::ostringstream sql;
  sql << "SELECT COUNT(*) FROM information_schema.COLUMNS "
         "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'server_text_entry' AND "
         "COLUMN_NAME="
      << st_sql_literal(column);
  return st_mysql_scalar(db, sql.str()) > 0;
}

void st_ensure_columns(odb::database* db) {
  if(!db) {
    return;
  }
  if(!st_column_exists(db, "component")) {
    st_mysql_exec(db,
                  "ALTER TABLE server_text_entry ADD COLUMN component TINYINT UNSIGNED NOT "
                  "NULL DEFAULT 0 AFTER kind");
  }
  if(!st_column_exists(db, "version_str")) {
    st_mysql_exec(db,
                  "ALTER TABLE server_text_entry ADD COLUMN version_str VARCHAR(96) NULL "
                  "AFTER headline");
  }
  else {
    const unsigned long long width = st_mysql_scalar(
      db, "SELECT CHARACTER_MAXIMUM_LENGTH FROM information_schema.COLUMNS "
          "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'server_text_entry' "
          "AND COLUMN_NAME = 'version_str'");
    if(width > 0 && width < 96) {
      st_mysql_exec(db,
                    "ALTER TABLE server_text_entry MODIFY COLUMN version_str VARCHAR(96) NULL");
    }
  }
}

bool st_ensure_table(odb::database* db) {
  if(g_table_ready) {
    return true;
  }
  if(!db) {
    return false;
  }
  if(!st_mysql_exec(db, kCreateServerTextTable)) {
    return false;
  }
  st_ensure_columns(db);
  g_table_ready = st_table_exists(db);
  return g_table_ready;
}

const char* st_component_label(ServerTextComponent component) {
  switch(component) {
  case ServerTextComponent::server:
    return "Server";
  case ServerTextComponent::world:
    return "World";
  default:
    return "";
  }
}

bool st_iequals_prefix(const char* text, const char* word, std::size_t len) {
  if(!text || !word) {
    return false;
  }
  for(std::size_t i = 0; i < len; ++i) {
    if(std::tolower(static_cast<unsigned char>(text[i])) !=
       std::tolower(static_cast<unsigned char>(word[i]))) {
      return false;
    }
  }
  return true;
}

void st_skip_ansi_codes(const char*& p) {
  while(p && *p == '$' && p[1] == 'c') {
    p += 6;
    while(*p == ' ') {
      ++p;
    }
  }
}

bool st_parse_component_token(const char* token, ServerTextComponent& out) {
  if(!token || !*token) {
    return false;
  }
  if(!str_cmp(token, "server") || !str_cmp(token, "codice")) {
    out = ServerTextComponent::server;
    return true;
  }
  if(!str_cmp(token, "world") || !str_cmp(token, "aree")) {
    out = ServerTextComponent::world;
    return true;
  }
  if(!str_cmp(token, "general") || !str_cmp(token, "generale")) {
    out = ServerTextComponent::general;
    return true;
  }
  return false;
}

bool st_looks_like_version(const char* token) {
  if(!token || !*token) {
    return false;
  }
  bool saw_digit = false;
  for(const char* p = token; *p; ++p) {
    if(std::isdigit(static_cast<unsigned char>(*p))) {
      saw_digit = true;
      continue;
    }
    if(*p == '.' || *p == '-' || *p == '_') {
      continue;
    }
    if(*p == 'r' && p == token) {
      continue;
    }
    return false;
  }
  return saw_digit;
}

bool st_parse_component_version_headline(const char* text, ServerTextComponent& component,
                                         std::string& version, std::string& plain_headline) {
  component = ServerTextComponent::general;
  version.clear();
  plain_headline.clear();
  if(!text) {
    return false;
  }
  const char* scan = text;
  st_skip_ansi_codes(scan);
  if(st_iequals_prefix(scan, "Server", 6)) {
    component = ServerTextComponent::server;
    scan += 6;
  }
  else if(st_iequals_prefix(scan, "World", 5)) {
    component = ServerTextComponent::world;
    scan += 5;
  }
  else {
    plain_headline = text;
    return true;
  }
  st_skip_ansi_codes(scan);
  while(*scan == ' ') {
    ++scan;
  }
  char version_token[MAX_INPUT_LENGTH];
  const char* tail = one_argument(scan, version_token);
  if(st_looks_like_version(version_token)) {
    version = version_token;
    scan = tail;
    while(*scan == ' ') {
      ++scan;
    }
  }
  plain_headline = scan ? scan : "";
  return true;
}

bool st_parse_add_tokens(const char* text, ServerTextComponent& component, std::string& version,
                         std::string& plain_headline) {
  component = ServerTextComponent::general;
  version.clear();
  plain_headline.clear();
  if(!text || !*text) {
    return false;
  }
  char buf[MAX_INPUT_LENGTH];
  std::strncpy(buf, text, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  char token[MAX_INPUT_LENGTH];
  const char* tail = one_argument(buf, token);
  ServerTextComponent parsed = ServerTextComponent::general;
  if(!st_parse_component_token(token, parsed)) {
    plain_headline = text;
    return true;
  }
  component = parsed;
  if(!tail || !*tail) {
    return true;
  }
  while(*tail == ' ') {
    ++tail;
  }
  char rest_buf[MAX_INPUT_LENGTH];
  std::strncpy(rest_buf, tail, sizeof(rest_buf) - 1);
  rest_buf[sizeof(rest_buf) - 1] = '\0';
  char version_token[MAX_INPUT_LENGTH];
  const char* after = one_argument(rest_buf, version_token);
  if(st_looks_like_version(version_token)) {
    version = version_token;
    plain_headline = after ? after : "";
  }
  else {
    plain_headline = tail;
  }
  while(!plain_headline.empty() && plain_headline[0] == ' ') {
    plain_headline.erase(0, 1);
  }
  return true;
}

constexpr int kComponentLabelWidth = 10;
constexpr int kVersionColumnWidth = 8;

const char* st_version_block_header() {
  return "$c0015Versioni correnti:$c0007\n\r$c0011---------------------------------$c0007\n\r";
}

void st_append_fixed_padding(std::ostringstream& oss, int width, int used) {
  for(int i = used; i < width; ++i) {
    oss << ' ';
  }
}

void st_append_label_padding(std::ostringstream& oss, ServerTextComponent component) {
  const char* label = st_component_label(component);
  const int label_len = label ? static_cast<int>(std::strlen(label)) : 0;
  oss << label;
  st_append_fixed_padding(oss, kComponentLabelWidth, label_len);
}

std::string st_format_version_line(const ServerTextRow& row) {
  if(row.component == ServerTextComponent::general) {
    return row.headline;
  }
  std::ostringstream oss;
  oss << "$c0011";
  st_append_label_padding(oss, row.component);
  oss << "$c0010";
  if(!row.version_str.empty()) {
    oss << row.version_str;
    st_append_fixed_padding(oss, kVersionColumnWidth,
                            static_cast<int>(row.version_str.size()));
    oss << ' ';
  }
  if(!row.headline.empty()) {
    oss << row.headline;
  }
  oss << "$c0007";
  return oss.str();
}

std::string st_format_display_line(const ServerTextRow& row) {
  return st_format_version_line(row);
}

bool st_parse_display_date(const char* line, int& day, int& month, int& year,
                           const char*& headline_out) {
  if(!line || std::strlen(line) < 11) {
    return false;
  }
  if(!std::isdigit(static_cast<unsigned char>(line[0])) ||
     !std::isdigit(static_cast<unsigned char>(line[1])) ||
     line[2] != '/' ||
     !std::isdigit(static_cast<unsigned char>(line[3])) ||
     !std::isdigit(static_cast<unsigned char>(line[4])) ||
     line[5] != '/' ||
     !std::isdigit(static_cast<unsigned char>(line[6])) ||
     !std::isdigit(static_cast<unsigned char>(line[7])) ||
     !std::isdigit(static_cast<unsigned char>(line[8])) ||
     !std::isdigit(static_cast<unsigned char>(line[9]))) {
    return false;
  }
  day = (line[0] - '0') * 10 + (line[1] - '0');
  month = (line[3] - '0') * 10 + (line[4] - '0');
  year = (line[6] - '0') * 1000 + (line[7] - '0') * 100 + (line[8] - '0') * 10 +
         (line[9] - '0');
  if(day < 1 || day > 31 || month < 1 || month > 12) {
    return false;
  }
  const char* rest = line + 10;
  while(*rest == ' ') {
    ++rest;
  }
  if(!*rest) {
    return false;
  }
  headline_out = rest;
  return true;
}

int st_sort_key_from_ymd(int year, int month, int day) {
  return year * 10000 + month * 100 + day;
}

bool st_is_skippable_seed_line(const std::string& line) {
  if(line.empty()) {
    return true;
  }
  if(line.find("Versioni correnti") != std::string::npos) {
    return true;
  }
  if(line.find_first_not_of("- \t\r\n") == std::string::npos) {
    return true;
  }
  return false;
}

bool st_seed_from_file(odb::database* db, ServerTextKind kind, const char* path) {
  std::ifstream in(path);
  if(!in.is_open()) {
    mudlog(LOG_CHECK, "server_text seed: no file %s", path);
    return false;
  }
  std::string line;
  int inserted = 0;
  while(std::getline(in, line)) {
    while(!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
      line.pop_back();
    }
    if(st_is_skippable_seed_line(line)) {
      continue;
    }
    int day = 0;
    int month = 0;
    int year = 0;
    const char* after_date = line.c_str();
    int sort_key = 0;
    std::string entry_date_sql = "NULL";
    if(st_parse_display_date(line.c_str(), day, month, year, after_date)) {
      sort_key = st_sort_key_from_ymd(year, month, day);
      char buf[32];
      std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
      entry_date_sql = std::string("'") + buf + "'";
    }
    else {
      after_date = line.c_str();
      sort_key = static_cast<int>(time(nullptr));
    }
    ServerTextComponent component = ServerTextComponent::general;
    std::string version;
    std::string plain_headline;
    st_parse_component_version_headline(after_date, component, version, plain_headline);
    if(plain_headline.empty()) {
      continue;
    }
    std::ostringstream sql;
    sql << "INSERT INTO server_text_entry (kind, component, headline, version_str, body_long, "
           "entry_date, sort_key, active, author, created_at, updated_at) VALUES ("
        << static_cast<unsigned>(kind) << ',' << static_cast<unsigned>(component) << ','
        << st_sql_literal(plain_headline.c_str()) << ','
        << (version.empty() ? "NULL" : st_sql_literal(version.c_str())) << ",NULL,"
        << entry_date_sql << ',' << sort_key << ",1,'seed',NOW(),NOW())";
    if(st_mysql_exec(db, sql.str())) {
      ++inserted;
    }
  }
  mudlog(LOG_CHECK, "server_text seed: %s kind=%u inserted=%d", path,
         static_cast<unsigned>(kind), inserted);
  return inserted > 0;
}

void st_seed_if_empty(odb::database* db) {
  for(unsigned k = 0; k <= 3; ++k) {
    const auto kind = static_cast<ServerTextKind>(k);
    const unsigned long long count =
      st_mysql_scalar(db, "SELECT COUNT(*) FROM server_text_entry WHERE kind=" +
                               std::to_string(k));
    if(count > 0) {
      continue;
    }
    switch(kind) {
    case ServerTextKind::news:
      st_seed_from_file(db, kind, NEWS_FILE.c_str());
      break;
    case ServerTextKind::wiznews:
      st_seed_from_file(db, kind, WIZNEWS_FILE.c_str());
      break;
    case ServerTextKind::motd:
      st_seed_from_file(db, kind, MOTD_FILE.c_str());
      break;
    case ServerTextKind::wizmotd:
      st_seed_from_file(db, kind, WIZ_MOTD_FILE.c_str());
      break;
    }
  }
}

void st_load_rows(odb::database* db, ServerTextKind kind, std::vector<ServerTextRow>& out) {
  out.clear();
  std::ostringstream sql;
  sql << "SELECT id, component, headline, version_str, body_long, entry_date, sort_key, author "
         "FROM server_text_entry WHERE kind="
      << static_cast<unsigned>(kind) << " AND active=1 ORDER BY sort_key DESC, id DESC";
  MYSQL_RES* res = nullptr;
  if(!st_mysql_query(db, sql.str(), res) || !res) {
    return;
  }
  MYSQL_ROW row;
  while((row = mysql_fetch_row(res))) {
    ServerTextRow r;
    r.kind = kind;
    if(row[0]) {
      r.id = std::strtoull(row[0], nullptr, 10);
    }
    if(row[1]) {
      r.component = static_cast<ServerTextComponent>(std::atoi(row[1]));
    }
    if(row[2]) {
      r.headline = row[2];
    }
    if(row[3] && row[3][0]) {
      r.version_str = row[3];
    }
    if(row[4] && row[4][0]) {
      r.body_long = row[4];
      r.has_body_long = true;
    }
    if(row[5] && row[5][0]) {
      int y = 0;
      int m = 0;
      int d = 0;
      if(std::sscanf(row[5], "%d-%d-%d", &y, &m, &d) == 3) {
        r.year = y;
        r.month = m;
        r.day = d;
        r.has_date = true;
      }
    }
    if(row[6]) {
      r.sort_key = std::atoi(row[6]);
    }
    if(row[7]) {
      r.author = row[7];
    }
    out.push_back(std::move(r));
  }
  mysql_free_result(res);
}

void st_copy_truncated(char* dest, const std::string& src) {
  if(!dest) {
    return;
  }
  const std::size_t max_len = MAX_STRING_LENGTH - 1;
  if(src.size() > max_len) {
    mudlog(LOG_ERROR, "server_text buffer truncated (len=%zu)", src.size());
    std::memcpy(dest, src.data(), max_len);
    dest[max_len] = '\0';
  }
  else {
    std::memcpy(dest, src.c_str(), src.size() + 1);
  }
}

void st_rebuild_news_buffer(const std::vector<ServerTextRow>& rows, char* dest,
                            ServerTextKind kind, bool numbered_list) {
  std::ostringstream oss;
  if(!rows.empty()) {
    if(kind == ServerTextKind::news) {
      oss << st_version_block_header();
    }
    else if(kind == ServerTextKind::wiznews) {
      oss << "$c0015News immortali:$c0007\n\r$c0011---------------------------------$c0007\n\r";
    }
  }
  const char* read_hint = kind == ServerTextKind::wiznews ? "wiznews read " : "news read ";
  int n = 1;
  for(const ServerTextRow& r : rows) {
    if(numbered_list) {
      oss << " [" << n << ']';
    }
    if(r.has_date) {
      char dbuf[16];
      std::snprintf(dbuf, sizeof(dbuf), " %02d/%02d/%04d ", r.day, r.month, r.year);
      oss << dbuf;
    }
    else if(numbered_list) {
      oss << ' ';
    }
    oss << st_format_display_line(r);
    if(numbered_list && r.has_body_long) {
      oss << "  (" << read_hint << n << ')';
    }
    oss << "\n\n";
    ++n;
  }
  st_copy_truncated(dest, oss.str());
}

std::vector<std::string> st_fetch_motd_slot(odb::database* db, ServerTextKind kind,
                                            ServerTextComponent component) {
  std::vector<std::string> lines;
  std::ostringstream sql;
  sql << "SELECT headline, version_str, body_long, entry_date FROM server_text_entry WHERE kind="
      << static_cast<unsigned>(kind) << " AND component=" << static_cast<unsigned>(component)
      << " AND active=1 ORDER BY sort_key DESC, id DESC LIMIT 1";
  MYSQL_RES* res = nullptr;
  if(!st_mysql_query(db, sql.str(), res) || !res) {
    return lines;
  }
  if(MYSQL_ROW row = mysql_fetch_row(res)) {
    ServerTextRow r;
    r.component = component;
    if(row[0]) {
      r.headline = row[0];
    }
    if(row[1] && row[1][0]) {
      r.version_str = row[1];
    }
    if(row[2] && row[2][0]) {
      r.body_long = row[2];
      r.has_body_long = true;
    }
    if(row[3] && row[3][0]) {
      int y = 0;
      int m = 0;
      int d = 0;
      if(std::sscanf(row[3], "%d-%d-%d", &y, &m, &d) == 3) {
        r.day = d;
        r.month = m;
        r.year = y;
        r.has_date = true;
      }
    }
    std::ostringstream line;
    if(r.has_date) {
      char dbuf[16];
      std::snprintf(dbuf, sizeof(dbuf), "%02d/%02d/%04d ", r.day, r.month, r.year);
      line << dbuf;
    }
    line << st_format_version_line(r);
    lines.push_back(line.str());
    if(r.has_body_long) {
      lines.push_back(r.body_long);
    }
  }
  mysql_free_result(res);
  return lines;
}

void st_append_motd_block(std::ostringstream& wrapped, const std::vector<std::string>& block,
                          bool& first_block, bool section_break) {
  if(block.empty()) {
    return;
  }
  if(!first_block) {
    wrapped << (section_break ? "\n\r\n\r" : "\n\r");
  }
  for(std::size_t i = 0; i < block.size(); ++i) {
    if(i > 0) {
      wrapped << "\n\r";
    }
    wrapped << block[i];
  }
  first_block = false;
}

void st_rebuild_motd_buffer(odb::database* db, ServerTextKind kind, char* dest) {
  const std::vector<std::string> general =
    st_fetch_motd_slot(db, kind, ServerTextComponent::general);
  const std::vector<std::string> server =
    st_fetch_motd_slot(db, kind, ServerTextComponent::server);
  const std::vector<std::string> world =
    st_fetch_motd_slot(db, kind, ServerTextComponent::world);
  if(general.empty() && server.empty() && world.empty()) {
    dest[0] = '\0';
    return;
  }
  std::ostringstream wrapped;
  wrapped << st_version_block_header() << "\n\r";
  bool first_block = true;
  st_append_motd_block(wrapped, general, first_block, false);
  st_append_motd_block(wrapped, server, first_block, true);
  st_append_motd_block(wrapped, world, first_block, false);
  wrapped << "\n\r";
  st_copy_truncated(dest, wrapped.str());
}

void st_rebuild_all_buffers(odb::database* db) {
  st_load_rows(db, ServerTextKind::news, g_news_rows);
  st_load_rows(db, ServerTextKind::wiznews, g_wiznews_rows);
  st_rebuild_news_buffer(g_news_rows, news, ServerTextKind::news, true);
  st_rebuild_news_buffer(g_wiznews_rows, wiznews, ServerTextKind::wiznews, true);
  st_rebuild_motd_buffer(db, ServerTextKind::motd, motd);
  st_rebuild_motd_buffer(db, ServerTextKind::wizmotd, wmotd);
}

const std::vector<ServerTextRow>& st_rows_for_kind(ServerTextKind kind) {
  return kind == ServerTextKind::wiznews ? g_wiznews_rows : g_news_rows;
}

void st_reload_rows_for_kind(odb::database* db, ServerTextKind kind) {
  if(kind == ServerTextKind::wiznews) {
    st_load_rows(db, kind, g_wiznews_rows);
  }
  else {
    st_load_rows(db, kind, g_news_rows);
  }
}

bool st_get_row_by_list_index(ServerTextKind kind, int list_index, const ServerTextRow*& out) {
  const std::vector<ServerTextRow>& rows = st_rows_for_kind(kind);
  if(list_index < 1 || static_cast<std::size_t>(list_index) > rows.size()) {
    return false;
  }
  out = &rows[static_cast<std::size_t>(list_index - 1)];
  return true;
}

void st_show_read(struct char_data* ch, ServerTextKind kind, int list_index) {
  const ServerTextRow* row = nullptr;
  if(!st_get_row_by_list_index(kind, list_index, row)) {
    send_to_char("Non c'e' una news con quel numero.\n\r", ch);
    return;
  }
  if(!row->has_body_long || row->body_long.empty()) {
    send_to_char("Questa news non ha testo esteso.\n\r", ch);
    return;
  }
  std::ostringstream oss;
  oss << "=== News #" << list_index << " ===\n\r";
  if(row->has_date) {
    char dbuf[16];
    std::snprintf(dbuf, sizeof(dbuf), "%02d/%02d/%04d", row->day, row->month, row->year);
    oss << dbuf << " — ";
  }
  if(row->component != ServerTextComponent::general) {
    oss << st_component_label(row->component);
    if(!row->version_str.empty()) {
      oss << ' ' << row->version_str;
    }
    oss << " — ";
  }
  oss << row->headline << "\n\r";
  if(!row->author.empty()) {
    oss << "Autore: " << row->author << "\n\r";
  }
  oss << "\n\r" << row->body_long << "\n\r";
  if(!ch->desc) {
    return;
  }
  page_string(ch->desc, oss.str().c_str(), false);
}

bool st_parse_subcommand(const char* arg, char* sub, char* rest) {
  sub[0] = '\0';
  rest[0] = '\0';
  if(!arg || !*arg) {
    return false;
  }
  char argbuf[MAX_INPUT_LENGTH];
  std::strncpy(argbuf, arg, sizeof(argbuf) - 1);
  argbuf[sizeof(argbuf) - 1] = '\0';
  const char* tail = one_argument(argbuf, sub);
  if(sub[0] == '\0') {
    return false;
  }
  if(tail && *tail) {
    std::strncpy(rest, tail, MAX_INPUT_LENGTH - 1);
    rest[MAX_INPUT_LENGTH - 1] = '\0';
  }
  return true;
}

unsigned long long st_insert_entry(odb::database* db, ServerTextKind kind,
                                   ServerTextComponent component, const char* headline,
                                   const char* version, const char* author, int sort_key,
                                   int day, int month, int year, bool has_date,
                                   const char* body = nullptr) {
  std::string entry_date_sql = "NULL";
  if(has_date) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    entry_date_sql = std::string("'") + buf + "'";
  }
  const char* body_sql = "NULL";
  std::string body_lit;
  if(body && *body) {
    body_lit = st_sql_literal(body);
    body_sql = body_lit.c_str();
  }
  std::ostringstream sql;
  sql << "INSERT INTO server_text_entry (kind, component, headline, version_str, body_long, "
         "entry_date, sort_key, active, author, created_at, updated_at) VALUES ("
      << static_cast<unsigned>(kind) << ',' << static_cast<unsigned>(component) << ','
      << st_sql_literal(headline) << ','
      << (version && *version ? st_sql_literal(version) : "NULL")
      << ',' << body_sql << ',' << entry_date_sql << ',' << sort_key << ",1,"
      << st_sql_literal(author) << ",NOW(),NOW())";
  if(!st_mysql_exec(db, sql.str())) {
    return 0;
  }
  return st_mysql_scalar(db, "SELECT LAST_INSERT_ID()");
}

bool st_deactivate_entry(odb::database* db, unsigned long long id) {
  std::ostringstream sql;
  sql << "UPDATE server_text_entry SET active=0, updated_at=NOW() WHERE id=" << id;
  return st_mysql_exec(db, sql.str());
}

bool st_reactivate_entry(odb::database* db, unsigned long long id) {
  std::ostringstream sql;
  sql << "UPDATE server_text_entry SET active=1, updated_at=NOW() WHERE id=" << id
      << " AND active=0";
  return st_mysql_exec(db, sql.str());
}

bool st_update_body_long(odb::database* db, unsigned long long id, const char* body) {
  std::ostringstream sql;
  sql << "UPDATE server_text_entry SET body_long=" << st_sql_literal(body ? body : "")
      << ", updated_at=NOW() WHERE id=" << id;
  return st_mysql_exec(db, sql.str());
}

bool st_update_author(odb::database* db, unsigned long long id, const char* author) {
  std::ostringstream sql;
  sql << "UPDATE server_text_entry SET author=" << st_sql_literal(author ? author : "")
      << ", updated_at=NOW() WHERE id=" << id;
  return st_mysql_exec(db, sql.str());
}

bool st_replace_motd(odb::database* db, ServerTextKind kind, ServerTextComponent component,
                     const char* version, const char* text, const char* author) {
  const unsigned k = static_cast<unsigned>(kind);
  const unsigned c = static_cast<unsigned>(component);
  std::ostringstream deactivate;
  deactivate << "UPDATE server_text_entry SET active=0, updated_at=NOW() WHERE kind=" << k
             << " AND component=" << c << " AND active=1";
  st_mysql_exec(db, deactivate.str());
  return st_insert_entry(db, kind, component, text, version, author,
                         static_cast<int>(time(nullptr)), 0, 0, 0, false) != 0;
}

void st_admin_list(struct char_data* ch, ServerTextKind kind) {
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    send_to_char("News DB non disponibile.\n\r", ch);
    return;
  }
  st_rebuild_all_buffers(db);
  const std::vector<ServerTextRow>& rows = st_rows_for_kind(kind);
  if(rows.empty()) {
    send_to_char("Nessuna voce.\n\r", ch);
    return;
  }
  int n = 1;
  for(const ServerTextRow& r : rows) {
    std::ostringstream line;
    line << " [" << n << "] id=" << r.id;
    if(r.has_date) {
      line << ' ' << r.day << '/' << r.month << '/' << r.year;
    }
    if(r.component == ServerTextComponent::server) {
      line << " [Server";
    }
    else if(r.component == ServerTextComponent::world) {
      line << " [World";
    }
    if(r.component != ServerTextComponent::general) {
      if(!r.version_str.empty()) {
        line << ' ' << r.version_str;
      }
      line << ']';
    }
    line << ' ' << r.headline;
    if(r.has_body_long) {
      line << " [+body]";
    }
    if(!r.author.empty()) {
      line << " (" << r.author << ')';
    }
    line << "\n\r";
    send_to_char(line.str().c_str(), ch);
    ++n;
  }
}

void st_admin_add(struct char_data* ch, ServerTextKind kind, const char* rest) {
  if(!rest || !*rest) {
    send_to_char("Sintassi: add [gg/mm/aaaa] [server|world] [versione] headline\n\r", ch);
    return;
  }
  int day = 0;
  int month = 0;
  int year = 0;
  int sort_key = static_cast<int>(time(nullptr));
  bool has_date = false;
  char buf[MAX_INPUT_LENGTH];
  std::strncpy(buf, rest, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  char date_token[MAX_INPUT_LENGTH];
  const char* tail = one_argument(buf, date_token);
  const char* parse_from = rest;
  const char* after_date = nullptr;
  if(st_parse_display_date(date_token, day, month, year, after_date)) {
    has_date = true;
    sort_key = st_sort_key_from_ymd(year, month, day);
    parse_from = tail;
    while(*parse_from == ' ') {
      ++parse_from;
    }
  }
  ServerTextComponent component = ServerTextComponent::general;
  std::string version;
  std::string plain_headline;
  st_parse_add_tokens(parse_from, component, version, plain_headline);
  if(plain_headline.empty()) {
    send_to_char("Headline mancante.\n\r", ch);
    return;
  }
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    send_to_char("News DB non disponibile.\n\r", ch);
    return;
  }
  const unsigned long long id = st_insert_entry(
    db, kind, component, plain_headline.c_str(),
    version.empty() ? nullptr : version.c_str(), GET_NAME(ch), sort_key, day, month, year,
    has_date);
  if(!id) {
    send_to_char("Inserimento fallito.\n\r", ch);
    return;
  }
  st_rebuild_all_buffers(db);
  send_to_char("Ok.\n\r", ch);
}

void st_admin_del(struct char_data* ch, ServerTextKind kind, const char* rest) {
  int list_index = 0;
  if(!rest || !*rest || std::sscanf(rest, "%d", &list_index) != 1 || list_index < 1) {
    send_to_char("Sintassi: del <numero in lista>\n\r", ch);
    return;
  }
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    send_to_char("News DB non disponibile.\n\r", ch);
    return;
  }
  st_reload_rows_for_kind(db, kind);
  const ServerTextRow* row = nullptr;
  if(!st_get_row_by_list_index(kind, list_index, row)) {
    send_to_char("Numero non valido.\n\r", ch);
    return;
  }
  if(!st_deactivate_entry(db, row->id)) {
    send_to_char("Eliminazione fallita.\n\r", ch);
    return;
  }
  st_rebuild_all_buffers(db);
  send_to_char("Ok.\n\r", ch);
}

void st_admin_deleted(struct char_data* ch, ServerTextKind kind) {
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    send_to_char("News DB non disponibile.\n\r", ch);
    return;
  }
  std::ostringstream sql;
  sql << "SELECT id, component, headline, version_str, body_long, entry_date, author "
         "FROM server_text_entry WHERE kind="
      << static_cast<unsigned>(kind)
      << " AND active=0 ORDER BY updated_at DESC, id DESC";
  MYSQL_RES* res = nullptr;
  if(!st_mysql_query(db, sql.str(), res) || !res) {
    send_to_char("Nessuna voce eliminata.\n\r", ch);
    return;
  }
  int count = 0;
  MYSQL_ROW row;
  while((row = mysql_fetch_row(res))) {
    ++count;
    std::ostringstream line;
    const unsigned long long id = row[0] ? std::strtoull(row[0], nullptr, 10) : 0;
    line << " id=" << id;
    if(row[5] && row[5][0]) {
      int y = 0;
      int m = 0;
      int d = 0;
      if(std::sscanf(row[5], "%d-%d-%d", &y, &m, &d) == 3) {
        line << ' ' << d << '/' << m << '/' << y;
      }
    }
    const int component = row[1] ? std::atoi(row[1]) : 0;
    if(component == static_cast<int>(ServerTextComponent::server)) {
      line << " [Server";
    }
    else if(component == static_cast<int>(ServerTextComponent::world)) {
      line << " [World";
    }
    if(component != static_cast<int>(ServerTextComponent::general)) {
      if(row[3] && row[3][0]) {
        line << ' ' << row[3];
      }
      line << ']';
    }
    if(row[2]) {
      line << ' ' << row[2];
    }
    if(row[4] && row[4][0]) {
      line << " [+body]";
    }
    if(row[6] && row[6][0]) {
      line << " (" << row[6] << ')';
    }
    line << "\n\r";
    send_to_char(line.str().c_str(), ch);
  }
  mysql_free_result(res);
  if(count == 0) {
    send_to_char("Nessuna voce eliminata.\n\r", ch);
  }
  else {
    const char* cmd = kind == ServerTextKind::wiznews ? "wiznews" : "news";
    char hint[MAX_INPUT_LENGTH];
    std::snprintf(hint, sizeof(hint), "Usa: %s undel <id>\n\r", cmd);
    send_to_char(hint, ch);
  }
}

void st_admin_undel(struct char_data* ch, ServerTextKind kind, const char* rest) {
  if(!rest || !*rest) {
    send_to_char("Sintassi: undel <id>\n\r", ch);
    return;
  }
  char* end = nullptr;
  const unsigned long long id = std::strtoull(rest, &end, 10);
  if(id < 1 || end == rest) {
    send_to_char("Sintassi: undel <id>\n\r", ch);
    return;
  }
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    send_to_char("News DB non disponibile.\n\r", ch);
    return;
  }
  std::ostringstream check_sql;
  check_sql << "SELECT id, active FROM server_text_entry WHERE id=" << id
            << " AND kind=" << static_cast<unsigned>(kind) << " LIMIT 1";
  MYSQL_RES* res = nullptr;
  if(!st_mysql_query(db, check_sql.str(), res) || !res) {
    send_to_char("Id non trovato.\n\r", ch);
    return;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if(!row || !row[0]) {
    mysql_free_result(res);
    send_to_char("Id non trovato per questo comando.\n\r", ch);
    return;
  }
  const int active = row[1] ? std::atoi(row[1]) : 1;
  mysql_free_result(res);
  if(active != 0) {
    send_to_char("Quella voce e' gia' attiva.\n\r", ch);
    return;
  }
  if(!st_reactivate_entry(db, id)) {
    send_to_char("Riattivazione fallita.\n\r", ch);
    return;
  }
  st_rebuild_all_buffers(db);
  send_to_char("Ok, voce riattivata.\n\r", ch);
}

void st_admin_body(struct char_data* ch, ServerTextKind kind, const char* rest) {
  int list_index = 0;
  if(!rest || !*rest || std::sscanf(rest, "%d", &list_index) != 1 || list_index < 1) {
    send_to_char("Sintassi: body <numero in lista>\n\r", ch);
    return;
  }
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    send_to_char("News DB non disponibile.\n\r", ch);
    return;
  }
  st_reload_rows_for_kind(db, kind);
  const ServerTextRow* row = nullptr;
  if(!st_get_row_by_list_index(kind, list_index, row)) {
    send_to_char("Numero non valido.\n\r", ch);
    return;
  }
  if(!ch->desc) {
    return;
  }
  auto pending = BodyWritePending{};
  pending.entry_id = row->id;
  pending.kind = kind;
  pending.buffer = nullptr;
  g_body_pending[ch->desc] = pending;
  ch->desc->str = &g_body_pending[ch->desc].buffer;
  ch->desc->max_str = MAX_STRING_LENGTH - 1;
  send_to_char("Inserisci il testo esteso. Termina con @ su una riga da sola.\n\r", ch);
}

void st_news_syntax(struct char_data* ch, ServerTextKind kind) {
  const char* cmd = kind == ServerTextKind::wiznews ? "wiznews" : "news";
  char line[MAX_INPUT_LENGTH];
  std::snprintf(line, sizeof(line), "Sintassi: %s\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line), "          %s read <numero>\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line), "          %s list\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line),
           "          %s add [gg/mm/aaaa] [server|world] [versione] headline\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line), "          %s del <numero>\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line), "          %s deleted\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line), "          %s undel <id>\n\r", cmd);
  send_to_char(line, ch);
  std::snprintf(line, sizeof(line), "          %s body <numero>\n\r", cmd);
  send_to_char(line, ch);
}

void st_dispatch_player(struct char_data* ch, ServerTextKind kind, const char* arg,
                        char* buffer, const char* cmd_name) {
  char sub[MAX_INPUT_LENGTH];
  char rest[MAX_INPUT_LENGTH];
  if(!st_parse_subcommand(arg, sub, rest)) {
    ShowStaticPagedText(ch, buffer, cmd_name);
    return;
  }
  if(!str_cmp(sub, "read")) {
    int n = 0;
    if(std::sscanf(rest, "%d", &n) != 1 || n < 1) {
      send_to_char(kind == ServerTextKind::wiznews ? "Sintassi: wiznews read <numero>\n\r"
                                                   : "Sintassi: news read <numero>\n\r",
                   ch);
      return;
    }
    st_show_read(ch, kind, n);
    return;
  }
  /* Immortali: argomento sconosciuto → help, non la pagina news. */
  if(GetMaxLevel(ch) >= IMMORTALE) {
    st_news_syntax(ch, kind);
    return;
  }
  ShowStaticPagedText(ch, buffer, cmd_name);
}

void st_dispatch_admin(struct char_data* ch, ServerTextKind kind, const char* arg, char* buffer,
                       const char* cmd_name, int min_level) {
  if(GetMaxLevel(ch) < min_level) {
    st_dispatch_player(ch, kind, arg, buffer, cmd_name);
    return;
  }
  char sub[MAX_INPUT_LENGTH];
  char rest[MAX_INPUT_LENGTH];
  if(!st_parse_subcommand(arg, sub, rest)) {
    ShowStaticPagedText(ch, buffer, cmd_name);
    return;
  }
  if(!str_cmp(sub, "read")) {
    int n = 0;
    if(std::sscanf(rest, "%d", &n) != 1 || n < 1) {
      send_to_char(kind == ServerTextKind::wiznews ? "Sintassi: wiznews read <numero>\n\r"
                                                   : "Sintassi: news read <numero>\n\r",
                   ch);
      return;
    }
    st_show_read(ch, kind, n);
    return;
  }
  if(!str_cmp(sub, "list")) {
    st_admin_list(ch, kind);
    return;
  }
  if(!str_cmp(sub, "add")) {
    st_admin_add(ch, kind, rest);
    return;
  }
  if(!str_cmp(sub, "del")) {
    st_admin_del(ch, kind, rest);
    return;
  }
  if(!str_cmp(sub, "deleted")) {
    st_admin_deleted(ch, kind);
    return;
  }
  if(!str_cmp(sub, "undel")) {
    st_admin_undel(ch, kind, rest);
    return;
  }
  if(!str_cmp(sub, "body")) {
    st_admin_body(ch, kind, rest);
    return;
  }
  st_news_syntax(ch, kind);
}

bool st_finish_body_write_impl(struct descriptor_data* d) {
  if(!d) {
    return false;
  }
  auto it = g_body_pending.find(d);
  if(it == g_body_pending.end()) {
    return false;
  }
  BodyWritePending pending = it->second;
  /* d->str punta a pending.buffer nella map: invalidare prima dell'erase. */
  d->str = nullptr;
  g_body_pending.erase(it);

  const char* text = pending.buffer ? pending.buffer : "";
  odb::database* db = Sql::getMysql();
  if(db && pending.entry_id) {
    st_update_body_long(db, pending.entry_id, text);
    st_rebuild_all_buffers(db);
  }
  if(pending.buffer) {
    free(pending.buffer);
    pending.buffer = nullptr;
  }
  if(d->character) {
    send_to_char("Testo esteso salvato.\n\r", d->character);
  }
  return true;
}

void st_motd_syntax(struct char_data* ch, ServerTextKind kind) {
  if(kind == ServerTextKind::wizmotd) {
    send_to_char("Sintassi: wizmotd [show]\n\r", ch);
    send_to_char("          wizmotd set [general|server|world] [versione] <testo>\n\r", ch);
  }
  else {
    send_to_char("Sintassi: motd [show]\n\r", ch);
    send_to_char("          motd set [general|server|world] [versione] <testo>\n\r", ch);
  }
  send_to_char("            (senza componente = general)\n\r", ch);
}

bool st_do_motd_set(struct char_data* ch, ServerTextKind kind, const char* rest,
                    const char* syntax);

void st_do_motd_command(struct char_data* ch, ServerTextKind kind, const char* arg, char* buffer,
                        const char* cmd_name, int min_level) {
  if(!arg || !*arg) {
    ShowStaticPagedText(ch, buffer, cmd_name);
    return;
  }
  char sub[MAX_INPUT_LENGTH];
  char rest[MAX_INPUT_LENGTH];
  if(!st_parse_subcommand(arg, sub, rest)) {
    ShowStaticPagedText(ch, buffer, cmd_name);
    return;
  }
  if(!str_cmp(sub, "show")) {
    ShowStaticPagedText(ch, buffer, cmd_name);
    return;
  }
  if(!str_cmp(sub, "set")) {
    if(GetMaxLevel(ch) < min_level) {
      send_to_char("Pardon?\n\r", ch);
      return;
    }
    if(!rest[0]) {
      st_motd_syntax(ch, kind);
      return;
    }
    st_do_motd_set(ch, kind, rest, kind == ServerTextKind::wizmotd
                                   ? "Sintassi: wizmotd set [general|server|world] [versione] "
                                     "<testo>\n\r"
                                   : "Sintassi: motd set [general|server|world] [versione] "
                                     "<testo>\n\r");
    return;
  }
  // If user is below min_level and tries any other command, default to show
  if(GetMaxLevel(ch) < min_level) {
    ShowStaticPagedText(ch, buffer, cmd_name);
    return;
  }
  st_motd_syntax(ch, kind);
}

bool st_do_motd_set(struct char_data* ch, ServerTextKind kind, const char* rest,
                    const char* syntax) {
  if(!rest || !*rest) {
    send_to_char(syntax, ch);
    return false;
  }
  ServerTextComponent component = ServerTextComponent::general;
  std::string version;
  std::string plain_headline;
  st_parse_add_tokens(rest, component, version, plain_headline);
  if(plain_headline.empty()) {
    send_to_char(syntax, ch);
    return false;
  }
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db) ||
     !st_replace_motd(db, kind, component, version.empty() ? nullptr : version.c_str(),
                      plain_headline.c_str(), GET_NAME(ch))) {
    send_to_char(kind == ServerTextKind::wizmotd ? "Salvataggio wizmotd fallito.\n\r"
                                                 : "Salvataggio motd fallito.\n\r",
                 ch);
    return false;
  }
  st_rebuild_all_buffers(db);
  send_to_char(kind == ServerTextKind::wizmotd ? "Wizmotd aggiornato.\n\r" : "Motd aggiornato.\n\r",
               ch);
  return true;
}

/** Map git commit author (and co-op notes) to in-game news author. Extend as needed. */
std::string st_news_author_from_commit(const char* git_author, const char* subject,
                                       const char* body) {
  auto lower = [](const char* s) {
    std::string out = s ? s : "";
    for(char& c : out) {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
  };
  const std::string author_l = lower(git_author);
  const std::string subject_l = lower(subject);
  const std::string body_l = lower(body);
  const std::string hay = author_l + '\n' + subject_l + '\n' + body_l;

  // Montero / cooperation with Montero → Croneh
  if(author_l == "montero" || hay.find("in cooperation with montero") != std::string::npos ||
     hay.find("co-authored-by: montero") != std::string::npos) {
    return "Croneh";
  }
  // Alar (and historical git names) → Alar
  if(author_l == "alar" || author_l == "alar77" || author_l == "alar l'oscuro") {
    return "Alar";
  }

  if(git_author && *git_author) {
    return git_author;
  }
  return "server";
}

std::string st_humanize_build(const char* build) {
  std::string out = build ? build : "";
  for(char& c : out) {
    if(c == '-') {
      c = ' ';
    }
  }
  return out;
}

std::string st_trim_copy(const std::string& s) {
  std::size_t b = 0;
  while(b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) {
    ++b;
  }
  std::size_t e = s.size();
  while(e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
    --e;
  }
  return s.substr(b, e - b);
}

bool st_starts_with_ci(const std::string& s, const char* prefix) {
  if(!prefix) {
    return false;
  }
  const std::size_t n = std::strlen(prefix);
  if(s.size() < n) {
    return false;
  }
  for(std::size_t i = 0; i < n; ++i) {
    if(std::tolower(static_cast<unsigned char>(s[i])) !=
       std::tolower(static_cast<unsigned char>(prefix[i]))) {
      return false;
    }
  }
  return true;
}

/** True if humanized/raw subject starts with WIZ marker (WIZ:, WIZ-, WIZ ). */
bool st_has_wiz_title_prefix(const char* build) {
  const std::string h = st_trim_copy(st_humanize_build(build));
  if(!st_starts_with_ci(h, "wiz")) {
    return false;
  }
  if(h.size() == 3) {
    return true;
  }
  const char sep = h[3];
  return sep == ' ' || sep == '-' || sep == ':' || sep == '_';
}

std::string st_strip_wiz_title_prefix(const std::string& humanized) {
  std::string h = st_trim_copy(humanized);
  if(!st_starts_with_ci(h, "wiz")) {
    return h;
  }
  if(h.size() == 3) {
    return "";
  }
  const char sep = h[3];
  if(sep != ' ' && sep != '-' && sep != ':' && sep != '_') {
    return h;
  }
  return st_trim_copy(h.substr(4));
}

/**
 * Split commit body into NEWS: / WIZ: sections (case-insensitive headers at line start).
 * If neither tag appears, has_* stay false and outs stay empty (caller uses full body).
 */
void st_extract_news_wiz_sections(const char* body, std::string& news_out, std::string& wiz_out,
                                  bool& has_news_tag, bool& has_wiz_tag) {
  news_out.clear();
  wiz_out.clear();
  has_news_tag = false;
  has_wiz_tag = false;
  if(!body || !*body) {
    return;
  }

  enum class Section { none, news, wiz };
  Section cur = Section::none;
  std::ostringstream news_ss;
  std::ostringstream wiz_ss;

  const std::string text = body;
  std::size_t i = 0;
  while(i < text.size()) {
    std::size_t eol = text.find('\n', i);
    std::string line =
      (eol == std::string::npos) ? text.substr(i) : text.substr(i, eol - i);
    if(!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    i = (eol == std::string::npos) ? text.size() : eol + 1;

    std::string trimmed = st_trim_copy(line);
    auto take_header = [&](const char* tag) -> bool {
      if(!st_starts_with_ci(trimmed, tag)) {
        return false;
      }
      const std::size_t n = std::strlen(tag);
      std::string rest = st_trim_copy(trimmed.substr(n));
      if(st_starts_with_ci(tag, "news")) {
        has_news_tag = true;
        cur = Section::news;
        if(!rest.empty()) {
          news_ss << rest << '\n';
        }
      }
      else {
        has_wiz_tag = true;
        cur = Section::wiz;
        if(!rest.empty()) {
          wiz_ss << rest << '\n';
        }
      }
      return true;
    };

    if(take_header("WIZNEWS:") || take_header("NEWS:") || take_header("WIZ:")) {
      continue;
    }
    if(cur == Section::news) {
      news_ss << line << '\n';
    }
    else if(cur == Section::wiz) {
      wiz_ss << line << '\n';
    }
  }

  news_out = st_trim_copy(news_ss.str());
  wiz_out = st_trim_copy(wiz_ss.str());
}

struct CommitAnnouncePlan {
  std::string headline;
  std::string news_body;
  std::string wiz_body;
  bool want_news = false;
  bool want_wiznews = false;
};

CommitAnnouncePlan st_plan_commit_announce(const char* build, const char* body) {
  CommitAnnouncePlan plan;
  const std::string human = st_humanize_build(build);
  const bool wiz_title = st_has_wiz_title_prefix(build);
  plan.headline = st_strip_wiz_title_prefix(human);
  if(plan.headline.empty()) {
    plan.headline = human;
  }

  std::string news_sec;
  std::string wiz_sec;
  bool has_news_tag = false;
  bool has_wiz_tag = false;
  st_extract_news_wiz_sections(body, news_sec, wiz_sec, has_news_tag, has_wiz_tag);

  const std::string full_body = body ? st_trim_copy(body) : "";

  if(has_news_tag || has_wiz_tag) {
    if(has_news_tag) {
      plan.want_news = true;
      plan.news_body = news_sec;
    }
    if(has_wiz_tag) {
      plan.want_wiznews = true;
      plan.wiz_body = wiz_sec;
    }
    // Release with only NEWS: also mirror to wiznews for staff visibility.
    if(is_release() && plan.want_news && !plan.want_wiznews) {
      plan.want_wiznews = true;
      plan.wiz_body = plan.news_body;
    }
    return plan;
  }

  // No section tags: title WIZ: → wiznews only; else news (+ wiznews on release).
  if(wiz_title) {
    plan.want_wiznews = true;
    plan.wiz_body = full_body;
    return plan;
  }
  plan.want_news = true;
  plan.news_body = full_body;
  if(is_release()) {
    plan.want_wiznews = true;
    plan.wiz_body = full_body;
  }
  return plan;
}

bool st_upsert_version_announce(odb::database* db, ServerTextKind kind, const char* ver,
                                const std::string& author, const std::string& headline,
                                const char* body_text, const char* log_tag) {
  if(!db || !ver || !*ver || !log_tag) {
    return false;
  }
  const unsigned k = static_cast<unsigned>(kind);
  const unsigned component = static_cast<unsigned>(ServerTextComponent::server);
  const char* body = (body_text && *body_text) ? body_text : nullptr;

  std::ostringstream exists_sql;
  exists_sql << "SELECT id, IFNULL(body_long,''), IFNULL(author,'') FROM server_text_entry WHERE "
                "kind="
             << k << " AND component=" << component << " AND active=1 AND version_str="
             << st_sql_literal(ver) << " ORDER BY id DESC LIMIT 1";
  MYSQL_RES* res = nullptr;
  unsigned long long existing_id = 0;
  bool existing_has_body = false;
  std::string existing_author;
  if(st_mysql_query(db, exists_sql.str(), res) && res) {
    if(MYSQL_ROW row = mysql_fetch_row(res)) {
      if(row[0]) {
        existing_id = std::strtoull(row[0], nullptr, 10);
      }
      if(row[1] && row[1][0]) {
        existing_has_body = true;
      }
      if(row[2]) {
        existing_author = row[2];
      }
    }
    mysql_free_result(res);
  }
  if(existing_id > 0) {
    bool changed = false;
    if(existing_author != author) {
      if(st_update_author(db, existing_id, author.c_str())) {
        mudlog(LOG_CHECK, "server_text_boot: auto-%s author update id=%llu to %s", log_tag,
               static_cast<unsigned long long>(existing_id), author.c_str());
        changed = true;
      }
      else {
        mudlog(LOG_SYSERR, "server_text_boot: auto-%s author update failed id=%llu", log_tag,
               static_cast<unsigned long long>(existing_id));
      }
    }
    if(!existing_has_body && body) {
      if(st_update_body_long(db, existing_id, body)) {
        mudlog(LOG_CHECK, "server_text_boot: auto-%s body backfill id=%llu version=%s", log_tag,
               static_cast<unsigned long long>(existing_id), ver);
        changed = true;
      }
      else {
        mudlog(LOG_SYSERR, "server_text_boot: auto-%s body backfill failed id=%llu", log_tag,
               static_cast<unsigned long long>(existing_id));
      }
    }
    return changed;
  }

  const time_t now = time(nullptr);
  const struct tm* tm_now = localtime(&now);
  int year = 0;
  int month = 0;
  int day = 0;
  if(tm_now) {
    year = tm_now->tm_year + 1900;
    month = tm_now->tm_mon + 1;
    day = tm_now->tm_mday;
  }
  const int sort_key = st_sort_key_from_ymd(year, month, day);
  const unsigned long long id =
    st_insert_entry(db, kind, ServerTextComponent::server, headline.c_str(), ver, author.c_str(),
                    sort_key, day, month, year, true, body);
  if(id == 0) {
    mudlog(LOG_SYSERR, "server_text_boot: auto-%s insert failed version=%s", log_tag, ver);
    return false;
  }
  const int has_body = body ? 1 : 0;
  mudlog(LOG_CHECK, "server_text_boot: auto-%s id=%llu version=%s author=%s has_body=%d", log_tag,
         static_cast<unsigned long long>(id), ver, author.c_str(), has_body);
  return true;
}

/** Insert/backfill news and/or wiznews from commit subject/body (NEWS:/WIZ: split). */
bool st_ensure_commit_announces(odb::database* db) {
  if(!db) {
    return false;
  }
  const char* ver = version();
  const char* build = release();
  const char* body = release_body();
  if(!ver || !*ver) {
    return false;
  }
  const CommitAnnouncePlan plan = st_plan_commit_announce(build, body);
  if(!plan.want_news && !plan.want_wiznews) {
    return false;
  }
  const std::string author = st_news_author_from_commit(release_author(), build, body);
  bool changed = false;
  if(plan.want_news) {
    changed = st_upsert_version_announce(db, ServerTextKind::news, ver, author, plan.headline,
                                         plan.news_body.c_str(), "news") ||
              changed;
  }
  if(plan.want_wiznews) {
    changed = st_upsert_version_announce(db, ServerTextKind::wiznews, ver, author, plan.headline,
                                         plan.wiz_body.c_str(), "wiznews") ||
              changed;
  }
  return changed;
}

/**
 * Keep motd slot "server" in sync only for real rX.Y[.Z] release builds.
 * Feature/alpha binaries must not rewrite the classic short Server line.
 */
bool st_ensure_release_motd(odb::database* db) {
  if(!db || !is_release()) {
    return false;
  }
  const char* ver = motd_version();
  const char* build = release();
  if(!ver || !*ver) {
    return false;
  }
  const std::string author =
    st_news_author_from_commit(release_author(), build, release_body());
  const unsigned kind = static_cast<unsigned>(ServerTextKind::motd);
  const unsigned component = static_cast<unsigned>(ServerTextComponent::server);

  const char* hl_override = motd_headline();
  std::string headline;
  if(hl_override && *hl_override) {
    headline = hl_override;
  }
  else {
    headline = st_humanize_build(build);
  }

  std::ostringstream exists_sql;
  exists_sql << "SELECT id, IFNULL(version_str,''), IFNULL(author,''), IFNULL(headline,'') "
                "FROM server_text_entry WHERE kind="
             << kind << " AND component=" << component
             << " AND active=1 ORDER BY id DESC LIMIT 1";
  MYSQL_RES* res = nullptr;
  unsigned long long existing_id = 0;
  std::string existing_version;
  std::string existing_author;
  std::string existing_headline;
  if(st_mysql_query(db, exists_sql.str(), res) && res) {
    if(MYSQL_ROW row = mysql_fetch_row(res)) {
      if(row[0]) {
        existing_id = std::strtoull(row[0], nullptr, 10);
      }
      if(row[1]) {
        existing_version = row[1];
      }
      if(row[2]) {
        existing_author = row[2];
      }
      if(row[3]) {
        existing_headline = row[3];
      }
    }
    mysql_free_result(res);
  }

  if(existing_id > 0 && existing_version == ver) {
    bool changed = false;
    if(existing_author != author) {
      if(st_update_author(db, existing_id, author.c_str())) {
        mudlog(LOG_CHECK, "server_text_boot: auto-motd author update id=%llu to %s",
               static_cast<unsigned long long>(existing_id), author.c_str());
        changed = true;
      }
    }
    // Only overwrite headline when MOTD_HEADLINE was set at compile time.
    if(hl_override && *hl_override && existing_headline != headline) {
      std::ostringstream sql;
      sql << "UPDATE server_text_entry SET headline=" << st_sql_literal(headline.c_str())
          << ", updated_at=NOW() WHERE id=" << existing_id;
      if(st_mysql_exec(db, sql.str())) {
        mudlog(LOG_CHECK, "server_text_boot: auto-motd headline update id=%llu",
               static_cast<unsigned long long>(existing_id));
        changed = true;
      }
    }
    return changed;
  }

  const int sort_key = static_cast<int>(time(nullptr));

  std::ostringstream deactivate;
  deactivate << "UPDATE server_text_entry SET active=0, updated_at=NOW() WHERE kind=" << kind
             << " AND component=" << component << " AND active=1";
  st_mysql_exec(db, deactivate.str());

  // No entry_date: classic motd Server/World lines are undated.
  const unsigned long long id =
    st_insert_entry(db, ServerTextKind::motd, ServerTextComponent::server, headline.c_str(),
                    ver, author.c_str(), sort_key, 0, 0, 0, false, nullptr);
  if(id == 0) {
    mudlog(LOG_SYSERR, "server_text_boot: auto-motd insert failed version=%s", ver);
    return false;
  }
  mudlog(LOG_CHECK, "server_text_boot: auto-motd id=%llu version=%s author=%s",
         static_cast<unsigned long long>(id), ver, author.c_str());
  return true;
}

} /* anonymous */

void server_text_boot() {
  odb::database* db = Sql::getMysql();
  if(!db || !st_ensure_table(db)) {
    mudlog(LOG_ERROR, "server_text_boot: MySQL table unavailable, keeping file buffers");
    file_to_string(NEWS_FILE.c_str(), news);
    file_to_string(WIZNEWS_FILE.c_str(), wiznews);
    file_to_string(MOTD_FILE.c_str(), motd);
    file_to_string(WIZ_MOTD_FILE.c_str(), wmotd);
    return;
  }
  st_seed_if_empty(db);
  if(st_ensure_commit_announces(db)) {
    mudlog(LOG_CHECK, "server_text_boot: commit announces updated for %s", version());
  }
  if(st_ensure_release_motd(db)) {
    mudlog(LOG_CHECK, "server_text_boot: updated motd server slot for %s", version());
  }
  st_rebuild_all_buffers(db);
  mudlog(LOG_CHECK, "server_text_boot: loaded news=%zu wiznews=%zu from MySQL",
         g_news_rows.size(), g_wiznews_rows.size());
}

void server_text_reload() {
  server_text_boot();
}

bool server_text_finish_body_write(struct descriptor_data* d) {
  return st_finish_body_write_impl(d);
}

void server_text_do_news(struct char_data* ch, const char* arg) {
  if(ch == nullptr) {
    mudlog(LOG_SYSERR, "ch==nullptr in server_text_do_news");
    return;
  }
  st_dispatch_admin(ch, ServerTextKind::news, arg, news, "do_news", MAESTRO_DEI_CREATORI);
}

void server_text_do_wiznews(struct char_data* ch, const char* arg) {
  if(ch == nullptr) {
    mudlog(LOG_SYSERR, "ch==nullptr in server_text_do_wiznews");
    return;
  }
  st_dispatch_admin(ch, ServerTextKind::wiznews, arg, wiznews, "do_wiznews", IMMORTALE);
}

void server_text_do_motd(struct char_data* ch, const char* arg) {
  if(ch == nullptr) {
    return;
  }
  st_do_motd_command(ch, ServerTextKind::motd, arg, motd, "do_motd", MAESTRO_DEI_CREATORI);
}

void server_text_do_wizmotd(struct char_data* ch, const char* arg) {
  if(ch == nullptr) {
    return;
  }
  st_do_motd_command(ch, ServerTextKind::wizmotd, arg, wmotd, "do_wizmotd", IMMORTALE);
}

ACTION_FUNC(do_motd) {
  server_text_do_motd(ch, arg);
}

ACTION_FUNC(do_wizmotd) {
  server_text_do_wizmotd(ch, arg);
}

} /* namespace Alarmud */

#else /* !USE_MYSQL */

namespace Alarmud {

void server_text_boot() {}
void server_text_reload() {}
bool server_text_finish_body_write(struct descriptor_data*) {
  return false;
}
void server_text_do_news(struct char_data* ch, const char* arg) {
  (void)arg;
  if(ch) {
    ShowStaticPagedText(ch, news, "do_news");
  }
}
void server_text_do_wiznews(struct char_data* ch, const char* arg) {
  (void)arg;
  if(ch) {
    ShowStaticPagedText(ch, wiznews, "do_wiznews");
  }
}
void server_text_do_motd(struct char_data*, const char*) {}
void server_text_do_wizmotd(struct char_data*, const char*) {}

ACTION_FUNC(do_motd) {}
ACTION_FUNC(do_wizmotd) {}

} /* namespace Alarmud */

#endif /* USE_MYSQL */
