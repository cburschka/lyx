/*
 *  File:        math_cursor.C
 *  Purpose:     Interaction for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     January 1996
 *  Description: Math interaction for a WYSIWYG math editor.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Math & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>
#include <lyxrc.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
#include "support/LAssert.h"
#include "BufferView.h"
#include "debug.h"
#include "LColor.h"
#include "frontends/Painter.h"
#include "math_cursor.h"
#include "formulabase.h"
#include "math_autocorrect.h"
#include "math_arrayinset.h"
#include "math_braceinset.h"
#include "math_casesinset.h"
#include "math_charinset.h"
#include "math_extern.h"
#include "math_factory.h"
#include "math_hullinset.h"
#include "math_iterator.h"
#include "math_macroarg.h"
#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "math_replace.h"
#include "math_scriptinset.h"
#include "math_spaceinset.h"
#include "math_support.h"
#include "math_unknowninset.h"

#include <algorithm>
#include <cctype>

//#define FILEDEBUG 1

using std::endl;
using std::min;
using std::max;
using std::swap;
using std::vector;
using std::ostringstream;
using std::isalpha;


// matheds own cut buffer
MathGridInset theCutBuffer = MathGridInset(1, 1);


MathCursor::MathCursor(InsetFormulaBase * formula, bool front)
	:	formula_(formula), autocorrect_(false), selection_(false)
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
	Cursor_.push_back(MathCursorPos(t.nucleus()));
}


void MathCursor::pushLeft(MathAtom & t)
{
	//cerr << "Entering atom "; t->write(cerr, false); cerr << " left\n";
	push(t);
	t->idxFirst(idx(), pos());
}


void MathCursor::pushRight(MathAtom & t)
{
	//cerr << "Entering atom "; t->write(cerr, false); cerr << " right\n";
	posLeft();
	push(t);
	t->idxLast(idx(), pos());
}


bool MathCursor::popLeft()
{
	//cerr << "Leaving atom to the left\n";
	if (depth() <= 1) {
		if (depth() == 1)
			par()->notifyCursorLeaves();
		return false;
	}
	par()->notifyCursorLeaves();
	Cursor_.pop_back();
	return true;
}


bool MathCursor::popRight()
{
	//cerr << "Leaving atom "; par()->write(cerr, false); cerr << " right\n";
	if (depth() <= 1) {
		if (depth() == 1)
			par()->notifyCursorLeaves();
		return false;
	}
	par()->notifyCursorLeaves();
	Cursor_.pop_back();
	posRight();
	return true;
}



#if FILEDEBUG
	void MathCursor::dump(char const * what) const
	{
		lyxerr << "MC: " << what << "\n";
		lyxerr << " Cursor: " << depth() << "\n";
		for (unsigned i = 0; i < depth(); ++i)
			lyxerr << "    i: " << i << " " << Cursor_[i] << "\n";
		lyxerr << " Anchor: " << Anchor_.size() << "\n";
		for (unsigned i = 0; i < Anchor_.size(); ++i)
			lyxerr << "    i: " << i << " " << Anchor_[i] << "\n";
		lyxerr	<< " sel: " << selection_ << "\n";
	}
#else
	void MathCursor::dump(char const *) const {}
#endif


bool MathCursor::isInside(MathInset const * p) const
{
	for (unsigned i = 0; i < depth(); ++i)
		if (Cursor_[i].par_ == p)
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
		if (t.nucleus() != Anchor_[depth()].par_)
			return false;
	}
	return true;
}


bool MathCursor::inNucleus() const
{
	return par()->asScriptInset() && idx() == 2;
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
	par()->idxFirst(idx(), pos());
}


void MathCursor::last()
{
	Cursor_.clear();
	push(formula_->par());
	par()->idxLast(idx(), pos());
}


bool positionable(MathIterator const & cursor, MathIterator const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.size() > anchor.size())
		return false;

	// anchor might be deeper, should have same path then
	for (MathIterator::size_type i = 0; i < cursor.size(); ++i)
		if (cursor[i].par_ != anchor[i].par_)
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
	if (!par()->idxHome(idx(), pos()))
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
	if (!par()->idxEnd(idx(), pos()))
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
	//lyxerr << "inserting mark\n";
	array().insert(pos(), MathAtom(new MathCharInset(0)));
}


void MathCursor::markErase()
{
	//lyxerr << "deleting mark\n";
	array().erase(pos());
}


void MathCursor::plainInsert(MathAtom const & t)
{
	dump("plainInsert");
	array().insert(pos(), t);
	++pos();
}


void MathCursor::insert(string const & str)
{
	//lyxerr << "inserting '" << str << "'\n";
	selClearOrDel();
	for (string::const_iterator it = str.begin(); it != str.end(); ++it)
		plainInsert(MathAtom(new MathCharInset(*it)));
}


void MathCursor::insert(char c)
{
	//lyxerr << "inserting '" << c << "'\n";
	selClearOrDel();
	plainInsert(MathAtom(new MathCharInset(c)));
}


void MathCursor::insert(MathAtom const & t)
{
	macroModeClose();
	selClearOrDel();
	plainInsert(t);
}


void MathCursor::niceInsert(MathAtom const & t)
{
	macroModeClose();
	MathGridInset safe = grabAndEraseSelection();
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


void MathCursor::paste(MathArray const & ar)
{
	Anchor_ = Cursor_;
	selection_ = true;
	array().insert(pos(), ar);
	pos() += ar.size();
}


void MathCursor::paste(MathGridInset const & data)
{
	if (data.nargs() == 1) {
		// single cell/part of cell
		paste(data.cell(0));
	} else {
		// multiple cells
		idx_type idx; // index of upper left cell
		MathGridInset * p = enclosingGrid(idx);
		col_type const numcols = min(data.ncols(), p->ncols() - p->col(idx));
		row_type const numrows = min(data.nrows(), p->nrows() - p->row(idx));
		for (row_type row = 0; row < numrows; ++row) {
			for (col_type col = 0; col < numcols; ++col) {
				idx_type i = p->index(row + p->row(idx), col + p->col(idx));
				p->cell(i).append(data.cell(data.index(row, col)));
			}
			// append the left over horizontal cells to the last column
			idx_type i = p->index(row + p->row(idx), p->ncols() - 1);
			for (MathInset::col_type col = numcols; col < data.ncols(); ++col)
				p->cell(i).append(data.cell(data.index(row, col)));
		}
		// append the left over vertical cells to the last _cell_
		idx_type i = p->nargs() - 1;
		for (row_type row = numrows; row < data.nrows(); ++row)
			for (col_type col = 0; col < data.ncols(); ++col)
				p->cell(i).append(data.cell(data.index(row, col)));
	}
}


void MathCursor::backspace()
{
	autocorrect_ = false;
	if (pos() == 0) {
		pullArg();
		return;
	}

	if (selection_) {
		selDel();
		return;
	}

/*
	if (prevAtom()->asScriptInset()) {
		// simply enter nucleus
		left();
		return;
	}

	if (inNucleus()) {
		// we are in nucleus
		if (pos() == 1) {
		}
	}
*/

	--pos();
	plainErase();
}


