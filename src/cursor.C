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
#include "coordcache.h"
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

#include "insets/insettabular.h"
#include "insets/insettext.h"

#include "mathed/math_data.h"
#include "mathed/math_inset.h"
#include "mathed/math_scriptinset.h"
#include "mathed/math_macrotable.h"

#include "support/limited_stack.h"

#include "frontends/LyXView.h"
#include "frontends/font_metrics.h"

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#include <sstream>
#include <limits>

using lyx::pit_type;

using std::string;
using std::vector;
using std::endl;
#ifndef CXX_GLOBAL_CSTD
using std::isalpha;
#endif
using std::min;
using std::swap;

namespace {

	bool
	positionable(DocIterator const & cursor, DocIterator const & anchor)
	{
		// avoid deeper nested insets when selecting
		if (cursor.depth() > anchor.depth())
			return false;

		// anchor might be deeper, should have same path then
		for (size_t i = 0; i < cursor.depth(); ++i)
			if (&cursor[i].inset() != &anchor[i].inset())
				return false;

		// position should be ok.
		return true;
	}


	// Find position closest to (x, y) in cell given by iter.
	// Used only in mathed
	DocIterator bruteFind2(LCursor const & c, int x, int y)
	{
		double best_dist = std::numeric_limits<double>::max();

		DocIterator result;

		DocIterator it = c;
		it.top().pos() = 0;
		DocIterator et = c;
		et.top().pos() = et.top().asMathInset()->cell(et.top().idx()).size();
		for (size_t i = 0; ; ++i) {
			int xo;
			int yo;
			it.inset().cursorPos(it.top(), c.boundary() && ((i+1) == it.depth()), xo, yo);
			double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
			// '<=' in order to take the last possible position
			// this is important for clicking behind \sum in e.g. '\sum_i a'
			lyxerr[Debug::DEBUG] << "i: " << i << " d: " << d
				<< " best: " << best_dist << endl;
			if (d <= best_dist) {
				best_dist = d;
				result = it;
			}
			if (it == et)
				break;
			it.forwardPos();
		}
		return result;
	}


	/// moves position closest to (x, y) in given box
	bool bruteFind(LCursor & cursor,
		int x, int y, int xlow, int xhigh, int ylow, int yhigh)
	{
		BOOST_ASSERT(!cursor.empty());
		InsetBase & inset = cursor[0].inset();

		DocIterator it = doc_iterator_begin(inset);
		DocIterator const et = doc_iterator_end(inset);

		double best_dist = std::numeric_limits<double>::max();;
		DocIterator best_cursor = et;

		for ( ; it != et; it.forwardPos()) {
			// avoid invalid nesting when selecting
			if (bv_funcs::status(&cursor.bv(), it) == bv_funcs::CUR_INSIDE
			    && (!cursor.selection() || positionable(it, cursor.anchor_))) {
				Point p = bv_funcs::getPos(it, false);
				int xo = p.x_;
				int yo = p.y_;
				if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
					double const dx = xo - x;
					double const dy = yo - y;
					double const d = dx * dx + dy * dy;
					// '<=' in order to take the last possible position
					// this is important for clicking behind \sum in e.g. '\sum_i a'
					if (d <= best_dist) {
						lyxerr << "*" << endl;
						best_dist   = d;
						best_cursor = it;
					}
				}
			}
		}

		if (best_cursor != et) {
			cursor.setCursor(best_cursor);
			return true;
		}

