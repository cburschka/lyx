/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxrow.h"


Row::Row()
	: par_(0), pos_(0), fill_(0), height_(0), width_(0),
	  ascent_of_text_(0), baseline_(0), next_(0), previous_(0)
{}


void Row::par(Paragraph * p)
{
	par_ = p;
}


void Row::pos(pos_type p)
{
	pos_ = p;
}


Row::pos_type Row::pos() const
{
	return pos_;
}


void Row::fill(int f)
{
	fill_ = f;
}


int Row::fill() const
{
	return fill_;
}


void Row::height(unsigned short h)
{
	height_ = h;
}


void Row::width(unsigned int w)
{
	width_ = w;
}


unsigned int Row::width() const
{
	return width_;
}


void Row::ascent_of_text(unsigned short a)
{
	ascent_of_text_ = a;
}


unsigned short Row::ascent_of_text() const
{
	return ascent_of_text_;
}


void Row::baseline(unsigned int b)
{
	baseline_ = b;
}


unsigned int Row::baseline() const
{
	return baseline_;
}


void Row::next(Row * r)
{
	next_ = r;
}


void Row::previous(Row * r)
{
	previous_ = r;
}


Row * Row::previous() const
{
	return previous_;
}
