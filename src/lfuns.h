// -*- C++ -*-
/**
 * \file lfuns.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 *
 * To add a new function:
 * - add a new enum constant immediately before LFUN_LASTACTION
 * - add an appropriate line in LyXAction.cpp
 * - add a branch to a suitable ::doDispatch() method
 * - add correct test to the corresponding ::getStatus() method
 */

#ifndef LFUNS_H
#define LFUNS_H

#include <iosfwd>


namespace lyx {

/** These are all the lyx functions, the main mechanism
 * through which the frontends communicate with the core.
 *
 * They are managed in LyXAction.cpp and handled in various
 * ::dispatch() functions, starting with LyXFunc.C:dispatch(),
 * BufferView::dispatch(), Cursor::dispatch() and
 * Inset*::doDispatch();
 */

/** LFUN documentation (a start at least, Chr 2007-08-12)
 *
 * The documentation below is primarily notes about restrictions and
 * oddities relating to the different LFUNs. Note that this
 * documentation may well be moved to a more suitable location later
 * on.
 *
 * The documentation is interspersed with the enum:s. Another choice
 * was to put it in a separate list. The best choice was unclear, so
 * this may change. Particularly if doxygen can be used somehow.
 */

enum kb_action {
	LFUN_UNKNOWN_ACTION = -1,
	// 0
	LFUN_NOACTION = 0,
	LFUN_COMMAND_PREFIX,
	LFUN_SCREEN_RECENTER,
	LFUN_BUFFER_NEW,
	LFUN_BUFFER_WRITE,
	// 5
	LFUN_BUFFER_WRITE_AS,
/**	LFUN_BUFFER_WRITE_AS,		
 * Function:	Rename and save current buffer.
 * Syntax:	buffer-write-as <filename>
 * In:		<filename> = New name of the buffer/file. A relative path
 *		is with respect to the original location of the buffer/file.
 */
	LFUN_BUILD_PROGRAM,
	LFUN_BUFFER_VIEW,
	LFUN_BUFFER_CLOSE,
	LFUN_LYX_QUIT,
	// 10
	LFUN_BUFFER_AUTO_SAVE,
	LFUN_UNDO,
	LFUN_REDO,
	LFUN_PASTE,
	LFUN_PRIMARY_SELECTION_PASTE,
	// 15
	LFUN_CUT,
	LFUN_COPY,
	LFUN_ERROR_NEXT,
	LFUN_NOTE_NEXT,
	LFUN_INSET_TOGGLE,
	// 20
	LFUN_HYPHENATION_POINT_INSERT,
	LFUN_LIGATURE_BREAK_INSERT,
	LFUN_HFILL_INSERT,
	LFUN_FONT_FREE_APPLY,
	LFUN_FONT_FREE_UPDATE,
	// 25
	LFUN_FONT_EMPH,
	LFUN_FONT_BOLD,
	LFUN_FONT_ROMAN,
	LFUN_FONT_NOUN,
	LFUN_FONT_ITAL,
	// 30
	LFUN_FONT_FRAK,
	LFUN_CHAR_FORWARD,
	LFUN_CHAR_BACKWARD,
	LFUN_UP,
	LFUN_DOWN,
	// 35
	LFUN_SCREEN_UP,
	LFUN_SCREEN_DOWN,
	LFUN_LINE_BEGIN,
	LFUN_LINE_END,
	LFUN_CELL_FORWARD,
	// 40
	LFUN_CELL_BACKWARD,
	LFUN_WORD_FORWARD,
	LFUN_WORD_BACKWARD,
	LFUN_BUFFER_BEGIN,
	LFUN_BUFFER_END,
	// 45
	LFUN_CHAR_FORWARD_SELECT,
	LFUN_CHAR_BACKWARD_SELECT,
	LFUN_UP_SELECT,
	LFUN_DOWN_SELECT,
	LFUN_SCREEN_UP_SELECT,
	// 50
	LFUN_SCREEN_DOWN_SELECT,
	LFUN_LINE_BEGIN_SELECT,
	LFUN_LINE_END_SELECT,
	LFUN_WORD_FORWARD_SELECT,
	LFUN_WORD_BACKWARD_SELECT,
	// 55
	LFUN_WORD_SELECT,
	LFUN_BUFFER_BEGIN_SELECT,
	LFUN_BUFFER_END_SELECT,
	LFUN_SPACE_INSERT,               // JSpitzm 20030520
	LFUN_MARK_TOGGLE,
	// 60
	LFUN_CHAR_DELETE_FORWARD,
	LFUN_CHAR_DELETE_BACKWARD,
	LFUN_BREAK_LINE,
	LFUN_BREAK_PARAGRAPH,
	LFUN_QUOTE_INSERT,
	// 65
	LFUN_ACCENT_CIRCUMFLEX,
	LFUN_MATH_SUBSCRIPT,
	LFUN_MATH_SUPERSCRIPT,
	LFUN_ACCENT_GRAVE,
	LFUN_ACCENT_ACUTE,
	// 70
	LFUN_ACCENT_TILDE,
	LFUN_ACCENT_CEDILLA,
	LFUN_ACCENT_MACRON,
	LFUN_ACCENT_UNDERBAR,
	LFUN_ACCENT_UNDERDOT,
	// 75
	LFUN_ACCENT_CIRCLE,
	LFUN_ACCENT_TIE,
	LFUN_ACCENT_BREVE,
	LFUN_ACCENT_CARON,
	LFUN_ACCENT_SPECIAL_CARON,
	// 80
	LFUN_ACCENT_HUNGARIAN_UMLAUT,
	LFUN_ACCENT_UMLAUT,
	LFUN_ACCENT_DOT,
	LFUN_ACCENT_OGONEK,
	LFUN_SELF_INSERT,
	// 85
	LFUN_GETBUFNAME,
	LFUN_SERVER_GET_XY,
	LFUN_SERVER_SET_XY,
	LFUN_SERVER_CHAR_AFTER,
	LFUN_LINEATCURSOR,
	// 90
	LFUN_SERVER_GET_LAYOUT,
	LFUN_SERVER_GET_FONT,
	LFUN_SERVER_GET_NAME,
	LFUN_SERVER_NOTIFY,
	LFUN_SERVER_GOTO_FILE_ROW,
	// 95
	LFUN_NOTE_INSERT,
	LFUN_ENVIRONMENT_INSERT,        // unused as of 20060905
	LFUN_KEYMAP_OFF,
	LFUN_KEYMAP_PRIMARY,
	LFUN_KEYMAP_SECONDARY,
	// 100
	LFUN_KEYMAP_TOGGLE,
	LFUN_MATH_INSERT,
	LFUN_MATH_MATRIX,
	LFUN_MATH_LIMITS,
	LFUN_MATH_DELIM,                // Alejandro 180696
	// 105
	LFUN_MATH_DISPLAY,              // Alejandro 180696
	LFUN_MATH_MODE,                 // Alejandro 040696
	LFUN_MATH_NUMBER,               // Alejandro 040696
	LFUN_MATH_NONUMBER,             // Alejandro 180696
	LFUN_MATH_SIZE,                 // Alejandro 150896
	// 110
	LFUN_MATH_MACRO,                // ale970510
	LFUN_MATH_EXTERN,               // Andre' 20010424
	LFUN_MATH_MUTATE,               // Andre' 20010523
	LFUN_MATH_IMPORT_SELECTION,     // Andre' 20010704
	LFUN_MATH_SPACE,                // Andre' 20010725
	// 115
	LFUN_WORD_DELETE_FORWARD,
	LFUN_WORD_DELETE_BACKWARD,
	LFUN_LINE_DELETE,
	LFUN_MARK_OFF,
	LFUN_MARK_ON,
	// 120
	LFUN_LAYOUT,
	LFUN_LAYOUT_PARAGRAPH,
	LFUN_DROP_LAYOUTS_CHOICE,       // used in bindings as of 20060905
	LFUN_FONT_TYPEWRITER,           // changed from FONT_CODE 20070920
	LFUN_FONT_SANS,
	// 125
	LFUN_FONT_DEFAULT,
	LFUN_FONT_UNDERLINE,
	LFUN_FONT_SIZE,
	LFUN_FONT_STATE,
	LFUN_WORD_UPCASE,
	// 130
	LFUN_WORD_LOWCASE,
	LFUN_WORD_CAPITALIZE,
	LFUN_LABEL_INSERT,
	LFUN_DEPTH_DECREMENT,
	LFUN_DEPTH_INCREMENT,
	// 135
	LFUN_MENU_OPEN,                 // used in bindings as of 20060905
	LFUN_CANCEL,
	LFUN_META_PREFIX,
	LFUN_COMMAND_EXECUTE,
	LFUN_FILE_INSERT,
	// 140
	LFUN_FILE_INSERT_PLAINTEXT,     // CFO-G 1997-11-19
	LFUN_FILE_INSERT_PLAINTEXT_PARA,// Levon 2001-02-14
	LFUN_FILE_NEW,                  // for scripting purposes
	LFUN_FILE_OPEN,
	LFUN_PARAGRAPH_UP,              // Asger 1996-10-01
	// 145
	LFUN_PARAGRAPH_UP_SELECT,       // Asger 1996-10-01
	LFUN_PARAGRAPH_DOWN,            // Asger 1996-10-01
	LFUN_PARAGRAPH_DOWN_SELECT,     // Asger 1996-10-01
	LFUN_BREAK_PARAGRAPH_SKIP,
	LFUN_DELETE_BACKWARD_SKIP,
	// 150
	LFUN_DELETE_FORWARD_SKIP,
	LFUN_BUFFER_NEW_TEMPLATE,       // Asger 1997-02-02
	LFUN_BUFFER_RELOAD,             // Asger 1997-02-02
	LFUN_RECONFIGURE,               // Asger 1997-02-14
	LFUN_INDEX_PRINT,               // Lgb 97-02-27
	// 155
	LFUN_CELL_SPLIT,
	LFUN_BUFFER_CHILD_OPEN,         // Ale 970528
	LFUN_TOC_INSERT,                // Lgb 97-05-27
	LFUN_FLOAT_LIST,                // Lgb 20010503
	LFUN_BUFFER_TOGGLE_READ_ONLY,   // Lgb 97-05-27
	// 160
	LFUN_VC_REGISTER,               // Lgb 97-07-01
	LFUN_VC_CHECK_IN,               // Lgb 97-07-01
	LFUN_VC_CHECK_OUT,              // Lgb 97-07-01
	LFUN_VC_REVERT,                 // Lgb 97-07-01
	LFUN_VC_UNDO_LAST,              // Lgb 97-07-01
	// 165
	LFUN_BUFFER_EXPORT,             // Lgb 97-07-29
	LFUN_LABEL_GOTO,                // Ale 970806
	LFUN_DOTS_INSERT,               // Asger 970929
	LFUN_END_OF_SENTENCE_PERIOD_INSERT,   // Asger 970929
	LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR, // ARRae 971202
	// 170
	LFUN_BUFFER_CHKTEX,             // Asger 971030
	LFUN_HTML_INSERT,               // CFO-G 971121; must co-exist with URL_INSERT as of 20060905
	LFUN_URL_INSERT,                // CFO-G 971121
	LFUN_WORD_FIND_FORWARD,         // Etienne 980216
	LFUN_WORD_FIND_BACKWARD,        // Etienne 980220
	// 175
	LFUN_APPENDIX,                  // ettrich 980505
	LFUN_BUFFER_IMPORT,             // Asger 980724
	LFUN_MENU_SEPARATOR_INSERT,     // Asger 990220
	LFUN_COMMAND_SEQUENCE,          // Andre' 991111
	LFUN_PREFERENCES_SAVE,          // Lgb 991127
	// 180
	LFUN_HELP_OPEN,                 // Jug 990627
	LFUN_DATE_INSERT,	              // jdblair 20000131
	LFUN_LANGUAGE,                  // Dekel 20000203
	LFUN_ERT_INSERT,	              // Jug 20000218
	LFUN_FOOTNOTE_INSERT,           // Jug 20000307
	// 185
	LFUN_PARAGRAPH_SPACING,         // Lgb 20000411
	LFUN_TABULAR_INSERT,            // Jug 20000412
	LFUN_LOFVIEW,                   // Dekel 20000519
	LFUN_LOTVIEW,                   // Dekel 20000519
	LFUN_LOAVIEW,                   // Dekel 20000519
	// 190
	LFUN_SET_COLOR,                 // SLior 20000611
	LFUN_MARGINALNOTE_INSERT,       // Lgb 20000626
	LFUN_FLOAT_INSERT,              // Lgb 20000627
	LFUN_FLOAT_WIDE_INSERT,         // Lgb 20010531
	LFUN_CAPTION_INSERT,            // Lgb 20000718; inactive as of 20060905
	// 195
	LFUN_BUFFER_SWITCH,
	LFUN_TABULAR_FEATURE,           // Jug 20000728
	LFUN_LAYOUT_TABULAR,            // Jug 20000731
	LFUN_BUFFER_UPDATE,             // Dekel 20000805
	LFUN_INDEX_INSERT,              // Angus 20000803
	// 200
	LFUN_SCREEN_FONT_UPDATE,        // ARRae 20000813
	LFUN_PARAGRAPH_GOTO,            // Dekel 20000826
	LFUN_REFERENCE_NEXT,            // Dekel 20010114
	LFUN_BOOKMARK_SAVE,             // Dekel 20010127
	LFUN_BOOKMARK_GOTO,             // Dekel 20010127
	// 205
	LFUN_SELECT_FILE_SYNC,          // Levon 20010214
	LFUN_MESSAGE,                   // Lgb 20010408; for scripting purposes, output in minibuffer
	LFUN_CHARS_TRANSPOSE,           // Lgb 20010425
	LFUN_ESCAPE,                    // Lgb 20010517
	LFUN_THESAURUS_ENTRY,           // Levon 20010720
	// 210
	LFUN_OPTIONAL_INSERT,           // Martin 12 Aug 2002
	LFUN_MOUSE_PRESS,               // André 9 Aug 2002
	LFUN_MOUSE_MOTION,              // André 9 Aug 2002
	LFUN_MOUSE_RELEASE,             // André 9 Aug 2002
	LFUN_MOUSE_DOUBLE,              // André 9 Aug 2002
	// 215
	LFUN_MOUSE_TRIPLE,              // André 9 Aug 2002
	LFUN_WRAP_INSERT,               // Dekel 7 Apr 2002
	LFUN_CHANGES_TRACK,             // Levon 20021001 (cool date !)
	LFUN_CHANGES_MERGE,             // Levon 20021016
	LFUN_CHANGE_ACCEPT,             // Levon 20021016
	// 220
	LFUN_CHANGE_REJECT,             // Levon 20021016
	LFUN_ALL_CHANGES_ACCEPT,        // Levon 20021016
	LFUN_ALL_CHANGES_REJECT,        // Levon 20021016
	LFUN_BIBITEM_INSERT,            // André 14 Feb 2003
	LFUN_DIALOG_SHOW,
	// 225
	LFUN_DIALOG_SHOW_NEW_INSET,
	LFUN_DIALOG_UPDATE,
	LFUN_DIALOG_HIDE,
	LFUN_DIALOG_TOGGLE,             // JSpitzm 20070430
	LFUN_DIALOG_DISCONNECT_INSET,
	// 230
	LFUN_INSET_APPLY,
	LFUN_INSET_INSERT,
	LFUN_INSET_MODIFY,
	LFUN_INSET_DIALOG_UPDATE,
	LFUN_INSET_SETTINGS,
	// 235
	LFUN_PARAGRAPH_PARAMS_APPLY,
	LFUN_PARAGRAPH_UPDATE,
	LFUN_EXTERNAL_EDIT,
	LFUN_BRANCH_INSERT,
	LFUN_BOX_INSERT,
	// 240
	LFUN_LINE_INSERT,
	LFUN_PAGEBREAK_INSERT,
	LFUN_REPEAT,
	LFUN_FINISHED_LEFT,
	LFUN_FINISHED_RIGHT,
	// 245
	LFUN_FLEX_INSERT,
	LFUN_WORD_FIND,
	LFUN_WORD_REPLACE,
	LFUN_BUFFER_EXPORT_CUSTOM,
	LFUN_BUFFER_PRINT,
	// 250
	LFUN_NEXT_INSET_TOGGLE,
	LFUN_ALL_INSETS_TOGGLE,
	LFUN_BUFFER_LANGUAGE,
	LFUN_TEXTCLASS_APPLY,
	LFUN_TEXTCLASS_LOAD,
	// 255
	LFUN_BUFFER_SAVE_AS_DEFAULT,
	LFUN_BUFFER_PARAMS_APPLY,
	LFUN_LYXRC_APPLY,
	LFUN_GRAPHICS_EDIT,
	LFUN_INSET_REFRESH,
	// 260
	LFUN_BUFFER_NEXT,
	LFUN_BUFFER_PREVIOUS,
	LFUN_WORDS_COUNT,
	LFUN_CHANGES_OUTPUT,             // jspitzm 20050121
	LFUN_BIBTEX_DATABASE_ADD,
	// 265
	LFUN_BIBTEX_DATABASE_DEL,
	LFUN_CITATION_INSERT,
	LFUN_OUTLINE_UP,                 // Vermeer 20060323
	LFUN_OUTLINE_DOWN,
	LFUN_OUTLINE_IN,
	// 270
	LFUN_OUTLINE_OUT,
	LFUN_PARAGRAPH_MOVE_DOWN,        // Edwin 20060408
	LFUN_PARAGRAPH_MOVE_UP,          // Edwin 20060408
	LFUN_BUFFER_TOGGLE_COMPRESSION,  // bpeng 20060427
	LFUN_MATH_BIGDELIM,
	// 275
	LFUN_CLIPBOARD_PASTE,
	LFUN_INSET_DISSOLVE,             // jspitzm 20060807
	LFUN_CHANGE_NEXT,
	LFUN_WINDOW_NEW,                 // Abdel 20061021
	LFUN_WINDOW_CLOSE,               // Abdel 20061023
	// 280
	LFUN_UNICODE_INSERT,             // Lgb 20061022
	LFUN_BOOKMARK_CLEAR,             // bpeng 20061031
	LFUN_NOMENCL_INSERT,             // Ugras
	LFUN_NOMENCL_PRINT,              // Ugras
	LFUN_CLEARPAGE_INSERT,           // Ugras 20061125
	// 285
	LFUN_CLEARDOUBLEPAGE_INSERT,     // Ugras 20061125
	LFUN_LISTING_INSERT,             // Herbert 20011110, bpeng 20070502
	LFUN_TOOLBAR_TOGGLE,             // Edwin 20070521
	LFUN_BUFFER_WRITE_ALL,           // rgh, gpothier 200707XX
	LFUN_PARAGRAPH_PARAMS,           // rgh, 200708XX
	// 290
	LFUN_LAYOUT_MODULES_CLEAR,       // rgh, 20070825
	LFUN_LAYOUT_MODULE_ADD,          // rgh, 20070825
	LFUN_LAYOUT_RELOAD,              // rgh, 20070903
	LFUN_MASTER_BUFFER_VIEW,         // Tommaso, 20070920
	LFUN_MASTER_BUFFER_UPDATE,       // Tommaso, 20070920
	LFUN_INFO_INSERT,                // bpeng, 20071007

	LFUN_LASTACTION                  // end of the table
};

std::ostream & operator<<(std::ostream &, kb_action);


} // namespace lyx

#endif // LFUNS_H
