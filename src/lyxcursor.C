/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include "lyxcursor.h"


void LyXCursor::par(LyXParagraph * p) 
{
	par_ = p;
}


LyXParagraph * LyXCursor::par()
{
	return par_;
}


LyXParagraph * LyXCursor::par() const
{
	return par_;
}


void LyXCursor::pos(LyXParagraph::size_type p)
{
	pos_ = p;
}


LyXParagraph::size_type LyXCursor::pos() const
{
	return pos_;
}


void LyXCursor::boundary(bool b) 
{
	boundary_ = b;
}


bool LyXCursor::boundary() const
{
	return boundary_;
}


void LyXCursor::x(int n)
{
	x_ = n;
}

int LyXCursor::x() const
{
	return x_;
}


void LyXCursor::x_fix(int i)
{
	x_fix_ = i;
}


int LyXCursor::x_fix() const
{
	return x_fix_;
}


void LyXCursor::y(unsigned long i)
{
	y_ = i;
}


unsigned long LyXCursor::y() const
{
	return y_;
}


void LyXCursor::row(Row * r)
{
	row_ = r;
}


Row * LyXCursor::row()
{
	return row_;
}


Row * LyXCursor::row() const
{
	return row_;
}
