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

#include <lyxrc.h>

#include "support/lstrings.h"
#include "support/LAssert.h"
#include "support/limited_stack.h"
#include "debug.h"
#include "Lsstream.h"
#include "frontends/Painter.h"
#include "math_cursor.h"
#include "formulabase.h"
#include "funcrequest.h"
#include "math_autocorrect.h"
#include "math_braceinset.h"
#include "math_commentinset.h"
#include "math_charinset.h"
#include "math_extern.h"
#include "math_factory.h"
#include "math_fontinset.h"
#include "math_gridinset.h"
#include "math_iterator.h"
#include "math_macroarg.h"
#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "math_scriptinset.h"
#include "math_spaceinset.h"
#include "math_support.h"
#include "math_unknowninset.h"

#include <algorithm>
#include <cctype>

//#define FILEDEBUG 1

using namespace lyx::support;

using std::endl;
using std::min;
using std::max;
using std::swap;
using std::vector;
using std::ostringstream;
using std::isalpha;


// matheds own cut buffer
limited_stack<string> theCutBuffer;


MathCursor::MathCursor(InsetFormulaBase * formula, bool front)
	:	formula_(formula), autocorrect_(false), selection_(false), targetx_(-1)
{
	front ? first() : last();
	Anchor_ = Cursor_;
}


MathCursor::~MathCursor()
{
	// ensure that 'notifyCursorLeave' is called
	while (popLeft())
		;
}


void MathCursor::push(MathAtom & t)
{
	Cursor_.push_back(CursorPos(t.nucleus()));
}


void MathCursor::pushLeft(MathAtom & t)
{
	//lyxerr << "Entering atom " << t << " left" << endl;
	push(t);
	t->idxFirst(idx(), pos());
}


void MathCursor::pushRight(MathAtom & t)
{
	//lyxerr << "Entering atom " << t << " right" << endl;
	posLeft();
	push(t);
	t->idxLast(idx(), pos());
}


bool MathCursor::popLeft()
{
	//lyxerr << "Leaving atom to the left" << endl;
	if (depth() <= 1) {
		if (depth() == 1)
			inset()->notifyCursorLeaves(idx());
		return false;
	}
	inset()->notifyCursorLeaves(idx());
	Cursor_.pop_back();
	return true;
}


bool MathCursor::popRight()
{
	//lyxerr << "Leaving atom "; inset()->write(cerr, false); cerr << " right" << endl;
	if (depth() <= 1) {
		if (depth() == 1)
			inset()->notifyCursorLeaves(idx());
		return false;
	}
	inset()->notifyCursorLeaves(idx());
	Cursor_.pop_back();
	posRight();
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


bool MathCursor::isInside(MathInset const * p) const
{
	for (unsigned i = 0; i < depth(); ++i)
		if (Cursor_[i].inset_ == p)
			return true;
	return false;
}


bool MathCursor::openable(MathAtom const & t, bool sel) const
{
	if (!t->isActive())
		return false;

	if (t->lock())
		return false;

	if (sel) {
		// we can't move into anything new during selection
		if (depth() == Anchor_.size())
			return false;
		if (t.operator->() != Anchor_[depth()].inset_)
			return false;
	}
	return true;
}


bool MathCursor::inNucleus() const
{
	return inset()->asScriptInset() && idx() == 2;
}


bool MathCursor::posLeft()
{
	if (pos() == 0)
		return false;
	--pos();
	return true;
}


bool MathCursor::posRight()
{
	if (pos() == size())
		return false;
	++pos();
	return true;
}


bool MathCursor::left(bool sel)
{
	dump("Left 1");
	autocorrect_ = false;
	targetx_ = -1; // "no target"
	if (inMacroMode()) {
		macroModeClose();
		return true;
	}
	selHandle(sel);

	if (hasPrevAtom() && openable(prevAtom(), sel)) {
		pushRight(prevAtom());
		return true;
	}

	return posLeft() || idxLeft() || popLeft() || selection_;
}


bool MathCursor::right(bool sel)
{
	dump("Right 1");
	autocorrect_ = false;
	targetx_ = -1; // "no target"
	if (inMacroMode()) {
		macroModeClose();
		return true;
	}
	selHandle(sel);

	if (hasNextAtom() && openable(nextAtom(), sel)) {
		pushLeft(nextAtom());
		return true;
	}

	return posRight() || idxRight() || popRight() || selection_;
}


void MathCursor::first()
{
	Cursor_.clear();
	push(formula_->par());
	inset()->idxFirst(idx(), pos());
}


void MathCursor::last()
{
	Cursor_.clear();
	push(formula_->par());
	inset()->idxLast(idx(), pos());
}


bool positionable
	(MathIterator const & cursor, MathIterator const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.size() > anchor.size())
		return false;

	// anchor might be deeper, should have same path then
	for (MathIterator::size_type i = 0; i < cursor.size(); ++i)
		if (cursor[i].inset_ != anchor[i].inset_)
			return false;

	// position should be ok.
	return true;
}


