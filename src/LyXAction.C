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

#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>

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


/* This table is sorted alphabetically [asierra 14Jan96] */
/* This table MUST be sorted alphabetically, incidentally! */ 
kb_func_table const lyx_func_table[] = {
	{ "accent-acute",                         LFUN_ACUTE },
	{ "accent-breve",                         LFUN_BREVE },
	{ "accent-caron",                         LFUN_CARON },
	{ "accent-cedilla",                       LFUN_CEDILLA },
	{ "accent-circle",                        LFUN_CIRCLE },
	{ "accent-circumflex",                    LFUN_CIRCUMFLEX },
	{ "accent-dot",                           LFUN_DOT },
	{ "accent-grave",                         LFUN_GRAVE },
	{ "accent-hungarian-umlaut",              LFUN_HUNG_UMLAUT },
	{ "accent-macron",                        LFUN_MACRON },
	{ "accent-ogonek",                        LFUN_OGONEK },
	{ "accent-special-caron",                 LFUN_SPECIAL_CARON },
	{ "accent-tie",                           LFUN_TIE },
	{ "accent-tilde",                         LFUN_TILDE },
	{ "accent-umlaut",                        LFUN_UMLAUT },
	{ "accent-underbar",                      LFUN_UNDERBAR },
	{ "accent-underdot",                      LFUN_UNDERDOT },
	{ "accent-vector",                        LFUN_VECTOR }, 
	{ "appendix",                         LFUN_APPENDIX }, 
	{ "apropos",				  LFUN_APROPOS },
	{ "backward-select",                      LFUN_LEFTSEL },
	{ "bibtex-database-add",                  LFUN_BIBDB_ADD },
	{ "bibtex-database-del",                  LFUN_BIBDB_DEL },
	{ "bibtex-insert",                        LFUN_INSERT_BIBTEX },
	{ "bibtex-style",                         LFUN_BIBTEX_STYLE },
	{ "break-line",                           LFUN_BREAKLINE },
	{ "break-paragraph",                      LFUN_BREAKPARAGRAPH },
	{ "break-paragraph-keep-layout",          LFUN_BREAKPARAGRAPHKEEPLAYOUT },
	{ "break-paragraph-skip",                 LFUN_BREAKPARAGRAPH_SKIP },
	{ "buffer-auto-save",                     LFUN_AUTOSAVE },
	{ "buffer-begin",                         LFUN_BEGINNINGBUF },
	{ "buffer-begin-select",                  LFUN_BEGINNINGBUFSEL },
	{ "buffer-child-insert",                  LFUN_CHILDINSERT },  // ale970521
	{ "buffer-child-open",                    LFUN_CHILDOPEN },  // ale970528
	{ "buffer-chktex",			  LFUN_RUNCHKTEX }, // Asger 971030
	{ "buffer-close",                         LFUN_CLOSEBUFFER },         
	{ "buffer-end",                           LFUN_ENDBUF },
	{ "buffer-end-select",                    LFUN_ENDBUFSEL },
	{ "buffer-export",                        LFUN_EXPORT },
	{ "buffer-fax",                           LFUN_FAX },
	{ "buffer-float-insert",                  LFUN_INSERTFOOTNOTE },
	{ "buffer-import",                        LFUN_IMPORT },
	{ "buffer-itemize-bullets-select",        LFUN_BUFFERBULLETSSELECT },
	{ "buffer-new",                           LFUN_MENUNEW },     
	{ "buffer-new-template",                  LFUN_MENUNEWTMPLT },
	{ "buffer-open",                          LFUN_MENUOPEN },    
	{ "buffer-previous",                      LFUN_PREVBUFFER },  
	{ "buffer-print",                         LFUN_MENUPRINT },   
	{ "buffer-reload",                        LFUN_MENURELOAD },   
	{ "buffer-toggle-read-only",              LFUN_READ_ONLY_TOGGLE },
	{ "buffer-typeset",                       LFUN_RUNLATEX },        // Alejandro's proposal
	{ "buffer-typeset-ps",			  LFUN_RUNDVIPS },
	{ "buffer-view",                          LFUN_PREVIEW },
	{ "buffer-view-ps",                       LFUN_PREVIEWPS },
	{ "buffer-write",                         LFUN_MENUWRITE },   
	{ "buffer-write-as",                      LFUN_MENUWRITEAS }, 
        { "build-program",                        LFUN_BUILDPROG },
	{ "cancel",                               LFUN_CANCEL },
	{ "char-backward",			  LFUN_LEFT },
	{ "char-forward",			  LFUN_RIGHT },
        { "citation-insert",			  LFUN_INSERT_CITATION },
	{ "command-execute", 			  LFUN_EXEC_COMMAND },
	{ "command-prefix",                       LFUN_PREFIX },
	{ "command-sequence",                       LFUN_SEQUENCE },
	{ "copy",                                 LFUN_COPY },
	{ "cut",                                  LFUN_CUT },
	{ "delete-backward",                      LFUN_BACKSPACE },
	{ "delete-backward-skip",                 LFUN_BACKSPACE_SKIP },
	{ "delete-forward",                       LFUN_DELETE },
	{ "delete-forward-skip",                  LFUN_DELETE_SKIP },
	{ "depth-decrement",                      LFUN_DEPTH_MIN },
	{ "depth-increment",                      LFUN_DEPTH_PLUS },  
	{ "depth-next",                           LFUN_DEPTH },
	{ "dots-insert",			  LFUN_LDOTS },
	{ "down",                                 LFUN_DOWN },                 
	{ "down-select",                          LFUN_DOWNSEL },
	{ "drop-layouts-choice",                  LFUN_DROP_LAYOUTS_CHOICE },
	{ "end-of-sentence-period-insert",	  LFUN_END_OF_SENTENCE },
	{ "error-next",                           LFUN_GOTOERROR },   
	{ "error-remove-all",			  LFUN_REMOVEERRORS },
	{ "figure-insert",                        LFUN_FIGURE },
	{ "file-insert",                          LFUN_FILE_INSERT },
	{ "file-insert-ascii",                    LFUN_FILE_INSERT_ASCII },
	{ "file-new",                             LFUN_FILE_NEW },
	{ "file-open",                            LFUN_FILE_OPEN },
	{ "find-replace",                         LFUN_MENUSEARCH },  
	{ "font-bold",                            LFUN_BOLD },        
	{ "font-code",                            LFUN_CODE },        
	{ "font-default",                         LFUN_DEFAULT },
	{ "font-emph",                            LFUN_EMPH },
	{ "font-free",                            LFUN_FREE },
	{ "font-noun",                            LFUN_NOUN },
	{ "font-roman",                           LFUN_ROMAN },
	{ "font-sans",                            LFUN_SANS },
	{ "font-size",                            LFUN_FONT_SIZE },
	{ "font-state",                           LFUN_FONT_STATE }, 
	{ "font-underline",                       LFUN_UNDERLINE },   
	{ "footnote-insert",                      LFUN_FOOTMELT },    
	{ "forward-select",                       LFUN_RIGHTSEL },
	{ "hfill-insert",                         LFUN_HFILL },
	{ "html-insert",                          LFUN_HTMLURL },
	{ "hyphenation-point-insert",             LFUN_HYPHENATION },
	{ "index-insert",                         LFUN_INDEX_INSERT },
	{ "index-insert-last",                    LFUN_INDEX_INSERT_LAST },
	{ "index-print",                          LFUN_INDEX_PRINT },
#if 0
	{ "inset-latex-insert",                   LFUN_INSERT_INSET_LATEX },
#endif
	{ "keymap-off",                           LFUN_KMAP_OFF },
	{ "keymap-primary",                       LFUN_KMAP_PRIM },
	{ "keymap-secondary",                     LFUN_KMAP_SEC },
	{ "keymap-toggle",                        LFUN_KMAP_TOGGLE },
	{ "label-insert",                         LFUN_INSERT_LABEL },
	{ "latex-view-log",                       LFUN_LATEX_LOG },
	{ "layout",                               LFUN_LAYOUT },
	{ "layout-character",                     LFUN_LAYOUT_CHARACTER },
	{ "layout-copy",			  LFUN_LAYOUT_COPY },
	{ "layout-document",                      LFUN_LAYOUT_DOCUMENT },
	//{ "layout-number",                      LFUN_LAYOUTNO }, // internal only
	{ "layout-paper",			  LFUN_LAYOUT_PAPER },
	{ "layout-paragraph",                     LFUN_LAYOUT_PARAGRAPH },
	{ "layout-paste",			  LFUN_LAYOUT_PASTE },
	{ "layout-preamble",                      LFUN_LAYOUT_PREAMBLE },
	{ "layout-quotes",                        LFUN_LAYOUT_QUOTES },
	{ "layout-save-default",                  LFUN_LAYOUT_SAVE_DEFAULT },
	{ "layout-table",                         LFUN_LAYOUT_TABLE },
	{ "line-begin",                           LFUN_HOME },        
	{ "line-begin-select",                    LFUN_HOMESEL },     
	{ "line-delete-forward",                  LFUN_DELETE_LINE_FORWARD },  
	{ "line-end",                             LFUN_END },         
	{ "line-end-select",                      LFUN_ENDSEL },
	{ "loa-insert",                           LFUN_LOA_INSERT },
	{ "lof-insert",                           LFUN_LOF_INSERT },
	{ "lot-insert",                           LFUN_LOT_INSERT },
	{ "lyx-quit",                             LFUN_QUIT },
	{ "marginpar-insert",                     LFUN_MARGINMELT },  
	{ "mark-off",                             LFUN_MARK_OFF },    
	{ "mark-on",                              LFUN_MARK_ON },     
	{ "mark-toggle",                          LFUN_SETMARK }, 
	{ "math-delim",                           LFUN_MATH_DELIM },
	{ "math-display",                         LFUN_MATH_DISPLAY }, // Alejandro's proposal
	{ "math-greek",                           LFUN_GREEK },   
	{ "math-greek-toggle",                    LFUN_GREEK_TOGGLE },   
	{ "math-insert",                          LFUN_INSERT_MATH },   
	{ "math-limits",                          LFUN_MATH_LIMITS },
        { "math-macro",                           LFUN_MATH_MACRO },
        { "math-macro-arg",                       LFUN_MATH_MACROARG },
	{ "math-matrix",                          LFUN_INSERT_MATRIX }, 
	{ "math-mode",                            LFUN_MATH_MODE },
	{ "math-nonumber",                        LFUN_MATH_NONUMBER },
	{ "math-number",                          LFUN_MATH_NUMBER }, 
	{ "math-size",                            LFUN_MATH_SIZE }, 
	{ "melt",                                 LFUN_MELT },            // Needs better name. What about "float-disolve" or "float-extract" (Lgb)
	{ "menu-open",                            LFUN_MENU_OPEN_BY_NAME },
	{ "menu-separator-insert",                LFUN_MENU_SEPARATOR },
	{ "meta-prefix",                          LFUN_META_FAKE },
	{ "note-insert",                          LFUN_INSERT_NOTE },
	{ "note-next",                            LFUN_GOTONOTE },
	{ "open-stuff",                           LFUN_OPENSTUFF },       // Needs better name.
	{ "paragraph-down",			  LFUN_DOWN_PARAGRAPH },
	{ "paragraph-down-select",		  LFUN_DOWN_PARAGRAPHSEL },
	{ "paragraph-up",			  LFUN_UP_PARAGRAPH },
	{ "paragraph-up-select",		  LFUN_UP_PARAGRAPHSEL },
        { "parent-insert",		          LFUN_PARENTINSERT },
	{ "paste",                                LFUN_PASTE },
	{ "primary-selection-paste",              LFUN_PASTESELECTION },
	{ "protected-space-insert",               LFUN_PROTECTEDSPACE },
	{ "quote-insert",                         LFUN_QUOTE },
	{ "reconfigure",			  LFUN_RECONFIGURE },
	{ "redo",                                 LFUN_REDO },
        { "reference-back",                       LFUN_REFBACK },
        { "reference-goto",                       LFUN_REFGOTO }, 
	{ "reference-insert",                     LFUN_INSERT_REF },
	{ "reference-toggle",                     LFUN_REFTOGGLE },
	{ "screen-down",                          LFUN_NEXT },        
	{ "screen-down-select",                   LFUN_NEXTSEL },     
	{ "screen-recenter",                      LFUN_CENTER },
	{ "screen-up",                            LFUN_PRIOR },       
	{ "screen-up-select",                     LFUN_PRIORSEL },    
	{ "self-insert",                          LFUN_SELFINSERT },
	{ "server-char-after",                    LFUN_CHARATCURSOR },
	{ "server-get-font",                      LFUN_GETFONT },
	{ "server-get-latex",                     LFUN_GETLATEX },
	{ "server-get-layout",                    LFUN_GETLAYOUT },
	{ "server-get-name",                      LFUN_GETNAME },
	{ "server-get-tip",                       LFUN_GETTIP },
	{ "server-get-xy",                        LFUN_GETXY },
	{ "server-goto-file-row",                 LFUN_GOTOFILEROW },
	{ "server-notify",                        LFUN_NOTIFY },
	{ "server-set-xy",                        LFUN_SETXY },
	{ "spellchecker",                         LFUN_SPELLCHECK },  
	{ "symbol-insert",                        LFUN_INSERT_MATH },
	{ "tab-forward",                          LFUN_TAB },
	{ "tab-insert",                           LFUN_TABINSERT },
	{ "table-insert",                         LFUN_TABLE },
	{ "tex-mode",                             LFUN_TEX },
	{ "toc-insert",                           LFUN_TOC_INSERT },
	{ "toc-view",                             LFUN_TOCVIEW },
	{ "toggle-cursor-follows-scrollbar",      LFUN_TOGGLECURSORFOLLOW },
	{ "toolbar-add-to",                       LFUN_ADD_TO_TOOLBAR },
	{ "toolbar-push",                         LFUN_PUSH_TOOLBAR },
	{ "undo",                                 LFUN_UNDO },
	{ "up",                                   LFUN_UP },          
	{ "up-select",                            LFUN_UPSEL },
	{ "url-insert",                           LFUN_URL },
	{ "vc-check-in",                          LFUN_VC_CHECKIN }, 
	{ "vc-check-out",                         LFUN_VC_CHECKOUT }, 
	{ "vc-history",                           LFUN_VC_HISTORY }, 
	{ "vc-register",                          LFUN_VC_REGISTER }, 
	{ "vc-revert",                            LFUN_VC_REVERT }, 
	{ "vc-undo-last",                         LFUN_VC_UNDO }, 
	{ "word-backward",                        LFUN_WORDLEFT },
	{ "word-backward-select",                 LFUN_WORDLEFTSEL },
	{ "word-capitalize",                      LFUN_CAPITALIZE_WORD },
	{ "word-delete-backward",                 LFUN_DELETE_WORD_BACKWARD },
	{ "word-delete-forward",                  LFUN_DELETE_WORD_FORWARD }, 
	{ "word-find-backward",                   LFUN_WORDFINDBACKWARD },
	{ "word-find-forward",                    LFUN_WORDFINDFORWARD },
	{ "word-forward",                         LFUN_WORDRIGHT },
	{ "word-forward-select",                  LFUN_WORDRIGHTSEL },
	{ "word-lowcase",                         LFUN_LOWCASE_WORD },
	{ "word-upcase",                          LFUN_UPCASE_WORD }
};


