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
#include "support/std_sstream.h"

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
	: DocumentIterator(bv), anchor_(bv),
	  cached_y_(0), x_target_(-1), selection_(false), mark_(false)
{}


void LCursor::reset()
{
	clear();
	push_back(CursorSlice());
	anchor_.clear();
	anchor_.push_back(CursorSlice());
	cached_y_ = 0;
	clearTargetX();
	selection_ = false;
	mark_ = false;
}


void LCursor::setCursor(DocumentIterator const & cur, bool sel)
{
	// this (intentionally) does not touch the anchor
	DocumentIterator::operator=(cur);
	selection() = sel;
}


DispatchResult LCursor::dispatch(FuncRequest const & cmd0)
{
	lyxerr << "\nLCursor::dispatch: cmd: " << cmd0 << endl << *this << endl;
	BOOST_ASSERT(pos() <= lastpos());
	BOOST_ASSERT(idx() <= lastidx());
	BOOST_ASSERT(par() <= lastpar());
	FuncRequest cmd = cmd0;
	nopop_ = false;
	DocumentIterator orig = *this;
	disp_.update(true);
	disp_.val(NONE);
	while (size() != 1) {
		// the inset's dispatch() is supposed to reset the update and
		// val flags if necessary 
		inset()->dispatch(*this, cmd);
		
		// "Mutate" the request for semi-handled requests that need
		// additional handling in outer levels.
		switch (disp_.val()) {
			case NONE:
				// the inset handled the event fully
				return DispatchResult(true, true);
			case FINISHED_LEFT:
				// the inset handled the event partially
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
				//lyxerr << "not handled on level " << depth()
				//	<< " val: " << disp_.val() << endl;
				break;
		}
	}
	bv().text()->dispatch(*this, cmd);
	if (nopop_)
		setCursor(orig, false);
	//lyxerr << "   result: " << res.val() << endl;
	return disp_;
}


bool LCursor::getStatus(FuncRequest const & cmd, FuncStatus & status)
{
	lyxerr << "\nLCursor::getStatus: cmd: " << cmd << endl << *this << endl;
	DocumentIterator orig = *this;
	BOOST_ASSERT(pos() <= lastpos());
	BOOST_ASSERT(idx() <= lastidx());
	BOOST_ASSERT(par() <= lastpar());
	for ( ; size() != 0; pop_back()) {
		// the inset's getStatus() will return 'true' if it made
		// a definitive decision on whether it want to handle the
		// request or not. The result of this decision is put into
		// the 'status' parameter.
		bool const res = inset()->getStatus(*this, cmd, status);
		if (res) {
			setCursor(orig, false);
			return true;
		}
	}
	bool const res = bv().text()->getStatus(*this, cmd, status);
	setCursor(orig, false);
	return res;
}


BufferView & LCursor::bv() const
{
	return DocumentIterator::bv();
}


/*
void LCursor::pop(int depth)
{
	while (int(size()) > depth + 1)
		pop();
	lyxerr << "LCursor::pop() result: " << *this << endl;
}
*/


void LCursor::pop()
{
	BOOST_ASSERT(size() >= 1);
	pop_back();
	anchor_.pop_back();
}


void LCursor::push(InsetBase * p)
{
	push_back(CursorSlice(p));
}


void LCursor::pushLeft(InsetBase * p)
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Entering inset " << t << " left" << endl;
	push(p);
	p->idxFirst(*this);
}


bool LCursor::popLeft()
{
	BOOST_ASSERT(!empty());
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
	BOOST_ASSERT(!empty());
	//lyxerr << "Leaving inset to the right" << endl;
	if (depth() <= 1) {
		if (depth() == 1)
			inset()->notifyCursorLeaves(idx());
		return false;
	}
	inset()->notifyCursorLeaves(idx());
	pop();
	++pos();
	return true;
}


int LCursor::currentMode()
{
	BOOST_ASSERT(!empty());
	for (int i = size() - 1; i >= 1; --i) {
		int res = operator[](i).inset()->currentMode();
		if (res != MathInset::UNDECIDED_MODE)
			return res;
	}
	return MathInset::TEXT_MODE;
}


void LCursor::updatePos()
{
	BOOST_ASSERT(!empty());
	if (size() > 1)
		cached_y_ = bv().top_y() + back().inset()->yo();
		//cached_y_ = back().inset()->yo();
}


void LCursor::getDim(int & asc, int & des) const
{
	BOOST_ASSERT(!empty());
	if (inMathed()) {
		BOOST_ASSERT(inset());
		BOOST_ASSERT(inset()->asMathInset());
		//inset()->asMathInset()->getCursorDim(asc, des);
		asc = 10;
		des = 10;
	} else {
		Row const & row = textRow();
		asc = row.baseline();
		des = row.height() - asc;
	}
}


