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


std::ostream & operator<<(std::ostream & os, CursorItem const & item)
{
	os << " inset: " << item.inset_
	   << " text: " << item.text()
//	   << " par: " << item.par_
//	   << " pos: " << item.pos_
	   << " x: " << item.inset_->x()
	   << " y: " << item.inset_->y()
;
	return os;
}


LyXText * CursorItem::text() const
{
	return inset_->getText(0);
}


std::ostream & operator<<(std::ostream & os, LCursor const & cursor)
{
	os << "\n";
	for (size_t i = 0, n = cursor.data_.size(); i != n; ++i)
		os << "   " << cursor.data_[i] << "\n";
	return os;
}


LCursor::LCursor(BufferView * bv)
	: bv_(bv)
{}


DispatchResult LCursor::dispatch(FuncRequest const & cmd0)
{
	lyxerr << "\nLCursor::dispatch: " << *this << endl;
	FuncRequest cmd = cmd0;

	for (int i = data_.size() - 1; i >= 0; --i) {
		CursorItem const & citem = data_[i];
		lyxerr << "trying to dispatch to inset " << citem.inset_ << endl;
		DispatchResult res = citem.inset_->dispatch(cmd);
		if (res.dispatched()) {
			lyxerr << " successfully dispatched to inset " << citem.inset_ << endl;
			return DispatchResult(true, true);
		}
		// remove one level of cursor
		switch (res.val()) {
			case FINISHED:
				pop(i);
				cmd = FuncRequest(bv_, LFUN_FINISHED_LEFT);
				break;
			case FINISHED_RIGHT:
				pop(i);
				cmd = FuncRequest(bv_, LFUN_FINISHED_RIGHT);
				break;
			case FINISHED_UP: 
				pop(i);
				cmd = FuncRequest(bv_, LFUN_FINISHED_UP);
				break;
			case FINISHED_DOWN:
				pop(i);
				cmd = FuncRequest(bv_, LFUN_FINISHED_DOWN);
				break;
			default:
				lyxerr << "not handled on level " << i << " val: " << res.val() << endl;
				break;
		}
	}
	lyxerr << "trying to dispatch to main text " << bv_->text << endl;
	DispatchResult res = bv_->text->dispatch(cmd);
	lyxerr << "   result: " << res.val() << endl;

	if (!res.dispatched()) {
		lyxerr << "trying to dispatch to bv " << bv_ << endl;
		bool sucess = bv_->dispatch(cmd);
		lyxerr << "   result: " << sucess << endl;
		res.dispatched(sucess);
	}

	return res;
}


void LCursor::push(UpdatableInset * inset)
{
	lyxerr << "LCursor::push()  inset: " << inset << endl;
	data_.push_back(CursorItem(inset));
	cached_y_ = bv_->top_y() + innerInset()->y();
}


void LCursor::pop(int depth)
{
	lyxerr << "LCursor::pop() to " << depth << endl;
	while (depth < data_.size()) {
		lyxerr <<   "LCursor::pop a level " << endl;
		data_.pop_back();
	}
}


void LCursor::pop()
{
	lyxerr << "LCursor::pop() " << endl;
	//BOOST_ASSERT(!data_.empty());
	if (data_.empty())
		lyxerr << "### TRYING TO POP FROM EMPTY CURSOR" << endl;
	else
		data_.pop_back();
}


UpdatableInset * LCursor::innerInset() const
{
	return data_.empty() ? 0 : data_.back().inset_;
}


LyXText * LCursor::innerText() const
{
	if (!data_.empty()) {
		// go up until first non-0 text is hit
		// (innermost text is 0 e.g. for mathed and the outer tabular level)
		for (int i = data_.size() - 1; i >= 0; --i)
			if (data_[i].text())
				return data_[i].text();
	}
	return bv_->text;
}


void LCursor::updatePos()
{
	if (!data_.empty())
		cached_y_ = bv_->top_y() + innerInset()->y();
}


void LCursor::getDim(int & asc, int & desc) const
{
	LyXText * txt = innerText();
	
	if (txt) {
		Row const & row = *txt->cursorRow();
		asc = row.baseline();
		desc = row.height() - asc;
	} else
		innerInset()->getCursorDim(bv_, asc, desc);
}


void LCursor::getPos(int & x, int & y) const
{
	if (data_.empty()) {
		x = bv_->text->cursor.x();
		y = bv_->text->cursor.y();
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
		//inset->getCursorPos(bv_, x, y);
		//y = inset->insetInInsetY() + bv_->text->cursor.y();
		inset->getCursorPos(bv_, x, y);
		x += inset->x();
		y += cached_y_;
	}
}


UpdatableInset * LCursor::innerInsetOfType(int code) const
{
	for (int i = data_.size() - 1; i >= 0; --i)
		if (data_[i].inset_->lyxCode() == code)
			return data_[i].inset_;
	return 0;
}

	
InsetTabular * LCursor::innerInsetTabular() const
{
	return static_cast<InsetTabular *>
		(innerInsetOfType(InsetOld::TABULAR_CODE));
}