		return false;
	}


	/// moves position closest to (x, y) in given box
	bool bruteFind3(LCursor & cur, int x, int y, bool up)
	{
		BufferView & bv = cur.bv();
		int ylow  = up ? 0 : y + 1;
		int yhigh = up ? y - 1 : bv.workHeight();
		int xlow = 0;
		int xhigh = bv.workWidth();

// FIXME: bit more work needed to get 'from' and 'to' right.
		pit_type from = cur.bottom().pit();
		//pit_type to = cur.bottom().pit();
		//lyxerr << "Pit start: " << from << endl;

		//lyxerr << "bruteFind3: x: " << x << " y: " << y
		//	<< " xlow: " << xlow << " xhigh: " << xhigh 
		//	<< " ylow: " << ylow << " yhigh: " << yhigh 
		//	<< endl;
		InsetBase & inset = bv.buffer()->inset();
		DocIterator it = doc_iterator_begin(inset);
		it.pit() = from;
		DocIterator et = doc_iterator_end(inset);

		double best_dist = std::numeric_limits<double>::max();
		DocIterator best_cursor = et;

		for ( ; it != et; it.forwardPos()) {
			// avoid invalid nesting when selecting
			if (bv_funcs::status(&bv, it) == bv_funcs::CUR_INSIDE
			    && (!cur.selection() || positionable(it, cur.anchor_))) {
				Point p = bv_funcs::getPos(it, false);
				int xo = p.x_;
				int yo = p.y_;
				if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
					double const dx = xo - x;
					double const dy = yo - y;
					double const d = dx * dx + dy * dy;
					//lyxerr << "itx: " << xo << " ity: " << yo << " d: " << d
					//	<< " dx: " << dx << " dy: " << dy
					//	<< " idx: " << it.idx() << " pos: " << it.pos()
					//	<< " it:\n" << it
					//	<< endl;
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
		if (best_cursor == et)
			return false;
		cur.setCursor(best_cursor);
		return true;
	}

} // namespace anon


// be careful: this is called from the bv's constructor, too, so
// bv functions are not yet available!
LCursor::LCursor(BufferView & bv)
	: DocIterator(), bv_(&bv), anchor_(), x_target_(-1),
	  selection_(false), mark_(false), logicalpos_(false)
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


// this (intentionally) does neither touch anchor nor selection status
void LCursor::setCursor(DocIterator const & cur)
{
	DocIterator::operator=(cur);
}


void LCursor::dispatch(FuncRequest const & cmd0)
{
	lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION
			     << " cmd: " << cmd0 << '\n'
			     << *this << endl;
	if (empty())
		return;

	fixIfBroken();
	FuncRequest cmd = cmd0;
	LCursor safe = *this;

	for (; depth(); pop()) {
		lyxerr[Debug::DEBUG] << "LCursor::dispatch: cmd: "
			<< cmd0 << endl << *this << endl;
		BOOST_ASSERT(pos() <= lastpos());
		BOOST_ASSERT(idx() <= lastidx());
		BOOST_ASSERT(pit() <= lastpit());

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
		lyxerr[Debug::DEBUG] << "RESTORING OLD CURSOR!" << endl;
		operator=(safe);
		disp_.dispatched(false);
	}
}


DispatchResult LCursor::result() const
{
	return disp_;
}


BufferView & LCursor::bv() const
{
	BOOST_ASSERT(bv_);
	return *bv_;
}


Buffer & LCursor::buffer() const
{
	BOOST_ASSERT(bv_);
	BOOST_ASSERT(bv_->buffer());
	return *bv_->buffer();
}


void LCursor::pop()
{
	BOOST_ASSERT(depth() >= 1);
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
	for (int i = depth() - 1; i >= 0; --i) {
		int res = operator[](i).inset().currentMode();
		if (res != InsetBase::UNDECIDED_MODE)
			return res;
	}
	return InsetBase::TEXT_MODE;
}


void LCursor::getPos(int & x, int & y) const
{
	Point p = bv_funcs::getPos(*this, boundary());
	x = p.x_;
	y = p.y_;
}


void LCursor::paste(string const & data)
{
	if (!data.empty())
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
	BOOST_ASSERT(anchor_.depth() >= depth());
	CursorSlice normal = anchor_[depth() - 1];
	if (depth() < anchor_.depth() && top() <= normal) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos();
	}
	return normal;
}


CursorSlice LCursor::selBegin() const
{
	if (!selection())
		return top();
	return anchor() < top() ? anchor() : top();
}


CursorSlice LCursor::selEnd() const
{
	if (!selection())
		return top();
	return anchor() > top() ? anchor() : top();
}


DocIterator LCursor::selectionBegin() const
{
	if (!selection())
		return *this;
	return anchor() < top() ? anchor_ : *this;
}


DocIterator LCursor::selectionEnd() const
{
	if (!selection())
		return *this;
	return anchor() > top() ? anchor_ : *this;
}


void LCursor::setSelection()
{
	selection() = true;
	// A selection with no contents is not a selection
#ifdef WITH_WARNINGS
#warning doesnt look ok
#endif
	if (pit() == anchor().pit() && pos() == anchor().pos())
		selection() = false;
}