/* 
   This table contains the actions that modify a buffer and therefore
   are not allowed for RO files. Do you think we have too much tables?
   Each single integer in this table replaces 5 lines of code in lyxfunc
   that include at least 3 function calls (several integers in the 
   code segment).
 
   The table could in the near future be expanded adding a second integer 
   field with attributes using these tags:

enum FUNC_ATTRIB {
    LFAT_NONE= 0,     // Nothing special
    LFAT_ISRO= 1,     // Is readonly (does not modify a buffer)
    LFAT_ISIA= 2,     // Is interactive (requires a GUI)
    LFAT_REQARG= 4,   // Requires argument
    LFAT_ISMO= 8,     // Only math mode
    LFAT_ETCETC
};

struct {
    kb_action action;
    unsigned  attrib; 
};

Alejandro-970604
*/

kb_action func_attrib_table[] = {
	LFUN_ACUTE,
	LFUN_AUTOSAVE,
	LFUN_BACKSPACE,
	LFUN_BACKSPACE_SKIP,
	LFUN_BIBDB_ADD,
	LFUN_BIBDB_DEL,
	LFUN_BIBTEX_STYLE,
	LFUN_BOLD,
	LFUN_BREAKLINE,
	LFUN_BREAKPARAGRAPH,
	LFUN_BREAKPARAGRAPHKEEPLAYOUT,
	LFUN_BREAKPARAGRAPH_SKIP,
	LFUN_BREVE,
	LFUN_BUFFERBULLETSSELECT,
	LFUN_CAPITALIZE_WORD,
	LFUN_CARON,
	LFUN_CEDILLA,
	LFUN_CHILDINSERT,
	LFUN_CIRCLE,
	LFUN_CIRCUMFLEX,
	LFUN_CODE,
	LFUN_CUT,
	LFUN_DEFAULT,
	LFUN_DELETE,
	LFUN_DELETE_LINE_FORWARD,
	LFUN_DELETE_SKIP,
	LFUN_DELETE_WORD_BACKWARD,
	LFUN_DELETE_WORD_FORWARD,
	LFUN_DEPTH,
	LFUN_DEPTH_MIN,
	LFUN_DEPTH_PLUS,
	LFUN_DOT,
	LFUN_EMPH,
	LFUN_END_OF_SENTENCE,
	LFUN_FIGURE,
	LFUN_FILE_INSERT,
	LFUN_FILE_INSERT_ASCII,
	LFUN_FONT_SIZE,
	LFUN_FOOTMELT,
	LFUN_FREE,
	LFUN_GRAVE,
	LFUN_HFILL,
	LFUN_HTMLURL,
	LFUN_HUNG_UMLAUT,
	LFUN_HYPHENATION,
	LFUN_INDEX_INSERT,
	LFUN_INDEX_INSERT_LAST,
	LFUN_INDEX_PRINT,
	LFUN_INSERTFOOTNOTE,
	LFUN_INSERT_BIBTEX,
	LFUN_INSERT_CITATION,
#if 0
	LFUN_INSERT_INSET_LATEX,
#endif
	LFUN_INSERT_LABEL,
	LFUN_INSERT_MATH,
	LFUN_INSERT_MATRIX,
	LFUN_INSERT_NOTE,
	LFUN_INSERT_REF,
	LFUN_LAYOUT,
	LFUN_LAYOUTNO,
	LFUN_LAYOUT_CHARACTER,
	LFUN_LAYOUT_PASTE,
	LFUN_LAYOUT_QUOTES,
	LFUN_LDOTS,
	LFUN_LOA_INSERT,
	LFUN_LOF_INSERT,
	LFUN_LOT_INSERT,
	LFUN_LOWCASE_WORD,
	LFUN_MACRON,
	LFUN_MARGINMELT,
	LFUN_MATH_DELIM,
	LFUN_MATH_DISPLAY,
	LFUN_MATH_MACRO,
	LFUN_MATH_MACROARG,
	LFUN_MATH_MODE,
	LFUN_MATH_NONUMBER,
	LFUN_MATH_NUMBER,
	LFUN_MATH_SIZE,
	LFUN_MELT,
	LFUN_MENU_SEPARATOR,
	LFUN_NOUN,
	LFUN_OGONEK,
	LFUN_PARENTINSERT,
	LFUN_PASTE,
	LFUN_PASTESELECTION,
	LFUN_PROTECTEDSPACE,
	LFUN_QUOTE,
	LFUN_REDO,
	LFUN_REFTOGGLE,
	LFUN_ROMAN,
	LFUN_SANS,
	LFUN_SELFINSERT,
	LFUN_SPECIAL_CARON,
	LFUN_TABINSERT,
	LFUN_TABLE,
	LFUN_TEX,
	LFUN_TIE,
	LFUN_TILDE,
	LFUN_TOC_INSERT,
	LFUN_UMLAUT,
	LFUN_UNDERBAR,
	LFUN_UNDERDOT,
	LFUN_UNDERLINE,
	LFUN_UNDO,
	LFUN_UNKNOWN_ACTION,
	LFUN_UPCASE_WORD,
	LFUN_URL,
	LFUN_VECTOR,
	LFUN_WORDFINDFORWARD,
	LFUN_WORDFINDBACKWARD
};

