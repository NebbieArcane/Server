/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __EDITOR_TERM_HPP
#define __EDITOR_TERM_HPP
/***************************  System  include ************************************/
#include <string>
/***************************  Local    include ************************************/
namespace Alarmud {

struct char_data;
struct descriptor_data;

/** Clear screen only in VT100 mode; otherwise emit a newline separator. */
void EditorClear(struct char_data* ch);
/** Cursor positioning only in VT100 mode; no output otherwise. */
void EditorGoto(struct char_data* ch, int row, int col);
/** Pad with spaces up to column in linear (non-VT100) mode; no-op in VT100. */
void EditorPadToColumn(struct char_data* ch, int already, int column);
/** Append a padded two-column line (left width, then right) ending with \\n\\r.
 *  If right is null or empty, appends only the left column. */
void EditorAppendTwoColumns(std::string& out, const char* left, const char* right, int left_width = 45);
/** Send an editor prompt. VT100: message only (byte-identical). Linear mode:
 *  strip leading newlines/CRs (they only space text after a VT100 home-clear),
 *  then append a newline if the message does not already end with one, so
 *  Mudlet and other modern clients flush the prompt immediately. */
void EditorPrompt(struct char_data* ch, const char* msg);
/** Start the multi-line text editor on *field for an oedit/medit/redit text.
 *  The current text is kept aside: ending with a lone @ before typing anything
 *  restores it instead of leaving the field empty. */
void EditorBeginText(struct char_data* ch, char** field);
/** Called when the multi-line editor terminates: restores the saved text if
 *  nothing was typed, otherwise drops it. Returns true if it restored. */
bool EditorFinishText(struct descriptor_data* d);
/** Descriptor closing while editing: put the saved text back. */
void EditorAbortText(struct descriptor_data* d);
/** True when the descriptor is inside oedit, medit or redit. */
bool IsEditorState(int connected);

} // namespace Alarmud
#endif // __EDITOR_TERM_HPP
