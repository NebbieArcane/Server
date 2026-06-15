/*
 * account.hpp — ODB persistence model for nebbie database tables.
 *
 * Schema reference: nebbie_stru.sql
 */

#ifndef SRC_ODB_ACCOUNT_HPP_
#define SRC_ODB_ACCOUNT_HPP_

#include <cstddef>
#include <ctime>
#include <string>

#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <odb/core.hxx>
#include <odb/nullable.hxx>

namespace Alarmud {

enum class inventory_deleted_for {
  DEATH,
  RENT_EXPIRED,
  NUKE,
  TRAP,
  MANUAL,
  SCRAP
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_achievements_key {
  unsigned long long toon_id;
  unsigned char category;
  unsigned short slot_index;
};

class character_achievements {
public:
  character_achievements_key key;
  int value;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_affects_key {
  unsigned long long toon_id;
  unsigned char slot;
};

class character_affects {
public:
  character_affects_key key;
  short type;
  short duration;
  int modifier;
  int location;
  int bitvector;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_aliases_key {
  unsigned long long toon_id;
  unsigned char slot;
};

class character_aliases {
public:
  character_aliases_key key;
  std::string alias_text;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_classes_key {
  unsigned long long toon_id;
  unsigned char class_index;
};

class character_classes {
public:
  character_classes_key key;
  unsigned char level;
};

class character_core {
public:
  unsigned long long toon_id;
  odb::nullable<std::string> description;
  int class_primary;
  unsigned char sex;
  int race;
  unsigned int birth;
  int played;
  unsigned int last_logon;
  unsigned int weight;
  unsigned int height;
  short hometown;
  bool talks_0;
  bool talks_1;
  bool talks_2;
  int speaks;
  int user_flags;
  int extra_flags;
  int age_modifier;
  std::string authcode;
  short wimpy_level;
  short load_room;
  int start_room;
  unsigned char spells_to_learn;
  int alignment;
  unsigned int act;
  unsigned int affected_by;
  unsigned int affected_by2;
  signed char condition_drunk;
  signed char condition_full;
  signed char condition_thirst;
  short save_throw_0;
  short save_throw_1;
  short save_throw_2;
  short save_throw_3;
  short save_throw_4;
  short save_throw_5;
  short save_throw_6;
  short save_throw_7;
  boost::posix_time::ptime updated_at;
};

class character_death_snapshot {
public:
  unsigned long long toon_id;
  int saved_exp;
  unsigned int saved_at;
  boost::posix_time::ptime updated_at;
};

class character_inventory {
public:
  unsigned long long id;
  unsigned long long toon_id;
  unsigned short list_index;
  unsigned short item_number;
  int value0;
  int value1;
  int value2;
  int value3;
  int extra_flags;
  int extra_flags2;
  int weight;
  int timer;
  unsigned int bitvector;
  std::string obj_name;
  std::string short_desc;
  std::string description;
  unsigned char wear_pos;
  unsigned char depth;
  bool deleted;
  odb::nullable<boost::posix_time::ptime> deleted_on;
  odb::nullable<inventory_deleted_for> deleted_for;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_inventory_affect_key {
  unsigned long long inventory_id;
  unsigned char affect_slot;
};

class character_inventory_affect {
public:
  character_inventory_affect_key key;
  short location;
  int modifier;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_mercy_key {
  unsigned long long toon_id;
  unsigned short quest_index;
};

class character_mercy {
public:
  character_mercy_key key;
  int value;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_prefs_key {
  unsigned long long toon_id;
  std::string pref_key;
};

class character_prefs {
public:
  character_prefs_key key;
  std::string pref_value;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_quest_progress_key {
  unsigned long long toon_id;
  unsigned short quest_index;
  unsigned char mob_slot;
};

class character_quest_progress {
public:
  character_quest_progress_key key;
  int mob_vnum;
};

class character_rent {
public:
  unsigned long long toon_id;
  int gold_left;
  int total_cost;
  unsigned int last_update;
  int minimum_stay;
  int object_count;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_resistance_key {
  unsigned long long toon_id;
  unsigned int damage_type;
};

class character_resistance {
public:
  character_resistance_key key;
  short value;
};

#ifdef ODB_COMPILER
#pragma db value
#endif
struct character_skills_key {
  unsigned long long toon_id;
  unsigned short skill_id;
};

class character_skills {
public:
  character_skills_key key;
  unsigned char learned;
  signed char flags;
  signed char special;
  signed char nummem;
};

class character_stats {
public:
  unsigned long long toon_id;
  signed char str;
  signed char str_add;
  signed char intel;
  signed char wis;
  signed char dex;
  signed char con;
  signed char chr;
  signed char extra;
  signed char extra2;
  short mana;
  short max_mana;
  unsigned char mana_gain;
  short hit;
  short max_hit;
  unsigned char hit_gain;
  short move;
  short max_move;
  unsigned char move_gain;
  unsigned short p_rune_dei;
  short points_extra1;
  short points_extra2;
  unsigned char points_extra3;
  short armor;
  int gold;
  int bank_gold;
  int exp;
  int true_exp;
  int extra_dual;
  signed char hitroll;
  signed char damroll;
  signed char libero;
};

class registered {
public:
  std::string name;
  odb::nullable<std::string> realname;
  odb::nullable<std::string> god;
  odb::nullable<std::string> email1;
  odb::nullable<std::string> email2;
  odb::nullable<std::string> note;
};

class toonExtra {
public:
  unsigned long long id;
  std::string field;
  std::string value;
};

class toonRent {
public:
  unsigned long long id;
  unsigned long long vnum;
  std::string effects;
  bool bank;
};

class toon {
public:
  toon()
      : id(0), name(""), password(""), title(""), level(0),
        lastlogin(boost::posix_time::not_a_date_time), lasthost(""),
        owner_id(0), migrated_at(), schema_version(0) {}

