/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXAction.h"
#include "debug.h"
#include "gettext.h"
#include "support/lstrings.h"

using std::ostream;
using std::endl;

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
 
// These are globals. 
LyXAction lyxaction;

void LyXAction::newFunc(kb_action action, string const & name,
			string const & helpText, unsigned int attrib)
{
	lyx_func_map[name] = action;
	func_info tmpinfo;
	tmpinfo.name = name;
	tmpinfo.attrib = attrib;
	tmpinfo.helpText = helpText;
	lyx_info_map[action] = tmpinfo;
}


struct lfun_item {
	kb_action action;
	char const * name;
	char const * helpText;
	unsigned int attrib;
};

void LyXAction::init()
{
	// This function was changed to use the array below in initalization
	// instead of calling newFunc numerous times because of compilation
	// times. Since the array is not static we get back the memory it
	// occupies after the init is completed. It compiles several
	//magnitudes faster.
	
	static bool init = false;
	if (init) return;

	lfun_item items[] = {
		{ LFUN_ACUTE, "accent-acute", "", Noop },
		{ LFUN_BREVE, "accent-breve", "", Noop },
		{ LFUN_CARON, "accent-caron", "", Noop },
		{ LFUN_CEDILLA, "accent-cedilla", "", Noop },
		{ LFUN_CIRCLE, "accent-circle", "", Noop },
		{ LFUN_CIRCUMFLEX, "accent-circumflex", "", Noop },
		{ LFUN_DOT, "accent-dot", "", Noop },
		{ LFUN_GRAVE, "accent-grave", "", Noop },
		{ LFUN_HUNG_UMLAUT, "accent-hungarian-umlaut", "", Noop },
		{ LFUN_MACRON, "accent-macron", "", Noop },
		{ LFUN_OGONEK, "accent-ogonek", "", Noop },
		{ LFUN_SPECIAL_CARON, "accent-special-caron", "", Noop },
		{ LFUN_TIE, "accent-tie", "", Noop },
		{ LFUN_TILDE, "accent-tilde", "", Noop },
		{ LFUN_UMLAUT, "accent-umlaut", "", Noop },
		{ LFUN_UNDERBAR, "accent-underbar", "", Noop },
		{ LFUN_UNDERDOT, "accent-underdot", "", Noop },
		{ LFUN_VECTOR, "accent-vector", "", Noop },
		{ LFUN_APPENDIX, "appendix", N_("Insert appendix"), Noop },
		{ LFUN_APROPOS, "apropos", N_("Describe command"),
		  NoBuffer|ReadOnly },
		{ LFUN_LEFTSEL, "backward-select",
		  N_("Select previous char"), ReadOnly },
		{ LFUN_BIBDB_ADD, "bibtex-database-add", "", Noop },
		{ LFUN_BIBDB_DEL, "bibtex-database-del", "", Noop },
		{ LFUN_INSERT_BIBTEX, "bibtex-insert", N_("Insert bibtex"),
		  Noop },
		{ LFUN_BIBTEX_STYLE, "bibtex-style", "", Noop },
		{ LFUN_BREAKLINE, "break-line", "", Noop },
		{ LFUN_BREAKPARAGRAPH, "break-paragraph", "", Noop },
		{ LFUN_BREAKPARAGRAPHKEEPLAYOUT, "break-paragraph-keep-layout",
		  "", Noop },
		{ LFUN_BREAKPARAGRAPH_SKIP, "break-paragraph-skip", "", Noop },
		{ LFUN_BUILDPROG, "build-program", 
		  N_("Build program"), ReadOnly },
		{ LFUN_AUTOSAVE, "buffer-auto-save", N_("Autosave"), Noop },
		{ LFUN_BEGINNINGBUF, "buffer-begin",
		  N_("Go to beginning of document"), ReadOnly },
		{ LFUN_BEGINNINGBUFSEL, "buffer-begin-select",
		  N_("Select to beginning of document"), ReadOnly },
		{ LFUN_CHILDINSERT, "buffer-child-insert", "", Noop },
		{ LFUN_CHILDOPEN, "buffer-child-open", "", ReadOnly },
		{ LFUN_RUNCHKTEX, "buffer-chktex", N_("Check TeX"), ReadOnly },
		{ LFUN_CLOSEBUFFER, "buffer-close", N_("Close"), ReadOnly },
		{ LFUN_ENDBUF, "buffer-end",
		  N_("Go to end of document"), ReadOnly },
		{ LFUN_ENDBUFSEL, "buffer-end-select",
		  N_("Select to end of document"), ReadOnly },
		{ LFUN_EXPORT, "buffer-export", N_("Export to"), ReadOnly },
		{ LFUN_FAX, "buffer-fax", N_("Fax"), ReadOnly },
		{ LFUN_INSERTFOOTNOTE, "buffer-float-insert", "", Noop },
		{ LFUN_IMPORT, "buffer-import",
		  N_("Import document"), NoBuffer },
		{ LFUN_BUFFERBULLETSSELECT, "buffer-itemize-bullets-select",
		  "", Noop },
		{ LFUN_MENUNEW, "buffer-new", N_("New document") , NoBuffer },
		{ LFUN_MENUNEWTMPLT,"buffer-new-template",
		  N_("New document from template"), NoBuffer },
		{ LFUN_MENUOPEN, "buffer-open", N_("Open"), NoBuffer },
		{ LFUN_MENUPRINT, "buffer-print", N_("Print"), ReadOnly },
		{ LFUN_MENURELOAD, "buffer-reload",
		  N_("Revert to saved"), ReadOnly },
		{ LFUN_READ_ONLY_TOGGLE, "buffer-toggle-read-only",
		  N_("Toggle read-only"), ReadOnly },
		{ LFUN_RUNLATEX, "buffer-typeset", N_("Update DVI"),
		  ReadOnly },
		{ LFUN_RUNDVIPS, "buffer-typeset-ps",
		  N_("Update PostScript"), ReadOnly },
		{ LFUN_PREVIEW, "buffer-view", N_("View DVI") , ReadOnly },
		{ LFUN_PREVIEWPS, "buffer-view-ps",
		  N_("View PostScript") , ReadOnly },
		{ LFUN_MENUWRITE, "buffer-write", N_("Save"), ReadOnly },
		{ LFUN_MENUWRITEAS, "buffer-write-as", N_("Save As"),
		  ReadOnly },
		{ LFUN_CANCEL, "cancel", N_("Cancel"), NoBuffer },
		{ LFUN_LEFT, "char-backward", N_("Go one char back"),
		  ReadOnly },
		{ LFUN_RIGHT, "char-forward", N_("Go one char forward"),
		  ReadOnly },
		{ LFUN_INSERT_CITATION, "citation-insert",
		  N_("Insert citation"), Noop },
		{ LFUN_EXEC_COMMAND, "command-execute", "", NoBuffer },
		{ LFUN_PREFIX, "command-prefix",
		  N_("Execute command"), NoBuffer },
		{ LFUN_SEQUENCE, "command-sequence", "", Noop },
		{ LFUN_COPY, "copy", N_("Copy"), ReadOnly },
		{ LFUN_CUT, "cut", N_("Cut"), Noop },
		{ LFUN_BACKSPACE, "delete-backward", "", Noop },
		{ LFUN_BACKSPACE_SKIP, "delete-backward-skip", "", Noop },
		{ LFUN_DELETE, "delete-forward", "", Noop },
		{ LFUN_DELETE_SKIP, "delete-forward-skip", "", Noop },
		{ LFUN_DEPTH_MIN, "depth-decrement",
		  N_("Decrement environment depth"), Noop },
		{ LFUN_DEPTH_PLUS, "depth-increment",
		  N_("Increment environment depth"), Noop },
		{ LFUN_DEPTH, "depth-next", 
		  N_("Change environment depth"), Noop },
		{ LFUN_LDOTS, "dots-insert", N_("Insert ... dots"), Noop },
		{ LFUN_DOWN, "down", N_("Go down"), ReadOnly },
		{ LFUN_DOWNSEL, "down-select",
		  N_("Select next line"), ReadOnly },
		{ LFUN_DROP_LAYOUTS_CHOICE, "drop-layouts-choice",
		  N_("Choose Paragraph Environment"), ReadOnly },
		{ LFUN_END_OF_SENTENCE, "end-of-sentence-period-insert",
		  N_("Insert end of sentence period"), Noop },
		{ LFUN_GOTOERROR, "error-next", N_("Go to next error"), Noop },
		{ LFUN_REMOVEERRORS, "error-remove-all",
		  N_("Remove all error boxes"), ReadOnly },
		{ LFUN_INSET_ERT, "ert-inset-insert",
		  N_("Insert a new ERT Inset"), Noop },
		{ LFUN_FIGURE, "figure-insert", N_("Insert Figure"), Noop },
		{ LFUN_INSERT_GRAPHICS, "graphics-insert",
		  N_("Insert Graphics"), Noop },
		{ LFUN_FILE_INSERT, "file-insert", "", Noop },
		{ LFUN_FILE_INSERT_ASCII, "file-insert-ascii", "", Noop },
		{ LFUN_FILE_NEW, "file-new", "", NoBuffer },
		{ LFUN_FILE_OPEN, "file-open", "", NoBuffer },
		{ LFUN_MENUSEARCH, "find-replace", N_("Find & Replace"),
		  ReadOnly },
		{ LFUN_BOLD, "font-bold", N_("Toggle bold"), Noop },
		{ LFUN_CODE, "font-code", N_("Toggle code style"), Noop },
		{ LFUN_DEFAULT, "font-default", N_("Default font style"),
		  Noop },
		{ LFUN_EMPH, "font-emph", N_("Toggle emphasize"), Noop },
		{ LFUN_FREE, "font-free", N_("Toggle user defined style"),
		  Noop },
		{ LFUN_NOUN, "font-noun", N_("Toggle noun style"), Noop },
		{ LFUN_ROMAN, "font-roman", N_("Toggle roman font style"),
		  Noop },
		{ LFUN_SANS, "font-sans", N_("Toggle sans font style"), Noop },
		{ LFUN_FONT_SIZE, "font-size", N_("Set font size"), Noop },
		{ LFUN_FONT_STATE, "font-state", N_("Show font state"),
		  ReadOnly },
		{ LFUN_UNDERLINE, "font-underline",
		  N_("Toggle font underline"), Noop },
		{ LFUN_FOOTMELT, "footnote-insert", N_("Insert Footnote"),
		  Noop },
		{ LFUN_INSET_FOOTNOTE, "footnote-inset-insert",
		  N_("Insert Footnote"), Noop },
		{ LFUN_RIGHTSEL, "forward-select", N_("Select next char"),
		  ReadOnly },
		{ LFUN_HFILL, "hfill-insert",
		  N_("Insert horizontal fill"), Noop },
		{ LFUN_HTMLURL, "html-insert", "", Noop },
		{ LFUN_HYPHENATION, "hyphenation-point-insert",
		  N_("Insert hyphenation point"), Noop },
		{ LFUN_INDEX_INSERT, "index-insert",
		  N_("Insert index item"), Noop },
		{ LFUN_INDEX_INSERT_LAST, "index-insert-last",
		  N_("Insert last index item"), Noop },
		{ LFUN_INDEX_PRINT, "index-print", N_("Insert index list"),
		  Noop },
		{ LFUN_KMAP_OFF, "keymap-off", N_("Turn off keymap"),
		  ReadOnly },
		{ LFUN_KMAP_PRIM, "keymap-primary",
		  N_("Use primary keymap"), ReadOnly },
		{ LFUN_KMAP_SEC, "keymap-secondary",
		  N_("Use secondary keymap"), ReadOnly },
		{ LFUN_KMAP_TOGGLE, "keymap-toggle", N_("Toggle keymap"),
		  ReadOnly },
		{ LFUN_INSERT_LABEL, "label-insert", N_("Insert Label"),
		  Noop },
		{ LFUN_LANGUAGE, "language", N_("Change language"), Noop },
		{ LFUN_LATEX_LOG, "latex-view-log", N_("View LaTeX log"),
		  ReadOnly },
		{ LFUN_LAYOUT, "layout", "", Noop },
		{ LFUN_LAYOUT_CHARACTER, "layout-character", "", Noop },
		{ LFUN_LAYOUT_COPY, "layout-copy",
		  N_("Copy paragraph environment type"), Noop },
		{ LFUN_LAYOUT_DOCUMENT, "layout-document", "", ReadOnly },
		{ LFUN_LAYOUTNO, "layout-number", "", Noop }, // internal only
		{ LFUN_LAYOUT_PARAGRAPH, "layout-paragraph", "", ReadOnly },
		{ LFUN_LAYOUT_PAPER, "layout-paper", "", ReadOnly },
		{ LFUN_LAYOUT_PASTE, "layout-paste",
		  N_("Paste paragraph environment type"), Noop },
		{ LFUN_LAYOUT_PREAMBLE, "layout-preamble", "", ReadOnly },
		{ LFUN_LAYOUT_QUOTES, "layout-quotes", "", ReadOnly },
		{ LFUN_LAYOUT_SAVE_DEFAULT, "layout-save-default", "",
		  ReadOnly },
		{ LFUN_LAYOUT_TABLE, "layout-table", "", Noop },
		{ LFUN_HOME, "line-begin",
		  N_("Go to beginning of line"), ReadOnly },
		{ LFUN_HOMESEL, "line-begin-select",
		  N_("Select to beginning of line"), ReadOnly },
		{ LFUN_DELETE_LINE_FORWARD, "line-delete-forward", "", Noop },
		{ LFUN_END, "line-end", N_("Go to end of line"), ReadOnly },
		{ LFUN_ENDSEL, "line-end-select",
		  N_("Select to end of line"), ReadOnly },
		{ LFUN_LOA_INSERT, "loa-insert",
		  N_("Insert list of algorithms"), Noop },
		{ LFUN_LOAVIEW, "loa-view",
		  N_("View list of algorithms"), ReadOnly },
		{ LFUN_LOF_INSERT, "lof-insert",
		  N_("Insert list of figures"), Noop },
		{ LFUN_LOFVIEW, "lof-view",
		  N_("View list of figures"), ReadOnly },
		{ LFUN_LOT_INSERT, "lot-insert",
		  N_("Insert list of tables"), Noop },
		{ LFUN_LOTVIEW, "lot-view",
		  N_("View list of tables"), ReadOnly },
		{ LFUN_QUIT, "lyx-quit", N_("Exit"), NoBuffer },
		{ LFUN_MARGINMELT, "marginpar-insert",
		  N_("Insert Margin note"), Noop },
		{ LFUN_MARK_OFF, "mark-off", "", ReadOnly },
		{ LFUN_MARK_ON, "mark-on", "", ReadOnly },
		{ LFUN_SETMARK, "mark-toggle", "", ReadOnly },
		{ LFUN_MATH_DELIM, "math-delim", "", Noop },
		{ LFUN_MATH_DISPLAY, "math-display", "", Noop },
		{ LFUN_GREEK, "math-greek", N_("Math Greek"), Noop },
		{ LFUN_GREEK_TOGGLE, "math-greek-toggle", "", Noop },
		{ LFUN_INSERT_MATH, "math-insert",
		  N_("Insert math symbol"), Noop },
		{ LFUN_MATH_LIMITS, "math-limits", "", Noop },
		{ LFUN_MATH_MACRO, "math-macro", "", Noop },
		{ LFUN_MATH_MACROARG, "math-macro-arg", "", Noop },
		{ LFUN_INSERT_MATRIX, "math-matrix", "", Noop },
		{ LFUN_MATH_MODE, "math-mode", N_("Math mode"), Noop },
		{ LFUN_MATH_NONUMBER, "math-nonumber", "", Noop },
		{ LFUN_MATH_NUMBER, "math-number", "", Noop },
		{ LFUN_MATH_SIZE, "math-size", "", Noop },
		{ LFUN_MELT, "melt", N_("Melt"), Noop },
		{ LFUN_MENU_OPEN_BY_NAME, "menu-open", "", NoBuffer },
		{ LFUN_MENU_SEPARATOR, "menu-separator-insert", "", Noop },
		{ LFUN_META_FAKE, "meta-prefix", "", NoBuffer },
		{ LFUN_INSERT_NOTE, "note-insert", "", Noop },
		{ LFUN_GOTONOTE, "note-next", "", ReadOnly },
		{ LFUN_OPENSTUFF, "open-stuff", "", ReadOnly },
		{ LFUN_DOWN_PARAGRAPH, "paragraph-down",
		  N_("Go one paragraph down"), ReadOnly },
		{ LFUN_DOWN_PARAGRAPHSEL, "paragraph-down-select",
		  N_("Select next paragraph"), ReadOnly },
		{ LFUN_UP_PARAGRAPH, "paragraph-up",
		  N_("Go one paragraph up"), ReadOnly },
		{ LFUN_UP_PARAGRAPHSEL, "paragraph-up-select",
		  N_("Select previous paragraph"), ReadOnly },
		{ LFUN_PARENTINSERT, "parent-insert", "", Noop },
		{ LFUN_PASTE, "paste", N_("Paste") , Noop },
		{ LFUN_SAVEPREFERENCES, "preferences-save",
		  "Save Preferences", NoBuffer },
		{ LFUN_PASTESELECTION, "primary-selection-paste", "", Noop },
		{ LFUN_PROTECTEDSPACE, "protected-space-insert",
		  N_("Insert protected space"), Noop },
		{ LFUN_QUOTE, "quote-insert", N_("Insert quote"), Noop },
		{ LFUN_RECONFIGURE, "reconfigure",
		  N_("Reconfigure"), NoBuffer },
		{ LFUN_REDO, "redo", N_("Redo"), Noop },
		{ LFUN_REFBACK, "reference-back", "", ReadOnly },
		{ LFUN_REFGOTO, "reference-goto", "", ReadOnly },
		{ LFUN_INSERT_REF, "reference-insert",
		  N_("Insert cross reference"), ReadOnly },
		{ LFUN_REFTOGGLE, "reference-toggle", "", Noop },
		{ LFUN_NEXT, "screen-down", "", ReadOnly },
		{ LFUN_NEXTSEL, "screen-down-select", "", ReadOnly },
		{ LFUN_CENTER, "screen-recenter", "", ReadOnly },
		{ LFUN_PRIOR, "screen-up", "", ReadOnly },
		{ LFUN_PRIORSEL, "screen-up-select", "", ReadOnly },
		{ LFUN_SELFINSERT, "self-insert", "", Noop },
		{ LFUN_CHARATCURSOR, "server-char-after", "", ReadOnly },
		{ LFUN_GETFONT, "server-get-font", "", ReadOnly },
		{ LFUN_GETLATEX, "server-get-latex", "", ReadOnly },
		{ LFUN_GETLAYOUT, "server-get-layout", "", ReadOnly },
		{ LFUN_GETNAME, "server-get-name", "", ReadOnly },
		{ LFUN_GETTIP, "server-get-tip", "", ReadOnly },
		{ LFUN_GETXY, "server-get-xy", "", ReadOnly },
		{ LFUN_GOTOFILEROW, "server-goto-file-row", "", Noop },
		{ LFUN_NOTIFY, "server-notify", "", ReadOnly },
		{ LFUN_SETXY, "server-set-xy", "", ReadOnly },
		{ LFUN_SPELLCHECK, "spellchecker", "", Noop },
		{ LFUN_INSERT_MATH, "symbol-insert", "", Noop },
		{ LFUN_SHIFT_TAB, "tab-backward", "", Noop },
		{ LFUN_TAB, "tab-forward", "", Noop },
		{ LFUN_TABINSERT, "tab-insert", "", Noop },
		{ LFUN_TABLE, "table-insert", N_("Insert Table"), Noop },
		{ LFUN_INSET_TABULAR, "tabular-inset-insert",
		  N_("Insert a new Tabular Inset"), Noop },
		{ LFUN_TEX, "tex-mode", N_("Toggle TeX style"), Noop },
		{ LFUN_INSET_TEXT, "text-inset-insert",
		  N_("Insert a new Text Inset"), Noop },
		{ LFUN_TOC_INSERT, "toc-insert",
		  N_("Insert table of contents"), Noop },
		{ LFUN_TOCVIEW, "toc-view",
		  N_("View table of contents"), ReadOnly },
		{ LFUN_TOGGLECURSORFOLLOW, "toggle-cursor-follows-scrollbar",
		  N_("Toggle cursor does/doesn't follow the scrollbar"),
		  ReadOnly },
		{ LFUN_ADD_TO_TOOLBAR, "toolbar-add-to", "", NoBuffer },
		{ LFUN_PUSH_TOOLBAR, "toolbar-push", "", NoBuffer },
		{ LFUN_UNDO, "undo", N_("Undo"), Noop },
		{ LFUN_UP, "up", "", ReadOnly },
		{ LFUN_UPSEL, "up-select", "", ReadOnly },
		{ LFUN_URL, "url-insert", "", Noop },
		{ LFUN_VC_CHECKIN, "vc-check-in", "", ReadOnly },
		{ LFUN_VC_CHECKOUT, "vc-check-out", "", ReadOnly },
		{ LFUN_VC_HISTORY, "vc-history", "", ReadOnly },
		{ LFUN_VC_REGISTER, "vc-register",
		  N_("Register document under version control"), ReadOnly },
		{ LFUN_VC_REVERT, "vc-revert", "", ReadOnly },
		{ LFUN_VC_UNDO, "vc-undo-last", "", ReadOnly },
		{ LFUN_WORDLEFT, "word-backward", "", ReadOnly },
		{ LFUN_WORDLEFTSEL, "word-backward-select", "", ReadOnly },
		{ LFUN_CAPITALIZE_WORD, "word-capitalize", "", Noop },
		{ LFUN_DELETE_WORD_BACKWARD, "word-delete-backward",
		  "", Noop },
		{ LFUN_DELETE_WORD_FORWARD, "word-delete-forward", "", Noop },
		{ LFUN_WORDFINDBACKWARD, "word-find-backward", "", ReadOnly },
		{ LFUN_WORDFINDFORWARD, "word-find-forward", "", ReadOnly },
		{ LFUN_WORDRIGHT, "word-forward", "", ReadOnly },
		{ LFUN_WORDRIGHTSEL, "word-forward-select", "", ReadOnly },
		{ LFUN_LOWCASE_WORD, "word-lowcase", "", Noop },
		{ LFUN_UPCASE_WORD, "word-upcase", "", Noop },
		{ LFUN_DATE_INSERT, "date-insert", "", Noop },
		{ LFUN_PARAGRAPH_SPACING, "paragraph-spacing", "", Noop },
		{ LFUN_NOACTION, "", "", Noop }
	};

	int i = 0;
	while (items[i].action != LFUN_NOACTION) {
		newFunc(items[i].action,
			items[i].name,
			_(items[i].helpText),
			items[i].attrib);
		++i;
	}

	init = true;
}


