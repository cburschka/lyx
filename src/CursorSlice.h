// -*- C++ -*-
/**
 * \file CursorSlice.h
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

#include "support/types.h"
#include "insets/Inset.h"

#include <boost/signal.hpp>
#include <boost/signals/trackable.hpp>

#include <cstddef>
#include <iosfwd>


namespace lyx {

class Inset;
class MathData;
class Text;
class Paragraph;

/// This encapsulates a single slice of a document iterator as used e.g.
/// for cursors.

// After IU, the distinction of MathInset and InsetOld as well as
// that of MathData and Text should vanish. They are conceptually the
// same (now...)

class CursorSlice : public boost::signals::trackable {
public:
	/// Those needs inset_ access.
	///@{
	friend class DocIterator;
	friend class StableDocIterator;
	///@}

	/// type for cell number in inset
	typedef size_t idx_type;
	/// type for row indices
	typedef size_t row_type;
	/// type for col indices
	typedef size_t col_type;

	///
	CursorSlice();
	///
	CursorSlice(CursorSlice const &);
	///
	explicit CursorSlice(Inset &);
	///
	virtual ~CursorSlice();
	///
	CursorSlice & operator=(CursorSlice const &);
	///
	bool isValid() const;

	/// the current inset
	Inset & inset() const { return *inset_; }
	/// return the cell this cursor is in
	idx_type idx() const { return idx_; }
	/// return the cell this cursor is in
	idx_type & idx() { return idx_; }
	/// return the last cell in this inset
	idx_type lastidx() const { return nargs() - 1; }
	/// return the offset of the paragraph this cursor is in
	pit_type pit() const { return pit_; }
	/// set the offset of the paragraph this cursor is in
	pit_type & pit() { return pit_; }
	/// increments the paragraph this cursor is in
	void incrementPar();
	/// decrements the paragraph this cursor is in
	void decrementPar();
	/// return the position within the paragraph
	pos_type pos() const { return pos_; }
	/// return the position within the paragraph
	pos_type & pos() { return pos_; }
	/// return the last position within the paragraph
	pos_type lastpos() const;
	/// return the number of embedded cells
	size_t nargs() const { return inset_->nargs(); }
	/// return the number of columns (1 in non-grid-like insets)
	size_t ncols() const { return inset_->ncols(); }
	/// return the number of rows (1 in non-grid-like insets)
	size_t nrows() const { return inset_->nrows(); }
	/*!
	 * \return the grid row of the current cell.
	 * This does only make sense in grid like insets.
	 */
	row_type row() const;
	/*!
	 * \return the grid column of the current cell.
	 * This does only make sense in grid like insets.
	 */
	col_type col() const;

	///
	/// texted specific stuff
	///
	/// returns text corresponding to this position
	Text * text() { return inset_->getText(idx_); }
	/// returns text corresponding to this position
	Text const * text() const { return inset_->getText(idx_); }
	/// paragraph in this cell
	Paragraph & paragraph();
	/// paragraph in this cell
	Paragraph const & paragraph() const;

	///
	/// mathed specific stuff
	///
	/// returns the owning inset if it is a InsetMath, else 0
	InsetMath * asInsetMath() const { return inset_->asInsetMath(); }
	/// returns cell corresponding to this position
	MathData & cell() const;

	/// write some debug information to \p os
	friend std::ostream & operator<<(std::ostream &, CursorSlice const &);
private:
	///
	void invalidate();

	/// pointer to 'owning' inset. This is some kind of cache.
	Inset * inset_;

	/*!
	 * Cell index of a position in this inset.
	 * This is the primary cell information also for grid like insets,
	 * although we have the convenience functions row() and col() for
	 * those * and column changes every time the number of columns ornumber
	 * of rows changes. Normally the cursor should stay in the same cell,
	 * so these changes should typically be performed like the following:
	 * \code
	 * row_type const r = cur.row();
	 * col_type const c = cur.col();
	 * // change nrows() and/or ncols()
	 * cur.idx = index(r, c);
	 * \endcode
	 */
	idx_type idx_;
	/// paragraph in this cell (used by texted)
	pit_type pit_;
	/// true if 'pit' was properly initialized
	bool pit_valid_;
	/// position in this cell
	pos_type pos_;
	/// connection to referred \c inset_ destruction signal.
	boost::signals::connection inset_connection_;
};

/// test for equality
bool operator==(CursorSlice const &, CursorSlice const &);
/// test for inequality
bool operator!=(CursorSlice const &, CursorSlice const &);
/// test for order
bool operator<(CursorSlice const &, CursorSlice const &);
/// test for order
bool operator>(CursorSlice const &, CursorSlice const &);
/// test for order
bool operator<=(CursorSlice const &, CursorSlice const &);


} // namespace lyx

#endif
