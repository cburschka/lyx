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

#include "support/types.h"

#include <vector>

class BufferView;
class InsetOld;
class DispatchResult;
class FuncRequest;
class LyXText;


/**
 * The cursor class describes the position of a cursor within a document.
 */

class CursorItem {
public:
	///
	CursorItem() : inset_(0), text_(0), idx_(0), par_(0), pos_(0) {}
	///
	CursorItem(InsetOld * inset, LyXText * text)
		: inset_(inset), text_(text), idx_(0), par_(0), pos_(0)
	{}
	///
	friend std::ostream & operator<<(std::ostream &, CursorItem const &);
public:
	///
	InsetOld * inset_;
	///
	LyXText * text_;
	///
	int idx_;
	///
	int par_;
	///
	int pos_;
};


class LCursor {
public:
	///
	LCursor(BufferView * bv);
	/// dispatch from innermost inset upwards
	DispatchResult dispatch(FuncRequest const & cmd);
	/// adjust cursor acording to result
	bool handleResult(DispatchResult const & res);
	///
	void push(InsetOld *, LyXText *);
	///
	void pop();
	///
	InsetOld * innerInset() const;
	///
	LyXText * innerText() const;
	///
	friend std::ostream & operator<<(std::ostream &, LCursor const &);
public:
	/// mainly used as stack, but wee need random access
	std::vector<CursorItem> data_;
	///
	BufferView * bv_;
};

#endif // LYXCURSOR_H
