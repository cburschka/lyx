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

#include "mathed/math_data.h"
#include "mathed/math_inset.h"

#include <boost/assert.hpp>

using std::vector;
using std::endl;


LCursor::LCursor(BufferView & bv)
	: cursor_(1), anchor_(1), bv_(&bv), current_(0),
	  cached_y_(0), x_target_(-1),
	  selection_(false), mark_(false)
{}


DispatchResult LCursor::dispatch(FuncRequest const & cmd0)
{
	lyxerr << "\nLCursor::dispatch: " << *this << endl;
	FuncRequest cmd = cmd0;

	for (int i = cursor_.size() - 1; i >= 1; --i) {
		current_ = i;
		CursorSlice const & citem = cursor_[i];
		lyxerr << "trying to dispatch to inset " << citem.inset_ << endl;
		DispatchResult res = inset()->dispatch(*this, cmd);
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
	DispatchResult res = bv_->text()->dispatch(*this, cmd);
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
	++current_;
	updatePos();
}


void LCursor::pop(int depth)
{
	lyxerr << "LCursor::pop() to depth " << depth << endl;
	while (cursor_.size() > depth)
		pop();
}


void LCursor::pop()
{
	lyxerr << "LCursor::pop() a level" << endl;
	//BOOST_ASSERT(!cursor_.empty());
	if (cursor_.size() <= 1)
		lyxerr << "### TRYING TO POP FROM EMPTY CURSOR" << endl;
	else {
		cursor_.pop_back();
		anchor_.pop_back();
		current_ = cursor_.size() - 1;
	}
	lyxerr << "LCursor::pop() current now: " << current_ << endl;
}


CursorSlice & LCursor::current()
{
	//lyxerr << "accessing cursor slice " << current_
	//	<< ": " << cursor_[current_] << endl;
	return cursor_[current_];
}


CursorSlice const & LCursor::current() const
{
	//lyxerr << "accessing cursor slice " << current_
	//	<< ": " << cursor_[current_] << endl;
	return cursor_[current_];
}


LyXText * LCursor::innerText() const
{
	//lyxerr << "LCursor::innerText()  depth: " << cursor_.size() << endl;
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
		asc = 10;
		desc = 10;
		//innerInset()->getCursorDim(asc, desc);
	}
}


void LCursor::getPos(int & x, int & y) const
{
	if (cursor_.size() <= 1) {
		x = bv_->text()->cursorX();
		y = bv_->text()->cursorY();
//		y -= bv_->top_y();
	} else {
		if (inset()->asUpdatableInset()) {
			// Would be nice to clean this up to make some understandable sense...
			// Non-obvious. The reason we have to have these
			// extra checks is that the ->getCursor() calls rely
			// on the inset's own knowledge of its screen position.
			// If we scroll up or down in a big enough increment, the
			// inset->draw() is not called: this doesn't update
			// inset.top_baseline, so getCursor() returns an old value.
			// Ugly as you like.
			inset()->asUpdatableInset()->getCursorPos(cursor_.back().idx_, x, y);
			x += inset()->asUpdatableInset()->x();
			y += cached_y_;
		} else if (inset()->asMathInset()) {
#warning FIXME
			x = 100;
			y = 100;
		} else {
			// this should not happen
			BOOST_ASSERT(false);
		}
	}
}


InsetBase * LCursor::innerInsetOfType(int code) const
{
	for (int i = cursor_.size() - 1; i >= 1; --i)
		if (cursor_[i].inset_->lyxCode() == code)
			return cursor_[i].inset_;
	return 0;
}


InsetTabular * LCursor::innerInsetTabular() const
{
	return static_cast<InsetTabular *>(innerInsetOfType(InsetBase::TABULAR_CODE));
}


void LCursor::resetAnchor()
{
	anchor_ = cursor_;
}


BufferView & LCursor::bv() const
{
	return *bv_;
}