  toon(const char *name, const char *password = "invalid",
       const char *title = "")
      : id(0), name(name), password(password), title(title), level(0),
        lastlogin(boost::posix_time::not_a_date_time), lasthost(""),
        owner_id(0), migrated_at(), schema_version(0) {}

  unsigned long long id;
  std::string name;
  std::string password;
  std::string title;
  unsigned short level;
  boost::posix_time::ptime lastlogin;
  std::string lasthost;
  unsigned long long owner_id;
  /** Cutover gate — colonne su MySQL, non mappate ODB (vedi toon_migration +
   * nebbie_stru.sql). */
  odb::nullable<boost::posix_time::ptime> migrated_at;
  unsigned short schema_version;
};

class toon_nuke_blacklist {
public:
  unsigned long long id;
  unsigned long long toon_id;
  std::string toon_name;
  boost::posix_time::ptime nuked_at;
  std::string nuked_by;
};

class user {
public:
  unsigned long long id;
  std::string nickname;
  std::string email;
  boost::posix_time::ptime registered;
  std::string password;
  unsigned short level;
  odb::nullable<std::string> backup_email;
  bool ptr;
  std::string choosen;
  bool authorized;

  user()
      : id(0), nickname(""), email(""),
        registered(boost::posix_time::from_time_t(time(nullptr))), password(""),
        level(0), backup_email(), ptr(false), choosen(""), authorized(false) {}

