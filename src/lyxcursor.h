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

#include "paragraph.h"

struct Row;

/** All these variables should be explained. Matthias?
 */
class LyXCursor {
public:
	LyXCursor();
	///
	void par(Paragraph * p);
	///
	//Paragraph * par();
	///
	Paragraph * par() const;
	///
	void pos(Paragraph::size_type p);
	///
	Paragraph::size_type pos() const;
	///
	void boundary(bool b);
	///
	bool boundary() const;
	///
	void x(int i);
	///
	int x() const;
	///
	void x_fix(int i);
	///
	int x_fix() const;
	///
	void y(int i);
	///
	int y() const;
	///
	void row(Row * r);
	///
	//Row * row();
	///
	Row * row() const;
private:
	/// The paragraph the cursor is in.
	Paragraph * par_;
	/// The position inside the paragraph
	Paragraph::size_type pos_;
	///
	bool boundary_;
	///
	int x_;
	///
	int x_fix_;
	///
	int y_;
	///
	Row * row_;
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
