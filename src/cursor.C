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

#include "BufferView.h"
#include "buffer.h"
#include "cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "encoding.h"
#include "funcrequest.h"
#include "language.h"
#include "lfuns.h"
#include "lyxfont.h"
#include "lyxfunc.h" // only for setMessage()
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "pariterator.h"

#include "insets/updatableinset.h"
#include "insets/insettabular.h"
#include "insets/insettext.h"

#include "mathed/math_data.h"
#include "mathed/math_support.h"
#include "mathed/math_inset.h"

#include "support/limited_stack.h"
#include "support/std_sstream.h"

#include "frontends/LyXView.h"

#include <boost/assert.hpp>

using lyx::par_type;

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


namespace {

void region(CursorSlice const & i1, CursorSlice const & i2,
	LCursor::row_type & r1, LCursor::row_type & r2,
	LCursor::col_type & c1, LCursor::col_type & c2)
{
	InsetBase & p = i1.inset();
	c1 = p.col(i1.idx());
	c2 = p.col(i2.idx());
	if (c1 > c2)
		swap(c1, c2);
	r1 = p.row(i1.idx());
	r2 = p.row(i2.idx());
	if (r1 > r2)
		swap(r1, r2);
}

}


LCursor::LCursor(BufferView & bv)
	: DocIterator(), bv_(&bv), anchor_(), x_target_(-1),
	  selection_(false), mark_(false)
{}


void LCursor::reset(InsetBase & inset)
{
	clear();
	push_back(CursorSlice(inset));
	anchor_ = DocIterator(inset);
	clearTargetX();
	selection_ = false;
	mark_ = false;
}


void LCursor::setCursor(DocIterator const & cur, bool sel)
{
	// this (intentionally) does not touch the anchor
	DocIterator::operator=(cur);
	selection() = sel;
}


DispatchResult LCursor::dispatch(FuncRequest const & cmd0)
{
	lyxerr << "\nLCursor::dispatch: cmd: " << cmd0 << endl << *this << endl;
	if (empty())
		return DispatchResult();

	FuncRequest cmd = cmd0;
	LCursor safe = *this;

	for ( ; size(); pop()) {
		//lyxerr << "\nLCursor::dispatch: cmd: " << cmd0 << endl << *this << endl;
		BOOST_ASSERT(pos() <= lastpos());
		BOOST_ASSERT(idx() <= lastidx());
		BOOST_ASSERT(par() <= lastpar());

		// The common case is 'LFUN handled, need update', so make the
		// LFUN handler's life easier by assuming this as default value.
		// The handler can reset the update and val flags if necessary.
		disp_.update(true);
		disp_.dispatched(true);
		inset().dispatch(*this, cmd);
		if (disp_.dispatched())
			break;
	}
	// it completely to get a 'bomb early' behaviour in case this
	// object will be used again.
	if (!disp_.dispatched()) {
		lyxerr << "RESTORING OLD CURSOR!" << endl;
		operator=(safe);
		disp_.dispatched(false);
	}
	return disp_;
}


bool LCursor::getStatus(FuncRequest const & cmd, FuncStatus & status)
{
	// This is, of course, a mess. Better create a new doc iterator and use
	// this in Inset::getStatus. This might require an additional
	// BufferView * arg, though (which should be avoided)
	LCursor safe = *this;
	bool res = false;
	for ( ; size(); pop()) {
		//lyxerr << "\nLCursor::getStatus: cmd: " << cmd << endl << *this << endl;
		BOOST_ASSERT(pos() <= lastpos());
		BOOST_ASSERT(idx() <= lastidx());
		BOOST_ASSERT(par() <= lastpar());

		// The inset's getStatus() will return 'true' if it made
		// a definitive decision on whether it want to handle the
		// request or not. The result of this decision is put into
		// the 'status' parameter.
		if (inset().getStatus(*this, cmd, status)) {
			res = true;
			break;
		}
	}
	operator=(safe);
	return res;
}


BufferView & LCursor::bv() const
{
	return *bv_;
}


void LCursor::pop()
{
	BOOST_ASSERT(size() >= 1);
	pop_back();
}


void LCursor::push(InsetBase & p)
{
	push_back(CursorSlice(p));
}


