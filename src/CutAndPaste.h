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

#ifdef __GNUG__
#pragma interface
#endif

#include "layout.h"

class Paragraph;

///
class CutAndPaste {
public:
	///
	static
	bool cutSelection(Paragraph * startpar, Paragraph ** endpar,
                      int start, int & end, char tc, bool doclear = false);
	///
	static
	bool copySelection(Paragraph * startpar, Paragraph * endpar,
                       int start, int end, char tc);
	///
	static
	bool pasteSelection(Paragraph ** par, Paragraph ** endpar,
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
                                    Paragraph * par);
	///
	static
	bool checkPastePossible(Paragraph *);
};

#endif