void MathCursor::setPos(int x, int y)
{
	dump("setPos 1");
	bool res = bruteFind(x, y,
		formula()->xlow(), formula()->xhigh(),
		formula()->ylow(), formula()->yhigh());
	if (!res) {
		// this can happen on creation of "math-display"
		dump("setPos 1.5");
		first();
	}
	targetx_ = -1; // "no target"
	dump("setPos 2");
}



bool MathCursor::home(bool sel)
{
	dump("home 1");
	autocorrect_ = false;
	selHandle(sel);
	macroModeClose();
	if (!inset()->idxHome(idx(), pos()))
		return popLeft();
	dump("home 2");
	targetx_ = -1; // "no target"
	return true;
}


bool MathCursor::end(bool sel)
{
	dump("end 1");
	autocorrect_ = false;
	selHandle(sel);
	macroModeClose();
	if (!inset()->idxEnd(idx(), pos()))
		return popRight();
	dump("end 2");
	targetx_ = -1; // "no target"
	return true;
}


void MathCursor::plainErase()
{
	array().erase(pos());
}


void MathCursor::markInsert()
{
	//lyxerr << "inserting mark" << endl;
	array().insert(pos(), MathAtom(new MathCharInset(0)));
}


void MathCursor::markErase()
{
	//lyxerr << "deleting mark" << endl;
	array().erase(pos());
}


void MathCursor::plainInsert(MathAtom const & t)
{
	dump("plainInsert");
	array().insert(pos(), t);
	++pos();
}


void MathCursor::insert2(string const & str)
{
	MathArray ar;
	asArray(str, ar);
	insert(ar);
}


void MathCursor::insert(string const & str)
{
	//lyxerr << "inserting '" << str << "'" << endl;
	selClearOrDel();
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		plainInsert(MathAtom(new MathCharInset(*it)));
}


void MathCursor::insert(char c)
{
	//lyxerr << "inserting '" << c << "'" << endl;
	selClearOrDel();
	plainInsert(MathAtom(new MathCharInset(c)));
}


void MathCursor::insert(MathAtom const & t)
{
	macroModeClose();
	selClearOrDel();
	plainInsert(t);
}


void MathCursor::niceInsert(string const & t)
{
	MathArray ar;
	asArray(t, ar);
	if (ar.size() == 1)
		niceInsert(ar[0]);
	else
		insert(ar);
}


void MathCursor::niceInsert(MathAtom const & t)
{
	macroModeClose();
	string safe = grabAndEraseSelection();
	plainInsert(t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posLeft();
		pushLeft(nextAtom());
		paste(safe);
	}
}


void MathCursor::insert(MathArray const & ar)
{
	macroModeClose();
	if (selection_)
		eraseSelection();
	array().insert(pos(), ar);
	pos() += ar.size();
}


void MathCursor::paste(string const & data)
{
	dispatch(FuncRequest(LFUN_PASTE, data));
}