LyXAction::LyXAction()
{
	init();
}


// Search for an existent pseudoaction, return LFUN_UNKNOWN_ACTION
// if it doesn't exist.
int LyXAction::searchActionArg(kb_action action, string const & arg) const
{
	arg_map::const_iterator pit = lyx_arg_map.find(action);

	if (pit == lyx_arg_map.end()) {
		// the action does not have any pseudoactions
		lyxerr[Debug::ACTION] << "Action " << action
				      << " does not have any pseudo actions."
				      << endl;
		return LFUN_UNKNOWN_ACTION;
	}
	
	arg_item::const_iterator aci = (*pit).second.find(arg);

	if (aci == (*pit).second.end()) {
		// the action does not have any pseudoactions with this arg
		lyxerr[Debug::ACTION] 
			<< "Action " << action
			<< "does not have any pseudoactions with arg "
			<< arg << endl;
		return LFUN_UNKNOWN_ACTION;
	}

	// pseudo action exist
	lyxerr[Debug::ACTION] << "Pseudoaction exist[" 
			      << action << '|' 
			      << arg << "] = " << (*aci).second << endl;

	return (*aci).second;
}


// Returns a pseudo-action given an action and its argument.
int LyXAction::getPseudoAction(kb_action action, string const & arg) const
{
	int psdaction = searchActionArg(action, arg);

	if (isPseudoAction(psdaction)) return psdaction;

	static unsigned int pseudo_counter = LFUN_LASTACTION;

	// Create new pseudo action.
	pseudo_func tmp_p;
	tmp_p.action = action;
	tmp_p.arg = arg;
	lyx_pseudo_map[++pseudo_counter] = tmp_p;

	// First ensure that the action is in lyx_arg_map;
	lyx_arg_map[action];
	// get the arg_item map
	arg_map::iterator ami = lyx_arg_map.find(action);
	// put the new pseudo function in it
	(*ami).second[arg] = pseudo_counter;

	lyxerr[Debug::ACTION] << "Creating new pseudoaction "
			      << pseudo_counter << " for [" << action
			      << '|' << arg << "]\n";
	
	return pseudo_counter;
}


