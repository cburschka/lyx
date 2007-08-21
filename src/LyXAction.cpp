/**
 * \file LyXAction.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LyXAction.h"

#include "debug.h"
#include "FuncRequest.h"

#include "support/lstrings.h"

#include <boost/assert.hpp>


namespace lyx {

using support::split;
using support::trim;

using std::endl;
using std::string;
using std::ostream;


/*
     NAMING RULES FOR USER-COMMANDS
     Here's the set of rules to apply when a new command name is introduced:

     1) Use the object.event order. That is, use `word-forward'
	instead of `forward-word'.
     2) Don't introduce an alias for an already named object. Same for events.
     3) Forward movement or focus is called `forward' (not `right').
     4) Backward movement or focus is called `backward' (not `left').
     5) Upward movement of focus is called `up'.
     6) Downward movement is called `down'.
     7) The begin of an object is called `begin' (not `start').
     8) The end of an object is called `end'.

     (May 19 1996, 12:04, RvdK)
*/

LyXAction lyxaction;


void LyXAction::newFunc(kb_action action, string const & name,
			unsigned int attrib)
{
	lyx_func_map[name] = action;
	func_info tmpinfo;
	tmpinfo.name = name;
	tmpinfo.attrib = attrib;
	lyx_info_map[action] = tmpinfo;
}