void LCursor::setSelection(DocIterator const & where, size_t n)
{
	setCursor(where);
	selection() = true;
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


void LCursor::selHandle(bool sel)
{
	//lyxerr << "LCursor::selHandle" << endl;
	if (sel == selection())
		return;

	resetAnchor();
	selection() = sel;
}


std::ostream & operator<<(std::ostream & os, LCursor const & cur)
{
	os << "\n cursor:                                | anchor:\n";
	for (size_t i = 0, n = cur.depth(); i != n; ++i) {
		os << " " << cur[i] << " | ";
		if (i < cur.anchor_.depth())
			os << cur.anchor_[i];
		else
			os << "-------------------------------";
		os << "\n";
	}
	for (size_t i = cur.depth(), n = cur.anchor_.depth(); i < n; ++i) {
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
	for (size_t i = 0; i != depth(); ++i)
		if (&operator[](i).inset() == p)
			return true;
	return false;
}


void LCursor::leaveInset(InsetBase const & inset)
{
	for (size_t i = 0; i != depth(); ++i) {
		if (&operator[](i).inset() == &inset) {
			resize(i);
			return;
		}
	}
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
	if (depth() >= anchor_.depth())
		return false;
	if (!ptr_cmp(t.nucleus(), &anchor_[depth()].inset()))
		return false;

	return true;
}


void LCursor::setScreenPos(int x, int y)
{
	x_target() = x;
	bruteFind(*this, x, y, 0, bv().workWidth(), 0, bv().workHeight());
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
	for_each(str.begin(), str.end(),
		 boost::bind(static_cast<void(LCursor::*)(char)>
			     (&LCursor::insert), this, _1));
}


void LCursor::insert(char c)
{
	//lyxerr << "LCursor::insert char '" << c << "'" << endl;
	BOOST_ASSERT(!empty());
	if (inMathed()) {
		lyx::cap::selClearOrDel(*this);
		insert(new MathCharInset(c));
	} else {
		text()->insertChar(*this, c);
	}
}


void LCursor::insert(MathAtom const & t)
{
	//lyxerr << "LCursor::insert MathAtom '" << t << "'" << endl;
	macroModeClose();
	lyx::cap::selClearOrDel(*this);
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
	string const safe = lyx::cap::grabAndEraseSelection(*this);
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
		lyx::cap::eraseSelection(*this);
	cell().insert(pos(), ar);
	pos() += ar.size();
}


bool LCursor::backspace()
{
	autocorrect() = false;

	if (selection()) {
		lyx::cap::selDel(*this);
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
		lyx::cap::selDel(*this);
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
	if (pos() != lastpos() && nextAtom()->nargs() > 0) {
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
	setCursor(save);
	autocorrect() = false;
	return selection();
}


bool LCursor::down()
{
	macroModeClose();
	DocIterator save = *this;
	if (goUpDown(false))
		return true;
	setCursor(save);
	autocorrect() = false;
	return selection();
}


void LCursor::macroModeClose()
{
	if (!inMacroMode())
		return;
	MathUnknownInset * p = activeMacro();
	p->finalize();
	string const s = p->name();
	--pos();
	cell().erase(pos());

	// do nothing if the macro name is empty
	if (s == "\\")
		return;

	string const name = s.substr(1);

	// prevent entering of recursive macros
	// FIXME: this is only a weak attempt... only prevents immediate
	// recursion
	InsetBase const * macro = innerInsetOfType(InsetBase::MATHMACRO_CODE);
	if (macro && macro->getInsetName() == name)
		lyxerr << "can't enter recursive macro" << endl;

	plainInsert(createMathInset(name));
}


string LCursor::macroName()
{
	return inMacroMode() ? activeMacro()->name() : string();
}


void LCursor::handleNest(MathAtom const & a, int c)
{
	//lyxerr << "LCursor::handleNest: " << c << endl;
	MathAtom t = a;
	asArray(lyx::cap::grabAndEraseSelection(*this), t.nucleus()->cell(c));
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


void LCursor::setTargetX()
{
	// For now this is good enough. A better solution would be to
	// avoid this rebreak by setting cursorX only after drawing
	bottom().text()->redoParagraph(bottom().pit());
	int x;
	int y;
	getPos(x, y);
	x_target_ = x;
}


bool LCursor::inMacroMode() const
{
	if (pos() == 0)
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
	if (idx() > lastidx()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << ' ' << nargs()
		       << " in: " << &inset() << endl;
		idx() = lastidx();
	}

	if (pos() > lastpos()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << ' ' << lastpos() <<  " in idx: " << idx()
		       << " in atom: '";
		WriteStream wi(lyxerr, false, true);
		inset().asMathInset()->write(wi);
		lyxerr << endl;
		pos() = lastpos();
	}
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
				push(*const_cast<MathScriptInset*>(p));
				idx() = p->idxOfScript(up);
				pos() = lastpos();
				return true;
			}
		}

		// try right
		if (pos() != lastpos()) {
			MathScriptInset const * p = nextAtom()->asScriptInset();
			if (p && p->has(up)) {
				push(*const_cast<MathScriptInset*>(p));
				idx() = p->idxOfScript(up);
				pos() = 0;
				return true;
			}
		}
	}

// FIXME: Switch this on for more robust movement
#if 0

	return bruteFind3(*this, xo, yo, up); 

#else
	//xarray().boundingBox(xlow, xhigh, ylow, yhigh);
	//if (up)
	//	yhigh = yo - 4;
	//else
	//	ylow = yo + 4;
	//if (bruteFind(*this, xo, yo, xlow, xhigh, ylow, yhigh)) {
	//	lyxerr << "updown: handled by brute find in the same cell" << endl;
	//	return true;
	//}

	// try to find an inset that knows better then we
	while (true) {
		lyxerr << "updown: We are in " << &inset() << " idx: " << idx() << endl;
		// ask inset first
		if (inset().idxUpDown(*this, up)) {
			lyxerr << "idxUpDown triggered" << endl;
			// try to find best position within this inset
			if (!selection())
				setCursor(bruteFind2(*this, xo, yo));
			return true;
		}

		// no such inset found, just take something "above"
		if (!popLeft()) {
			lyxerr << "updown: popleft failed (strange case)" << endl;
			int ylow  = up ? 0 : yo + 1;
			int yhigh = up ? yo - 1 : bv().workHeight();
			return bruteFind(*this, xo, yo, 0, bv().workWidth(), ylow, yhigh);
		}

		// any improvement so far?
		lyxerr << "updown: popLeft succeeded" << endl;
		int xnew;
		int ynew;
		getPos(xnew, ynew);
		if (up ? ynew < yo : ynew > yo)
			return true;
	}

	// we should not come here.
	BOOST_ASSERT(false);
#endif
}


void LCursor::handleFont(string const & font)
{
	lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION << ": " << font << endl;
	string safe;
	if (selection()) {
		macroModeClose();
		safe = lyx::cap::grabAndEraseSelection(*this);
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
		ParagraphList const & pars = text()->paragraphs();

		// should be const ...
		pit_type startpit = selBegin().pit();
		pit_type endpit = selEnd().pit();
		size_t const startpos = selBegin().pos();
		size_t const endpos = selEnd().pos();

		if (startpit == endpit)
			return pars[startpit].asString(buffer, startpos, endpos, label);

		// First paragraph in selection
		string result = pars[startpit].
			asString(buffer, startpos, pars[startpit].size(), label) + "\n\n";

		// The paragraphs in between (if any)
		for (pit_type pit = startpit + 1; pit != endpit; ++pit) {
			Paragraph const & par = pars[pit];
			result += par.asString(buffer, 0, par.size(), label) + "\n\n";
		}

		// Last paragraph in selection
		result += pars[endpit].asString(buffer, 0, endpos, label);

		return result;
	}

	if (inMathed())
		return lyx::cap::grabSelection(*this);

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
	for (s = depth() - 1; s >= 0; --s)
		if (operator[](s).text())
			break;
	CursorSlice const & sl = operator[](s);
	LyXText const & text = *sl.text();
	LyXFont font = text.getPar(sl.pit()).getFont(
		bv().buffer()->params(), sl.pos(), outerFont(sl.pit(), text.paragraphs()));
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


void LCursor::needsUpdate()
{
	disp_.update(true);
}


void LCursor::noUpdate()
{
	disp_.update(false);
}


LyXFont LCursor::getFont() const
{
	// HACK. far from being perfect...
	int s = 0;
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (s = depth() - 1; s >= 0; --s)
		if (operator[](s).text())
			break;
	CursorSlice const & sl = operator[](s);
	LyXText const & text = *sl.text();
	LyXFont font = text.getPar(sl.pit()).getFont(
		bv().buffer()->params(),
		sl.pos(),
		outerFont(sl.pit(), text.paragraphs()));

	return font;
}


void LCursor::fixIfBroken()
{
	// find out last good level
	LCursor copy = *this;
	size_t newdepth = depth();
	while (!copy.empty()) {
		if (copy.idx() > copy.lastidx()) {
			lyxerr << "wrong idx " << copy.idx()
			       << ", max is " << copy.lastidx()
			       << " at level " << copy.depth()
			       << ". Trying to correct this."  << endl;
			newdepth = copy.depth() - 1;
		}
		else if (copy.pit() > copy.lastpit()) {
			lyxerr << "wrong pit " << copy.pit()
			       << ", max is " << copy.lastpit()
			       << " at level " << copy.depth()
			       << ". Trying to correct this."  << endl;
			newdepth = copy.depth() - 1;
		}
		else if (copy.pos() > copy.lastpos()) {
			lyxerr << "wrong pos " << copy.pos()
			       << ", max is " << copy.lastpos()
			       << " at level " << copy.depth()
			       << ". Trying to correct this."  << endl;
			newdepth = copy.depth() - 1;
		}
		copy.pop();
	}
	// shrink cursor to a size where everything is valid, possibly
	// leaving insets
	while (depth() > newdepth) {
		pop();
		lyxerr << "correcting cursor to level " << depth() << endl;
	}
}
