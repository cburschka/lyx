/**
 * \file cursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Alfredo Braunstein
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
#include "lyxfunc.h" // only for setMessage()
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/updatableinset.h"
#include "insets/insettabular.h"
#include "insets/insettext.h"

#include "mathed/math_data.h"
#include "mathed/math_hullinset.h"
#include "mathed/math_support.h"

#include "support/limited_stack.h"
#include "frontends/LyXView.h"

#include <boost/assert.hpp>

using std::string;
using std::vector;
using std::endl;
#ifndef CXX_GLOBAL_CSTD
using std::isalpha;
#endif
using std::min;
using std::swap;



// our own cut buffer
limited_stack<string> theCutBuffer;


LCursor::LCursor(BufferView & bv)
	: cursor_(1), anchor_(1), bv_(&bv), current_(0),
	  cached_y_(0), x_target_(-1),
	  selection_(false), mark_(false)
{}


void LCursor::reset()
{
	cursor_.clear();
	anchor_.clear();
	cursor_.push_back(CursorSlice());
	anchor_.push_back(CursorSlice());
	current_ = 0;
	cached_y_ = 0;
	clearTargetX();
	selection_ = false;
	mark_ = false;
}


DispatchResult LCursor::dispatch(FuncRequest const & cmd0)
{
	//lyxerr << "\nLCursor::dispatch: cmd: " << cmd0 << endl << *this << endl;
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
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
				break;
			case FINISHED_RIGHT:
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
				break;
			case FINISHED_UP:
				cmd = FuncRequest(LFUN_FINISHED_UP);
				break;
			case FINISHED_DOWN:
				cmd = FuncRequest(LFUN_FINISHED_DOWN);
				break;
			default:
				lyxerr << "not handled on level " << i << " val: " << res.val() << endl;
				break;
		}
	}
	current_ = 0;
	lyxerr << "trying to dispatch to main text " << bv_->text()
		<< " with cursor: " << *this << endl;
	DispatchResult res = bv_->text()->dispatch(*this, cmd);
	lyxerr << "   result: " << res.val() << endl;
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
	while (int(cursor_.size()) > depth + 1)
		pop();
	lyxerr << "LCursor::pop() result: " << *this << endl;
}


void LCursor::pop()
{
	BOOST_ASSERT(cursor_.size() >= 1);
	cursor_.pop_back();
	anchor_.pop_back();
	current_ = cursor_.size() - 1;
}


void LCursor::pushLeft(InsetBase * p)
{
	BOOST_ASSERT(!cursor_.empty());
	//lyxerr << "Entering inset " << t << " left" << endl;
	push(p);
	p->idxFirst(*this);
}


bool LCursor::popLeft()
{
	BOOST_ASSERT(!cursor_.empty());
	//lyxerr << "Leaving inset to the left" << endl;
	if (depth() <= 1) {
		if (depth() == 1)
			inset()->notifyCursorLeaves(idx());
		return false;
	}
	inset()->notifyCursorLeaves(idx());
	pop();
	return true;
}


bool LCursor::popRight()
{
	BOOST_ASSERT(!cursor_.empty());
	//lyxerr << "Leaving inset to the right" << endl;
	if (depth() <= 1) {
		if (depth() == 1)
			inset()->notifyCursorLeaves(idx());
		return false;
	}
	inset()->notifyCursorLeaves(idx());
	pop();
	posRight();
	return true;
}


CursorSlice & LCursor::current()
{
	BOOST_ASSERT(!cursor_.empty());
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


int LCursor::currentMode()
{
	BOOST_ASSERT(!cursor_.empty());
	for (int i = cursor_.size() - 1; i >= 1; --i) {
		int res = cursor_[i].inset()->currentMode();
		if (res != MathInset::UNDECIDED_MODE)
			return res;
	}
	return MathInset::TEXT_MODE;
}


LyXText * LCursor::innerText() const
{
	BOOST_ASSERT(!cursor_.empty());
	//lyxerr << "LCursor::innerText()  depth: " << cursor_.size() << endl;
	if (cursor_.size() > 1) {
		// go up until first non-0 text is hit
		// (innermost text is 0 in mathed)
		for (int i = cursor_.size() - 1; i >= 1; --i)
			if (cursor_[i].text())
				return cursor_[i].text();
	}
	return bv_->text();
}


CursorSlice const & LCursor::innerTextSlice() const
{
	BOOST_ASSERT(!cursor_.empty());
	//lyxerr << "LCursor::innerTextSlice()  depth: " << cursor_.size() << endl;
	if (cursor_.size() > 1) {
		// go up until first non-0 text is hit
		// (innermost text is 0 in mathed)
		for (int i = cursor_.size() - 1; i >= 1; --i)
			if (cursor_[i].text())
				return cursor_[i];
	}
	return cursor_[0];
}


void LCursor::updatePos()
{
	BOOST_ASSERT(!cursor_.empty());
	if (cursor_.size() > 1)
		cached_y_ = bv_->top_y() + cursor_.back().inset()->yo();
		//cached_y_ = cursor_.back().inset()->yo();
}


void LCursor::getDim(int & asc, int & des) const
{
	BOOST_ASSERT(!cursor_.empty());
	LyXText * text = innerText();
#warning crashes with text-in-math
	if (0 && text) {
		RowList::iterator const rit = text->cursorRow();
		if (rit != text->endRow()) {
			asc = rit->baseline();
			des = rit->height() - asc;
		} else {
			asc = 10;
			des = 10;
		}
	} else {
		asc = 10;
		des = 10;
		//innerInset()->getCursorDim(asc, des);
	}
}


void LCursor::getPos(int & x, int & y) const
{
	BOOST_ASSERT(!cursor_.empty());
	x = 0;
	y = 0;
	if (cursor_.size() <= 1) {
		x = bv_->text()->cursorX(cursor_.front());
		y = bv_->text()->cursorY(cursor_.front());
	} else {
		inset()->getCursorPos(cursor_.back(), x, y);
		// getCursorPos gives _screen_ coordinates. We need to add
		// top_y to get document coordinates. This is hidden in cached_y_.
		//y += cached_y_ - inset()->yo();
		// The rest is non-obvious. The reason we have to have these
		// extra computation is that the getCursorPos() calls rely
		// on the inset's own knowledge of its screen position.
		// If we scroll up or down in a big enough increment,
		// inset->draw() is not called: this doesn't update
		// inset.yo_, so getCursor() returns an old value.
		// Ugly as you like.
	}
	//lyxerr << "#### LCursor::getPos: " << *this 
	// << " x: " << x << " y: " << y << endl;
}


void LCursor::paste(string const & data)
{
	dispatch(FuncRequest(LFUN_PASTE, data));
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


CursorSlice const & LCursor::selBegin() const
{
	if (!selection())
		return cursor_.back();
	// can't use std::min as this creates a new object
	return anchor() < cursor_.back() ? anchor() : cursor_.back();
}


CursorSlice & LCursor::selBegin()
{
	if (!selection())
		return cursor_.back();
	return anchor() < cursor_.back() ? anchor() : cursor_.back();
}


CursorSlice const & LCursor::selEnd() const
{
	if (!selection())
		return cursor_.back();
	return anchor() > cursor_.back() ? anchor() : cursor_.back();
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


void LCursor::setSelection(CursorBase const & where, size_t n)
{
	selection() = true;
	cursor_ = where;
	anchor_ = where;
	pos() += n;
}


void LCursor::clearSelection()
{
	selection() = false;
	mark() = false;
	resetAnchor();
	bv().unsetXSel();
}


int & LCursor::x_target()
{
	return x_target_;
}


int LCursor::x_target() const
{
	return x_target_;
}


void LCursor::clearTargetX()
{
	x_target_ = -1;
}


LyXText * LCursor::text() const
{
	return current_ ? current().text() : bv_->text();
}


Paragraph & LCursor::paragraph()
{
	BOOST_ASSERT(!inMathed());
	return current_ ? current().paragraph() : *bv_->text()->getPar(par());
}


Paragraph const & LCursor::paragraph() const
{
	BOOST_ASSERT(!inMathed());
	return current_ ? current().paragraph() : *bv_->text()->getPar(par());
}


LCursor::par_type LCursor::lastpar() const
{
	return inMathed() ? 0 : text()->paragraphs().size() - 1;
}


LCursor::pos_type LCursor::lastpos() const
{
	InsetBase * inset = current().inset();
	return inset && inset->asMathInset() ? cell().size() : paragraph().size();
}


LCursor::row_type LCursor::crow() const
{
	return paragraph().row(pos());
}


LCursor::row_type LCursor::lastcrow() const
{
	return paragraph().rows.size();
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
	if (pos() != 0)
		prevInset()->infoize2(os);
	// overwite old message
	os << "                    ";
}


namespace {

void region(CursorSlice const & i1, CursorSlice const & i2,
	LCursor::row_type & r1, LCursor::row_type & r2,
	LCursor::col_type & c1, LCursor::col_type & c2)
{
	InsetBase * p = i1.inset();
	c1 = p->col(i1.idx_);
	c2 = p->col(i2.idx_);
	if (c1 > c2)
		swap(c1, c2);
	r1 = p->row(i1.idx_);
	r2 = p->row(i2.idx_);
	if (r1 > r2)
		swap(r1, r2);
}

}


string LCursor::grabSelection()
{
	if (!selection())
		return string();

	CursorSlice i1 = selBegin();
	CursorSlice i2 = selEnd();

	if (i1.idx_ == i2.idx_) {
		if (i1.inset()->asMathInset()) {
			MathArray::const_iterator it = i1.cell().begin();
			return asString(MathArray(it + i1.pos_, it + i2.pos_));
		} else {
			return "unknown selection 1";
		}
	}

	row_type r1, r2;
	col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);

	string data;
	if (i1.inset()->asMathInset()) {
		for (row_type row = r1; row <= r2; ++row) {
			if (row > r1)
				data += "\\\\";
			for (col_type col = c1; col <= c2; ++col) {
				if (col > c1)
					data += '&';
				data += asString(i1.asMathInset()->cell(i1.asMathInset()->index(row, col)));
			}
		}
	} else {
		data = "unknown selection 2";
	}
	return data;
}


void LCursor::eraseSelection()
{
	CursorSlice i1 = selBegin();
	CursorSlice i2 = selEnd();
#warning FIXME
	if (i1.inset()->asMathInset()) {
		if (i1.idx_ == i2.idx_) {
			i1.cell().erase(i1.pos_, i2.pos_);
		} else {
			MathInset * p = i1.asMathInset();
			row_type r1, r2;
			col_type c1, c2;
			region(i1, i2, r1, r2, c1, c2);
			for (row_type row = r1; row <= r2; ++row)
				for (col_type col = c1; col <= c2; ++col)
					p->cell(p->index(row, col)).clear();
		}
		current() = i1;
	} else {
		lyxerr << "can't erase this selection 1" << endl;
	}
}


string LCursor::grabAndEraseSelection()
{
	if (!selection())
		return string();
	string res = grabSelection();
	eraseSelection();
	selection() = false;
	return res;
}


void LCursor::selClear()
{
	resetAnchor();
	clearSelection();
}


void LCursor::selCopy()
{
	if (selection()) {
		theCutBuffer.push(grabSelection());
		selection() = false;
	} else {
		//theCutBuffer.erase();
	}
}


void LCursor::selCut()
{
	theCutBuffer.push(grabAndEraseSelection());
}


void LCursor::selDel()
{
	if (selection()) {
		eraseSelection();
		selection() = false;
	}
}


void LCursor::selPaste(size_t n)
{
	selClearOrDel();
	if (n < theCutBuffer.size())
		paste(theCutBuffer[n]);
	//grabSelection();
	selection() = false;
}


void LCursor::selHandle(bool sel)
{
	if (sel == selection())
		return;
	resetAnchor();
	selection() = sel;
}


void LCursor::selStart()
{
	resetAnchor();
	selection() = true;
}


void LCursor::selClearOrDel()
{
	if (lyxrc.auto_region_delete)
		selDel();
	else
		selection() = false;
}


std::ostream & operator<<(std::ostream & os, LCursor const & cur)
{
	os << "\n";
	for (size_t i = 0, n = cur.cursor_.size(); i != n; ++i)
		os << "  (" << cur.cursor_[i] << " | " << cur.anchor_[i] << "\n";
	return os << "current: " << cur.current_ << endl;
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
	while (top.idx() < top.lastidx()) {
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
	CursorSlice & cur = it.back();
	cur.idx() = cur.lastidx();
	cur.pos() = cur.lastpos();
	return it;
}




///////////////////////////////////////////////////////////////////
//
// The part below is the non-integrated rest of the original math
// cursor. This should be either generalized for texted or moved
// back to the math insets.
//
///////////////////////////////////////////////////////////////////

#include "mathed/math_braceinset.h"
#include "mathed/math_charinset.h"
#include "mathed/math_commentinset.h"
#include "mathed/math_factory.h"
#include "mathed/math_gridinset.h"
#include "mathed/math_macroarg.h"
#include "mathed/math_macrotemplate.h"
#include "mathed/math_mathmlstream.h"
#include "mathed/math_scriptinset.h"
#include "mathed/math_spaceinset.h"
#include "mathed/math_support.h"
#include "mathed/math_unknowninset.h"

//#define FILEDEBUG 1


bool LCursor::isInside(InsetBase const * p)
{
	for (unsigned i = 0; i < depth(); ++i)
		if (cursor_[i].inset() == p)
			return true;
	return false;
}


bool LCursor::openable(MathAtom const & t)
{
	if (!t->isActive())
		return false;

	if (t->lock())
		return false;

	if (!selection())
		return true;

	// we can't move into anything new during selection
	if (depth() == anchor_.size())
		return false;
	if (t.nucleus() != anchor_[depth()].inset())
		return false;

	return true;
}


bool LCursor::inNucleus()
{
	return inset()->asMathInset()->asScriptInset() && idx() == 2;
}


bool LCursor::left()
{
	autocorrect() = false;
	clearTargetX();
	if (inMacroMode()) {
		macroModeClose();
		return true;
	}

	if (pos() != 0 && openable(prevAtom())) {
		posLeft();
		push(nextAtom().nucleus());
		inset()->idxLast(*this);
		return true;
	}

	return posLeft() || idxLeft() || popLeft() || selection();
}


bool LCursor::right()
{
	autocorrect() = false;
	clearTargetX();
	if (inMacroMode()) {
		macroModeClose();
		return true;
	}

	if (pos() != lastpos() && openable(nextAtom())) {
		pushLeft(nextAtom().nucleus());
		inset()->idxFirst(*this);
		return true;
	}

	return posRight() || idxRight() || popRight() || selection();
}


bool positionable(CursorBase const & cursor, CursorBase const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.size() > anchor.size())
		return false;

	// anchor might be deeper, should have same path then
	for (size_t i = 0; i < cursor.size(); ++i)
		if (cursor[i].inset() != anchor[i].inset())
			return false;

	// position should be ok.
	return true;
}


void LCursor::setScreenPos(int x, int y)
{
	bool res = bruteFind(x, y, formula()->xlow(), formula()->xhigh(),
		formula()->ylow(), formula()->yhigh());
	if (!res) {
		// this can happen on creation of "math-display"
		idx() = 0;
		pos() = 0;
	}
	clearTargetX();
}



bool LCursor::home()
{
	autocorrect() = false;
	macroModeClose();
	if (!inset()->idxHome(*this))
		return popLeft();
	clearTargetX();
	return true;
}


bool LCursor::end()
{
	autocorrect() = false;
	macroModeClose();
	if (!inset()->idxEnd(*this))
		return popRight();
	clearTargetX();
	return true;
}


void LCursor::plainErase()
{
	cell().erase(pos());
}


void LCursor::markInsert()
{
	cell().insert(pos(), MathAtom(new MathCharInset(0)));
}


void LCursor::markErase()
{
	cell().erase(pos());
}


void LCursor::plainInsert(MathAtom const & t)
{
	cell().insert(pos(), t);
	++pos();
}


void LCursor::insert2(string const & str)
{
	MathArray ar;
	asArray(str, ar);
	insert(ar);
}


void LCursor::insert(string const & str)
{
	lyxerr << "inserting '" << str << "'" << endl;
	selClearOrDel();
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		plainInsert(MathAtom(new MathCharInset(*it)));
}


void LCursor::insert(char c)
{
	lyxerr << "inserting '" << c << "'" << endl;
	selClearOrDel();
	plainInsert(MathAtom(new MathCharInset(c)));
}


void LCursor::insert(MathAtom const & t)
{
	macroModeClose();
	selClearOrDel();
	plainInsert(t);
}


void LCursor::niceInsert(string const & t)
{
	lyxerr << "*** LCursor::niceInsert 1: " << t << endl;
	MathArray ar;
	asArray(t, ar);
	if (ar.size() == 1)
		niceInsert(ar[0]);
	else
		insert(ar);
}


void LCursor::niceInsert(MathAtom const & t)
{
	lyxerr << "*** LCursor::niceInsert 2: " << t << endl;
	macroModeClose();
	string safe = grabAndEraseSelection();
	plainInsert(t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posLeft();
		pushLeft(nextAtom().nucleus());
		paste(safe);
	}
	lyxerr << "*** LCursor::niceInsert 3: " << t << endl;
}


void LCursor::insert(MathArray const & ar)
{
	macroModeClose();
	if (selection())
		eraseSelection();
	cell().insert(pos(), ar);
	pos() += ar.size();
}


bool LCursor::backspace()
{
	autocorrect() = false;

	if (selection()) {
		selDel();
		return true;
	}

	if (pos() == 0) {
		if (inset()->nargs() == 1 && depth() == 1 && lastpos() == 0)
			return false;
		pullArg();
		return true;
	}

	if (inMacroMode()) {
		MathUnknownInset * p = activeMacro();
		if (p->name().size() > 1) {
			p->setName(p->name().substr(0, p->name().size() - 1));
			return true;
		}
	}

	if (pos() != 0 && prevAtom()->nargs() > 0) {
		// let's require two backspaces for 'big stuff' and
		// highlight on the first
		selection() = true;
		left();
	} else {
		--pos();
		plainErase();
	}
	return true;
}


bool LCursor::erase()
{
	autocorrect() = false;
	if (inMacroMode())
		return true;

	if (selection()) {
		selDel();
		return true;
	}

	// delete empty cells if possible
	if (pos() == lastpos() && inset()->idxDelete(idx()))
		return true;

	// special behaviour when in last position of cell
	if (pos() == lastpos()) {
		bool one_cell = inset()->nargs() == 1;
		if (one_cell && depth() == 1 && lastpos() == 0)
			return false;
		// remove markup
		if (one_cell)
			pullArg();
		else
			inset()->idxGlue(idx());
		return true;
	}

	if (pos() != lastpos() && inset()->nargs() > 0) {
		selection() = true;
		right();
	} else {
		plainErase();
	}

	return true;
}


bool LCursor::up()
{
	macroModeClose();
	CursorBase save = cursor_;
	if (goUpDown(true))
		return true;
	cursor_ = save;
	autocorrect() = false;
	return selection();
}


bool LCursor::down()
{
	macroModeClose();
	CursorBase save = cursor_;
	if (goUpDown(false))
		return true;
	cursor_ = save;
	autocorrect() = false;
	return selection();
}


void LCursor::macroModeClose()
{
	if (!inMacroMode())
		return;
	MathUnknownInset * p = activeMacro();
	p->finalize();
	string s = p->name();
	--pos();
	cell().erase(pos());

	// do nothing if the macro name is empty
	if (s == "\\")
		return;

	string const name = s.substr(1);

	// prevent entering of recursive macros
	if (formula()->lyxCode() == InsetOld::MATHMACRO_CODE
			&& formula()->getInsetName() == name)
		lyxerr << "can't enter recursive macro" << endl;

	niceInsert(createMathInset(name));
}


string LCursor::macroName()
{
	return inMacroMode() ? activeMacro()->name() : string();
}


void LCursor::handleNest(MathAtom const & a, int c)
{
	MathAtom t = a;
	asArray(grabAndEraseSelection(), t.nucleus()->cell(c));
	insert(t);
	posLeft();
	pushLeft(t.nucleus());
}


int LCursor::targetX() const
{
	if (x_target() != -1)
		return x_target();
	int x = 0;
	int y = 0;
	getPos(x, y);
	return x;
}


MathHullInset * LCursor::formula() const
{
	for (int i = cursor_.size() - 1; i >= 1; --i)
		if (cursor_[i].inset()->lyxCode() == InsetBase::MATH_CODE)
			return static_cast<MathHullInset *>(cursor_[i].inset());
	return 0;
}


void LCursor::adjust(pos_type from, int diff)
{
	if (pos() > from)
		pos() += diff;
	if (anchor().pos_ > from)
		anchor().pos_ += diff;
	// just to be on the safe side
	// theoretically unecessary
	normalize();
}


bool LCursor::inMacroMode() const
{
	if (!pos() != 0)
		return false;
	MathUnknownInset const * p = prevAtom()->asUnknownInset();
	return p && !p->final();
}


MathUnknownInset * LCursor::activeMacro()
{
	return inMacroMode() ? prevAtom().nucleus()->asUnknownInset() : 0;
}


bool LCursor::inMacroArgMode() const
{
	return pos() > 0 && prevAtom()->getChar() == '#';
}


MathGridInset * LCursor::enclosingGrid(idx_type & idx) const
{
	for (MathInset::difference_type i = depth() - 1; i >= 0; --i) {
		MathInset * m = cursor_[i].inset()->asMathInset();
		if (!m)
			return 0;
		MathGridInset * p = m->asGridInset();
		if (p) {
			idx = cursor_[i].idx_;
			return p;
		}
	}
	return 0;
}


void LCursor::pullArg()
{
#warning look here
#if 0
	MathArray ar = cell();
	if (popLeft()) {
		plainErase();
		cell().insert(pos(), ar);
		resetAnchor();
	} else {
		formula()->mutateToText();
	}
#endif
}


void LCursor::touch()
{
#warning look here
#if 0
	CursorBase::const_iterator it = cursor_.begin();
	CursorBase::const_iterator et = cursor_.end();
	for ( ; it != et; ++it)
		it->cell().touch();
#endif
}


void LCursor::normalize()
{
	if (idx() >= nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << ' ' << nargs()
		       << " in: " << inset() << endl;
	}
	idx() = min(idx(), lastidx());

	if (pos() > lastpos()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << ' ' << lastpos() <<  " in idx: " << idx()
		       << " in atom: '";
		WriteStream wi(lyxerr, false, true);
		inset()->asMathInset()->write(wi);
		lyxerr << endl;
	}
	pos() = min(pos(), lastpos());
}


char LCursor::valign()
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(idx);
	return p ? p->valign() : '\0';
}


char LCursor::halign()
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(idx);
	return p ? p->halign(idx % p->ncols()) : '\0';
}


bool LCursor::goUpDown(bool up)
{
	// Be warned: The 'logic' implemented in this function is highly fragile.
	// A distance of one pixel or a '<' vs '<=' _really_ matters.
	// So fiddle around with it only if you know what you are doing!
  int xo = 0;
	int yo = 0;
	getPos(xo, yo);

	// check if we had something else in mind, if not, this is the future goal
	if (x_target() == -1)
		x_target() = xo;
	else
		xo = x_target();

	// try neigbouring script insets
	if (!selection()) {
		// try left
		if (pos() != 0) {
			MathScriptInset const * p = prevAtom()->asScriptInset();
			if (p && p->has(up)) {
				--pos();
				push(inset());
				idx() = up; // the superscript has index 1
				pos() = lastpos();
				//lyxerr << "updown: handled by scriptinset to the left" << endl;
				return true;
			}
		}

		// try right
		if (pos() != lastpos()) {
			MathScriptInset const * p = nextAtom()->asScriptInset();
			if (p && p->has(up)) {
				push(inset());
				idx() = up;
				pos() = 0;
				//lyxerr << "updown: handled by scriptinset to the right" << endl;
				return true;
			}
		}
	}

	// try current cell for e.g. text insets
	if (inset()->idxUpDown2(*this, up))
		return true;

	//xarray().boundingBox(xlow, xhigh, ylow, yhigh);
	//if (up)
	//	yhigh = yo - 4;
	//else
	//	ylow = yo + 4;
	//if (bruteFind(xo, yo, xlow, xhigh, ylow, yhigh)) {
	//	lyxerr << "updown: handled by brute find in the same cell" << endl;
	//	return true;
	//}

	// try to find an inset that knows better then we
	while (1) {
		//lyxerr << "updown: We are in " << inset() << " idx: " << idx() << endl;
		// ask inset first
		if (inset()->idxUpDown(*this, up)) {
			// try to find best position within this inset
			if (!selection())
				bruteFind2(xo, yo);
			return true;
		}

		// no such inset found, just take something "above"
		//lyxerr << "updown: handled by strange case" << endl;
		if (!popLeft()) {
			return
				bruteFind(xo, yo,
					formula()->xlow(),
					formula()->xhigh(),
					up ? formula()->ylow() : yo + 4,
					up ? yo - 4 : formula()->yhigh()
				);
		}

		// any improvement so far?
		int xnew, ynew;
		getPos(xnew, ynew);
		if (up ? ynew < yo : ynew > yo)
			return true;
	}
}


bool LCursor::bruteFind(int x, int y, int xlow, int xhigh, int ylow, int yhigh)
{
	CursorBase best_cursor;
	double best_dist = 1e10;

	CursorBase it = ibegin(formula());
	CursorBase et = iend(formula());
	while (1) {
		// avoid invalid nesting when selecting
		if (!selection() || positionable(it, anchor_)) {
			int xo, yo;
			CursorSlice & cur = it.back();
			cur.inset()->getCursorPos(cur, xo, yo);
			if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
				double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
				//lyxerr << "x: " << x << " y: " << y << " d: " << endl;
				// '<=' in order to take the last possible position
				// this is important for clicking behind \sum in e.g. '\sum_i a'
				if (d <= best_dist) {
					best_dist   = d;
					best_cursor = it;
				}
			}
		}

		if (it == et)
			break;
		increment(it);
	}

	if (best_dist < 1e10)
		cursor_ = best_cursor;
	return best_dist < 1e10;
}


void LCursor::bruteFind2(int x, int y)
{
	double best_dist = 1e10;

	CursorBase it = cursor_;
	it.back().pos(0);
	CursorBase et = cursor_;
	int n = et.back().asMathInset()->cell(et.back().idx_).size();
	et.back().pos(n);
	for (int i = 0; ; ++i) {
		int xo, yo;
		CursorSlice & cur = it.back();
		cur.inset()->getCursorPos(cur, xo, yo);
		double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
		// '<=' in order to take the last possible position
		// this is important for clicking behind \sum in e.g. '\sum_i a'
		lyxerr << "i: " << i << " d: " << d << " best: " << best_dist << endl;
		if (d <= best_dist) {
			best_dist = d;
			cursor_ = it;
		}
		if (it == et)
			break;
		increment(it);
	}
}


bool LCursor::idxLineLast()
{
	idx() -= idx() % ncols();
	idx() += ncols() - 1;
	pos() = lastpos();
	return true;
}


bool LCursor::idxLeft()
{
	return inset()->idxLeft(*this);
}


bool LCursor::idxRight()
{
	return inset()->idxRight(*this);
}


bool LCursor::script(bool up)
{
	// Hack to get \\^ and \\_ working
	lyxerr << "handling script: up: " << up << endl;
	if (inMacroMode() && macroName() == "\\") {
		if (up)
			niceInsert(createMathInset("mathcircumflex"));
		else
			interpret('_');
		return true;
	}

	macroModeClose();
	string safe = grabAndEraseSelection();
	if (inNucleus()) {
		// we are in a nucleus of a script inset, move to _our_ script
		inset()->asMathInset()->asScriptInset()->ensure(up);
		idx() = up;
		pos() = 0;
	} else if (pos() != 0 && prevAtom()->asScriptInset()) {
		--pos();
		nextAtom().nucleus()->asScriptInset()->ensure(up);
		push(nextInset());
		idx() = up;
		pos() = lastpos();
	} else if (pos() != 0) {
		--pos();
		cell()[pos()] = MathAtom(new MathScriptInset(nextAtom(), up));
		push(nextInset());
		idx() = up;
		pos() = 0;
	} else {
		plainInsert(MathAtom(new MathScriptInset(up)));
		--pos();
		nextAtom().nucleus()->asScriptInset()->ensure(up);
		push(nextInset());
		idx() = up;
		pos() = 0;
	}
	paste(safe);
	return true;
}


bool LCursor::interpret(char c)
{
	//lyxerr << "interpret 2: '" << c << "'" << endl;
	clearTargetX();
	if (inMacroArgMode()) {
		posLeft();
		plainErase();
#warning FIXME
#if 0
		int n = c - '0';
		MathMacroTemplate const * p = formula()->asMacroTemplate();
		if (p && 1 <= n && n <= p->numargs())
			insert(MathAtom(new MathMacroArgument(c - '0')));
		else {
			insert(createMathInset("#"));
			interpret(c); // try again
		}
#endif
		return true;
	}

	// handle macroMode
	if (inMacroMode()) {
		string name = macroName();
		//lyxerr << "interpret name: '" << name << "'" << endl;

		if (isalpha(c)) {
			activeMacro()->setName(activeMacro()->name() + c);
			return true;
		}

		// handle 'special char' macros
		if (name == "\\") {
			// remove the '\\'
			backspace();
			if (c == '\\') {
				if (currentMode() == MathInset::TEXT_MODE)
					niceInsert(createMathInset("textbackslash"));
				else
					niceInsert(createMathInset("backslash"));
			} else if (c == '{') {
				niceInsert(MathAtom(new MathBraceInset));
			} else {
				niceInsert(createMathInset(string(1, c)));
			}
			return true;
		}

		// leave macro mode and try again if necessary
		macroModeClose();
		if (c == '{')
			niceInsert(MathAtom(new MathBraceInset));
		else if (c != ' ')
			interpret(c);
		return true;
	}

	// This is annoying as one has to press <space> far too often.
	// Disable it.

	if (0) {
		// leave autocorrect mode if necessary
		if (autocorrect() && c == ' ') {
			autocorrect() = false;
			return true;
		}
	}

	// just clear selection on pressing the space bar
	if (selection() && c == ' ') {
		selection() = false;
		return true;
	}

	selClearOrDel();

	if (c == '\\') {
		//lyxerr << "starting with macro" << endl;
		insert(MathAtom(new MathUnknownInset("\\", false)));
		return true;
	}

	if (c == '\n') {
		if (currentMode() == MathInset::TEXT_MODE)
			insert(c);
		return true;
	}

	if (c == ' ') {
		if (currentMode() == MathInset::TEXT_MODE) {
			// insert spaces in text mode,
			// but suppress direct insertion of two spaces in a row
			// the still allows typing  '<space>a<space>' and deleting the 'a', but
			// it is better than nothing...
			if (!pos() != 0 || prevAtom()->getChar() != ' ')
				insert(c);
			return true;
		}
		if (pos() != 0 && prevAtom()->asSpaceInset()) {
			prevAtom().nucleus()->asSpaceInset()->incSpace();
			return true;
		}
		if (popRight())
			return true;
		// if are at the very end, leave the formula
		return pos() != lastpos();
	}

	if (c == '_') {
		script(false);
		return true;
	}

	if (c == '^') {
		script(true);
		return true;
	}

	if (c == '{' || c == '}' || c == '#' || c == '&' || c == '$') {
		niceInsert(createMathInset(string(1, c)));
		return true;
	}

	if (c == '%') {
		niceInsert(MathAtom(new MathCommentInset));
		return true;
	}

	// try auto-correction
	//if (autocorrect() && hasPrevAtom() && math_autocorrect(prevAtom(), c))
	//	return true;

	// no special circumstances, so insert the character without any fuss
	insert(c);
	autocorrect() = true;
	return true;
}


void LCursor::lockToggle()
{
	if (pos() != lastpos()) {
		// toggle previous inset ...
		nextAtom().nucleus()->lock(!nextAtom()->lock());
	} else if (popLeft() && pos() != lastpos()) {
		// ... or enclosing inset if we are in the last inset position
		nextAtom().nucleus()->lock(!nextAtom()->lock());
		posRight();
	}
}


CursorSlice LCursor::normalAnchor()
{
	if (anchor_.size() < depth()) {
		resetAnchor();
		lyxerr << "unusual Anchor size" << endl;
	}
	//lyx::BOOST_ASSERT(Anchor_.size() >= cursor.depth());
	// use Anchor on the same level as Cursor
	CursorSlice normal = anchor_[current_];
#if 0
	if (depth() < anchor_.size() && !(normal < xx())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
#endif
	return normal;
}


/*
DispatchResult dispatch(LCursor & cur, FuncRequest const & cmd)
{
	// mouse clicks are somewhat special
	// check
	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_DOUBLE: {
		CursorSlice & pos = cursor_.back();
		int x = 0;
		int y = 0;
		getPos(x, y);
		if (x < cmd.x && pos() != 0) {
			DispatchResult const res = prevAtom().nucleus()->dispatch(cmd);
			if (res.dispatched())
				return res;
		}
		if (x > cmd.x && pos() != lastpos()) {
			DispatchResult const res = inset()->dispatch(cmd);
			if (res.dispatched())
				return res;
		}
	}
	default:
	break;
	}
}
*/


