// -*- C++ -*-
/**
 * \file lyxcursor.h
 * Copyright 1995-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Matthias Ettrich
 */

#ifndef LYXCURSOR_H
#define LYXCURSOR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "support/types.h"

class Paragraph;
class Row;

/** 
 * The cursor class describes the position of a cursor within a document.
 * Several cursors exist within LyX; for example, when locking an inset,
 * the position of the cursor in the containing inset is stored.
 *
 * FIXME: true ?
 */
class LyXCursor {
public:
	LyXCursor();
	/// set the paragraph that contains this cursor
	void par(Paragraph * p);
	/// return the paragraph this cursor is in
	Paragraph * par() const;
	/// set the position within the paragraph
	void pos(lyx::pos_type p);
	/// return the position within the paragraph
	lyx::pos_type pos() const;
	/// FIXME
	void boundary(bool b);
	/// FIXME
	bool boundary() const;
	/// set the x position in pixels
	void x(int i);
	/// return the x position in pixels
	int x() const;
	/// set the stored next-line position when at the end of a row
	void ix(int i);
	/**
	 * Return the x position of the start of the next row, when this
	 * cursor is at the end of the previous row, for insets that take
	 * a full row.
	 *
	 * FIXME: explain why we need this ?
	 */
	int ix() const;
	/// set the cached x position
	void x_fix(int i);
	/**
	 * Return the cached x position of the cursor. This is used for when
	 * we have text like :
	 *
	 * blah blah blah blah| blah blah blah
	 * blah blah blah
	 * blah blah blah blah blah blah
	 *
	 * When we move onto row 3, we would like to be vertically aligned
	 * with where we were in row 1, despite the fact that row 2 is 
	 * shorter than x()
	 */
	int x_fix() const;
	/// set the y position in pixels
	void y(int i);
	/// return the y position in pixels
	int y() const;
	/// set the stored next-line y position when at the end of a row
	void iy(int i);
	/**
	 * Return the y position of the start of the next row, when this
	 * cursor is at the end of the previous row, for insets that take
	 * a full row.
	 *
	 * FIXME: explain why we need this ? especially for y...
	 */
	int iy() const;
	/// set the row of the paragraph the cursor is in
	void row(Row * r);
	/// return the row of the paragraph this cursor is in
	Row * row() const;
	/// set the stored next row
	void irow(Row * r);
	/**
	 * Return the next row, when this
	 * cursor is at the end of the previous row, for insets that take
	 * a full row.
	 *
	 * FIXME: explain why we need this ? especially for y...
	 */
	Row * irow() const;
private:
	/// The paragraph the cursor is in.
	Paragraph * par_;
	/// The position inside the paragraph
	lyx::pos_type pos_;
	/// FIXME
	bool boundary_;
	/// the pixel x position
	int x_;
	/// the stored next-row x position 
	int ix_;
	/// the cached x position
	int x_fix_;
	/// the pixel y position
	int y_;
	/// the stored next-row y position
	int iy_;
	/// the containing row
	Row * row_;
	/// the containing row for the next line 
	Row * irow_;
};

/// these three dictate the others
inline
bool operator==(LyXCursor const & a, LyXCursor const & b)
{
	return (a.par() == b.par())
		&& (a.pos() == b.pos())
		&& a.boundary() == b.boundary();
}

inline
bool operator!=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a == b);
}

/// only compares y() and pos(). Can this be done in another way?
inline
bool operator<(LyXCursor const & a, LyXCursor const & b)
{
	return (a.y() < b.y() && a.pos() < b.pos());
}

inline
bool operator>(LyXCursor const & a, LyXCursor const & b)
{
	return b < a;
}

inline
bool operator>=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a < b);
}


inline
bool operator<=(LyXCursor const & a, LyXCursor const & b)
{
	return !(a > b);
}

#endif // LYXCURSOR_H
