/*!
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

#include "FuncRequest.h"

#include "support/debug.h"
#include "support/lstrings.h"

#include "support/lassert.h"

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

/* LFUN documentation (a start at least, Chr 2007-08-12)
 *
 * The documentation below is primarily notes about restrictions and
 * oddities relating to the different LFUNs.
 *
 * Try to find an appropriate (thematical) place when adding the new LFUN.
 *
 * Doxygen template below. Some notes: Parameters should be set in uppercase
 * and put in <BRACKETS>, as so.
 */

/*!
 * \var lyx::FuncCode lyx::LFUN_
 * \li Action: 
 * \li Notion: 
 * \li Syntax: 
 * \li Params: 
 * \li Sample:
 * \li Origin: 
 * \endvar
 */

LyXAction lyxaction;


void LyXAction::newFunc(FuncCode action, string const & name,
			unsigned int attrib, LyXAction::func_type type)
{
	lyx_func_map[name] = action;
	FuncInfo tmpinfo;
	tmpinfo.name = name;
	tmpinfo.attrib = attrib;
	tmpinfo.type = type;
	lyx_info_map[action] = tmpinfo;
}


// Needed for LFUNs documentation to be accepted, since doxygen won't take
// \var inside functions.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
		FuncCode action;
		char const * name;
		unsigned int attrib;
		func_type type;
	};

	ev_item const items[] = {
#endif
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_ACUTE
 * \li Action: Adds an acute accent \htmlonly (&aacute;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-acute
 * \endvar
 */
		{ LFUN_ACCENT_ACUTE, "accent-acute", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_BREVE
 * \li Action: Adds a breve accent \htmlonly (&#259;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-breve
 * \endvar
 */
		{ LFUN_ACCENT_BREVE, "accent-breve", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_CARON
 * \li Action: Adds a caron \htmlonly (&#462;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-caron
 * \endvar
 */
		{ LFUN_ACCENT_CARON, "accent-caron", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_CEDILLA
 * \li Action: Adds a cedilla \htmlonly (&ccedil;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-cedilla
 * \endvar
 */
		{ LFUN_ACCENT_CEDILLA, "accent-cedilla", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_CIRCLE
 * \li Action: Adds a circle accent \htmlonly (&aring;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-circle
 * \endvar
 */
		{ LFUN_ACCENT_CIRCLE, "accent-circle", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_CIRCUMFLEX
 * \li Action: Adds a circumflex \htmlonly (&ecirc;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-circumflex
 * \endvar
 */
		{ LFUN_ACCENT_CIRCUMFLEX, "accent-circumflex", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_DOT
 * \li Action: Adds a dot accent \htmlonly (&#380;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-dot
 * \endvar
 */
		{ LFUN_ACCENT_DOT, "accent-dot", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_GRAVE
 * \li Action: Adds a grave accent \htmlonly (&egrave;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-grave
 * \endvar
 */
		{ LFUN_ACCENT_GRAVE, "accent-grave", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_HUNGARIAN_UMLAUT
 * \li Action: Adds a Hungarian umlaut \htmlonly (&#337;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-grave
 * \endvar
 */
		{ LFUN_ACCENT_HUNGARIAN_UMLAUT, "accent-hungarian-umlaut", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_MACRON
 * \li Action: Adds a macron \htmlonly (&#257;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-macron
 * \endvar
 */
		{ LFUN_ACCENT_MACRON, "accent-macron", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_OGONEK
 * \li Action: Adds an ogonek accent \htmlonly (&#261;)\endhtmlonly
               to the next character typed.
 * \li Syntax: accent-ogonek
 * \endvar
 */
		{ LFUN_ACCENT_OGONEK, "accent-ogonek", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_TIE
 * \li Action: Adds a tie \htmlonly (a&#865;)\endhtmlonly
               over the next two character typed.
 * \li Notion: the following char will finish the tie.
 * \li Syntax: accent-tie
 * \endvar
 */
		{ LFUN_ACCENT_TIE, "accent-tie", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_TILDE
 * \li Action: Adds a tilde \htmlonly (&atilde;)\endhtmlonly
               over the next character typed.
 * \li Syntax: accent-tilde
 * \endvar
 */
		{ LFUN_ACCENT_TILDE, "accent-tilde", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_UMLAUT
 * \li Action: Adds an umlaut \htmlonly (&auml;)\endhtmlonly
               over the next character typed.
 * \li Syntax: accent-umlaut
 * \endvar
 */
		{ LFUN_ACCENT_UMLAUT, "accent-umlaut", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_UNDERBAR
 * \li Action: Adds a bar \htmlonly (a&#800;)\endhtmlonly
               under the next character typed.
 * \li Syntax: accent-underbar
 * \endvar
 */
		{ LFUN_ACCENT_UNDERBAR, "accent-underbar", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ACCENT_UNDERDOT
 * \li Action: Adds a dot \htmlonly (&#7841;)\endhtmlonly
               under the next character typed.
 * \li Syntax: accent-underdot
 * \endvar
 */
		{ LFUN_ACCENT_UNDERDOT, "accent-underdot", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_CAPTION_INSERT
 * \li Action: Inserts a caption inset.
 * \li Syntax: caption-insert
 * \li Origin: Lgb, 18 Jul 2000
 * \endvar
 */
		{ LFUN_CAPTION_INSERT, "caption-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DATE_INSERT
 * \li Action: Inserts the current date.
 * \li Syntax: date-insert [<ARG>]
 * \li Params: <ARG>: Format of date. The default value (%x) can be set
                     in Preferences->Date format. For possible formats
                     see manual page of strftime function.
 * \li Origin: jdblair, 31 Jan 2000
 * \endvar
 */
		{ LFUN_DATE_INSERT, "date-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FOOTNOTE_INSERT
 * \li Action: Inserts a footnote inset.
 * \li Syntax: footnote-insert
 * \li Origin: Jug, 7 Mar 2000
 * \endvar
 */
		{ LFUN_FOOTNOTE_INSERT, "footnote-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ERT_INSERT
 * \li Action: Inserts an ERT inset.
 * \li Syntax: ert-insert
 * \li Origin: Jug, 18 Feb 2000
 * \endvar
 */
		{ LFUN_ERT_INSERT, "ert-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FLOAT_INSERT
 * \li Action: Inserts a float inset.
 * \li Syntax: float-insert <TYPE>
 * \li Params: <TYPE>: type of float depends on the used textclass. Usually
                       "algorithm", "table", "figure" parameters can be given.
 * \li Origin: Lgb, 27 Jun 2000
 * \endvar
 */
		{ LFUN_FLOAT_INSERT, "float-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FLOAT_WIDE_INSERT
 * \li Action: Inserts float insets as in #LFUN_FLOAT_INSERT but span multiple columns.
 * \li Notion: Corresponds to the starred floats (figure*, table*, etc.) in LaTeX.
 * \li Syntax: float-wide-insert <TYPE>
 * \li Params: <TYPE>: type of float depends on the used textclass. Usually
                       "algorithm", "table", "figure" parameters can be given.
 * \li Origin: Lgb, 31 Oct 2001
 * \endvar
 */
		{ LFUN_FLOAT_WIDE_INSERT, "float-wide-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FLOAT_LIST_INSERT
 * \li Action: Inserts the list of floats in the document.
 * \li Syntax: float-list-insert <TYPE>
 * \li Params: <TYPE>: type of float depends on the used textclass. Usually
                       "algorithm", "table", "figure" parameters can be given.
 * \li Origin: Lgb, 3 May 2001
 * \endvar
 */
		{ LFUN_FLOAT_LIST_INSERT, "float-list-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WRAP_INSERT
 * \li Action: Inserts floats wrapped by the text around.
 * \li Syntax: wrap-insert <TYPE>
 * \li Params: <TYPE>: table|figure
 * \li Origin: Dekel, 7 Apr 2002
 * \endvar
 */
		{ LFUN_WRAP_INSERT, "wrap-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_OPTIONAL_INSERT
 * \li Action: Inserts an optional-argument (short title) inset.
 * \li Syntax: optional-insert
 * \li Origin: Martin, 12 Aug 2002
 * \endvar
 */
		{ LFUN_OPTIONAL_INSERT, "optional-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LINE_INSERT
 * \li Action: Inserts a horizontal line.
 * \li Syntax: line-insert
 * \li Origin: poenitz,  Oct 27 2003
 * \endvar
 */
		{ LFUN_LINE_INSERT, "line-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_NEWPAGE_INSERT
 * \li Action: Inserts a new page.
 * \li Syntax: newpage-insert<ARG>
 * \li Params: <ARG>: <newpage|pagebreak|clearpage|cleardoublepage> default: newpage
 * \li Origin: uwestoehr, 24 Nov 2007
 * \endvar
 */
		{ LFUN_NEWPAGE_INSERT, "newpage-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_MARGINALNOTE_INSERT
 * \li Action: Inserts a marginal note.
 * \li Syntax: marginalnote-insert
 * \li Origin: Lgb, 26 Jun 2000
 * \endvar
 */
		{ LFUN_MARGINALNOTE_INSERT, "marginalnote-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_UNICODE_INSERT
 * \li Action: Inserts a single unicode character.
 * \li Syntax: unicode-insert <CHAR>
 * \li Params: <CHAR>: The character to insert, given as its code
                       point, in hexadecimal, e.g.: unicode-insert 0x0100.
 * \li Origin: Lgb, 22 Oct 2006
 * \endvar
 */
		{ LFUN_UNICODE_INSERT, "unicode-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LISTING_INSERT
 * \li Action: Inserts a new listings inset.
 * \li Syntax: listing-insert
 * \li Origin: Herbert, 10 Nov 2001; bpeng, 2 May 2007
 * \endvar
 */
		{ LFUN_LISTING_INSERT, "listing-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_QUOTE_INSERT
 * \li Action: Inserts quotes according to the type and quote-language preference.
 * \li Action: Currently  English, Swedish, German, Polish, French, Danish quotes
               are distinguished.
 * \li Syntax: quote-insert [<TYPE>]
 * \li Params: TYPE: 'single' for single quotes, otherwise double quotes will be used.
 * \endvar
 */
		{ LFUN_QUOTE_INSERT, "quote-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INFO_INSERT
 * \li Action: Displays shortcuts, lyxrc, package and textclass availability and menu
               information in a non-editable boxed InsetText.
 * \li Notion: Apart from lfun arguments you can use the following method: \n
               1. input the type and argument of this inset, e.g. "menu paste", in
               the work area.\n
	       2. select the text and run info-insert lfun.\n
 * \li Syntax: info-insert <TYPE> <ARG>
 * \li Params: <TYPE>: shortcut|lyxrc|package|textclass|menu|buffer \n
               <ARG>: argument for a given type. Look into InsetInfo.h for detailed
	              description.
 * \li Origin: bpeng, 7 Oct 2007
 * \endvar
 */
		{ LFUN_INFO_INSERT, "info-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BRANCH_INSERT
 * \li Action: Inserts branch inset.
 * \li Syntax: branch-insert <BRANCH-NAME>
 * \li Origin: vermeer, 17 Aug 2003
 * \endvar
 */
		{ LFUN_BRANCH_INSERT, "branch-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BOX_INSERT
 * \li Action: Inserts Box inset.
 * \li Syntax: box-insert [<TYPE>]
 * \li Params: <TYPE>: Boxed|Frameless|Framed|ovalbox|Ovalbox|Shadowbox|Shaded|Doublebox \n
                       Framed is the default one.
 * \li Origin: vermeer, 7 Oct 2003
 * \endvar
 */
		{ LFUN_BOX_INSERT, "box-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FLEX_INSERT
 * \li Action: Inserts CharStyle, Custom inset or XML short element.
 * \li Notion: Look into the Customization manual for more information about
               these elements.\n
               To make this command enabled the layout file for the document
               class you're using has to load the character styles. There are
               a few contained in the Logical Markup module. You can also of
               course create some yourself. \n
	       For dissolving the element see #LFUN_INSET_DISSOLVE.
 * \li Syntax: flex-insert <TYPE:Name>
 * \li Params: TYPE: CharStyle|Custom|Element|Standard
		     Identifies whether this is a Character Style, a
		     Custom Inset or an XML Element, and which dynamical 
		     sub-menu this flex inset is in on the LyX menu tree. 
		     If Standard (currently unused): none of these.
	       Name: This name must be defined either in your layout file
		     or imported by some module. The definition is \n
		     InsetLayout <TYPE:Name>
 * \li Sample: flex-insert CharStyle:Code
 * \endvar
 */
		{ LFUN_FLEX_INSERT, "flex-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SELF_INSERT
 * \li Action: Inserts the given string (accordingly to the correct keymap).
 * \li Notion: Automatically replace the currently selected text. Depends on lyxrc
               settings "auto_region_delete".
 * \li Syntax: self-insert <STRING>
 * \endvar
 */
		{ LFUN_SELF_INSERT, "self-insert", SingleParUpdate, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_SPACE_INSERT
 * \li Action: Inserts one of horizontal space insets.
 * \li Syntax: space-insert <NAME> [<LEN>]
 * \li Params: <NAME>: normal, protected, thin, quad, qquad, enspace, enskip,
                       negthinspace, hfill, hfill*, dotfill, hrulefill, hspace,
                       hspace* \n
               <LEN>: length for custom spaces (hspace, hspace* for protected)
 * \li Origin: JSpitzm, 20 May 2003, Mar 17 2008
 * \endvar
 */
		{ LFUN_SPACE_INSERT, "space-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_HYPERLINK_INSERT
 * \li Action: Inserts hyperlinks into the document (clickable in pdf output).
 * \li Notion: Hyperlink target can be set via selection + hyperlink-insert function.
 * \li Syntax: href-insert [<TARGET>]
 * \li Origin: CFO-G, 21 Nov 1997
 * \endvar
 */
		{ LFUN_HYPERLINK_INSERT, "href-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SPECIALCHAR_INSERT
 * \li Action: Inserts various characters into the document.
 * \li Syntax: specialchar-insert <CHAR>
 * \li Params: <CHAR>: hyphenation, ligature-break, slash, nobreakdash, dots,
                       end-of-sentence, menu-separator.
 * \li Origin: JSpitzm, 6 Dec 2007
 * \endvar
 */
		{ LFUN_SPECIALCHAR_INSERT, "specialchar-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_TOC_INSERT
 * \li Action: Inserts table of contents.
 * \li Syntax: toc-insert
 * \li Origin: Lgb, 27 May 97
 * \endvar
 */
		{ LFUN_TOC_INSERT, "toc-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_APPENDIX
 * \li Action: Start (or remove) Appendix on the given cursor position.
 * \li Syntax: appendix
 * \li Origin: ettrich, 5 May 1998
 * \endvar
 */
		{ LFUN_APPENDIX, "appendix", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_INDEX_INSERT
 * \li Action: Inserts Index entry.
 * \li Notion: It automatically takes the word on the cursor position.
 * \li Syntax: index-insert
 * \li Origin: Angus, 3 Aug 2000
 * \endvar
 */
		{ LFUN_INDEX_INSERT, "index-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INDEX_PRINT
 * \li Action: Inserts list of Index entries on a new page.
 * \li Syntax: index-print
 * \li Origin: Lgb, 27 Feb 1997
 * \endvar
 */
		{ LFUN_INDEX_PRINT, "index-print", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_NOMENCL_INSERT
 * \li Action: Inserts Nomenclature entry.
 * \li Notion: It automatically takes the word on the cursor position if no symbol is given.
 * \li Syntax: nomencl-insert [<SYMBOL>]
 * \li Origin: Ugras, 4 Nov 2006
 * \endvar
 */
		{ LFUN_NOMENCL_INSERT, "nomencl-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_NOMENCLATURE_PRINT
 * \li Action: Inserts list of Nomenclature entries.
 * \li Syntax: nomenclature-print
 * \li Origin: Ugras, 4 Nov 2006
 * \endvar
 */
		{ LFUN_NOMENCL_PRINT, "nomencl-print", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_NOTE_INSERT
 * \li Action: Inserts Note on the current cursor postion,
               move selection inside the inset.
 * \li Syntax: note-insert [<TYPE>]
 * \li Params: <TYPE>: <Note|Greyedout|Comment> default: Note
 * \endvar
 */
		{ LFUN_NOTE_INSERT, "note-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_NOTE_NEXT
 * \li Action: Moves the cursor to the begining of next Note inset.
 * \li Syntax: note-next
 * \endvar
 */
		{ LFUN_NOTE_NEXT, "note-next", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_NOTES_MUTATE
 * \li Action: Changes all Note insets of a particular type (source)
               to a different type (target) fot the current document.
 * \li Syntax: notes-mutate <SOURCE> <TARGET>
 * \li Params: <SOURCE/TARGET>: Note|Comment|Greyedout
 * \li Origin: ps, 18 Jun 2008
 * \endvar
 */
		{ LFUN_NOTES_MUTATE, "notes-mutate", Argument, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_NEWLINE_INSERT
 * \li Action: Inserts a line break or new line.
 * \li Syntax: newline-insert [<ARG>]
 * \li Params: <ARG>: <newline|linebreak> default: newline
 * \li Origin: JSpitzm, 25 Mar 2008
 * \endvar
 */
		{ LFUN_NEWLINE_INSERT, "newline-insert", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_ESCAPE
 * \li Action: Clears the selection. If no text is selected call #LFUN_FINISHED_FORWARD.
 * \li Syntax: escape
 * \li Origin: Lgb, 17 May 2001
 * \endvar
 */
		{ LFUN_ESCAPE, "escape", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DOWN
 * \li Action: Moves the cursor one line in downward direction.
 * \li Syntax: down
 * \endvar
 */
		{ LFUN_DOWN, "down", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_UP
 * \li Action: Moves the cursor one line in upward direction.
 * \li Syntax: up
 * \endvar
 */
		{ LFUN_UP, "up", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DOWN_SELECT
 * \li Action: Moves the cursor one line in downward direction adding the current
               position to the selection.
 * \li Syntax: down-select
 * \endvar
 */
		{ LFUN_DOWN_SELECT, "down-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_UP_SELECT
 * \li Action: Moves the cursor one line in upward direction adding the current
               position to the selection.
 * \li Syntax: up-select
 * \endvar
 */
		{ LFUN_UP_SELECT, "up-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::SCREEN_DOWN
 * \li Action: Moves the cursor one page in downward direction.
 * \li Syntax: screen-down
 * \endvar
 */
		{ LFUN_SCREEN_DOWN, "screen-down", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SCREEN_UP
 * \li Action: Moves the cursor one page in upward direction.
 * \li Syntax: screen-up
 * \endvar
 */
		{ LFUN_SCREEN_UP, "screen-up", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SCREEN_DOWN_SELECT
 * \li Action: Moves the cursor one screen in downward direction adding the current
               position to the selection.
 * \li Syntax: screen-down-select
 * \endvar
 */
		{ LFUN_SCREEN_DOWN_SELECT, "screen-down-select", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SCREEN_UP_SELECT
 * \li Action: Moves the cursor one page in upward direction adding the current
               position to the selection.
 * \li Syntax: screen-up-select
 * \endvar
 */
		{ LFUN_SCREEN_UP_SELECT, "screen-up-select", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SCROLL
 * \li Action: Scroll the buffer view.
 * \li Notion: Only scrolls the screen up or down; does not move the cursor.
 * \li Syntax: scroll <TYPE> <QUANTITY>
 * \li Params: <TYPE>:  line|page\n
               <QUANTITY>: up|down|<number>
 * \li Origin: Abdelrazak Younes, Dec 27 2007
 * \endvar
 */
		{ LFUN_SCROLL, "scroll", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SCREEN_RECENTER
 * \li Action: Recenters the screen on the current cursor position.
 * \li Syntax: screen-recenter
 * \endvar
 */
		{ LFUN_SCREEN_RECENTER, "screen-recenter", ReadOnly, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_BACKWARD
 * \li Action: Moves the cursor one position logically backwards.
 * \li Notion: This is not the action which should be bound to the arrow keys,
               because backwards may be left or right, depending on the
               language. The arrow keys should be bound to #LFUN_CHAR_LEFT or
               #LFUN_CHAR_RIGHT actions, which in turn may employ this one.
 * \li Syntax: char-backward
 * \endvar
 */
		{ LFUN_CHAR_BACKWARD, "char-backward", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_BACKWARD_SELECT
 * \li Action: Moves the cursor one position logically backwards, adding
               traversed position to the selection.
 * \li Notion: See also #LFUN_CHAR_BACKWARD.
 * \li Syntax: char-backward-select
 * \endvar
 */
		{ LFUN_CHAR_BACKWARD_SELECT, "char-backward-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_DELETE_BACKWARD
 * \li Action: Deletes one character in the backward direction (usually the "BackSpace" key).
 * \li Syntax: char-delete-backward
 * \endvar
 */
		{ LFUN_CHAR_DELETE_BACKWARD, "char-delete-backward", SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_DELETE_FORWARD
 * \li Action: Deletes one character in the backward direction (usually the "Delete" key).
 * \li Syntax: char-delete-forward
 * \endvar
 */
		{ LFUN_CHAR_DELETE_FORWARD, "char-delete-forward", SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_FORWARD
 * \li Action: Moves the cursor one position logically forward.
 * \li Notion: This is not the action which should be bound to the arrow keys,
               because forward may be left or right, depending on the language.
               The arrow keys should be bound to #LFUN_CHAR_LEFT or
               #LFUN_CHAR_RIGHT actions, which in turn may employ this one.
 * \li Syntax: char-forward
 * \endvar
 */
		{ LFUN_CHAR_FORWARD, "char-forward", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_FORWARD_SELECT
 * \li Action: Moves the cursor one position logically forward, adding
               traversed position to the selection.
 * \li Notion: See also #LFUN_CHAR_FORWARD.
 * \li Syntax: char-forward-select
 * \endvar
 */
		{ LFUN_CHAR_FORWARD_SELECT, "char-forward-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_LEFT
 * \li Action: Moves the cursor one position "to the left".
 * \li Notion: This is the action which should be taken when the "left" key
               is pressed. Generally, it moves the cursor one position to the
               left. However, in Bidi text this become slightly more
               complicated, and there are different modes of cursor movement.
               In "visual mode", this moves left, plain and simple. In "logical
               mode", movement is logically forward in RTL paragraphs, and
               logically backwards in LTR paragraphs.
 * \li Syntax: char-left
 * \endvar
 */
		{ LFUN_CHAR_LEFT, "char-left", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_LEFT_SELECT
 * \li Action: Moves the cursor one position "to the left", adding 
               traversed position to the selection.
 * \li Notion: See also #LFUN_CHAR_LEFT for exact details of the movement.
 * \li Syntax: char-left-select
 * \endvar
 */
		{ LFUN_CHAR_LEFT_SELECT, "char-left-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_RIGHT
 * \li Action: Moves the cursor one position "to the right".
 * \li Notion: This is the action which should be taken when the "right" key
               is pressed. Generally, it moves the cursor one position to the
               right. However, in Bidi text this become slightly more
               complicated, and there are different modes of cursor movement.
               In "visual mode", this moves right, plain and simple. In "logical
               mode", movement is logically forward in LTR paragraphs, and
               logically backwards in RTL paragraphs.
 * \li Syntax: char-right
 * \endvar
 */
		{ LFUN_CHAR_RIGHT, "char-right", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHAR_RIGHT_SELECT
 * \li Action: Moves the cursor one position "to the right", adding
               traversed position to the selection.
 * \li Notion: See also #LFUN_CHAR_RIGHT for exact details of the movement.
 * \li Syntax: char-right-select
 * \endvar
 */
		{ LFUN_CHAR_RIGHT_SELECT, "char-right-select", ReadOnly | SingleParUpdate, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_BACKWARD
 * \li Action: Moves the cursor to the logically previous beginning of a word.
 * \li Notion: This is not the action which should be bound to the arrow keys,
               because backwards may be left or right, depending on the
               language. The arrow keys should be bound to #LFUN_WORD_LEFT or
               #LFUN_WORD_RIGHT actions, which in turn may employ this one.
 * \li Syntax: word-backward
 * \endvar
 */
		{ LFUN_WORD_BACKWARD, "word-backward", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_BACKWARD_SELECT
 * \li Action: Moves the cursor to the logically previous beginning of a word,
			   adding the logically traversed text to the selection.
 * \li Notion: See also #LFUN_WORD_BACKWARD.
 * \li Syntax: word-backward-select
 * \endvar
 */
		{ LFUN_WORD_BACKWARD_SELECT, "word-backward-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_DELETE_BACKWARD
 * \li Action: Deletes characters to the begining of the word (usually the "C+BackSpace" key).
 * \li Syntax: word-delete-backward
 * \endvar
 */
		{ LFUN_WORD_DELETE_BACKWARD, "word-delete-backward", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_DELETE_FORWARD
 * \li Action: Deletes characters to the end of the word (usually the "C+Delete" key).
 * \li Syntax: word-delete-forward
 * \endvar
 */
		{ LFUN_WORD_DELETE_FORWARD, "word-delete-forward", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_FIND_FORWARD
 * \li Action: Search for a given string in forward direction.
 * \li Notion: Case sensitive, match words. If no argument given, last search repeated.
 * \li Syntax: word-find-forward [<STRING>]
 * \li Origin: Etienne, 16 Feb 1998
 * \endvar
 */
		{ LFUN_WORD_FIND_FORWARD, "word-find-forward", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_FIND_BACKWARD
 * \li Action: Search for a given string in backward direction.
 * \li Notion: Case sensitive, match words. If no argument given, last search repeated.
 * \li Syntax: word-find-backward [<STRING>]
 * \li Origin: Etienne, 20 Feb 1998
 * \endvar
 */
		{ LFUN_WORD_FIND_BACKWARD, "word-find-backward", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_FIND
 * \li Action: Search for next occurence of a string.
 * \li Syntax: word-find [<DATA>]
 * \li Params: <DATA>: data encoded from Find dialog (see #lyx::find2string()).
                       If no parameter is given, search with last find-dialog
		       data is used for search (i.e. find-next).
 * \li Origin: poenitz, Jan 7 2004
 * \endvar
 */
		{ LFUN_WORD_FIND, "word-find", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_FIND
 * \li Action: Replace a string in the document.
 * \li Syntax: word-replace [<DATA>]
 * \li Params: <DATA>: data is of the form
                       "<search> \n
                        <replace> \n
                        <casesensitive> <matchword> <all> <forward>"
 * \li Origin: poenitz, Jan 7 2004
 * \endvar
 */
		{ LFUN_WORD_REPLACE, "word-replace", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_FORWARD
 * \li Action: Moves the cursor to the logically next beginning of a word.
 * \li Notion: This is not the action which should be bound to the arrow keys,
               because forward may be left or right, depending on the language.
               The arrow keys should be bound to #LFUN_WORD_LEFT or
               #LFUN_WORD_RIGHT actions, which in turn may employ this one.
 * \li Syntax: word-forward
 * \endvar
 */
		{ LFUN_WORD_FORWARD, "word-forward", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_FORWARD_SELECT
 * \li Action: Moves the cursor to the logically next beginning of a word, 
  			   adding the logically traversed text to the selection.
 * \li Notion: See also #LFUN_WORD_FORWARD.
 * \li Syntax: word-forward-select
 * \endvar
 */
		{ LFUN_WORD_FORWARD_SELECT, "word-forward-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_LEFT
 * \li Action: Moves the cursor to the next beginning of a word "on the left".
 * \li Notion: This is the action which should be taken when the (e.g., ctrl-)
			   "left" key is pressed. Generally, it moves the cursor to the 
			   next beginning of a word on the left. However, in Bidi text this 
			   become slightly more complicated, and there are different modes
			   of cursor movement. In "visual mode", this moves left, plain and
			   simple. In "logical mode", movement is logically forward in RTL
			   paragraphs, and logically backwards in LTR paragraphs.
 * \li Syntax: word-left
 * \li Origin: dov, 28 Oct 2007
 * \endvar
 */
		{ LFUN_WORD_LEFT, "word-left", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_LEFT_SELECT
 * \li Action: Moves the cursor to the next beginning of a word "on the left",
			   adding *logically* traversed text to the selection.
 * \li Notion: See also #LFUN_WORD_LEFT for exact details of the movement.
 * \li Syntax: word-left-select
 * \li Origin: dov, 28 Oct 2007
 * \endvar
 */
		{ LFUN_WORD_LEFT_SELECT, "word-left-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_RIGHT
 * \li Action: Moves the cursor to the next beginning of a word "on the right".
 * \li Notion: This is the action which should be taken when the (e.g., ctrl-)
			   "right" key is pressed. Generally, it moves the cursor to the 
			   next beginning of a word on the right. However, in Bidi text 
			   this become slightly more complicated, and there are different
			   modes of cursor movement. In "visual mode", this moves right,
			   plain and simple. In "logical mode", movement is logically 
			   forward in LTR paragraphs, and logically backwards in RTL 
			   paragraphs.
 * \li Syntax: word-right
 * \li Origin: dov, 28 Oct 2007
 * \endvar
 */
		{ LFUN_WORD_RIGHT, "word-right", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_RIGHT_SELECT
 * \li Action: Moves the cursor to the next beginning of a word "on the right",
			   adding *logically* traversed text to the selection.
 * \li Notion: See also #LFUN_WORD_RIGHT for exact details of the movement.
 * \li Syntax: word-right-select
 * \li Origin: dov, 28 Oct 2007
 * \endvar
 */
		{ LFUN_WORD_RIGHT_SELECT, "word-right-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_SELECT
 * \li Action: Puts the word where the cursor stands into the selection.
 * \li Syntax: word-select
 * \li Author: Andre, 11 Sep 2002
 * \endvar
 */
		{ LFUN_WORD_SELECT, "word-select", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_CAPITALIZE
 * \li Action: Capitalizes the words in the selection (i.e. the first letters)
               or the letter on the cursor position.
 * \li Syntax: word-capitalize
 * \endvar
 */
		{ LFUN_WORD_CAPITALIZE, "word-capitalize", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_UPCASE
 * \li Action: Change the words in the selection or from the cursor position
               to the end of word to the upper case.
 * \li Syntax: word-upcase
 * \endvar
 */
		{ LFUN_WORD_UPCASE, "word-upcase", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_WORD_LOWCASE
 * \li Action: Change the words in the selection or from the cursor position
               to the end of word to the lower case.
 * \li Syntax: word-lowcase
 * \endvar
 */
		{ LFUN_WORD_LOWCASE, "word-lowcase", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_THESAURUS_ENTRY
 * \li Action: Look up thesaurus entries with respect to the word under the cursor.
 * \li Syntax: thesaurus-entry
 * \li Origin: Levon, 20 Jul 2001
 * \endvar
 */
		{ LFUN_THESAURUS_ENTRY, "thesaurus-entry", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_BEGIN
 * \li Action: Move the cursor to the beginning of the document.
 * \li Syntax: buffer-begin
 * \endvar
 */
		{ LFUN_BUFFER_BEGIN, "buffer-begin", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_BEGIN_SELECT
 * \li Action: Move the cursor to the beginning of the document adding the
               traversed text to the selection.
 * \li Syntax: buffer-begin-select
 * \endvar
 */
		{ LFUN_BUFFER_BEGIN_SELECT, "buffer-begin-select", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_END
 * \li Action: Move the cursor to the end of the document.
 * \li Syntax: buffer-end
 * \endvar
 */
		{ LFUN_BUFFER_END, "buffer-end", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_END_SELECT
 * \li Action: Move the cursor to the end of the document adding the
               traversed text to the selection.
 * \li Syntax: buffer-end-select
 * \endvar
 */
		{ LFUN_BUFFER_END_SELECT, "buffer-end-select", ReadOnly, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_LINE_BEGIN
 * \li Action: Move the cursor to the begining of the (screen) line.
 * \li Syntax: line-begin
 * \endvar
 */
		{ LFUN_LINE_BEGIN, "line-begin", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LINE_BEGIN_SELECT
 * \li Action: Move the cursor to the beginning of the (screen) line adding the
               traversed text to the selection.
 * \li Syntax: line-begin-select
 * \endvar
 */
		{ LFUN_LINE_BEGIN_SELECT, "line-begin-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LINE_END
 * \li Action: Move the cursor to the end of the (screen) line.
 * \li Syntax: line-end
 * \endvar
 */
		{ LFUN_LINE_END, "line-end", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LINE_END_SELECT
 * \li Action: Move the cursor to the end of the (screen) line adding the
               traversed text to the selection.
 * \li Syntax: line-end-select
 * \endvar
 */
		{ LFUN_LINE_END_SELECT, "line-end-select", ReadOnly | SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LINE_DELETE
 * \li Action: Deletes the letters to the end of the (screen) line or
               deletes the selection.
 * \li Syntax: line-delete-forward
 * \endvar
 */
		{ LFUN_LINE_DELETE, "line-delete-forward", Noop, Edit }, // there is no line-delete-backward
/*!
 * \var lyx::FuncCode lyx::LFUN_COPY
 * \li Action: Copies to the clipboard the last edit.
 * \li Syntax: copy
 * \endvar
 */
		{ LFUN_COPY, "copy", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CUT
 * \li Action: Cuts to the clipboard.
 * \li Syntax: cut
 * \endvar
 */
		{ LFUN_CUT, "cut", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PASTE
 * \li Action: Pastes material from the active clipboard.
 * \li Syntax: paste [<TYPE>]
 * \li Params: <TYPE>: pdf|png|jpeg|linkback
 * \endvar
 */
		{ LFUN_PASTE, "paste", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CLIPBOARD_PASTE
 * \li Action: Pastes text from the active clipboard.
 * \li Syntax: clipboard-paste [<ARG>]
 * \li Params: <ARG>: "paragraph" will cause pasting as one paragraph, i.e. "Join lines".
 * \li Origin: baum, 10 Jul 2006
 * \endvar
 */
		{ LFUN_CLIPBOARD_PASTE, "clipboard-paste", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PRIMARY_SELECTION_PASTE
 * \li Action: Pastes the currently text selected text.
 * \li Notion: Primary selection mechanism is linux-only thing.
 * \li Syntax: primary-selection-paste [<ARG>]
 * \li Params: <ARG>: "paragraph" will cause pasting as one paragraph, i.e. "Join lines".
 * \endvar
 */
		{ LFUN_PRIMARY_SELECTION_PASTE, "primary-selection-paste", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_UNDO
 * \li Action: Undoes the last edit.
 * \li Syntax: undo
 * \endvar
 */
		{ LFUN_UNDO, "undo", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_REDO
 * \li Action: Redoes the last thing undone.
 * \li Syntax: redo
 * \endvar
 */
		{ LFUN_REDO, "redo", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_REPEAT
 * \li Action: Repeat the given command.
 * \li Syntax: repeat <COUNT> <LFUN-COMMAND>
 * \li Author: poenitz, 27 Oct 2003
 * \endvar
 */
		{ LFUN_REPEAT, "repeat", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHARS_TRANSPOSE
 * \li Action: Transposes the character at the cursor with the one before it.
 * \li Syntax: chars-transpose
 * \li Author: Lgb, 25 Apr 2001
 * \endvar
 */
		{ LFUN_CHARS_TRANSPOSE, "chars-transpose", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_DEPTH_DECREMENT
 * \li Action: Decrease the nesting depth of the (selected) paragraph(s)
               inside lists.
 * \li Syntax: depth-decrement
 * \endvar
 */
		{ LFUN_DEPTH_DECREMENT, "depth-decrement", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DEPTH_INCREMENT
 * \li Action: Increase the nesting depth of the (selected) paragraph(s)
               inside lists.
 * \li Syntax: depth-increment
 * \endvar
 */
		{ LFUN_DEPTH_INCREMENT, "depth-increment", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_BOLD
 * \li Action: Toggles the bold font (selection-wise).
 * \li Syntax: font-bold
 * \endvar
 */
		{ LFUN_FONT_BOLD, "font-bold", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_TYPEWRITER
 * \li Action: Toggles the typewriter family font (selection-wise).
 * \li Syntax: font-typewriter
 * \endvar
 */
		{ LFUN_FONT_TYPEWRITER, "font-typewriter", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_UNDERLINE
 * \li Action: Toggles underline in the font (selection-wise).
 * \li Syntax: font-underline
 * \endvar
 */
		{ LFUN_FONT_UNDERLINE, "font-underline", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_EMPH
 * \li Action: Toggles the emphasis font style (selection-wise).
 * \li Syntax: font-emph
 * \endvar
 */
		{ LFUN_FONT_EMPH, "font-emph", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_NOUN
 * \li Action: Toggles Noun text style font (selection-wise).
 * \li Syntax: font-noun
 * \endvar
 */
		{ LFUN_FONT_NOUN, "font-noun", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_ROMAN
 * \li Action: Toggles Roman family font (selection-wise).
 * \li Syntax: font-roman
 * \endvar
 */
		{ LFUN_FONT_ROMAN, "font-roman", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_SANS
 * \li Action: Toggles Sans Serif family font (selection-wise).
 * \li Syntax: font-sans
 * \endvar
 */
		{ LFUN_FONT_SANS, "font-sans", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_FRAK
 * \li Action: Toggles Fraktur family font (math-mode, selection-wise).
 * \li Syntax: font-frak
 * \li Origin: vermeer, 10 Jan 2002
 * \endvar
 */
		{ LFUN_FONT_FRAK, "font-frak", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_ITAL
 * \li Action: Toggles Italics font shape (math-mode, selection-wise).
 * \li Syntax: font-ital
 * \li Origin: vermeer, 10 Jan 2002
 * \endvar
 */
		{ LFUN_FONT_ITAL, "font-ital", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_DEFAULT
 * \li Action: Reverts the settings of the font to the default values (selection-wise).
 * \li Syntax: font-default
 * \endvar
 */
		{ LFUN_FONT_DEFAULT, "font-default", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_SIZE
 * \li Action: Sets font size according to lyx format string.
 * \li Syntax: font-size <SIZE>
 * \li Params: <SIZE>: tiny|scriptsize|footnotesize|small|normal|large|larger|
                       largest|huge|giant|increase|decrease|default
 * \endvar
 */
		{ LFUN_FONT_SIZE, "font-size", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_TEXTSTYLE_APPLY
 * \li Action: Toggle user-defined (=last-time used) text style.
 * \li Notion: This style is set via #LFUN_TEXTSTYLE_UPDATE, which is
               automatically trigerred when using Text Style dialog.
 * \li Syntax: textstyle-apply
 * \li Origin: leeming, 12 Mar 2003
 * \endvar
 */
		{ LFUN_TEXTSTYLE_APPLY, "textstyle-apply", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_TEXTSTYLE_UPDATE
 * \li Action: Apply text style and update the settings to be used by #LFUN_TEXTSTYLE_APPLY.
 * \li Syntax: textstyle-update <FONT_INFO>
 * \li Params: <FONT_INFO>: specifies font atributes, e.g. family, series, shape,
                            size, emph, noun, underbar, number, color, language,
			    toggleall.\n
			    Use lyx -dbg action for exact syntax of text-style
			    dialog parameters.
 * \li Origin: leeming, 12 Mar 2003
 * \endvar
 */
		{ LFUN_TEXTSTYLE_UPDATE, "textstyle-update", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_SCREEN_FONT_UPDATE
 * \li Action: Update fonts and its metrics.
 * \li Notion: Automatically called after zoom, dpi, font names, or norm change.
 * \li Syntax: screen-font-update
 * \li Origin: ARRae, 13 Aug 2000
 * \endvar
 */
		{ LFUN_SCREEN_FONT_UPDATE, "screen-font-update", NoBuffer, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_FONT_STATE
 * \li Action: Returns the info about the current font.
 * \li Syntax: font-state
 * \endvar
 */
		{ LFUN_FONT_STATE, "font-state", ReadOnly, Layout },

/*!
 * \var lyx::FuncCode lyx::LFUN_CITATION_INSERT
 * \li Action: Inserts citation from loaded citation database.
 * \li Syntax: citation-insert [<KEY>[|<TEXT_BEFORE>]]
 * \li Params: <KEY>: Citation (shortcut listed in available citations). \n
               <TEXT_BEFORE>: text which should appear before citation.
 * \li Origin: AAS, 97-02-23
 * \endvar
 */
		{ LFUN_CITATION_INSERT, "citation-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BIBTEX_DATABASE_ADD
 * \li Action: Adds database, which will be used for bibtex citations.
 * \li Notion: Databases are added to the first BibTeX inset
               (Inset->List/TOC->BibTeX bibliography) found from the cursor postion.
 * \li Syntax: bibtex-database-add <DATABASE-NAME>
 * \li Origin: Ale, 30 May 1997
 * \endvar
 */
		{ LFUN_BIBTEX_DATABASE_ADD, "bibtex-database-add", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BIBTEX_DATABASE_DEL
 * \li Action: Adds database, which will be used for bibtex citations.
 * \li Notion: Databases are deleted from the first BibTeX inset
               (Inset->List/TOC->BibTeX bibliography) found from the cursor postion.
 * \li Syntax: bibtex-database-del <DATABASE-NAME>
 * \li Origin: Ale, 30 May 1997
 * \endvar
 */
		{ LFUN_BIBTEX_DATABASE_DEL, "bibtex-database-del", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_LAYOUT
 * \li Action: Sets the layout (that is, environment) for the current paragraph.
 * \li Syntax: layout <LAYOUT>
 * \li Params: <LAYOUT>: the layout to use
 * \endvar
 */
		{ LFUN_LAYOUT, "layout", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_LAYOUT_PARAGRAPH
 * \li Action: Launches the paragraph settings dialog.
 * \li Syntax: layout-paragraph
 * \endvar
 */
		{ LFUN_LAYOUT_PARAGRAPH, "layout-paragraph", ReadOnly, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_LAYOUT_TABULAR
 * \li Action: Launches the tabular settings dialog.
 * \li Syntax: layout-tabular
 * \li Origin: Jug, 31 Jul 2000
 * \endvar
 */
		{ LFUN_LAYOUT_TABULAR, "layout-tabular", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_DROP_LAYOUTS_CHOICE
 * \li Action: Displays list of layout choices.
 * \li Notion: In the current (as of 2007) Qt4 frontend, this LFUN opens the
               dropbox allowing for choice of layout.
 * \li Syntax: drop-layouts-choice
 * \endvar
 */
		{ LFUN_DROP_LAYOUTS_CHOICE, "drop-layouts-choice", ReadOnly, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_LAYOUT_MODULES_CLEAR
 * \li Action: Clears the module list.
 * \li Notion: Clears the list of included modules for the current buffer.
 * \li Syntax: layout-modules-clear
 * \li Origin: rgh, 25 August 2007
 * \endvar
 */
		{ LFUN_LAYOUT_MODULES_CLEAR, "layout-modules-clear", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_LAYOUT_MODULE_ADD
 * \li Action: Adds a module.
 * \li Notion: Adds a module to the list of included modules for the current buffer.
 * \li Syntax: layout-module-add <MODULE>
 * \li Params: <MODULE>: the module to be added
 * \li Origin: rgh, 25 August 2007
 * \endvar
 */
		{ LFUN_LAYOUT_MODULE_ADD, "layout-module-add", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_LAYOUT_RELOAD
 * \li Action: Reloads layout information.
 * \li Notion: Reloads all layout information for the current buffer from disk, thus
               recognizing any changes that have been made to layout files on the fly.
               This is intended to be used only by layout developers and should not be
               used when one is trying to do actual work.
 * \li Syntax: layout-reload
 * \li Origin: rgh, 3 September 2007
 * \endvar
 */
		{ LFUN_LAYOUT_RELOAD, "layout-reload", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_TEXTCLASS_APPLY
 * \li Action: Sets the text class for the current buffer.
 * \li Syntax: textclass-apply <TEXTCLASS>
 * \li Params: <TEXTCLASS>: the textclass to set. Note that this must be
                   the filename, minus the ".layout" extension.
 * \endvar
 */
		{ LFUN_TEXTCLASS_APPLY, "textclass-apply", Noop, Layout },
/*!
 * \var lyx::FuncCode lyx::LFUN_TEXTCLASS_LOAD
 * \li Action: Loads information for a textclass from disk.
 * \li Syntax: textclass-load <TEXTCLASS>
 * \li Params: <TEXTCLASS>: the textclass to load. Note that this must be
                   the filename, minus the ".layout" extension.
 * \endvar
 */
		{ LFUN_TEXTCLASS_LOAD, "textclass-load", Noop, Layout },

/*!
 * \var lyx::FuncCode lyx::LFUN_MARK_OFF
 * \li Action: Disable selecting of text-region.
 * \li Syntax: mark-off
 * \endvar
 */
		{ LFUN_MARK_OFF, "mark-off", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_MARK_ON
 * \li Action: Enable selecting of text-region.
 * \li Notion: After enabling you can simply move arrow keys to get selected region.
 * \li Syntax: mark-on
 * \endvar
 */
		{ LFUN_MARK_ON, "mark-on", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_MARK_TOGGLE
 * \li Action: Toggle between #LFUN_MARK_ON and #LFUN_MARK_OFF .
 * \li Syntax: mark-toggle
 * \li Origin: poenitz, May 5 2006
 * \endvar
 */
		{ LFUN_MARK_TOGGLE, "mark-toggle", ReadOnly, Edit },
		
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_DELIM
 * \li Action: Inserts math delimiters (e.g. parentheses, brackets) enclosing expression.
 * \li Syntax: math-delim [<LEFT>] [<RIGHT>]
 * \li Params: <LEFT/RIGHT>: Delimiters to be used. Each delimiter can be specified by
                             either a LaTeX name or a valid character.
                             ( is the default letter.
 * \li Sample: math-delim { rangle
 * \li Origin: Alejandro, 18 Jun 1996
 * \endvar
 */
		{ LFUN_MATH_DELIM, "math-delim", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_BIGDELIM
 * \li Action: Inserts math fixed size delimiters (e.g. parentheses, brackets) enclosing expression.
 * \li Syntax: math-bigdelim <LSIZE> <LDELIM> <RSIZE> <RDELIM>
 * \li Params: <L/RSIZE>: bigl/r|Bigl/r|biggl/r|Biggl/r \n
               <L/RDELIM>: TeX code for delimiter. See Delimiter dialog for delimiters to be used.
 * \li Sample: math-bigdelim "Bigl" "\Downarrow" "Bigr" "\}"
 * \li Origin: Enrico & Georg, 7 May 2006
 * \endvar
 */
		{ LFUN_MATH_BIGDELIM, "math-bigdelim", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_DISPLAY
 * \li Action: Creates a new displayed equation in text mode.
               Toggles inlined/display formula in math mode.
 * \li Syntax: math-display [<ARG>]
 * \li Params: <ARG>: this argument will be passed to #LFUN_MATH_INSERT when creating
                      new equation from the text mode.
 * \li Origin: Alejandro, 18 Jun 1996
 * \endvar
 */
		{ LFUN_MATH_DISPLAY, "math-display", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_INSERT
 * \li Action: Inserts math objects and symbols.
 * \li Syntax: math-insert <ARG>
 * \li Params: <ARG>: Symbol or LaTeX code to be inserted.
 * \endvar
 */
		{ LFUN_MATH_INSERT, "math-insert", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_SUBSCRIPT
 * \li Action: Enters subscript expression in math expression.
 * \li Syntax: math-subscript
 * \li Origin: vermeer, 12 Dec 2001
 * \endvar
 */
		{ LFUN_MATH_SUBSCRIPT, "math-subscript", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_SUPERSCRIPT
 * \li Action: Enters subscript expression in math expression.
 * \li Syntax: math-superscript
 * \li Origin: vermeer, 12 Dec 2001
 * \endvar
 */
		{ LFUN_MATH_SUPERSCRIPT, "math-superscript", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_LIMITS
 * \li Action: Toggles the position of the limits from above/below to the right
               side an vice versa in integral symbol, a limit, a summation, etc.
 * \li Notion: Put the cursor before the symbol with the limits and then invoke
               math-limits.
 * \li Syntax: math-limits [<STATE>]
 * \li Params: <STATE>: limits|nolimits
 * \endvar
 */
		{ LFUN_MATH_LIMITS, "math-limits", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO
 * \li Action: Inserts a math macro definition at the cursor position in the text.
 * \li Syntax: math-macro <NAME> [<NARGS>] [def]
 * \li Params: <NAME>: The name of the macro, e.g. "mymacro".
               <NARGS>: The number of parameters of the macro. Default is 0.
               "def": Has no effect anymore, just for compatibility with former LyX versions.
 * \li Origin: ale, 10 May 1997;  sts, 21 Dec 2007
 * \endvar
 */
		{ LFUN_MATH_MACRO, "math-macro", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MUTATE
 * \li Action: Mutates the type of math inset to the newly selected one.
 * \li Syntax: math-mutate <TYPE>
 * \li Params: <TYPE>: none|simple|equation|eqnarray|align|alignat|xalignat|xxalignat|
                       multline|gather|flalign
 * \li Origin: Andre', 23 May 2001
 * \endvar
 */
		{ LFUN_MATH_MUTATE, "math-mutate", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_SPACE
 * \li Action: Inserts space into math expression.
 * \li Notion: Use spacebar after entering this space to change type of space.
 * \li Syntax: math-space [<TYPE>]
 * \li Params: <TYPE>: negative spaces: !|negmedspace|negthickspace \n
                       positive spaces: ,|:|;|quad|qquad \n
                       "," used by default.
 * \li Origin: Andre', 25 Jul 2001; ps, 16 Jun 2008
 * \endvar
 */
		{ LFUN_MATH_SPACE, "math-space", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MATRIX
 * \li Action: Inserts a matrix.
 * \li Syntax: math-matrix <COLS> <ROWS> [<ALIGN>]
 * \li Params: <ALIGN>: Alignment is a word composed of the vertical alignment
                        (b, c or t) (i.e. 1 char) and the horizontal alignments 
			(l, c or r) (i.e. <COL> chars).
 * \li Sample: math-matrix 3 3 bccc
 * \endvar
 */
		{ LFUN_MATH_MATRIX, "math-matrix", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MODE
 * \li Action: In text mode enters math mode (i.e. puts math insets on the current
               cursor position), in math mode enters text mode inside math expression.
 * \li Notion: If there is some selected text, it puts the text inside created math box.
 * \li Syntax: math-mode [<ARG>]
 * \li Params: <ARG>: eventual argument (LaTeX code) is passed to #LFUN_MATH_INSERT .
 * \li Origin: Alejandro, 4 Jun 1996
 * \endvar
 */
		{ LFUN_MATH_MODE, "math-mode", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_NUMBER_LINE_TOGGLE
 * \li Action: Toggles numbering of the current formula line.
 * \li Notion: Must be in display formula mode.
 * \li Syntax: math-number-line-toggle
 * \li Origin: Alejandro, 18 Jun 1996
 * \endvar
 */
		{ LFUN_MATH_NUMBER_LINE_TOGGLE, "math-number-line-toggle", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_NUMBER_TOGGLE
 * \li Action: Toggles numbering/labeling of the current formula.
 * \li Notion: Must be in display formula mode.
 * \li Syntax: math-number-toggle
 * \li Origin: Alejandro, 4 Jun 1996
 * \endvar
 */
		{ LFUN_MATH_NUMBER_TOGGLE, "math-number-toggle", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_EXTERN
 * \li Action: Calls external program and passes the current expression/equation
               as an argument for the calculation in the format appropriate to the
               given language.
 * \li Notion: Selection can be used to determine the input for the external program.
 * \li Syntax: math-extern <LANG> [<COMMAND>]
 * \li Params: <LANG>: octave|maxima|maple|mathematica|script \n
                       where "script" stands fot the external script (normalized
                       expression will be passed)
 * \li Origin: Andre', 24 Apr 2001
 * \li Sample: math-extern maple simplify
 * \endvar
 */
		{ LFUN_MATH_EXTERN, "math-extern", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_SIZE
 * \li Action: Changes arbitrarily the size used by math fonts inside a context.
 * \li Notion: Provides an interface to the LaTeX math mode font size commands.
 * \li Syntax: math-size <STYLE>
 *\ li Params: <STYLE>: \displaystyle|\textstyle|\scriptstyle|\scriptscriptstyle
 * \li Origin: Alejandro, 15 Aug 1996; ps, 14 Jun 2008
 * \endvar
 */
		{ LFUN_MATH_SIZE, "math-size", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_UNFOLD
 * \li Action: Unfold a Math Macro.
 * \li Notion: Unfold the Math Macro the cursor is in, i.e.
               display it as \foo.
 * \li Syntax: math-macro-unfold
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_UNFOLD, "math-macro-unfold", ReadOnly | SingleParUpdate, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_FOLD
 * \li Action: Fold a Math Macro.
 * \li Notion: Fold the Math Macro the cursor is in if it was
               unfolded, i.e. displayed as \foo before.
 * \li Syntax: math-macro-fold
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_FOLD, "math-macro-fold", ReadOnly | SingleParUpdate, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_ADD_PARAM
 * \li Action: Add a parameter.
 * \li Notion: Add a parameter to a Math Macro.
 * \li Params: <NUM>: The number of the parameter behind which the new one
               will be added (1 for the first, i.e. use 0 for add a
               parameter at the left), defaults to the last one.
 * \li Syntax: math-macro-add-param <NUM>
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_ADD_PARAM, "math-macro-add-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_REMOVE_PARAM
 * \li Action: Remove the last parameter.
 * \li Notion: Remove the last parameter of a Math Macro and
               remove its value in all instances of the macro
               in the buffer.
 * \li Params: <NUM>: The number of the parameter to be deleted (1 for
               the first), defaults to the last one.
 * \li Syntax: math-macro-remove-param <NUM>
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_REMOVE_PARAM, "math-macro-remove-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_APPEND_GREEDY_PARAM
 * \li Action: Append a greedy parameter.
 * \li Notion: Append a greedy parameter to a Math Macro which
               eats the following mathed cell in every instance of
               the macro in the buffer.
 * \li Syntax: math-macro-append-greedy-param
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_APPEND_GREEDY_PARAM, "math-macro-append-greedy-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM
 * \li Action: Remove a greedy parameter.
 * \li Notion: Remove a greedy parameter of a Math Macro and spit
               out the values of it in every instance of the macro
               in the buffer. If it is an optional parameter the [valud]
               format is used.
 * \li Syntax: math-macro-remove-greedy-param
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM, "math-macro-remove-greedy-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_MAKE_OPTIONAL
 * \li Action: Make a parameter optional.
 * \li Notion: Turn the first non-optional parameter of a Math Macro
               into an optional parameter with a default value.
 * \li Syntax: math-macro-make-optional
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_MAKE_OPTIONAL, "math-macro-make-optional", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_MAKE_NONOPTIONAL
 * \li Action: Make a parameter non-optional.
 * \li Notion: Turn the last optional parameter of a Math Macro
               into a non-optional parameter. The default value is
               remembered to be reused later if the user changes his mind.
 * \li Syntax: math-macro-make-nonoptional
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_MAKE_NONOPTIONAL, "math-macro-make-nonoptional", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM
 * \li Action: Add an optional parameter.
 * \li Notion: Insert an optional parameter just behind the
               already existing optional parameters.
 * \li Syntax: math-macro-add-optional-param
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM, "math-macro-add-optional-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM
 * \li Action: Remove the last optional parameter.
 * \li Notion: Remove the last optional parameter of a Math Macro and
               remove it in all the instances of the macro in the buffer.
 * \li Syntax: math-macro-remove-optional-param
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM, "math-macro-remove-optional-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM
 * \li Action: Add a greedy optional parameter.
 * \li Notion: Add a greedy optional parameter which eats the value
               from the following cells in mathed which are in the [value]
               format.
 * \li Syntax: math-macro-add-greedy-optional-param
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM, "math-macro-add-greedy-optional-param", Noop, Math },
/*!
 * \var lyx::FuncCode lyx::LFUN_IN_MATHMACROTEMPLATE
 * \li Action: Only active in Math Macro definition.
 * \li Notion: Dummy function which is only active in a Math Macro definition.
               It's used to toggle the Math Macro toolbar if the cursor moves
               into a Math Macro definition.
 * \li Syntax: in-mathmacrotemplate
 * \li Origin: sts, 06 January 2008
 * \endvar
 */
		{ LFUN_IN_MATHMACROTEMPLATE, "in-mathmacrotemplate", Noop, Math },

/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_MOVE_DOWN
 * \li Action: Moves the current paragraph downwards in the document.
 * \li Syntax: paragraph-move-down
 * \li Origin: Edwin, 8 Apr 2006
 * \endvar
 */
		{ LFUN_PARAGRAPH_MOVE_DOWN, "paragraph-move-down", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_MOVE_UP
 * \li Action: Moves the current paragraph upwards in the document.
 * \li Syntax: paragraph-move-up
 * \li Origin: Edwin, 8 Apr 2006
 * \endvar
 */
		{ LFUN_PARAGRAPH_MOVE_UP, "paragraph-move-up", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_UP
 * \li Action: Move the cursor to the next paragraph (or begining of the current one)
               in upward direction.
 * \li Syntax: paragraph-up
 * \li Origin: Asger, 1 Oct 1996
 * \endvar
 */
		{ LFUN_PARAGRAPH_UP, "paragraph-up", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_UP_SELECT
 * \li Action: Move the cursor and select the text to the next paragraph (or
               begining of the current one) in upward direction.
 * \li Syntax: paragraph-up-select
 * \li Origin: Asger, 1 Oct 1996
 * \endvar
 */
		{ LFUN_PARAGRAPH_UP_SELECT, "paragraph-up-select", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_DOWN
 * \li Action: Move the cursor to the next paragraph (or begining of the current one)
               in downward direction.
 * \li Syntax: paragraph-down
 * \li Origin: Asger, 1 Oct 1996
 * \endvar
 */
		{ LFUN_PARAGRAPH_DOWN, "paragraph-down", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_DOWN_SELECT
 * \li Action: Move the cursor and select the text to the next paragraph (or
               begining of the current one) in downward direction.
 * \li Syntax: paragraph-down-select
 * \li Origin: Asger, 1 Oct 1996
 * \endvar
 */
		{ LFUN_PARAGRAPH_DOWN_SELECT, "paragraph-down-select", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_GOTO
 * \li Action: Jump to a paragraph given by its id number and optionally the
               desired position within the paragraph.
 * \li Notion: Note that id number of paragraph is not the sequential number of paragraph
               seen on the screen. Moreover the id is unique for all opened buffers (documents).
 * \li Syntax: paragraph-goto <PAR_ID_NUMBER> <POSITION_IN_PAR>
 * \li Params: <PAR_ID_NUMBER>:  paragraph id
               <POSITION_IN_PAR>: desired position within the paragraph
 * \li Origin: Dekel, 26 Aug 2000
 * \endvar
 */
		{ LFUN_PARAGRAPH_GOTO, "paragraph-goto", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BREAK_PARAGRAPH
 * \li Action: Breaks the current paragraph at the current location.
 * \li Syntax: break-paragraph
 * \endvar
 */
		{ LFUN_PARAGRAPH_SPACING, "paragraph-spacing", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BREAK_PARAGRAPH
 * \li Action: Breaks the current paragraph at the current location.
 * \li Notion: Removes the selection.
 * \li Syntax: break-paragraph [<LAYOUT>]
 * \li Params: <LAYOUT>: "inverse" - decreases depth by one (or change layout
                         to default layout) when the cursor is at the end of
                         the line.
 * \endvar
 */
		{ LFUN_BREAK_PARAGRAPH, "break-paragraph", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_PARAMS
 * \li Action: Change paragraph settings.
 * \li Notion: Modifies the current paragraph, or currently selected paragraphs.
               This function only modifies, and does not override, existing settings.
               Note that the "leftindent" indent setting is deprecated.
 * \li Syntax: paragraph-params [<INDENT>] [<SPACING>] [<ALIGN>] [<OTHERS>]
 * \li Params: <INDENT>:  \\noindent|\\indent|\\indent-toggle|\\leftindent LENGTH\n
               <SPACING>: \\paragraph_spacing default|single|onehalf|double|other\n
               <ALIGN>:   \\align block|left|right|center|default\n
               <OTHERS>:  \\labelwidthstring WIDTH|\\start_of_appendix\n
 * \li Origin: rgh, Aug 15 2007
 * \endvar
 */
		{ LFUN_PARAGRAPH_PARAMS, "paragraph-params", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_PARAMS_APPLY
 * \li Action: Change paragraph settings.
 * \li Notion: Overwrite all nonspecified settings to the default ones.
               Use paragraph-params lfun if you don't want to overwrite others settings.
 * \li Syntax: paragraph-params-apply <INDENT> <SPACING> <ALIGN> <OTHERS>
 * \li Params: For parameters see #LFUN_PARAGRAPH_PARAMS
 * \li Origin: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_PARAGRAPH_PARAMS_APPLY, "paragraph-params-apply", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_PARAGRAPH_UPDATE
 * \li Action: Updates the values inside the paragraph dialog from the paragraph.
 * \li Notion: This is internal LFUN, not to be used by users. Called internally
               by #LFUN_DIALOG_UPDATE.
 * \li Origin: leeming, 13 Mar 2003
 * \endvar
 */
		{ LFUN_PARAGRAPH_UPDATE, "", Noop, Hidden },

/*!
 * \var lyx::FuncCode lyx::LFUN_OUTLINE_UP
 * \li Action: Move the current group in the upward direction in the
               structure of the document.
 * \li Notion: The "group" can be Part/Chapter/Section/etc. It moves
               the whole substructure of the group.
 * \li Syntax: outline-up
 * \li Origin: Vermeer, 23 Mar 2006
 * \endvar
 */
		{ LFUN_OUTLINE_UP, "outline-up", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_OUTLINE_DOWN
 * \li Action: Move the current group in the downward direction in the
               structure of the document.
 * \li Notion: The "group" can be Part/Chapter/Section/etc. It moves
               the whole substructure of the group.
 * \li Syntax: outline-down
 * \li Origin: Vermeer, 23 Mar 2006
 * \endvar
 */
		{ LFUN_OUTLINE_DOWN, "outline-down", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_OUTLINE_IN
 * \li Action: Moves the current group in the downward direction in the
               hierarchy of the document structure.
 * \li Notion: Part -> Chapter -> Section -> etc.
 * \li Syntax: outline-in
 * \li Origin: Vermeer, 23 Mar 2006
 * \endvar
 */
		{ LFUN_OUTLINE_IN, "outline-in", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_OUTLINE_OUT
 * \li Action: Moves the current group in the upward direction in the
               hierarchy of the document structure.
 * \li Notion: Part <- Chapter <- Section <- etc.
 * \li Syntax: outline-out
 * \li Origin: Vermeer, 23 Mar 2006
 * \endvar
 */
		{ LFUN_OUTLINE_OUT, "outline-out", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_EDIT
 * \li Action: Edit the inset at cursor with an external application,
 *             if one is attributed.
 * \li Syntax: inset-edit [<INSET_PARAMS>]
 * \li Params: <INSET_PARAMS>: Parameters for the inset.
                               Currently only the filename will be considered.
 * \li Origin: JSpitzm, 27 Apr 2006
 * \endvar
 */
		{ LFUN_INSET_EDIT, "inset-edit", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_TABULAR_INSERT
 * \li Action: Inserts table into the document.
 * \li Syntax: tabular-insert [<ROWS> <COLUMNS>]
 * \li Params: In case no arguments are given show insert dialog.
 * \li Origin: Jug, 12 Apr 2000
 * \endvar
 */
		{ LFUN_TABULAR_INSERT, "tabular-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_TABULAR_FEATURE
 * \li Action: Sets various features to the table/cell on the current cursor position.
 * \li Notion: Various math-environment features are handled here as well, e.g.
               add-vline-left/right for Grid/Cases environment
 * \li Syntax: tabular-feature <FEATURE> [<ARG>]
 * \li Params: <FEATURE>: append-row|append-column|delete-row|delete-column|copy-row|copy-column|
                       toggle-line-top|toggle-line-bottom|toggle-line-left|toggle-line-right|
                       align-left|align-right|align-center|align-block|valign-top|valign-bottom|
                       valign-middle|m-align-left|m-align-right|m-align-center|m-valign-top|
                       m-valign-bottom|m-valign-middle|multicolumn|set-all-lines|unset-all-lines|
                       set-longtabular|unset-longtabular|set-pwidth|set-mpwidth|
                       set-rotate-tabular|unset-rotate-tabular|toggle-rotate-tabular|
                       set-rotate-cell|unset-rotate-cell|toggle-rotate-cell|set-usebox|set-lthead|
                       unset-lthead|set-ltfirsthead|unset-ltfirsthead|set-ltfoot|unset-ltfoot|
                       set-ltlastfoot|unset-ltlastfoot|set-ltnewpage|toggle-ltcaption|
                       set-special-column|set-special-multi|set-booktabs|unset-booktabs|
                       set-top-space|set-bottom-space|set-interline-space|set-border-lines \n
		<ARG>: additional argument for some commands, use debug mode to explore its values.
 * \li Origin: Jug, 28 Jul 2000
 * \endvar
 */
		{ LFUN_TABULAR_FEATURE, "tabular-feature", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CELL_BACKWARD
 * \li Action: Moves the cursor to the previous cell inside the table.
 * \li Syntax: cell-backward
 * \li Origin: Jug, 22 May 2000
 * \endvar
 */
		{ LFUN_CELL_BACKWARD, "cell-backward", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CELL_FORWARD
 * \li Action: Moves the cursor to the next cell inside the table.
 * \li Syntax: cell-forward
 * \endvar
 */
		{ LFUN_CELL_FORWARD, "cell-forward", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CELL_SPLIT
 * \li Action: Splits cell and shifts right part to the next cell (inside the math grid).
 * \li Syntax: cell-split
 * \li Origin: Ale, 15 May 1997
 * \endvar
 */
		{ LFUN_CELL_SPLIT, "cell-split", Noop, Edit },

		{ LFUN_VC_CHECK_IN, "vc-check-in", ReadOnly, System },
		{ LFUN_VC_CHECK_OUT, "vc-check-out", ReadOnly, System },
		{ LFUN_VC_REGISTER, "vc-register", ReadOnly, System },
		{ LFUN_VC_REVERT, "vc-revert", ReadOnly, System },
		{ LFUN_VC_UNDO_LAST, "vc-undo-last", ReadOnly, System },

/*!
 * \var lyx::FuncCode lyx::LFUN_CHANGES_TRACK
 * \li Action: Toggles change tracking to on/off.
 * \li Syntax: changes-track
 * \li Origin: levon, 1 Oct 2002
 * \endvar
 */
		{ LFUN_CHANGES_TRACK, "changes-track", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHANGES_OUTPUT
 * \li Action: Toggles showing of change tracking in typesetted output.
 * \li Syntax: changes-output
 * \li Origin: jspitzm, 21 Jan 2005
 * \endvar
 */
		{ LFUN_CHANGES_OUTPUT, "changes-output", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHANGE_NEXT
 * \li Action: Moves the cursor to the position of the next change
               of the change tracking records.
 * \li Syntax: change-next
 * \li Origin: schmitt, 4 Oct 2006
 * \endvar
 */
		{ LFUN_CHANGE_NEXT, "change-next", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHANGES_MERGE
 * \li Action: Open change tracking dialog for merging and moves the cursor
               to the position of the next change.
 * \li Syntax: changes-merge
 * \li Origin: Levon, 16 Oct 2002
 * \endvar
 */
		{ LFUN_CHANGES_MERGE, "changes-merge", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHANGE_ACCEPT
 * \li Action: Accepts tracked change inside the selection.
 * \li Syntax: change-accept
 * \li Origin: Levon, 16 Oct 2002
 * \endvar
 */
		{ LFUN_CHANGE_ACCEPT, "change-accept", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_CHANGE_REJECT
 * \li Action: Rejects tracked change inside the selection.
 * \li Syntax: change-accept
 * \li Origin: Levon, 16 Oct 2002
 * \endvar
 */
		{ LFUN_CHANGE_REJECT, "change-reject", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ALL_CHANGES_ACCEPT
 * \li Action: Accepts all tracked changes in the document.
 * \li Syntax: all-changes-accept
 * \li Origin: Levon, 16 Oct 2002
 * \endvar
 */
		{ LFUN_ALL_CHANGES_ACCEPT, "all-changes-accept", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_ALL_CHANGES_REJECT
 * \li Action: Rejects all tracked changes in the document.
 * \li Notion: Reject does not work recursively; the user may have to repeat the operation.
 * \li Syntax: all-changes-reject
 * \li Origin: Levon, 16 Oct 2002
 * \endvar
 */
		{ LFUN_ALL_CHANGES_REJECT, "all-changes-reject", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_APPLY
 * \li Action: Apply data for an inset.
 * \li Notion: LFUN_INSET_APPLY is sent from the dialogs when the data should
               be applied. This is either changed to #LFUN_INSET_MODIFY or
               #LFUN_INSET_INSERT depending on the context where it is called.
 * \li Syntax: inset-apply <ARGS>
 * \li Params: See #LFUN_INSET_INSERT .
 * \endvar
 */
		{ LFUN_INSET_APPLY, "inset-apply", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_DISSOLVE
 * \li Action: Dissolve the current inset into text.
 * \li Syntax: inset-dissolve [<INSET>]
 * \li Params: <INSET>: this can be used to make sure the right kind of inset
                        is dissolved. For example "dissolve" entry in the charstyles
                        sub-menu should only dissolve the charstyle inset, even if the
                        cursor is inside several nested insets of different type.\n
			For values see #lyx::InsetLayout::lyxtype_ .
 * \li Author: JSpitz, 7 Aug 2006
 * \endvar
 */
		{ LFUN_INSET_DISSOLVE, "inset-dissolve", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_INSERT
 * \li Action: Insert new inset (type given by the parameters).
 * \li Syntax: inset-insert <INSET> <ARGS>
 * \li Params: <INSET>: <bibitem|bibtex|cite|ert|listings|external|graphics|
                         hyperlink|include|index|label|nomencl|vspace|ref|toc>\n
               <ARGS>: depends on the given inset. Use "lyx -dbg action" to
	               explore.
 * \li Sample: inset-insert ref LatexCommand <Format> reference "<label name>"\end_inset \n
               where <label name> is the name of the referenced label and
	       <Format> is one of the following: \n
	       ref -- <reference> \n
	       eqref -- (<reference>) \n
	       pageref -- <page> \n
               vpageref -- on <page> \n
	       vref -- <reference> on <page> \n
               prettyref -- Formatted reference
 * \endvar
 */
		{ LFUN_INSET_INSERT, "inset-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_MODIFY
 * \li Action: Modify existing inset.
 * \li Notion: Used for label, floats, listings, box, branch, external, wrap
               bibtex, ert, command, graphics, note, space, vspace, tabular,
               bibitem, inlude, ref insets.
 * \li Syntax: inset-modify <INSET> <ARGS>
 * \li Params: See #LFUN_INSET_INSERT for further details.
 * \endvar
 */
		{ LFUN_INSET_MODIFY, "", Noop, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_NEXT_INSET_MODIFY
 * \li Action: Modify the inset at cursor position, if there is one.
 * \li Notion: Used for label, floats, listings, box, branch, external, wrap
               bibtex, ert, command, graphics, note, space, vspace, tabular,
               bibitem, inlude, ref insets.
 * \li Syntax: next-inset-modify <INSET> <ARGS> or next-inset-modify changetype <TYPE>
 * \li Params: See #LFUN_INSET_INSERT for further details.
 * \li Origin: JSpitzm, 23 Mar 2008
 * \endvar
 */
		{ LFUN_NEXT_INSET_MODIFY, "next-inset-modify", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_DIALOG_UPDATE
 * \li Action: Updates the values inside the dialog from the inset.
 * \li Notion: This is internal LFUN, not to be used by users. Called internally
               by #LFUN_DIALOG_UPDATE
 * \li Params: <DIALOG-NAME>
 * \li Origin: leeming, 25 Feb 2003
 * \endvar
 */
		{ LFUN_INSET_DIALOG_UPDATE, "", Noop, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_SETTINGS
 * \li Action: Open the inset's properties dialog.
 * \li Notion: Used for box, branch, ert, float, listings, note, tabular, wrap insets.
 * \li Syntax: inset-settings <INSET>
 * \li Params: <INSET>: <box|branch|ert|float|listings|note|tabular|wrap>
 * \endvar
 */
		{ LFUN_INSET_SETTINGS, "inset-settings", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_NEXT_INSET_TOGGLE
 * \li Action: Toggles the inset at cursor position. For collapsables, this means it will
               be (un-)collapsed, in case of other insets, the editing widget (dialog) will
               be entered. Also cf. #LFUN_INSET_SETTINGS.
 * \li Notion: Used for label, floats, listings, box, branch, external, wrap
               bibtex, ert, command, graphics, note, space, vspace, tabular,
               bibitem, inlude, ref insets.
 * \li Syntax: next-inset-toggle <ARG>
 * \li Params: <ARG>: these are passed as arguments to #LFUN_INSET_TOGGLE .
 * \li Origin: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_NEXT_INSET_TOGGLE, "next-inset-toggle", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_INSET_TOGGLE
 * \li Action: Toggles the collapsable inset we are currently in.
 * \li Syntax: inset-toggle [<ARG>]
 * \li Params: <ARG>: <open|close|toggle|assign>. \n
               open/close/toggle are for collapsable insets. close can be currently used
               by #LFUN_NEXT_INSET_TOGGLE. toggle is used when no argument is given.\n
               assign is for branch inset.
 * \li Origin: lasgouttes, 19 Jul 2001
 * \endvar
 */
		{ LFUN_INSET_TOGGLE, "inset-toggle", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_ALL_INSETS_TOGGLE
 * \li Action: Toggles (open/closes) all collapsable insets (of a given type) in the document.
 * \li Notion: Used for box, branch, ert, float, listings, note, tabular, wrap insets.
 * \li Syntax: all-insets-toggle <STATE> <INSET>
 * \li Params: <STATE>: <toggle|open|close> default: toggle \n
               <INSET>: <box|branch|ert|float|listings|note|tabular|wrap> default: all insets \n
 * \li Origin: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_ALL_INSETS_TOGGLE, "all-insets-toggle", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_GRAPHICS_GROUPS_UNIFY
 * \li Action: Unify all graphics insets with the one given as an parameter.
 * \li Notion: This is internally used for synchronize certain group of graphics insets.
 * \li Syntax: graphics-groups-unigfy <GRAPHICS_PARAMS>
 * \li Params: <GRAPHICS_PARAMS>: Parameters for graphics inset
                                  (syntax can be seen in .lyx files).
 * \li Origin: sanda, 6 May 2008
 * \endvar
 */
		{ LFUN_GRAPHICS_GROUPS_UNIFY, "graphics-groups-unify", Argument, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_SET_GRAPHICS_GROUP
 * \li Action: Set the group for the graphics inset on the cursor position.
 * \li Syntax: set-graphics-group [<GROUP>]
 * \li Params: <GROUP>: Id for an existing group. In case tthe Id is an empty string,
                        the graphics inset is removed from the current group.
 * \li Origin: sanda, 6 May 2008
 * \endvar
 */
		{ LFUN_SET_GRAPHICS_GROUP, "set-graphics-group", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_FINISHED_FORWARD
 * \li Action: Moves the cursor out of the current slice, going forward.
 * \li Notion: Cursor movement within an inset may be different than cursor
               movement in the surrounding text. This action should be called
               automatically by the cursor movement within the inset, when
               movement within the inset has ceased (reached the end of the
               last paragraph, for example), in order to move correctly
               back into the surrounding text.
 * \endvar
 */
		{ LFUN_FINISHED_FORWARD, "", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_FINISHED_BACKWARD
 * \li Action: Moves the cursor out of the current slice, going backwards.
 * \li Notion: See also #LFUN_FINISHED_FORWARD.
 * \endvar
 */
		{ LFUN_FINISHED_BACKWARD, "", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_FINISHED_RIGHT
 * \li Action: Moves the cursor out of the current slice, going right.
 * \li Notion: See also #LFUN_FINISHED_FORWARD
 * \endvar
 */
		{ LFUN_FINISHED_RIGHT, "", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_FINISHED_LEFT
 * \li Action: Moves the cursor out of the current slice, going left.
 * \li Notion: See also #LFUN_FINISHED_FORWARD.
 * \endvar
 */
		{ LFUN_FINISHED_LEFT, "", ReadOnly, Hidden },

/*!
 * \var lyx::FuncCode lyx::LFUN_LANGUAGE
 * \li Action: Set language from the current cursor position.
 * \li Syntax: language <LANG>
 * \li Params: <LANG>: Requested language. Look in lib/languages for
                       the list.
 * \li Origin: Dekel, 2 Mar 2000
 * \endvar
 */
		{ LFUN_LANGUAGE, "language", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_LABEL_GOTO
 * \li Action: Goto a label.
 * \li Syntax: label-goto [<LABEL>]
 * \li Params: <LABEL>: Requested label. If no label is given and refernce
			is on cursor position, Bookmark 0 is saved and
			cursor moves to the position of referenced label.
 * \li Origin: Ale, 6 Aug 1997
 * \endvar
 */
		{ LFUN_LABEL_GOTO, "label-goto", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_LABEL_INSERT
 * \li Action: Inserts label to text or displayed formula.
 * \li Syntax: label-insert [<LABEL>]
 * \li Params: <LABEL>: Requested label. If no label is given dialog requesting
                        name will be opened.
 * \endvar
 */
		{ LFUN_LABEL_INSERT, "label-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_REFERENCE_NEXT
 * \li Action: Go to the next label or cross-reference.
 * \li Syntax: reference-next
 * \li Origin: Dekel, 14 Jan 2001
 * \endvar
 */
		{ LFUN_REFERENCE_NEXT, "reference-next", ReadOnly, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_BOOKMARK_GOTO
 * \li Action: Goto a bookmark.
 * \li Notion: Moves the cursor to the numbered bookmark, opening the file
               if necessary. Note that bookmarsk are saved per-session, not
               per file.
 * \li Syntax: bookmark-goto <NUMBER>
 * \li Params: <NUMBER>: the number of the bookmark to restore.
 * \li Origin: Dekel, 27 January 2001
 * \endvar
 */
		{ LFUN_BOOKMARK_GOTO, "bookmark-goto", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BOOKMARK_SAVE
 * \li Action: Save a bookmark.
 * \li Notion: Saves a numbered bookmark to the sessions file. The number
               must be between 1 and 9, inclusive. Note that bookmarks are
               saved per-session, not per file.
 * \li Syntax: bookmark-save <NUMBER>
 * \li Params: <NUMBER>: the number of the bookmark to save.
 * \li Origin: Dekel, 27 January 2001
 * \endvar
 */
		{ LFUN_BOOKMARK_SAVE, "bookmark-save", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BOOKMARK_CLEAR
 * \li Action: Clears the list of saved bookmarks.
 * \li Syntax: bookmark-clear
 * \li Origin: bpeng, 31 October 2006
 * \endvar
 */
		{ LFUN_BOOKMARK_CLEAR, "bookmark-clear", NoBuffer, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_HELP_OPEN
 * \li Action: Open the given help file according to the language setting.
 * \li Syntax: help-open <FILE>[.lyx]
 * \li Params: <FILE>: any document from (/usr/share/)doc directory.
 * \li Author: Jug, 27 Jun 1999
 * \endvar
 */
		{ LFUN_HELP_OPEN, "help-open", NoBuffer | Argument, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_LYX_QUIT
 * \li Action: Terminates the current LyX instance.
 * \li Notion: Terminates the current LyX instance, asking whether to save
               modified documents, etc.
 * \li Syntax: lyx-quit
 * \endvar
 */
		{ LFUN_LYX_QUIT, "lyx-quit", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_TOOLBAR_TOGGLE
 * \li Action: Toggles visibility of a given toolbar between on/off/auto.
 * \li Notion: Skiping "auto" when allowauto is false.
 * \li Syntax: toolbar-toggle <NAME> [allowauto]
 * \li Params: <NAME>: standard|extra|table|math|mathmacrotemplate|
                       minibuffer|review|view/update|math_panels
 * \li Origin: Edwin, 21 May 2007
 * \endvar
 */
		{ LFUN_TOOLBAR_TOGGLE, "toolbar-toggle", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_MENU_OPEN
 * \li Action: Opens the menu given by its name.
 * \li Syntax: menu-open <NAME>
 * \li Params: <NAME>: menu name. See various .inc files in lib/ui for candidates.
 * \endvar
 */
		{ LFUN_MENU_OPEN, "menu-open", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_UI_TOGGLE
 * \li Action: Various UI visibility-toggling actions.
 * \li Syntax: ui-toggle <statusbar|menubar|frame|fullscreen>
 * \li Params: statusbar  : Toggle visibility of the statusbar.\n
               menubar    : Toggle visibility of the menubar.\n
	       scrollbar  : Toggle visibility of the scrollbar.\n
	       frame      : Toggle visibility of the frames around editing window.\n
	       fullscreen : Toggle fullscreen mode. This also covers calling the
	                    previous functions. However #LFUN_TOOLBAR_TOGGLE for the
			    custom tweaks of the toolbars should be used.
 * \li Origin: ps, 9 Feb 2007
 * \endvar
 */
		{ LFUN_UI_TOGGLE, "ui-toggle", NoBuffer, Buffer },

/*!
 * \var lyx::FuncCode lyx::WINDOW_NEW
 * \li Action: Creates new empty LyX window.
 * \li Notion: Already opened documents from the previous window can be found under View menu.
 * \li Syntax: window-new [<GEOMETRY>]
 * \li Params: <GEOMETRY>: pass the geometry of the window. This parameter is currently 
                           accepted only on Windows platform.
 * \li Origin: Abdel, 21 Oct 2006
 * \endvar
 */
		{ LFUN_WINDOW_NEW, "window-new", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_WINDOW_CLOSE
 * \li Action: Closes the current LyX window.
 * \li Syntax: window-close
 * \li Origin: Abdel, 23 Oct 2006
 * \endvar
 */
		{ LFUN_WINDOW_CLOSE, "window-close", NoBuffer, Buffer },

/*!
 * \var lyx::FuncCode lyx::LFUN_SPLIT_VIEW
 * \li Action: Creates another split view of current buffer.
 * \li Notion: All split views act in the same way indpendently.
 * \li Syntax: split-view <vertical|horizontal>
 * \li Params: horizontal : The work areas are laid out side by side.\n
               vertical   : The work areas laid out vertically.\n
 * \li Origin: abdel, 20 Feb 2008
 * \endvar
 */
		{ LFUN_SPLIT_VIEW, "split-view", ReadOnly, Buffer },

/*!
 * \var lyx::FuncCode lyx::LFUN_CLOSE_TAB_GROUP
 * \li Action: Close the current tab group.
 * \li Notion: This only closes the work areas, not the buffer themselves.
               The still opened buffers can be visualized in another tab group.
 * \li Syntax: close-tab-group
 * \li Origin: abdel, 21 Feb 2008
 * \endvar
 */
		{ LFUN_CLOSE_TAB_GROUP, "close-tab-group", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_DIALOG_SHOW
 * \li Action: Shows hidden dialog or create new one for a given function/inset settings etc.
 * \li Syntax: dialog-show <NAME> [<DATA>]
 * \li Params: <NAME>: aboutlyx|bibitem|bibtex|box|branch|changes|character|citation|
               document|errorlist|ert|external|file|findreplace|float|graphics|
               include|index|info|nomenclature|label|log|mathdelimiter|mathmatrix|
               note|paragraph|prefs|print|ref|sendto|space|spellchecker|symbols|
               tabular|tabularcreate|thesaurus|texinfo|toc|href|view-source|vspace|
               wrap|listings|<SPECIAL> \n
               <SPECIAL>: latexlog|vclog \n
               <DATA>: data, usually settings for the given dialog. Use debug mode for the
                       details.
 * \li Origin: leeming, 17 Jun 2003
 * \endvar
 */
		{ LFUN_DIALOG_SHOW, "dialog-show", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DIALOG_SHOW_NEW_INSET
 * \li Action: Shows hidden dialog or create new one for a given inset settings etc.
 * \li Notion: Internally uses #LFUN_DIALOG_SHOW with processed data for a given inset.
 * \li Syntax: dialog-show-new-inset <NAME> [<DATA>]
 * \li Params: See #LFUN_DIALOG_SHOW .
 * \li Origin: leeming, 25 Feb 2003
 * \endvar
 */
		{ LFUN_DIALOG_SHOW_NEW_INSET, "dialog-show-new-inset", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DIALOG_UPDATE
 * \li Action: Updates the dialog values from the inset/paragraph/document.
 * \li Syntax: dialog-update <NAME>
 * \li Params: <NAME>: paragraph|prefs|<INSET> \n
               <INSET>: inset name
 * \li Origin: leeming, 25 Feb 2003
 * \endvar
 */
		{ LFUN_DIALOG_UPDATE, "dialog-update", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DIALOG_HIDE
 * \li Action: Hides showed dialog. Counterpart to #LFUN_DIALOG_SHOW .
 * \li Syntax: dialog-hide <NAME>
 * \li Params: See #LFUN_DIALOG_SHOW .
 * \li Origin: leeming, 25 Feb 2003
 * \endvar
 */
		{ LFUN_DIALOG_HIDE, "dialog-hide", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DIALOG_TOGGLE
 * \li Action: Toggles dialog between showed/hidden state.
 * \li Notion: Internally uses #LFUN_DIALOG_SHOW , #LFUN_DIALOG_HIDE .
 * \li Syntax: dialog-toggle <NAME> [<DATA>]
 * \li Params: See #LFUN_DIALOG_SHOW .
 * \li Origin: JSpitzm, 30 Apr 2007
 * \endvar
 */
		{ LFUN_DIALOG_TOGGLE, "dialog-toggle", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_DIALOG_DISCONNECT_INSET
 * \li Action: Closes opened connection to opened inset.
 * \li Notion: Connection is used for apply functions.
 * \li Syntax: dialog-disconnect-inset <INSET-NAME>
 * \li Origin: leeming, 25 Feb 2003
 * \endvar
 */
		{ LFUN_DIALOG_DISCONNECT_INSET, "dialog-disconnect-inset", Noop, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_MOUSE_PRESS
 * \li Action: This function is called when mouse button is pressed (inside workarea).
               Action depends on the context.
 * \li Notion: This is internal LFUN, not to be used by users.
 * \li Origin: André, 9 Aug 2002
 * \endvar
 */
		{ LFUN_MOUSE_PRESS, "", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_MOUSE_DOUBLE
 * \li Action: This function is called when double click on mouse button is
               pressed (inside workarea). Action depends on the context.
 * \li Notion: This is internal LFUN, not to be used by users.
 * \li Origin: André, 9 Aug 2002
 * \endvar
 */
		{ LFUN_MOUSE_DOUBLE, "", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_MOUSE_TRIPLE
 * \li Action: This function is called when triple click on mouse button is
               pressed (inside workarea). Action depends on the context.
 * \li Notion: This is internal LFUN, not to be used by users.
 * \li Origin: André, 9 Aug 2002
 * \endvar
 */
		{ LFUN_MOUSE_TRIPLE, "", ReadOnly, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_MOUSE_MOTION
 * \li Action: This function is called when mouse cursor is moving over the text.
               Action depends on the context.
 * \li Notion: This is internal LFUN, not to be used by users.
 * \li Origin: André, 9 Aug 2002
 * \endvar
 */
		{ LFUN_MOUSE_MOTION, "", ReadOnly | SingleParUpdate, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_MOUSE_RELEASE
 * \li Action: This function is called when mouse button is released (inside workarea).
               Action depends on the context.
 * \li Notion: This is internal LFUN, not to be used by users.
 * \li Origin: André, 9 Aug 2002
 * \endvar
 */
		{ LFUN_MOUSE_RELEASE, "", ReadOnly, Hidden },

/*!
 * \var lyx::FuncCode lyx::LFUN_KEYMAP_OFF
 * \li Action: Turn off the loaded keyboard map.
 * \li Syntax: keymap-off
 * \endvar
 */
		{ LFUN_KEYMAP_OFF, "keymap-off", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_KEYMAP_PRIMARY
 * \li Action: Turn on the primary keyboard map.
 * \li Notion: Maps were widely used in past, when X-windows didn't have nowadays
               keyboard support. They can be still used to maintain uniform keyboard
               layout across the various plaforms.
 * \li Syntax: keymap-primary
 * \endvar
 */
		{ LFUN_KEYMAP_PRIMARY, "keymap-primary", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_KEYMAP_SECONDARY
 * \li Action: Turn on the secondary keyboard map.
 * \li Syntax: keymap-secondary
 * \endvar
 */
		{ LFUN_KEYMAP_SECONDARY, "keymap-secondary", ReadOnly, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_KEYMAP_TOGGLE
 * \li Action: Toggles keyboard maps (first/second/off).
 * \li Syntax: keymap-toggle
 * \li Origin: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_KEYMAP_TOGGLE, "keymap-toggle", ReadOnly, Edit },

/*!
 * \var lyx::FuncCode lyx::LFUN_SERVER_GET_LAYOUT
 * \li Action: Returns the current layout (that is environment) name
               on the cursor position.
 * \li Syntax: server-get-layout
 * \endvar
 */
		{ LFUN_SERVER_GET_LAYOUT, "server-get-layout", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_SERVER_GET_FILENAME
 * \li Action: Returns path and file name of the currently edited document.
 * \li Syntax: server-get-filename
 * \endvar
 */
		{ LFUN_SERVER_GET_FILENAME, "server-get-filename", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_SERVER_GOTO_FILE_ROW
 * \li Action: Sets the cursor position based on the row number of generated TeX file.
 * \li Syntax: server-goto-file-row <LYX_FILE_NAME> <ROW_NUMBER>
 * \li Origin: Edmar, 23 Dec 1998
 * \endvar
 */
		{ LFUN_SERVER_GOTO_FILE_ROW, "server-goto-file-row", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_SERVER_NOTIFY
 * \li Action: Sends notify message about the last key-sequence to client.
 * \li Notion: This can be used to grab last key-sequence used inside the LyX window.
 * \li Syntax: server-notify
 * \endvar
 */
		{ LFUN_SERVER_NOTIFY, "server-notify", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_SERVER_SET_XY
 * \li Action: Sets the cursor position based on the editing area coordinates
               (similar as clicking on that point with left mouse button).
 * \li Syntax: server-set-xy <X> <Y>
 * \endvar
 */
		{ LFUN_SERVER_SET_XY, "server-set-xy", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_SERVER_GET_XY
 * \li Action: Returns the coordinates of cursor position in the editing area.
 * \li Syntax: server-get-xy
 * \endvar
 */
		{ LFUN_SERVER_GET_XY, "server-get-xy", ReadOnly, System },

/*!
 * \var lyx::FuncCode lyx::LFUN_BUILD_PROGRAM
 * \li Action: Generates the code (literate programming).
 * \li Notion: Latex file with extension \literate_extension is generated.
               Then LyX invokes \build_command (with a default of``make'') to generate the code and
	       \build_error_filter to process the compilation error messages. \n
	       In case you want to process your literate file with a script, or some other program,
	       just insert in your lyxrc file an entry with:\n
	       \build_command "my_script my_arguments" \n
	       The \build_error_filter differs from the \literate_error_filter only in that the
	       former will identify error messages from your compiler.
 * \li Syntax: build-program
 * \endvar
 */
		{ LFUN_BUILD_PROGRAM, "build-program", ReadOnly, Buffer },

/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_AUTO_SAVE
 * \li Action: Saves the current buffer to a temporary file.
 * \li Notion: Saves the current buffer to a file named "#filename#". This LFUN
               is called automatically by LyX, to "autosave" the current buffer.
 * \li Syntax: buffer-auto-save
 * \endvar
 */
		{ LFUN_BUFFER_AUTO_SAVE, "buffer-auto-save", Noop, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_CHILD_OPEN
 * \li Action: Loads the given child document.
 * \li Notion: The current document is treated as a parent.
 * \li Syntax: buffer-child-open <FILE>
 * \li Params: <FILE>: Filename of the child. The directory of the parent is assumed by default.
 * \li Origin: Ale, 28 May 1997
 * \endvar
 */
		{ LFUN_BUFFER_CHILD_OPEN, "buffer-child-open", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_CHKTEX
 * \li Action: Runs chktex for the current document.
 * \li Syntax: buffer-chktex
 * \li Origin: Asger, 30 Oct 1997
 * \endvar
 */
		{ LFUN_BUFFER_CHKTEX, "buffer-chktex", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_TOGGLE_COMPRESSION
 * \li Action: Toggles compression of the current document on/off.
 * \li Syntax: buffer-toggle-compression
 * \li Origin: bpeng, 27 Apr 2006
 * \endvar
 */
		{ LFUN_BUFFER_TOGGLE_COMPRESSION, "buffer-toggle-compression", Noop, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_CLOSE
 * \li Action: Closes the current buffer.
 * \li Notion: Closes the current buffer, asking whether to save it, etc,
               if the buffer has been modified.
 * \li Syntax: buffer-close
 * \endvar
 */
		{ LFUN_BUFFER_CLOSE, "buffer-close", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_EXPORT
 * \li Action: Exports the current buffer (document) to the given format.
 * \li Syntax: buffer-export <FORMAT>
 * \li Params: <FORMAT> is either "custom" or one of the formats which you
                        can find in Tools->Preferences->File formats->Format.
			Usual format you will enter is "pdf2" (pdflatex),
			"pdflatex" (plain tex for pdflatex) or "ps" for postscript.\n
			In case of "custom" you will be asked for a format you
			want to start from and for the command that you want to
			apply to this format. Internally the control is then passed
			to #LFUN_BUFFER_EXPORT_CUSTOM.
 * \li Origin: Lgb, 29 Jul 1997
 * \endvar
 */
		{ LFUN_BUFFER_EXPORT, "buffer-export", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_EXPORT_CUSTOM
 * \li Action: Exports the current buffer (document) from the given format using
               the given command on it.
 * \li Syntax: buffer-export-custom <FORMAT> <COMMAND>
 * \li Params: <FORMAT> format to start from (LyX will care to produce such
                        intermediate file).\n
               <COMMAND> this command will be launched on the file. Note that you can
	       use "$$FName" string to qualify the intermediate file.
 * \li Sample: buffer-export-custom dvi dvips -f $$FName -o myfile.ps
 * \li Origin: leeming, 27 Mar 2004
 * \endvar
 */
		{ LFUN_BUFFER_EXPORT_CUSTOM, "buffer-export-custom", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_PRINT
 * \li Action: Prints the current document.
 * \li Notion: Many settings can be given via the preferences dialog.
 * \li Syntax: buffer-print <TARGET> <TARGET-NAME> <COMMAND>
 * \li Params: <TARGET> is either "printer" or "file".\n
               <TARGER-NAME> is either "default" or file name or printer name.\n
	       <COMMAND> command ensuring the printing job.
 * \li Sample: buffer-print file "/trash/newfile1.ps" "dvips"
 * \li Origin: leeming, 28 Mar 2004
 * \endvar
 */
		{ LFUN_BUFFER_PRINT, "buffer-print", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_IMPORT
 * \li Action: Import a given file as a lyx document.
 * \li Notion: File can be imported iff lyx file format is (transitively) reachable via
               defined convertors in preferences. Look into File->Import menu to get
	       an idea of the currently active import formats.
 * \li Syntax: buffer-import <FORMAT> [<FILE>]
 * \li Origin: Asger, 24 Jul 1998
 * \endvar
 */
		{ LFUN_BUFFER_IMPORT, "buffer-import", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_NEW
 * \li Action: Creates a new buffer (that is, document).
 * \li Notion: Implicit path can be set in Preferences dialog.
 * \li Syntax: buffer-new [<FILE>]
 * \li Params: <FILE>: filename of created file with absolute path.
 * \endvar
 */
		{ LFUN_BUFFER_NEW, "buffer-new", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_NEW_TEMPLATE
 * \li Action: Creates a new buffer (that is, document) from a template.
 * \li Notion: Path for new files and templates can be set in Preferences dialog.
               Template will be asked for via Open-dialog.
 * \li Syntax: buffer-new-template [<FILE>]
 * \li Params: <FILE>: filename of created file with absolute path.
 * \endvar
 */
		{ LFUN_BUFFER_NEW_TEMPLATE,"buffer-new-template", NoBuffer, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_RELOAD
 * \li Action: Reverts opened document.
 * \li Syntax: buffer-reload
 * \li Origin: Asger, 2 Feb 1997
 * \endvar
 */
		{ LFUN_BUFFER_RELOAD, "buffer-reload", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_SWITCH
 * \li Action: Switch to the given buffer.
 * \li Notion: This is useful also in case you need simultaneously more views of the edited
               document in different LyX windows.
 * \li Syntax: buffer-new-template <BUFFER>
 * \li Params: <BUFFER>: already opened document which is to be shown.
 * \endvar
 */
		{ LFUN_BUFFER_SWITCH, "buffer-switch", NoBuffer | ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_TOGGLE_READ_ONLY
 * \li Action: Toggle editing mode of the current document between read/write and read-only.
 * \li Syntax: buffer-toggle-read-only
 * \li Origin: Lgb, 27 May 1997
 * \endvar
 */
		{ LFUN_BUFFER_TOGGLE_READ_ONLY, "buffer-toggle-read-only", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_VIEW
 * \li Action: Displays current buffer in chosen format.
 * \li Notion: Displays the contents of the current buffer in the chosen
               format, for example, PDF or DVI. This runs the necessary
               converter, calls the defined viewer, and so forth.
 * \li Syntax: buffer-view <FORMAT>
 * \li Params: <FORMAT>: The format to display, where this is one of the
                         formats defined (in the current GUI) in the
                         Tools>Preferences>File Formats dialog.
 * \endvar
 */
		{ LFUN_BUFFER_VIEW, "buffer-view", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_UPDATE
 * \li Action: Exports the current document and put the result into the
               temporary directory.
 * \li Notion: In case you are already viewing the exported document (see #LFUN_BUFFER_VIEW)
               the output will be rewriten - updated. This is useful in case your viewer
	       is able to detect such changes (e.g. ghostview for postscript).
 * \li Syntax: buffer-update <FORMAT>
 * \li Params: <FORMAT>: The format to display, where this is one of the
                         formats defined (in the current GUI) in the
                         Tools>Preferences>File Formats dialog.
 * \li Origin: Dekel, 5 Aug 2000
 * \endvar
 */
		{ LFUN_BUFFER_UPDATE, "buffer-update", ReadOnly, Buffer },

/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_WRITE
 * \li Action: Saves the current buffer.
 * \li Notion: Saves the current buffer to disk, using the filename that
               is already associated with the buffer, asking for one if
               none is yet assigned.
 * \li Syntax: buffer-write
 * \endvar
 */
		{ LFUN_BUFFER_WRITE, "buffer-write", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_WRITE_AS
 * \li Action: Rename and save current buffer.
 * \li Syntax: buffer-write-as <FILENAME>
 * \li Params: <FILENAME>: New name of the buffer/file. A relative path
               is with respect to the original location of the buffer/file.
 * \endvar
 */
		{ LFUN_BUFFER_WRITE_AS, "buffer-write-as", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_WRITE_ALL
 * \li Action: Save all changed documents.
 * \li Syntax: buffer-write-all
 * \li Origin: rgh, gpothier 6 Aug 2007
 * \endvar
 */
		{ LFUN_BUFFER_WRITE_ALL, "buffer-write-all", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_NEXT
 * \li Action: Switch to the next opened document.
 * \li Notion: Note that this does not necessarily mean next in tabbar
               (for full list see View menu).
 * \li Syntax: buffer-next
 * \endvar
 */
		{ LFUN_BUFFER_NEXT, "buffer-next", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_PREVIOUS
 * \li Action: Switch to the previous opened document.
 * \li Syntax: buffer-previous
 * \endvar
 */
		{ LFUN_BUFFER_PREVIOUS, "buffer-previous", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_MASTER_BUFFER_UPDATE
 * \li Action: When run from a child document, this updates (exports) document built
                from the master buffer. If a master is not found, it updates the
                current buffer.
 * \li Syntax: master-buffer-update
 * \li Author: Tommaso, 20 Sep 2007
 * \endvar
 */
		{ LFUN_MASTER_BUFFER_UPDATE, "master-buffer-update", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_MASTER_BUFFER_VIEW
 * \li Action: When run from a child document, this command shows a preview built
               from the master buffer. If a master is not found, it previews the
	       current buffer.
 * \li Syntax: master-buffer-view
 * \li Author: Tommaso, 20 Sep 2007
 * \endvar
 */
		{ LFUN_MASTER_BUFFER_VIEW, "master-buffer-view", ReadOnly, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_LANGUAGE
 * \li Action: Set language of the current document.
 * \li Syntax: buffer-language <LANG>
 * \li Params: <LANG>: language name. See lib/languages for list.
 * \li Author: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_BUFFER_LANGUAGE, "buffer-language", Noop, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_SAVE_AS_DEFAULT
 * \li Action: Save the current document settings as default.
 * \li Notion: The file will will be saved into ~/.lyx/templates/defaults.lyx .
 * \li Syntax: buffer-save-as-default [<ARGS>]
 * \li Params: <ARGS>: contains the particular settings to be saved. They obey the syntax
                       you can find in document header of usual .lyx file.
 * \li Author: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_BUFFER_SAVE_AS_DEFAULT, "buffer-save-as-default", Noop, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BUFFER_PARAMS_APPLY
 * \li Action: Apply the given settings to the current document.
 * \li Syntax: buffer-params-apply [<ARGS>]
 * \li Params: <ARGS>: contains the particular settings to be saved. They obey the syntax
                       you can find in document header of usual .lyx file.
 * \li Author: leeming, 30 Mar 2004
 * \endvar
 */
		{ LFUN_BUFFER_PARAMS_APPLY, "buffer-params-apply", Noop, Buffer },

/*!
 * \var lyx::FuncCode lyx::LFUN_FILE_INSERT
 * \li Action: Inserts another LyX file.
 * \li Syntax: file-insert [<FILE>]
 * \li Params: <FILE>: Filename to be inserted.
 * \endvar
 */
		{ LFUN_FILE_INSERT, "file-insert", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FILE_INSERT_PLAINTEXT
 * \li Action: Inserts plain text file.
 * \li Syntax: file-insert-plaintext [<FILE>]
 * \li Params: <FILE>: Filename to be inserted.
 * \li Origin: CFO-G, 19 Nov 1997
 * \endvar
 */
		{ LFUN_FILE_INSERT_PLAINTEXT, "file-insert-plaintext", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FILE_INSERT_PLAINTEXT_PARA
 * \li Action: Inserts plain text file as paragraph (i.e. join lines).
 * \li Syntax: file-insert-plaintext-para [<FILE>]
 * \li Params: <FILE>: Filename to be inserted.
 * \li Origin: Levon, 14 Feb 2001
 * \endvar
 */
		{ LFUN_FILE_INSERT_PLAINTEXT_PARA, "file-insert-plaintext-para", Noop, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_FILE_OPEN
 * \li Action: Open LyX document.
 * \li Syntax: file-open [<FILE>]
 * \li Params: <FILE>: Filename to be opened.
 * \endvar
 */
		{ LFUN_FILE_OPEN, "file-open", NoBuffer, Buffer },

/*!
 * \var lyx::FuncCode lyx::LFUN_CALL
 * \li Action: Executes a command defined in a .def file.
 * \li Notion: The definitions are by default read from lib/commands/default.def.\n
               A .def file allows to define a command with \\define "<NAME>" "<LFUN>"
               where <NAME> is the name of the new command and <LFUN> is the lfun code
               to be executed (see e.g. #LFUN_COMMAND_SEQUENCE).
               \\def_file "FileName" allows to include another .def file. \n
               This is particularly useful in connection with toolbar buttons:
               Since the name of the button image for this lfun is 
               lib/images/commands/<NAME>.png this is the way to assign an image
               to a complex command-sequence.
 * \li Syntax: call <NAME>
 * \li Params: <NAME>: Name of the command that must be called.
 * \li Origin: broider, 2 Oct 2007
 * \endvar
 */
		{ LFUN_CALL, "call", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_META_PREFIX
 * \li Action: Simulate halting Meta key (Alt key on PCs).
 * \li Notion: Used for buffer editation not for GUI control.
 * \li Syntax: meta-prefix
 * \endvar
 */
		{ LFUN_META_PREFIX, "meta-prefix", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_CANCEL
 * \li Action: Cancels sequence prepared by #LFUN_META_PREFIX .
 * \li Syntax: cancel
 * \endvar
 */
		{ LFUN_CANCEL, "cancel", NoBuffer, System },

/*!
 * \var lyx::FuncCode lyx::LFUN_COMMAND_EXECUTE
 * \li Action: Opens the minibuffer toolbar so that the user can type in there.
 * \li Notion: Usually bound to M-x shortcut.
 * \li Syntax: command-execute
 * \endvar
 */
		{ LFUN_COMMAND_EXECUTE, "command-execute", NoBuffer, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_COMMAND_PREFIX
 * \li Action: Return the current key sequence and available options as a string.
 * \li Notion: No options are added if no curmap kb map exists. \n
               This is probably usable only with connection to lyxserver.
 * \li Syntax: command-prefix
 * \endvar
 */
		{ LFUN_COMMAND_PREFIX, "command-prefix", NoBuffer, Hidden },
/*!
 * \var lyx::FuncCode lyx::LFUN_COMMAND_SEQUENCE
 * \li Action: Run more commands (LFUN and its parameters) in a sequence.
 * \li Syntax: command-sequence <CMDS>
 * \li Params: <CMDS>: Sequence of commands separated by semicolons.
 * \li Sample: command-sequence cut; ert-insert; self-insert \; paste; self-insert {}; inset-toggle;
 * \li Origin: Andre', 11 Nov 1999
 * \endvar
 */
		{ LFUN_COMMAND_SEQUENCE, "command-sequence", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_MESSAGE
 * \li Action: Shows message in statusbar (for script purposes).
 * \li Syntax: message <STRING>
 * \li Origin: Lgb, 8 Apr 2001
 * \endvar
 */
		{ LFUN_MESSAGE, "message", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_PREFERENCES_SAVE
 * \li Action: Save user preferences.
 * \li Syntax: preferences-save
 * \li Origin: Lgb, 27 Nov 1999
 * \endvar
 */
		{ LFUN_PREFERENCES_SAVE, "preferences-save", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_RECONFIGURE
 * \li Action: Reconfigure the automatic settings.
 * \li Syntax: reconfigure
 * \li Origin: Asger, 14 Feb 1997
 * \endvar
 */
		{ LFUN_RECONFIGURE, "reconfigure", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_LYXRC_APPLY
 * \li Action: Apply the given settings to user preferences.
 * \li Syntax: lyxrc-apply <SETTINGS>
 * \li Params: <SETTINGS>: settings which are to be set. Take a look into ~/.lyx/preferences
                           to get an idea which commands to use and their syntax.
                           #lyx::LyXRC::LyXRCTags has the list of possible commands.
 * \endvar
 */
		{ LFUN_LYXRC_APPLY, "lyxrc-apply", NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE
 * \li Action: Determine whether keep cursor inside the editing window regardless
               the scrollbar movement.
 * \li Syntax: toggle-cursor-follows-scrollbar
 * \li Origin: ARRae, 2 Dec 1997
 * \endvar
 */
		{ LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE, "cursor-follows-scrollbar-toggle", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_SET_COLOR
 * \li Action: Set the given LyX color to the color defined by the X11 name given.
 * \li Notion: A new color entry is created if the color is unknown.
               Color names can be stored as a part of user settings.
 * \li Syntax: set-color <LYX_NAME> <X11_NAME>
 * \li Origin: SLior, 11 Jun 2000
 * \endvar
 */
		{ LFUN_SET_COLOR, "set-color", ReadOnly | NoBuffer, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_STATISTICS
 * \li Action: Count the statistics (number of words and characters)
               in the document or in the given selection.
 * \li Notion: Note that this function gives the number of words/chars written,
               not the number of characters which will be typeset.
 * \li Syntax: statistics
 * \li Origin: lasgouttes, Jan 27 2004; ps, Jan 8 2008
 * \endvar
 */
		{ LFUN_STATISTICS, "statistics", ReadOnly, System },
/*!
 * \var lyx::FuncCode lyx::LFUN_COMPLETION_INLINE
 * \li Action: Show the inline completion at the cursor position.
 * \li Syntax: completion-inline
 * \li Origin: sts, Feb 19 2008
 * \endvar
 */
		{ LFUN_COMPLETION_INLINE, "completion-inline", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_COMPLETION_POPUP
 * \li Action: Show the completion popup at the cursor position.
 * \li Syntax: completion-popup
 * \li Origin: sts, Feb 19 2008
 * \endvar
 */
		{ LFUN_COMPLETION_POPUP, "completion-popup", ReadOnly | NoUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_COMPLETION_COMPLETE
 * \li Action: Try to complete the word or command at the cursor position.
 * \li Syntax: completion-complete
 * \li Origin: sts, Feb 19 2008
 * \endvar
 */
		{ LFUN_COMPLETION_COMPLETE, "complete", SingleParUpdate, Edit },
/*!
 * \var lyx::FuncCode lyx::LFUN_BRANCH_ACTIVATE
 * \li Action: Activate the branch
 * \li Syntax: branch-activate <BRANCH>
 * \li Params: <BRANCH>: The branch to activate
 * \li Sample: lyx -e pdf2 -x "branch-activate answers" finalexam.lyx \n
               could be used to export a pdf with the answers branch included
               without one's having to open LyX and activate the branch manually.
 * \li Origin: rgh, 27 May 2008
 * \endvar
 */
		{ LFUN_BRANCH_ACTIVATE, "branch-activate", Argument, Buffer },
/*!
 * \var lyx::FuncCode lyx::LFUN_BRANCH_ACTIVATE
 * \li Action: De-activate the branch
 * \li Syntax: branch-deactivate <BRANCH>
 * \li Params: <BRANCH>: The branch to deactivate
 * \li Origin: rgh, 27 May 2008
 * \endvar
 */
		{ LFUN_BRANCH_DEACTIVATE, "branch-deactivate", Argument, Buffer },

		{ LFUN_NOACTION, "", Noop, Hidden }
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	};

	for (int i = 0; items[i].action != LFUN_NOACTION; ++i) {
		newFunc(items[i].action, items[i].name, items[i].attrib, items[i].type);
	}

	init = true;
}
#endif

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


string const LyXAction::getActionName(FuncCode action) const
{
	info_map::const_iterator const it = lyx_info_map.find(action);
	return it != lyx_info_map.end() ? it->second.name : string();
}


LyXAction::func_type const LyXAction::getActionType(FuncCode action) const
{
	info_map::const_iterator const it = lyx_info_map.find(action);
	return it != lyx_info_map.end() ? it->second.type : Hidden;
}


bool LyXAction::funcHasFlag(FuncCode action,
			    LyXAction::func_attrib flag) const
{
	info_map::const_iterator ici = lyx_info_map.find(action);

	if (ici == lyx_info_map.end()) {
		LYXERR0("action: " << action << " is not known.");
		LASSERT(false, /**/);
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
