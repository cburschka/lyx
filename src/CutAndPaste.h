// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 the LyX Team.
 *
 * ====================================================== */

#ifndef CUTANDPASTE_H
#define CUTANDPASTE_H

#include "support/types.h"

class Paragraph;
class BufferParams;
class LyXTextClass;

///
namespace CutAndPaste {

/// realcut == false is we actually want a delete
bool cutSelection(Paragraph * startpar, Paragraph * endpar,
		  int start, int & end, lyx::textclass_type tc,
		  bool doclear = false, bool realcut = true);

///
bool copySelection(Paragraph * startpar, Paragraph * endpar,
		   int start, int end, lyx::textclass_type tc);
///
bool pasteSelection(Paragraph ** par, Paragraph ** endpar,
		    int & pos, lyx::textclass_type tc);

///
int nrOfParagraphs();

/** needed to switch between different classes this works
    for a list of paragraphs beginning with the specified par
    return value is the number of wrong conversions
*/
int SwitchLayoutsBetweenClasses(lyx::textclass_type c1,
				lyx::textclass_type c2,
				Paragraph * par,
				BufferParams const & bparams);
///
bool checkPastePossible();

} // end of CutAndPaste

#endif