  user(const std::string &nickname, const std::string &email,
       const std::string &password = "")
      : id(0), nickname(nickname), email(email),
        registered(boost::posix_time::from_time_t(time(nullptr))),
        password(password), level(0), backup_email(), ptr(false), choosen(""),
        authorized(false) {}
};

#ifdef ODB_COMPILER
#pragma db view object(user)
#endif
class userCount {
public:
#ifdef ODB_COMPILER
#pragma db column("count(" + user::id + ")")
#endif
  std::size_t count;
};

class legacy {
public:
  std::string name;
  std::string realname;
  std::string email1;
  std::string email2;
};

#ifdef ODB_COMPILER
#pragma db view type("v_character_summary")
#endif
class v_character_summary {
public:
  unsigned long long toon_id;
  std::string toon_name;
  unsigned long long account_id;
  std::string account_email;
  std::string title;
  unsigned short toon_best_level;
  int race;
  int alignment;
  short hit;
  short max_hit;
  int gold;
  int exp;
};

#ifdef ODB_COMPILER
#pragma db model version(1, 1, closed)
#pragma db model version(1, 2, open)

#pragma db object(character_achievements) session(false)
#pragma db member(character_achievements::key) id
#pragma db member(character_achievements_key::toon_id) not_null
#pragma db member(character_achievements_key::category) not_null
#pragma db member(character_achievements_key::slot_index) not_null
#pragma db member(character_achievements::value) not_null default(0)

#pragma db object(character_affects) session(false)
#pragma db member(character_affects::key) id
#pragma db member(character_affects_key::toon_id) not_null
#pragma db member(character_affects_key::slot) not_null
#pragma db member(character_affects::type) not_null default(0)
#pragma db member(character_affects::duration) not_null default(0)
#pragma db member(character_affects::modifier) not_null default(0)
#pragma db member(character_affects::location) not_null default(0)
#pragma db member(character_affects::bitvector) not_null default(0)

#pragma db object(character_aliases) session(false)
#pragma db member(character_aliases::key) id
#pragma db member(character_aliases_key::toon_id) not_null
#pragma db member(character_aliases_key::slot) not_null
#pragma db member(character_aliases::alias_text) type("varchar(512)") not_null

#pragma db object(character_classes) session(false)
#pragma db member(character_classes::key) id
#pragma db member(character_classes_key::toon_id) not_null
#pragma db member(character_classes_key::class_index) not_null
#pragma db member(character_classes::level) not_null default(0)

#pragma db object(character_core) session(false)
#pragma db member(character_core::toon_id) id not_null
#pragma db member(character_core::description) type("varchar(240)") null
#pragma db member(character_core::class_primary) not_null default(0)
#pragma db member(character_core::sex) not_null default(0)
#pragma db member(character_core::race) not_null default(0)
#pragma db member(character_core::birth) not_null default(0)
#pragma db member(character_core::played) not_null default(0)
#pragma db member(character_core::last_logon) not_null default(0)
#pragma db member(character_core::weight) not_null default(0)
#pragma db member(character_core::height) not_null default(0)
#pragma db member(character_core::hometown) not_null default(0)
#pragma db member(character_core::talks_0) not_null default(0)
#pragma db member(character_core::talks_1) not_null default(0)
#pragma db member(character_core::talks_2) not_null default(0)
#pragma db member(character_core::speaks) not_null default(0)
#pragma db member(character_core::user_flags) not_null default(0)
#pragma db member(character_core::extra_flags) not_null default(0)
#pragma db member(character_core::age_modifier) not_null default(0)
#pragma db member(character_core::authcode) type("varchar(7)")                 \
    not_null default("")
#pragma db member(character_core::wimpy_level) not_null default(0)
#pragma db member(character_core::load_room) not_null default(0)
#pragma db member(character_core::start_room) not_null default(0)
#pragma db member(character_core::spells_to_learn) not_null default(0)
#pragma db member(character_core::alignment) not_null default(0)
#pragma db member(character_core::act) not_null default(0)
#pragma db member(character_core::affected_by) not_null default(0)
#pragma db member(character_core::affected_by2) not_null default(0)
#pragma db member(character_core::condition_drunk) not_null default(0)
#pragma db member(character_core::condition_full) not_null default(0)
#pragma db member(character_core::condition_thirst) not_null default(0)
#pragma db member(character_core::save_throw_0) not_null default(0)
#pragma db member(character_core::save_throw_1) not_null default(0)
#pragma db member(character_core::save_throw_2) not_null default(0)
#pragma db member(character_core::save_throw_3) not_null default(0)
#pragma db member(character_core::save_throw_4) not_null default(0)
#pragma db member(character_core::save_throw_5) not_null default(0)
#pragma db member(character_core::save_throw_6) not_null default(0)
#pragma db member(character_core::save_throw_7) not_null default(0)
#pragma db member(character_core::updated_at) type("TIMESTAMP") not_null

#pragma db object(character_death_snapshot) session(false)
#pragma db member(character_death_snapshot::toon_id) id not_null
#pragma db member(character_death_snapshot::saved_exp) not_null
#pragma db member(character_death_snapshot::saved_at) not_null
#pragma db member(character_death_snapshot::updated_at) type("DATETIME")       \
    not_null

#pragma db object(character_inventory) session(false)
#pragma db member(character_inventory::id) id auto
#pragma db member(character_inventory::toon_id) not_null index
#pragma db member(character_inventory::list_index) not_null
#pragma db member(character_inventory::item_number) not_null default(0)
#pragma db member(character_inventory::value0) not_null default(0)
#pragma db member(character_inventory::value1) not_null default(0)
#pragma db member(character_inventory::value2) not_null default(0)
#pragma db member(character_inventory::value3) not_null default(0)
#pragma db member(character_inventory::extra_flags) not_null default(0)
#pragma db member(character_inventory::extra_flags2) not_null default(0)
#pragma db member(character_inventory::weight) not_null default(0)
#pragma db member(character_inventory::timer) not_null default(0)
#pragma db member(character_inventory::bitvector) not_null default(0)
#pragma db member(character_inventory::obj_name) type("varchar(128)")          \
    not_null default("")
#pragma db member(character_inventory::short_desc) type("varchar(128)")        \
    not_null default("")
#pragma db member(character_inventory::description) type("varchar(256)")       \
    not_null default("")
#pragma db member(character_inventory::wear_pos) not_null default(0)
#pragma db member(character_inventory::depth) not_null default(0)
#pragma db member(character_inventory::deleted) not_null default(0)
#pragma db member(character_inventory::deleted_on) type("DATETIME") null
#pragma db member(character_inventory::deleted_for)                            \
    type("ENUM('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL','SCRAP')") null
#pragma db index(character_inventory::"idx_inventory_toon_active")             \
    members(toon_id, deleted, list_index)
#pragma db index(character_inventory::"idx_inventory_toon_deleted_on")         \
    members(toon_id, deleted_on)
#pragma db index(character_inventory::"idx_inventory_toon_deleted_for")        \
    members(toon_id, deleted_for)

#pragma db object(character_inventory_affect) session(false)
#pragma db member(character_inventory_affect::key) id
#pragma db member(character_inventory_affect_key::inventory_id) not_null
#pragma db member(character_inventory_affect_key::affect_slot) not_null
#pragma db member(character_inventory_affect::location) not_null default(0)
#pragma db member(character_inventory_affect::modifier) not_null default(0)

#pragma db object(character_mercy) session(false)
#pragma db member(character_mercy::key) id
#pragma db member(character_mercy_key::toon_id) not_null
#pragma db member(character_mercy_key::quest_index) not_null
#pragma db member(character_mercy::value) not_null default(0)

#pragma db object(character_prefs) session(false)
#pragma db member(character_prefs::key) id
#pragma db member(character_prefs_key::toon_id) not_null
#pragma db member(character_prefs_key::pref_key) type("varchar(32)") not_null
#pragma db member(character_prefs::pref_value) type("varchar(1024)") not_null

#pragma db object(character_quest_progress) session(false)
#pragma db member(character_quest_progress::key) id
#pragma db member(character_quest_progress_key::toon_id) not_null
#pragma db member(character_quest_progress_key::quest_index) not_null
#pragma db member(character_quest_progress_key::mob_slot) not_null
#pragma db member(character_quest_progress::mob_vnum) not_null default(0)

#pragma db object(character_rent) session(false)
#pragma db member(character_rent::toon_id) id not_null
#pragma db member(character_rent::gold_left) not_null default(0)
#pragma db member(character_rent::total_cost) not_null default(0)
#pragma db member(character_rent::last_update) not_null default(0)
#pragma db member(character_rent::minimum_stay) not_null default(0)
#pragma db member(character_rent::object_count) not_null default(0)

#pragma db object(character_resistance) session(false)
#pragma db member(character_resistance::key) id
#pragma db member(character_resistance_key::toon_id) not_null
#pragma db member(character_resistance_key::damage_type) not_null
#pragma db member(character_resistance::value) not_null default(0)

#pragma db object(character_skills) session(false)
#pragma db member(character_skills::key) id
#pragma db member(character_skills_key::toon_id) not_null
#pragma db member(character_skills_key::skill_id) not_null
#pragma db member(character_skills::learned) not_null default(0)
#pragma db member(character_skills::flags) not_null default(0)
#pragma db member(character_skills::special) not_null default(0)
#pragma db member(character_skills::nummem) not_null default(0)

#pragma db object(character_stats) session(false)
#pragma db member(character_stats::toon_id) id not_null
#pragma db member(character_stats::str) not_null default(0)
#pragma db member(character_stats::str_add) not_null default(0)
#pragma db member(character_stats::intel) not_null default(0)
#pragma db member(character_stats::wis) not_null default(0)
#pragma db member(character_stats::dex) not_null default(0)
#pragma db member(character_stats::con) not_null default(0)
#pragma db member(character_stats::chr) not_null default(0)
#pragma db member(character_stats::extra) not_null default(0)
#pragma db member(character_stats::extra2) not_null default(0)
#pragma db member(character_stats::mana) not_null default(0)
#pragma db member(character_stats::max_mana) not_null default(0)
#pragma db member(character_stats::mana_gain) not_null default(0)
#pragma db member(character_stats::hit) not_null default(0)
#pragma db member(character_stats::max_hit) not_null default(0)
#pragma db member(character_stats::hit_gain) not_null default(0)
#pragma db member(character_stats::move) not_null default(0)
#pragma db member(character_stats::max_move) not_null default(0)
#pragma db member(character_stats::move_gain) not_null default(0)
#pragma db member(character_stats::p_rune_dei) not_null default(0)
#pragma db member(character_stats::points_extra1) not_null default(0)
#pragma db member(character_stats::points_extra2) not_null default(0)
#pragma db member(character_stats::points_extra3) not_null default(0)
#pragma db member(character_stats::armor) not_null default(100)
#pragma db member(character_stats::gold) not_null default(0)
#pragma db member(character_stats::bank_gold) not_null default(0)
#pragma db member(character_stats::exp) not_null default(0)
#pragma db member(character_stats::true_exp) not_null default(0)
#pragma db member(character_stats::extra_dual) not_null default(0)
#pragma db member(character_stats::hitroll) not_null default(0)
#pragma db member(character_stats::damroll) not_null default(0)
#pragma db member(character_stats::libero) not_null default(0)

#pragma db object(registered) session(false)
#pragma db member(registered::name) id type("varchar(64)") not_null
#pragma db member(registered::realname) type("varchar(256)") null
#pragma db member(registered::god) type("varchar(64)") null
#pragma db member(registered::email1) type("varchar(256)") null index
#pragma db member(registered::email2) type("varchar(256)") null index
#pragma db member(registered::note) type("varchar(512)") null

#pragma db object(toon)
#pragma db member(toon::id) id auto
#pragma db member(toon::name) type("varchar(32)") not_null unique
#pragma db member(toon::password) type("varchar(128)") not_null
#pragma db member(toon::title) type("varchar(128)") not_null
#pragma db member(toon::level) not_null default(0)
#pragma db member(toon::lastlogin) type("DATETIME") null
#pragma db member(toon::lasthost) type("varchar(255)") not_null default("")
#pragma db member(toon::owner_id) not_null index
#pragma db member(toon::migrated_at) transient
#pragma db member(toon::schema_version) transient

#pragma db object(toonExtra)
#pragma db member(toonExtra::id) id auto
#pragma db member(toonExtra::field) type("varchar(32)") not_null
#pragma db member(toonExtra::value) type("varchar(1024)") not_null
#pragma db index(toonExtra::"idfield") members(id, field)

#pragma db object(toonRent)
#pragma db member(toonRent::id) id auto
#pragma db member(toonRent::vnum) not_null
#pragma db member(toonRent::effects) type("varchar(10240)") not_null
#pragma db member(toonRent::bank) not_null

#pragma db object(toon_nuke_blacklist) session(false)
#pragma db member(toon_nuke_blacklist::id) id auto
#pragma db member(toon_nuke_blacklist::toon_id) not_null unique
#pragma db member(toon_nuke_blacklist::toon_name) type("varchar(32)")          \
    not_null index
#pragma db member(toon_nuke_blacklist::nuked_at) type("DATETIME") not_null
#pragma db member(toon_nuke_blacklist::nuked_by) type("varchar(32)") not_null

#pragma db object(user) session(false)
#pragma db member(user::id) id auto
#pragma db member(user::nickname) type("VARCHAR(255)") not_null
#pragma db member(user::email) type("VARCHAR(255)") not_null unique
#pragma db member(user::registered) type("DATETIME") null
#pragma db member(user::password) type("VARCHAR(128)") not_null
#pragma db member(user::level) not_null default(0)
#pragma db member(user::backup_email) type("VARCHAR(255)") null
#pragma db member(user::ptr) not_null default(0)
#pragma db member(user::choosen) transient
#pragma db member(user::authorized) transient

#pragma db object(legacy)
#pragma db member(legacy::name) id type("varchar(32)") not_null
#pragma db member(legacy::realname) type("varchar(255)") not_null
#pragma db member(legacy::email1) type("varchar(255)") not_null
#pragma db member(legacy::email2) type("varchar(255)") not_null
#endif

} /* namespace Alarmud */

#endif /* SRC_ODB_ACCOUNT_HPP_ */