MathAtom const & LCursor::prevAtom() const
{
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom & LCursor::prevAtom()
{
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom const & LCursor::nextAtom() const
{
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


MathAtom & LCursor::nextAtom()
{
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


bool LCursor::posLeft()
{
	if (pos() == 0)
		return false;
	--pos();
	return true;
}


bool LCursor::posRight()
{
	if (pos() == lastpos())
		return false;
	++pos();
	return true;
}


CursorSlice & LCursor::anchor()
{
	return anchor_.back();
}


CursorSlice const & LCursor::anchor() const
{
	return anchor_.back();
}


CursorSlice const & LCursor::selStart() const
{
	if (!selection())
		return cursor_.back();
	// can't use std::min as this creates a new object
	return anchor() < cursor_.back() ? anchor() : cursor_.back();
}


CursorSlice const & LCursor::selEnd() const
{
	if (!selection())
		return cursor_.back();
	return anchor() > cursor_.back() ? anchor() : cursor_.back();
}


CursorSlice & LCursor::selStart()
{
	if (!selection())
		return cursor_.back();
	return anchor() < cursor_.back() ? anchor() : cursor_.back();
}


CursorSlice & LCursor::selEnd()
{
	if (selection())
		return cursor_.back();
	return anchor() > cursor_.back() ? anchor() : cursor_.back();
}


void LCursor::setSelection()
{
	selection() = true;
	// a selection with no contents is not a selection
	if (par() == anchor().par() && pos() == anchor().pos())
		selection() = false;
}


void LCursor::clearSelection()
{
	selection() = false;
	mark() = false;
	resetAnchor();
	bv().unsetXSel();
}



//
// CursorBase
//


void increment(CursorBase & it)
{
	CursorSlice & top = it.back();
	MathArray & ar = top.asMathInset()->cell(top.idx_);

	// move into the current inset if possible
	// it is impossible for pos() == size()!
	MathInset * n = 0;
	if (top.pos() != top.lastpos())
		n = (ar.begin() + top.pos_)->nucleus();
	if (n && n->isActive()) {
		it.push_back(CursorSlice(n));
		return;
	}

	// otherwise move on one cell back if possible
	if (top.pos() < top.lastpos()) {
		// pos() == lastpos() is valid!
		++top.pos_;
		return;
	}

	// otherwise try to move on one cell if possible
	while (top.idx_ + 1 < top.asMathInset()->nargs()) {
		// idx() == nargs() is _not_ valid!
		++top.idx_;
		if (top.asMathInset()->validCell(top.idx_)) {
			top.pos_ = 0;
			return;
		}
	}

	// otherwise leave array, move on one back
	// this might yield pos() == size(), but that's a ok.
	it.pop_back();
	// it certainly invalidates top
	++it.back().pos_;
}


CursorBase ibegin(InsetBase * p)
{
	CursorBase it;
	it.push_back(CursorSlice(p));
	return it;
}


CursorBase iend(InsetBase * p)
{
	CursorBase it;
	it.push_back(CursorSlice(p));
	CursorSlice & top = it.back();
	top.idx_ = top.asMathInset()->nargs() - 1;
	top.pos_ = top.asMathInset()->cell(top.idx_).size();
	return it;
}


void LCursor::x_target(int x)
{
	x_target_ = x;
}


int LCursor::x_target() const
{
	return x_target_;
}


Paragraph & LCursor::paragraph()
{
	return current_ ? current().paragraph() : *bv_->text()->getPar(par());
}


Paragraph const & LCursor::paragraph() const
{
	return current_ ? current().paragraph() : *bv_->text()->getPar(par());
}


LCursor::pos_type LCursor::lastpos() const
{
	return current_ ? current().lastpos() : bv_->text()->getPar(par())->size();
}


size_t LCursor::nargs() const
{
	// assume 1x1 grid for 'plain text'
	return current_ ? current().nargs() : 1;
}


size_t LCursor::ncols() const
{
	// assume 1x1 grid for 'plain text'
	return current_ ? current().ncols() : 1;
}


size_t LCursor::nrows() const
{
	// assume 1x1 grid for 'plain text'
	return current_ ? current().nrows() : 1;
}


LCursor::row_type LCursor::row() const
{
	BOOST_ASSERT(current_ > 0);
	return current().row();
}


LCursor::col_type LCursor::col() const
{
	BOOST_ASSERT(current_ > 0);
	return current().col();
}


MathArray const & LCursor::cell() const
{
	BOOST_ASSERT(current_ > 0);
	return current().cell();
}


MathArray & LCursor::cell()
{
	BOOST_ASSERT(current_ > 0);
	return current().cell();
}


void LCursor::info(std::ostream & os)
{
	for (int i = 1, n = depth(); i < n; ++i) {
		cursor_[i].inset()->infoize(os);
		os << "  ";
	}
#warning FIXME
	//if (pos() != 0)
	//	prevAtom()->infoize2(os);
	// overwite old message
	os << "                    ";
}


std::ostream & operator<<(std::ostream & os, LCursor const & cur)
{
	os << "\n";
	for (size_t i = 0, n = cur.cursor_.size(); i != n; ++i)
		os << "  (" << cur.cursor_[i] << " | " << cur.anchor_[i] << "\n";
	return os;
}
