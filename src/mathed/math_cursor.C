/**
 * \file math_cursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_cursor.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "formulabase.h"
#include "funcrequest.h"
#include "lyxrc.h"
#include "math_braceinset.h"
#include "math_commentinset.h"
#include "math_charinset.h"
#include "math_factory.h"
#include "math_gridinset.h"
#include "math_macroarg.h"
#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "math_scriptinset.h"
#include "math_spaceinset.h"
#include "math_support.h"
#include "math_unknowninset.h"

#include "support/limited_stack.h"
#include "support/std_sstream.h"

#include <boost/assert.hpp>

//#define FILEDEBUG 1

using std::string;
using std::endl;
#ifndef CXX_GLOBAL_CSTD
using std::isalpha;
#endif
using std::min;
using std::swap;
using std::ostringstream;


// matheds own cut buffer
limited_stack<string> theCutBuffer;


MathCursor::MathCursor(BufferView * bv, InsetFormulaBase * formula, bool front)
	:	formula_(formula), autocorrect_(false), selection_(false)
{
	front ? first(bv->fullCursor()) : last(bv->fullCursor());
}


MathCursor::~MathCursor()
{
	// ensure that 'notifyCursorLeave' is called
	//while (popLeft())
	//	;
}


void MathCursor::push(LCursor & cur, MathAtom & t)
{
	cur.push(t.nucleus());
}


void MathCursor::pushLeft(LCursor & cur, MathAtom & t)
{
	//lyxerr << "Entering atom " << t << " left" << endl;
	push(cur, t);
	t->idxFirst(cur);
}


void MathCursor::pushRight(LCursor & cur, MathAtom & t)
{
	//lyxerr << "Entering atom " << t << " right" << endl;
	posLeft(cur);
	push(cur, t);
	t->idxLast(cur);
}


bool MathCursor::popLeft(LCursor & cur)
{
	//lyxerr << "Leaving atom to the left" << endl;
	if (cur.depth() <= 1) {
		if (cur.depth() == 1)
			cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
		return false;
	}
	cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
	cur.pop();
	return true;
}


bool MathCursor::popRight(LCursor & cur)
{
	//lyxerr << "Leaving atom "; bv.inset->asMathInset()->write(cerr, false); cerr << " right" << endl;
	if (cur.depth() <= 1) {
		if (cur.depth() == 1)
			cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
		return false;
	}
	cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
	cur.pop();
	posRight(cur);
	return true;
}



#if FILEDEBUG
	void MathCursor::dump(char const * what) const
	{
		lyxerr << "MC: " << what << endl;
		lyxerr << " Cursor: " << cur.depth() << endl;
		for (unsigned i = 0; i < cur.depth(); ++i)
			lyxerr << "    i: " << i << ' ' << Cursor_[i] << endl;
		lyxerr << " Anchor: " << Anchor_.size() << endl;
		for (unsigned i = 0; i < Anchor_.size(); ++i)
			lyxerr << "    i: " << i << ' ' << Anchor_[i] << endl;
		lyxerr	<< " sel: " << selection_ << endl;
	}
#else
	void MathCursor::dump(char const *) const {}
#endif


bool MathCursor::isInside(MathInset const *) const
{
#warning FIXME
/*
	for (unsigned i = 0; i < cur.depth(); ++i)
		if (Cursor_[i].asMathInset() == p)
			return true;
*/
	return false;
}


bool MathCursor::openable(MathAtom const & t, bool sel) const
{
	if (!t->isActive())
		return false;

	if (t->lock())
		return false;

#warning FIXME
#if 0
	if (sel) {
		// we can't move into anything new during selection
		if (cur.depth() == Anchor_.size())
			return false;
		if (t.operator->() != Anchor_[cur.depth()].asMathInset())
			return false;
	}
#else
	if (sel)
		return false;
#endif

	return true;
}


bool MathCursor::inNucleus(LCursor & cur) const
{
	return cur.inset()->asMathInset()->asScriptInset() && cur.idx() == 2;
}


bool MathCursor::posLeft(LCursor & cur)
{
	if (cur.pos() == 0)
		return false;
	--cur.pos();
	return true;
}


bool MathCursor::posRight(LCursor & cur)
{
	if (cur.pos() == cur.lastpos())
		return false;
	++cur.pos();
	return true;
}


bool MathCursor::left(LCursor & cur, bool sel)
{
	dump("Left 1");
	autocorrect_ = false;
	cur.bv().x_target(-1); // "no target"
	if (inMacroMode(cur)) {
		macroModeClose(cur);
		return true;
	}
	selHandle(cur, sel);

	if (hasPrevAtom(cur) && openable(prevAtom(cur), sel)) {
		pushRight(cur, prevAtom(cur));
		return true;
	}

	return posLeft(cur) || idxLeft(cur) || popLeft(cur) || selection_;
}


