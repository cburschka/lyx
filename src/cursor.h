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

class CursorItem : public TextCursor {
public:
	///
	CursorItem() : inset_(0), text_(0), idx_(0), par_(0), pos_(0) {}
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


class Cursor {
public:
	///
	Cursor() {}
	///
	DispatchResult dispatch(FuncRequest const & cmd);
public:
	/// mainly used as stack, bnut wee need random access
	std::vector<CursorItem> data_;
};

/// build cursor from current cursor in view
void buildCursor(Cursor & cursor, BufferView & bv);

/// build cursor from (x,y) coordinates
void buildCursor(Cursor & cursor, BufferView & bv, int x, int y);

#endif // LYXCURSOR_H