void MathCursor::erase()
{
	autocorrect_ = false;
	if (inMacroMode())
		return;

	if (selection_) {
		selDel();
		return;
	}

	// delete empty cells if possible
	if (array().empty())
		if (par()->idxDelete(idx()))
			return;

	// old behaviour when in last position of cell
	if (pos() == size()) {
		par()->idxGlue(idx());
		return;
	}

/*
	// if we are standing in front of a script inset, grab item before us and
	// move it into nucleus
	// and remove first thing.
	if (hasNextAtom() && nextAtom()->asScriptInset()) {
		if (hasPrevAtom()) {
			MathAtom at = prevAtom();
			--pos();
			array().erase(pos());
			pushLeft(nextAtom());
			if (array().empty())
				array().push_back(at);
			else
				array()[0] = at;
			pos() = 1;
		} else {
			pushLeft(nextAtom());
			array().clear();
		}
		return;
	}
*/

	plainErase();
}


void MathCursor::delLine()
{
	autocorrect_ = false;
	macroModeClose();

	if (selection_) {
		selDel();
		return;
	}

	if (par()->nrows() > 1) {
		// grid are the only things with more than one row...
		lyx::Assert(par()->asGridInset());
		par()->asGridInset()->delRow(hullRow());
	}

	if (idx() >= par()->nargs())
		idx() = par()->nargs() - 1;

	if (pos() > size())
		pos() = size();
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


bool MathCursor::toggleLimits()
{
	if (!hasNextAtom() || !nextAtom()->asScriptInset())
		return false;
	MathScriptInset * t = nextAtom().nucleus()->asScriptInset();
	int old = t->limits();
	t->limits(old < 0 ? 1 : -1);
	return old != t->limits();
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
	if (s != "\\")
		interpret(s);
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
		theCutBuffer = grabSelection();
		selection_ = false;
	} else {
		theCutBuffer = MathGridInset(1, 1);
	}
}