bool MathCursor::backspace()
{
	autocorrect_ = false;

	if (selection_) {
		selDel();
		return true;
	}

	if (pos() == 0) {
		if (inset()->ncols() == 1 &&
			  inset()->nrows() == 1 &&
			  depth() == 1 &&
			  size() == 0)
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

	--pos();
	plainErase();
	return true;
}


bool MathCursor::erase()
{
	autocorrect_ = false;
	if (inMacroMode())
		return true;

	if (selection_) {
		selDel();
		return true;
	}

	// delete empty cells if possible
	if (array().empty())
		if (inset()->idxDelete(idx()))
			return true;

	// old behaviour when in last position of cell
	if (pos() == size()) {
		if (inset()->ncols() == 1 && inset()->nrows() == 1 && depth() == 1 && size() == 0)
			return false;
		else{
			inset()->idxGlue(idx());
			return true;
		}
	}

	plainErase();
	return true;
}


bool MathCursor::up(bool sel)
{
	dump("up 1");
	macroModeClose();
	selHandle(sel);
	MathIterator save = Cursor_;
	if (goUpDown(true))
		return true;
	Cursor_ = save;
	autocorrect_ = false;
	return selection_;
}


bool MathCursor::down(bool sel)
{
	dump("down 1");
	macroModeClose();
	selHandle(sel);
	MathIterator save = Cursor_;
	if (goUpDown(false))
		return true;
	Cursor_ = save;
	autocorrect_ = false;
	return selection_;
}


void MathCursor::macroModeClose()
{
	if (!inMacroMode())
		return;
	MathUnknownInset * p = activeMacro();
	p->finalize();
	string s = p->name();
	--pos();
	array().erase(pos());

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


string MathCursor::macroName() const
{
	return inMacroMode() ? activeMacro()->name() : string();
}


void MathCursor::selClear()
{
	Anchor_.clear();
	selection_ = false;
}


void MathCursor::selCopy()
{
	dump("selCopy");
	if (selection_) {
		theCutBuffer.push(grabSelection());
		selection_ = false;
	} else {
		//theCutBuffer.erase();
	}
}


void MathCursor::selCut()
{
	dump("selCut");
	theCutBuffer.push(grabAndEraseSelection());
}


void MathCursor::selDel()
{
	dump("selDel");
	if (selection_) {
		eraseSelection();
		selection_ = false;
	}
}


void MathCursor::selPaste(size_t n)
{
	dump("selPaste");
	selClearOrDel();
	if (n < theCutBuffer.size())
		paste(theCutBuffer[n]);
	//grabSelection();
	selection_ = false;
}


void MathCursor::selHandle(bool sel)
{
	if (sel == selection_)
		return;
	//clear();
	Anchor_ = Cursor_;
	selection_ = sel;
}


void MathCursor::selStart()
{
	dump("selStart 1");
	//clear();
	Anchor_ = Cursor_;
	selection_ = true;
	dump("selStart 2");
}


void MathCursor::selClearOrDel()
{
	if (lyxrc.auto_region_delete)
		selDel();
	else
		selection_ = false;
}


void MathCursor::drawSelection(PainterInfo & pi) const
{
	if (!selection_)
		return;
	CursorPos i1;
	CursorPos i2;
	getSelection(i1, i2);
	i1.inset_->drawSelection(pi, i1.idx_, i1.pos_, i2.idx_, i2.pos_);
}


void MathCursor::handleNest(MathAtom const & a)
{
	MathAtom at = a;
	asArray(grabAndEraseSelection(), at.nucleus()->cell(0));
	insert(at);
	pushRight(prevAtom());
}


void MathCursor::getPos(int & x, int & y) const
{
	inset()->getPos(idx(), pos(), x, y);
}


int MathCursor::targetX() const
{
	if (targetx_ != -1)
		return targetx_;
	int x = 0, y = 0;
	getPos(x, y);
	return x;
}


MathInset * MathCursor::inset() const
{
	return cursor().inset_;
}


InsetFormulaBase * MathCursor::formula() const
{
	return formula_;
}


MathCursor::idx_type MathCursor::idx() const
{
	return cursor().idx_;
}


MathCursor::idx_type & MathCursor::idx()
{
	return cursor().idx_;
}


MathCursor::pos_type MathCursor::pos() const
{
	return cursor().pos_;
}


void MathCursor::adjust(pos_type from, difference_type diff)
{
	if (cursor().pos_ > from)
		cursor().pos_ += diff;
	if (Anchor_.back().pos_ > from)
		Anchor_.back().pos_ += diff;
	// just to be on the safe side
	// theoretically unecessary
	normalize();
}


MathCursor::pos_type & MathCursor::pos()
{
	return cursor().pos_;
}


bool MathCursor::inMacroMode() const
{
	if (!hasPrevAtom())
		return false;
	MathUnknownInset const * p = prevAtom()->asUnknownInset();
	return p && !p->final();
}


MathUnknownInset * MathCursor::activeMacro()
{
	return inMacroMode() ? prevAtom().nucleus()->asUnknownInset() : 0;
}


MathUnknownInset const * MathCursor::activeMacro() const
{
	return inMacroMode() ? prevAtom()->asUnknownInset() : 0;
}


bool MathCursor::inMacroArgMode() const
{
	return pos() > 0 && prevAtom()->getChar() == '#';
}


bool MathCursor::selection() const
{
	return selection_;
}


MathGridInset * MathCursor::enclosingGrid(MathCursor::idx_type & idx) const
{
	for (MathInset::difference_type i = depth() - 1; i >= 0; --i) {
		MathGridInset * p = Cursor_[i].inset_->asGridInset();
		if (p) {
			idx = Cursor_[i].idx_;
			return p;
		}
	}
	return 0;
}


void MathCursor::popToHere(MathInset const * p)
{
	while (depth() && Cursor_.back().inset_ != p)
		Cursor_.pop_back();
}


void MathCursor::popToEnclosingGrid()
{
	while (depth() && !Cursor_.back().inset_->asGridInset())
		Cursor_.pop_back();
}


void MathCursor::popToEnclosingHull()
{
	while (depth() && !Cursor_.back().inset_->asHullInset())
		Cursor_.pop_back();
}


void MathCursor::pullArg()
{
	dump("pullarg");
	MathArray a = array();
	if (popLeft()) {
		plainErase();
		array().insert(pos(), a);
		Anchor_ = Cursor_;
	} else {
		formula()->mutateToText();
	}
}


void MathCursor::touch()
{
	MathIterator::const_iterator it = Cursor_.begin();
	MathIterator::const_iterator et = Cursor_.end();
	for ( ; it != et; ++it)
		it->cell().touch();
}


void MathCursor::normalize()
{
	if (idx() >= inset()->nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << ' ' << inset()->nargs()
		       << " in: " << inset() << endl;
		dump("error 2");
	}
	idx() = min(idx(), inset()->nargs() - 1);

	if (pos() > size()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << ' ' << size() <<  " in idx: " << idx()
		       << " in atom: '";
		WriteStream wi(lyxerr, false, true);
		inset()->write(wi);
		lyxerr << endl;
		dump("error 4");
	}
	pos() = min(pos(), size());
}


MathCursor::size_type MathCursor::size() const
{
	return array().size();
}


bool MathCursor::hasPrevAtom() const
{
	return pos() > 0;
}


bool MathCursor::hasNextAtom() const
{
	return pos() < size();
}


MathAtom const & MathCursor::prevAtom() const
{
	Assert(pos() > 0);
	return array()[pos() - 1];
}


MathAtom & MathCursor::prevAtom()
{
	Assert(pos() > 0);
	return array()[pos() - 1];
}


MathAtom const & MathCursor::nextAtom() const
{
	Assert(pos() < size());
	return array()[pos()];
}


MathAtom & MathCursor::nextAtom()
{
	Assert(pos() < size());
	return array()[pos()];
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;

	if (idx() >= inset()->nargs()) {
		lyxerr << "############  idx_ " << idx() << " not valid" << endl;
		return dummy;
	}

	if (depth() == 0) {
		lyxerr << "############  depth() == 0 not valid" << endl;
		return dummy;
	}

	return cursor().cell();
}


void MathCursor::idxNext()
{
	inset()->idxNext(idx(), pos());
}


void MathCursor::idxPrev()
{
	inset()->idxPrev(idx(), pos());
}


char MathCursor::valign() const
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(idx);
	return p ? p->valign() : '\0';
}


char MathCursor::halign() const
{
	idx_type idx;
	MathGridInset * p = enclosingGrid(idx);
	return p ? p->halign(idx % p->ncols()) : '\0';
}


void MathCursor::getSelection(CursorPos & i1, CursorPos & i2) const
{
	CursorPos anc = normalAnchor();
	if (anc < cursor()) {
		i1 = anc;
		i2 = cursor();
	} else {
		i1 = cursor();
		i2 = anc;
	}
}


CursorPos & MathCursor::cursor()
{
	Assert(depth());
	return Cursor_.back();
}


CursorPos const & MathCursor::cursor() const
{
	Assert(depth());
	return Cursor_.back();
}


bool MathCursor::goUpDown(bool up)
{
	// Be warned: The 'logic' implemented in this function is highly fragile.
	// A distance of one pixel or a '<' vs '<=' _really_ matters.
	// So fiddle around with it only if you know what you are doing!
  int xo = 0;
	int yo = 0;
	getPos(xo, yo);

	// check if we had something else in mind, if not, this is the future goal
	if (targetx_ == -1)
		targetx_ = xo;
	else
		xo = targetx_;

	// try neigbouring script insets
	// try left
	if (hasPrevAtom()) {
		MathScriptInset const * p = prevAtom()->asScriptInset();
		if (p && p->has(up)) {
			--pos();
			push(nextAtom());
			idx() = up; // the superscript has index 1
			pos() = size();
			///lyxerr << "updown: handled by scriptinset to the left" << endl;
			return true;
		}
	}

	// try right
	if (hasNextAtom()) {
		MathScriptInset const * p = nextAtom()->asScriptInset();
		if (p && p->has(up)) {
			push(nextAtom());
			idx() = up;
			pos() = 0;
			///lyxerr << "updown: handled by scriptinset to the right" << endl;
			return true;
		}
	}

	// try current cell for e.g. text insets
	if (inset()->idxUpDown2(idx(), pos(), up, targetx_))
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
		///lyxerr << "updown: We are in " << *inset() << " idx: " << idx() << endl;
		// ask inset first
		if (inset()->idxUpDown(idx(), pos(), up, targetx_)) {
			// try to find best position within this inset
			if (!selection())
				bruteFind2(xo, yo);
			return true;
		}

		// no such inset found, just take something "above"
		///lyxerr << "updown: handled by strange case" << endl;
		if (!popLeft())
			return
				bruteFind(xo, yo,
					formula()->xlow(),
					formula()->xhigh(),
					up ? formula()->ylow() : yo + 4,
					up ? yo - 4 : formula()->yhigh()
				);

		// any improvement so far?
		int xnew, ynew;
		getPos(xnew, ynew);
		if (up ? ynew < yo : ynew > yo)
			return true;
	}
}