// Retrieves the real action and its argument.
// perhaps a pair<kb_action, string> should be returned?
kb_action LyXAction::retrieveActionArg(int pseudo, string & arg) const
{
	pseudo_map::const_iterator pit = lyx_pseudo_map.find(pseudo);

	if (pit != lyx_pseudo_map.end()) {
		lyxerr[Debug::ACTION] << "Found the pseudoaction: ["
				      << (*pit).second.action << '|'
				      << (*pit).second.arg << '\n';
		arg = (*pit).second.arg;
		return (*pit).second.action;
	} else {
		lyxerr << "Lyx Error: Unrecognized pseudo-action\n";
		return LFUN_UNKNOWN_ACTION;
	}
	
}


// Returns an action tag from a string.
int LyXAction::LookupFunc(string const & func) const
{
	if (func.empty()) return LFUN_NOACTION;

	// split action and arg
	string actstr;
	string argstr = split(func, actstr, ' ');
	lyxerr[Debug::ACTION] << "Action: " << actstr << '\n';
	lyxerr[Debug::ACTION] << "Arg   : " << argstr << '\n';

	func_map::const_iterator fit = lyx_func_map.find(actstr);

	if (!argstr.empty() && fit != lyx_func_map.end()) {
		// might be pseudo (or create one)
		return getPseudoAction((*fit).second, argstr);
	}

	return fit != lyx_func_map.end() ? (*fit).second : LFUN_UNKNOWN_ACTION;
}