void MathCursor::selCut()
{
	dump("selCut");
	theCutBuffer = grabAndEraseSelection();
}


void MathCursor::selDel()
{
	dump("selDel");
	if (selection_) {
		eraseSelection();
		selection_ = false;
	}
}


void MathCursor::selPaste()
{
	dump("selPaste");
	selClearOrDel();
	paste(theCutBuffer);
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


void MathCursor::selGet(MathArray & ar)
{
	dump("selGet");
	if (selection_)
		ar = grabSelection().glue();
}


void MathCursor::drawSelection(MathPainterInfo & pi) const
{
	if (!selection_)
		return;
	MathCursorPos i1;
	MathCursorPos i2;
	getSelection(i1, i2);
	i1.par_->drawSelection(pi, i1.idx_, i1.pos_, i2.idx_, i2.pos_);
}


void MathCursor::handleNest(MathAtom const & at)
{
#ifdef WITH_WARNINGS
#warning temporarily disabled
	//at->cell(0) = grabAndEraseSelection().glue();
#endif
	insert(at);
	pushRight(prevAtom());
}


void MathCursor::getPos(int & x, int & y)
{
	par()->getPos(idx(), pos(), x, y);
}


MathInset * MathCursor::par() const
{
	return cursor().par_;
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
		MathGridInset * p = Cursor_[i].par_->asGridInset();
		if (p) {
			idx = Cursor_[i].idx_;
			return p;
		}
	}
	return 0;
}


MathHullInset * MathCursor::enclosingHull(MathCursor::idx_type & idx) const
{
	for (MathInset::difference_type i = depth() - 1; i >= 0; --i) {
		MathHullInset * p = Cursor_[i].par_->asHullInset();
		if (p) {
			idx = Cursor_[i].idx_;
			return p;
		}
	}
	return 0;
}


void MathCursor::popToEnclosingGrid()
{
	while (depth() && !Cursor_.back().par_->asGridInset())
		Cursor_.pop_back();
}


void MathCursor::popToEnclosingHull()
{
	while (depth() && !Cursor_.back().par_->asHullInset())
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
	if (idx() >= par()->nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << " " << par()->nargs() << " in: " << par() << "\n";
		dump("error 2");
	}
	idx() = min(idx(), par()->nargs() - 1);

	if (pos() > size()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << " " << size() <<  " in idx: " << idx()
			<< " in atom: '";
		WriteStream wi(lyxerr, false, true);
		par()->write(wi);
		lyxerr << "\n";
		dump("error 4");
	}
	pos() = min(pos(), size());

	// remove empty scripts if possible
	if (1) {
		for (pos_type i = 0; i < size(); ++i) {
			MathScriptInset * p = array()[i].nucleus()->asScriptInset();
			if (p) {
				p->removeEmptyScripts();
				//if (p->empty())
				//	array().erase(i);
			}
		}
	}

	// fix again position
	pos() = min(pos(), size());
}


MathCursor::size_type MathCursor::size() const
{
	return array().size();
}


MathCursor::col_type MathCursor::hullCol() const
{
	idx_type idx = 0;
	MathHullInset * p = enclosingHull(idx);
	return p->col(idx);
}


MathCursor::row_type MathCursor::hullRow() const
{
	idx_type idx = 0;
	MathHullInset * p = enclosingHull(idx);
	return p->row(idx);
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
	lyx::Assert(pos() > 0);
	return array()[pos() - 1];
}


MathAtom & MathCursor::prevAtom()
{
	lyx::Assert(pos() > 0);
	return array()[pos() - 1];
}


MathAtom const & MathCursor::nextAtom() const
{
	lyx::Assert(pos() < size());
	return array()[pos()];
}


MathAtom & MathCursor::nextAtom()
{
	lyx::Assert(pos() < size());
	return array()[pos()];
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;

	if (idx() >= par()->nargs()) {
		lyxerr << "############  idx_ " << idx() << " not valid\n";
		return dummy;
	}

	if (depth() == 0) {
		lyxerr << "############  depth() == 0 not valid\n";
		return dummy;
	}

	return cursor().cell();
}


void MathCursor::idxNext()
{
	par()->idxNext(idx(), pos());
}


void MathCursor::idxPrev()
{
	par()->idxPrev(idx(), pos());
}


