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

#include "support/std_string.h"
#include <vector>

class Buffer;
class BufferParams;
class ErrorList;
class LyXTextClass;
class Paragraph;

///
namespace CutAndPaste {

///
std::vector<string>
CutAndPaste::availableSelections(Buffer const & buffer);

///
PitPosPair cutSelection(BufferParams const & params,
			ParagraphList & pars,
			ParagraphList::iterator startpit,
			ParagraphList::iterator endpit,
			int start, int end, lyx::textclass_type tc,
			bool doclear = false);
///
PitPosPair eraseSelection(BufferParams const & params,
			  ParagraphList & pars,
			  ParagraphList::iterator startpit,
			  ParagraphList::iterator endpit,
			  int start, int end, bool doclear = false);
///
bool copySelection(ParagraphList::iterator startpit,
		   ParagraphList::iterator endpit,
		   int start, int end, lyx::textclass_type tc);
///
std::pair<PitPosPair, ParagraphList::iterator>
pasteSelection(Buffer const & buffer,
	       ParagraphList & pars,
	       ParagraphList::iterator pit, int pos,
	       lyx::textclass_type tc, ErrorList &);

///
std::pair<PitPosPair, ParagraphList::iterator>
pasteSelection(Buffer const & buffer,
	       ParagraphList & pars,
	       ParagraphList::iterator pit, int pos,
	       lyx::textclass_type tc,
	       size_t cuts_indexm, ErrorList &);

///
int nrOfParagraphs();

/** Needed to switch between different classes this works
    for a list of paragraphs beginning with the specified par
    return value is the number of wrong conversions.
*/
int SwitchLayoutsBetweenClasses(lyx::textclass_type c1,
				lyx::textclass_type c2,
				ParagraphList & par,
				ErrorList &);
///
bool checkPastePossible();

} // end of CutAndPaste

#endif
