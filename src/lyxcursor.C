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

#include "lyxcursor.h"


LyXCursor::LyXCursor()
	: par_(0), pos_(0), boundary_(false),
	  x_(0), x_fix_(0), y_(0), row_(0)
{}


void LyXCursor::par(Paragraph * p)
{
	par_ = p;
}


Paragraph * LyXCursor::par() const
{
	return par_;
}


void LyXCursor::pos(lyx::pos_type p)
{
	pos_ = p;
}


lyx::pos_type LyXCursor::pos() const
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


void LyXCursor::y(int i)
{
	y_ = i;
}


int LyXCursor::y() const
{
	return y_;
}


void LyXCursor::row(Row * r)
{
	row_ = r;
}


Row * LyXCursor::row() const
{
	return row_;
}
