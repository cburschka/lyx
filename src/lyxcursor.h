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

#ifndef LYXCURSOR_H
#define LYXCURSOR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "support/types.h"

class Paragraph;
class Row;

/** All these variables should be explained. Matthias?
 */
class LyXCursor {
public:
	///
	LyXCursor();
	///
	void par(Paragraph * p);
	///
	Paragraph * par() const;
	///
	void pos(lyx::pos_type p);
	///
	lyx::pos_type pos() const;
	///
	void boundary(bool b);
	///
	bool boundary() const;
	///
	void x(int i);
	///
	int x() const;
	///
	void ix(int i);
	///
	int ix() const;
	///
	void x_fix(int i);
	///
	int x_fix() const;
	///
	void y(int i);
	///
	int y() const;
	///
	void iy(int i);
	///
	int iy() const;
	///
	void row(Row * r);
	///
	Row * row() const;
	///
	void irow(Row * r);
	///
	Row * irow() const;
private:
	/// The paragraph the cursor is in.
	Paragraph * par_;
	/// The position inside the paragraph
	lyx::pos_type pos_;
	///
	bool boundary_;
	///
	int x_;
	/// the x position of the position before the inset when we put
	/// the cursor on the end of the row before, otherwise equal to x.
	int ix_;
	///
	int x_fix_;
	///
	int y_;
	/// the y position of the position before the inset when we put
	/// the cursor on the end of the row before, otherwise equal to y.
	int iy_;
	///
	Row * row_;
	/// the row of the position before the inset when we put
	/// the cursor on the end of the row before, otherwise equal to row.
	Row * irow_;
};

///
inline
bool operator==(LyXCursor const & a, LyXCursor const & b)
{
	return (a.par() == b.par())
		&& (a.pos() == b.pos())
		&& a.boundary() == b.boundary();
}

///
inline
bool operator!=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a == b);
}

///
inline
bool operator<(LyXCursor const & a, LyXCursor const & b)
{
	// Can this be done in a nother way?
	return (a.y() < b.y() && a.pos() < b.pos());
}

///
inline
bool operator>(LyXCursor const & a, LyXCursor const & b)
{
	return b < a;
}

///
inline
bool operator>=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a < b);
}


///
inline
bool operator<=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a > b);
}

#endif
