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
    CutAndPaste();
    ~CutAndPaste();
    bool cutSelection(LyXParagraph *startpar, LyXParagraph **endpar,
		      int start, int & end, char tc, bool doclear=false);
    bool copySelection(LyXParagraph *startpar, LyXParagraph *endpar,
		       int start, int end, char tc);
    bool pasteSelection(LyXParagraph **par, LyXParagraph **endpar,
			int & pos, char tc);
    int nrOfParagraphs() const;
    /** needed to switch between different classes this works
	for a list of paragraphs beginning with the specified par 
	return value is the number of wrong conversions
    */ 
    int SwitchLayoutsBetweenClasses(LyXTextClassList::size_type class1,
				    LyXTextClassList::size_type class2,
				    LyXParagraph * par);
    char getBufferTextClass();
    bool checkPastePossible(LyXParagraph *, int pos);

private:
    ///
    void DeleteBuffer();
    ///
    char textclass;

};

#endif