void LyXAction::init()
{
	// This function was changed to use the array below in initalization
	// instead of calling newFunc numerous times because of compilation
	// times. Since the array is not static we get back the memory it
	// occupies after the init is completed. It compiles several
	// magnitudes faster.

	static bool init;
	if (init) return;

	struct ev_item {
		kb_action action;
		char const * name;
		unsigned int attrib;
	};

	ev_item const items[] = {
		{ LFUN_ACCENT_ACUTE, "accent-acute", Noop },
		{ LFUN_ACCENT_BREVE, "accent-breve", Noop },
		{ LFUN_ACCENT_CARON, "accent-caron", Noop },
		{ LFUN_ACCENT_CEDILLA, "accent-cedilla", Noop },
		{ LFUN_ACCENT_CIRCLE, "accent-circle", Noop },
		{ LFUN_ACCENT_CIRCUMFLEX, "accent-circumflex", Noop },
		{ LFUN_ACCENT_DOT, "accent-dot", Noop },
		{ LFUN_ACCENT_GRAVE, "accent-grave", Noop },
		{ LFUN_ACCENT_HUNGARIAN_UMLAUT, "accent-hungarian-umlaut", Noop },
		{ LFUN_ACCENT_MACRON, "accent-macron", Noop },
		{ LFUN_ACCENT_OGONEK, "accent-ogonek", Noop },
		{ LFUN_ACCENT_SPECIAL_CARON, "accent-special-caron", Noop },
		{ LFUN_ACCENT_TIE, "accent-tie", Noop },
		{ LFUN_ACCENT_TILDE, "accent-tilde", Noop },
		{ LFUN_ACCENT_UMLAUT, "accent-umlaut", Noop },
		{ LFUN_ACCENT_UNDERBAR, "accent-underbar", Noop },
		{ LFUN_ACCENT_UNDERDOT, "accent-underdot", Noop },
		{ LFUN_APPENDIX, "appendix", Noop },
		{ LFUN_BOOKMARK_GOTO, "bookmark-goto", NoBuffer },
		{ LFUN_BOOKMARK_SAVE, "bookmark-save", ReadOnly },
		{ LFUN_BOOKMARK_CLEAR, "bookmark-clear", NoBuffer },
		{ LFUN_BREAK_LINE, "break-line", Noop },
		{ LFUN_BREAK_PARAGRAPH, "break-paragraph", Noop },
		{ LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT, "break-paragraph-keep-layout", Noop },
		{ LFUN_BREAK_PARAGRAPH_SKIP, "break-paragraph-skip", Noop },
		{ LFUN_BUILD_PROGRAM, "build-program", ReadOnly },
		{ LFUN_BUFFER_AUTO_SAVE, "buffer-auto-save", Noop },
		{ LFUN_BUFFER_BEGIN, "buffer-begin", ReadOnly },
		{ LFUN_BUFFER_BEGIN_SELECT, "buffer-begin-select", ReadOnly },
		{ LFUN_BUFFER_CHILD_OPEN, "buffer-child-open", ReadOnly },
		{ LFUN_BUFFER_CHKTEX, "buffer-chktex", ReadOnly },
		{ LFUN_BUFFER_TOGGLE_COMPRESSION, "buffer-toggle-compression", Noop},
		{ LFUN_BUFFER_CLOSE, "buffer-close", ReadOnly },
		{ LFUN_BUFFER_END, "buffer-end", ReadOnly },
		{ LFUN_BUFFER_END_SELECT, "buffer-end-select", ReadOnly },
		{ LFUN_BUFFER_EXPORT, "buffer-export", ReadOnly },
		{ LFUN_BUFFER_EXPORT_CUSTOM, "buffer-export-custom", ReadOnly },
		{ LFUN_BUFFER_PRINT, "buffer-print", ReadOnly },
		{ LFUN_BUFFER_IMPORT, "buffer-import", NoBuffer },
		{ LFUN_BUFFER_NEW, "buffer-new", NoBuffer },
		{ LFUN_BUFFER_NEW_TEMPLATE,"buffer-new-template", NoBuffer },
		{ LFUN_BUFFER_RELOAD, "buffer-reload", ReadOnly },
		{ LFUN_BUFFER_SWITCH, "buffer-switch", NoBuffer | ReadOnly },
		{ LFUN_BUFFER_TOGGLE_READ_ONLY, "buffer-toggle-read-only", ReadOnly },
		{ LFUN_BUFFER_UPDATE, "buffer-update", ReadOnly },
		{ LFUN_BUFFER_VIEW, "buffer-view", ReadOnly },
		{ LFUN_BUFFER_WRITE, "buffer-write", ReadOnly },
		{ LFUN_BUFFER_WRITE_AS, "buffer-write-as", ReadOnly },
		{ LFUN_BUFFER_WRITE_ALL, "buffer-write-all", ReadOnly },
		{ LFUN_CANCEL, "cancel", NoBuffer },
		{ LFUN_CAPTION_INSERT, "caption-insert", Noop },
		{ LFUN_CHAR_BACKWARD, "char-backward", ReadOnly | NoUpdate},
		{ LFUN_CHAR_BACKWARD_SELECT, "backward-select", ReadOnly | SingleParUpdate },
		{ LFUN_CHAR_DELETE_BACKWARD, "delete-backward", SingleParUpdate },
		{ LFUN_CHAR_DELETE_FORWARD, "delete-forward", SingleParUpdate },
		{ LFUN_CHAR_FORWARD, "char-forward", ReadOnly | NoUpdate},
		{ LFUN_CHAR_FORWARD_SELECT, "forward-select", ReadOnly | SingleParUpdate },
		{ LFUN_CLIPBOARD_PASTE, "clipboard-paste", Noop },
		{ LFUN_COMMAND_EXECUTE, "command-execute", NoBuffer },
		{ LFUN_COMMAND_PREFIX, "command-prefix", NoBuffer },
		{ LFUN_COMMAND_SEQUENCE, "command-sequence", NoBuffer },
		{ LFUN_COPY, "copy", ReadOnly },
		{ LFUN_CUT, "cut", Noop },
		{ LFUN_DATE_INSERT, "date-insert", Noop },
		{ LFUN_DELETE_BACKWARD_SKIP, "delete-backward-skip", Noop },
		{ LFUN_DELETE_FORWARD_SKIP, "delete-forward-skip", Noop },
		{ LFUN_DEPTH_DECREMENT, "depth-decrement", Noop },
		{ LFUN_DEPTH_INCREMENT, "depth-increment", Noop },
		{ LFUN_DOTS_INSERT, "dots-insert", Noop },
		{ LFUN_DOWN, "down", ReadOnly | NoUpdate },
		{ LFUN_DOWN_SELECT, "down-select", ReadOnly | SingleParUpdate },
		{ LFUN_DROP_LAYOUTS_CHOICE, "drop-layouts-choice", ReadOnly },
		{ LFUN_END_OF_SENTENCE_PERIOD_INSERT, "end-of-sentence-period-insert", Noop },
		{ LFUN_ENVIRONMENT_INSERT, "environment-insert", Noop },
		{ LFUN_ERROR_NEXT, "error-next", ReadOnly },
		{ LFUN_ERT_INSERT, "ert-insert", Noop },
		{ LFUN_FILE_INSERT, "file-insert", Noop },
		{ LFUN_FILE_INSERT_PLAINTEXT, "file-insert-plaintext", Noop },
		{ LFUN_FILE_INSERT_PLAINTEXT_PARA, "file-insert-plaintext-para", Noop },
		{ LFUN_FILE_NEW, "file-new", NoBuffer },
		{ LFUN_FILE_OPEN, "file-open", NoBuffer },
		{ LFUN_FLOAT_INSERT, "float-insert", Noop },
		{ LFUN_FLOAT_WIDE_INSERT, "float-wide-insert", Noop },
		{ LFUN_WRAP_INSERT, "wrap-insert", Noop },
		{ LFUN_FONT_BOLD, "font-bold", Noop },
		{ LFUN_FONT_CODE, "font-code", Noop },
		{ LFUN_FONT_DEFAULT, "font-default", Noop },
		{ LFUN_FONT_EMPH, "font-emph", Noop },
		{ LFUN_FONT_FREE_APPLY, "font-free-apply", Noop },
		{ LFUN_FONT_FREE_UPDATE, "font-free-update", Noop },
		{ LFUN_FONT_NOUN, "font-noun", Noop },
		{ LFUN_FONT_ROMAN, "font-roman", Noop },
		{ LFUN_FONT_SANS, "font-sans", Noop },
		{ LFUN_FONT_FRAK, "font-frak", Noop },
		{ LFUN_FONT_ITAL, "font-ital", Noop },
		{ LFUN_FONT_SIZE, "font-size", Noop },
		{ LFUN_FONT_STATE, "font-state", ReadOnly },
		{ LFUN_FONT_UNDERLINE, "font-underline", Noop },
		{ LFUN_FOOTNOTE_INSERT, "footnote-insert", Noop },
		{ LFUN_HFILL_INSERT, "hfill-insert", Noop },
		{ LFUN_HELP_OPEN, "help-open", NoBuffer | Argument},
		{ LFUN_HTML_INSERT, "html-insert", Noop },
		{ LFUN_HYPHENATION_POINT_INSERT, "hyphenation-point-insert", Noop },
		{ LFUN_LIGATURE_BREAK_INSERT, "ligature-break-insert", Noop },
		{ LFUN_INDEX_INSERT, "index-insert", Noop },
		{ LFUN_INDEX_PRINT, "index-print", Noop },
		{ LFUN_KEYMAP_OFF, "keymap-off", ReadOnly },
		{ LFUN_KEYMAP_PRIMARY, "keymap-primary", ReadOnly },
		{ LFUN_KEYMAP_SECONDARY, "keymap-secondary", ReadOnly },
		{ LFUN_KEYMAP_TOGGLE, "keymap-toggle", ReadOnly },
		{ LFUN_LABEL_INSERT, "label-insert", Noop },
		{ LFUN_OPTIONAL_INSERT, "optional-insert", Noop },
		{ LFUN_BIBITEM_INSERT, "bibitem-insert", Noop },
		{ LFUN_CITATION_INSERT, "citation-insert", Noop },
		{ LFUN_BIBTEX_DATABASE_ADD, "bibtex-database-add", Noop },
		{ LFUN_BIBTEX_DATABASE_DEL, "bibtex-database-del", Noop },
		{ LFUN_LINE_INSERT, "line-insert", Noop },
		{ LFUN_PAGEBREAK_INSERT, "pagebreak-insert", Noop },
		{ LFUN_LANGUAGE, "language", Noop },
		{ LFUN_LAYOUT, "layout", Noop },
		{ LFUN_LAYOUT_PARAGRAPH, "layout-paragraph", ReadOnly },
		{ LFUN_LAYOUT_TABULAR, "layout-tabular", Noop },
		{ LFUN_LINE_BEGIN, "line-begin", ReadOnly | NoUpdate},
		{ LFUN_LINE_BEGIN_SELECT, "line-begin-select", ReadOnly | SingleParUpdate },
		{ LFUN_LINE_DELETE, "line-delete-forward", Noop }, // there is no line-delete-backward
		{ LFUN_LINE_END, "line-end", ReadOnly | NoUpdate},
		{ LFUN_LINE_END_SELECT, "line-end-select", ReadOnly | SingleParUpdate },
#if 0
		{ LFUN_LIST_INSERT, "list-insert", Noop },
#endif
		{ LFUN_LYX_QUIT, "lyx-quit", NoBuffer },
		{ LFUN_MARGINALNOTE_INSERT, "marginalnote-insert", Noop },
		{ LFUN_MARK_OFF, "mark-off", ReadOnly },
		{ LFUN_MARK_ON, "mark-on", ReadOnly },
		{ LFUN_MARK_TOGGLE, "mark-toggle", ReadOnly },
		{ LFUN_MATH_DELIM, "math-delim", Noop },
		{ LFUN_MATH_BIGDELIM, "math-bigdelim", Noop },
		{ LFUN_MATH_DISPLAY, "math-display", Noop },
		{ LFUN_MATH_INSERT, "math-insert", Noop },
		{ LFUN_MATH_SUBSCRIPT, "math-subscript", Noop },
		{ LFUN_MATH_SUPERSCRIPT, "math-superscript", Noop },
		{ LFUN_MATH_LIMITS, "math-limits", Noop },
		{ LFUN_MATH_MACRO, "math-macro", Noop },
		{ LFUN_MATH_MUTATE, "math-mutate", Noop },
		{ LFUN_MATH_SPACE, "math-space", Noop },
		{ LFUN_MATH_IMPORT_SELECTION, "math-import-selection", Noop },
		{ LFUN_MATH_MATRIX, "math-matrix", Noop },
		{ LFUN_MATH_MODE, "math-mode", Noop },
		{ LFUN_MATH_NONUMBER, "math-nonumber", Noop },
		{ LFUN_MATH_NUMBER, "math-number", Noop },
		{ LFUN_MATH_EXTERN, "math-extern", Noop },
		{ LFUN_MATH_SIZE, "math-size", Noop },
		{ LFUN_MENU_OPEN, "menu-open", NoBuffer },
		{ LFUN_MENU_SEPARATOR_INSERT, "menu-separator-insert", Noop },
		{ LFUN_META_PREFIX, "meta-prefix", NoBuffer },
		{ LFUN_BRANCH_INSERT, "branch-insert", Noop },
		{ LFUN_CHARSTYLE_INSERT, "charstyle-insert", Noop },
		{ LFUN_NOTE_INSERT, "note-insert", Noop },
		{ LFUN_BOX_INSERT, "box-insert", Noop },
		{ LFUN_NOTE_NEXT, "note-next", ReadOnly },
		{ LFUN_INSET_TOGGLE, "", ReadOnly },
		{ LFUN_NEXT_INSET_TOGGLE, "next-inset-toggle", ReadOnly },
		{ LFUN_ALL_INSETS_TOGGLE, "all-insets-toggle", ReadOnly },
		{ LFUN_PARAGRAPH_DOWN, "paragraph-down", ReadOnly | NoUpdate},
		{ LFUN_PARAGRAPH_DOWN_SELECT, "paragraph-down-select", ReadOnly },
		{ LFUN_PARAGRAPH_GOTO, "paragraph-goto", ReadOnly },
		{ LFUN_OUTLINE_UP, "outline-up", Noop },
		{ LFUN_OUTLINE_DOWN, "outline-down", Noop },
		{ LFUN_OUTLINE_IN, "outline-in", Noop },
		{ LFUN_OUTLINE_OUT, "outline-out", Noop },
		{ LFUN_PARAGRAPH_SPACING, "paragraph-spacing", Noop },
		{ LFUN_PARAGRAPH_UP, "paragraph-up", ReadOnly | NoUpdate},
		{ LFUN_PARAGRAPH_UP_SELECT, "paragraph-up-select", ReadOnly },
		{ LFUN_PASTE, "paste", Noop },
		{ LFUN_PREFERENCES_SAVE, "preferences-save", NoBuffer },
		{ LFUN_PRIMARY_SELECTION_PASTE, "primary-selection-paste", Noop },
		{ LFUN_QUOTE_INSERT, "quote-insert", Noop },
		{ LFUN_RECONFIGURE, "reconfigure", NoBuffer },
		{ LFUN_REDO, "redo", Noop },
		{ LFUN_LABEL_GOTO, "label-goto", ReadOnly },
		{ LFUN_REFERENCE_NEXT, "reference-next", ReadOnly },
		{ LFUN_SCREEN_DOWN, "screen-down", ReadOnly },
		{ LFUN_SCREEN_DOWN_SELECT, "screen-down-select", ReadOnly },
		{ LFUN_SCREEN_FONT_UPDATE, "screen-font-update", NoBuffer },
		{ LFUN_SCREEN_RECENTER, "screen-recenter", ReadOnly },
		{ LFUN_SCREEN_UP, "screen-up", ReadOnly },
		{ LFUN_SCREEN_UP_SELECT, "screen-up-select", ReadOnly },
		{ LFUN_SELF_INSERT, "self-insert", SingleParUpdate },
		{ LFUN_SPACE_INSERT, "space-insert", Noop },
		{ LFUN_SERVER_CHAR_AFTER, "server-char-after", ReadOnly },
		{ LFUN_SERVER_GET_FONT, "server-get-font", ReadOnly },
		{ LFUN_SERVER_GET_LAYOUT, "server-get-layout", ReadOnly },
		{ LFUN_SERVER_GET_NAME, "server-get-name", ReadOnly },
		{ LFUN_SERVER_GET_XY, "server-get-xy", ReadOnly },
		{ LFUN_SERVER_GOTO_FILE_ROW, "server-goto-file-row", ReadOnly },
		{ LFUN_SERVER_NOTIFY, "server-notify", ReadOnly },
		{ LFUN_SERVER_SET_XY, "server-set-xy", ReadOnly },
		{ LFUN_SET_COLOR, "set-color", ReadOnly | NoBuffer },
		{ LFUN_CELL_BACKWARD, "cell-backward", Noop },
		{ LFUN_CELL_FORWARD, "cell-forward", Noop },
		{ LFUN_CELL_SPLIT, "cell-split", Noop },
		{ LFUN_TABULAR_INSERT, "tabular-insert", Noop },
		{ LFUN_TABULAR_FEATURE, "tabular-feature", Noop },
#if 0
		{ LFUN_THEOREM_INSERT, "theorem-insert", Noop },
#endif
		{ LFUN_THESAURUS_ENTRY, "thesaurus-entry", ReadOnly },
		{ LFUN_TOC_INSERT, "toc-insert", Noop },
		{ LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR, "toggle-cursor-follows-scrollbar", ReadOnly },
		{ LFUN_UNDO, "undo", Noop },
		{ LFUN_UP, "up", ReadOnly | NoUpdate},
		{ LFUN_UP_SELECT, "up-select", ReadOnly | SingleParUpdate },
		{ LFUN_URL_INSERT, "url-insert", Noop },
		{ LFUN_VC_CHECK_IN, "vc-check-in", ReadOnly },
		{ LFUN_VC_CHECK_OUT, "vc-check-out", ReadOnly },
		{ LFUN_VC_REGISTER, "vc-register", ReadOnly },
		{ LFUN_VC_REVERT, "vc-revert", ReadOnly },
		{ LFUN_VC_UNDO_LAST, "vc-undo-last", ReadOnly },
		{ LFUN_WORD_BACKWARD, "word-backward", ReadOnly | NoUpdate},
		{ LFUN_WORD_BACKWARD_SELECT, "word-backward-select", ReadOnly | SingleParUpdate },
		{ LFUN_WORD_CAPITALIZE, "word-capitalize", Noop },
		{ LFUN_WORD_DELETE_BACKWARD, "word-delete-backward", Noop },
		{ LFUN_WORD_DELETE_FORWARD, "word-delete-forward", Noop },
		{ LFUN_WORD_FIND_BACKWARD, "word-find-backward", ReadOnly },
		{ LFUN_WORD_FIND_FORWARD, "word-find-forward", ReadOnly },
		{ LFUN_WORD_FORWARD, "word-forward", ReadOnly | NoUpdate},
		{ LFUN_WORD_FORWARD_SELECT, "word-forward-select", ReadOnly | SingleParUpdate },
		{ LFUN_WORD_LOWCASE, "word-lowcase", Noop },
		{ LFUN_WORD_SELECT, "word-select", ReadOnly },
		{ LFUN_WORD_UPCASE, "word-upcase", Noop },
		{ LFUN_MESSAGE, "message", NoBuffer },
		{ LFUN_CHARS_TRANSPOSE, "chars-transpose", Noop },
		{ LFUN_FLOAT_LIST, "float-list", Noop },
		{ LFUN_ESCAPE, "escape", ReadOnly },
		{ LFUN_CHANGES_TRACK, "changes-track", Noop },
		{ LFUN_CHANGES_OUTPUT, "changes-output", Noop },
		{ LFUN_CHANGE_NEXT, "change-next", ReadOnly },
		{ LFUN_CHANGES_MERGE, "changes-merge", Noop },
		{ LFUN_CHANGE_ACCEPT, "change-accept", Noop },
		{ LFUN_CHANGE_REJECT, "change-reject", Noop },
		{ LFUN_ALL_CHANGES_ACCEPT, "all-changes-accept", Noop },
		{ LFUN_ALL_CHANGES_REJECT, "all-changes-reject", Noop },
		{ LFUN_DIALOG_SHOW, "dialog-show", NoBuffer },
		{ LFUN_DIALOG_SHOW_NEW_INSET, "dialog-show-new-inset", Noop },
		{ LFUN_DIALOG_UPDATE, "dialog-update", NoBuffer },
		{ LFUN_DIALOG_HIDE, "dialog-hide", NoBuffer },
		{ LFUN_DIALOG_TOGGLE, "dialog-toggle", NoBuffer },
		{ LFUN_DIALOG_DISCONNECT_INSET, "dialog-disconnect-inset", Noop },
		{ LFUN_INSET_APPLY, "inset-apply", Noop },
		{ LFUN_INSET_DISSOLVE, "inset-dissolve", Noop },
		{ LFUN_INSET_INSERT, "inset-insert", Noop },
		{ LFUN_INSET_MODIFY, "", Noop },
		{ LFUN_INSET_DIALOG_UPDATE, "", Noop },
		{ LFUN_INSET_SETTINGS, "inset-settings", ReadOnly },
		{ LFUN_PARAGRAPH_PARAMS, "paragraph-params", Noop },
		{ LFUN_PARAGRAPH_PARAMS_APPLY, "paragraph-params-apply", Noop },
		{ LFUN_PARAGRAPH_UPDATE, "", Noop },
		{ LFUN_EXTERNAL_EDIT, "external-edit", Noop },
		{ LFUN_GRAPHICS_EDIT, "graphics-edit", Noop },
		{ LFUN_REPEAT, "repeat", NoBuffer },
		{ LFUN_WORD_FIND, "word-find", ReadOnly },
		{ LFUN_WORD_REPLACE, "word-replace", Noop },
		{ LFUN_BUFFER_LANGUAGE, "buffer-language", Noop },
		{ LFUN_TEXTCLASS_APPLY, "textclass-apply", Noop },
		{ LFUN_TEXTCLASS_LOAD, "textclass-load", Noop },
		{ LFUN_BUFFER_SAVE_AS_DEFAULT, "buffer-save-as-default", Noop },
		{ LFUN_BUFFER_PARAMS_APPLY, "buffer-params-apply", Noop },
		{ LFUN_LYXRC_APPLY, "lyxrc-apply", NoBuffer },
		{ LFUN_INSET_REFRESH, "", Noop },
		{ LFUN_BUFFER_NEXT, "buffer-next", ReadOnly },
		{ LFUN_BUFFER_PREVIOUS, "buffer-previous", ReadOnly },
		{ LFUN_WORDS_COUNT, "words-count", ReadOnly },
		{ LFUN_FINISHED_RIGHT, "", ReadOnly },
		{ LFUN_FINISHED_LEFT, "", ReadOnly },
		{ LFUN_MOUSE_PRESS, "", ReadOnly },
		{ LFUN_MOUSE_MOTION, "", ReadOnly | SingleParUpdate },
		{ LFUN_MOUSE_RELEASE, "", ReadOnly },
		{ LFUN_MOUSE_DOUBLE, "", ReadOnly },
		{ LFUN_MOUSE_TRIPLE, "", ReadOnly },
		{ LFUN_PARAGRAPH_MOVE_DOWN, "paragraph-move-down", Noop },
		{ LFUN_PARAGRAPH_MOVE_UP, "paragraph-move-up", Noop },
		{ LFUN_WINDOW_NEW, "window-new", NoBuffer },
		{ LFUN_WINDOW_CLOSE, "window-close", NoBuffer },
		{ LFUN_UNICODE_INSERT, "unicode-insert", Noop },
		{ LFUN_TOOLBAR_TOGGLE, "toolbar-toggle", NoBuffer },
		{ LFUN_NOMENCL_INSERT, "nomencl-insert", Noop },
		{ LFUN_NOMENCL_PRINT, "nomencl-print", Noop },
		{ LFUN_CLEARPAGE_INSERT, "clearpage-insert", Noop },
		{ LFUN_CLEARDOUBLEPAGE_INSERT, "cleardoublepage-insert", Noop },
		{ LFUN_LISTING_INSERT, "listing-insert", Noop },

		{ LFUN_NOACTION, "", Noop }
	};

	for (int i = 0; items[i].action != LFUN_NOACTION; ++i) {
		newFunc(items[i].action, items[i].name, items[i].attrib);
	}

	init = true;
}


