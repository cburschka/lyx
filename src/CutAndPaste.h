// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995-2000 the LyX Team.
 *
 * ====================================================== */

#ifndef CUTANDPASTE_H
#define CUTANDPASTE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "layout.h"

class LyXParagraph;

///
class CutAndPaste {
public:
	///
	static
	bool cutSelection(LyXParagraph * startpar, LyXParagraph ** endpar,
			  int start, int & end,
			  char tc, bool doclear = false);
	///
	static
	bool copySelection(LyXParagraph * startpar, LyXParagraph * endpar,
			   int start, int end, char tc);
	///
	static
	bool pasteSelection(LyXParagraph ** par, LyXParagraph ** endpar,
			    int & pos, char tc);
	///
	static
	int nrOfParagraphs();
	/** needed to switch between different classes this works
	    for a list of paragraphs beginning with the specified par 
	    return value is the number of wrong conversions
	*/
	static
	int SwitchLayoutsBetweenClasses(LyXTextClassList::size_type class1,
					LyXTextClassList::size_type class2,
					LyXParagraph * par);
	///
	static
	bool checkPastePossible(LyXParagraph *);
};

#endif
