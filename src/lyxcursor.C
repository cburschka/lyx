/**
 * \file lyxcursor.C
 * Copyright 1995-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Matthias Ettrich
 */

#include <config.h>

#include "lyxcursor.h"


LyXCursor::LyXCursor()
	: par_(), pos_(0), boundary_(false),
	  x_(0), ix_(0), x_fix_(0), y_(0), iy_(0), row_(0)
{}


void LyXCursor::par(ParagraphList::iterator pit)
{
	par_ = pit;
}


ParagraphList::iterator LyXCursor::par() const
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


void LyXCursor::ix(int n)
{
	ix_ = n;
}

int LyXCursor::ix() const
{
	return ix_;
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


void LyXCursor::iy(int i)
{
	iy_ = i;
}


int LyXCursor::iy() const
{
	return iy_;
}


void LyXCursor::row(RowList::iterator r)
{
	row_ = r;
}


RowList::iterator LyXCursor::row() const
{
	return row_;
}


void LyXCursor::irow(RowList::iterator r)
{
	irow_ = r;
}


RowList::iterator LyXCursor::irow() const
{
	return irow_;
}


bool operator==(LyXCursor const & a, LyXCursor const & b)
{
	return a.par() == b.par()
	    && a.pos() == b.pos()
	    && a.boundary() == b.boundary();
}


bool operator!=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a == b);
}

