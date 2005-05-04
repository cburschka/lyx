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

#include "support/types.h"

#include <string>
#include <vector>

class Buffer;
class ErrorList;
class LyXTextClass;
class LCursor;
class ParagraphList;

///
namespace lyx {
namespace cap {

///
std::vector<std::string> const availableSelections(Buffer const & buffer);
///
std::string getSelection(Buffer const & buffer, size_t sel_index);

///
void cutSelection(LCursor & cur, bool doclear, bool realcut);

/**
 * Sets the selection from the current cursor position to length
 * characters to the right. No safety checks.
 */
void setSelectionRange(LCursor & cur, lyx::pos_type length);
/// simply replace using the font of the first selected character
void replaceSelectionWithString(LCursor & cur, std::string const & str);
/// replace selection helper
void replaceSelection(LCursor & cur);

///
void cutSelection(LCursor & cur, bool doclear = true, bool realcut = true);
///
void copySelection(LCursor & cur);
///
void pasteSelection(LCursor & cur, size_t sel_index = 0);

/** Needed to switch between different classes. This works
 *  for a list of paragraphs beginning with the specified par.
 *  It changes layouts and character styles.
 */
void SwitchBetweenClasses(lyx::textclass_type c1,
				lyx::textclass_type c2,
				ParagraphList & par,
				ErrorList &);

// only used by the spellchecker
void replaceWord(LCursor & cur, std::string const & replacestring);

///
std::string grabSelection(LCursor & cur);
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
} // namespace cap
} // namespce lyx

#endif
