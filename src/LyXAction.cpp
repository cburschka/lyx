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

#include "support/debug.h"
#include "FuncRequest.h"

#include "support/lstrings.h"

#include <boost/assert.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {

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
			unsigned int attrib, LyXAction::func_type type)
{
	lyx_func_map[name] = action;
	func_info tmpinfo;
	tmpinfo.name = name;
	tmpinfo.attrib = attrib;
	tmpinfo.type = type;
	lyx_info_map[action] = tmpinfo;
}


// Needed for LFUNs documentation to be accepted, since doxygen won't take
// \var inside functions.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
void LyXAction::init()
#endif
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
		func_type type;
	};

	ev_item const items[] = {
		{ LFUN_ACCENT_ACUTE, "accent-acute", Noop, Layout },
		{ LFUN_ACCENT_BREVE, "accent-breve", Noop, Layout },
		{ LFUN_ACCENT_CARON, "accent-caron", Noop, Layout },
		{ LFUN_ACCENT_CEDILLA, "accent-cedilla", Noop, Layout },
		{ LFUN_ACCENT_CIRCLE, "accent-circle", Noop, Layout },
		{ LFUN_ACCENT_CIRCUMFLEX, "accent-circumflex", Noop, Layout },
		{ LFUN_ACCENT_DOT, "accent-dot", Noop, Layout },
		{ LFUN_ACCENT_GRAVE, "accent-grave", Noop, Layout },
		{ LFUN_ACCENT_HUNGARIAN_UMLAUT, "accent-hungarian-umlaut", Noop, Layout },
		{ LFUN_ACCENT_MACRON, "accent-macron", Noop, Layout },
		{ LFUN_ACCENT_OGONEK, "accent-ogonek", Noop, Layout },
		{ LFUN_ACCENT_SPECIAL_CARON, "accent-special-caron", Noop, Layout },
		{ LFUN_ACCENT_TIE, "accent-tie", Noop, Layout },
		{ LFUN_ACCENT_TILDE, "accent-tilde", Noop, Layout },
		{ LFUN_ACCENT_UMLAUT, "accent-umlaut", Noop, Layout },
		{ LFUN_ACCENT_UNDERBAR, "accent-underbar", Noop, Layout },
		{ LFUN_ACCENT_UNDERDOT, "accent-underdot", Noop, Layout },

		{ LFUN_APPENDIX, "appendix", Noop, Edit },

		{ LFUN_BREAK_PARAGRAPH, "break-paragraph", Noop, Edit },
		{ LFUN_BREAK_PARAGRAPH_SKIP, "break-paragraph-skip", Noop, Edit },

		{ LFUN_CAPTION_INSERT, "caption-insert", Noop, Edit },
		{ LFUN_DATE_INSERT, "date-insert", Noop, Edit },
		{ LFUN_FOOTNOTE_INSERT, "footnote-insert", Noop, Edit },
		{ LFUN_ERT_INSERT, "ert-insert", Noop, Edit },
		{ LFUN_FLOAT_INSERT, "float-insert", Noop, Edit },
		{ LFUN_FLOAT_WIDE_INSERT, "float-wide-insert", Noop, Edit },
		{ LFUN_WRAP_INSERT, "wrap-insert", Noop, Edit },
		{ LFUN_HFILL_INSERT, "hfill-insert", Noop, Edit },
		{ LFUN_OPTIONAL_INSERT, "optional-insert", Noop, Edit },
		{ LFUN_LINE_INSERT, "line-insert", Noop, Edit },
		{ LFUN_NEWPAGE_INSERT, "newpage-insert", Noop, Edit },
		{ LFUN_PAGEBREAK_INSERT, "pagebreak-insert", Noop, Edit },
		{ LFUN_MARGINALNOTE_INSERT, "marginalnote-insert", Noop, Edit },
		{ LFUN_UNICODE_INSERT, "unicode-insert", Noop, Edit },
		{ LFUN_LISTING_INSERT, "listing-insert", Noop, Edit },
		{ LFUN_QUOTE_INSERT, "quote-insert", Noop, Edit },
		{ LFUN_INFO_INSERT, "info-insert", Noop, Edit },
		{ LFUN_BRANCH_INSERT, "branch-insert", Noop, Edit },
		{ LFUN_BOX_INSERT, "box-insert", Noop, Edit },
		{ LFUN_FLEX_INSERT, "flex-insert", Noop, Edit },
		{ LFUN_SELF_INSERT, "self-insert", SingleParUpdate, Hidden },
		{ LFUN_SPACE_INSERT, "space-insert", Noop, Edit },
		{ LFUN_HYPERLINK_INSERT, "href-insert", Noop, Edit },
		{ LFUN_SPECIALCHAR_INSERT, "specialchar-insert", Noop, Edit },
		{ LFUN_CLEARPAGE_INSERT, "clearpage-insert", Noop, Edit },
		{ LFUN_CLEARDOUBLEPAGE_INSERT, "cleardoublepage-insert", Noop, Edit },
		{ LFUN_TOC_INSERT, "toc-insert", Noop, Edit },
#if 0
		{ LFUN_LIST_INSERT, "list-insert", Noop, Edit },
#endif
#if 0
		{ LFUN_THEOREM_INSERT, "theorem-insert", Noop, Edit },
#endif

		{ LFUN_DELETE_BACKWARD_SKIP, "delete-backward-skip", Noop, Edit },
		{ LFUN_DELETE_FORWARD_SKIP, "delete-forward-skip", Noop, Edit },
		{ LFUN_DOWN, "down", ReadOnly | NoUpdate, Edit },
		{ LFUN_DOWN_SELECT, "down-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_UP, "up", ReadOnly | NoUpdate, Edit },
		{ LFUN_UP_SELECT, "up-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_SCREEN_DOWN, "screen-down", ReadOnly, Edit },
		{ LFUN_SCREEN_DOWN_SELECT, "screen-down-select", ReadOnly, Edit },
		{ LFUN_SCREEN_FONT_UPDATE, "screen-font-update", NoBuffer, Layout },
		{ LFUN_SCROLL, "scroll", ReadOnly, Edit },
		{ LFUN_SCREEN_RECENTER, "screen-recenter", ReadOnly, Edit },
		{ LFUN_SCREEN_UP, "screen-up", ReadOnly, Edit },
		{ LFUN_SCREEN_UP_SELECT, "screen-up-select", ReadOnly, Edit },

		{ LFUN_CHAR_BACKWARD, "char-backward", ReadOnly | NoUpdate, Edit },
		{ LFUN_CHAR_BACKWARD_SELECT, "char-backward-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_CHAR_DELETE_BACKWARD, "delete-backward", SingleParUpdate, Edit },
		{ LFUN_CHAR_DELETE_FORWARD, "delete-forward", SingleParUpdate, Edit },
		{ LFUN_CHAR_FORWARD, "char-forward", ReadOnly | NoUpdate, Edit },
		{ LFUN_CHAR_FORWARD_SELECT, "char-forward-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_CHAR_LEFT, "char-left", ReadOnly | NoUpdate, Edit },
		{ LFUN_CHAR_LEFT_SELECT, "char-left-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_CHAR_RIGHT, "char-right", ReadOnly | NoUpdate, Edit },
		{ LFUN_CHAR_RIGHT_SELECT, "char-right-select", ReadOnly | SingleParUpdate, Edit },

		{ LFUN_WORD_BACKWARD, "word-backward", ReadOnly | NoUpdate, Edit },
		{ LFUN_WORD_BACKWARD_SELECT, "word-backward-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_WORD_CAPITALIZE, "word-capitalize", Noop, Edit },
		{ LFUN_WORD_DELETE_BACKWARD, "word-delete-backward", Noop, Edit },
		{ LFUN_WORD_DELETE_FORWARD, "word-delete-forward", Noop, Edit },
		{ LFUN_WORD_FIND_BACKWARD, "word-find-backward", ReadOnly, Edit },
		{ LFUN_WORD_FIND_FORWARD, "word-find-forward", ReadOnly, Edit },
		{ LFUN_WORD_FORWARD, "word-forward", ReadOnly | NoUpdate, Edit },
		{ LFUN_WORD_FORWARD_SELECT, "word-forward-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_WORD_LEFT, "word-left", ReadOnly | NoUpdate, Edit },
		{ LFUN_WORD_LEFT_SELECT, "word-left-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_WORD_LOWCASE, "word-lowcase", Noop, Edit },
		{ LFUN_WORD_RIGHT, "word-right", ReadOnly | NoUpdate, Edit },
		{ LFUN_WORD_RIGHT_SELECT, "word-right-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_WORD_SELECT, "word-select", ReadOnly, Edit },
		{ LFUN_WORD_UPCASE, "word-upcase", Noop, Edit },
		{ LFUN_WORD_FIND, "word-find", ReadOnly, Edit },
		{ LFUN_WORD_REPLACE, "word-replace", Noop, Edit },

		{ LFUN_LINE_BEGIN, "line-begin", ReadOnly | NoUpdate, Edit },
		{ LFUN_LINE_BEGIN_SELECT, "line-begin-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_LINE_BREAK, "line-break", Noop, Edit },
		{ LFUN_LINE_DELETE, "line-delete-forward", Noop, Edit }, // there is no line-delete-backward
		{ LFUN_LINE_END, "line-end", ReadOnly | NoUpdate, Edit },
		{ LFUN_LINE_END_SELECT, "line-end-select", ReadOnly | SingleParUpdate, Edit },
		{ LFUN_NEW_LINE, "new-line", Noop, Edit },

		{ LFUN_PARAGRAPH_MOVE_DOWN, "paragraph-move-down", Noop, Edit },
		{ LFUN_PARAGRAPH_MOVE_UP, "paragraph-move-up", Noop, Edit },

		{ LFUN_COPY, "copy", ReadOnly, Edit },
		{ LFUN_CUT, "cut", Noop, Edit },
		{ LFUN_PASTE, "paste", Noop, Edit },
		{ LFUN_CLIPBOARD_PASTE, "clipboard-paste", Noop, Edit },
		{ LFUN_PRIMARY_SELECTION_PASTE, "primary-selection-paste", Noop, Edit },
		{ LFUN_UNDO, "undo", Noop, Edit },
		{ LFUN_REDO, "redo", Noop, Edit },
		{ LFUN_REPEAT, "repeat", NoBuffer, Edit },
		{ LFUN_CHARS_TRANSPOSE, "chars-transpose", Noop, Edit },

		{ LFUN_DEPTH_DECREMENT, "depth-decrement", Noop, Edit },
		{ LFUN_DEPTH_INCREMENT, "depth-increment", Noop, Edit },
		{ LFUN_ENVIRONMENT_INSERT, "environment-insert", Noop, Edit },

		{ LFUN_FONT_BOLD, "font-bold", Noop, Layout },
		{ LFUN_FONT_TYPEWRITER, "font-typewriter", Noop, Layout },
		{ LFUN_FONT_DEFAULT, "font-default", Noop, Layout },
		{ LFUN_FONT_EMPH, "font-emph", Noop, Layout },
		{ LFUN_FONT_FREE_APPLY, "font-free-apply", Noop, Layout },
		{ LFUN_FONT_FREE_UPDATE, "font-free-update", Noop, Layout },
		{ LFUN_FONT_NOUN, "font-noun", Noop, Layout },
		{ LFUN_FONT_ROMAN, "font-roman", Noop, Layout },
		{ LFUN_FONT_SANS, "font-sans", Noop, Layout },
		{ LFUN_FONT_FRAK, "font-frak", Noop, Layout },
		{ LFUN_FONT_ITAL, "font-ital", Noop, Layout },
		{ LFUN_FONT_SIZE, "font-size", Noop, Layout },
		{ LFUN_FONT_STATE, "font-state", ReadOnly, Layout },
		{ LFUN_FONT_UNDERLINE, "font-underline", Noop, Layout },

		{ LFUN_INDEX_INSERT, "index-insert", Noop, Edit },
		{ LFUN_INDEX_PRINT, "index-print", Noop, Edit },

		{ LFUN_NOMENCL_INSERT, "nomencl-insert", Noop, Edit },
		{ LFUN_NOMENCL_PRINT, "nomencl-print", Noop, Edit },

		{ LFUN_NOTE_INSERT, "note-insert", Noop, Edit },
		{ LFUN_NOTE_NEXT, "note-next", ReadOnly, Edit },

		{ LFUN_BIBITEM_INSERT, "bibitem-insert", Noop, Edit },
		{ LFUN_CITATION_INSERT, "citation-insert", Noop, Edit },
		{ LFUN_BIBTEX_DATABASE_ADD, "bibtex-database-add", Noop, Edit },
		{ LFUN_BIBTEX_DATABASE_DEL, "bibtex-database-del", Noop, Edit },

		{ LFUN_LAYOUT, "layout", Noop, Layout },
		{ LFUN_LAYOUT_PARAGRAPH, "layout-paragraph", ReadOnly, Layout },
		{ LFUN_LAYOUT_TABULAR, "layout-tabular", Noop, Layout },
		{ LFUN_DROP_LAYOUTS_CHOICE, "drop-layouts-choice", ReadOnly, Layout },
		{ LFUN_LAYOUT_MODULES_CLEAR, "layout-modules-clear", Noop, Layout },
		{ LFUN_LAYOUT_MODULE_ADD, "layout-module-add", Noop, Layout },
		{ LFUN_LAYOUT_RELOAD, "layout-reload", Noop, Layout },
		{ LFUN_TEXTCLASS_APPLY, "textclass-apply", Noop, Layout },
		{ LFUN_TEXTCLASS_LOAD, "textclass-load", Noop, Layout },

		{ LFUN_MARK_OFF, "mark-off", ReadOnly, Edit },
		{ LFUN_MARK_ON, "mark-on", ReadOnly, Edit },
		{ LFUN_MARK_TOGGLE, "mark-toggle", ReadOnly, Edit },
		{ LFUN_MATH_DELIM, "math-delim", Noop, Math },
		{ LFUN_MATH_BIGDELIM, "math-bigdelim", Noop, Math },
		{ LFUN_MATH_DISPLAY, "math-display", Noop, Math },
		{ LFUN_MATH_INSERT, "math-insert", Noop, Math },
		{ LFUN_MATH_SUBSCRIPT, "math-subscript", Noop, Math },
		{ LFUN_MATH_SUPERSCRIPT, "math-superscript", Noop, Math },
		{ LFUN_MATH_LIMITS, "math-limits", Noop, Math },
		{ LFUN_MATH_MACRO, "math-macro", Noop, Math },
		{ LFUN_MATH_MUTATE, "math-mutate", Noop, Math },
		{ LFUN_MATH_SPACE, "math-space", Noop, Math },
		{ LFUN_MATH_IMPORT_SELECTION, "math-import-selection", Noop, Math },
		{ LFUN_MATH_MATRIX, "math-matrix", Noop, Math },
		{ LFUN_MATH_MODE, "math-mode", Noop, Math },
		{ LFUN_MATH_NUMBER_LINE_TOGGLE, "math-number-line-toggle", Noop, Math },
		{ LFUN_MATH_NUMBER_TOGGLE, "math-number-toggle", Noop, Math },
		{ LFUN_MATH_EXTERN, "math-extern", Noop, Math },
		{ LFUN_MATH_SIZE, "math-size", Noop, Math },
		{ LFUN_MATH_MACRO_UNFOLD, "math-macro-unfold", ReadOnly | SingleParUpdate, Math },
		{ LFUN_MATH_MACRO_FOLD, "math-macro-fold", ReadOnly | SingleParUpdate, Math },
		{ LFUN_MATH_MACRO_ADD_PARAM, "math-macro-add-param", Noop, Math },
		{ LFUN_MATH_MACRO_REMOVE_PARAM, "math-macro-remove-param", Noop, Math },
		{ LFUN_MATH_MACRO_APPEND_GREEDY_PARAM, "math-macro-append-greedy-param", Noop, Math },
		{ LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM, "math-macro-remove-greedy-param", Noop, Math },
		{ LFUN_MATH_MACRO_MAKE_OPTIONAL, "math-macro-make-optional", Noop, Math },
		{ LFUN_MATH_MACRO_MAKE_NONOPTIONAL, "math-macro-make-nonoptional", Noop, Math },
		{ LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM, "math-macro-add-optional-param", Noop, Math },
		{ LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM, "math-macro-remove-optional-param", Noop, Math },
		{ LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM, "math-macro-add-greedy-optional-param", Noop, Math },
		{ LFUN_IN_MATHMACROTEMPLATE, "in-mathmacrotemplate", Noop, Math },

		{ LFUN_PARAGRAPH_DOWN, "paragraph-down", ReadOnly | NoUpdate, Edit },
		{ LFUN_PARAGRAPH_DOWN_SELECT, "paragraph-down-select", ReadOnly, Edit },
		{ LFUN_PARAGRAPH_GOTO, "paragraph-goto", ReadOnly, Edit },
		{ LFUN_OUTLINE_UP, "outline-up", Noop, Edit },
		{ LFUN_OUTLINE_DOWN, "outline-down", Noop, Edit },
		{ LFUN_OUTLINE_IN, "outline-in", Noop, Edit },
		{ LFUN_OUTLINE_OUT, "outline-out", Noop, Edit },

		{ LFUN_PARAGRAPH_SPACING, "paragraph-spacing", Noop, Edit },
		{ LFUN_PARAGRAPH_UP, "paragraph-up", ReadOnly | NoUpdate, Edit },
		{ LFUN_PARAGRAPH_UP_SELECT, "paragraph-up-select", ReadOnly, Edit },

		{ LFUN_EXTERNAL_EDIT, "external-edit", Noop, Edit },
		{ LFUN_GRAPHICS_EDIT, "graphics-edit", Noop, Edit },

		{ LFUN_CELL_BACKWARD, "cell-backward", Noop, Edit },
		{ LFUN_CELL_FORWARD, "cell-forward", Noop, Edit },
		{ LFUN_CELL_SPLIT, "cell-split", Noop, Edit },
		{ LFUN_TABULAR_INSERT, "tabular-insert", Noop, Edit },
		{ LFUN_TABULAR_FEATURE, "tabular-feature", Noop, Edit },

		{ LFUN_VC_CHECK_IN, "vc-check-in", ReadOnly, System },
		{ LFUN_VC_CHECK_OUT, "vc-check-out", ReadOnly, System },
		{ LFUN_VC_REGISTER, "vc-register", ReadOnly, System },
		{ LFUN_VC_REVERT, "vc-revert", ReadOnly, System },
		{ LFUN_VC_UNDO_LAST, "vc-undo-last", ReadOnly, System },

		{ LFUN_CHANGES_TRACK, "changes-track", Noop, Edit },
		{ LFUN_CHANGES_OUTPUT, "changes-output", Noop, Edit },
		{ LFUN_CHANGE_NEXT, "change-next", ReadOnly, Edit },
		{ LFUN_CHANGES_MERGE, "changes-merge", Noop, Edit },
		{ LFUN_CHANGE_ACCEPT, "change-accept", Noop, Edit },
		{ LFUN_CHANGE_REJECT, "change-reject", Noop, Edit },
		{ LFUN_ALL_CHANGES_ACCEPT, "all-changes-accept", Noop, Edit },
		{ LFUN_ALL_CHANGES_REJECT, "all-changes-reject", Noop, Edit },

		{ LFUN_INSET_APPLY, "inset-apply", Noop, Edit },
		{ LFUN_INSET_DISSOLVE, "inset-dissolve", Noop, Edit },
		{ LFUN_INSET_INSERT, "inset-insert", Noop, Edit },
		{ LFUN_INSET_MODIFY, "", Noop, Hidden },
		{ LFUN_INSET_DIALOG_UPDATE, "", Noop, Hidden },
		{ LFUN_INSET_SETTINGS, "inset-settings", ReadOnly, Edit },
		{ LFUN_INSET_REFRESH, "", Noop, Hidden },
		{ LFUN_NEXT_INSET_TOGGLE, "next-inset-toggle", ReadOnly, Edit },
		{ LFUN_INSET_TOGGLE, "", ReadOnly, Hidden },
		{ LFUN_ALL_INSETS_TOGGLE, "all-insets-toggle", ReadOnly, Edit },

		{ LFUN_PARAGRAPH_PARAMS, "paragraph-params", Noop, Edit },
		{ LFUN_PARAGRAPH_PARAMS_APPLY, "paragraph-params-apply", Noop, Edit },
		{ LFUN_PARAGRAPH_UPDATE, "", Noop, Hidden },

		{ LFUN_FINISHED_FORWARD, "", ReadOnly, Hidden },
		{ LFUN_FINISHED_BACKWARD, "", ReadOnly, Hidden },
		{ LFUN_FINISHED_RIGHT, "", ReadOnly, Hidden },
		{ LFUN_FINISHED_LEFT, "", ReadOnly, Hidden },

		{ LFUN_LANGUAGE, "language", Noop, Edit },

		{ LFUN_LABEL_GOTO, "label-goto", ReadOnly, Edit },
		{ LFUN_LABEL_INSERT, "label-insert", Noop, Edit },
		{ LFUN_REFERENCE_NEXT, "reference-next", ReadOnly, Edit },

		{ LFUN_BOOKMARK_GOTO, "bookmark-goto", NoBuffer, Edit },
		{ LFUN_BOOKMARK_SAVE, "bookmark-save", ReadOnly, Edit },
		{ LFUN_BOOKMARK_CLEAR, "bookmark-clear", NoBuffer, Edit },

		{ LFUN_ERROR_NEXT, "error-next", ReadOnly, Edit },
		{ LFUN_HELP_OPEN, "help-open", NoBuffer | Argument, Buffer },
		{ LFUN_LYX_QUIT, "lyx-quit", NoBuffer, Buffer },
		{ LFUN_TOOLBAR_TOGGLE, "toolbar-toggle", NoBuffer, Edit },
		{ LFUN_MENU_OPEN, "menu-open", NoBuffer, Buffer },

		{ LFUN_WINDOW_NEW, "window-new", NoBuffer, Buffer },
		{ LFUN_WINDOW_CLOSE, "window-close", NoBuffer, Buffer },

		{ LFUN_DIALOG_SHOW, "dialog-show", NoBuffer, Edit },
		{ LFUN_DIALOG_SHOW_NEW_INSET, "dialog-show-new-inset", Noop, Edit },
		{ LFUN_DIALOG_UPDATE, "dialog-update", NoBuffer, Edit },
		{ LFUN_DIALOG_HIDE, "dialog-hide", NoBuffer, Edit },
		{ LFUN_DIALOG_TOGGLE, "dialog-toggle", NoBuffer, Edit },
		{ LFUN_DIALOG_DISCONNECT_INSET, "dialog-disconnect-inset", Noop, Edit },

		{ LFUN_MOUSE_PRESS, "", ReadOnly, Hidden },
		{ LFUN_MOUSE_MOTION, "", ReadOnly | SingleParUpdate, Hidden },
		{ LFUN_MOUSE_RELEASE, "", ReadOnly, Hidden },
		{ LFUN_MOUSE_DOUBLE, "", ReadOnly, Hidden },
		{ LFUN_MOUSE_TRIPLE, "", ReadOnly, Hidden },

		{ LFUN_KEYMAP_OFF, "keymap-off", ReadOnly, Edit },
		{ LFUN_KEYMAP_PRIMARY, "keymap-primary", ReadOnly, Edit },
		{ LFUN_KEYMAP_SECONDARY, "keymap-secondary", ReadOnly, Edit },
		{ LFUN_KEYMAP_TOGGLE, "keymap-toggle", ReadOnly, Edit },

		{ LFUN_MESSAGE, "message", NoBuffer, System },
		{ LFUN_FLOAT_LIST, "float-list", Noop, Edit },
		{ LFUN_ESCAPE, "escape", ReadOnly, Edit },

		{ LFUN_SERVER_CHAR_AFTER, "server-char-after", ReadOnly, System },
		{ LFUN_SERVER_GET_FONT, "server-get-font", ReadOnly, System },
		{ LFUN_SERVER_GET_LAYOUT, "server-get-layout", ReadOnly, System },
		{ LFUN_SERVER_GET_NAME, "server-get-name", ReadOnly, System },
		{ LFUN_SERVER_GET_XY, "server-get-xy", ReadOnly, System },
		{ LFUN_SERVER_GOTO_FILE_ROW, "server-goto-file-row", ReadOnly, System },
		{ LFUN_SERVER_NOTIFY, "server-notify", ReadOnly, System },
		{ LFUN_SERVER_SET_XY, "server-set-xy", ReadOnly, System },

		{ LFUN_BUILD_PROGRAM, "build-program", ReadOnly, Buffer },

		{ LFUN_BUFFER_AUTO_SAVE, "buffer-auto-save", Noop, Buffer },
		{ LFUN_BUFFER_BEGIN, "buffer-begin", ReadOnly, Edit },
		{ LFUN_BUFFER_BEGIN_SELECT, "buffer-begin-select", ReadOnly, Edit },
		{ LFUN_BUFFER_CHILD_OPEN, "buffer-child-open", ReadOnly, Buffer },
		{ LFUN_BUFFER_CHKTEX, "buffer-chktex", ReadOnly, Buffer },
		{ LFUN_BUFFER_TOGGLE_COMPRESSION, "buffer-toggle-compression", Noop, Buffer },
		{ LFUN_BUFFER_TOGGLE_EMBEDDING, "buffer-toggle-embedding", Noop, Buffer },
		{ LFUN_BUFFER_CLOSE, "buffer-close", ReadOnly, Buffer },
		{ LFUN_BUFFER_END, "buffer-end", ReadOnly, Edit },
		{ LFUN_BUFFER_END_SELECT, "buffer-end-select", ReadOnly, Edit },
		{ LFUN_BUFFER_EXPORT, "buffer-export", ReadOnly, Buffer },
		{ LFUN_BUFFER_EXPORT_CUSTOM, "buffer-export-custom", ReadOnly, Buffer },
		{ LFUN_BUFFER_PRINT, "buffer-print", ReadOnly, Buffer },
		{ LFUN_BUFFER_IMPORT, "buffer-import", NoBuffer, Buffer },
		{ LFUN_BUFFER_NEW, "buffer-new", NoBuffer, Buffer },
		{ LFUN_BUFFER_NEW_TEMPLATE,"buffer-new-template", NoBuffer, Buffer },
		{ LFUN_BUFFER_RELOAD, "buffer-reload", ReadOnly, Buffer },
		{ LFUN_BUFFER_SWITCH, "buffer-switch", NoBuffer | ReadOnly, Edit },
		{ LFUN_BUFFER_TOGGLE_READ_ONLY, "buffer-toggle-read-only", ReadOnly, Buffer },
		{ LFUN_BUFFER_UPDATE, "buffer-update", ReadOnly, Edit },
		{ LFUN_BUFFER_VIEW, "buffer-view", ReadOnly, Edit },
		{ LFUN_BUFFER_WRITE, "buffer-write", ReadOnly, Buffer },
		{ LFUN_BUFFER_WRITE_AS, "buffer-write-as", ReadOnly, Buffer },
		{ LFUN_BUFFER_WRITE_ALL, "buffer-write-all", ReadOnly, Buffer },
		{ LFUN_BUFFER_NEXT, "buffer-next", ReadOnly, Buffer },
		{ LFUN_BUFFER_PREVIOUS, "buffer-previous", ReadOnly, Buffer },
		{ LFUN_MASTER_BUFFER_UPDATE, "master-buffer-update", ReadOnly, Edit },
		{ LFUN_MASTER_BUFFER_VIEW, "master-buffer-view", ReadOnly, Edit },
		{ LFUN_BUFFER_LANGUAGE, "buffer-language", Noop, Buffer },
		{ LFUN_BUFFER_SAVE_AS_DEFAULT, "buffer-save-as-default", Noop, Buffer },
		{ LFUN_BUFFER_PARAMS_APPLY, "buffer-params-apply", Noop, Buffer },

		{ LFUN_FILE_INSERT, "file-insert", Noop, Buffer },
		{ LFUN_FILE_INSERT_PLAINTEXT, "file-insert-plaintext", Noop, Edit },
		{ LFUN_FILE_INSERT_PLAINTEXT_PARA, "file-insert-plaintext-para", Noop, Edit },
		{ LFUN_FILE_NEW, "file-new", NoBuffer, Buffer },
		{ LFUN_FILE_OPEN, "file-open", NoBuffer, Buffer },

		{ LFUN_META_PREFIX, "meta-prefix", NoBuffer, System },

		{ LFUN_CALL, "call", NoBuffer, System },
		{ LFUN_CANCEL, "cancel", NoBuffer, System },

		{ LFUN_COMMAND_EXECUTE, "command-execute", NoBuffer, Edit },
		{ LFUN_COMMAND_PREFIX, "command-prefix", NoBuffer, Hidden },
		{ LFUN_COMMAND_SEQUENCE, "command-sequence", NoBuffer, System },

		{ LFUN_PREFERENCES_SAVE, "preferences-save", NoBuffer, Edit },
		{ LFUN_RECONFIGURE, "reconfigure", NoBuffer, System },
		{ LFUN_LYXRC_APPLY, "lyxrc-apply", NoBuffer, System },
		{ LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR, "toggle-cursor-follows-scrollbar", ReadOnly, Edit },

		{ LFUN_SET_COLOR, "set-color", ReadOnly | NoBuffer, Edit },
		{ LFUN_THESAURUS_ENTRY, "thesaurus-entry", ReadOnly, Edit },
		{ LFUN_STATISTICS, "statistics", ReadOnly, System },

		{ LFUN_NOACTION, "", Noop, Hidden }
	};

	for (int i = 0; items[i].action != LFUN_NOACTION; ++i) {
		newFunc(items[i].action, items[i].name, items[i].attrib, items[i].type);
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


LyXAction::func_type const LyXAction::getActionType(kb_action action) const
{
	info_map::const_iterator const it = lyx_info_map.find(action);
	return it != lyx_info_map.end() ? it->second.type : Hidden;
}


bool LyXAction::funcHasFlag(kb_action action,
			    LyXAction::func_attrib flag) const
{
	info_map::const_iterator ici = lyx_info_map.find(action);

	if (ici == lyx_info_map.end()) {
		LYXERR0("action: " << action << " is not known.");
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


} // namespace lyx