int LyXAction::psd_idx = 0;
kb_func_table const * LyXAction::lyx_func_table = &::lyx_func_table[0];
kb_func_table* LyXAction::lyx_func_args = 0;
 
/* === code ============================================================== */

/* This routines allow binding actions with argument.
 * Provisionally a fixed size array and global functions are used.
 * [asierra 20Jan96]
 */
#define MAX_PSEUDO_ACTION 128


LyXAction::LyXAction()
{
	lyx_func_args= new kb_func_table[MAX_PSEUDO_ACTION];
        funcCount = sizeof(::lyx_func_table) / sizeof(::kb_func_table);
}


LyXAction::~LyXAction()
{
	if (lyx_func_args) {
// This is wrong, so I'll just disable it for now.
// The problem is that we might free memory twice in some situations...
// It's better to leak a bit that to crash. (Asger)
//		for (int i= 0; i < psd_idx; i++) {
//			free(lyx_func_args[i].name);
//		}
		delete[] lyx_func_args;
		lyx_func_args = 0;
	}
}


// Search for an existent pseudoaction, return -1 if it doesn't exist.
int  LyXAction::searchActionArg(kb_action action, char const *arg)
{
	kb_func_table *tb = &lyx_func_args[0];
	for (int i= 0; i<psd_idx; i++) {
		if (action == tb->action && !strcmp(tb->name, arg)) {	 

			lyxerr[Debug::KEY] << "Pseudoaction already exist[" 
					   << action << '|' 
					   << arg << ']' << endl;

			return LFUN_LASTACTION+i;
		}
		tb++;
	}
	return -1;
}