//#ifdef WITH_WARNINGS
//#warning Not working as it should.
//#endif
// I have no clue what is wrong with it... (Lgb)
int LyXAction::getApproxFunc(string const & func) const
	// This func should perhaps also be able to return a list of all
	// actions that has func as a prefix. That should actually be quite
	// easy, just let it return a vector<int> or something.
{
	int action = LookupFunc(func);
	if (action == LFUN_UNKNOWN_ACTION) {
		// func is not an action, but perhaps it is
		// part of one...check if it is prefix if one of the
		// actions.
		// Checking for prefix is not so simple, but
		// using a simple bounding function gives
		// a similar result.  [ale 19981103]
	 	func_map::const_iterator fit = 
 			lyx_func_map.lower_bound(func);
		
 		if (fit != lyx_func_map.end()) {
			action =  (*fit).second;
		}
	} else {  // Go get the next function
	 	func_map::const_iterator fit = 
 			lyx_func_map.upper_bound(func);
		
 		if (fit != lyx_func_map.end()) {
			action =  (*fit).second;
		}
	}
	
	return action;
}


string LyXAction::getApproxFuncName(string const & func) const
{
	int f = getApproxFunc(func);
	// This will return empty string if f isn't an action.
	return getActionName(f);
}


string LyXAction::getActionName(int action) const
{
	info_map::const_iterator iit =
		lyx_info_map.find(static_cast<kb_action>(action));

	return iit != lyx_info_map.end() ? (*iit).second.name : string();
}


