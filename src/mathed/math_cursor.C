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
	front ? first(*bv) : last(*bv);
}


MathCursor::~MathCursor()
{
	// ensure that 'notifyCursorLeave' is called
	//while (popLeft())
	//	;
}


void MathCursor::push(BufferView & bv, MathAtom & t)
{
	bv.fullCursor().push(t.nucleus());
}


void MathCursor::pushLeft(BufferView & bv, MathAtom & t)
{
	//lyxerr << "Entering atom " << t << " left" << endl;
	push(bv, t);
	t->idxFirst(bv);
}


void MathCursor::pushRight(BufferView & bv, MathAtom & t)
{
	//lyxerr << "Entering atom " << t << " right" << endl;
	posLeft(bv);
	push(bv, t);
	t->idxLast(bv);
}


bool MathCursor::popLeft(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	//lyxerr << "Leaving atom to the left" << endl;
	if (depth(bv) <= 1) {
		if (depth(bv) == 1)
			cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
		return false;
	}
	cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
	bv.fullCursor().pop();
	return true;
}


bool MathCursor::popRight(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	//lyxerr << "Leaving atom "; bv.inset->asMathInset()->write(cerr, false); cerr << " right" << endl;
	if (depth(bv) <= 1) {
		if (depth(bv) == 1)
			cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
		return false;
	}
	cur.inset()->asMathInset()->notifyCursorLeaves(cur.idx());
	bv.fullCursor().pop();
	posRight(bv);
	return true;
}



#if FILEDEBUG
	void MathCursor::dump(char const * what) const
	{
		lyxerr << "MC: " << what << endl;
		lyxerr << " Cursor: " << depth() << endl;
		for (unsigned i = 0; i < depth(); ++i)
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
	for (unsigned i = 0; i < depth(); ++i)
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
		if (depth() == Anchor_.size())
			return false;
		if (t.operator->() != Anchor_[depth()].asMathInset())
			return false;
	}
#else
	if (sel)
		return false;
#endif

	return true;
}


bool MathCursor::inNucleus(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);	
	return cur.inset()->asMathInset()->asScriptInset() && cur.idx() == 2;
}


bool MathCursor::posLeft(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);	
	if (cur.pos() == 0)
		return false;
	--cur.pos();
	return true;
}


bool MathCursor::posRight(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);	
	if (cur.pos() == cur.lastpos())
		return false;
	++cur.pos();
	return true;
}


bool MathCursor::left(BufferView & bv, bool sel)
{
	dump("Left 1");
	autocorrect_ = false;
	bv.x_target(-1); // "no target"
	if (inMacroMode(bv)) {
		macroModeClose(bv);
		return true;
	}
	selHandle(bv, sel);

	if (hasPrevAtom(bv) && openable(prevAtom(bv), sel)) {
		pushRight(bv, prevAtom(bv));
		return true;
	}

	return posLeft(bv) || idxLeft(bv) || popLeft(bv) || selection_;
}


bool MathCursor::right(BufferView & bv, bool sel)
{
	dump("Right 1");
	autocorrect_ = false;
	bv.x_target(-1); // "no target"
	if (inMacroMode(bv)) {
		macroModeClose(bv);
		return true;
	}
	selHandle(bv, sel);

	if (hasNextAtom(bv) && openable(nextAtom(bv), sel)) {
		pushLeft(bv, nextAtom(bv));
		return true;
	}

	return posRight(bv) || idxRight(bv) || popRight(bv) || selection_;
}


void MathCursor::first(BufferView & bv)
{
#warning FIXME
	//Cursor_.clear();
	push(bv, formula_->par());
	bv.cursor().inset()->asMathInset()->idxFirst(bv);
	bv.resetAnchor();
}