bool MathCursor::bruteFind
	(int x, int y, int xlow, int xhigh, int ylow, int yhigh)
{
	MathIterator best_cursor;
	double best_dist = 1e10;

	MathIterator it = ibegin(formula()->par().nucleus());
	MathIterator et = iend(formula()->par().nucleus());
	while (1) {
		// avoid invalid nesting when selecting
		if (!selection_ || positionable(it, Anchor_)) {
			int xo, yo;
			it.back().getPos(xo, yo);
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
		++it;
	}

	if (best_dist < 1e10)
		Cursor_ = best_cursor;
	return best_dist < 1e10;
}


void MathCursor::bruteFind2(int x, int y)
{
	double best_dist = 1e10;

	MathIterator it = Cursor_;
	it.back().setPos(0);
	MathIterator et = Cursor_;
	et.back().setPos(it.cell().size());
	for (int i = 0; ; ++i) {
		int xo, yo;
		it.back().getPos(xo, yo);
		double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
		// '<=' in order to take the last possible position
		// this is important for clicking behind \sum in e.g. '\sum_i a'
		lyxerr << "i: " << i << " d: " << d << " best: " << best_dist << endl;
		if (d <= best_dist) {
			best_dist = d;
			Cursor_   = it;
		}
		if (it == et)
			break;
		++it;
	}
}


bool MathCursor::idxLineLast()
{
	idx() -= idx() % inset()->ncols();
	idx() += inset()->ncols() - 1;
	pos() = size();
	return true;
}

bool MathCursor::idxLeft()
{
	return inset()->idxLeft(idx(), pos());
}


bool MathCursor::idxRight()
{
	return inset()->idxRight(idx(), pos());
}


bool MathCursor::script(bool up)
{
	// Hack to get \\^ and \\_ working
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
		inset()->asScriptInset()->ensure(up);
		idx() = up;
		pos() = 0;
	} else if (hasPrevAtom() && prevAtom()->asScriptInset()) {
		prevAtom().nucleus()->asScriptInset()->ensure(up);
		pushRight(prevAtom());
		idx() = up;
		pos() = size();
	} else if (hasPrevAtom()) {
		--pos();
		array()[pos()] = MathAtom(new MathScriptInset(nextAtom(), up));
		pushLeft(nextAtom());
		idx() = up;
		pos() = 0;
	} else {
		plainInsert(MathAtom(new MathScriptInset(up)));
		prevAtom().nucleus()->asScriptInset()->ensure(up);
		pushRight(prevAtom());
		idx() = up;
		pos() = 0;
	}
	paste(safe);
	dump("1");
	return true;
}


