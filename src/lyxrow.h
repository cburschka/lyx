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

#ifndef LYXROW_H
#define LYXROW_H

#include "lyxparagraph.h"

///
class Row {
public:
	///
	Row();
	///
	void par(LyXParagraph * p);
	///
	LyXParagraph * par();
	///
	LyXParagraph * par() const;
	///
	void pos(LyXParagraph::size_type p);
	///
	LyXParagraph::size_type pos() const;
	///
	void fill(int f);
	///
	int fill() const;
	///
	void height(unsigned short h);
	///
	unsigned short height() const;
	///
	void ascent_of_text(unsigned short a);
	///
	unsigned short ascent_of_text() const;
	///
	void baseline(unsigned int b);
	///
	unsigned int baseline() const;
	///
	void next(Row * r);
	///
	Row * next() const;
	///
	void previous(Row * r);
	///
	Row * previous() const;
private:
	///
	LyXParagraph * par_;
	///
	LyXParagraph::size_type pos_;
	/** what is missing to a full row can be negative.
	  Needed for hfills, flushright, block etc. */
	mutable int fill_;
	///
	unsigned short height_;
	///
	unsigned short ascent_of_text_;
	///
	unsigned int baseline_;
	///
	Row * next_;
	///
	Row * previous_;
};

#endif
