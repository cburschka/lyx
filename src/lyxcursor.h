// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXCURSOR_H
#define LYXCURSOR_H

#include "lyxparagraph.h"

struct Row;

/** All these variavles should be explained. Matthias?
 */
struct LyXCursor {
	///
	LyXParagraph * par;
	///
	LyXParagraph::size_type pos;
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