bool MathCursor::right(LCursor & cur, bool sel)
{
	dump("Right 1");
	autocorrect_ = false;
	cur.bv().x_target(-1); // "no target"
	if (inMacroMode(cur)) {
		macroModeClose(cur);
		return true;
	}
	selHandle(cur, sel);

	if (hasNextAtom(cur) && openable(nextAtom(cur), sel)) {
		pushLeft(cur, nextAtom(cur));
		return true;
	}

	return posRight(cur) || idxRight(cur) || popRight(cur) || selection_;
}


void MathCursor::first(LCursor & cur)
{
#warning FIXME
	//Cursor_.clear();
	push(cur, formula_->par());
	cur.inset()->asMathInset()->idxFirst(cur);
	cur.resetAnchor();
}


void MathCursor::last(LCursor & cur)
{
#warning FIXME
	//Cursor_.clear();
	push(cur, formula_->par());
	cur.inset()->asMathInset()->idxLast(cur);
	cur.resetAnchor();
}


bool positionable(CursorBase const & cursor, CursorBase const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.size() > anchor.size())
		return false;

	// anchor might be deeper, should have same path then
	for (CursorBase::size_type i = 0; i < cursor.size(); ++i)
		if (cursor[i].asMathInset() != anchor[i].asMathInset())
			return false;

	// position should be ok.
	return true;
}


void MathCursor::setScreenPos(LCursor & cur, int x, int y)
{
	dump("setScreenPos 1");
	bool res = bruteFind(cur, x, y,
		formula()->xlow(), formula()->xhigh(),
		formula()->ylow(), formula()->yhigh());
	if (!res) {
		// this can happen on creation of "math-display"
		dump("setScreenPos 1.5");
		first(cur);
	}
	cur.bv().x_target(-1); // "no target"
	dump("setScreenPos 2");
}



bool MathCursor::home(LCursor & cur, bool sel)
{
	dump("home 1");
	autocorrect_ = false;
	selHandle(cur, sel);
	macroModeClose(cur);
	if (!cur.inset()->asMathInset()->idxHome(cur))
		return popLeft(cur);
	dump("home 2");
	cur.bv().x_target(-1); // "no target"
	return true;
}


bool MathCursor::end(LCursor & cur, bool sel)
{
	dump("end 1");
	autocorrect_ = false;
	selHandle(cur, sel);
	macroModeClose(cur);
	if (!cur.inset()->asMathInset()->idxEnd(cur))
		return popRight(cur);
	dump("end 2");
	cur.bv().x_target(-1); // "no target"
	return true;
}


void MathCursor::plainErase(LCursor & cur)
{
	cur.cell().erase(cur.pos());
}


void MathCursor::markInsert(LCursor & cur)
{
	//lyxerr << "inserting mark" << endl;
	cur.cell().insert(cur.pos(), MathAtom(new MathCharInset(0)));
}


void MathCursor::markErase(LCursor & cur)
{
	//lyxerr << "deleting mark" << endl;
	cur.cell().erase(cur.pos());
}


void MathCursor::plainInsert(LCursor & cur, MathAtom const & t)
{
	dump("plainInsert");
	cur.cell().insert(cur.pos(), t);
	++cur.pos();
}


void MathCursor::insert2(LCursor & cur, string const & str)
{
	MathArray ar;
	asArray(str, ar);
	insert(cur, ar);
}


void MathCursor::insert(LCursor & cur, string const & str)
{
	//lyxerr << "inserting '" << str << "'" << endl;
	selClearOrDel(cur);
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		plainInsert(cur, MathAtom(new MathCharInset(*it)));
}


void MathCursor::insert(LCursor & cur, char c)
{
	//lyxerr << "inserting '" << c << "'" << endl;
	selClearOrDel(cur);
	plainInsert(cur, MathAtom(new MathCharInset(c)));
}


void MathCursor::insert(LCursor & cur, MathAtom const & t)
{
	macroModeClose(cur);
	selClearOrDel(cur);
	plainInsert(cur, t);
}


void MathCursor::niceInsert(LCursor & cur, string const & t)
{
	MathArray ar;
	asArray(t, ar);
	if (ar.size() == 1)
		niceInsert(cur, ar[0]);
	else
		insert(cur, ar);
}


void MathCursor::niceInsert(LCursor & cur, MathAtom const & t)
{
	macroModeClose(cur);
	string safe = grabAndEraseSelection(cur);
	plainInsert(cur, t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posLeft(cur);
		pushLeft(cur, nextAtom(cur));
		paste(cur, safe);
	}
}