bool MathCursor::interpret(char c)
{
	//lyxerr << "interpret 2: '" << c << "'" << endl;
	targetx_ = -1; // "no target"
	if (inMacroArgMode()) {
		--pos();
		plainErase();
		int n = c - '0';
		MathMacroTemplate const * p = formula()->par()->asMacroTemplate();
		if (p && 1 <= n && n <= p->numargs())
			insert(MathAtom(new MathMacroArgument(c - '0')));
		else {
			insert(createMathInset("#"));
			interpret(c); // try again
		}
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
			if (!hasPrevAtom() || prevAtom()->getChar() != ' ')
				insert(c);
			return true;
		}
		if (hasPrevAtom() && prevAtom()->asSpaceInset()) {
			prevAtom().nucleus()->asSpaceInset()->incSpace();
			return true;
		}
		if (popRight())
			return true;
		// if are at the very end, leave the formula
		return pos() != size();
	}

	if (c == '{' || c == '}' || c == '#' || c == '&' || c == '$') {
		createMathInset(string(1, c));
		return true;
	}

	if (c == '%') {
		niceInsert(MathAtom(new MathCommentInset));
		return true;
	}

	// try auto-correction
	//if (autocorrect_ && hasPrevAtom() && math_autocorrect(prevAtom(), c))
	//	return true;

	// no special circumstances, so insert the character without any fuss
	insert(c);
	autocorrect_ = true;
	return true;
}