void MathCursor::last(BufferView & bv)
{
#warning FIXME
	//Cursor_.clear();
	push(bv, formula_->par());
	bv.cursor().inset()->asMathInset()->idxLast(bv);
	bv.resetAnchor();
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


void MathCursor::setScreenPos(BufferView & bv, int x, int y)
{
	dump("setScreenPos 1");
	bool res = bruteFind(bv, x, y,
		formula()->xlow(), formula()->xhigh(),
		formula()->ylow(), formula()->yhigh());
	if (!res) {
		// this can happen on creation of "math-display"
		dump("setScreenPos 1.5");
		first(bv);
	}
	bv.x_target(-1); // "no target"
	dump("setScreenPos 2");
}



bool MathCursor::home(BufferView & bv, bool sel)
{
	dump("home 1");
	autocorrect_ = false;
	selHandle(bv, sel);
	macroModeClose(bv);
	if (!bv.cursor().inset()->asMathInset()->idxHome(bv))
		return popLeft(bv);
	dump("home 2");
	bv.x_target(-1); // "no target"
	return true;
}


bool MathCursor::end(BufferView & bv, bool sel)
{
	dump("end 1");
	autocorrect_ = false;
	selHandle(bv, sel);
	macroModeClose(bv);
	if (!bv.cursor().inset()->asMathInset()->idxEnd(bv))
		return popRight(bv);
	dump("end 2");
	bv.x_target(-1); // "no target"
	return true;
}


void MathCursor::plainErase(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	cur.cell().erase(cur.pos());
}


void MathCursor::markInsert(BufferView & bv)
{
	//lyxerr << "inserting mark" << endl;
	CursorSlice & cur = cursorTip(bv);
	cur.cell().insert(cur.pos(), MathAtom(new MathCharInset(0)));
}


void MathCursor::markErase(BufferView & bv)
{
	//lyxerr << "deleting mark" << endl;
	CursorSlice & cur = cursorTip(bv);
	cur.cell().erase(cur.pos());
}


void MathCursor::plainInsert(BufferView & bv, MathAtom const & t)
{
	dump("plainInsert");
	CursorSlice & cur = cursorTip(bv);
	cur.cell().insert(cur.pos(), t);
	++cur.pos();
}


void MathCursor::insert2(BufferView & bv, string const & str)
{
	MathArray ar;
	asArray(str, ar);
	insert(bv, ar);
}


void MathCursor::insert(BufferView & bv, string const & str)
{
	//lyxerr << "inserting '" << str << "'" << endl;
	selClearOrDel(bv);
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		plainInsert(bv, MathAtom(new MathCharInset(*it)));
}


void MathCursor::insert(BufferView & bv, char c)
{
	//lyxerr << "inserting '" << c << "'" << endl;
	selClearOrDel(bv);
	plainInsert(bv, MathAtom(new MathCharInset(c)));
}


void MathCursor::insert(BufferView & bv, MathAtom const & t)
{
	macroModeClose(bv);
	selClearOrDel(bv);
	plainInsert(bv, t);
}


void MathCursor::niceInsert(BufferView & bv, string const & t)
{
	MathArray ar;
	asArray(t, ar);
	if (ar.size() == 1)
		niceInsert(bv, ar[0]);
	else
		insert(bv, ar);
}


void MathCursor::niceInsert(BufferView & bv, MathAtom const & t)
{
	macroModeClose(bv);
	string safe = grabAndEraseSelection(bv);
	plainInsert(bv, t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posLeft(bv);
		pushLeft(bv, nextAtom(bv));
		paste(bv, safe);
	}
}


void MathCursor::insert(BufferView & bv, MathArray const & ar)
{
	CursorSlice & cur = cursorTip(bv);
	macroModeClose(bv);
	if (selection_)
		eraseSelection(bv);
	cur.cell().insert(cur.pos(), ar);
	cur.pos() += ar.size();
}


void MathCursor::paste(BufferView & bv, string const & data)
{
	dispatch(bv, FuncRequest(LFUN_PASTE, data));
}


bool MathCursor::backspace(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	autocorrect_ = false;

	if (selection_) {
		selDel(bv);
		return true;
	}

	if (cur.pos() == 0) {
		if (cur.inset()->asMathInset()->nargs() == 1 &&
			  depth(bv) == 1 &&
			  cur.lastpos() == 0)
			return false;
		pullArg(bv);
		return true;
	}

	if (inMacroMode(bv)) {
		MathUnknownInset * p = activeMacro(bv);
		if (p->name().size() > 1) {
			p->setName(p->name().substr(0, p->name().size() - 1));
			return true;
		}
	}

	if (hasPrevAtom(bv) && prevAtom(bv)->nargs() > 0) {
		// let's require two backspaces for 'big stuff' and
		// highlight on the first
		left(bv, true);
	} else {
		--cur.pos();
		plainErase(bv);
	}
	return true;
}


bool MathCursor::erase(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	autocorrect_ = false;
	if (inMacroMode(bv))
		return true;

	if (selection_) {
		selDel(bv);
		return true;
	}

	// delete empty cells if possible
#warning FIXME
	//if (cur.cell().empty() && cur.inset()->idxDelete(cur.idx()))
	//		return true;

	// special behaviour when in last position of cell
	if (cur.pos() == cur.lastpos()) {
		bool one_cell = cur.inset()->asMathInset()->nargs() == 1;
		if (one_cell && depth(bv) == 1 && cur.lastpos() == 0)
			return false;
		// remove markup
		if (one_cell)
			pullArg(bv);
		else
			cur.inset()->asMathInset()->idxGlue(cur.idx());
		return true;
	}

	if (hasNextAtom(bv) && nextAtom(bv)->nargs() > 0)
		right(bv, true);
	else
		plainErase(bv);

	return true;
}


bool MathCursor::up(BufferView & bv, bool sel)
{
	dump("up 1");
	macroModeClose(bv);
	selHandle(bv, sel);
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


bool MathCursor::down(BufferView & bv, bool sel)
{
	dump("down 1");
	macroModeClose(bv);
	selHandle(bv, sel);
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


void MathCursor::macroModeClose(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);	
	if (!inMacroMode(bv))
		return;
	MathUnknownInset * p = activeMacro(bv);
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

	niceInsert(bv, createMathInset(name));
}


string MathCursor::macroName(BufferView & bv) const
{
	return inMacroMode(bv) ? activeMacro(bv)->name() : string();
}


void MathCursor::selClear(BufferView & bv)
{
	bv.resetAnchor();
	bv.clearSelection();
}


void MathCursor::selCopy(BufferView & bv)
{
	dump("selCopy");
	if (selection_) {
		theCutBuffer.push(grabSelection(bv));
		selection_ = false;
	} else {
		//theCutBuffer.erase();
	}
}


void MathCursor::selCut(BufferView & bv)
{
	dump("selCut");
	theCutBuffer.push(grabAndEraseSelection(bv));
}


void MathCursor::selDel(BufferView & bv)
{
	dump("selDel");
	if (selection_) {
		eraseSelection(bv);
		selection_ = false;
	}
}


void MathCursor::selPaste(BufferView & bv, size_t n)
{
	dump("selPaste");
	selClearOrDel(bv);
	if (n < theCutBuffer.size())
		paste(bv, theCutBuffer[n]);
	//grabSelection(bv);
	selection_ = false;
}


void MathCursor::selHandle(BufferView & bv, bool sel)
{
	if (sel == selection_)
		return;
	//clear();
	bv.resetAnchor();
	selection_ = sel;
}


void MathCursor::selStart(BufferView & bv)
{
	dump("selStart 1");
	//clear();
	bv.resetAnchor();
	selection_ = true;
	dump("selStart 2");
}


void MathCursor::selClearOrDel(BufferView & bv)
{
	if (lyxrc.auto_region_delete)
		selDel(bv);
	else
		selection_ = false;
}


void MathCursor::drawSelection(PainterInfo & pi) const
{
	if (!selection_)
		return;
	CursorSlice i1;
	CursorSlice i2;
	getSelection(*pi.base.bv, i1, i2);
	i1.asMathInset()->drawSelection(pi, i1.idx_, i1.pos_, i2.idx_, i2.pos_);
}


void MathCursor::handleNest(BufferView & bv, MathAtom const & a, int c)
{
	MathAtom at = a;
	asArray(grabAndEraseSelection(bv), at.nucleus()->cell(c));
	insert(bv, at);
	pushRight(bv, prevAtom(bv));
}


void MathCursor::getScreenPos(BufferView & bv, int & x, int & y) const
{
	CursorSlice & cur = cursorTip(bv);
	cur.inset()->asMathInset()->getScreenPos(cur.idx(), cur.pos(), x, y);
}


int MathCursor::targetX(BufferView & bv) const
{
	if (bv.x_target() != -1)
		return bv.x_target();
	int x = 0;
	int y = 0;
	getScreenPos(bv, x, y);
	return x;
}


InsetFormulaBase * MathCursor::formula() const
{
	return formula_;
}


void MathCursor::adjust(BufferView & bv, pos_type from, difference_type diff)
{	
	CursorSlice & cur = cursorTip(bv);
	if (cur.pos() > from)
		cur.pos() += diff;
#warning FIXME
#if 0
	if (Anchor_.back().pos_ > from)
		Anchor_.back().pos_ += diff;
	// just to be on the safe side
	// theoretically unecessary
#endif
	normalize(bv);
}


bool MathCursor::inMacroMode(BufferView & bv) const
{
	if (!hasPrevAtom(bv))
		return false;
	MathUnknownInset const * p = prevAtom(bv)->asUnknownInset();
	return p && !p->final();
}


MathUnknownInset * MathCursor::activeMacro(BufferView & bv)
{
	return inMacroMode(bv) ? prevAtom(bv).nucleus()->asUnknownInset() : 0;
}


MathUnknownInset const * MathCursor::activeMacro(BufferView & bv) const
{
	return inMacroMode(bv) ? prevAtom(bv)->asUnknownInset() : 0;
}


bool MathCursor::inMacroArgMode(BufferView & bv) const
{
	return bv.cursor().pos() > 0 && prevAtom(bv)->getChar() == '#';
}


bool MathCursor::selection() const
{
	return selection_;
}


MathGridInset * MathCursor::enclosingGrid
	(BufferView &, MathCursor::idx_type &) const
{
#warning FIXME
#if 0
	for (MathInset::difference_type i = depth() - 1; i >= 0; --i) {
		MathGridInset * p = Cursor_[i].asMathInset()->asGridInset();
		if (p) {
			idx = Cursor_[i].idx_;
			return p;
		}
	}
#endif
	return 0;
}


void MathCursor::popToHere(BufferView & bv, MathInset const * p)
{
	while (depth(bv) && bv.cursor().asMathInset() != p)
		bv.fullCursor().cursor_.pop_back();
}


void MathCursor::popToEnclosingGrid(BufferView & bv)
{
	while (depth(bv) && !bv.cursor().asMathInset()->asGridInset())
		bv.fullCursor().cursor_.pop_back();
}


void MathCursor::popToEnclosingHull(BufferView & bv)
{
	while (depth(bv) && !bv.cursor().asMathInset()->asGridInset())
		bv.fullCursor().cursor_.pop_back();
}


void MathCursor::pullArg(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	dump("pullarg");
	MathArray ar = cur.cell();
	if (popLeft(bv)) {
		plainErase(bv);
		cur.cell().insert(cur.pos(), ar);
		bv.resetAnchor();
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


void MathCursor::normalize(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
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


bool MathCursor::hasPrevAtom(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);
	return cur.pos() > 0;
}


bool MathCursor::hasNextAtom(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);
	return cur.pos() < cur.lastpos();
}


MathAtom const & MathCursor::prevAtom(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);
	BOOST_ASSERT(cur.pos() > 0);
	return cur.cell()[cur.pos() - 1];
}


MathAtom & MathCursor::prevAtom(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	BOOST_ASSERT(cur.pos() > 0);
	return cur.cell()[cur.pos() - 1];
}


MathAtom const & MathCursor::nextAtom(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);
	BOOST_ASSERT(cur.pos() < cur.lastpos());
	return cur.cell()[cur.pos()];
}


MathAtom & MathCursor::nextAtom(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	BOOST_ASSERT(cur.pos() < cur.lastpos());
	return cur.cell()[cur.pos()];
}


void MathCursor::idxNext(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	cur.inset()->asMathInset()->idxNext(bv);
}


void MathCursor::idxPrev(BufferView & bv)
{
	CursorSlice & cur = cursorTip(bv);
	cur.inset()->asMathInset()->idxPrev(bv);
}


char MathCursor::valign(BufferView & bv) const
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(bv, idx);
	return p ? p->valign() : '\0';
}


