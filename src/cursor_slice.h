// -*- C++ -*-
/**
 * \file cursor_slice.h
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

#ifndef CURSORSLICE_H
#define CURSORSLICE_H

#include "ParagraphList_fwd.h"

#include "support/types.h"

#include <cstddef>
#include <iosfwd>

class BufferView;
class InsetBase;
class MathInset;
class MathArray;
class LyXText;
class Paragraph;
class UpdatableInset;


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
	typedef lyx::paroffset_type par_type;
	/// type for cursor positions within a cell
	typedef lyx::pos_type pos_type;
	/// type for row indices
	typedef size_t row_type;
	/// type for col indices
	typedef size_t col_type;

	///
	CursorSlice();
	///
	explicit CursorSlice(InsetBase &);

	/// the current inset
	InsetBase & inset() const { return *inset_; }
	/// return the cell this cursor is in
	idx_type idx() const;
	/// return the cell this cursor is in
	idx_type & idx();
	/// return the last cell in this inset
	idx_type lastidx() const { return nargs() - 1; }
	/// return the paragraph this cursor is in
	par_type par() const;
	/// set the paragraph this cursor is in
	par_type & par();
	/// increments the paragraph this cursor is in
	void incrementPar();
	/// increments the paragraph this cursor is in
	void decrementPar();
	/// return the position within the paragraph
	pos_type pos() const;
	/// return the position within the paragraph
	pos_type & pos();
	/// return the last position within the paragraph
	pos_type lastpos() const;
	/// return the number of embedded cells
	size_t nargs() const;
	/// return the number of embedded cells
	size_t ncols() const;
	/// return the number of embedded cells
	size_t nrows() const;
	/// return the grid row of the current cell
	row_type row() const;
	/// return the grid row of the current cell
	col_type col() const;

	///
	/// texted specific stuff
	///
	/// see comment for the member
	bool boundary() const;
	/// see comment for the member
	bool & boundary();
	///
	LyXText * text() const;
	///
	UpdatableInset * asUpdatableInset() const;
	///
	Paragraph & paragraph();
	///
	Paragraph const & paragraph() const;

	///
	/// mathed specific stuff
	///
	/// returns cell corresponding to this position
	MathArray & cell() const;
	///
	MathInset * asMathInset() const;

	///
	friend std::ostream & operator<<(std::ostream &, CursorSlice const &);
public:
	/// pointer to 'owning' inset. This is some kind of cache.
	InsetBase * inset_;
private:
	/// cell index of a position in this inset
	idx_type idx_;
	/// paragraph in this cell (used by texted)
	par_type par_;
	/// true of 'pit' was properly initialized
	bool pit_valid_;
	/// position in this cell
	pos_type pos_;
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
};

/// test for equality
bool operator==(CursorSlice const &, CursorSlice const &);
/// test for inequality
bool operator!=(CursorSlice const &, CursorSlice const &);
/// test for order
bool operator<(CursorSlice const &, CursorSlice const &);
/// test for order
bool operator>(CursorSlice const &, CursorSlice const &);

#endif
