/**
 * \file Cursor.cpp
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
#include "bufferview_funcs.h"
#include "Buffer.h"
#include "Cursor.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "Language.h"
#include "lfuns.h"
#include "Font.h"
#include "LyXFunc.h" // only for setMessage()
#include "LyXRC.h"
#include "Row.h"
#include "LyXText.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParIterator.h"

#include "insets/InsetTabular.h"
#include "insets/InsetText.h"

#include "mathed/MathData.h"
#include "mathed/InsetMath.h"
#include "mathed/InsetMathScript.h"
#include "mathed/MacroTable.h"

#include "support/limited_stack.h"

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#include <sstream>
#include <limits>
#include <map>

namespace lyx {

using std::string;
using std::vector;
using std::endl;
using std::min;
using std::for_each;

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
	DocIterator bruteFind2(Cursor const & c, int x, int y)
	{
		double best_dist = std::numeric_limits<double>::max();

		DocIterator result;

		DocIterator it = c;
		it.top().pos() = 0;
		DocIterator et = c;
		et.top().pos() = et.top().asInsetMath()->cell(et.top().idx()).size();
		for (size_t i = 0;; ++i) {
			int xo;
			int yo;
			Inset const * inset = &it.inset();
			std::map<Inset const *, Point> const & data =
				c.bv().coordCache().getInsets().getData();
			std::map<Inset const *, Point>::const_iterator I = data.find(inset);

			// FIXME: in the case where the inset is not in the cache, this
			// means that no part of it is visible on screen. In this case
			// we don't do elaborate search and we just return the forwarded
			// DocIterator at its beginning.
			if (I == data.end()) {
				it.top().pos() = 0;
				return it;
			}

			Point o = I->second;
			inset->cursorPos(c.bv(), it.top(), c.boundary(), xo, yo);
			// Convert to absolute
			xo += o.x_;
			yo += o.y_;
			double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
			// '<=' in order to take the last possible position
			// this is important for clicking behind \sum in e.g. '\sum_i a'
			LYXERR(Debug::DEBUG) << "i: " << i << " d: " << d
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
	bool bruteFind(Cursor & cursor,
		int x, int y, int xlow, int xhigh, int ylow, int yhigh)
	{
		BOOST_ASSERT(!cursor.empty());
		Inset & inset = cursor[0].inset();
		BufferView & bv = cursor.bv();

		CoordCache::InnerParPosCache const & cache =
			bv.coordCache().getParPos().find(cursor.bottom().text())->second;
		// Get an iterator on the first paragraph in the cache
		DocIterator it(inset);
		it.push_back(CursorSlice(inset));
		it.pit() = cache.begin()->first;
		// Get an iterator after the last paragraph in the cache
		DocIterator et(inset);
		et.push_back(CursorSlice(inset));
		et.pit() = boost::prior(cache.end())->first;
		if (et.pit() >= et.lastpit())
			et = doc_iterator_end(inset);
		else
			++et.pit();

		double best_dist = std::numeric_limits<double>::max();;
		DocIterator best_cursor = et;

		for ( ; it != et; it.forwardPos(true)) {
			// avoid invalid nesting when selecting
			if (!cursor.selection() || positionable(it, cursor.anchor_)) {
				Point p = bv_funcs::getPos(bv, it, false);
				int xo = p.x_;
				int yo = p.y_;
				if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
					double const dx = xo - x;
					double const dy = yo - y;
					double const d = dx * dx + dy * dy;
					// '<=' in order to take the last possible position
					// this is important for clicking behind \sum in e.g. '\sum_i a'
					if (d <= best_dist) {
						//	lyxerr << "*" << endl;
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
	bool bruteFind3(Cursor & cur, int x, int y, bool up)
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
		Inset & inset = bv.buffer()->inset();
		DocIterator it = doc_iterator_begin(inset);
		it.pit() = from;
		DocIterator et = doc_iterator_end(inset);

		double best_dist = std::numeric_limits<double>::max();
		DocIterator best_cursor = et;

		for ( ; it != et; it.forwardPos()) {
			// avoid invalid nesting when selecting
			if (bv_funcs::status(&bv, it) == bv_funcs::CUR_INSIDE
			    && (!cur.selection() || positionable(it, cur.anchor_))) {
				Point p = bv_funcs::getPos(bv, it, false);
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
Cursor::Cursor(BufferView & bv)
	: DocIterator(), bv_(&bv), anchor_(), x_target_(-1),
	  selection_(false), mark_(false), logicalpos_(false)
{}


void Cursor::reset(Inset & inset)
{
	clear();
	push_back(CursorSlice(inset));
	anchor_ = DocIterator(inset);
	clearTargetX();
	selection_ = false;
	mark_ = false;
}


// this (intentionally) does neither touch anchor nor selection status
void Cursor::setCursor(DocIterator const & cur)
{
	DocIterator::operator=(cur);
}


void Cursor::dispatch(FuncRequest const & cmd0)
{
	LYXERR(Debug::DEBUG) << BOOST_CURRENT_FUNCTION
			     << " cmd: " << cmd0 << '\n'
			     << *this << endl;
	if (empty())
		return;

	fixIfBroken();
	FuncRequest cmd = cmd0;
	Cursor safe = *this;

	for (; depth(); pop()) {
		LYXERR(Debug::DEBUG) << "Cursor::dispatch: cmd: "
			<< cmd0 << endl << *this << endl;
		BOOST_ASSERT(pos() <= lastpos());
		BOOST_ASSERT(idx() <= lastidx());
		BOOST_ASSERT(pit() <= lastpit());

		// The common case is 'LFUN handled, need update', so make the
		// LFUN handler's life easier by assuming this as default value.
		// The handler can reset the update and val flags if necessary.
		disp_.update(Update::FitCursor | Update::Force);
		disp_.dispatched(true);
		inset().dispatch(*this, cmd);
		if (disp_.dispatched())
			break;
	}
	// it completely to get a 'bomb early' behaviour in case this
	// object will be used again.
	if (!disp_.dispatched()) {
		LYXERR(Debug::DEBUG) << "RESTORING OLD CURSOR!" << endl;
		operator=(safe);
		disp_.update(Update::None);
		disp_.dispatched(false);
	}
}


DispatchResult Cursor::result() const
{
	return disp_;
}


BufferView & Cursor::bv() const
{
	BOOST_ASSERT(bv_);
	return *bv_;
}


Buffer & Cursor::buffer() const
{
	BOOST_ASSERT(bv_);
	BOOST_ASSERT(bv_->buffer());
	return *bv_->buffer();
}


void Cursor::pop()
{
	BOOST_ASSERT(depth() >= 1);
	pop_back();
}


void Cursor::push(Inset & p)
{
	push_back(CursorSlice(p));
}


void Cursor::pushLeft(Inset & p)
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Entering inset " << t << " left" << endl;
	push(p);
	p.idxFirst(*this);
}


bool Cursor::popLeft()
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Leaving inset to the left" << endl;
	inset().notifyCursorLeaves(*this);
	if (depth() == 1)
		return false;
	pop();
	return true;
}


bool Cursor::popRight()
{
	BOOST_ASSERT(!empty());
	//lyxerr << "Leaving inset to the right" << endl;
	const pos_type lp = (depth() > 1) ? (*this)[depth() - 2].lastpos() : 0;
	inset().notifyCursorLeaves(*this);
	if (depth() == 1)
		return false;
	pop();
	pos() += lastpos() - lp + 1;
	return true;
}


int Cursor::currentMode()
{
	BOOST_ASSERT(!empty());
	for (int i = depth() - 1; i >= 0; --i) {
		int res = operator[](i).inset().currentMode();
		if (res != Inset::UNDECIDED_MODE)
			return res;
	}
	return Inset::TEXT_MODE;
}


void Cursor::getPos(int & x, int & y) const
{
	Point p = bv_funcs::getPos(bv(), *this, boundary());
	x = p.x_;
	y = p.y_;
}


Row const & Cursor::textRow() const
{
	ParagraphMetrics const & pm = bv().parMetrics(text(), pit());
	BOOST_ASSERT(!pm.rows().empty());
	return pm.getRow(pos(), boundary());
}


void Cursor::resetAnchor()
{
	anchor_ = *this;
}



bool Cursor::posLeft()
{
	if (pos() == 0)
		return false;
	--pos();
	return true;
}


bool Cursor::posRight()
{
	if (pos() == lastpos())
		return false;
	++pos();
	return true;
}


CursorSlice Cursor::anchor() const
{
	BOOST_ASSERT(anchor_.depth() >= depth());
	CursorSlice normal = anchor_[depth() - 1];
	if (depth() < anchor_.depth() && top() <= normal) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos();
	}
	return normal;
}


CursorSlice Cursor::selBegin() const
{
	if (!selection())
		return top();
	return anchor() < top() ? anchor() : top();
}


CursorSlice Cursor::selEnd() const
{
	if (!selection())
		return top();
	return anchor() > top() ? anchor() : top();
}


DocIterator Cursor::selectionBegin() const
{
	if (!selection())
		return *this;
	DocIterator di = (anchor() < top() ? anchor_ : *this);
	di.resize(depth());
	return di;
}


DocIterator Cursor::selectionEnd() const
{
	if (!selection())
		return *this;
	DocIterator di = (anchor() > top() ? anchor_ : *this);
	if (di.depth() > depth()) {
		di.resize(depth());
		++di.pos();
	}
	return di;
}


void Cursor::setSelection()
{
	selection() = true;
	// A selection with no contents is not a selection
#ifdef WITH_WARNINGS
#warning doesnt look ok
#endif
	if (pit() == anchor().pit() && pos() == anchor().pos())
		selection() = false;
}


void Cursor::setSelection(DocIterator const & where, int n)
{
	setCursor(where);
	selection() = true;
	anchor_ = where;
	pos() += n;
}


void Cursor::clearSelection()
{
	selection() = false;
	mark() = false;
	resetAnchor();
}


int & Cursor::x_target()
{
	return x_target_;
}


int Cursor::x_target() const
{
	return x_target_;
}


void Cursor::clearTargetX()
{
	x_target_ = -1;
}



void Cursor::info(odocstream & os) const
{
	for (int i = 1, n = depth(); i < n; ++i) {
		operator[](i).inset().infoize(os);
		os << "  ";
	}
	if (pos() != 0) {
		Inset const * inset = prevInset();
		// prevInset() can return 0 in certain case.
		if (inset)
			prevInset()->infoize2(os);
	}
	// overwite old message
	os << "                    ";
}


bool Cursor::selHandle(bool sel)
{
	//lyxerr << "Cursor::selHandle" << endl;
	if (sel == selection())
		return false;

	resetAnchor();
	selection() = sel;
	cap::saveSelection(*this);
	return true;
}


std::ostream & operator<<(std::ostream & os, Cursor const & cur)
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

} // namespace lyx


///////////////////////////////////////////////////////////////////
//
// The part below is the non-integrated rest of the original math
// cursor. This should be either generalized for texted or moved
// back to mathed (in most cases to InsetMathNest).
//
///////////////////////////////////////////////////////////////////

#include "mathed/InsetMathChar.h"
#include "mathed/InsetMathGrid.h"
#include "mathed/InsetMathScript.h"
#include "mathed/InsetMathUnknown.h"
#include "mathed/MathFactory.h"
#include "mathed/MathStream.h"
#include "mathed/MathSupport.h"


namespace lyx {

//#define FILEDEBUG 1


bool Cursor::isInside(Inset const * p)
{
	for (size_t i = 0; i != depth(); ++i)
		if (&operator[](i).inset() == p)
			return true;
	return false;
}


void Cursor::leaveInset(Inset const & inset)
{
	for (size_t i = 0; i != depth(); ++i) {
		if (&operator[](i).inset() == &inset) {
			resize(i);
			return;
		}
	}
}


bool Cursor::openable(MathAtom const & t) const
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


void Cursor::setScreenPos(int x, int y)
{
	x_target() = x;
	bruteFind(*this, x, y, 0, bv().workWidth(), 0, bv().workHeight());
}



void Cursor::plainErase()
{
	cell().erase(pos());
}


void Cursor::markInsert()
{
	insert(char_type(0));
}


void Cursor::markErase()
{
	cell().erase(pos());
}


void Cursor::plainInsert(MathAtom const & t)
{
	cell().insert(pos(), t);
	++pos();
}


void Cursor::insert(docstring const & str)
{
	for_each(str.begin(), str.end(),
		 boost::bind(static_cast<void(Cursor::*)(char_type)>
			     (&Cursor::insert), this, _1));
}


void Cursor::insert(char_type c)
{
	//lyxerr << "Cursor::insert char '" << c << "'" << endl;
	BOOST_ASSERT(!empty());
	if (inMathed()) {
		cap::selClearOrDel(*this);
		insert(new InsetMathChar(c));
	} else {
		text()->insertChar(*this, c);
	}
}


void Cursor::insert(MathAtom const & t)
{
	//lyxerr << "Cursor::insert MathAtom '" << t << "'" << endl;
	macroModeClose();
	cap::selClearOrDel(*this);
	plainInsert(t);
}


void Cursor::insert(Inset * inset)
{
	if (inMathed())
		insert(MathAtom(inset));
	else
		text()->insertInset(*this, inset);
}


void Cursor::niceInsert(docstring const & t)
{
	MathData ar;
	asArray(t, ar);
	if (ar.size() == 1)
		niceInsert(ar[0]);
	else
		insert(ar);
}


void Cursor::niceInsert(MathAtom const & t)
{
	macroModeClose();
	docstring const safe = cap::grabAndEraseSelection(*this);
	plainInsert(t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posLeft();
		// be careful here: don't use 'pushLeft(t)' as this we need to
		// push the clone, not the original
		pushLeft(*nextInset());
		// We may not use niceInsert here (recursion)
		MathData ar;
		asArray(safe, ar);
		insert(ar);
	}
}


void Cursor::insert(MathData const & ar)
{
	macroModeClose();
	if (selection())
		cap::eraseSelection(*this);
	cell().insert(pos(), ar);
	pos() += ar.size();
}


bool Cursor::backspace()
{
	autocorrect() = false;

	if (selection()) {
		cap::eraseSelection(*this);
		return true;
	}

	if (pos() == 0) {
		// If empty cell, and not part of a big cell
		if (lastpos() == 0 && inset().nargs() == 1) {
			popLeft();
			// Directly delete empty cell: [|[]] => [|] 
			if (inMathed()) {
				plainErase();
				resetAnchor();
				return true;
			}
			// [|], can not delete from inside
			return false;
		} else {
			if (inMathed())
				pullArg();
			else
				popLeft();
			return true;
		}
	}

	if (inMacroMode()) {
		InsetMathUnknown * p = activeMacro();
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


bool Cursor::erase()
{
	autocorrect() = false;
	if (inMacroMode())
		return true;

	if (selection()) {
		cap::eraseSelection(*this);
		return true;
	}

	// delete empty cells if possible
	if (pos() == lastpos() && inset().idxDelete(idx()))
		return true;

	// special behaviour when in last position of cell
	if (pos() == lastpos()) {
		bool one_cell = inset().nargs() == 1;
		if (one_cell && lastpos() == 0) {
			popLeft();
			// Directly delete empty cell: [|[]] => [|] 
			if (inMathed()) {
				plainErase();
				resetAnchor();
				return true;
			}
			// [|], can not delete from inside
			return false;
		}
		// remove markup
		if (!one_cell)
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


bool Cursor::up()
{
	macroModeClose();
	DocIterator save = *this;
	if (goUpDown(true))
		return true;
	setCursor(save);
	autocorrect() = false;
	return selection();
}


bool Cursor::down()
{
	macroModeClose();
	DocIterator save = *this;
	if (goUpDown(false))
		return true;
	setCursor(save);
	autocorrect() = false;
	return selection();
}


bool Cursor::macroModeClose()
{
	if (!inMacroMode())
		return false;
	InsetMathUnknown * p = activeMacro();
	p->finalize();
	docstring const s = p->name();
	--pos();
	cell().erase(pos());

	// do nothing if the macro name is empty
	if (s == "\\")
		return false;

	// trigger updates of macros, at least, if no full
	// updates take place anyway
	updateFlags(Update::Force);

	docstring const name = s.substr(1);
	InsetMathNest * const in = inset().asInsetMath()->asNestInset();
	if (in && in->interpretString(*this, s))
		return true;
	plainInsert(createInsetMath(name));
	return true;
}


docstring Cursor::macroName()
{
	return inMacroMode() ? activeMacro()->name() : docstring();
}


void Cursor::handleNest(MathAtom const & a, int c)
{
	//lyxerr << "Cursor::handleNest: " << c << endl;
	MathAtom t = a;
	asArray(cap::grabAndEraseSelection(*this), t.nucleus()->cell(c));
	insert(t);
	posLeft();
	pushLeft(*nextInset());
}


int Cursor::targetX() const
{
	if (x_target() != -1)
		return x_target();
	int x = 0;
	int y = 0;
	getPos(x, y);
	return x;
}


void Cursor::setTargetX()
{
	int x;
	int y;
	getPos(x, y);
	x_target_ = x;
}


bool Cursor::inMacroMode() const
{
	if (pos() == 0)
		return false;
	InsetMathUnknown const * p = prevAtom()->asUnknownInset();
	return p && !p->final();
}


InsetMathUnknown * Cursor::activeMacro()
{
	return inMacroMode() ? prevAtom().nucleus()->asUnknownInset() : 0;
}


void Cursor::pullArg()
{
#ifdef WITH_WARNINGS
#warning Look here
#endif
	MathData ar = cell();
	if (popLeft() && inMathed()) {
		plainErase();
		cell().insert(pos(), ar);
		resetAnchor();
	} else {
		//formula()->mutateToText();
	}
}


void Cursor::touch()
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


void Cursor::normalize()
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
		odocstringstream os;
		WriteStream wi(os, false, true);
		inset().asInsetMath()->write(wi);
		lyxerr << to_utf8(os.str()) << endl;
		pos() = lastpos();
	}
}


bool Cursor::goUpDown(bool up)
{
	// Be warned: The 'logic' implemented in this function is highly
	// fragile. A distance of one pixel or a '<' vs '<=' _really
	// matters. So fiddle around with it only if you think you know
	// what you are doing!

	int xo = 0;
	int yo = 0;
	getPos(xo, yo);

	// check if we had something else in mind, if not, this is the future
	// target
	if (x_target() == -1)
		x_target() = xo;
	else
		xo = x_target();

	// try neigbouring script insets
	if (!selection()) {
		// try left
		if (pos() != 0) {
			InsetMathScript const * p = prevAtom()->asScriptInset();
			if (p && p->has(up)) {
				--pos();
				push(*const_cast<InsetMathScript*>(p));
				idx() = p->idxOfScript(up);
				pos() = lastpos();
				return true;
			}
		}

		// try right
		if (pos() != lastpos()) {
			InsetMathScript const * p = nextAtom()->asScriptInset();
			if (p && p->has(up)) {
				push(*const_cast<InsetMathScript*>(p));
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
		//lyxerr << "updown: We are in " << &inset() << " idx: " << idx() << endl;
		// ask inset first
		if (inset().idxUpDown(*this, up)) {
			//lyxerr << "idxUpDown triggered" << endl;
			// try to find best position within this inset
			if (!selection())
				setCursor(bruteFind2(*this, xo, yo));
			return true;
		}

		// no such inset found, just take something "above"
		if (!popLeft()) {
			//lyxerr << "updown: popleft failed (strange case)" << endl;
			int ylow  = up ? 0 : yo + 1;
			int yhigh = up ? yo - 1 : bv().workHeight();
			return bruteFind(*this, xo, yo, 0, bv().workWidth(), ylow, yhigh);
		}

		// any improvement so far?
		//lyxerr << "updown: popLeft succeeded" << endl;
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


void Cursor::handleFont(string const & font)
{
	LYXERR(Debug::DEBUG) << BOOST_CURRENT_FUNCTION << ": " << font << endl;
	docstring safe;
	if (selection()) {
		macroModeClose();
		safe = cap::grabAndEraseSelection(*this);
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
			MathData::iterator bt = cell().begin();
			MathAtom at = createInsetMath(from_utf8(font));
			at.nucleus()->cell(0) = MathData(bt, bt + pos());
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


void Cursor::message(docstring const & msg) const
{
	theLyXFunc().setMessage(msg);
}


void Cursor::errorMessage(docstring const & msg) const
{
	theLyXFunc().setErrorMessage(msg);
}


docstring Cursor::selectionAsString(bool label) const
{
	if (!selection())
		return docstring();

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
		docstring result = pars[startpit].
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
		return cap::grabSelection(*this);

	return docstring();
}


docstring Cursor::currentState()
{
	if (inMathed()) {
		odocstringstream os;
		info(os);
		return os.str();
	}

	if (inTexted())
		return text()->currentState(*this);

	return docstring();
}


docstring Cursor::getPossibleLabel()
{
	return inMathed() ? from_ascii("eq:") : text()->getPossibleLabel(*this);
}


Encoding const * Cursor::getEncoding() const
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
	Font font = text.getPar(sl.pit()).getFont(
		bv().buffer()->params(), sl.pos(), outerFont(sl.pit(), text.paragraphs()));
	return font.language()->encoding();
}


void Cursor::undispatched()
{
	disp_.dispatched(false);
}


void Cursor::dispatched()
{
	disp_.dispatched(true);
}


void Cursor::updateFlags(Update::flags f)
{
	disp_.update(f);
}


void Cursor::noUpdate()
{
	disp_.update(Update::None);
}


Font Cursor::getFont() const
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
	Font font = text.getPar(sl.pit()).getFont(
		bv().buffer()->params(),
		sl.pos(),
		outerFont(sl.pit(), text.paragraphs()));

	return font;
}


void Cursor::fixIfBroken()
{
	// find out last good level
	Cursor copy = *this;
	size_t newdepth = depth();
	while (!copy.empty()) {
		if (copy.idx() > copy.lastidx()) {
			lyxerr << "wrong idx " << copy.idx()
			       << ", max is " << copy.lastidx()
			       << " at level " << copy.depth()
			       << ". Trying to correct this."  << endl;
			lyxerr << "old: " << *this << endl;
			newdepth = copy.depth() - 1;
		}
		else if (copy.pit() > copy.lastpit()) {
			lyxerr << "wrong pit " << copy.pit()
			       << ", max is " << copy.lastpit()
			       << " at level " << copy.depth()
			       << ". Trying to correct this."  << endl;
			lyxerr << "old: " << *this << endl;
			newdepth = copy.depth() - 1;
		}
		else if (copy.pos() > copy.lastpos()) {
			lyxerr << "wrong pos " << copy.pos()
			       << ", max is " << copy.lastpos()
			       << " at level " << copy.depth()
			       << ". Trying to correct this."  << endl;
			lyxerr << "old: " << *this << endl;
			newdepth = copy.depth() - 1;
		}
		copy.pop();
	}
	// shrink cursor to a size where everything is valid, possibly
	// leaving insets
	while (depth() > newdepth) {
		pop();
		lyxerr << "correcting cursor to level " << depth() << endl;
		lyxerr << "new: " << *this << endl;
		clearSelection();
	}
}


bool Cursor::isRTL() const
{
	return top().paragraph().isRightToLeftPar(bv().buffer()->params());
}


} // namespace lyx