// Returns a pseudo-action given an action and its argument.
int LyXAction::getPseudoAction(kb_action action, char const *arg)
{
	// Check if the pseudo-action already exist.
	int psdaction = searchActionArg(action, arg);
   
	if (psdaction >= 0) return psdaction;

	if (psd_idx>= MAX_PSEUDO_ACTION) {
		lyxerr << "Lyx Error: No more pseudo-actions allowed"
		       << "           Tell the developers." << endl;
		return -1;
	}
	lyx_func_args[psd_idx].name = strdup(arg);
	lyx_func_args[psd_idx].action = action;
	psd_idx++;
	return LFUN_LASTACTION+psd_idx-1;
}


// Retrieves the real action and its argument.
int LyXAction::retrieveActionArg(int i, char const** arg)
{
	i -= LFUN_LASTACTION;
	if (i >= 0 &&  i <psd_idx) {
		*arg = lyx_func_args[i].name;
		return (int)lyx_func_args[i].action;
	} else {
		lyxerr << "Lyx Error: Unrecognized pseudo-action" << endl;
		return -1;
	}
}


// Returns an action tag from a string.
int LyXAction::LookupFunc(char const * func)
{
	if (!func) 
		return LFUN_UNKNOWN_ACTION;         
	if (func[0] == '\0')
		return LFUN_NOACTION;
 
   /* In the scan loop below, l can never become r, so set r one past the last
      valid func table slot. RVDK_PATCH_5 */
	int k, l= 0, r = funcCount;
	int action = LFUN_UNKNOWN_ACTION;
	char * arg = strchr(const_cast<char*>(func), ' '); // const cast to
	// help a sun complier, will go away when this func is rewritten
	// to use std::containers and std::string.

	if (arg) *(arg++) = '\0';      

	while (l < r) {
		last_action_idx = (l+r)/2;
		k = strcmp(lyx_func_table[last_action_idx].name, func);
		if (k == 0) {
			action = lyx_func_table[last_action_idx].action;
			break;
		} else
			if (k<0) l = last_action_idx+1; else r = last_action_idx;
	}
	if (arg && action >= 0) {
		action = getPseudoAction((kb_action)action, arg);
		lyxerr[Debug::KEY] << "Pseudo action_arg["
				   << action << '|' 
				   << arg << ']' << endl;
	}
	return action;
}


