// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXROW_H
#define LYXROW_H

#ifdef __GNUG__
#pragma interface
#endif

#include "paragraph.h"

///
class Row {
public:
	///
	Row();
	///
	void par(Paragraph * p);
	///
	Paragraph * par();
	///
	Paragraph * par() const;
	///
	void pos(Paragraph::size_type p);
	///
	Paragraph::size_type pos() const;
	///
	void fill(int f);
	///
	int fill() const;
	///
	void height(unsigned short h);
	///
	unsigned short height() const;
	///
	void width(unsigned int w);
	///
	unsigned int width() const;
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
	Paragraph * par_;
	///
	Paragraph::size_type pos_;
	/** what is missing to a full row can be negative.
	  Needed for hfills, flushright, block etc. */
	mutable int fill_;
	///
	unsigned short height_;
	///
	unsigned int width_;
	///
	unsigned short ascent_of_text_;
	///
	unsigned int baseline_;
	///
	Row * next_;
	///
	Row * previous_;
};


inline
Paragraph * Row::par()
{
	return par_;
}


inline
Paragraph * Row::par() const
{
	return par_;
}


inline
unsigned short Row::height() const
{
	return height_;
}


inline
Row * Row::next() const
{
	return next_;
}

#endif
