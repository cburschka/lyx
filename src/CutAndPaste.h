// -*- C++ -*-
/* \file CutAndPaste.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jurgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CUTANDPASTE_H
#define CUTANDPASTE_H

#include "support/types.h"
#include "ParagraphList.h"

class Paragraph;
class BufferParams;
class LyXTextClass;

///
namespace CutAndPaste {
///
PitPosPair cutSelection(ParagraphList & pars,
	     ParagraphList::iterator startpit,
	     ParagraphList::iterator endpit,
	     int start, int end, lyx::textclass_type tc,
	     bool doclear = false);
///
PitPosPair eraseSelection(ParagraphList & pars,
	       ParagraphList::iterator startpit,
	       ParagraphList::iterator endpit,
	       int start, int end, bool doclear = false);
///
bool copySelection(ParagraphList::iterator startpit,
		   ParagraphList::iterator endpit,
		   int start, int end, lyx::textclass_type tc);
///
std::pair<PitPosPair, ParagraphList::iterator>
pasteSelection(ParagraphList & pars,
	       ParagraphList::iterator pit, int pos,
	       lyx::textclass_type tc);

///
std::pair<PitPosPair, ParagraphList::iterator>
pasteSelection(ParagraphList & pars,
	       ParagraphList::iterator pit, int pos,
	       lyx::textclass_type tc,
	       size_t cuts_index);

///
int nrOfParagraphs();

/** Needed to switch between different classes this works
    for a list of paragraphs beginning with the specified par
    return value is the number of wrong conversions.
*/
int SwitchLayoutsBetweenClasses(lyx::textclass_type c1,
				lyx::textclass_type c2,
				ParagraphList & par);
///
bool checkPastePossible();

} // end of CutAndPaste

#endif