void LCursor::getPos(int & x, int & y) const
{
	BOOST_ASSERT(!empty());
	x = 0;
	y = 0;
	if (size() == 1) {
		x = bv().text()->cursorX(front());
		y = bv().text()->cursorY(front());
	} else {
		if (!inset()) {
			lyxerr << "#### LCursor::getPos: " << *this << endl;
			BOOST_ASSERT(inset());
		}
		inset()->getCursorPos(back(), x, y);
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


void LCursor::resetAnchor()
{
	anchor_ = *this;
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
		return back();
	return anchor() < back() ? anchor() : back();
}


CursorSlice & LCursor::selBegin()
{
	if (!selection())
		return back();
	// can't use std::min as this returns a const ref
	return anchor() < back() ? anchor() : back();
}


CursorSlice const & LCursor::selEnd() const
{
	if (!selection())
		return back();
	return anchor() > back() ? anchor() : back();
}


CursorSlice & LCursor::selEnd()
{
	if (!selection())
		return back();
	// can't use std::min as this returns a const ref
	return anchor() > back() ? anchor() : back();
}


void LCursor::setSelection()
{
	selection() = true;
	// a selection with no contents is not a selection
	if (par() == anchor().par() && pos() == anchor().pos())
		selection() = false;
}


void LCursor::setSelection(DocumentIterator const & where, size_t n)
{
	selection() = true;
	setCursor(where, false);
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



void LCursor::info(std::ostream & os) const
{
	for (int i = 1, n = depth(); i < n; ++i) {
		operator[](i).inset()->infoize(os);
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
	//lyxerr << "LCursor::eraseSelection" << endl;
	CursorSlice const & i1 = selBegin();
	CursorSlice const & i2 = selEnd();
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
		back() = i1;
	} else {
		lyxerr << "can't erase this selection 1" << endl;
	}
	//lyxerr << "LCursor::eraseSelection end" << endl;
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
	//lyxerr << "LCursor::selDel" << endl;
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
	//lyxerr << "LCursor::selHandle" << endl;
	if (sel == selection())
		return;
	resetAnchor();
	selection() = sel;
}


void LCursor::selClearOrDel()
{
	//lyxerr << "LCursor::selClearOrDel" << endl;
	if (lyxrc.auto_region_delete)
		selDel();
	else
		selection() = false;
}


std::ostream & operator<<(std::ostream & os, LCursor const & cur)
{
	for (size_t i = 0, n = cur.size(); i != n; ++i)
		os << " " << cur.operator[](i) << " | " << cur.anchor_[i] << "\n";
	os << " selection: " << cur.selection_ << endl;
	return os;
}




///////////////////////////////////////////////////////////////////
//
// The part below is the non-integrated rest of the original math
// cursor. This should be either generalized for texted or moved
// back to mathed (in most cases to MathNestInset).
//
///////////////////////////////////////////////////////////////////

#include "mathed/math_charinset.h"
#include "mathed/math_factory.h"
#include "mathed/math_gridinset.h"
#include "mathed/math_macroarg.h"
#include "mathed/math_macrotemplate.h"
#include "mathed/math_mathmlstream.h"
#include "mathed/math_scriptinset.h"
#include "mathed/math_support.h"
#include "mathed/math_unknowninset.h"

//#define FILEDEBUG 1


bool LCursor::isInside(InsetBase const * p)
{
	for (unsigned i = 0; i < depth(); ++i)
		if (operator[](i).inset() == p)
			return true;
	return false;
}


bool LCursor::openable(MathAtom const & t) const
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
	if (!ptr_cmp(t.nucleus(), anchor_[depth()].inset()))
		return false;

	return true;
}


bool positionable(DocumentIterator const & cursor,
	DocumentIterator const & anchor)
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


void LCursor::insert(string const & str)
{
	lyxerr << "LCursor::insert str '" << str << "'" << endl;
	selClearOrDel();
#if 0
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		plainInsert(MathAtom(new MathCharInset(*it)));
#else
	MathArray ar;
	asArray(str, ar);
	insert(ar);
#endif
}


void LCursor::insert(char c)
{
	//lyxerr << "LCursor::insert char '" << c << "'" << endl;
	selClearOrDel();
	plainInsert(MathAtom(new MathCharInset(c)));
}


void LCursor::insert(MathAtom const & t)
{
	//lyxerr << "LCursor::insert MathAtom: " << endl;
	macroModeClose();
	selClearOrDel();
	plainInsert(t);
}


void LCursor::insert(InsetBase * inset)
{
	if (inMathed())
		insert(MathAtom(inset));
	else
		text()->insertInset(*this, inset);
}


void LCursor::niceInsert(string const & t)
{
	MathArray ar;
	asArray(t, ar);
	if (ar.size() == 1)
		niceInsert(ar[0]);
	else
		insert(ar);
}


void LCursor::niceInsert(MathAtom const & t)
{
	macroModeClose();
	string safe = grabAndEraseSelection();
	plainInsert(t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posLeft();
		// be careful here: don't use 'pushLeft(t)' as this we need to
		// push the clone, not the original
		pushLeft(nextAtom().nucleus());
		paste(safe);
	}
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
		--pos();
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
		++pos();
	} else {
		plainErase();
	}

	return true;
}


bool LCursor::up()
{
	macroModeClose();
	DocumentIterator save = *this;
	if (goUpDown(true))
		return true;
	setCursor(save, false);
	autocorrect() = false;
	return selection();
}


bool LCursor::down()
{
	macroModeClose();
	DocumentIterator save = *this;
	if (goUpDown(false))
		return true;
	setCursor(save, false);
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
	InsetBase const * macro = innerInsetOfType(InsetBase::MATHMACRO_CODE);
	if (macro && macro->getInsetName() == name)
		lyxerr << "can't enter recursive macro" << endl;

	niceInsert(createMathInset(name));
}


string LCursor::macroName()
{
	return inMacroMode() ? activeMacro()->name() : string();
}


void LCursor::handleNest(MathAtom const & a, int c)
{
	//lyxerr << "LCursor::handleNest: " << c << endl;
	MathAtom t = a;
	asArray(grabAndEraseSelection(), t.nucleus()->cell(c));
	insert(t);
	posLeft();
	pushLeft(nextAtom().nucleus());
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
	for (int i = size() - 1; i >= 1; --i) {
		MathInset * inset = operator[](i).inset()->asMathInset();
		if (inset && inset->asHullInset())
			return static_cast<MathHullInset *>(inset);
	}
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
		MathInset * m = operator[](i).inset()->asMathInset();
		if (!m)
			return 0;
		MathGridInset * p = m->asGridInset();
		if (p) {
			idx = operator[](i).idx_;
			return p;
		}
	}
	return 0;
}