int LyXAction::getApproxFunc(char const *func)
{
    int action = LookupFunc(func);
    if (action<0) {
	int k = strncmp(lyx_func_table[last_action_idx].name,
			func, strlen(func));
	if (k<0 && last_action_idx<funcCount-1) 
	  last_action_idx++;
	else if (k>0 && last_action_idx>0)
	  last_action_idx--;
	
	action = lyx_func_table[last_action_idx].action;
    } else if (last_action_idx<funcCount-1)
      last_action_idx++;
    
    return action;
}


char const *LyXAction::getApproxFuncName(char const *func)
{
    getApproxFunc(func);
    return lyx_func_table[last_action_idx].name;
}


char const *LyXAction::getActionName(int action) const
{
	// why LFUN_LASTACTION -1? Because LFUN_LASTACTION is too
	// large an because there is one internal command, hence
	// lyx_func_table is 2 less then LFUN_LASTACTION (Lgb)
    for (int i= 0; i < funcCount; i++) {
	if (lyx_func_table[i].action == action)
	  return lyx_func_table[i].name;
    }
    
    // Hmm.. let's see whether this is a pseudoaction
    action -= LFUN_LASTACTION;
    if (action>= 0 && action<psd_idx) {
	return lyx_func_args[action].name;
    }
    
    return "";
}


