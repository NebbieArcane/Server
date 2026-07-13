/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* server_text.hpp - news, wiznews, motd, wizmotd from MySQL (server_text_entry).
 * */
#ifndef SRC_SERVER_TEXT_HPP_
#define SRC_SERVER_TEXT_HPP_
/***************************  Local    include ************************************/
#include "structs.hpp"

namespace Alarmud {

enum class ServerTextKind : unsigned char {
  news = 0,
  wiznews = 1,
  motd = 2,
  wizmotd = 3
};

/** Codice (Server) vs aree (World); general = annuncio senza versione. */
enum class ServerTextComponent : unsigned char {
  general = 0,
  server = 1,
  world = 2
};

/** Boot/reload: ensure table, seed from files if empty, rebuild global buffers. */
void server_text_boot();

/** Reload only (called from reload_files_and_scripts). */
void server_text_reload();

/** Called from string_add when multi-line input ends with '@'. */
bool server_text_finish_body_write(struct descriptor_data* d);

/** Player / staff read commands (subcommands parsed from arg). */
void server_text_do_news(struct char_data* ch, const char* arg);
void server_text_do_wiznews(struct char_data* ch, const char* arg);

/** Staff motd set/show. */
void server_text_do_motd(struct char_data* ch, const char* arg);
void server_text_do_wizmotd(struct char_data* ch, const char* arg);

ACTION_FUNC(do_motd);
ACTION_FUNC(do_wizmotd);

} /* namespace Alarmud */

#endif /* SRC_SERVER_TEXT_HPP_ */
