// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/

#ifndef LYXCURSOR_H
#define LYXCURSOR_H

#include "lyxparagraph.h"

struct Row;

/** All these variavles should be explained. Matthias?
 */
struct LyXCursor {
	///
	LyXParagraph * par;
#ifdef NEW_TEXT
	///
	LyXParagraph::size_type pos;
#else
	///
	int pos;
#endif
	///
	int x;
	///
	int x_fix;
	///
	long y;
	///
	Row * row;
};

#endif