void MathCursor::splitCell()
{
	if (idx() + 1 == par()->nargs())
		return;
	MathArray ar = array();
	ar.erase(0, pos());
	array().erase(pos(), size());
	++idx();
	pos() = 0;
	array().insert(0, ar);
}


void MathCursor::breakLine()
{
	// leave inner cells
	while (popRight())
		;

	idx_type dummy;
	MathHullInset * p = enclosingHull(dummy);
	if (!p)
		return;

	if (p->getType() == "simple" || p->getType() == "equation") {
		p->mutate("eqnarray");
		idx() = 1;
		pos() = 0;
	} else {
		p->addRow(hullRow());

		// split line
		const row_type r = hullRow();
		for (col_type c = hullCol() + 1; c < p->ncols(); ++c)
			std::swap(p->cell(p->index(r, c)), p->cell(p->index(r + 1, c)));

		// split cell
		splitCell();
		std::swap(p->cell(idx()), p->cell(idx() + p->ncols() - 1));
	}
}


//void MathCursor::readLine(MathArray & ar) const
//{
//	idx_type base = row() * par()->ncols();
//	for (idx_type off = 0; off < par()->ncols(); ++off)
//		ar.push_back(par()->cell(base + off));
//}


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


void MathCursor::getSelection(MathCursorPos & i1, MathCursorPos & i2) const
{
	MathCursorPos anc = normalAnchor();
	if (anc < cursor()) {
		i1 = anc;
		i2 = cursor();
	} else {
		i1 = cursor();
		i2 = anc;
	}
}


MathCursorPos & MathCursor::cursor()
{
	lyx::Assert(depth());
	return Cursor_.back();
}


