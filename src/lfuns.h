// -*- C++ -*-
/**
 * \file lfuns.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 *
 * To add a new function:
 * - add a new enum constant and doc immediately before LFUN_LASTACTION
 * - add an appropriate line in LyXAction.cpp
 * - add a branch to a suitable ::doDispatch() method
 * - add correct test to the corresponding ::getStatus() method
 */

#ifndef LFUNS_H
#define LFUNS_H

#include "support/strfwd.h"


namespace lyx {

/** These are all the lyx functions, the main mechanism
 * through which the frontends communicate with the core.
 *
 * They are managed in LyXAction.cpp and handled in various
 * ::dispatch() functions, starting with LyXFunc.C:dispatch(),
 * BufferView::dispatch(), Cursor::dispatch() and
 * Inset*::doDispatch();
 */

/* LFUN documentation (a start at least, Chr 2007-08-12)
 *
 * The documentation below is primarily notes about restrictions and
 * oddities relating to the different LFUNs.
 *
 * Doxygen template below. Some notes: Parameters should be set in uppercase
 * and put in <BRACKETS>, as so.
 */

/**
 * 
 * \li Action: 
 * \li Notion: 
 * \li Syntax: 
 * \li Params: 
 * \li Origin: 
 */

enum kb_action {
	LFUN_UNKNOWN_ACTION = -1,
	// 0
	LFUN_NOACTION = 0,
	LFUN_COMMAND_PREFIX,
/**
 * LFUN_SCREEN_RECENTER
 * \li Action: Recenters the screen on the current cursor position
 * \li Syntax: screen-recenter
 */
	LFUN_SCREEN_RECENTER,
/**
 * LFUN_BUFFER_NEW
 * \li Action: Creates a new buffer (that is, document)
 * \li Syntax: buffer-new
 */
	LFUN_BUFFER_NEW,
/**
 * LFUN_BUFFER_WRITE
 * \li Action: Saves the current buffer
 * \li Notion: Saves the current buffer to disk, using the filename that
               is already associated with the buffer, asking for one if
               none is yet assigned.
 * \li Syntax: buffer-write
 */
	LFUN_BUFFER_WRITE,
	// 5
/**
 * LFUN_BUFFER_WRITE_AS
 * \li Action: Rename and save current buffer.
 * \li Syntax: buffer-write-as <FILENAME>
 * \li Params: <FILENAME>: New name of the buffer/file. A relative path
 *             is with respect to the original location of the buffer/file.
 */
	LFUN_BUFFER_WRITE_AS,
	LFUN_BUILD_PROGRAM,
/**
 * LFUN_BUFFER_VIEW
 * \li Action: Displays current buffer in chosen format
 * \li Notion: Displays the contents of the current buffer in the chosen
               format, for example, PDF or DVI. This runs the necessary
               converter, calls the defined viewer, and so forth.
 * \li Syntax: buffer-view <FORMAT>
 * \li Params: <FORMAT>: The format to display, where this is one of the
                         formats defined (in the current GUI) in the
                         Tools>Preferences>File Formats dialog.
 */
	LFUN_BUFFER_VIEW,
/**
 * LFUN_BUFFER_CLOSE
 * \li Action: Closes the current buffer
 * \li Notion: Closes the current buffer, asking whether to save it, etc,
               if the buffer has been modified.
 * \li Syntax: buffer-close
 */
	LFUN_BUFFER_CLOSE,
/**
 * LFUN_LYX_QUIT
 * \li Action: Terminates the current LyX instance
 * \li Notion: Terminates the current LyX instance, asking whether to save
               modified documents, etc.
 * \li Syntax: lyx-quit
 */
	LFUN_LYX_QUIT,
	// 10
/**
 * LFUN_BUFFER_AUTO_SAVE
 * \li Action: Saves the current buffer to a temporary file
 * \li Notion: Saves the current buffer to a file named "#filename#". This LFUN 
               is called automatically by LyX, to "autosave" the current buffer.
	* \li Syntax: buffer-auto-save
 */
	LFUN_BUFFER_AUTO_SAVE,
/**
 * LFUN_UNDO
 * \li Action: Undoes the last edit
 * \li Syntax: undo
 */
	LFUN_UNDO,
/**
 * LFUN_REDO
 * \li Action: Redoes the last thing undone
 * \li Syntax: redo
 */
	LFUN_REDO,
/**
 * LFUN_PASTE
 * \li Action: Pastes from the active clipboard
 * \li Syntax: paste
 */
	LFUN_PASTE,
/**
 * LFUN_PRIMARY_SELECTION_PASTE
 * \li Action: Pastes the material currently selected
 * \li Syntax: primary-selection-paste
 */
	LFUN_PRIMARY_SELECTION_PASTE,
	// 15
/**
 * LFUN_CUT
 * \li Action: Cuts to the clipboard
 * \li Syntax: cut
 */
	LFUN_CUT,
/**
 * LFUN_COPY
 * \li Action: Copies to the clipboard the last edit
 * \li Syntax: copy
 */
	LFUN_COPY,
	LFUN_ERROR_NEXT,
	LFUN_NOTE_NEXT,
	LFUN_INSET_TOGGLE,
	// 20
	LFUN_HFILL_INSERT,
	LFUN_FONT_FREE_APPLY,
	LFUN_FONT_FREE_UPDATE,
	LFUN_FONT_EMPH,
	LFUN_FONT_BOLD,
	// 25
	LFUN_FONT_ROMAN,
	LFUN_FONT_NOUN,
	LFUN_FONT_ITAL,
	LFUN_FONT_FRAK,
	LFUN_CHAR_FORWARD,
	// 30
	LFUN_CHAR_BACKWARD,
	LFUN_UP,
	LFUN_DOWN,
	LFUN_SCREEN_UP,
	LFUN_SCREEN_DOWN,
	// 35
	LFUN_LINE_BEGIN,
	LFUN_LINE_END,
	LFUN_CELL_FORWARD,
	LFUN_CELL_BACKWARD,
	LFUN_WORD_FORWARD,
	// 40
	LFUN_WORD_BACKWARD,
	LFUN_BUFFER_BEGIN,
	LFUN_BUFFER_END,
	LFUN_CHAR_FORWARD_SELECT,
	LFUN_CHAR_BACKWARD_SELECT,
	// 45
	LFUN_UP_SELECT,
	LFUN_DOWN_SELECT,
	LFUN_SCREEN_UP_SELECT,
	LFUN_SCREEN_DOWN_SELECT,
	LFUN_LINE_BEGIN_SELECT,
	// 50
	LFUN_LINE_END_SELECT,
	LFUN_WORD_FORWARD_SELECT,
	LFUN_WORD_BACKWARD_SELECT,
	LFUN_WORD_SELECT,
	LFUN_BUFFER_BEGIN_SELECT,
	// 55
	LFUN_BUFFER_END_SELECT,
	LFUN_SPACE_INSERT,              // JSpitzm 20030520
	LFUN_SPECIALCHAR_INSERT,        // JSpitzm, 20071206
/**
 * LFUN_MARK_TOGGLE
 * \li Action: Toggle between MARK_ON and MARK_OFF
 * \li Syntax: mark-toggle
 * \li Origin: poenitz, May 5 2006
 */
	LFUN_MARK_TOGGLE,
	LFUN_CHAR_DELETE_FORWARD,
	// 60
	LFUN_CHAR_DELETE_BACKWARD,
	LFUN_NEW_LINE,
	LFUN_LINE_BREAK,                // uwestoehr 20071125
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
/**
 * LFUN_MATH_NUMBER
 * \li Action: Enable numbering of the current formula.
 * \li Notion: Must be in display formula mode.
 * \li Syntax: math-number
 * \li Origin: Alejandro 4 Jun 1996
 */
	LFUN_MATH_NUMBER,
/**
 * LFUN_MATH_NONUMBER
 * \li Action: Disable numbering of the current formula.
 * \li Notion: Must be in display formula mode.
 * \li Syntax: math-nonumber
 * \li Origin: Alejandro 18 Jun 1996
 */
	LFUN_MATH_NONUMBER,
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
/**
 * LFUN_MARK_OFF
 * \li Action: Disable selecting of text-region.
 * \li Syntax: mark-off
 */
	LFUN_MARK_OFF,
/**
 * LFUN_MARK_ON
 * \li Action: Enable selecting of text-region.
 * \li Notion: After enabling you can simply move arrow keys to get selected region.
 * \li Syntax: mark-on
 */
	LFUN_MARK_ON,
	// 120
 /**
 * LFUN_LAYOUT
 * \li Action: Sets the layout (that is, environment) for the current paragraph
 * \li Syntax: layout <LAYOUT>
 * \li Params: <LAYOUT>: the layout to use
 */
	LFUN_LAYOUT,
/**
 * LFUN_LAYOUT_PARAGRAPH
 * \li Action: Launches the paragraph settings dialog
 * \li Syntax: layout-paragraph
 */
	LFUN_LAYOUT_PARAGRAPH,
/**
 * LFUN_DROP_LAYOUTS_CHOICE
 * \li Action: Displays list of layout choices
 * \li Notion: In the current (as of 2007) Qt4 frontend, this LFUN opens the
               dropbox allowing for choice of layout
 * \li Syntax: drop-layouts-choice
 */
	LFUN_DROP_LAYOUTS_CHOICE,       // used in bindings as of 20071228
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
/**
 * LFUN_RECONFIGURE
 * \li Action: Reconfigure the automatic settings.
 * \li Syntax: reconfigure
 * \li Origin: Asger , 14 Feb 1997
 */
	LFUN_RECONFIGURE,
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
	LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR, // ARRae 971202
	LFUN_BUFFER_CHKTEX,             // Asger 971030
	LFUN_HYPERLINK_INSERT,          // CFO-G 971121
	// 170
	LFUN_WORD_FIND_FORWARD,         // Etienne 980216
	LFUN_WORD_FIND_BACKWARD,        // Etienne 980220
	LFUN_APPENDIX,                  // ettrich 980505
	LFUN_BUFFER_IMPORT,             // Asger 980724
	LFUN_COMMAND_SEQUENCE,          // Andre' 991111
	// 175
/**
 * LFUN_PREFERENCES_SAVE
 * \li Action: Save user preferences.
 * \li Syntax: preferences-save
 * \li Origin: Lgb 27 Nov 1999
 */
	LFUN_PREFERENCES_SAVE,
	LFUN_HELP_OPEN,                 // Jug 990627
	LFUN_DATE_INSERT,               // jdblair 20000131
	LFUN_LANGUAGE,                  // Dekel 20000203
	LFUN_ERT_INSERT,                // Jug 20000218
	// 180
	LFUN_FOOTNOTE_INSERT,           // Jug 20000307
	LFUN_PARAGRAPH_SPACING,         // Lgb 20000411
	LFUN_TABULAR_INSERT,            // Jug 20000412
	LFUN_LOFVIEW,                   // Dekel 20000519
	LFUN_LOTVIEW,                   // Dekel 20000519
	// 185
	LFUN_LOAVIEW,                   // Dekel 20000519
	LFUN_SET_COLOR,                 // SLior 20000611
	LFUN_MARGINALNOTE_INSERT,       // Lgb 20000626
	LFUN_FLOAT_INSERT,              // Lgb 20000627
	LFUN_FLOAT_WIDE_INSERT,         // Lgb 20010531
	// 190
	LFUN_CAPTION_INSERT,            // Lgb 20000718; inactive as of 20060905
	LFUN_BUFFER_SWITCH,
	LFUN_TABULAR_FEATURE,           // Jug 20000728
	LFUN_LAYOUT_TABULAR,            // Jug 20000731
	LFUN_BUFFER_UPDATE,             // Dekel 20000805
	// 195
	LFUN_INDEX_INSERT,              // Angus 20000803
	LFUN_SCREEN_FONT_UPDATE,        // ARRae 20000813
	LFUN_PARAGRAPH_GOTO,            // Dekel 20000826
	LFUN_REFERENCE_NEXT,            // Dekel 20010114
/**
 * LFUN_BOOKMARK_SAVE
 * \li Action: Save a bookmark
 * \li Notion: Saves a numbered bookmark to the sessions file. The number
               must be between 1 and 9, inclusive. Note that bookmarks are
               saved per-session, not per file.
 * \li Syntax: bookmark-save <NUMBER>
 * \li Params: <NUMBER>: the number of the bookmark to save.
 * \li Origin: Dekel, 27 January 2001
 */
	LFUN_BOOKMARK_SAVE,
	// 200
/**
 * LFUN_BOOKMARK_GOTO
 * \li Action: Goto a bookmark
 * \li Notion: Moves the cursor to the numbered bookmark, opening the file
               if necessary. Note that bookmarsk are saved per-session, not
               per file.
 * \li Syntax: bookmark-goto <NUMBER>
 * \li Params: <NUMBER>: the number of the bookmark to restore.
 * \li Origin: Dekel, 27 January 2001
 */
	LFUN_BOOKMARK_GOTO,
	LFUN_SELECT_FILE_SYNC,          // Levon 20010214
	LFUN_MESSAGE,                   // Lgb 20010408; for scripting purposes, output in minibuffer
	LFUN_CHARS_TRANSPOSE,           // Lgb 20010425
	LFUN_ESCAPE,                    // Lgb 20010517
	// 205
	LFUN_THESAURUS_ENTRY,           // Levon 20010720
	LFUN_OPTIONAL_INSERT,           // Martin 12 Aug 2002
	LFUN_MOUSE_PRESS,               // André 9 Aug 2002
	LFUN_MOUSE_MOTION,              // André 9 Aug 2002
	LFUN_MOUSE_RELEASE,             // André 9 Aug 2002
	// 210
	LFUN_MOUSE_DOUBLE,              // André 9 Aug 2002
	LFUN_MOUSE_TRIPLE,              // André 9 Aug 2002
	LFUN_WRAP_INSERT,               // Dekel 7 Apr 2002
	LFUN_CHANGES_TRACK,             // Levon 20021001 (cool date !)
	LFUN_CHANGES_MERGE,             // Levon 20021016
	// 215
	LFUN_CHANGE_ACCEPT,             // Levon 20021016
	LFUN_CHANGE_REJECT,             // Levon 20021016
	LFUN_ALL_CHANGES_ACCEPT,        // Levon 20021016
	LFUN_ALL_CHANGES_REJECT,        // Levon 20021016
	LFUN_BIBITEM_INSERT,            // André 14 Feb 2003
	// 220
	LFUN_DIALOG_SHOW,
	LFUN_DIALOG_SHOW_NEW_INSET,
	LFUN_DIALOG_UPDATE,
	LFUN_DIALOG_HIDE,
	LFUN_DIALOG_TOGGLE,             // JSpitzm 20070430
	// 225
	LFUN_DIALOG_DISCONNECT_INSET,
	LFUN_INSET_APPLY,
	LFUN_INSET_INSERT,
	LFUN_INSET_MODIFY,
	LFUN_INSET_DIALOG_UPDATE,
	// 230
	LFUN_INSET_SETTINGS,
/**
 * LFUN_PARAGRAPH_PARAMS_APPLY
 * \li Action: Change paragraph settings.
 * \li Notion: Overwrite all nonspecified settings to the default ones. 
               Use paragraph-params lfun if you don't want to overwrite others settings.
 * \li Syntax: paragraph-params-apply <INDENT> <SPACING> <ALIGN> <OTHERS>
 * \li Params: For parameters see LFUN_PARAGRAPH_PARAMS
 * \li Origin: leeming, 30 Mar 2004
 */
	LFUN_PARAGRAPH_PARAMS_APPLY,
	LFUN_PARAGRAPH_UPDATE,
	LFUN_EXTERNAL_EDIT,
	LFUN_BRANCH_INSERT,
	// 235
	LFUN_BOX_INSERT,
	LFUN_LINE_INSERT,
	LFUN_NEWPAGE_INSERT,            // uwestoehr 20071124
	LFUN_PAGEBREAK_INSERT,
	LFUN_REPEAT,
	// 240
	LFUN_FINISHED_LEFT,
	LFUN_FINISHED_RIGHT,
	LFUN_FLEX_INSERT,
	LFUN_WORD_FIND,
	LFUN_WORD_REPLACE,
	// 245
	LFUN_BUFFER_EXPORT_CUSTOM,
	LFUN_BUFFER_PRINT,
	LFUN_NEXT_INSET_TOGGLE,
	LFUN_ALL_INSETS_TOGGLE,
	LFUN_BUFFER_LANGUAGE,
	// 250
/**
 * LFUN_TEXTCLASS_APPLY
 * \li Action: Sets the text class for the current buffer
 * \li Syntax: textclass-apply <TEXTCLASS>
 * \li Params: <TEXTCLASS>: the textclass to set. Note that this must be
                   the filename, minus the ".layout" extension.
 */
	LFUN_TEXTCLASS_APPLY,
/**
 * LFUN_TEXTCLASS_LOAD
 * \li Action: Loads information for a textclass from disk
 * \li Syntax: textclass-load <TEXTCLASS>
 * \li Params: <TEXTCLASS>: the textclass to load. Note that this must be
                   the filename, minus the ".layout" extension.
 */
	LFUN_TEXTCLASS_LOAD,
	LFUN_BUFFER_SAVE_AS_DEFAULT,
	LFUN_BUFFER_PARAMS_APPLY,
	LFUN_LYXRC_APPLY,
	// 255
	LFUN_GRAPHICS_EDIT,
	LFUN_INSET_REFRESH,
	LFUN_BUFFER_NEXT,
	LFUN_BUFFER_PREVIOUS,
/**
 * LFUN_STATISTICS
 * \li Action: Count the statistics (number of words and characters)
               in the document or in the given selection.
 * \li Notion: Note that this function gives the number of words/chars written,
               not the number of characters which will be typeset.
 * \li Syntax: statistics
 * \li Origin: lasgouttes, Jan 27 2004; ps, Jan 8 2008
 */
	LFUN_STATISTICS,
	// 260
	LFUN_CHANGES_OUTPUT,             // jspitzm 20050121
	LFUN_BIBTEX_DATABASE_ADD,
	LFUN_BIBTEX_DATABASE_DEL,
	LFUN_CITATION_INSERT,
/**
 * LFUN_OUTLINE_UP
 * \li Action: Move the current group in the upward direction in the
               structure of the document.
 * \li Notion: The "group" can be Part/Chapter/Section/etc. It moves
               the whole substructure of the group.
 * \li Syntax: outline-up
 * \li Origin: Vermeer, 23 Mar 2006
 */
	LFUN_OUTLINE_UP,
	// 265
/**
 * LFUN_OUTLINE_DOWN
 * \li Action: Move the current group in the downward direction in the
               structure of the document.
 * \li Notion: The "group" can be Part/Chapter/Section/etc. It moves
               the whole substructure of the group.
 * \li Syntax: outline-down
 * \li Origin: Vermeer, 23 Mar 2006
 */
	LFUN_OUTLINE_DOWN,
/**
 * LFUN_OUTLINE_IN
 * \li Action: Moves the current group in the downward direction in the
               hierarchy of the document structure.
 * \li Notion: Part -> Chapter -> Section -> etc.
 * \li Syntax: outline-in
 * \li Origin: Vermeer, 23 Mar 2006
 */
	LFUN_OUTLINE_IN,
/**
 * LFUN_OUTLINE_OUT
 * \li Action: Moves the current group in the upward direction in the
               hierarchy of the document structure.
 * \li Notion: Part <- Chapter <- Section <- etc.
 * \li Syntax: outline-out
 * \li Origin: Vermeer, 23 Mar 2006
 */
	LFUN_OUTLINE_OUT,
/**
 * LFUN_PARAGRAPH_MOVE_DOWN
 * \li Action: Moves the current paragraph downwards in the document.
 * \li Syntax: paragraph-move-down
 * \li Origin: Edwin, 8 Apr 2006
 */
	LFUN_PARAGRAPH_MOVE_DOWN,
/**
 * LFUN_PARAGRAPH_MOVE_UP
 * \li Action: Moves the current paragraph upwards in the document.
 * \li Syntax: paragraph-move-up
 * \li Origin: Edwin, 8 Apr 2006
 */
	LFUN_PARAGRAPH_MOVE_UP,
	// 270
	LFUN_BUFFER_TOGGLE_COMPRESSION,  // bpeng 20060427
	LFUN_MATH_BIGDELIM,
	LFUN_CLIPBOARD_PASTE,
	LFUN_INSET_DISSOLVE,             // jspitzm 20060807
	LFUN_CHANGE_NEXT,
	// 275
	LFUN_WINDOW_NEW,                 // Abdel 20061021
	LFUN_WINDOW_CLOSE,               // Abdel 20061023
	LFUN_UNICODE_INSERT,             // Lgb 20061022
/**
 * LFUN_BOOKMARK_CLEAR
 * \li Action: Clears the list of saved bookmarks
 * \li Syntax: bookmark-clear
 * \li Origin: bpeng, 31 October 2006
 */
	LFUN_BOOKMARK_CLEAR,             // bpeng 20061031
	LFUN_NOMENCL_INSERT,             // Ugras
	// 280
	LFUN_NOMENCL_PRINT,              // Ugras
	LFUN_CLEARPAGE_INSERT,           // Ugras 20061125
	LFUN_CLEARDOUBLEPAGE_INSERT,     // Ugras 20061125
	LFUN_LISTING_INSERT,             // Herbert 20011110, bpeng 20070502
	LFUN_TOOLBAR_TOGGLE,             // Edwin 20070521
	// 285
	LFUN_BUFFER_WRITE_ALL,           // rgh, gpothier 200707XX
/**
 * LFUN_PARAGRAPH_PARAMS
 * \li Action: Change paragraph settings
 * \li Notion: Modifies the current paragraph, or currently selected paragraphs.
               This function only modifies, and does not override, existing settings.
               Note that the "leftindent" indent setting is deprecated.
 * \li Syntax: paragraph-params [<INDENT>] [<SPACING>] [<ALIGN>] [<OTHERS>]
 * \li Params: <INDENT>:  \\noindent|\\indent|\\indent-toggle|\\leftindent LENGTH\n
               <SPACING>: \\paragraph_spacing default|single|onehalf|double|other\n
               <ALIGN>:   \\align block|left|right|center|default\n
               <OTHERS>:  \\labelwidthstring WIDTH|\\start_of_appendix\n
 * \li Origin: rgh, Aug 15 2007
 */
	LFUN_PARAGRAPH_PARAMS,
/**
 * LFUN_LAYOUT_MODULES_CLEAR
 * \li Action: Clears the module list
 * \li Notion: Clears the list of included modules for the current buffer. 
 * \li Syntax: layout-modules-clear
 * \li Origin: rgh, 25 August 2007
 */
	LFUN_LAYOUT_MODULES_CLEAR,
/**
 * LFUN_LAYOUT_MODULE_ADD
 * \li Action: Adds a module
 * \li Notion: Adds a module to the list of included modules for the current buffer.
 * \li Syntax: layout-module-add <MODULE>
 * \li Params: <MODULE>: the module to be added
 * \li Origin: rgh, 25 August 2007
 */
	LFUN_LAYOUT_MODULE_ADD,
/**
 * LFUN_LAYOUT_RELOAD
 * \li Action: Reloads layout information
 * \li Notion: Reloads all layout information for the current buffer from disk, thus
               recognizing any changes that have been made to layout files on the fly. 
               This is intended to be used only by layout developers and should not be
               used when one is trying to do actual work.
 * \li Syntax: layout-reload
 * \li Origin: rgh, 3 September 2007
 */
	LFUN_LAYOUT_RELOAD,
	// 290
	LFUN_MASTER_BUFFER_VIEW,         // Tommaso, 20070920
	LFUN_MASTER_BUFFER_UPDATE,       // Tommaso, 20070920
	LFUN_INFO_INSERT,                // bpeng, 20071007
	LFUN_CALL,                       // broider, 20071002
	LFUN_BUFFER_TOGGLE_EMBEDDING,    // bpeng, 20071021
	// 295
	LFUN_CHAR_LEFT,			 // dov, 20071022
	LFUN_CHAR_LEFT_SELECT,		 // dov, 20071022
	LFUN_CHAR_RIGHT,		 // dov, 20071022
	LFUN_CHAR_RIGHT_SELECT,		 // dov, 20071022
	LFUN_FINISHED_BACKWARD,		 // dov, 20071022
	// 300
	LFUN_FINISHED_FORWARD,		 // dov, 20071022
	LFUN_WORD_LEFT,			 // dov, 20071028
	LFUN_WORD_LEFT_SELECT,		 // dov, 20071028
	LFUN_WORD_RIGHT,		 // dov, 20071028
	LFUN_WORD_RIGHT_SELECT,		 // dov, 20071028
	// 305
/**
 * LFUN_MATH_MACRO_FOLD
 * \li Action: Fold a Math Macro
 * \li Notion: Fold the Math Macro the cursor is in if it was 
               unfolded, i.e. displayed as \foo before. 
 * \li Syntax: math-macro-fold
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_FOLD,
/**
 * LFUN_MATH_MACRO_UNFOLD
 * \li Action: Unfold a Math Macro
 * \li Notion: Unfold the Math Macro the cursor is in, i.e. 
               display it as \foo. 
 * \li Syntax: math-macro-unfold
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_UNFOLD,
/**
 * LFUN_MATH_MACRO_ADD_PARAM
 * \li Action: Add a parameter
 * \li Notion: Add a parameter to a Math Macro
 * \li Params: <NUM>: The number of the parameter behind which the new one
               will be added (1 for the first, i.e. use 0 for add a 
               parameter at the left), defaults to the last one.
 * \li Syntax: math-macro-add-param <NUM>
 * \li Origin: sts, 06 January 2008
 */	
	LFUN_MATH_MACRO_ADD_PARAM,
/**
 * LFUN_MATH_MACRO_REMOVE_PARAM
 * \li Action: Remove the last parameter
 * \li Notion: Remove the last parameter of a Math Macro and
               remove its value in all instances of the macro 
               in the buffer.
 * \li Params: <NUM>: The number of the parameter to be deleted (1 for 
               the first), defaults to the last one.
 * \li Syntax: math-macro-remove-param <NUM>
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_REMOVE_PARAM,
/**
 * LFUN_MATH_MACRO_APPEND_GREEDY_PARAM
 * \li Action: Append a greedy parameter
 * \li Notion: Append a greedy parameter to a Math Macro which
               eats the following mathed cell in every instance of
               the macro in the buffer.
 * \li Syntax: math-macro-append-greedy-param
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_APPEND_GREEDY_PARAM,
	// 310
/**
 * LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM
 * \li Action: Remove a greedy parameter
 * \li Notion: Remove a greedy parameter of a Math Macro and spit
               out the values of it in every instance of the macro 
               in the buffer. If it is an optional parameter the [valud]
               format is used.
 * \li Syntax: math-macro-remove-greedy-param
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM,
/**
 * LFUN_MATH_MACRO_MAKE_OPTIONAL
 * \li Action: Make a parameter optional
 * \li Notion: Turn the first non-optional parameter of a Math Macro
               into an optional parameter with a default value.
 * \li Syntax: math-macro-make-optional
 * \li Origin: sts, 06 January 2008
 */	
	LFUN_MATH_MACRO_MAKE_OPTIONAL,
/**
 * LFUN_MATH_MACRO_MAKE_NONOPTIONAL
 * \li Action: Make a parameter non-optional
 * \li Notion: Turn the last optional parameter of a Math Macro
               into a non-optional parameter. The default value is
               remembered to be reused later if the user changes his mind.
 * \li Syntax: math-macro-make-nonoptional
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_MAKE_NONOPTIONAL,
/**
 * LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM
 * \li Action: Add an optional parameter
 * \li Notion: Insert an optional parameter just behind the 
               already existing optional parameters.
 * \li Syntax: math-macro-add-optional-param
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM,
/**
 * LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM
 * \li Action: Remove the last optional parameter
 * \li Notion: Remove the last optional parameter of a Math Macro and
               remove it in all the instances of the macro in the buffer.
 * \li Syntax: math-macro-remove-optional-param
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM,

	// 315
/**
 * LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM
 * \li Action: Add a greedy optional parameter
 * \li Notion: Add a greedy optional parameter which eats the value
               from the following cells in mathed which are in the [value]
               format.
 * \li Syntax: math-macro-add-greedy-optional-param
 * \li Origin: sts, 06 January 2008
 */
	LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM,
/**
 * LFUN_IN_MATHMACROTEMPLATE
 * \li Action: Only active in Math Macro definition
 * \li Notion: Dummy function which is only active in a Math Macro definition. 
               It's used to toggle the Math Macro toolbar if the cursor moves
               into a Math Macro definition.
 * \li Syntax: in-mathmacrotemplate
 * \li Origin: sts, 06 January 2008
 */
	LFUN_IN_MATHMACROTEMPLATE,
/** LFUN_SCROLL
 * \li Action: scroll the buffer view
 * \li Notion: Only scrolls the screen up or down; does not move the cursor.
 * \li Syntax: scroll <TYPE> <QUANTITY>
 * \li Params: <TYPE>:  line|page\n
               <QUANTITY>: up|down|<number>\n
 * \li Origin: Abdelrazak Younes, Dec 27 2007
 */
	LFUN_SCROLL,
	LFUN_LASTACTION                  // end of the table
};


} // namespace lyx

#endif // LFUNS_H
