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

#include "ParagraphList_fwd.h"
#include "textcursor.h"

#include "support/types.h"

#include <vector>

class BufferView;
class DispatchResult;
class FuncRequest;
class LyXText;


/**
 * The cursor class describes the position of a cursor within a document.
 */

class CursorItem : public TextCursor {
public:
	///
	CursorItem() : text_(0) {}
public:
	///
	LyXText * text_;
	///
	ParagraphList::iterator pit_;
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
