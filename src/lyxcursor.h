// -*- C++ -*-
/**
 * \file lyxcursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXCURSOR_H
#define LYXCURSOR_H

#include "support/types.h"

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
	void par(lyx::paroffset_type pit);
	/// return the paragraph this cursor is in
	lyx::paroffset_type par() const;
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
	/// set the y position in pixels
	void y(int i);
	/// return the y position in pixels
	int y() const;

private:
	/// The paragraph the cursor is in.
	lyx::paroffset_type par_;
	/// The position inside the paragraph
	lyx::pos_type pos_;
	/**
	 * When the cursor position is i, is the cursor is after the i-th char
	 * or before the i+1-th char ? Normally, these two interpretations are
	 * equivalent, except when the fonts of the i-th and i+1-th char
	 * differ.
	 * We use boundary_ to distinguish between the two options:
	 * If boundary_=true, then the cursor is after the i-th char
	 * and if boundary_=false, then the cursor is before the i+1-th char.
	 *
	 * We currently use the boundary only when the language direction of
	 * the i-th char is different than the one of the i+1-th char.
	 * In this case it is important to distinguish between the two
	 * cursor interpretations, in order to give a reasonable behavior to
	 * the user.
	 */
	bool boundary_;
	/// the pixel x position
	int x_;
	/// the pixel y position
	int y_;
};

///
bool operator==(LyXCursor const & a, LyXCursor const & b);
///
bool operator!=(LyXCursor const & a, LyXCursor const & b);
///
bool operator<(LyXCursor const & a, LyXCursor const & b);

#endif // LYXCURSOR_H
