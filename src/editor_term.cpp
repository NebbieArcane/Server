/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "autoenums.hpp"
/***************************  Local    include ************************************/
#include "editor_term.hpp"
#include "comm.hpp"
#include "create.obj.hpp"   /* oedit_snapshot_clear */
#include "vt100c.hpp"
#include "vt_screen.hpp"     /* vt_csi_pos */

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
	send_to_char(vt_csi_pos(row, col).c_str(), ch);
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

namespace {

/* true se d->str punta a uno dei campi testo di victim. */
[[nodiscard]] bool editor_str_in_char(const descriptor_data* d, char_data* victim) {
	char_player_data& p = victim->player;
	for(char** const field : {&p.name, &p.short_descr, &p.long_descr, &p.description,
							   &p.title, &p.sounds, &p.distant_snds}) {
		if(d->str == field) {
			return true;
		}
	}
	return false;
}

/* true se d->str punta a un campo testo di obj o a una sua extra description. */
[[nodiscard]] bool editor_str_in_obj(const descriptor_data* d, obj_data* obj) {
	for(char** const field : {&obj->name, &obj->description, &obj->short_description,
							   &obj->action_description}) {
		if(d->str == field) {
			return true;
		}
	}
	for(extra_descr_data* ed = obj->ex_description;
			ed != nullptr && ed->nMagicNumber == EXDESC_VALID_MAGIC; ed = ed->next) {
		if(d->str == &ed->description) {
			return true;
		}
	}
	return false;
}

/* true se d->str punta a un campo testo di rp: nome, descrizione, uscite
 * (descrizione e keyword) o extra description. */
[[nodiscard]] bool editor_str_in_room(const descriptor_data* d, room_data* rp) {
	if(d->str == &rp->name || d->str == &rp->description) {
		return true;
	}
	for(room_direction_data* const door : rp->dir_option) {
		if(door != nullptr && (d->str == &door->general_description || d->str == &door->keyword)) {
			return true;
		}
	}
	for(extra_descr_data* ed = rp->ex_description; ed != nullptr; ed = ed->next) {
		if(d->str == &ed->description || d->str == &ed->keyword) {
			return true;
		}
	}
	return false;
}

/* Stacca d->str dal bersaglio che sta per essere liberato. Con il testo
 * originale da parte (oedit/medit/redit) lo rimette nel campo, che poi
 * libera free_char/free_obj; senza, il testo scritto resta nel campo e
 * viene liberato con lui. In entrambi i casi d->str non punta piu' li'. */
void editor_drop_text(descriptor_data* d) {
	if(d->str_backup != nullptr) {
		EditorAbortText(d);
	}
	d->str = nullptr;
}

/* Riporta in gioco chi era in medit/oedit/forgia su un bersaglio sparito. */
void editor_leave_state(descriptor_data* d, const char* msg) {
	char_data* const editor = d->character;
	send_to_char(msg, editor);
	if(d->connected == CON_OBJ_FORGING) {
		descriptor_set_connected(d, CON_PLYNG);
		return;
	}
	oedit_snapshot_clear(editor);
	act("$n smette di $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.",
		false, editor, nullptr, nullptr, TO_ROOM);
	descriptor_set_connected(d, CON_PLYNG);
	editor->specials.position = POSITION_STANDING;
}

} // namespace

void EditorReleaseChar(struct char_data* victim) {
	if(victim == nullptr) {
		return;
	}
	for(descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		char_data* const editor = d->character;
		if(d->str != nullptr && editor_str_in_char(d, victim)) {
			editor_drop_text(d);
			if(editor != nullptr && editor != victim && editor->specials.mobedit != victim) {
				send_to_char("\n\rCio' su cui stavi scrivendo non esiste piu'.\n\r", editor);
			}
		}
		if(editor == nullptr || editor == victim || editor->specials.mobedit != victim) {
			continue;
		}
		editor->specials.mobedit = nullptr;
		if(d->connected == CON_MOB_EDITING) {
			editor_leave_state(d, "\n\rIl mob che stavi modificando non esiste piu'.\n\r");
		}
	}
}

void EditorReleaseObj(struct obj_data* obj) {
	if(obj == nullptr) {
		return;
	}
	for(descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		char_data* const editor = d->character;
		if(d->str != nullptr && editor_str_in_obj(d, obj)) {
			editor_drop_text(d);
			if(editor != nullptr && editor->specials.objedit != obj) {
				send_to_char("\n\rCio' su cui stavi scrivendo non esiste piu'.\n\r", editor);
			}
		}
		if(editor == nullptr || editor->specials.objedit != obj) {
			continue;
		}
		editor->specials.objedit = nullptr;
		if(d->connected == CON_OBJ_EDITING || d->connected == CON_OBJ_FORGING) {
			editor_leave_state(d, "\n\rL'oggetto che stavi modificando non esiste piu'.\n\r");
		}
	}
}

void EditorReleaseRoom(struct room_data* rp) {
	if(rp == nullptr) {
		return;
	}
	for(descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		char_data* const editor = d->character;
		/* redit lavora sempre su real_roomp(editor->in_room): va chiuso finche'
		 * il builder e' ancora nella stanza, prima che venga spostato altrove. */
		const bool in_redit = editor != nullptr && d->connected == CON_EDITING &&
							  editor->in_room == rp->number;
		if(d->str != nullptr && editor_str_in_room(d, rp)) {
			editor_drop_text(d);
			if(editor != nullptr && !in_redit) {
				send_to_char("\n\rCio' su cui stavi scrivendo non esiste piu'.\n\r", editor);
			}
		}
		if(in_redit) {
			editor_leave_state(d, "\n\rLa stanza che stavi modificando e' stata ricaricata o distrutta.\n\r");
		}
	}
}

} // namespace Alarmud
