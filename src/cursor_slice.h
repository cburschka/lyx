// -*- C++ -*-
/**
 * \file cursor_slice.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CURSORSLICE_H
#define CURSORSLICE_H

#include <iosfwd>
#include <cstddef>

class InsetBase;
class UpdatableInset;
class MathInset;
class LyXText;
class MathArray;


/// This encapsulates a single slice of a document iterator as used e.g.
/// for cursors.

// After IU, the distinction of MathInset and UpdatableInset as well as
// that of MathArray and LyXText should vanish. They are conceptually the
// same (now...)

class CursorSlice {
public:
	/// type for cell number in inset
	typedef size_t idx_type;
	/// type for paragraph numbers positions within a cell
	typedef size_t par_type;
	/// type for cursor positions within a cell
	typedef size_t pos_type;

	///
	CursorSlice();
	///
	explicit CursorSlice(InsetBase *);

	///
	/// texted specific stuff
	///
	///
	LyXText * text() const;
	///
	UpdatableInset * asUpdatableInset() const;

	///
	/// mathed specific stuff
	///
	/// returns cell corresponding to this position
	MathArray & cell() const;
	/// returns cell corresponding to this position
	MathArray & cell(idx_type idx) const;
	/// gets screen position of the thing
	void getPos(int & x, int & y) const;
	/// set position
	void setPos(int pos);
	///
	MathInset * asMathInset() const;

	///
	friend std::ostream & operator<<(std::ostream &, CursorSlice const &);
public:
	/// pointer to an inset
	InsetBase * inset_;
	/// cell index of a position in this inset
	idx_type idx_;
	/// paragraph in this cell (used by texted)
	par_type par_;
	/// position in this cell
	pos_type pos_;
};

/// test for equality
bool operator==(CursorSlice const &, CursorSlice const &);
/// test for inequality
bool operator!=(CursorSlice const &, CursorSlice const &);
/// test for order
bool operator<(CursorSlice const &, CursorSlice const &);

#endif
