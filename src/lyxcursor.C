/**
 * \file lyxcursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxcursor.h"


LyXCursor::LyXCursor()
	: par_(0), pos_(0), boundary_(false)
{}


void LyXCursor::par(lyx::paroffset_type par)
{
	par_ = par;
}


lyx::paroffset_type LyXCursor::par() const
{
	return par_;
}


void LyXCursor::pos(lyx::pos_type pos)
{
	pos_ = pos;
}


lyx::pos_type LyXCursor::pos() const
{
	return pos_;
}


void LyXCursor::boundary(bool boundary)
{
	boundary_ = boundary;
}


bool LyXCursor::boundary() const
{
	return boundary_;
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


bool operator<(LyXCursor const & a, LyXCursor const & b)
{
	return (a.par() < b.par() ||
		(a.par() == b.par()  && a.pos() < b.pos()));
}