void LCursor::pushLeft(InsetBase & p)
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Entering inset " << t << " left" << endl;
	push(p);
	p.idxFirst(*this);
}


bool LCursor::popLeft()
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Leaving inset to the left" << endl;
	inset().notifyCursorLeaves(*this);
	if (depth() == 1)
		return false;
	pop();
	return true;
}


bool LCursor::popRight()
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Leaving inset to the right" << endl;
	inset().notifyCursorLeaves(*this);
	if (depth() == 1)
		return false;
	pop();
	++pos();
	return true;
}


int LCursor::currentMode()
{
	BOOST_ASSERT(!empty());
	for (int i = size() - 1; i >= 0; --i) {
		int res = operator[](i).inset().currentMode();
		if (res != InsetBase::UNDECIDED_MODE)
			return res;
	}
	return InsetBase::TEXT_MODE;
}


void LCursor::getDim(int & asc, int & des) const
{
	if (inMathed()) {
		BOOST_ASSERT(inset().asMathInset());
		//inset().asMathInset()->getCursorDim(asc, des);
		asc = 10;
		des = 10;
	} else if (inTexted()) {
		Row const & row = textRow();
		asc = row.baseline();
		des = row.height() - asc;
	} else {
		lyxerr << "should this happen?" << endl;
		asc = 10;
		des = 10;
	}
}


void LCursor::getPos(int & x, int & y) const
{
	x = 0;
	y = 0;
	if (!empty())
		inset().getCursorPos(back(), x, y);
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


CursorSlice LCursor::anchor() const
{
	BOOST_ASSERT(anchor_.size() >= size());
	CursorSlice normal = anchor_[size() - 1];
	if (size() < anchor_.size() && back() <= normal) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos();
	}
	return normal;
}


CursorSlice LCursor::selBegin() const
{
	if (!selection())
		return back();
	return anchor() < back() ? anchor() : back();
}


CursorSlice LCursor::selEnd() const
{
	if (!selection())
		return back();
	return anchor() > back() ? anchor() : back();
}


DocIterator LCursor::selectionBegin() const
{
	if (!selection())
		return *this;
	return anchor() < back() ? anchor_ : *this;
}


DocIterator LCursor::selectionEnd() const
{
	if (!selection())
		return *this;
	return anchor() > back() ? anchor_ : *this;
}


void LCursor::setSelection()
{
	selection() = true;
	// a selection with no contents is not a selection
#warning doesnt look ok
	if (par() == anchor().par() && pos() == anchor().pos())
		selection() = false;
}


void LCursor::setSelection(DocIterator const & where, size_t n)
{
	setCursor(where, true);
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
		operator[](i).inset().infoize(os);
		os << "  ";
	}
	if (pos() != 0)
		prevInset()->infoize2(os);
	// overwite old message
	os << "                    ";
}