void MathCursor::setSelection(MathIterator const & where, size_type n)
{
	selection_ = true;
	Anchor_ = where;
	Cursor_ = where;
	cursor().pos_ += n;
}


void MathCursor::insetToggle()
{
	if (hasNextAtom()) {
		// toggle previous inset ...
		nextAtom().nucleus()->lock(!nextAtom()->lock());
	} else if (popLeft() && hasNextAtom()) {
		// ... or enclosing inset if we are in the last inset position
		nextAtom().nucleus()->lock(!nextAtom()->lock());
		posRight();
	}
}


string MathCursor::info() const
{
	ostringstream os;
	os << "Math editor mode.  ";
	for (int i = 0, n = depth(); i < n; ++i) {
		Cursor_[i].inset_->infoize(os);
		os << "  ";
	}
	if (hasPrevAtom())
		prevAtom()->infoize2(os);
	os << "                    ";
	return STRCONV(os.str());
}


unsigned MathCursor::depth() const
{
	return Cursor_.size();
}




namespace {

void region(CursorPos const & i1, CursorPos const & i2,
	MathInset::row_type & r1, MathInset::row_type & r2,
	MathInset::col_type & c1, MathInset::col_type & c2)
{
	MathInset * p = i1.inset_;
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


string MathCursor::grabSelection() const
{
	if (!selection_)
		return string();

	CursorPos i1;
	CursorPos i2;
	getSelection(i1, i2);

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
			data += asString(i1.inset_->cell(i1.inset_->index(row, col)));
		}
	}
	return data;
}


