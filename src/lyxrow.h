// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXROW_H
#define LYXROW_H

#include "lyxparagraph.h"

///
struct Row {
	///
	Row()
		: par(0), pos(0), baseline(0), fill(0), height(0),
		  ascent_of_text(0), next(0), previous(0)
		{}
	///
	LyXParagraph * par;
	///
	LyXParagraph::size_type pos;
	///
	unsigned short  baseline;
	/** what is missing to a full row can be negative.
	  Needed for hfills, flushright, block etc. */
	mutable int fill;
	///
	unsigned short  height;
	///
	unsigned short ascent_of_text;
	
	///
	Row * next;
	///
	Row * previous;
};

#endif