MathCursorPos const & MathCursor::cursor() const
{
	lyx::Assert(depth());
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
			///lyxerr << "updown: handled by scriptinset to the left\n";
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
			///lyxerr << "updown: handled by scriptinset to the right\n";
			return true;
		}
	}

	// try current cell
	//xarray().boundingBox(xlow, xhigh, ylow, yhigh);
	//if (up)
	//	yhigh = yo - 4;
	//else
	//	ylow = yo + 4;
	//if (bruteFind(xo, yo, xlow, xhigh, ylow, yhigh)) {
	//	lyxerr << "updown: handled by brute find in the same cell\n";
	//	return true;
	//}

	// try to find an inset that knows better then we
	while (1) {
		///lyxerr << "updown: We are in " << *par() << " idx: " << idx() << '\n';
		// ask inset first
		if (par()->idxUpDown(idx(), pos(), up, targetx_))
			return true;

		// no such inset found, just take something "above"
		///lyxerr << "updown: handled by strange case\n";
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


bool MathCursor::idxLineFirst()
{
	idx() -= idx() % par()->ncols();
	pos() = 0;
	return true;
}


bool MathCursor::idxLineLast()
{
	idx() -= idx() % par()->ncols();
	idx() += par()->ncols() - 1;
	pos() = size();
	return true;
}

bool MathCursor::idxLeft()
{
	return par()->idxLeft(idx(), pos());
}


bool MathCursor::idxRight()
{
	return par()->idxRight(idx(), pos());
}


bool MathCursor::interpret(string const & s)
{
	//lyxerr << "interpret 1: '" << s << "'\n";
	if (s.empty())
		return true;

	//lyxerr << "char: '" << s[0] << "'  int: " << int(s[0]) << endl;
	//owner_->getIntl()->getTransManager().TranslateAndInsert(s[0], lt);
	//lyxerr << "trans: '" << s[0] << "'  int: " << int(s[0]) << endl;

	if (s.size() >= 5 && s.substr(0, 5) == "cases") {
		unsigned int n = 1;
		istringstream is(s.substr(5).c_str());
		is >> n;
		n = max(1u, n);
		niceInsert(MathAtom(new MathCasesInset(n)));
		return true;
	}

	if (s.size() >= 6 && s.substr(0, 6) == "matrix") {
		unsigned int m = 1;
		unsigned int n = 1;
		string v_align;
		string h_align;
		istringstream is(s.substr(6).c_str());
		is >> m >> n >> v_align >> h_align;
		m = max(1u, m);
		n = max(1u, n);
		v_align += 'c';
		niceInsert(MathAtom(new MathArrayInset("array", m, n, v_align[0], h_align)));
		return true;
	}

	if (s.size() >= 7 && s.substr(0, 7) == "replace") {
		ReplaceData rep;
		istringstream is(s.substr(7).c_str());
		string from, to;
		is >> from >> to;
		mathed_parse_cell(rep.from, from);
		mathed_parse_cell(rep.to, to);
		lyxerr << "replacing '" << from << "' with '" << to << "'\n";
		par()->replace(rep);
		return true;
	}

	string name = s.substr(1);

	if (name == "over" || name == "choose" || name == "atop") {
		MathAtom t(createMathInset(name));
		t.nucleus()->asNestInset()->cell(0) = array();
		array().clear();
		pos() = 0;
		niceInsert(t);
		popRight();
		left();
		return true;
	}

	// prevent entering of recursive macros
	if (formula()->lyxCode() == Inset::MATHMACRO_CODE
		&& formula()->getInsetName() == name)
	{
		lyxerr << "can't enter recursive macro\n";
		return true;
	}

	niceInsert(createMathInset(name));
	return true;
}


bool MathCursor::script(bool up)
{
	// Hack to get \\^ and \\_ working
	if (inMacroMode() && macroName() == "\\") {
		if (up)
			interpret("\\mathcircumflex");
		else
			interpret('_');
		return true;
	}

	macroModeClose();
	MathGridInset safe = grabAndEraseSelection();
	if (inNucleus()) {
		// we are in a nucleus of a script inset, move to _our_ script
		par()->asScriptInset()->ensure(up);
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
	//lyxerr << "interpret 2: '" << c << "'\n";
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
		//lyxerr << "interpret name: '" << name << "'\n";

		if (name.empty() && c == '\\') {
			backspace();
			interpret("\\backslash");
			return true;
		}

		if (isalpha(c)) {
			activeMacro()->setName(activeMacro()->name() + c);
			return true;
		}

		// handle 'special char' macros
		if (name == "\\") {
			// remove the '\\'
			backspace();
			if (c == '\\')
				interpret("\\backslash");
			else
				interpret(string("\\") + c);
			return true;
		}

		// leave macro mode and try again if necessary
		macroModeClose();
		if (c != ' ')
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
		//lyxerr << "starting with macro\n";
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

	if (c == '#') {
		insert(c); // LM_TC_TEX;
		return true;
	}

/*
	if (c == '{' || c == '}', c)) {
		insert(c); // LM_TC_TEX;
		return true;
	}
*/

	if (c == '{') {
		niceInsert(MathAtom(new MathBraceInset));
		return true;
	}

	if (c == '}') {
		return true;
	}

	if (c == '$') {
		insert(createMathInset("$"));
		return true;
	}

	if (c == '%') {
		insert(createMathInset("%"));
		return true;
	}

/*
	if (isalpha(c) && lastcode_ == LM_TC_GREEK) {
		insert(c, LM_TC_VAR);
		return true;
	}

	if (isalpha(c) && lastcode_ == LM_TC_GREEK1) {
		insert(c, LM_TC_VAR);
		lastcode_ = LM_TC_VAR;
		return true;
	}

	if (c == '\\') {
		insert(c, LM_TC_TEX);
		//bv->owner()->message(_("TeX mode"));
		return true;
	}
*/

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
		Cursor_[i].par_->infoize(os);
		os << "  ";
	}
	if (hasPrevAtom())
		if (prevAtom()->asSymbolInset() || prevAtom()->asScriptInset())
			prevAtom()->infoize(os);
	os << "                    ";
	return os.str().c_str(); // .c_str() needed for lyxstring
}


unsigned MathCursor::depth() const
{
	return Cursor_.size();
}




namespace {

void region(MathCursorPos const & i1, MathCursorPos const & i2,
	MathInset::row_type & r1, MathInset::row_type & r2,
	MathInset::col_type & c1, MathInset::col_type & c2)
{
	MathInset * p = i1.par_;
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


MathGridInset MathCursor::grabSelection() const
{
	if (!selection_)
		return MathGridInset();
	MathCursorPos i1;
	MathCursorPos i2;
	getSelection(i1, i2);
	// shouldn't we assert on i1.par_ == i2.par_?
	if (i1.idx_ == i2.idx_) {
		MathGridInset data(1, 1);
		MathArray::const_iterator it = i1.cell().begin();
		data.cell(0) = MathArray(it + i1.pos_, it + i2.pos_);
		return data;
	}
	row_type r1, r2;
	col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);
	MathGridInset data(c2 - c1 + 1, r2 - r1 + 1);
	for (row_type row = 0; row < data.nrows(); ++row)
		for (col_type col = 0; col < data.ncols(); ++col) {
			idx_type i = i1.par_->index(row + r1, col + c1);
			data.cell(data.index(row, col)) = i1.par_->cell(i);
		}
	return data;
}


void MathCursor::eraseSelection()
{
	MathCursorPos i1;
	MathCursorPos i2;
	getSelection(i1, i2);
	if (i1.idx_ == i2.idx_)
		i1.cell().erase(i1.pos_, i2.pos_);
	else {
		MathInset * p = i1.par_;
		row_type r1, r2;
		col_type c1, c2;
		region(i1, i2, r1, r2, c1, c2);
		for (row_type row = r1; row <= r2; ++row)
			for (col_type col = c1; col <= c2; ++col)
				p->cell(p->index(row, col)).clear();
	}
	cursor() = i1;
}


MathGridInset MathCursor::grabAndEraseSelection()
{
	if (!selection_)
		return MathGridInset();
	MathGridInset res = grabSelection();
	eraseSelection();
	selection_ = false;
	return res;
}


MathCursorPos MathCursor::normalAnchor() const
{
	if (Anchor_.size() < depth()) {
		Anchor_ = Cursor_;
		lyxerr << "unusual Anchor size\n";
	}
	//lyx::Assert(Anchor_.size() >= cursor.depth());
	// use Anchor on the same level as Cursor
	MathCursorPos normal = Anchor_[depth() - 1];
	if (depth() < Anchor_.size() && !(normal < cursor())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
	return normal;
}



void MathCursor::handleExtern(const string & arg)
{
	string lang;
	string extra;
	istringstream iss(arg.c_str());
	iss >> lang >> extra;
	if (extra.empty())
		extra = "noextra";


	if (selection()) {
		MathArray ar;
		selGet(ar);
		lyxerr << "use selection: " << ar << "\n";
		insert(pipeThroughExtern(lang, extra, ar));
		return;
	}

	MathArray eq;
	eq.push_back(MathAtom(new MathCharInset('=')));

	popToEnclosingHull();

	idx_type idx = 0;
	MathHullInset * hull = enclosingHull(idx);
	lyx::Assert(hull);
	idxLineFirst();

	if (hull->getType() == "simple") {
		MathArray::size_type pos = cursor().cell().find_last(eq);
		MathArray ar;
		if (pos == size()) {
			ar = array();
			lyxerr << "use whole cell: " << ar << "\n";
		} else {
			ar = MathArray(array().begin() + pos + 1, array().end());
			lyxerr << "use partial cell form pos: " << pos << "\n";
		}
		end();
		insert(eq);
		insert(pipeThroughExtern(lang, extra, ar));
		return;
	}

	if (hull->getType() == "equation") {
		lyxerr << "use equation inset\n";
		hull->mutate("eqnarray");
		MathArray & ar = cursor().cell();
		lyxerr << "use cell: " << ar << "\n";
		idxRight();
		cursor().cell() = eq;
		idxRight();
		cursor().cell() = pipeThroughExtern(lang, extra, ar);
		idxLineLast();
		return;
	}

	{
		lyxerr << "use eqnarray\n";
		idxLineLast();
		MathArray ar = cursor().cell();
		lyxerr << "use cell: " << ar << "\n";
		breakLine();
		idxRight();
		cursor().cell() = eq;
		idxRight();
		cursor().cell() = pipeThroughExtern(lang, extra, ar);
		idxLineLast();
	}

}


int MathCursor::dispatch(string const & cmd)
{
	// try to dispatch to adajcent items if they are not editable
	// actually, this should only happen for mouse clicks...
	if (hasNextAtom() && !openable(nextAtom(), false))
		if (int res = nextAtom().nucleus()->dispatch(cmd, 0, 0))
			return res;
	if (hasPrevAtom() && !openable(prevAtom(), false))
		if (int res = prevAtom().nucleus()->dispatch(cmd, 0, 0))
			return res;

	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		MathCursorPos & pos = Cursor_[i];
		if (int res = pos.par_->dispatch(cmd, pos.idx_, pos.pos_))
			return res;
	}
	return 0;
}


MathInset::mode_type MathCursor::currentMode() const
{
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		MathInset::mode_type res = Cursor_[i].par_->currentMode();
		if (res != MathInset::UNDECIDED_MODE)
			return res;
	}
	return MathInset::UNDECIDED_MODE;
}


void releaseMathCursor(BufferView * bv)
{
	if (!mathcursor)
		return;
	mathcursor->formula()->hideInsetCursor(bv);
	delete mathcursor;
	mathcursor = 0;
}