LyXAction::LyXAction()
{
	init();
}


// Returns an action tag from a string.
FuncRequest LyXAction::lookupFunc(string const & func) const
{
	string const func2 = trim(func);

	if (func2.empty()) {
		return FuncRequest(LFUN_NOACTION);
	}

	string cmd;
	string const arg = split(func2, cmd, ' ');

	func_map::const_iterator fit = lyx_func_map.find(cmd);

	return fit != lyx_func_map.end() ? FuncRequest(fit->second, arg) : FuncRequest(LFUN_UNKNOWN_ACTION);
}


string const LyXAction::getActionName(kb_action action) const
{
	info_map::const_iterator const it = lyx_info_map.find(action);
	return it != lyx_info_map.end() ? it->second.name : string();
}


bool LyXAction::funcHasFlag(kb_action action,
			    LyXAction::func_attrib flag) const
{
	info_map::const_iterator ici = lyx_info_map.find(action);

	if (ici == lyx_info_map.end()) {
		lyxerr << "action: " << action << " is not known." << endl;
		BOOST_ASSERT(false);
	}

	return ici->second.attrib & flag;
}


LyXAction::const_func_iterator LyXAction::func_begin() const
{
	return lyx_func_map.begin();
}


LyXAction::const_func_iterator LyXAction::func_end() const
{
	return lyx_func_map.end();
}


ostream & operator<<(ostream & o, kb_action action)
{
	return o << int(action);
}


} // namespace lyx
