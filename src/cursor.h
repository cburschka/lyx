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

#include "textcursor.h"
#include "cursor_slice.h"

#include "support/types.h"

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
	/// create 'empty' cursor
	explicit LCursor(BufferView * bv);
	/// dispatch from innermost inset upwards
	DispatchResult dispatch(FuncRequest const & cmd);
	///
	void push(UpdatableInset * inset);
	/// restrict cursor nesting to given size
	void pop(int depth);
	/// pop one level off the cursor
	void pop();
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
	///
	friend std::ostream & operator<<(std::ostream &, LCursor const &);
public:
	/// mainly used as stack, but wee need random access
	std::vector<CursorSlice> data_;
	///
	BufferView * bv_;
private:
	///
	int cached_y_;
};

#endif // LYXCURSOR_H
