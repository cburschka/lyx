// -*- C++ -*-
/**
 * \file CutAndPaste.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CUTANDPASTE_H
#define CUTANDPASTE_H

#include "ParagraphList_fwd.h"

#include "support/types.h"

#include <string>
#include <vector>

class Buffer;
class ErrorList;
class InsetText;
class LyXTextClass;
class LCursor;

///
namespace lyx {
namespace cap {

///
std::vector<std::string> const availableSelections(Buffer const & buffer);
///
lyx::size_type numberOfSelections();
///
std::string getSelection(Buffer const & buffer, size_t sel_index);

///
void cutSelection(LCursor & cur, bool doclear, bool realcut);

/* Replace using the font of the first selected character and select
 * the new string. When \c backwards == false, set anchor before
 * cursor; otherwise set cursor before anchor.
 */
void replaceSelectionWithString(LCursor & cur, std::string const & str, 
				bool backwards);
/// replace selection helper
void replaceSelection(LCursor & cur);

///
void cutSelection(LCursor & cur, bool doclear = true, bool realcut = true);
///
void copySelection(LCursor & cur);
///
void pasteSelection(LCursor & cur, ErrorList &, size_t sel_index = 0);

///
void pasteParagraphList(LCursor & cur, ParagraphList const & parlist,
			textclass_type textclass, ErrorList & errorList);


/** Needed to switch between different classes. This works
 *  for a list of paragraphs beginning with the specified par.
 *  It changes layouts and character styles.
 */
void switchBetweenClasses(lyx::textclass_type c1,
                          lyx::textclass_type c2,
                          InsetText & in, ErrorList &);

///
std::string grabSelection(LCursor const & cur);
///
void eraseSelection(LCursor & cur);
///
std::string grabAndEraseSelection(LCursor & cur);
// other selection methods
///
void selCut(LCursor & cur);
///
void selDel(LCursor & cur);
/// clears or deletes selection depending on lyxrc setting
void selClearOrDel(LCursor & cur);
/// pastes n-th element of cut buffer
void selPaste(LCursor & cur, size_t n);

/** Tabular has its own paste stack for multiple cells
 *  but it needs to know whether there is a more recent
 *  ordinary paste. Therefore which one is newer.
 */
//FIXME: this is a workaround for bug 1919. Replace this by
//an all-for-one-paste mechanism in 1.5
/// store whether tabular or ordinary paste stack is newer
void dirtyTabularStack(bool b);
/// is the tabular paste stack newer than the ordinary one?
bool tabularStackDirty();
} // namespace cap
} // namespce lyx

#endif
