/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "autoenums.hpp"
/***************************  Local    include ************************************/
#include "editor_term.hpp"
#include "comm.hpp"
#include "vt100c.hpp"

namespace Alarmud {

void EditorClear(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	if(ch->term == VT100) {
		send_to_char(VT_HOMECLR, ch);
	}
	else {
		send_to_char("\n\r", ch);
	}
}

void EditorGoto(struct char_data* ch, int row, int col) {
	if(ch == nullptr || ch->term != VT100) {
		return;
	}
	char buf[32];
	std::snprintf(buf, sizeof(buf), VT_CURSPOS, row, col);
	send_to_char(buf, ch);
}

void EditorPadToColumn(struct char_data* ch, int already, int column) {
	if(ch == nullptr || ch->term == VT100 || column <= 0) {
		return;
	}
	/* Match the historic menu padding: at least one space if the field
	 * already reached or passed the target column. */
	unsigned pad = 1u;
	if(already < column) {
		pad = static_cast<unsigned>(column - already);
	}
	const std::string spaces(pad, ' ');
	send_to_char(spaces.c_str(), ch);
}

void EditorAppendTwoColumns(std::string& out, const char* left, const char* right, int left_width) {
	const char* l = (left != nullptr) ? left : "";
	out.append(l);
	if(right == nullptr || *right == '\0') {
		out.append("\n\r");
		return;
	}
	const int len = static_cast<int>(std::strlen(l));
	int pad = left_width - len;
	if(pad < 1) {
		pad = 1;
	}
	out.append(static_cast<std::size_t>(pad), ' ');
	out.append(right);
	out.append("\n\r");
}


void EditorPrompt(struct char_data* ch, const char* msg) {
	if(ch == nullptr || msg == nullptr) {
		return;
	}
	if(ch->term == VT100) {
		send_to_char(msg, ch);
		return;
	}
	/* Gli a capo iniziali diventano uno solo: niente righe vuote in piu',
	 * ma la domanda resta su una riga sua anche dopo un "Current ...: X"
	 * mandato senza a capo. */
	bool had_newline = false;
	while(*msg == '\n' || *msg == '\r') {
		had_newline = true;
		++msg;
	}
	if(had_newline) {
		send_to_char("\n\r", ch);
	}
	send_to_char(msg, ch);
	const std::size_t n = std::strlen(msg);
	if(n == 0 || (msg[n - 1] != '\n' && msg[n - 1] != '\r')) {
		send_to_char("\n\r", ch);
	}
}

void EditorBeginText(struct char_data* ch, char** field) {
	if(ch == nullptr || ch->desc == nullptr || field == nullptr) {
		return;
	}
	struct descriptor_data* d = ch->desc;
	if(d->str_backup != nullptr) {
		free(d->str_backup);
	}
	d->str_backup = *field;
	*field = nullptr;
	d->str = field;
	d->max_str = MAX_STRING_LENGTH;
}

bool EditorFinishText(struct descriptor_data* d) {
	if(d == nullptr || d->str_backup == nullptr) {
		return false;
	}
	char* saved = d->str_backup;
	d->str_backup = nullptr;
	if(d->str != nullptr && (*d->str == nullptr || **d->str == '\0')) {
		if(*d->str != nullptr) {
			free(*d->str);
		}
		*d->str = saved;
		return true;
	}
	free(saved);
	return false;
}

void EditorAbortText(struct descriptor_data* d) {
	if(d == nullptr || d->str_backup == nullptr) {
		return;
	}
	if(d->str != nullptr) {
		if(*d->str != nullptr) {
			free(*d->str);
		}
		*d->str = d->str_backup;
		d->str = nullptr;
	}
	else {
		free(d->str_backup);
	}
	d->str_backup = nullptr;
}

bool IsEditorState(int connected) {
	return connected == CON_OBJ_EDITING ||
	       connected == CON_MOB_EDITING ||
	       connected == CON_EDITING;
}

} // namespace Alarmud