char MathCursor::halign(BufferView & bv) const
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(bv, idx);
	return p ? p->halign(idx % p->ncols()) : '\0';
}


void MathCursor::getSelection(BufferView & bv,
	CursorSlice & i1, CursorSlice & i2) const
{
	CursorSlice anc = normalAnchor(bv);
	if (anc < bv.cursor()) {
		i1 = anc;
		i2 = bv.cursor();
	} else {
		i1 = bv.cursor();
		i2 = anc;
	}
}


bool MathCursor::goUpDown(BufferView & bv, bool up)
{
	// Be warned: The 'logic' implemented in this function is highly fragile.
	// A distance of one pixel or a '<' vs '<=' _really_ matters.
	// So fiddle around with it only if you know what you are doing!
  int xo = 0;
	int yo = 0;
	getScreenPos(bv, xo, yo);

	// check if we had something else in mind, if not, this is the future goal
	if (bv.x_target() == -1)
		bv.x_target(xo);
	else
		xo = bv.x_target();

	// try neigbouring script insets
	if (!selection()) {
		// try left
		if (hasPrevAtom(bv)) {
			MathScriptInset const * p = prevAtom(bv)->asScriptInset();
			if (p && p->has(up)) {
				--bv.cursor().pos();
				push(bv, nextAtom(bv));
				bv.cursor().idx() = up; // the superscript has index 1
				bv.cursor().pos() = bv.cursor().lastpos();
				//lyxerr << "updown: handled by scriptinset to the left" << endl;
				return true;
			}
		}

		// try right
		if (hasNextAtom(bv)) {
			MathScriptInset const * p = nextAtom(bv)->asScriptInset();
			if (p && p->has(up)) {
				push(bv, nextAtom(bv));
				bv.cursor().idx() = up;
				bv.cursor().pos() = 0;
				//lyxerr << "updown: handled by scriptinset to the right" << endl;
				return true;
			}
		}
	}

	// try current cell for e.g. text insets
	if (bv.cursor().inset()->asMathInset()->idxUpDown2(bv, up, bv.x_target()))
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
		if (bv.cursor().inset()->asMathInset()->idxUpDown(bv, up, bv.x_target())) {
			// try to find best position within this inset
			if (!selection())
				bruteFind2(bv, xo, yo);
			return true;
		}

		// no such inset found, just take something "above"
		//lyxerr << "updown: handled by strange case" << endl;
		if (!popLeft(bv))
			return
				bruteFind(bv, xo, yo,
					formula()->xlow(),
					formula()->xhigh(),
					up ? formula()->ylow() : yo + 4,
					up ? yo - 4 : formula()->yhigh()
				);

		// any improvement so far?
		int xnew, ynew;
		getScreenPos(bv, xnew, ynew);
		if (up ? ynew < yo : ynew > yo)
			return true;
	}
}


