/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
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


void LyXAction::init()
{
	static bool init = false;
	if (init) return;
	
	newFunc(LFUN_ACUTE, "accent-acute",
		"", Noop);
	newFunc( LFUN_BREVE, "accent-breve",
		 "", Noop);
	newFunc( LFUN_CARON,"accent-caron",
		 "", Noop);
	newFunc(LFUN_CEDILLA,"accent-cedilla",
		"", Noop);
	newFunc(LFUN_CIRCLE,"accent-circle",
		"", Noop);
	newFunc(LFUN_CIRCUMFLEX,"accent-circumflex",
		"", Noop);
	newFunc(LFUN_DOT,"accent-dot",
		"", Noop);
	newFunc(LFUN_GRAVE,"accent-grave",
		"", Noop);
	newFunc(LFUN_HUNG_UMLAUT,"accent-hungarian-umlaut",
		"", Noop);
	newFunc(LFUN_MACRON,"accent-macron",
		"", Noop);
	newFunc(LFUN_OGONEK,"accent-ogonek",
		"", Noop);
	newFunc(LFUN_SPECIAL_CARON,"accent-special-caron",
		"", Noop);
	newFunc(LFUN_TIE,"accent-tie",
		"", Noop);
	newFunc(LFUN_TILDE,"accent-tilde",
		"", Noop);
	newFunc(LFUN_UMLAUT,"accent-umlaut",
		"", Noop);
	newFunc(LFUN_UNDERBAR,"accent-underbar",
		"", Noop);
	newFunc(LFUN_UNDERDOT,"accent-underdot",
		"", Noop);
	newFunc(LFUN_VECTOR,"accent-vector",
		"", Noop);
	newFunc(LFUN_APPENDIX,"appendix",
		_("Insert appendix"), Noop);
	newFunc(LFUN_APROPOS,"apropos",
		_("Describe command"), NoBuffer|ReadOnly);
	newFunc(LFUN_LEFTSEL,"backward-select",
		_("Select previous char"), ReadOnly);
	newFunc(LFUN_BIBDB_ADD,"bibtex-database-add",
		"", Noop);
	newFunc(LFUN_BIBDB_DEL,"bibtex-database-del",
		"", Noop);
	newFunc(LFUN_INSERT_BIBTEX,"bibtex-insert",
		_("Insert bibtex"), Noop);
	newFunc(LFUN_BIBTEX_STYLE,"bibtex-style",
		"", Noop);
	newFunc(LFUN_BREAKLINE,"break-line",
		"", Noop);
	newFunc(LFUN_BREAKPARAGRAPH,"break-paragraph",
		"", Noop);
	newFunc(LFUN_BREAKPARAGRAPHKEEPLAYOUT,"break-paragraph-keep-layout",
		"", Noop);
	newFunc(LFUN_BREAKPARAGRAPH_SKIP,"break-paragraph-skip",
		"", Noop);
	newFunc(LFUN_BUILDPROG, "build-program", _("Build program"), Noop);
	newFunc(LFUN_AUTOSAVE,"buffer-auto-save",
		_("Autosave"), Noop);
	newFunc(LFUN_BEGINNINGBUF,"buffer-begin",
		_("Go to beginning of document"), ReadOnly);
	newFunc(LFUN_BEGINNINGBUFSEL,"buffer-begin-select",
		_("Select to beginning of document"), ReadOnly);
	newFunc(LFUN_CHILDINSERT,"buffer-child-insert",
		"", Noop);
	newFunc(LFUN_CHILDOPEN,"buffer-child-open",
		"", ReadOnly);
	newFunc(LFUN_RUNCHKTEX,"buffer-chktex",
		_("Check TeX"), ReadOnly);
	newFunc(LFUN_CLOSEBUFFER,"buffer-close",
		_("Close"), ReadOnly);
	newFunc(LFUN_ENDBUF,"buffer-end",
		_("Go to end of document"), ReadOnly);
	newFunc(LFUN_ENDBUFSEL,"buffer-end-select",
		_("Select to end of document"), ReadOnly);
	newFunc(LFUN_EXPORT,"buffer-export",
		_("Export to"), ReadOnly);
	newFunc(LFUN_FAX,"buffer-fax",
		_("Fax"), ReadOnly);
	newFunc(LFUN_INSERTFOOTNOTE, "buffer-float-insert", "", Noop);
	newFunc(LFUN_IMPORT,"buffer-import",
		_("Import document"), NoBuffer);
	newFunc(LFUN_BUFFERBULLETSSELECT, "buffer-itemize-bullets-select",
		"", Noop);
	newFunc( LFUN_MENUNEW,"buffer-new",
		_("New document") , NoBuffer);
	newFunc(LFUN_MENUNEWTMPLT,"buffer-new-template",
		_("New document from template"), NoBuffer);
	newFunc(LFUN_MENUOPEN,"buffer-open",
		_("Open"), NoBuffer);
	newFunc(LFUN_PREVBUFFER,"buffer-previous",
		_("Switch to previous document"), ReadOnly);
	newFunc(LFUN_MENUPRINT,"buffer-print",
		_("Print"), ReadOnly);
	newFunc(LFUN_MENURELOAD,"buffer-reload",
		_("Revert to saved"), ReadOnly);
	newFunc(LFUN_READ_ONLY_TOGGLE,"buffer-toggle-read-only",
		_("Toggle read-only"), ReadOnly);
	newFunc(LFUN_RUNLATEX,"buffer-typeset",
		_("Update DVI"), ReadOnly);
	newFunc(LFUN_RUNDVIPS,"buffer-typeset-ps",
		 _("Update PostScript"), ReadOnly);
	newFunc(LFUN_PREVIEW,"buffer-view",
		_("View DVI") , ReadOnly);
	newFunc(LFUN_PREVIEWPS,"buffer-view-ps",
		_("View PostScript") , ReadOnly);
	newFunc(LFUN_MENUWRITE,"buffer-write",
		_("Save"), ReadOnly);
	newFunc(LFUN_MENUWRITEAS,"buffer-write-as",
		_("Save As"), ReadOnly);
	newFunc(LFUN_CANCEL,"cancel",
		_("Cancel"), NoBuffer);
	newFunc(LFUN_LEFT,"char-backward",
		_("Go one char back"), ReadOnly);
	newFunc(LFUN_RIGHT,"char-forward",
		_("Go one char forward"), ReadOnly);
	newFunc(LFUN_INSERT_CITATION,"citation-insert",
		_("Insert citation"), Noop);
	newFunc(LFUN_EXEC_COMMAND,"command-execute",
		"", NoBuffer);
	newFunc(LFUN_PREFIX,"command-prefix",
		_("Execute command"), NoBuffer);
	newFunc(LFUN_SEQUENCE, "command-sequence", "", Noop);
	newFunc(LFUN_COPY,"copy",
		_("Copy"), ReadOnly);
	newFunc(LFUN_CUT,"cut",
		_("Cut"), Noop);
	newFunc(LFUN_BACKSPACE,"delete-backward",
		"", Noop);
	newFunc(LFUN_BACKSPACE_SKIP,"delete-backward-skip",
		"", Noop);
	newFunc(LFUN_DELETE,"delete-forward",
		"", Noop);
	newFunc(LFUN_DELETE_SKIP,"delete-forward-skip",
		"", Noop);
	newFunc(LFUN_DEPTH_MIN,"depth-decrement",
		_("Decrement environment depth"), Noop);
	newFunc(LFUN_DEPTH_PLUS,"depth-increment",
		_("Increment environment depth"), Noop);
	newFunc(LFUN_DEPTH,"depth-next",
		_("Change environment depth"), Noop);
	newFunc(LFUN_LDOTS,"dots-insert",
		_("Insert ... dots"), Noop);
	newFunc(LFUN_DOWN,"down",
		_("Go down"), ReadOnly);
	newFunc(LFUN_DOWNSEL,"down-select",
		_("Select next line"), ReadOnly);
	newFunc(LFUN_DROP_LAYOUTS_CHOICE,"drop-layouts-choice",
		_("Choose Paragraph Environment"), ReadOnly);
	newFunc(LFUN_END_OF_SENTENCE,"end-of-sentence-period-insert",
		_("Insert end of sentence period"), Noop);
	newFunc(LFUN_GOTOERROR,"error-next",
		_("Go to next error"), Noop);
	newFunc(LFUN_REMOVEERRORS,"error-remove-all",
		_("Remove all error boxes"), ReadOnly);
	newFunc(LFUN_FIGURE,"figure-insert",
		_("Insert Figure"), Noop);
	newFunc(LFUN_FILE_INSERT,"file-insert",
		"", Noop);
	newFunc(LFUN_FILE_INSERT_ASCII,"file-insert-ascii",
		"", Noop);
	newFunc(LFUN_FILE_NEW,"file-new",
		"", NoBuffer);
	newFunc(LFUN_FILE_OPEN,"file-open",
		"", NoBuffer);
	newFunc( LFUN_MENUSEARCH,"find-replace",
		_("Find & Replace") , Noop);
	newFunc(LFUN_BOLD,"font-bold",
		_("Toggle bold"), Noop);
	newFunc(LFUN_CODE,"font-code",
		_("Toggle code style"), Noop);
	newFunc(LFUN_DEFAULT,"font-default",
		_("Default font style"), Noop);
	newFunc(LFUN_EMPH,"font-emph",
		_("Toggle emphasize"), Noop);
	newFunc(LFUN_FREE,"font-free",
		_("Toggle user defined style"), Noop);
	newFunc(LFUN_NOUN,"font-noun",
		_("Toggle noun style"), Noop);
	newFunc(LFUN_ROMAN,"font-roman",
		_("Toggle roman font style"), Noop);
	newFunc(LFUN_SANS,"font-sans",
		_("Toggle sans font style"), Noop);
	newFunc(LFUN_FONT_SIZE,"font-size",
		_("Set font size"), Noop);
	newFunc(LFUN_FONT_STATE,"font-state",
		_("Show font state"), ReadOnly);
	newFunc(LFUN_UNDERLINE,"font-underline",
		_("Toggle font underline"), Noop);
	newFunc(LFUN_FOOTMELT,"footnote-insert",
		_("Insert Footnote"), Noop);
	newFunc(LFUN_RIGHTSEL,"forward-select",
		_("Select next char"), ReadOnly);
	newFunc(LFUN_HFILL,"hfill-insert",
		_("Insert horizontal fill"), Noop);
	newFunc(LFUN_HTMLURL,"html-insert",
		"", Noop);
	newFunc(LFUN_HYPHENATION,"hyphenation-point-insert",
		_("Insert hyphenation point"), Noop);
	newFunc(LFUN_INDEX_INSERT,"index-insert",
		_("Insert index item"), Noop);
	newFunc(LFUN_INDEX_INSERT_LAST, "index-insert-last",
		_("Insert last index item"), Noop);
	newFunc(LFUN_INDEX_PRINT,"index-print",
		_("Insert index list"), Noop);
	newFunc(LFUN_KMAP_OFF,"keymap-off",
		_("Turn off keymap"), ReadOnly);
	newFunc(LFUN_KMAP_PRIM,"keymap-primary",
		_("Use primary keymap"), ReadOnly);
	newFunc(LFUN_KMAP_SEC,"keymap-secondary",
		_("Use secondary keymap"), ReadOnly);
	newFunc(LFUN_KMAP_TOGGLE,"keymap-toggle",
		_("Toggle keymap"), ReadOnly);
	newFunc(LFUN_INSERT_LABEL,"label-insert",
		_("Insert Label"), Noop);
	newFunc(LFUN_LATEX_LOG,"latex-view-log",
		_("View LaTeX log"), ReadOnly);
	newFunc(LFUN_LAYOUT,"layout",
		"", Noop);
	newFunc(LFUN_LAYOUT_CHARACTER,"layout-character",
		"", Noop);
	newFunc(LFUN_LAYOUT_COPY,"layout-copy",
		_("Copy paragraph environment type"), Noop);
	newFunc(LFUN_LAYOUT_DOCUMENT,"layout-document",
		"", ReadOnly);
	newFunc(LFUN_LAYOUTNO,"layout-number",
		"", Noop); // internal only
	newFunc(LFUN_LAYOUT_PARAGRAPH,"layout-paragraph",
		"", ReadOnly);
	newFunc(LFUN_LAYOUT_PAPER, "layout-paper", "", ReadOnly);
	newFunc(LFUN_LAYOUT_PASTE,"layout-paste",
		_("Paste paragraph environment type"), Noop);
	newFunc(LFUN_LAYOUT_PREAMBLE,"layout-preamble",
		"", ReadOnly);
	newFunc(LFUN_LAYOUT_QUOTES, "layout-quotes", "", ReadOnly);
	newFunc(LFUN_LAYOUT_SAVE_DEFAULT, "layout-save-default",
		"", ReadOnly);
	newFunc(LFUN_LAYOUT_TABLE,"layout-table",
		"", Noop);
	newFunc(LFUN_HOME,"line-begin",
		_("Go to beginning of line"), ReadOnly);
	newFunc(LFUN_HOMESEL,"line-begin-select",
		_("Select to beginning of line"), ReadOnly);
	newFunc(LFUN_DELETE_LINE_FORWARD,"line-delete-forward",
		"", Noop);
	newFunc(LFUN_END,"line-end",
		_("Go to end of line"), ReadOnly);
	newFunc(LFUN_ENDSEL,"line-end-select",
		_("Select to end of line"), ReadOnly);
	newFunc(LFUN_LOA_INSERT,"loa-insert",
		_("Insert list of algorithms"), Noop);
	newFunc(LFUN_LOF_INSERT,"lof-insert",
		_("Insert list of figures"), Noop);
	newFunc(LFUN_LOT_INSERT,"lot-insert",
		_("Insert list of tables"), Noop);
	newFunc(LFUN_QUIT,"lyx-quit",
		_("Exit"), NoBuffer);
	newFunc(LFUN_MARGINMELT,"marginpar-insert",
		_("Insert Margin note"), Noop);
	newFunc(LFUN_MARK_OFF,"mark-off",
		"", ReadOnly);
	newFunc(LFUN_MARK_ON,"mark-on",
		"", ReadOnly);
	newFunc(LFUN_SETMARK,"mark-toggle",
		"", ReadOnly);
	newFunc(LFUN_MATH_DELIM,"math-delim",
		"", Noop);
	newFunc(LFUN_MATH_DISPLAY,"math-display",
		"", Noop);
	newFunc(LFUN_GREEK,"math-greek",
		_("Math Greek"), Noop);
	newFunc(LFUN_GREEK_TOGGLE,"math-greek-toggle",
		"", Noop);
	newFunc(LFUN_INSERT_MATH,"math-insert",
		_("Insert math symbol"), Noop);
	newFunc(LFUN_MATH_LIMITS,"math-limits",
		"", Noop);
	newFunc(LFUN_MATH_MACRO,"math-macro",
		"", Noop);
	newFunc(LFUN_MATH_MACROARG,"math-macro-arg",
		"", Noop);
	newFunc(LFUN_INSERT_MATRIX,"math-matrix",
		"", Noop);
	newFunc(LFUN_MATH_MODE,"math-mode",
		_("Math mode"), Noop);
	newFunc(LFUN_MATH_NONUMBER,"math-nonumber",
		"", Noop);
	newFunc(LFUN_MATH_NUMBER,"math-number",
		"", Noop);
	newFunc(LFUN_MATH_SIZE,"math-size",
		"", Noop);
	newFunc(LFUN_MELT,"melt",
		_("Melt"), Noop);
	newFunc(LFUN_MENU_OPEN_BY_NAME,"menu-open",
		"", NoBuffer);
	newFunc(LFUN_MENU_SEPARATOR, "menu-separator-insert", "", Noop);
	newFunc(LFUN_META_FAKE,"meta-prefix",
		"", NoBuffer);
	newFunc(LFUN_INSERT_NOTE,"note-insert",
		"", Noop);
	newFunc(LFUN_GOTONOTE,"note-next",
		"", ReadOnly);
	newFunc(LFUN_OPENSTUFF,"open-stuff",
		"", ReadOnly);
	newFunc(LFUN_DOWN_PARAGRAPH,"paragraph-down",
		_("Go one paragraph down"), ReadOnly);
	newFunc(LFUN_DOWN_PARAGRAPHSEL,"paragraph-down-select",
		_("Select next paragraph"), ReadOnly);
	newFunc(LFUN_UP_PARAGRAPH,"paragraph-up",
		_("Go one paragraph up"), ReadOnly);
	newFunc(LFUN_UP_PARAGRAPHSEL,"paragraph-up-select",
		 _("Select previous paragraph"), ReadOnly);
	newFunc(LFUN_PARENTINSERT,"parent-insert",
		"", Noop);
	newFunc(LFUN_PASTE,"paste",
		_("Paste") , Noop);
	newFunc(LFUN_SAVEPREFERENCES, "preferences-save",
		"Save Preferences", NoBuffer);
	newFunc(LFUN_PASTESELECTION,"primary-selection-paste",
		"", Noop);
	newFunc(LFUN_PROTECTEDSPACE,"protected-space-insert",
		_("Insert protected space"), Noop);
	newFunc(LFUN_QUOTE,"quote-insert",
		_("Insert quote"), Noop);
	newFunc(LFUN_RECONFIGURE,"reconfigure",
		_("Reconfigure"), NoBuffer);
	newFunc(LFUN_REDO,"redo",
		_("Redo"), Noop);
	newFunc(LFUN_REFBACK,"reference-back",
		"", ReadOnly);
	newFunc(LFUN_REFGOTO,"reference-goto",
		"", ReadOnly);
	newFunc(LFUN_INSERT_REF,"reference-insert",
		_("Insert cross reference"), Noop);
	newFunc(LFUN_REFTOGGLE,"reference-toggle",
		"", Noop);
	newFunc(LFUN_NEXT,"screen-down",
		"", ReadOnly);
	newFunc(LFUN_NEXTSEL,"screen-down-select",
		"", ReadOnly);
	newFunc(LFUN_CENTER,"screen-recenter",
		"", ReadOnly);
	newFunc(LFUN_PRIOR,"screen-up",
		"", ReadOnly);
	newFunc(LFUN_PRIORSEL,"screen-up-select",
		"", ReadOnly);
	newFunc(LFUN_SELFINSERT,"self-insert",
		"", Noop);
	newFunc(LFUN_CHARATCURSOR,"server-char-after",
		"", ReadOnly);
	newFunc(LFUN_GETFONT,"server-get-font",
		"", ReadOnly);
	newFunc(LFUN_GETLATEX,"server-get-latex",
		"", ReadOnly);
	newFunc(LFUN_GETLAYOUT,"server-get-layout",
		"", ReadOnly);
	newFunc(LFUN_GETNAME,"server-get-name",
		"", ReadOnly);
	newFunc(LFUN_GETTIP,"server-get-tip",
		"", ReadOnly);
	newFunc(LFUN_GETXY,"server-get-xy",
		"", ReadOnly);
	newFunc(LFUN_GOTOFILEROW, "server-goto-file-row", "", Noop);
	newFunc(LFUN_NOTIFY,"server-notify",
		"", ReadOnly);
	newFunc(LFUN_SETXY,"server-set-xy",
		"", ReadOnly);
	newFunc(LFUN_SPELLCHECK,"spellchecker",
		"", Noop);
	newFunc(LFUN_INSERT_MATH,"symbol-insert",
		"", Noop);
	newFunc(LFUN_TAB,"tab-forward",
		"", Noop);
	newFunc(LFUN_TABINSERT,"tab-insert",
		"", Noop);
	newFunc(LFUN_TABLE,"table-insert",
		_("Insert Table"), Noop);
	newFunc(LFUN_TEX,"tex-mode",
		_("Toggle TeX style"), Noop);
	newFunc(LFUN_TOC_INSERT,"toc-insert",
		_("Insert table of contents"), Noop);
	newFunc(LFUN_TOCVIEW,"toc-view",
		_("View table of contents"), ReadOnly);
	newFunc(LFUN_TOGGLECURSORFOLLOW,"toggle-cursor-follows-scrollbar",
		_("Toggle cursor does/doesn't follow the scrollbar"),
		ReadOnly);
	newFunc(LFUN_ADD_TO_TOOLBAR,"toolbar-add-to",
		"", NoBuffer);
	newFunc(LFUN_PUSH_TOOLBAR,"toolbar-push",
		"", NoBuffer);
	newFunc(LFUN_UNDO,"undo",
		_("Undo"), Noop);
	newFunc(LFUN_UP,"up",
		"", ReadOnly);
	newFunc(LFUN_UPSEL,"up-select",
		"", ReadOnly);
	newFunc(LFUN_URL,"url-insert",
		"", Noop);
	newFunc(LFUN_VC_CHECKIN,"vc-check-in",
		"", ReadOnly);
	newFunc(LFUN_VC_CHECKOUT,"vc-check-out",
		"", ReadOnly);
	newFunc(LFUN_VC_HISTORY,"vc-history",
		"", ReadOnly);
	newFunc(LFUN_VC_REGISTER,"vc-register",
		_("Register document under version control"), ReadOnly);
	newFunc(LFUN_VC_REVERT,"vc-revert",
		"", ReadOnly);
	newFunc(LFUN_VC_UNDO,"vc-undo-last",
		"", ReadOnly);
	newFunc(LFUN_WORDLEFT,"word-backward",
		"", ReadOnly);
	newFunc(LFUN_WORDLEFTSEL,"word-backward-select",
		"", ReadOnly);
	newFunc(LFUN_CAPITALIZE_WORD,"word-capitalize",
		"", Noop);
	newFunc(LFUN_DELETE_WORD_BACKWARD,"word-delete-backward",
		"", Noop);
	newFunc(LFUN_DELETE_WORD_FORWARD,"word-delete-forward",
		"", Noop);
	newFunc(LFUN_WORDFINDBACKWARD,"word-find-backward",
		"", ReadOnly);
	newFunc(LFUN_WORDFINDFORWARD,"word-find-forward",
		"", ReadOnly);
	newFunc(LFUN_WORDRIGHT,"word-forward",
		"", ReadOnly);
	newFunc(LFUN_WORDRIGHTSEL,"word-forward-select",
		"", ReadOnly);
	newFunc(LFUN_LOWCASE_WORD,"word-lowcase",
		"", Noop);
	newFunc(LFUN_UPCASE_WORD,"word-upcase",
		"", Noop);

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
				    << " does not have any pseudo actions.\n";
		return LFUN_UNKNOWN_ACTION;
	}
	
	arg_item::const_iterator aci = (*pit).second.find(arg);

	if (aci == (*pit).second.end()) {
		// the action does not have any pseudoactions with this arg
		lyxerr[Debug::ACTION] 
			<< "Action " << action
			<< "does not have any pseudoactions with arg "
			<< arg << '\n';
		return LFUN_UNKNOWN_ACTION;
	}

	// pseudo action exist
	lyxerr[Debug::ACTION] << "Pseudoaction exist[" 
			    << action << '|' 
			    << arg << "] = " << (*aci).second << '\n';

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


#warning Not working as it should.
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
    info_map::const_iterator iit = lyx_info_map.find((kb_action)action);

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
		action = (kb_action) pseudoaction;

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
