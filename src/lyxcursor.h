// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/
#ifndef _LYXCURSOR_H
#define _LYXCURSOR_H

class LyXParagraph;
struct Row;

/** All these variavles should be explained. Matthias?
 */
struct LyXCursor {
	///
	LyXParagraph *par;
	///
	int pos;
	///
	int x;
	///
	int x_fix;
	///
	long y;
	///
	Row *row;
};

#endif