void MathCursor::insert(LCursor & cur, MathArray const & ar)
{
	macroModeClose(cur);
	if (selection_)
		eraseSelection(cur);
	cur.cell().insert(cur.pos(), ar);
	cur.pos() += ar.size();
}


void MathCursor::paste(LCursor & cur, string const & data)
{
	dispatch(cur, FuncRequest(LFUN_PASTE, data));
}


bool MathCursor::backspace(LCursor & cur)
{
	autocorrect_ = false;

	if (selection_) {
		selDel(cur);
		return true;
	}

	if (cur.pos() == 0) {
		if (cur.inset()->asMathInset()->nargs() == 1 &&
			  cur.depth() == 1 &&
			  cur.lastpos() == 0)
			return false;
		pullArg(cur);
		return true;
	}

	if (inMacroMode(cur)) {
		MathUnknownInset * p = activeMacro(cur);
		if (p->name().size() > 1) {
			p->setName(p->name().substr(0, p->name().size() - 1));
			return true;
		}
	}

	if (hasPrevAtom(cur) && prevAtom(cur)->nargs() > 0) {
		// let's require two backspaces for 'big stuff' and
		// highlight on the first
		left(cur, true);
	} else {
		--cur.pos();
		plainErase(cur);
	}
	return true;
}


bool MathCursor::erase(LCursor & cur)
{
	autocorrect_ = false;
	if (inMacroMode(cur))
		return true;

	if (selection_) {
		selDel(cur);
		return true;
	}

	// delete empty cells if possible
#warning FIXME
	//if (cur.cell().empty() && cur.inset()->idxDelete(cur.idx()))
	//		return true;

	// special behaviour when in last position of cell
	if (cur.pos() == cur.lastpos()) {
		bool one_cell = cur.inset()->asMathInset()->nargs() == 1;
		if (one_cell && cur.depth() == 1 && cur.lastpos() == 0)
			return false;
		// remove markup
		if (one_cell)
			pullArg(cur);
		else
			cur.inset()->asMathInset()->idxGlue(cur.idx());
		return true;
	}

	if (hasNextAtom(cur) && nextAtom(cur)->nargs() > 0)
		right(cur, true);
	else
		plainErase(cur);

	return true;
}


bool MathCursor::up(LCursor & cur, bool sel)
{
	dump("up 1");
	macroModeClose(cur);
	selHandle(cur, sel);
#warning FIXME
#if 0
	CursorBase save = Cursor_;
	if (goUpDown(true))
		return true;
	Cursor_ = save;
#endif
	autocorrect_ = false;
	return selection_;
}


bool MathCursor::down(LCursor & cur, bool sel)
{
	dump("down 1");
	macroModeClose(cur);
	selHandle(cur, sel);
#warning FIXME
#if 0
	CursorBase save = Cursor_;
	if (goUpDown(false))
		return true;
	Cursor_ = save;
#endif
	autocorrect_ = false;
	return selection_;
}


void MathCursor::macroModeClose(LCursor & cur)
{
	if (!inMacroMode(cur))
		return;
	MathUnknownInset * p = activeMacro(cur);
	p->finalize();
	string s = p->name();
	--cur.pos();
	cur.cell().erase(cur.pos());

	// do nothing if the macro name is empty
	if (s == "\\")
		return;

	string const name = s.substr(1);

	// prevent entering of recursive macros
	if (formula()->lyxCode() == InsetOld::MATHMACRO_CODE
			&& formula()->getInsetName() == name)
		lyxerr << "can't enter recursive macro" << endl;

	niceInsert(cur, createMathInset(name));
}


string MathCursor::macroName(LCursor & cur) const
{
	return inMacroMode(cur) ? activeMacro(cur)->name() : string();
}


void MathCursor::selClear(LCursor & cur)
{
	cur.resetAnchor();
	cur.bv().clearSelection();
}


void MathCursor::selCopy(LCursor & cur)
{
	dump("selCopy");
	if (selection_) {
		theCutBuffer.push(grabSelection(cur));
		selection_ = false;
	} else {
		//theCutBuffer.erase();
	}
}


void MathCursor::selCut(LCursor & cur)
{
	dump("selCut");
	theCutBuffer.push(grabAndEraseSelection(cur));
}


void MathCursor::selDel(LCursor & cur)
{
	dump("selDel");
	if (selection_) {
		eraseSelection(cur);
		selection_ = false;
	}
}