void MathCursor::eraseSelection()
{
	CursorPos i1;
	CursorPos i2;
	getSelection(i1, i2);
	if (i1.idx_ == i2.idx_)
		i1.cell().erase(i1.pos_, i2.pos_);
	else {
		MathInset * p = i1.inset_;
		row_type r1, r2;
		col_type c1, c2;
		region(i1, i2, r1, r2, c1, c2);
		for (row_type row = r1; row <= r2; ++row)
			for (col_type col = c1; col <= c2; ++col)
				p->cell(p->index(row, col)).clear();
	}
	cursor() = i1;
}


string MathCursor::grabAndEraseSelection()
{
	if (!selection_)
		return string();
	string res = grabSelection();
	eraseSelection();
	selection_ = false;
	return res;
}


CursorPos MathCursor::normalAnchor() const
{
	if (Anchor_.size() < depth()) {
		Anchor_ = Cursor_;
		lyxerr << "unusual Anchor size" << endl;
	}
	//lyx::Assert(Anchor_.size() >= cursor.depth());
	// use Anchor on the same level as Cursor
	CursorPos normal = Anchor_[depth() - 1];
	if (depth() < Anchor_.size() && !(normal < cursor())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
	return normal;
}


dispatch_result MathCursor::dispatch(FuncRequest const & cmd)
{
	// mouse clicks are somewhat special
	// check
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
		case LFUN_MOUSE_MOTION:
		case LFUN_MOUSE_RELEASE:
		case LFUN_MOUSE_DOUBLE: {
			CursorPos & pos = Cursor_.back();
			dispatch_result res = UNDISPATCHED;
			int x = 0, y = 0;
			getPos(x, y);
			if (x < cmd.x && hasPrevAtom()) {
				res = prevAtom().nucleus()->dispatch(cmd, pos.idx_, pos.pos_);
				if (res != UNDISPATCHED)
					return res;
			}
			if (x > cmd.x && hasNextAtom()) {
				res = nextAtom().nucleus()->dispatch(cmd, pos.idx_, pos.pos_);
				if (res != UNDISPATCHED)
					return res;
			}
		}
		default:
			break;
	}

	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		CursorPos & pos = Cursor_[i];
		dispatch_result res = pos.inset_->dispatch(cmd, pos.idx_, pos.pos_);
		if (res != UNDISPATCHED) {
			if (res == DISPATCHED_POP) {
				Cursor_.shrink(i + 1);
				selClear();
			}
			return res;
		}
	}
	return UNDISPATCHED;
}


MathInset::mode_type MathCursor::currentMode() const
{
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		MathInset::mode_type res = Cursor_[i].inset_->currentMode();
		if (res != MathInset::UNDECIDED_MODE)
			return res;
	}
	return MathInset::UNDECIDED_MODE;
}


void MathCursor::handleFont(string const & font)
{
	string safe;
	if (selection()) {
		macroModeClose();
		safe = grabAndEraseSelection();
	}

	if (array().size()) {
		// something left in the cell
		if (pos() == 0) {
			// cursor in first position
			popLeft();
		} else if (pos() == array().size()) {
			// cursor in last position
			popRight();
		} else {
			// cursor in between. split cell
			MathArray::iterator bt = array().begin();
			MathAtom at = createMathInset(font);
			at.nucleus()->cell(0) = MathArray(bt, bt + pos());
			cursor().cell().erase(bt, bt + pos());
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


void releaseMathCursor(BufferView * bv)
{
	if (mathcursor) {
		InsetFormulaBase * f =  mathcursor->formula();
		delete mathcursor;
		mathcursor = 0;
		f->insetUnlock(bv);
	}
}
