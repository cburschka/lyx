// -*- C++ -*-
/**
 * \file cursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CURSOR_H
#define CURSOR_H

#include "cursor_slice.h"

#include <iosfwd>
#include <vector>

class BufferView;
class UpdatableInset;
class MathAtom;
class DispatchResult;
class FuncRequest;
class LyXText;
class InsetTabular;


/**
 * The cursor class describes the position of a cursor within a document.
 */

class LCursor {
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

	/// create 'empty' cursor. REMOVE ME
	LCursor();
	/// create the cursor of a BufferView
	explicit LCursor(BufferView & bv);
	/// dispatch from innermost inset upwards
	DispatchResult dispatch(FuncRequest const & cmd);
	///
	void push(InsetBase * inset);
	/// restrict cursor nesting to given size
	void pop(int depth);
	/// pop one level off the cursor
	void pop();
	/// access to cursor 'tip'
	CursorSlice & top() { return cursor_.back(); }
	/// access to cursor 'tip'
	CursorSlice const & top() const { return cursor_.back(); }
	/// how many nested insets do we have?
	size_t depth() const { return cursor_.size(); }

	/// access to the topmost slice
	/// the current inset
	InsetBase * inset() const { return top().inset(); }
	/// return the text-ed cell this cursor is in
	idx_type idx() const { return top().idx(); }
	/// return the text-ed cell this cursor is in
	idx_type & idx() { return top().idx(); }
	/// return the paragraph this cursor is in
	par_type par() const { return top().par(); }
	/// return the paragraph this cursor is in
	par_type & par() { return top().par(); }
	/// return the position within the paragraph
	pos_type pos() const { return top().pos(); }
	/// return the position within the paragraph
	pos_type & pos() { return top().pos(); }
	/// return the last position within the paragraph
	pos_type lastpos() const { return top().lastpos(); }
	/// return the number of embedded cells
	size_t nargs() const { return top().nargs(); }
	/// return the number of embedded cells
	size_t ncols() const { return top().ncols(); }
	/// return the number of embedded cells
	size_t nrows() const { return top().nrows(); }
	/// return the grid row of the current cell
	row_type row() const { return top().row(); }
	/// return the grid row of the current cell
	col_type col() const { return top().col(); }

	//
	// math-specific part
	//
	/// return the mathed cell this cursor is in
	MathArray const & cell() const { return top().cell(); }
	/// return the mathed cell this cursor is in
	MathArray & cell() { return top().cell(); }
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
	///
	UpdatableInset * innerInset() const;
	///
	UpdatableInset * innerInsetOfType(int code) const;
	///
	InsetTabular * innerInsetTabular() const;
	///
	LyXText * innerText() const;
	/// returns x,y position
	void getPos(int & x, int & y) const;
	/// returns cursor dimension
	void getDim(int & asc, int & desc) const;

	//
	// common part
	//
	/// move one step to the left
	bool posLeft();
	/// move one step to the right
	bool posRight();

	/// cache the absolute coordinate from the top inset
	void updatePos();
	/// sets anchor to cursor position
	void resetAnchor(); 
	/// access to owning BufferView
	BufferView & bv() const; 
	///
	friend std::ostream & operator<<(std::ostream &, LCursor const &);
public:
	/// mainly used as stack, but wee need random access
	std::vector<CursorSlice> cursor_;
	/// The
	std::vector<CursorSlice> anchor_;
	///
	BufferView * bv_;
private:
	///
	int cached_y_;
};

#endif // LYXCURSOR_H