void MathCursor::selPaste(LCursor & cur, size_t n)
{
	dump("selPaste");
	selClearOrDel(cur);
	if (n < theCutBuffer.size())
		paste(cur, theCutBuffer[n]);
	//grabSelection(cur);
	selection_ = false;
}


void MathCursor::selHandle(LCursor & cur, bool sel)
{
	if (sel == selection_)
		return;
	//clear();
	cur.resetAnchor();
	selection_ = sel;
}


void MathCursor::selStart(LCursor & cur)
{
	dump("selStart 1");
	//clear();
	cur.resetAnchor();
	selection_ = true;
	dump("selStart 2");
}


void MathCursor::selClearOrDel(LCursor & cur)
{
	if (lyxrc.auto_region_delete)
		selDel(cur);
	else
		selection_ = false;
}


void MathCursor::drawSelection(PainterInfo & pi) const
{
	if (!selection_)
		return;
	CursorSlice i1;
	CursorSlice i2;
	getSelection(pi.base.bv->fullCursor(), i1, i2);
	i1.asMathInset()->drawSelection(pi, i1.idx_, i1.pos_, i2.idx_, i2.pos_);
}


void MathCursor::handleNest(LCursor & cur, MathAtom const & a, int c)
{
	MathAtom at = a;
	asArray(grabAndEraseSelection(cur), at.nucleus()->cell(c));
	insert(cur, at);
	pushRight(cur, prevAtom(cur));
}


void MathCursor::getScreenPos(LCursor & cur, int & x, int & y) const
{
	cur.inset()->asMathInset()->getScreenPos(cur.idx(), cur.pos(), x, y);
}


int MathCursor::targetX(LCursor & cur) const
{
	if (cur.bv().x_target() != -1)
		return cur.bv().x_target();
	int x = 0;
	int y = 0;
	getScreenPos(cur, x, y);
	return x;
}


InsetFormulaBase * MathCursor::formula() const
{
	return formula_;
}


void MathCursor::adjust(LCursor & cur, pos_type from, difference_type diff)
{	
	if (cur.pos() > from)
		cur.pos() += diff;
#warning FIXME
#if 0
	if (Anchor_.back().pos_ > from)
		Anchor_.back().pos_ += diff;
	// just to be on the safe side
	// theoretically unecessary
#endif
	normalize(cur);
}


bool MathCursor::inMacroMode(LCursor & cur) const
{
	if (!hasPrevAtom(cur))
		return false;
	MathUnknownInset const * p = prevAtom(cur)->asUnknownInset();
	return p && !p->final();
}


MathUnknownInset * MathCursor::activeMacro(LCursor & cur)
{
	return inMacroMode(cur) ? prevAtom(cur).nucleus()->asUnknownInset() : 0;
}


MathUnknownInset const * MathCursor::activeMacro(LCursor & cur) const
{
	return inMacroMode(cur) ? prevAtom(cur)->asUnknownInset() : 0;
}


bool MathCursor::inMacroArgMode(LCursor & cur) const
{
	return cur.pos() > 0 && prevAtom(cur)->getChar() == '#';
}


bool MathCursor::selection() const
{
	return selection_;
}


MathGridInset *
MathCursor::enclosingGrid(LCursor & cur, MathCursor::idx_type & idx) const
{
	for (MathInset::difference_type i = cur.depth() - 1; i >= 0; --i) {
		MathInset * m = cur.cursor_[i].inset()->asMathInset();
		if (!m)
			return 0;
		MathGridInset * p = m->asGridInset();
		if (p) {
			idx = cur.cursor_[i].idx_;
			return p;
		}
	}
	return 0;
}


void MathCursor::popToHere(LCursor & cur, MathInset const * p)
{
	while (cur.depth() && cur.inset()->asMathInset() != p)
		cur.pop();
}


void MathCursor::popToEnclosingGrid(LCursor & cur)
{
	while (cur.depth() && !cur.inset()->asMathInset()->asGridInset())
		cur.pop();
}


void MathCursor::popToEnclosingHull(LCursor & cur)
{
	while (cur.depth() && !cur.inset()->asMathInset()->asGridInset())
		cur.pop();
}


void MathCursor::pullArg(LCursor & cur)
{
	dump("pullarg");
	MathArray ar = cur.cell();
	if (popLeft(cur)) {
		plainErase(cur);
		cur.cell().insert(cur.pos(), ar);
		cur.resetAnchor();
	} else {
		formula()->mutateToText();
	}
}


void MathCursor::touch()
{
#warning
#if 0
	CursorBase::const_iterator it = Cursor_.begin();
	CursorBase::const_iterator et = Cursor_.end();
	for ( ; it != et; ++it)
		it->cell().touch();
#endif
}