bool MathCursor::bruteFind
	(BufferView & bv, int x, int y, int xlow, int xhigh, int ylow, int yhigh)
{
	CursorBase best_cursor;
	double best_dist = 1e10;

	CursorBase it = ibegin(formula()->par().nucleus());
	CursorBase et = iend(formula()->par().nucleus());
	while (1) {
		// avoid invalid nesting when selecting
		if (!selection_ || positionable(it, bv.fullCursor().anchor_)) {
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
		bv.fullCursor().cursor_ = best_cursor;
	return best_dist < 1e10;
}


void MathCursor::bruteFind2(BufferView & bv, int x, int y)
{
	double best_dist = 1e10;

	CursorBase it = bv.fullCursor().cursor_;
	it.back().pos(0);
	CursorBase et = bv.fullCursor().cursor_;
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
			bv.fullCursor().cursor_ = it;
		}
		if (it == et)
			break;
		increment(it);
	}
}


bool MathCursor::idxLineLast(BufferView & bv)
{
	CursorSlice & cur = bv.cursor();
	cur.idx() -= cur.idx() % cur.ncols();
	cur.idx() += cur.ncols() - 1;
	cur.pos() = cur.lastpos();
	return true;
}


bool MathCursor::idxLeft(BufferView & bv)
{
	return bv.cursor().inset()->asMathInset()->idxLeft(bv);
}