void LCursor::handleFont(string const & font)
{
	string safe;
	if (selection()) {
		macroModeClose();
		safe = grabAndEraseSelection();
	}

	if (lastpos() != 0) {
		// something left in the cell
		if (pos() == 0) {
			// cursor in first position
			popLeft();
		} else if (pos() == lastpos()) {
			// cursor in last position
			popRight();
		} else {
			// cursor in between. split cell
			MathArray::iterator bt = cell().begin();
			MathAtom at = createMathInset(font);
			at.nucleus()->cell(0) = MathArray(bt, bt + pos());
			cell().erase(bt, bt + pos());
			popLeft();
			plainInsert(at);
		}
	} else {
		// nothing left in the cell
		pullArg();
		plainErase();
	}
	insert(safe);
}


void LCursor::releaseMathCursor()
{
	if (inMathed())
		formula()->insetUnlock(bv());
}


bool LCursor::inMathed() const
{
	return formula();
}


InsetBase * LCursor::nextInset()
{
	if (pos() == lastpos())
		return 0;
	if (inMathed()) 
		return nextAtom().nucleus();
	Paragraph & par = paragraph();
	return par.isInset(pos()) ? par.getInset(pos()) : 0;
}


InsetBase * LCursor::prevInset()
{
	if (pos() == 0)
		return 0;
	if (inMathed()) 
		return prevAtom().nucleus();
	Paragraph & par = paragraph();
	return par.isInset(pos() - 1) ? par.getInset(pos() - 1) : 0;
}


void LCursor::message(string const & msg) const
{
	bv().owner()->getLyXFunc().setMessage(msg);
}


void LCursor::errorMessage(string const & msg) const
{
	bv().owner()->getLyXFunc().setErrorMessage(msg);
}