void MathCursor::normalize(LCursor & cur)
{
	if (cur.idx() >= cur.nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << cur.idx() << ' ' << cur.nargs()
		       << " in: " << cur.inset() << endl;
		dump("error 2");
	}
	cur.idx() = min(cur.idx(), cur.nargs() - 1);

	if (cur.pos() > cur.lastpos()) {
		lyxerr << "this should not really happen - 2: "
			<< cur.pos() << ' ' << cur.lastpos() <<  " in idx: " << cur.idx()
		       << " in atom: '";
		WriteStream wi(lyxerr, false, true);
		cur.inset()->asMathInset()->write(wi);
		lyxerr << endl;
		dump("error 4");
	}
	cur.pos() = min(cur.pos(), cur.lastpos());
}


bool MathCursor::hasPrevAtom(LCursor & cur) const
{
	return cur.pos() > 0;
}


bool MathCursor::hasNextAtom(LCursor & cur) const
{
	return cur.pos() < cur.lastpos();
}


MathAtom const & MathCursor::prevAtom(LCursor & cur) const
{
	BOOST_ASSERT(cur.pos() > 0);
	return cur.cell()[cur.pos() - 1];
}


MathAtom & MathCursor::prevAtom(LCursor & cur)
{
	BOOST_ASSERT(cur.pos() > 0);
	return cur.cell()[cur.pos() - 1];
}


MathAtom const & MathCursor::nextAtom(LCursor & cur) const
{
	BOOST_ASSERT(cur.pos() < cur.lastpos());
	return cur.cell()[cur.pos()];
}


MathAtom & MathCursor::nextAtom(LCursor & cur)
{
	BOOST_ASSERT(cur.pos() < cur.lastpos());
	return cur.cell()[cur.pos()];
}


void MathCursor::idxNext(LCursor & cur)
{
	cur.inset()->asMathInset()->idxNext(cur);
}


void MathCursor::idxPrev(LCursor & cur)
{
	cur.inset()->asMathInset()->idxPrev(cur);
}


char MathCursor::valign(LCursor & cur) const
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(cur, idx);
	return p ? p->valign() : '\0';
}


char MathCursor::halign(LCursor & cur) const
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(cur, idx);
	return p ? p->halign(idx % p->ncols()) : '\0';
}


void MathCursor::getSelection(LCursor & cur,
	CursorSlice & i1, CursorSlice & i2) const
{
	CursorSlice anc = normalAnchor(cur);
	if (anc < cur.top()) {
		i1 = anc;
		i2 = cur.top();
	} else {
		i1 = cur.top();
		i2 = anc;
	}
}


bool MathCursor::goUpDown(LCursor & cur, bool up)
{
	// Be warned: The 'logic' implemented in this function is highly fragile.
	// A distance of one pixel or a '<' vs '<=' _really_ matters.
	// So fiddle around with it only if you know what you are doing!
  int xo = 0;
	int yo = 0;
	getScreenPos(cur, xo, yo);

	// check if we had something else in mind, if not, this is the future goal
	if (cur.bv().x_target() == -1)
		cur.bv().x_target(xo);
	else
		xo = cur.bv().x_target();

	// try neigbouring script insets
	if (!selection()) {
		// try left
		if (hasPrevAtom(cur)) {
			MathScriptInset const * p = prevAtom(cur)->asScriptInset();
			if (p && p->has(up)) {
				--cur.pos();
				push(cur, nextAtom(cur));
				cur.idx() = up; // the superscript has index 1
				cur.pos() = cur.lastpos();
				//lyxerr << "updown: handled by scriptinset to the left" << endl;
				return true;
			}
		}

		// try right
		if (hasNextAtom(cur)) {
			MathScriptInset const * p = nextAtom(cur)->asScriptInset();
			if (p && p->has(up)) {
				push(cur, nextAtom(cur));
				cur.idx() = up;
				cur.pos() = 0;
				//lyxerr << "updown: handled by scriptinset to the right" << endl;
				return true;
			}
		}
	}

	// try current cell for e.g. text insets
	if (cur.inset()->asMathInset()->idxUpDown2(cur, up, cur.bv().x_target()))
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
		if (cur.inset()->asMathInset()->idxUpDown(cur, up, cur.bv().x_target())) {
			// try to find best position within this inset
			if (!selection())
				bruteFind2(cur, xo, yo);
			return true;
		}

		// no such inset found, just take something "above"
		//lyxerr << "updown: handled by strange case" << endl;
		if (!popLeft(cur))
			return
				bruteFind(cur, xo, yo,
					formula()->xlow(),
					formula()->xhigh(),
					up ? formula()->ylow() : yo + 4,
					up ? yo - 4 : formula()->yhigh()
				);

		// any improvement so far?
		int xnew, ynew;
		getScreenPos(cur, xnew, ynew);
		if (up ? ynew < yo : ynew > yo)
			return true;
	}
}


