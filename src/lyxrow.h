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
	LyXParagraph * par;
	///
#ifdef NEW_TEXT
	LyXParagraph::size_type pos;
#else
	int pos;
#endif
	///
	unsigned short  baseline;
	/** what is missing to a full row can be negative.
	  Needed for hfills, flushright, block etc. */
	int fill;
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