bool MathCursor::idxRight(BufferView & bv)
{
	return bv.cursor().inset()->asMathInset()->idxRight(bv);
}


bool MathCursor::script(BufferView & bv, bool up)
{
	// Hack to get \\^ and \\_ working
	if (inMacroMode(bv) && macroName(bv) == "\\") {
		if (up)
			niceInsert(bv, createMathInset("mathcircumflex"));
		else
			interpret(bv, '_');
		return true;
	}

	macroModeClose(bv);
	string safe = grabAndEraseSelection(bv);
	if (inNucleus(bv)) {
		// we are in a nucleus of a script inset, move to _our_ script
		bv.cursor().inset()->asMathInset()->asScriptInset()->ensure(up);
		bv.cursor().idx() = up;
		bv.cursor().pos() = 0;
	} else if (hasPrevAtom(bv) && prevAtom(bv)->asScriptInset()) {
		prevAtom(bv).nucleus()->asScriptInset()->ensure(up);
		pushRight(bv, prevAtom(bv));
		bv.cursor().idx() = up;
		bv.cursor().pos() = bv.cursor().lastpos();
	} else if (hasPrevAtom(bv)) {
		--bv.cursor().pos();
		bv.cursor().cell()[bv.cursor().pos()]
			= MathAtom(new MathScriptInset(nextAtom(bv), up));
		pushLeft(bv, nextAtom(bv));
		bv.cursor().idx() = up;
		bv.cursor().pos() = 0;
	} else {
		plainInsert(bv, MathAtom(new MathScriptInset(up)));
		prevAtom(bv).nucleus()->asScriptInset()->ensure(up);
		pushRight(bv, prevAtom(bv));
		bv.cursor().idx() = up;
		bv.cursor().pos() = 0;
	}
	paste(bv, safe);
	dump("1");
	return true;
}