// Returns one line help associated with function
// Now the missing strings are replaced by the command names. (Alejandro)
char const *LyXAction::helpText(kb_action action) const
{
        static bool is_sorted = false;
	static kb_func_table helpTexts[LFUN_LASTACTION] = 
	{
		{ _("Describe command"), LFUN_APROPOS },
		{ _("Select previous char"), LFUN_LEFTSEL },
		{ _("Insert bibtex"), LFUN_INSERT_BIBTEX },
		{ _("Autosave"), LFUN_AUTOSAVE },
		{ _("Go to beginning of document"), LFUN_BEGINNINGBUF },
		{ _("Select to beginning of document"), LFUN_BEGINNINGBUFSEL },
		{ _("Close"), LFUN_CLOSEBUFFER },
		{ _("Go to end of document"), LFUN_ENDBUF },
		{ _("Select to end of document"), LFUN_ENDBUFSEL },
		{ _("Fax"), LFUN_FAX },
		{ _("New document"), LFUN_MENUNEW },
		{ _("New document from template"), LFUN_MENUNEWTMPLT },
		{ _("Open"), LFUN_MENUOPEN },
		{ _("Switch to previous document"), LFUN_PREVBUFFER },
		{ _("Print"), LFUN_MENUPRINT },
		{ _("Revert to saved"), LFUN_MENURELOAD },
		{ _("Update DVI"), LFUN_RUNLATEX },
		{ _("Update PostScript"), LFUN_RUNDVIPS },
		{ _("View DVI"), LFUN_PREVIEW },
		{ _("View PostScript"), LFUN_PREVIEWPS },
                { _("Build program"), LFUN_BUILDPROG },
		{ _("Check TeX"), LFUN_RUNCHKTEX },
		{ _("Save"), LFUN_MENUWRITE },
		{ _("Save As"), LFUN_MENUWRITEAS },
		{ _("Cancel"), LFUN_CANCEL },
		{ _("Go one char back"), LFUN_LEFT },
		{ _("Go one char forward"), LFUN_RIGHT },
		{ _("Insert citation"), LFUN_INSERT_CITATION },
		{ _("Execute command"), LFUN_EXEC_COMMAND },
		{ _("Copy"), LFUN_COPY },
		{ _("Cut"), LFUN_CUT },
		{ _("Decrement environment depth"), LFUN_DEPTH_MIN },
		{ _("Increment environment depth"), LFUN_DEPTH_PLUS },
		{ _("Change environment depth"), LFUN_DEPTH },
		{ _("Change itemize bullet settings"), LFUN_BUFFERBULLETSSELECT },
		{ _("Go down"), LFUN_DOWN },
		{ _("Select next line"), LFUN_DOWNSEL },
		{ _("Choose Paragraph Environment"), LFUN_DROP_LAYOUTS_CHOICE },
		{ _("Go to next error"), LFUN_GOTOERROR },
		{ _("Insert Figure"), LFUN_FIGURE },
		{ _("Find & Replace"), LFUN_MENUSEARCH },
		{ _("Toggle cursor does/doesn't follow the scrollbar"), LFUN_TOGGLECURSORFOLLOW },
		{ _("Toggle bold"), LFUN_BOLD },
		{ _("Toggle code style"), LFUN_CODE },
		{ _("Default font style"), LFUN_DEFAULT },
		{ _("Toggle emphasize"), LFUN_EMPH },
		{ _("Toggle user defined style"), LFUN_FREE },
		{ _("Toggle noun style"), LFUN_NOUN },
		{ _("Toggle roman font style"), LFUN_ROMAN },
		{ _("Toggle sans font style"), LFUN_SANS },
		{ _("Set font size"), LFUN_FONT_SIZE },
		{ _("Show font state"), LFUN_FONT_STATE },
		{ _("Toggle font underline"), LFUN_UNDERLINE },
		{ _("Insert Footnote"), LFUN_FOOTMELT },
		{ _("Select next char"), LFUN_RIGHTSEL },
		{ _("Insert horizontal fill"), LFUN_HFILL },
		{ _("Insert hyphenation point"), LFUN_HYPHENATION },
		{ _("Insert ... dots"), LFUN_LDOTS },
		{ _("Insert end of sentence period"), LFUN_END_OF_SENTENCE },
		{ _("Turn off keymap"), LFUN_KMAP_OFF },
		{ _("Use primary keymap"), LFUN_KMAP_PRIM },
		{ _("Use secondary keymap"), LFUN_KMAP_SEC },
		{ _("Toggle keymap"), LFUN_KMAP_TOGGLE },
		{ _("Insert Label"), LFUN_INSERT_LABEL },
		{ _("Copy paragraph environment type"),	LFUN_LAYOUT_COPY },
		{ _("Paste paragraph environment type"), LFUN_LAYOUT_PASTE },
		{ _("Specify paper size and margins"), LFUN_LAYOUT_PAPER },
		{ _("Go to beginning of line"), LFUN_HOME },
		{ _("Select to beginning of line"), LFUN_HOMESEL },
		{ _("Go to end of line"), LFUN_END },
		{ _("Select to end of line"), LFUN_ENDSEL },
		{ _("Exit"), LFUN_QUIT },
		{ _("Insert Margin note"), LFUN_MARGINMELT },
		{ _("Math Greek"), LFUN_GREEK },   
		{ _("Math mode"), LFUN_MATH_MODE },
		{ _("Go one paragraph down"), LFUN_DOWN_PARAGRAPH },
		{ _("Select next paragraph"), LFUN_DOWN_PARAGRAPHSEL },
		{ _("Go one paragraph up"), LFUN_UP_PARAGRAPH },
		{ _("Select previous paragraph"), LFUN_UP_PARAGRAPHSEL },
		{ _("Paste"), LFUN_PASTE },
		{ _("Insert protected space"), LFUN_PROTECTEDSPACE },
		{ _("Insert quote"), LFUN_QUOTE },
		{ _("Reconfigure"), LFUN_RECONFIGURE },
		{ _("Redo"), LFUN_REDO },
		{ _("Insert cross reference"), LFUN_INSERT_REF },
		{ _("Insert Table"), LFUN_TABLE },
		{ _("Toggle TeX style"), LFUN_TEX },
		{ _("Undo"), LFUN_UNDO },
		{ _("Melt"), LFUN_MELT },
		{ _("Import document"), LFUN_IMPORT },
		{ _("Remove all error boxes"), LFUN_REMOVEERRORS },
		{ _("Insert menu separator"), LFUN_MENU_SEPARATOR }
//	        { "reference-back", LFUN_REFBACK },
//	        { "reference-goto", LFUN_REFGOTO }, 
	};
        
        // Sort to make it faster
        if (!is_sorted) {
		int i= 0; 
	        while (i < LFUN_LASTACTION-1) {
		    if (helpTexts[i].action == 0) {
			helpTexts[i].action = (kb_action)i;
			helpTexts[i].name = getActionName(i);
			i++;
		    } else if (helpTexts[i].action != i) {
			int k = helpTexts[i].action;
		        kb_func_table tmp = helpTexts[k];
			helpTexts[k] = helpTexts[i];
			helpTexts[i] = tmp;
			if (k < i) i++;
		    } else {
			i++;
		    }
		}
	        is_sorted = true;
	}

	if (action <= 1 || action >= funcCount)
		goto no_desc;
		
        if (helpTexts[action].action == action) {
	    return helpTexts[action].name;
	} 
	//	  + string(' ') + int(is_ro) + string(']'));
  no_desc:
	// In an ideal world, this never happens:
	return _("No description available!");
}


// Function to compare items from the attrib table.
extern "C" int actioncomp(const void *a, const void *b)
{
    int const *ia= (int const*)a, *ib= (int const*)b;
    return (*ia)-(*ib);
}

 
bool LyXAction::isFuncRO(kb_action action) const
{
    static bool is_sorted = false;    
    static int fCount = sizeof(::func_attrib_table) / sizeof(kb_action);
    
    if (!is_sorted) {
	qsort(func_attrib_table, fCount, sizeof(kb_action), actioncomp);
	is_sorted = true;
//	for (int i= 0; i<fCount; i++) 
//	  fprintf(stdout, "%d: %d\n", i, func_attrib_table[i]);
    }

    int m, k, l= 0, r = fCount;
    int is_ro = false;

    while (l < r) {
	m = (l+r)/2;
	k = func_attrib_table[m] - action;
	if (k == 0) {
	    is_ro = true;
	    break;
	} else
	  if (k<0) l = m+1; else r = m;
    }
    lyxerr.debug() << "RO[" << action
		   << " " << is_ro << "]" << endl;
    return is_ro;
}

ostream & operator<<(ostream & o, kb_action action)
{
	return o << int(action);
}