// Returns one line help associated with a (pseudo)action, i.e. appends
// the argument of the action if necessary
string LyXAction::helpText(int pseudoaction) const
{
	string help, arg;
	kb_action action;

	if (isPseudoAction(pseudoaction)) 
		action = retrieveActionArg(pseudoaction, arg);
	else 
		action = static_cast<kb_action>(pseudoaction);

	info_map::const_iterator ici = lyx_info_map.find(action);
	if (ici != lyx_info_map.end()) {
		if (lyxerr.debugging(Debug::ACTION)) {
			lyxerr << "Action: " << action << '\n';
			lyxerr << "   name: "
			       << (*ici).second.name << '\n';
			lyxerr << " attrib: "
			       << (*ici).second.attrib << '\n';
			lyxerr << "   help: "
			       << (*ici).second.helpText << '\n';
		}
		help = (*ici).second.helpText;
		// if the is no help text use the name of the func instead.
		if (help.empty()) help = (*ici).second.name;
	}

	if (help.empty()) {
		help = _("No description available!");
	} else if (!arg.empty()) {
		help += ' ';
		help += arg;
	}

	return help;
}


bool LyXAction::funcHasFlag(kb_action action, 
                            LyXAction::func_attrib flag) const 
{
	info_map::const_iterator ici = lyx_info_map.find(action);

	if (ici != lyx_info_map.end()) {
		return (*ici).second.attrib & flag;
	} else {
		// it really should exist, but...
		lyxerr << "No info about kb_action: " << action << '\n';
		return false;
	}

}


ostream & operator<<(ostream & o, kb_action action)
{
	return o << int(action);
}