string LCursor::grabSelection()
{
	if (!selection())
		return string();

	CursorSlice i1 = selBegin();
	CursorSlice i2 = selEnd();

	if (i1.idx() == i2.idx()) {
		if (i1.inset().asMathInset()) {
			MathArray::const_iterator it = i1.cell().begin();
			return asString(MathArray(it + i1.pos(), it + i2.pos()));
		} else {
			return "unknown selection 1";
		}
	}

	row_type r1, r2;
	col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);

	string data;
	if (i1.inset().asMathInset()) {
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
#ifdef WITH_WARNINGS
#warning FIXME
#endif
	if (i1.inset().asMathInset()) {
		if (i1.idx() == i2.idx()) {
			i1.cell().erase(i1.pos(), i2.pos());
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
	if (sel == selection()) {
		if (!sel)
			noUpdate();
		return;
	}
	
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
	for (size_t i = 0, n = cur.size(); i != n; ++i) {
		os << " " << cur.operator[](i) << " | ";
		if (i < cur.anchor_.size())
			os << cur.anchor_[i];
		else
			os << "-------------------------------";
		os << "\n";
	}
	for (size_t i = cur.size(), n = cur.anchor_.size(); i < n; ++i) {
		os << "------------------------------- | " << cur.anchor_[i] << "\n";
	}
	os << " selection: " << cur.selection_
	   << " x_target: " << cur.x_target_ << endl;
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
#include "mathed/math_mathmlstream.h"
#include "mathed/math_scriptinset.h"
#include "mathed/math_support.h"
#include "mathed/math_unknowninset.h"

//#define FILEDEBUG 1


bool LCursor::isInside(InsetBase const * p)
{
	for (unsigned i = 0; i < depth(); ++i)
		if (&operator[](i).inset() == p)
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
	if (depth() >= anchor_.size())
		return false;
	if (!ptr_cmp(t.nucleus(), &anchor_[depth()].inset()))
		return false;

	return true;
}


bool positionable(DocIterator const & cursor,
	DocIterator const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.size() > anchor.size())
		return false;

	// anchor might be deeper, should have same path then
	for (size_t i = 0; i < cursor.size(); ++i)
		if (&cursor[i].inset() != &anchor[i].inset())
			return false;

	// position should be ok.
	return true;
}


void LCursor::setScreenPos(int x, int y)
{
	x_target() = x;
	bruteFind(x, y, 0, bv().workWidth(), 0, bv().workHeight());
}



void LCursor::plainErase()
{
	cell().erase(pos());
}


void LCursor::markInsert()
{
	insert(char(0));
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
	//lyxerr << "LCursor::insert str '" << str << "'" << endl;
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		insert(*it);
}


void LCursor::insert(char c)
{
	//lyxerr << "LCursor::insert char '" << c << "'" << endl;
	BOOST_ASSERT(!empty());
	if (inMathed()) {
		selClearOrDel();
		insert(new MathCharInset(c));
	} else {
		text()->insertChar(*this, c);
	}
}


void LCursor::insert(MathAtom const & t)
{
	//lyxerr << "LCursor::insert MathAtom: " << endl;
	macroModeClose();
	selClearOrDel();
	plainInsert(t);
	lyxerr << "LCursor::insert MathAtom: cur:\n" << *this << endl;
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
		pushLeft(*nextInset());
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
		if (inset().nargs() == 1 && depth() == 1 && lastpos() == 0)
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
		resetAnchor();
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
	if (pos() == lastpos() && inset().idxDelete(idx()))
		return true;

	// special behaviour when in last position of cell
	if (pos() == lastpos()) {
		bool one_cell = inset().nargs() == 1;
		if (one_cell && depth() == 1 && lastpos() == 0)
			return false;
		// remove markup
		if (one_cell)
			pullArg();
		else
			inset().idxGlue(idx());
		return true;
	}

	// 'clever' UI hack: only erase large items if previously slected
	if (pos() != lastpos() && inset().nargs() > 0) {
		resetAnchor();
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
	DocIterator save = *this;
	if (goUpDown(true))
		return true;
	setCursor(save, false);
	autocorrect() = false;
	return selection();
}


bool LCursor::down()
{
	macroModeClose();
	DocIterator save = *this;
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
	pushLeft(*nextInset());
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


void LCursor::pullArg()
{
#ifdef WITH_WARNINGS
#warning Look here
#endif
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
#ifdef WITH_WARNINGS
#warning look here
#endif
#if 0
	DocIterator::const_iterator it = begin();
	DocIterator::const_iterator et = end();
	for ( ; it != et; ++it)
		it->cell().touch();
#endif
}


void LCursor::normalize()
{
	if (idx() >= nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << ' ' << nargs()
		       << " in: " << &inset() << endl;
	}
	idx() = min(idx(), lastidx());

	if (pos() > lastpos()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << ' ' << lastpos() <<  " in idx: " << idx()
		       << " in atom: '";
		WriteStream wi(lyxerr, false, true);
		inset().asMathInset()->write(wi);
		lyxerr << endl;
	}
	pos() = min(pos(), lastpos());
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
	if (inset().idxUpDown2(*this, up))
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
		//lyxerr << "updown: We are in " << &inset() << " idx: " << idx() << endl;
		// ask inset first
		if (inset().idxUpDown(*this, up)) {
			// try to find best position within this inset
			if (!selection())
				bruteFind2(xo, yo);
			return true;
		}

		// no such inset found, just take something "above"
		//lyxerr << "updown: handled by strange case" << endl;
		if (!popLeft()) {
			int ylow  = up ? 0 : yo + 1;
			int yhigh = up ? yo - 1 : bv().workHeight();
			return bruteFind(xo, yo, 0, bv().workWidth(), ylow, yhigh);
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
	BOOST_ASSERT(!empty());
	par_type beg, end;
	CursorSlice bottom = operator[](0);
	LyXText * text = bottom.text();
	BOOST_ASSERT(text);
	getParsInRange(text->paragraphs(), ylow, yhigh, beg, end);

	DocIterator it = doc_iterator_begin(bv().buffer()->inset());
	DocIterator et = doc_iterator_end(bv().buffer()->inset());
	//lyxerr << "x: " << x << " y: " << y << endl;
	//lyxerr << "xlow: " << xlow << " ylow: " << ylow << endl;
	//lyxerr << "xhigh: " << xhigh << " yhigh: " << yhigh << endl;

	it.par() = beg;
	//et.par() = text->parOffset(end);

	double best_dist = 10e10;
	DocIterator best_cursor = it;

	for ( ; it != et; it.forwardPos()) {
		// avoid invalid nesting when selecting
		if (!selection() || positionable(it, anchor_)) {
			int xo = 0, yo = 0;
			CursorSlice & cur = it.back();
			cur.inset().getCursorPos(cur, xo, yo);
			if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
				double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
				//lyxerr << "xo: " << xo << " yo: " << yo << " d: " << d << endl;
				// '<=' in order to take the last possible position
				// this is important for clicking behind \sum in e.g. '\sum_i a'
				if (d <= best_dist) {
					//lyxerr << "*" << endl;
					best_dist   = d;
					best_cursor = it;
				}
			}
		}
	}

	//lyxerr << "best_dist: " << best_dist << " cur:\n" << best_cursor << endl;
	if (best_dist < 1e10)
		setCursor(best_cursor, false);
	return best_dist < 1e10;
}


void LCursor::bruteFind2(int x, int y)
{
	double best_dist = 1e10;

	DocIterator it = *this;
	it.back().pos() = 0;
	DocIterator et = *this;
	et.back().pos() = et.back().asMathInset()->cell(et.back().idx()).size();
	for (int i = 0; ; ++i) {
		int xo, yo;
		CursorSlice & cur = it.back();
		cur.inset().getCursorPos(cur, xo, yo);
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
		ParagraphList & pars = text()->paragraphs();

		// should be const ...
		par_type startpit = selBegin().par();
		par_type endpit = selEnd().par();
		size_t const startpos = selBegin().pos();
		size_t const endpos = selEnd().pos();

		if (startpit == endpit)
			return pars[startpit].asString(buffer, startpos, endpos, label);

		// First paragraph in selection
		string result = pars[startpit].
			asString(buffer, startpos, pars[startpit].size(), label) + "\n\n";

		// The paragraphs in between (if any)
		for (par_type pit = startpit + 1; pit != endpit; ++pit) {
			Paragraph & par = pars[pit];
			result += par.asString(buffer, 0, par.size(), label) + "\n\n";
		}

		// Last paragraph in selection
		result += pars[endpit].asString(buffer, 0, endpos, label);

		return result;
	}

#ifdef WITH_WARNINGS
#warning and mathed?
#endif
	return string();
}


string LCursor::currentState()
{
	if (inMathed()) {
		std::ostringstream os;
		info(os);
		return os.str();
	}

	if (inTexted())
	 return text()->currentState(*this);

	return string();
}


string LCursor::getPossibleLabel()
{
	return inMathed() ? "eq:" : text()->getPossibleLabel(*this);
}


Encoding const * LCursor::getEncoding() const
{
	if (empty())
		return 0;
	if (!bv().buffer())
		return 0;
	int s = 0;
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (s = size() - 1; s >= 0; --s)
		if (operator[](s).text())
			break;
	CursorSlice const & sl = operator[](s);
	LyXText & text = *sl.text();
	LyXFont font = text.getPar(sl.par()).getFont(
		bv().buffer()->params(), sl.pos(), outerFont(sl.par(), text.paragraphs()));
	return font.language()->encoding();
}


void LCursor::undispatched()
{
	disp_.dispatched(false);
}


void LCursor::dispatched()
{
	disp_.dispatched(true);
}


void LCursor::noUpdate()
{
	disp_.update(false);
}
