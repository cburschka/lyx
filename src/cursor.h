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
class InsetTabular;
class LyXText;
class Paragraph;



// this is used for traversing math insets
typedef std::vector<CursorSlice> CursorBase;
/// move on one step
void increment(CursorBase &);
///
CursorBase ibegin(InsetBase * p);
///
CursorBase iend(InsetBase * p);


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
	/// access to current cursor slice
	CursorSlice & current();
	/// access to current cursor slice
	CursorSlice const & current() const;
	/// how many nested insets do we have?
	size_t depth() const { return cursor_.size(); }

	//
	// selection
	//
	/// selection active?
	bool selection() const { return selection_; }
	/// selection active?
	bool & selection() { return selection_; }
	/// did we place the anchor?
	bool mark() const { return mark_; }
	/// did we place the anchor?
	bool & mark() { return mark_; }
	///
	void setSelection();
	///
	void clearSelection();
	///
	CursorSlice & selStart();
	///
	CursorSlice const & selStart() const;
	///
	CursorSlice & selEnd();
	///
	CursorSlice const & selEnd() const;

	//
	// access to the 'current' cursor slice
	//
	/// the current inset
	InsetBase * inset() const { return current().inset(); }
	/// return the text-ed cell this cursor is in
	idx_type idx() const { return current().idx(); }
	/// return the text-ed cell this cursor is in
	idx_type & idx() { return current().idx(); }
	/// return the paragraph this cursor is in
	par_type par() const { return current().par(); }
	/// return the paragraph this cursor is in
	par_type & par() { return current().par(); }
	/// return the position within the paragraph
	pos_type pos() const { return current().pos(); }
	/// return the position within the paragraph
	pos_type & pos() { return current().pos(); }
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
	/// auto-correct mode
	bool autocorrect() const { return autocorrect_; }
	/// auto-correct mode
	bool & autocorrect() { return autocorrect_; }
	/// are we entering a macro name?
	bool macromode() const { return macromode_; }
	/// are we entering a macro name?
	bool & macromode() { return macromode_; }

	//
	// text-specific part
	///
	bool boundary() const { return current().boundary(); }
	///
	Paragraph & paragraph();
	///
	Paragraph const & paragraph() const;
	///
	InsetBase * innerInsetOfType(int code) const;
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

	/// set target x position of cursor
	void x_target(int x);
	/// return target x position of cursor
	int x_target() const;

	/// access to selection anchor
	CursorSlice & anchor();
	/// access to selection anchor
	CursorSlice const & anchor() const;
	/// cache the absolute coordinate from the top inset
	void updatePos();
	/// sets anchor to cursor position
	void resetAnchor(); 
	/// access to owning BufferView
	BufferView & bv() const; 
	/// get some interesting description of current position
	void info(std::ostream & os);

	/// output
	friend std::ostream & operator<<(std::ostream & os, LCursor const & cur);
public:
//private:
	/// mainly used as stack, but wee need random access
	std::vector<CursorSlice> cursor_;
	/// the anchor position
	std::vector<CursorSlice> anchor_;

private:
	/// don't implement this
	void operator=(LCursor const &);
	/// don't implement this
	LCursor(LCursor const &);

	///
	BufferView * const bv_;
	/// current slice
	int current_;
	///
	int cached_y_;
	/**
	 * The target x position of the cursor. This is used for when
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
	int x_target_;
	// do we have a selection?
	bool selection_;
	// are we on the way to get one?
	bool mark_;

	//
	// math specific stuff that could be promoted to "global" later
	//
	/// do we allow autocorrection
	bool autocorrect_;
	/// are we entering a macro name?
	bool macromode_;
};

#endif // LYXCURSOR_H