bool MathCursor::bruteFind
	(LCursor & cur, int x, int y, int xlow, int xhigh, int ylow, int yhigh)
{
	CursorBase best_cursor;
	double best_dist = 1e10;

	CursorBase it = ibegin(formula()->par().nucleus());
	CursorBase et = iend(formula()->par().nucleus());
	while (1) {
		// avoid invalid nesting when selecting
		if (!selection_ || positionable(it, cur.anchor_)) {
			int xo, yo;
			it.back().getScreenPos(xo, yo);
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
		cur.cursor_ = best_cursor;
	return best_dist < 1e10;
}


void MathCursor::bruteFind2(LCursor & cur, int x, int y)
{
	double best_dist = 1e10;

	CursorBase it = cur.cursor_;
	it.back().pos(0);
	CursorBase et = cur.cursor_;
	int n = et.back().asMathInset()->cell(et.back().idx_).size();
	et.back().pos(n);
	for (int i = 0; ; ++i) {
		int xo, yo;
		it.back().getScreenPos(xo, yo);
		double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
		// '<=' in order to take the last possible position
		// this is important for clicking behind \sum in e.g. '\sum_i a'
		lyxerr << "i: " << i << " d: " << d << " best: " << best_dist << endl;
		if (d <= best_dist) {
			best_dist = d;
			cur.cursor_ = it;
		}
		if (it == et)
			break;
		increment(it);
	}
}


bool MathCursor::idxLineLast(LCursor & cur)
{
	cur.idx() -= cur.idx() % cur.ncols();
	cur.idx() += cur.ncols() - 1;
	cur.pos() = cur.lastpos();
	return true;
}


bool MathCursor::idxLeft(LCursor & cur)
{
	return cur.inset()->asMathInset()->idxLeft(cur);
}


bool MathCursor::idxRight(LCursor & cur)
{
	return cur.inset()->asMathInset()->idxRight(cur);
}


bool MathCursor::script(LCursor & cur, bool up)
{
	// Hack to get \\^ and \\_ working
	if (inMacroMode(cur) && macroName(cur) == "\\") {
		if (up)
			niceInsert(cur, createMathInset("mathcircumflex"));
		else
			interpret(cur, '_');
		return true;
	}

	macroModeClose(cur);
	string safe = grabAndEraseSelection(cur);
	if (inNucleus(cur)) {
		// we are in a nucleus of a script inset, move to _our_ script
		cur.inset()->asMathInset()->asScriptInset()->ensure(up);
		cur.idx() = up;
		cur.pos() = 0;
	} else if (hasPrevAtom(cur) && prevAtom(cur)->asScriptInset()) {
		prevAtom(cur).nucleus()->asScriptInset()->ensure(up);
		pushRight(cur, prevAtom(cur));
		cur.idx() = up;
		cur.pos() = cur.lastpos();
	} else if (hasPrevAtom(cur)) {
		--cur.pos();
		cur.cell()[cur.pos()]
			= MathAtom(new MathScriptInset(nextAtom(cur), up));
		pushLeft(cur, nextAtom(cur));
		cur.idx() = up;
		cur.pos() = 0;
	} else {
		plainInsert(cur, MathAtom(new MathScriptInset(up)));
		prevAtom(cur).nucleus()->asScriptInset()->ensure(up);
		pushRight(cur, prevAtom(cur));
		cur.idx() = up;
		cur.pos() = 0;
	}
	paste(cur, safe);
	dump("1");
	return true;
}


bool MathCursor::interpret(LCursor & cur, char c)
{
	//lyxerr << "interpret 2: '" << c << "'" << endl;
	cur.bv().x_target(-1); // "no target"
	if (inMacroArgMode(cur)) {
		--cur.pos();
		plainErase(cur);
		int n = c - '0';
		MathMacroTemplate const * p = formula()->par()->asMacroTemplate();
		if (p && 1 <= n && n <= p->numargs())
			insert(cur, MathAtom(new MathMacroArgument(c - '0')));
		else {
			insert(cur, createMathInset("#"));
			interpret(cur, c); // try again
		}
		return true;
	}

	// handle macroMode
	if (inMacroMode(cur)) {
		string name = macroName(cur);
		//lyxerr << "interpret name: '" << name << "'" << endl;

		if (isalpha(c)) {
			activeMacro(cur)->setName(activeMacro(cur)->name() + c);
			return true;
		}

		// handle 'special char' macros
		if (name == "\\") {
			// remove the '\\'
			backspace(cur);
			if (c == '\\') {
				if (currentMode(cur) == MathInset::TEXT_MODE)
					niceInsert(cur, createMathInset("textbackslash"));
				else
					niceInsert(cur, createMathInset("backslash"));
			} else if (c == '{') {
				niceInsert(cur, MathAtom(new MathBraceInset));
			} else {
				niceInsert(cur, createMathInset(string(1, c)));
			}
			return true;
		}

		// leave macro mode and try again if necessary
		macroModeClose(cur);
		if (c == '{')
			niceInsert(cur, MathAtom(new MathBraceInset));
		else if (c != ' ')
			interpret(cur, c);
		return true;
	}

	// This is annoying as one has to press <space> far too often.
	// Disable it.

	if (0) {
		// leave autocorrect mode if necessary
		if (autocorrect_ && c == ' ') {
			autocorrect_ = false;
			return true;
		}
	}

	// just clear selection on pressing the space bar
	if (selection_ && c == ' ') {
		selection_ = false;
		return true;
	}

	selClearOrDel(cur);

	if (c == '\\') {
		//lyxerr << "starting with macro" << endl;
		insert(cur, MathAtom(new MathUnknownInset("\\", false)));
		return true;
	}

	if (c == '\n') {
		if (currentMode(cur) == MathInset::TEXT_MODE)
			insert(cur, c);
		return true;
	}

	if (c == ' ') {
		if (currentMode(cur) == MathInset::TEXT_MODE) {
			// insert spaces in text mode,
			// but suppress direct insertion of two spaces in a row
			// the still allows typing  '<space>a<space>' and deleting the 'a', but
			// it is better than nothing...
			if (!hasPrevAtom(cur) || prevAtom(cur)->getChar() != ' ')
				insert(cur, c);
			return true;
		}
		if (hasPrevAtom(cur) && prevAtom(cur)->asSpaceInset()) {
			prevAtom(cur).nucleus()->asSpaceInset()->incSpace();
			return true;
		}
		if (popRight(cur))
			return true;
		// if are at the very end, leave the formula
		return cur.pos() != cur.lastpos();
	}

	if (c == '_') {
		script(cur, false);
		return true;
	}

	if (c == '^') {
		script(cur, true);
		return true;
	}

	if (c == '{' || c == '}' || c == '#' || c == '&' || c == '$') {
		niceInsert(cur, createMathInset(string(1, c)));
		return true;
	}

	if (c == '%') {
		niceInsert(cur, MathAtom(new MathCommentInset));
		return true;
	}

	// try auto-correction
	//if (autocorrect_ && hasPrevAtom() && math_autocorrect(prevAtom(), c))
	//	return true;

	// no special circumstances, so insert the character without any fuss
	insert(cur, c);
	autocorrect_ = true;
	return true;
}


void MathCursor::setSelection
	(LCursor & cur, CursorBase const & where, size_t n)
{
	selection_ = true;
	cur.cursor_ = where;
	cur.anchor_ = where;
	cur.pos() += n;
}


void MathCursor::insetToggle(LCursor & cur)
{
	if (hasNextAtom(cur)) {
		// toggle previous inset ...
		nextAtom(cur).nucleus()->lock(!nextAtom(cur)->lock());
	} else if (popLeft(cur) && hasNextAtom(cur)) {
		// ... or enclosing inset if we are in the last inset position
		nextAtom(cur).nucleus()->lock(!nextAtom(cur)->lock());
		posRight(cur);
	}
}


string MathCursor::info(LCursor & cur) const
{
	ostringstream os;
	os << "Math editor mode.  ";
	for (int i = 0, n = cur.depth(); i < n; ++i) {
		cur.cursor_[i].asMathInset()->infoize(os);
		os << "  ";
	}
	if (hasPrevAtom(cur))
		prevAtom(cur)->infoize2(os);
	os << "                    ";
	return os.str();
}


namespace {

void region(CursorSlice const & i1, CursorSlice const & i2,
	MathInset::row_type & r1, MathInset::row_type & r2,
	MathInset::col_type & c1, MathInset::col_type & c2)
{
	MathInset * p = i1.asMathInset();
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


string MathCursor::grabSelection(LCursor & cur) const
{
	if (!selection_)
		return string();

	CursorSlice i1;
	CursorSlice i2;
	getSelection(cur, i1, i2);

	if (i1.idx_ == i2.idx_) {
		MathArray::const_iterator it = i1.cell().begin();
		return asString(MathArray(it + i1.pos_, it + i2.pos_));
	}

	row_type r1, r2;
	col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);

	string data;
	for (row_type row = r1; row <= r2; ++row) {
		if (row > r1)
			data += "\\\\";
		for (col_type col = c1; col <= c2; ++col) {
			if (col > c1)
				data += '&';
			data += asString(i1.asMathInset()->cell(i1.asMathInset()->index(row, col)));
		}
	}
	return data;
}


void MathCursor::eraseSelection(LCursor & cur)
{
	CursorSlice i1;
	CursorSlice i2;
	getSelection(cur, i1, i2);
	if (i1.idx_ == i2.idx_)
		i1.cell().erase(i1.pos_, i2.pos_);
	else {
		MathInset * p = i1.asMathInset();
		row_type r1, r2;
		col_type c1, c2;
		region(i1, i2, r1, r2, c1, c2);
		for (row_type row = r1; row <= r2; ++row)
			for (col_type col = c1; col <= c2; ++col)
				p->cell(p->index(row, col)).clear();
	}
	cur.top() = i1;
}


string MathCursor::grabAndEraseSelection(LCursor & cur)
{
	if (!selection_)
		return string();
	string res = grabSelection(cur);
	eraseSelection(cur);
	selection_ = false;
	return res;
}


CursorSlice MathCursor::normalAnchor(LCursor & cur) const
{
#warning FIXME
#if 0
	if (Anchor_.size() < cur.depth()) {
		cur.resetAnchor();
		lyxerr << "unusual Anchor size" << endl;
	}
	//lyx::BOOST_ASSERT(Anchor_.size() >= cursor.cur.depth());
	// use Anchor on the same level as Cursor
	CursorSlice normal = Anchor_[cur.depth() - 1];
	if (cur.depth() < Anchor_.size() && !(normal < cursor())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
	return normal;
#else
	return cur.top();
#endif
}


DispatchResult MathCursor::dispatch(LCursor &, FuncRequest const & cmd)
{
	// mouse clicks are somewhat special
	// check
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
		case LFUN_MOUSE_MOTION:
		case LFUN_MOUSE_RELEASE:
		case LFUN_MOUSE_DOUBLE: {
/*
			CursorSlice & pos = Cursor_.back();
			int x = 0;
			int y = 0;
			getScreenPos(x, y);
			if (x < cmd.x && hasPrevAtom()) {
				DispatchResult const res =
					prevAtom().nucleus()->dispatch(cur, cmd);
				if (res.dispatched())
					return res;
			}
			if (x > cmd.x && hasNextAtom()) {
				DispatchResult const res =
					nextAtom().nucleus()->dispatch(cur, cmd);
				if (res.dispatched())
					return res;
			}
*/
		}
		default:
			break;
	}

/*
	for (int i = cur.depth() - 1; i >= 0; --i) {
		CursorBase tmp = cur->cursor_;
		CursorSlice & pos = tmp.back()
		DispatchResult const res = pos.asMathInset()->dispatch(cur, cmd);
		if (res.dispatched()) {
			if (res.val() == FINISHED) {
				if (i + 1 < Cursor_.size())
					Cursor_.erase(Cursor_.begin() + i + 1, Cursor_.end());
				selClear();
			}
			return res;
		}
	}
*/
	return DispatchResult(false);
}


MathInset::mode_type MathCursor::currentMode(LCursor &) const
{
#if 0
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		MathInset::mode_type res = Cursor_[i].asMathInset()->currentMode();
		if (res != MathInset::UNDECIDED_MODE)
			return res;
	}
#endif
	return MathInset::UNDECIDED_MODE;
}


void MathCursor::handleFont(LCursor & cur, string const & font)
{
	string safe;
	if (selection()) {
		macroModeClose(cur);
		safe = grabAndEraseSelection(cur);
	}

	if (cur.lastpos() != 0) {
		// something left in the cell
		if (cur.pos() == 0) {
			// cursor in first position
			popLeft(cur);
		} else if (cur.pos() == cur.lastpos()) {
			// cursor in last position
			popRight(cur);
		} else {
			// cursor in between. split cell
			MathArray::iterator bt = cur.cell().begin();
			MathAtom at = createMathInset(font);
			at.nucleus()->cell(0) = MathArray(bt, bt + cur.pos());
			cur.cell().erase(bt, bt + cur.pos());
			popLeft(cur);
			plainInsert(cur, at);
		}
	} else {
		// nothing left in the cell
		pullArg(cur);
		plainErase(cur);
	}
	insert(cur, safe);
}


void releaseMathCursor(BufferView & bv)
{
	if (mathcursor) {
		InsetFormulaBase * f = mathcursor->formula();
		delete mathcursor;
		mathcursor = 0;
		f->insetUnlock(bv);
	}
}
