/**
 * \file cursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "buffer.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "iterators.h"
#include "lfuns.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "lyxrow.h"

#include "insets/updatableinset.h"
#include "insets/insettabular.h"
#include "insets/insettext.h"

#include <boost/assert.hpp>

using std::vector;
using std::endl;


std::ostream & operator<<(std::ostream & os, LCursor const & cur)
{
	os << "\n";
	for (size_t i = 0, n = cur.cursor_.size(); i != n; ++i)
		os << "   (" << cur.cursor_[i] << " | " << cur.anchor_[i] << "\n";
	return os;
}


LCursor::LCursor()
	: cursor_(1), anchor_(1), bv_(0)
{}


LCursor::LCursor(BufferView & bv)
	: cursor_(1), anchor_(1), bv_(&bv)
{}


DispatchResult LCursor::dispatch(FuncRequest const & cmd0)
{
	lyxerr << "\nLCursor::dispatch: " << *this << endl;
	FuncRequest cmd = cmd0;

	for (int i = cursor_.size() - 1; i >= 1; --i) {
		CursorSlice const & citem = cursor_[i];
		lyxerr << "trying to dispatch to inset " << citem.inset_ << endl;
		DispatchResult res = citem.inset_->dispatch(*bv_, cmd);
		if (res.dispatched()) {
			lyxerr << " successfully dispatched to inset " << citem.inset_ << endl;
			return DispatchResult(true, true);
		}
		// remove one level of cursor
		switch (res.val()) {
			case FINISHED:
				pop(i);
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
				break;
			case FINISHED_RIGHT:
				pop(i);
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
				break;
			case FINISHED_UP:
				pop(i);
				cmd = FuncRequest(LFUN_FINISHED_UP);
				break;
			case FINISHED_DOWN:
				pop(i);
				cmd = FuncRequest(LFUN_FINISHED_DOWN);
				break;
			default:
				lyxerr << "not handled on level " << i << " val: " << res.val() << endl;
				break;
		}
	}
	lyxerr << "trying to dispatch to main text " << bv_->text() << endl;
	DispatchResult res = bv_->text()->dispatch(*bv_, cmd);
	lyxerr << "   result: " << res.val() << endl;

	if (!res.dispatched()) {
		lyxerr << "trying to dispatch to bv " << bv_ << endl;
		bool sucess = bv_->dispatch(cmd);
		lyxerr << "   result: " << sucess << endl;
		res.dispatched(sucess);
	}

	return res;
}


void LCursor::push(InsetBase * inset)
{
	lyxerr << "LCursor::push()  inset: " << inset << endl;
	cursor_.push_back(CursorSlice(inset));
	anchor_.push_back(CursorSlice(inset));
	updatePos();
}



void LCursor::pop(int depth)
{
	lyxerr << "LCursor::pop() to " << depth << endl;
	while (cursor_.size() > 1 && depth < cursor_.size()) {
		lyxerr <<   "LCursor::pop a level " << endl;
		cursor_.pop_back();
		anchor_.pop_back();
	}
}


void LCursor::pop()
{
	lyxerr << "LCursor::pop() " << endl;
	//BOOST_ASSERT(!cursor_.empty());
	if (cursor_.size() <= 1)
		lyxerr << "### TRYING TO POP FROM EMPTY CURSOR" << endl;
	else {
		cursor_.pop_back();
		anchor_.pop_back();
	}
}


UpdatableInset * LCursor::innerInset() const
{
	return cursor_.size() <= 1 ? 0 : cursor_.back().asUpdatableInset();
}


LyXText * LCursor::innerText() const
{
	if (cursor_.size() > 1) {
		// go up until first non-0 text is hit
		// (innermost text is 0 e.g. for mathed and the outer tabular level)
		for (int i = cursor_.size() - 1; i >= 1; --i)
			if (cursor_[i].text())
				return cursor_[i].text();
	}
	return bv_->text();
}


void LCursor::updatePos()
{
	if (cursor_.size() > 1)
		cached_y_ = bv_->top_y() + cursor_.back().inset()->y();
}


void LCursor::getDim(int & asc, int & desc) const
{
	LyXText * txt = innerText();

	if (txt) {
		Row const & row = *txt->cursorRow();
		asc = row.baseline();
		desc = row.height() - asc;
	} else {
		innerInset()->getCursorDim(asc, desc);
	}
}


void LCursor::getPos(int & x, int & y) const
{
	if (cursor_.size() <= 1) {
		x = bv_->text()->cursorX();
		y = bv_->text()->cursorY();
//		y -= bv_->top_y();
	} else {
		// Would be nice to clean this up to make some understandable sense...
		UpdatableInset * inset = innerInset();
		// Non-obvious. The reason we have to have these
		// extra checks is that the ->getCursor() calls rely
		// on the inset's own knowledge of its screen position.
		// If we scroll up or down in a big enough increment, the
		// inset->draw() is not called: this doesn't update
		// inset.top_baseline, so getCursor() returns an old value.
		// Ugly as you like.
		if (inset) {
			inset->getCursorPos(cursor_.back().idx_, x, y);
			x += inset->x();
			y += cached_y_;
		}
	}
}


UpdatableInset * LCursor::innerInsetOfType(int code) const
{
	for (int i = cursor_.size() - 1; i >= 1; --i)
		if (cursor_[i].asUpdatableInset()->lyxCode() == code)
			return cursor_[i].asUpdatableInset();
	return 0;
}


InsetTabular * LCursor::innerInsetTabular() const
{
	return static_cast<InsetTabular *>
		(innerInsetOfType(InsetOld::TABULAR_CODE));
}


void LCursor::resetAnchor()
{
	anchor_ = cursor_;
}


BufferView & LCursor::bv() const
{
	return *bv_;
}
