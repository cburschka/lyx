/**
 * \file LyXAction.C
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
#include "funcrequest.h"

#include "support/lstrings.h"

#include <boost/assert.hpp>

using lyx::support::split;
using lyx::support::trim;

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
		{ LFUN_ACUTE, "accent-acute", Noop },
		{ LFUN_BREVE, "accent-breve", Noop },
		{ LFUN_CARON, "accent-caron", Noop },
		{ LFUN_CEDILLA, "accent-cedilla", Noop },
		{ LFUN_CIRCLE, "accent-circle", Noop },
		{ LFUN_CIRCUMFLEX, "accent-circumflex", Noop },
		{ LFUN_DOT, "accent-dot", Noop },
		{ LFUN_GRAVE, "accent-grave", Noop },
		{ LFUN_HUNG_UMLAUT, "accent-hungarian-umlaut", Noop },
		{ LFUN_MACRON, "accent-macron", Noop },
		{ LFUN_OGONEK, "accent-ogonek", Noop },
		{ LFUN_SPECIAL_CARON, "accent-special-caron", Noop },
		{ LFUN_TIE, "accent-tie", Noop },
		{ LFUN_TILDE, "accent-tilde", Noop },
		{ LFUN_UMLAUT, "accent-umlaut", Noop },
		{ LFUN_UNDERBAR, "accent-underbar", Noop },
		{ LFUN_UNDERDOT, "accent-underdot", Noop },
		{ LFUN_APPENDIX, "appendix", Noop },
		{ LFUN_LEFTSEL, "backward-select", ReadOnly },
		{ LFUN_BOOKMARK_GOTO, "bookmark-goto", ReadOnly },
		{ LFUN_BOOKMARK_SAVE, "bookmark-save", ReadOnly },
		{ LFUN_BREAKLINE, "break-line", Noop },
		{ LFUN_BREAKPARAGRAPH, "break-paragraph", Noop },
		{ LFUN_BREAKPARAGRAPHKEEPLAYOUT, "break-paragraph-keep-layout", Noop },
		{ LFUN_BREAKPARAGRAPH_SKIP, "break-paragraph-skip", Noop },
		{ LFUN_BUILDPROG, "build-program", ReadOnly },
		{ LFUN_AUTOSAVE, "buffer-auto-save", Noop },
		{ LFUN_BEGINNINGBUF, "buffer-begin", ReadOnly },
		{ LFUN_BEGINNINGBUFSEL, "buffer-begin-select", ReadOnly },
		{ LFUN_CHILDOPEN, "buffer-child-open", ReadOnly },
		{ LFUN_RUNCHKTEX, "buffer-chktex", ReadOnly },
		{ LFUN_CLOSEBUFFER, "buffer-close", ReadOnly },
		{ LFUN_ENDBUF, "buffer-end", ReadOnly },
		{ LFUN_ENDBUFSEL, "buffer-end-select", ReadOnly },
		{ LFUN_EXPORT, "buffer-export", ReadOnly },
		{ LFUN_IMPORT, "buffer-import", NoBuffer },
		{ LFUN_MENUNEW, "buffer-new", NoBuffer },
		{ LFUN_MENUNEWTMPLT,"buffer-new-template", NoBuffer },
		{ LFUN_MENURELOAD, "buffer-reload", ReadOnly },
		{ LFUN_SWITCHBUFFER, "buffer-switch", ReadOnly },
		{ LFUN_READ_ONLY_TOGGLE, "buffer-toggle-read-only", ReadOnly },
		{ LFUN_UPDATE, "buffer-update", ReadOnly },
		{ LFUN_PREVIEW, "buffer-view", ReadOnly },
		{ LFUN_MENUWRITE, "buffer-write", ReadOnly },
		{ LFUN_WRITEAS, "buffer-write-as", ReadOnly },
		{ LFUN_CANCEL, "cancel", NoBuffer },
		{ LFUN_INSET_CAPTION, "caption-insert", Noop },
		{ LFUN_LEFT, "char-backward", ReadOnly },
		{ LFUN_RIGHT, "char-forward", ReadOnly },
		{ LFUN_EXEC_COMMAND, "command-execute", NoBuffer },
		{ LFUN_PREFIX, "command-prefix", NoBuffer },
		{ LFUN_SEQUENCE, "command-sequence", NoBuffer },
		{ LFUN_COPY, "copy", ReadOnly },
		{ LFUN_CUT, "cut", Noop },
		{ LFUN_DATE_INSERT, "date-insert", Noop },
		{ LFUN_BACKSPACE, "delete-backward", Noop },
		{ LFUN_BACKSPACE_SKIP, "delete-backward-skip", Noop },
		{ LFUN_DELETE, "delete-forward", Noop },
		{ LFUN_DELETE_SKIP, "delete-forward-skip", Noop },
		{ LFUN_DEPTH_MIN, "depth-decrement", Noop },
		{ LFUN_DEPTH_PLUS, "depth-increment", Noop },
		{ LFUN_LDOTS, "dots-insert", Noop },
		{ LFUN_DOWN, "down", ReadOnly },
		{ LFUN_DOWNSEL, "down-select", ReadOnly },
		{ LFUN_DROP_LAYOUTS_CHOICE, "drop-layouts-choice", ReadOnly },
		{ LFUN_END_OF_SENTENCE, "end-of-sentence-period-insert", Noop },
		{ LFUN_ENVIRONMENT_INSERT, "environment-insert", Noop },
		{ LFUN_GOTOERROR, "error-next", ReadOnly },
		{ LFUN_INSET_ERT, "ert-insert", Noop },
		{ LFUN_FILE_INSERT, "file-insert", Noop },
		{ LFUN_FILE_INSERT_ASCII, "file-insert-ascii", Noop },
		{ LFUN_FILE_INSERT_ASCII_PARA, "file-insert-ascii-para", Noop },
		{ LFUN_FILE_NEW, "file-new", NoBuffer },
		{ LFUN_FILE_OPEN, "file-open", NoBuffer },
		{ LFUN_INSET_FLOAT, "float-insert", Noop },
		{ LFUN_INSET_WIDE_FLOAT, "float-wide-insert", Noop },
		{ LFUN_INSET_WRAP, "wrap-insert", Noop },
		{ LFUN_BOLD, "font-bold", Noop },
		{ LFUN_CODE, "font-code", Noop },
		{ LFUN_DEFAULT, "font-default", Noop },
		{ LFUN_EMPH, "font-emph", Noop },
		{ LFUN_FREEFONT_APPLY, "font-free-apply", Noop },
		{ LFUN_FREEFONT_UPDATE, "font-free-update", Noop },
		{ LFUN_NOUN, "font-noun", Noop },
		{ LFUN_ROMAN, "font-roman", Noop },
		{ LFUN_SANS, "font-sans", Noop },
		{ LFUN_FRAK, "font-frak", Noop },
		{ LFUN_ITAL, "font-ital", Noop },
		{ LFUN_FONT_SIZE, "font-size", Noop },
		{ LFUN_FONT_STATE, "font-state", ReadOnly },
		{ LFUN_UNDERLINE, "font-underline", Noop },
		{ LFUN_INSET_FOOTNOTE, "footnote-insert", Noop },
		{ LFUN_RIGHTSEL, "forward-select", ReadOnly },
		{ LFUN_HFILL, "hfill-insert", Noop },
		{ LFUN_HELP_OPEN, "help-open", NoBuffer | Argument},
		{ LFUN_HTMLURL, "html-insert", Noop },
		{ LFUN_HYPHENATION, "hyphenation-point-insert", Noop },
		{ LFUN_LIGATURE_BREAK, "ligature-break-insert", Noop },
		{ LFUN_INDEX_INSERT, "index-insert", Noop },
		{ LFUN_INDEX_PRINT, "index-print", Noop },
		{ LFUN_KMAP_OFF, "keymap-off", ReadOnly },
		{ LFUN_KMAP_PRIM, "keymap-primary", ReadOnly },
		{ LFUN_KMAP_SEC, "keymap-secondary", ReadOnly },
		{ LFUN_KMAP_TOGGLE, "keymap-toggle", ReadOnly },
		{ LFUN_INSERT_LABEL, "label-insert", Noop },
		{ LFUN_INSET_OPTARG, "optional-insert", Noop },
		{ LFUN_INSERT_BIBITEM, "bibitem-insert", Noop },
		{ LFUN_INSERT_LINE, "line-insert", Noop },
		{ LFUN_INSERT_PAGEBREAK, "pagebreak-insert", Noop },
		{ LFUN_LANGUAGE, "language", Noop },
		{ LFUN_LAYOUT, "layout", Noop },
		{ LFUN_LAYOUT_PARAGRAPH, "layout-paragraph", ReadOnly },
		{ LFUN_LAYOUT_TABULAR, "layout-tabular", Noop },
		{ LFUN_HOME, "line-begin", ReadOnly },
		{ LFUN_HOMESEL, "line-begin-select", ReadOnly },
		{ LFUN_DELETE_LINE_FORWARD, "line-delete-forward", Noop },
		{ LFUN_END, "line-end", ReadOnly },
		{ LFUN_ENDSEL, "line-end-select", ReadOnly },
#if 0
		{ LFUN_INSET_LIST, "list-insert", Noop },
#endif
		{ LFUN_QUIT, "lyx-quit", NoBuffer },
		{ LFUN_INSET_MARGINAL, "marginalnote-insert", Noop },
		{ LFUN_MARK_OFF, "mark-off", ReadOnly },
		{ LFUN_MARK_ON, "mark-on", ReadOnly },
		{ LFUN_SETMARK, "mark-toggle", ReadOnly },
		{ LFUN_MATH_DELIM, "math-delim", Noop },
		{ LFUN_MATH_DISPLAY, "math-display", Noop },
		{ LFUN_INSERT_MATH, "math-insert", Noop },
		{ LFUN_SUBSCRIPT, "math-subscript", Noop },
		{ LFUN_SUPERSCRIPT, "math-superscript", Noop },
		{ LFUN_MATH_LIMITS, "math-limits", Noop },
		{ LFUN_MATH_MACRO, "math-macro", Noop },
		{ LFUN_MATH_MUTATE, "math-mutate", Noop },
		{ LFUN_MATH_SPACE, "math-space", Noop },
		{ LFUN_MATH_IMPORT_SELECTION, "math-import-selection", Noop },
		{ LFUN_INSERT_MATRIX, "math-matrix", Noop },
		{ LFUN_MATH_MODE, "math-mode", Noop },
		{ LFUN_MATH_NONUMBER, "math-nonumber", Noop },
		{ LFUN_MATH_NUMBER, "math-number", Noop },
		{ LFUN_MATH_EXTERN, "math-extern", Noop },
		{ LFUN_MATH_SIZE, "math-size", Noop },
		{ LFUN_MENU_OPEN_BY_NAME, "menu-open", NoBuffer },
		{ LFUN_MENU_SEPARATOR, "menu-separator-insert", Noop },
		{ LFUN_META_FAKE, "meta-prefix", NoBuffer },
		{ LFUN_INSERT_BRANCH, "branch-insert", Noop },
		{ LFUN_INSERT_CHARSTYLE, "charstyle-insert", Noop },
		{ LFUN_INSERT_NOTE, "note-insert", Noop },
		{ LFUN_INSERT_BOX, "box-insert", Noop },
		{ LFUN_GOTONOTE, "note-next", ReadOnly },
		{ LFUN_INSET_TOGGLE, "inset-toggle", ReadOnly },
		{ LFUN_DOWN_PARAGRAPH, "paragraph-down", ReadOnly },
		{ LFUN_DOWN_PARAGRAPHSEL, "paragraph-down-select", ReadOnly },
		{ LFUN_GOTO_PARAGRAPH, "paragraph-goto", ReadOnly },
		{ LFUN_PARAGRAPH_SPACING, "paragraph-spacing", Noop },
		{ LFUN_UP_PARAGRAPH, "paragraph-up", ReadOnly },
		{ LFUN_UP_PARAGRAPHSEL, "paragraph-up-select", ReadOnly },
		{ LFUN_PASTE, "paste", Noop },
		{ LFUN_SAVEPREFERENCES, "preferences-save", NoBuffer },
		{ LFUN_PASTESELECTION, "primary-selection-paste", Noop },
		{ LFUN_QUOTE, "quote-insert", Noop },
		{ LFUN_RECONFIGURE, "reconfigure", NoBuffer },
		{ LFUN_REDO, "redo", Noop },
		{ LFUN_REF_GOTO, "reference-goto", ReadOnly },
		{ LFUN_REFERENCE_GOTO, "reference-next", ReadOnly },
		{ LFUN_NEXT, "screen-down", ReadOnly },
		{ LFUN_NEXTSEL, "screen-down-select", ReadOnly },
		{ LFUN_SCREEN_FONT_UPDATE, "screen-font-update", NoBuffer },
		{ LFUN_CENTER, "screen-recenter", ReadOnly },
		{ LFUN_PRIOR, "screen-up", ReadOnly },
		{ LFUN_PRIORSEL, "screen-up-select", ReadOnly },
		{ LFUN_SCROLL_INSET, "inset-scroll", ReadOnly },
		{ LFUN_SELFINSERT, "self-insert", Noop },
		{ LFUN_SPACE_INSERT, "space-insert", Noop },
		{ LFUN_CHARATCURSOR, "server-char-after", ReadOnly },
		{ LFUN_GETFONT, "server-get-font", ReadOnly },
		{ LFUN_GETLAYOUT, "server-get-layout", ReadOnly },
		{ LFUN_GETNAME, "server-get-name", ReadOnly },
		{ LFUN_GETXY, "server-get-xy", ReadOnly },
		{ LFUN_GOTOFILEROW, "server-goto-file-row", Noop },
		{ LFUN_NOTIFY, "server-notify", ReadOnly },
		{ LFUN_SETXY, "server-set-xy", ReadOnly },
		{ LFUN_SET_COLOR, "set-color", ReadOnly | NoBuffer },
		{ LFUN_CELL_BACKWARD, "cell-backward", Noop },
		{ LFUN_CELL_FORWARD, "cell-forward", Noop },
		{ LFUN_CELL_SPLIT, "cell-split", Noop },
		{ LFUN_TABULAR_INSERT, "tabular-insert", Noop },
		{ LFUN_TABULAR_FEATURE, "tabular-feature", Noop },
#if 0
		{ LFUN_INSET_THEOREM, "theorem-insert", Noop },
#endif
		{ LFUN_THESAURUS_ENTRY, "thesaurus-entry", ReadOnly },
		{ LFUN_TOC_INSERT, "toc-insert", Noop },
		{ LFUN_TOCVIEW, "toc-view", ReadOnly },
		{ LFUN_TOGGLECURSORFOLLOW, "toggle-cursor-follows-scrollbar", ReadOnly },
		{ LFUN_UNDO, "undo", Noop },
		{ LFUN_UP, "up", ReadOnly },
		{ LFUN_UPSEL, "up-select", ReadOnly },
		{ LFUN_URL, "url-insert", Noop },
		{ LFUN_VC_CHECKIN, "vc-check-in", ReadOnly },
		{ LFUN_VC_CHECKOUT, "vc-check-out", ReadOnly },
		{ LFUN_VC_REGISTER, "vc-register", ReadOnly },
		{ LFUN_VC_REVERT, "vc-revert", ReadOnly },
		{ LFUN_VC_UNDO, "vc-undo-last", ReadOnly },
		{ LFUN_WORDLEFT, "word-backward", ReadOnly },
		{ LFUN_WORDLEFTSEL, "word-backward-select", ReadOnly },
		{ LFUN_CAPITALIZE_WORD, "word-capitalize", Noop },
		{ LFUN_DELETE_WORD_BACKWARD, "word-delete-backward", Noop },
		{ LFUN_DELETE_WORD_FORWARD, "word-delete-forward", Noop },
		{ LFUN_WORDFINDBACKWARD, "word-find-backward", ReadOnly },
		{ LFUN_WORDFINDFORWARD, "word-find-forward", ReadOnly },
		{ LFUN_WORDRIGHT, "word-forward", ReadOnly },
		{ LFUN_WORDRIGHTSEL, "word-forward-select", ReadOnly },
		{ LFUN_LOWCASE_WORD, "word-lowcase", Noop },
		{ LFUN_WORDSEL, "word-select", ReadOnly },
		{ LFUN_UPCASE_WORD, "word-upcase", Noop },
		{ LFUN_MESSAGE, "message", NoBuffer },
		{ LFUN_TRANSPOSE_CHARS, "chars-transpose", Noop },
		{ LFUN_FLOAT_LIST, "float-list", Noop },
		{ LFUN_ESCAPE, "escape", ReadOnly },
		{ LFUN_FORKS_KILL, "kill-forks", NoBuffer },
		{ LFUN_TOOLTIPS_TOGGLE, "toggle-tooltips", NoBuffer },
		{ LFUN_TRACK_CHANGES, "track-changes", Noop },
		{ LFUN_MERGE_CHANGES, "merge-changes", Noop },
		{ LFUN_ACCEPT_CHANGE, "accept-change", Noop },
		{ LFUN_REJECT_CHANGE, "reject-change", Noop },
		{ LFUN_ACCEPT_ALL_CHANGES, "accept-all-changes", Noop },
		{ LFUN_REJECT_ALL_CHANGES, "reject-all-changes", Noop },
		{ LFUN_DIALOG_SHOW, "dialog-show", NoBuffer },
		{ LFUN_DIALOG_SHOW_NEW_INSET, "dialog-show-new-inset", Noop },
		{ LFUN_DIALOG_SHOW_NEXT_INSET, "dialog-show-next-inset", Noop },
		{ LFUN_DIALOG_UPDATE, "dialog-update", Noop },
		{ LFUN_DIALOG_HIDE, "dialog-hide", Noop },
		{ LFUN_DIALOG_DISCONNECT_INSET, "dialog-disconnect-inset", Noop },
		{ LFUN_INSET_APPLY, "inset-apply", Noop },
		{ LFUN_INSET_INSERT, "inset-insert", Noop },
		{ LFUN_INSET_MODIFY, "", Noop },
		{ LFUN_INSET_DIALOG_UPDATE, "", Noop },
		{ LFUN_INSET_SETTINGS, "inset-settings", ReadOnly },
		{ LFUN_INSET_DIALOG_SHOW, "inset-dialog-show", Noop },
		{ LFUN_PARAGRAPH_APPLY, "paragraph-params-apply", Noop },
		{ LFUN_PARAGRAPH_UPDATE, "", Noop },
		{ LFUN_EXTERNAL_EDIT, "external-edit", Noop },
		{ LFUN_REPEAT, "repeat", NoBuffer },
		{ LFUN_WORD_FIND, "word-find", Noop },
		{ LFUN_WORD_REPLACE, "word-replace", Noop },
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