bool MathCursor::interpret(BufferView & bv, char c)
{
	//lyxerr << "interpret 2: '" << c << "'" << endl;
	CursorSlice & cur = bv.cursor();
	bv.x_target(-1); // "no target"
	if (inMacroArgMode(bv)) {
		--cur.pos();
		plainErase(bv);
		int n = c - '0';
		MathMacroTemplate const * p = formula()->par()->asMacroTemplate();
		if (p && 1 <= n && n <= p->numargs())
			insert(bv, MathAtom(new MathMacroArgument(c - '0')));
		else {
			insert(bv, createMathInset("#"));
			interpret(bv, c); // try again
		}
		return true;
	}

	// handle macroMode
	if (inMacroMode(bv)) {
		string name = macroName(bv);
		//lyxerr << "interpret name: '" << name << "'" << endl;

		if (isalpha(c)) {
			activeMacro(bv)->setName(activeMacro(bv)->name() + c);
			return true;
		}

		// handle 'special char' macros
		if (name == "\\") {
			// remove the '\\'
			backspace(bv);
			if (c == '\\') {
				if (currentMode(bv) == MathInset::TEXT_MODE)
					niceInsert(bv, createMathInset("textbackslash"));
				else
					niceInsert(bv, createMathInset("backslash"));
			} else if (c == '{') {
				niceInsert(bv, MathAtom(new MathBraceInset));
			} else {
				niceInsert(bv, createMathInset(string(1, c)));
			}
			return true;
		}

		// leave macro mode and try again if necessary
		macroModeClose(bv);
		if (c == '{')
			niceInsert(bv, MathAtom(new MathBraceInset));
		else if (c != ' ')
			interpret(bv, c);
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

	selClearOrDel(bv);

	if (c == '\\') {
		//lyxerr << "starting with macro" << endl;
		insert(bv, MathAtom(new MathUnknownInset("\\", false)));
		return true;
	}

	if (c == '\n') {
		if (currentMode(bv) == MathInset::TEXT_MODE)
			insert(bv, c);
		return true;
	}

	if (c == ' ') {
		if (currentMode(bv) == MathInset::TEXT_MODE) {
			// insert spaces in text mode,
			// but suppress direct insertion of two spaces in a row
			// the still allows typing  '<space>a<space>' and deleting the 'a', but
			// it is better than nothing...
			if (!hasPrevAtom(bv) || prevAtom(bv)->getChar() != ' ')
				insert(bv, c);
			return true;
		}
		if (hasPrevAtom(bv) && prevAtom(bv)->asSpaceInset()) {
			prevAtom(bv).nucleus()->asSpaceInset()->incSpace();
			return true;
		}
		if (popRight(bv))
			return true;
		// if are at the very end, leave the formula
		return cur.pos() != cur.lastpos();
	}

	if (c == '_') {
		script(bv, false);
		return true;
	}

	if (c == '^') {
		script(bv, true);
		return true;
	}

	if (c == '{' || c == '}' || c == '#' || c == '&' || c == '$') {
		niceInsert(bv, createMathInset(string(1, c)));
		return true;
	}

	if (c == '%') {
		niceInsert(bv, MathAtom(new MathCommentInset));
		return true;
	}

	// try auto-correction
	//if (autocorrect_ && hasPrevAtom() && math_autocorrect(prevAtom(), c))
	//	return true;

	// no special circumstances, so insert the character without any fuss
	insert(bv, c);
	autocorrect_ = true;
	return true;
}


void MathCursor::setSelection
	(BufferView & bv, CursorBase const & where, size_t n)
{
	selection_ = true;
	bv.fullCursor().cursor_ = where;
	bv.fullCursor().anchor_ = where;
	bv.cursor().pos_ += n;
}


void MathCursor::insetToggle(BufferView & bv)
{
	if (hasNextAtom(bv)) {
		// toggle previous inset ...
		nextAtom(bv).nucleus()->lock(!nextAtom(bv)->lock());
	} else if (popLeft(bv) && hasNextAtom(bv)) {
		// ... or enclosing inset if we are in the last inset position
		nextAtom(bv).nucleus()->lock(!nextAtom(bv)->lock());
		posRight(bv);
	}
}


string MathCursor::info(BufferView & bv) const
{
	ostringstream os;
	os << "Math editor mode.  ";
	for (int i = 0, n = depth(bv); i < n; ++i) {
		bv.fullCursor().cursor_[i].asMathInset()->infoize(os);
		os << "  ";
	}
	if (hasPrevAtom(bv))
		prevAtom(bv)->infoize2(os);
	os << "                    ";
	return os.str();
}


unsigned MathCursor::depth(BufferView & bv) const
{
	return bv.fullCursor().cursor_.size();
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


string MathCursor::grabSelection(BufferView & bv) const
{
	if (!selection_)
		return string();

	CursorSlice i1;
	CursorSlice i2;
	getSelection(bv, i1, i2);

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


void MathCursor::eraseSelection(BufferView & bv)
{
	CursorSlice i1;
	CursorSlice i2;
	getSelection(bv, i1, i2);
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
	bv.cursor() = i1;
}


string MathCursor::grabAndEraseSelection(BufferView & bv)
{
	if (!selection_)
		return string();
	string res = grabSelection(bv);
	eraseSelection(bv);
	selection_ = false;
	return res;
}


CursorSlice MathCursor::normalAnchor(BufferView & bv) const
{
#warning FIXME
#if 0
	if (Anchor_.size() < depth()) {
		bv.resetAnchor();
		lyxerr << "unusual Anchor size" << endl;
	}
	//lyx::BOOST_ASSERT(Anchor_.size() >= cursor.depth());
	// use Anchor on the same level as Cursor
	CursorSlice normal = Anchor_[depth() - 1];
	if (depth() < Anchor_.size() && !(normal < cursor())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
	return normal;
#else
	return bv.cursor();
#endif
}


DispatchResult MathCursor::dispatch(BufferView &, FuncRequest const & cmd)
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
					prevAtom().nucleus()->dispatch(bv, cmd);
				if (res.dispatched())
					return res;
			}
			if (x > cmd.x && hasNextAtom()) {
				DispatchResult const res =
					nextAtom().nucleus()->dispatch(bv, cmd);
				if (res.dispatched())
					return res;
			}
*/
		}
		default:
			break;
	}

/*
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		CursorBase tmp = bv->Cursor_;
		CursorSlice & pos = tmp.back()
		DispatchResult const res = pos.asMathInset()->dispatch(bv, cmd);
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


MathInset::mode_type MathCursor::currentMode(BufferView &) const
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


void MathCursor::handleFont(BufferView & bv, string const & font)
{
	CursorSlice cur = cursorTip(bv);
	string safe;
	if (selection()) {
		macroModeClose(bv);
		safe = grabAndEraseSelection(bv);
	}

	if (cur.lastpos() != 0) {
		// something left in the cell
		if (cur.pos() == 0) {
			// cursor in first position
			popLeft(bv);
		} else if (cur.pos() == cur.lastpos()) {
			// cursor in last position
			popRight(bv);
		} else {
			// cursor in between. split cell
			MathArray::iterator bt = cur.cell().begin();
			MathAtom at = createMathInset(font);
			at.nucleus()->cell(0) = MathArray(bt, bt + cur.pos());
			cur.cell().erase(bt, bt + cur.pos());
			popLeft(bv);
			plainInsert(bv, at);
		}
	} else {
		// nothing left in the cell
		pullArg(bv);
		plainErase(bv);
	}
	insert(bv, safe);
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
