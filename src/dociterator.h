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

class BufferView;
class MathAtom;
class LyXText;
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
class DocumentIterator : public std::vector<CursorSlice>
{
public:
	/// type for cell number in inset
	typedef CursorSlice::idx_type idx_type;
	/// type for paragraph numbers positions within a cell
	typedef CursorSlice::par_type par_type;
	/// type for cursor positions within a cell
	typedef CursorSlice::pos_type pos_type;
	/// type for row indices
	typedef CursorSlice::row_type row_type;
	/// type for col indices
	typedef CursorSlice::col_type col_type;

public:
	///
	DocumentIterator();
	///
	explicit DocumentIterator(BufferView & bv);
	///
	BufferView & bv() const { return *bv_; } 

	//
	// access to slice at tip
	//
	/// access to tip
	CursorSlice & top() { return back(); }
	/// access to tip
	CursorSlice const & top() const { return back(); }
	/// how many nested insets do we have?
	size_t depth() const { return size(); }
	/// the containing inset
	InsetBase * inset() const { return back().inset(); }
	/// return the cell of the inset this cursor is in
	idx_type idx() const { return back().idx(); }
	/// return the cell of the inset this cursor is in
	idx_type & idx() { return back().idx(); }
	/// return the last possible cell in this inset
	idx_type lastidx() const;
	/// return the paragraph this cursor is in
	par_type par() const { return back().par(); }
	/// return the paragraph this cursor is in
	par_type & par() { return back().par(); }
	/// return the last possible paragraph in this inset
	par_type lastpar() const;
	/// return the position within the paragraph
	pos_type pos() const { return back().pos(); }
	/// return the position within the paragraph
	pos_type & pos() { return back().pos(); }
	/// return the last position within the paragraph
	pos_type lastpos() const;
	/// return the display row of the cursor with in the top par
	row_type crow() const;
	/// return the display row of the cursor with in the top par
	row_type lastcrow() const;

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
	/// see comment for boundary_ below
	bool boundary() const { return top().boundary(); }
	/// see comment for boundary_ below
	bool & boundary() { return top().boundary(); }
	/// the paragraph we're in
	Paragraph & paragraph();
	/// the paragraph we're in
	Paragraph const & paragraph() const;
	/// the row in the paragraph we're in
	Row & textRow();
	/// the row in the paragraph we're in
	Row const & textRow() const;
	///
	LyXText * text() const;
	///
	CursorSlice const & innerTextSlice() const;
	///
	InsetBase * innerInsetOfType(int code) const;
	///
	LyXText * innerText() const;

	//
	// elementary moving
	//
	/// move on one position
	void forwardPos();
	/// move on one paragraph
	void forwardPar();
	/// move on one cell
	void forwardIdx();
	/// move on one inset
	void forwardInset();
	/// output
	friend std::ostream &
	operator<<(std::ostream & os, DocumentIterator const & cur);

private:
	///
	BufferView * bv_;	
};


///
DocumentIterator bufferBegin(BufferView & bv);
///
DocumentIterator bufferEnd();
///
DocumentIterator insetBegin(BufferView & bv, InsetBase * inset);
///
DocumentIterator insetEnd();


// The difference to a ('non stable') DocumentIterator is the removed
// (overwritte by 0...) part of the CursorSlice data items. So this thing
// is suitable for external storage, but not for iteration as such.

class StableDocumentIterator {
public:
	///
	StableDocumentIterator() {}
	/// non-explicit intended
	StableDocumentIterator(const DocumentIterator & it);
	///
	DocumentIterator asDocumentIterator(BufferView & bv) const;
	///
	size_t size() const { return data_.size(); }
	///
	friend std::ostream &
	operator<<(std::ostream & os, StableDocumentIterator const & cur);
	///
	friend std::istream &
	operator>>(std::istream & is, StableDocumentIterator & cur);
private:
	std::vector<CursorSlice> data_;
};

bool operator==(StableDocumentIterator const &, StableDocumentIterator const &);

#endif
