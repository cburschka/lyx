// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/

#ifndef _LYXROW_H
#define _LYXROW_H

//#include "lyxparagraph.h"

class LyXParagraph;

///
struct Row {
	///
	LyXParagraph *par;
	///
	int pos;
	///
	unsigned short  baseline;
	///
	/** what is missing to a full row can be negative.
	  Needed for hfills, flushright, block etc. */
	int fill;
	///
	unsigned short  height;
	///
	unsigned short ascent_of_text;
	
	///
	Row* next;
	///
	Row* previous;
};

#endif
