// -*- C++ -*-
/**
 * \file dociterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DOCITERATOR_H
#define DOCITERATOR_H

#include "cursor_slice.h"

#include <vector>
#include <iosfwd>

class LyXText;
class MathAtom;
class Paragraph;
class Row;



// only needed for gcc 2.95, remove when support terminated
template <typename A, typename B>
bool ptr_cmp(A const * a, B const * b)
{
	return a == b;
}


// The public inheritance should go in favour of a suitable data member
// (or maybe private inheritance) at some point of time.
class DocIterator // : public std::vector<CursorSlice>
{
public:
	/// type for cell number in inset
	typedef CursorSlice::idx_type idx_type;
	/// type for paragraph numbers positions within a cell
	typedef CursorSlice::pit_type pit_type;
	/// type for cursor positions within a cell
	typedef CursorSlice::pos_type pos_type;
	/// type for row indices
	typedef CursorSlice::row_type row_type;
	/// type for col indices
	typedef CursorSlice::col_type col_type;

public:
	///
	DocIterator();
	///
	explicit DocIterator(InsetBase & inset);

	/// access slice at position \p i
	CursorSlice const & operator[](size_t i) const { return slices_[i]; }
	/// access slice at position \p i
	CursorSlice & operator[](size_t i) { return slices_[i]; }
	/// chop a few slices from the iterator
	void resize(size_t i) { slices_.resize(i); }

	/// is the iterator valid?
	operator const void*() const { return empty() ? 0 : this; }
	/// is this iterator invalid?
	bool operator!() const { return empty(); }

	/// does this iterator have any content?
	bool empty() const { return slices_.empty(); }

	//
	// access to slice at tip
	//
	/// access to tip
	CursorSlice & top() { return slices_.back(); }
	/// access to tip
	CursorSlice const & top() const { return slices_.back(); }
	/// access to outermost slice
	CursorSlice & bottom() { return slices_.front(); }
	/// access to outermost slice
	CursorSlice const & bottom() const { return slices_.front(); }
	/// how many nested insets do we have?
	size_t depth() const { return slices_.size(); }
	/// the containing inset
	InsetBase & inset() const { return top().inset(); }
	/// return the cell of the inset this cursor is in
	idx_type idx() const { return top().idx(); }
	/// return the cell of the inset this cursor is in
	idx_type & idx() { return top().idx(); }
	/// return the last possible cell in this inset
	idx_type lastidx() const;
	/// return the paragraph this cursor is in
	pit_type pit() const { return top().pit(); }
	/// return the paragraph this cursor is in
	pit_type & pit() { return top().pit(); }
	/// return the last possible paragraph in this inset
	pit_type lastpit() const;
	/// return the position within the paragraph
	pos_type pos() const { return top().pos(); }
	/// return the position within the paragraph
	pos_type & pos() { return top().pos(); }
	/// return the last position within the paragraph
	pos_type lastpos() const;

	/// return the number of embedded cells
	size_t nargs() const;
	/// return the number of embedded cells
	size_t ncols() const;
	/// return the number of embedded cells
	size_t nrows() const;
	/// return the grid row of the top cell
	row_type row() const;
	/// return the last row of the top grid
	row_type lastrow() const { return nrows() - 1; }
	/// return the grid column of the top cell
	col_type col() const;
	/// return the last column of the top grid
	col_type lastcol() const { return ncols() - 1; }
	/// the inset just behind the cursor
	InsetBase * nextInset();
	/// the inset just in front of the cursor
	InsetBase * prevInset();
	/// the inset just in front of the cursor
	InsetBase const * prevInset() const;
	///
	bool boundary() const { return boundary_; }
	///
	void boundary(bool b) { boundary_ = b; }

	/// are we in mathed?
	bool inMathed() const;
	/// are we in texted?
	bool inTexted() const;

	//
	// math-specific part
	//
	/// return the mathed cell this cursor is in
	MathArray const & cell() const;
	/// return the mathed cell this cursor is in
	MathArray & cell();
	/// the mathatom left of the cursor
	MathAtom const & prevAtom() const;
	/// the mathatom left of the cursor
	MathAtom & prevAtom();
	/// the mathatom right of the cursor
	MathAtom const & nextAtom() const;
	/// the mathatom right of the cursor
	MathAtom & nextAtom();

	//
	// text-specific part
	//
	/// the paragraph we're in
	Paragraph & paragraph();
	/// the paragraph we're in
	Paragraph const & paragraph() const;
	/// the row in the paragraph we're in
	Row & textRow();
	/// the row in the paragraph we're in
	Row const & textRow() const;
	///
	LyXText * text();
	///
	LyXText const * text() const;
	///
	InsetBase * innerInsetOfType(int code) const;
	///
	LyXText * innerText();
	///
	LyXText const * innerText() const;

	//
	// elementary moving
	//
	/// move on one logical position, do not descend into nested insets
	void forwardPosNoDescend();
	/// move on one logical position, descend into nested insets
	void forwardPos();
	/// move on one physical character or inset
	void forwardChar();
	/// move on one paragraph
	void forwardPar();
	/// move on one cell
	void forwardIdx();
	/// move on one inset
	void forwardInset();
	/// move backward one logical position
	void backwardPos();
	/// move backward one physical character or inset
	void backwardChar();
	/// move backward one paragraph
	void backwardPar();
	/// move backward one cell
	void backwardIdx();
	/// move backward one inset
	void backwardInset();

	/// are we some 'extension' (i.e. deeper nested) of the given iterator
	bool hasPart(DocIterator const & it) const;

	/// output
	friend std::ostream &
	operator<<(std::ostream & os, DocIterator const & cur);
	///
	friend bool operator==(DocIterator const &, DocIterator const &);
	///
	friend class StableDocIterator;
//protected:
	///
	void clear() { slices_.clear(); }
	///
	void push_back(CursorSlice const & sl) { slices_.push_back(sl); }
	///
	void pop_back() { slices_.pop_back(); }
	/// recompute the inset parts of the cursor from the document data
	void updateInsets(InsetBase * inset);

private:
	/**
	 * When the cursor position is i, is the cursor after the i-th char
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
	///
	std::vector<CursorSlice> const & internalData() const {
		return slices_;
	}
	///
	std::vector<CursorSlice> slices_;
	///
	InsetBase * inset_;
};


DocIterator doc_iterator_begin(InsetBase & inset);
DocIterator doc_iterator_end(InsetBase & inset);


inline
bool operator==(DocIterator const & di1, DocIterator const & di2)
{
	return di1.slices_ == di2.slices_;
}


inline
bool operator!=(DocIterator const & di1, DocIterator const & di2)
{
	return !(di1 == di2);
}


// The difference to a ('non stable') DocIterator is the removed
// (overwritte by 0...) part of the CursorSlice data items. So this thing
// is suitable for external storage, but not for iteration as such.

class StableDocIterator {
public:
	///
	StableDocIterator() {}
	/// non-explicit intended
	StableDocIterator(const DocIterator & it);
	///
	DocIterator asDocIterator(InsetBase * start) const;
	///
	size_t size() const { return data_.size(); }
	/// type for cursor positions within a cell
	typedef CursorSlice::pos_type pos_type;
	///  return the position within the paragraph
	pos_type pos() const { return data_.back().pos(); }
	///  return the position within the paragraph
	pos_type & pos() { return data_.back().pos(); }
	///
	friend std::ostream &
	operator<<(std::ostream & os, StableDocIterator const & cur);
	///
	friend std::istream &
	operator>>(std::istream & is, StableDocIterator & cur);
	///
	friend bool
	operator==(StableDocIterator const &, StableDocIterator const &);
private:
	std::vector<CursorSlice> data_;
};

#endif
