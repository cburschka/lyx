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

	/// create 'empty' cursor. REMOVE ME
	LCursor();
	/// create 'empty' cursor
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

	/// set the cell the cursor is in
	void cell(int);
	/// return the cell this cursor is in
	int cell() const;
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
	/// cache the absolute coordinate from the top inset
	void updatePos();
	/// sets anchor to cursor position
	void resetAnchor(); 
	/// sets anchor to cursor position
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