void LCursor::pullArg()
{
#warning Look here
	MathArray ar = cell();
	if (popLeft() && inMathed()) {
		plainErase();
		cell().insert(pos(), ar);
		resetAnchor();
	} else {
		//formula()->mutateToText();
	}
}


void LCursor::touch()
{
#warning look here
#if 0
	DocumentIterator::const_iterator it = begin();
	DocumentIterator::const_iterator et = end();
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
	// Be warned: The 'logic' implemented in this function is highly
	// fragile. A distance of one pixel or a '<' vs '<=' _really
	// matters. So fiddle around with it only if you think you know
	// what you are doing!
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
	DocumentIterator best_cursor(bv());
	double best_dist = 1e10;

	DocumentIterator it = bufferBegin(bv());
	DocumentIterator et = bufferEnd();
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
		it.forwardPos();
	}

	if (best_dist < 1e10)
		setCursor(best_cursor, false);
	return best_dist < 1e10;
}


void LCursor::bruteFind2(int x, int y)
{
	double best_dist = 1e10;

	DocumentIterator it = *this;
	it.back().pos() = 0;
	DocumentIterator et = *this;
	et.back().pos() = et.back().asMathInset()->cell(et.back().idx_).size();
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
			setCursor(it, false);
		}
		if (it == et)
			break;
		it.forwardPos();
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
	CursorSlice normal = anchor_[size() - 1];
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
		CursorSlice & pos = back();
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
	lyxerr << "LCursor::handleFont: " << font << endl;
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


void LCursor::message(string const & msg) const
{
	bv().owner()->getLyXFunc().setMessage(msg);
}


void LCursor::errorMessage(string const & msg) const
{
	bv().owner()->getLyXFunc().setErrorMessage(msg);
}


string LCursor::selectionAsString(bool label) const
{
	if (!selection())
		return string();

	if (inTexted()) {
		Buffer const & buffer = *bv().buffer();

		// should be const ...
		ParagraphList::iterator startpit = text()->getPar(selBegin());
		ParagraphList::iterator endpit = text()->getPar(selEnd());
		size_t const startpos = selBegin().pos();
		size_t const endpos = selEnd().pos();

		if (startpit == endpit)
			return startpit->asString(buffer, startpos, endpos, label);

		// First paragraph in selection
		string result =
			startpit->asString(buffer, startpos, startpit->size(), label) + "\n\n";

		// The paragraphs in between (if any)
		ParagraphList::iterator pit = startpit;
		for (++pit; pit != endpit; ++pit)
			result += pit->asString(buffer, 0, pit->size(), label) + "\n\n";

		// Last paragraph in selection
		result += endpit->asString(buffer, 0, endpos, label);

		return result;
	}

#warning an mathed?
	return string();
}


string LCursor::currentState()
{
	if (inMathed()) {
		std::ostringstream os;
		info(os);
		return os.str();
	}
	return text() ? text()->currentState(*this) : string();
}


// only used by the spellchecker
void LCursor::replaceWord(string const & replacestring)
{
	LyXText * t = text();
	BOOST_ASSERT(t);

	t->replaceSelectionWithString(*this, replacestring);
	t->setSelectionRange(*this, replacestring.length());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i)
		t->cursorLeft(*this);
}


void LCursor::update()
{
	bv().update();
}


string LCursor::getPossibleLabel()
{
	return inMathed() ? "eq:" : text()->getPossibleLabel(*this);
}


void LCursor::undispatched()
{
	disp_.dispatched(false);
}


void LCursor::dispatched(dispatch_result_t res)
{
	disp_.val(res);
}


void LCursor::noUpdate()
{
	disp_.update(false);
}


void LCursor::noPop()
{
	nopop_ = true;
}
