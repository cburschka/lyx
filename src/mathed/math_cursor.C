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

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LColor.h"
#include "Painter.h"
#include "math_cursor.h"
#include "formulabase.h"
#include "math_arrayinset.h"
#include "math_braceinset.h"
#include "math_boxinset.h"
#include "math_casesinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
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
#include "math_specialcharinset.h"
#include "math_support.h"

#include <algorithm>
#include <cctype>

#define FILEDEBUG 0

using std::endl;
using std::min;
using std::max;
using std::swap;
using std::vector;
using std::ostringstream;

namespace {

struct Selection
{
	typedef MathInset::col_type col_type;
	typedef MathInset::row_type row_type;
	typedef MathInset::idx_type idx_type;

	Selection()
		: data_(1, 1)
	{}

	void region(MathCursorPos const & i1, MathCursorPos const & i2,
		row_type & r1, row_type & r2, col_type & c1, col_type & c2)
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

	void grab(MathCursor const & cursor)
	{
		MathCursorPos i1;
		MathCursorPos i2;
		cursor.getSelection(i1, i2);
		// shouldn'tt we assert on i1.par_ == i2.par_?
		if (i1.idx_ == i2.idx_) {
			data_ = MathGridInset(1, 1);
			data_.cell(0) = MathArray(i1.cell(), i1.pos_, i2.pos_);
		} else {
			row_type r1, r2;
			col_type c1, c2;
			region(i1, i2, r1, r2, c1, c2);
			data_ = MathGridInset(c2 - c1 + 1, r2 - r1 + 1);
			for (row_type row = 0; row < data_.nrows(); ++row)
				for (col_type col = 0; col < data_.ncols(); ++col) {
					idx_type i = i1.par_->index(row + r1, col + c1);
					data_.cell(data_.index(row, col)) = i1.par_->cell(i);
				}
		}
	}

	void erase(MathCursor & cursor)
	{
		MathCursorPos i1;
		MathCursorPos i2;
		cursor.getSelection(i1, i2);
		if (i1.idx_ == i2.idx_)
			i1.cell().erase(i1.pos_, i2.pos_);
		else {
			MathInset * p = i1.par_;
			row_type r1, r2;
			col_type c1, c2;
			region(i1, i2, r1, r2, c1, c2);
			for (row_type row = r1; row <= r2; ++row)
				for (col_type col = c1; col <= c2; ++col)
					p->cell(p->index(row, col)).erase();
		}
		cursor.cursor() = i1;
	}

	void paste(MathCursor & cursor) const
	{
		if (data_.nargs() == 1) {
			// single cell/part of cell
			cursor.insert(data_.cell(0));
		} else {
			// mulitple cells
			idx_type idx; // index of upper left cell
			MathGridInset * p = cursor.enclosingGrid(idx);
			col_type const numcols = min(data_.ncols(), p->ncols() - p->col(idx));
			row_type const numrows = min(data_.nrows(), p->nrows() - p->row(idx));
			for (row_type row = 0; row < numrows; ++row) {
				for (col_type col = 0; col < numcols; ++col) {
					idx_type i = p->index(row + p->row(idx), col + p->col(idx));
					p->cell(i).push_back(data_.cell(data_.index(row, col)));
				}
				// append the left over horizontal cells to the last column
				idx_type i = p->index(row + p->row(idx), p->ncols() - 1);
				for (col_type col = numcols; col < data_.ncols(); ++col) 
					p->cell(i).push_back(data_.cell(data_.index(row, col)));
			}
			// append the left over vertical cells to the last _cell_
			idx_type i = p->nargs() - 1;
			for (row_type row = numrows; row < data_.nrows(); ++row) 
				for (col_type col = 0; col < data_.ncols(); ++col) 
					p->cell(i).push_back(data_.cell(data_.index(row, col)));
		}
	}

	// glues selection to one cell
	MathArray glue() const
	{
		MathArray ar;
		for (unsigned i = 0; i < data_.nargs(); ++i)
			ar.push_back(data_.cell(i));
		return ar;
	}

	void clear()
	{
		data_ = MathGridInset(1, 1);
	}

	MathGridInset data_;
};


Selection theSelection;



}


MathCursor::MathCursor(InsetFormulaBase * formula, bool left)
	: formula_(formula), lastcode_(LM_TC_MIN), selection_(false)
{
	left ? first() : last();
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
	if (Cursor_.size() <= 1)
		return false;
	Cursor_.pop_back();
	return true;
}


bool MathCursor::popRight()
{
	//cerr << "Leaving atom "; par()->write(cerr, false); cerr << " right\n";
	if (Cursor_.size() <= 1)
		return false;
	Cursor_.pop_back();
	posRight();
	return true;
}



#if FILEDEBUG
	void MathCursor::dump(char const * what) const
	{
		lyxerr << "MC: " << what << "\n";
		lyxerr << " Cursor: " << Cursor_.size() << "\n";
		for (unsigned i = 0; i < Cursor_.size(); ++i)
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
	for (unsigned i = 0; i < Cursor_.size(); ++i)
		if (Cursor_[i].par_ == p)
			return true;
	return false;
}


bool MathCursor::openable(MathAtom const & t, bool sel) const
{
	if (!t->isActive())
		return false;

	if (t->asScriptInset())
		return false;

	if (sel) {
		// we can't move into anything new during selection
		if (Cursor_.size() == Anchor_.size())
			return false;
		if (t.nucleus() != Anchor_[Cursor_.size()].par_)
			return false;
	}
	return true;
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
	if (inMacroMode()) {
		macroModeClose();
		lastcode_ = LM_TC_MIN;
		return true;
	}
	selHandle(sel);
	lastcode_ = LM_TC_MIN;

	if (hasPrevAtom() && openable(prevAtom(), sel)) {
		pushRight(prevAtom());
		return true;
	}

	return posLeft() || idxLeft() || popLeft() || selection_;
}


bool MathCursor::right(bool sel)
{
	dump("Right 1");
	if (inMacroMode()) {
		macroModeClose();
		lastcode_ = LM_TC_MIN;
		return true;
	}
	selHandle(sel);
	lastcode_ = LM_TC_MIN;

	if (hasNextAtom() && openable(nextAtom(), sel)) {
		pushLeft(nextAtom());
		return true;
	}

	return posRight() || idxRight() || popRight() || selection_;
}


void MathCursor::first()
{
	Cursor_.clear();
	pushLeft(formula_->par());
}


void MathCursor::last()
{
	first();
	end();
}


bool positionable(MathCursor::cursor_type const & cursor,
		  MathCursor::cursor_type const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.size() > anchor.size())
		return false;

	// anchor might be deeper, should have same path then
	for (MathCursor::cursor_type::size_type i = 0; i < cursor.size(); ++i)
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
		// this ccan happen on creation of "math-display"
		dump("setPos 1.5");
		first();
	}
	dump("setPos 2");
}



void MathCursor::home(bool sel)
{
	dump("home 1");
	selHandle(sel);
	macroModeClose();
	lastcode_ = LM_TC_MIN;
	if (!par()->idxHome(idx(), pos()))
		popLeft();
	dump("home 2");
}


void MathCursor::end(bool sel)
{
	dump("end 1");
	selHandle(sel);
	macroModeClose();
	lastcode_ = LM_TC_MIN;
	if (!par()->idxEnd(idx(), pos()))
		popRight();
	dump("end 2");
}


void MathCursor::plainErase()
{
	array().erase(pos());
}


void MathCursor::markInsert()
{
	//lyxerr << "inserting mark\n";
	array().insert(pos(), MathAtom(new MathCharInset(0, lastcode_)));
}


void MathCursor::markErase()
{
	//lyxerr << "deleting mark\n";
	array().erase(pos());
}


void MathCursor::plainInsert(MathAtom const & t)
{
	array().insert(pos(), t);
	++pos();
}


void MathCursor::insert(char c, MathTextCodes t)
{
	//lyxerr << "inserting '" << c << "'\n";
	plainInsert(MathAtom(new MathCharInset(c, t)));
}


void MathCursor::insert(char c)
{
	insert(c, lastcode_);
}


void MathCursor::insert(MathAtom const & t)
{
	macroModeClose();

	if (selection_) {
		if (t->nargs())
			selCut();
		else
			selDel();
	}

	plainInsert(t);
}


void MathCursor::niceInsert(MathAtom const & t)
{
	selCut();
	insert(t); // inserting invalidates the pointer!
	MathAtom const & p = prevAtom();
	if (p->nargs()) {
		posLeft();
		right();  // do not push for e.g. MathSymbolInset
		selPaste();
	}
}


void MathCursor::insert(MathArray const & ar)
{
	macroModeClose();
	if (selection_)
		selCut();

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


void MathCursor::backspace()
{
	if (pos() == 0) {
		pullArg(false);
		return;
	}

	if (selection_) {
		selDel();
		return;
	}

	MathScriptInset * p = prevAtom()->asScriptInset();
	if (p) {
		p->removeScript(p->hasUp());
		// Don't delete if there is anything left
		if (p->hasUp() || p->hasDown())
			return;
	}

	--pos();
	plainErase();
}


void MathCursor::erase()
{
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

	MathScriptInset * p = nextAtom()->asScriptInset();
	if (p) {
		p->removeScript(p->hasUp());
		// Don't delete if there is anything left
		if (p->hasUp() || p->hasDown())
			return;
	}

	plainErase();
}


void MathCursor::delLine()
{
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
	cursor_type save = Cursor_;
	if (goUpDown(true))
		return true;
	Cursor_ = save;
	return selection_;
}


bool MathCursor::down(bool sel)
{
	dump("down 1");
	macroModeClose();
	selHandle(sel);
	cursor_type save = Cursor_;
	if (goUpDown(false))
		return true;
	Cursor_ = save;
	return selection_;
}


bool MathCursor::toggleLimits()
{
	if (!hasNextAtom())
		return false;
	MathScriptInset * t = nextAtom()->asScriptInset();
	if (!t)
		return false;
	int old = t->limits();
	t->limits(old < 0 ? 1 : -1);
	return old != t->limits();
}


void MathCursor::macroModeClose()
{
	MathInset::difference_type const t = macroNamePos();
	if (t == -1)
		return;
	string s = macroName();
	array().erase(t, pos());
	pos() = t;
	if (s != "\\")
		interpret(s);
}


MathInset::difference_type MathCursor::macroNamePos() const
{
	for (MathInset::difference_type i = pos() - 1; i >= 0; --i) {
		MathAtom & p = array().at(i);
		if (p->code() == LM_TC_TEX && p->getChar() == '\\')
			return i;
	}
	return -1;
}


string MathCursor::macroName() const
{
	string s;
	MathInset::difference_type i = macroNamePos();
	for (; i >= 0 && i < int(pos()); ++i)
		s += array().at(i)->getChar();
	return s;
}


void MathCursor::selCopy()
{
	dump("selCopy");
	if (selection_) {
		theSelection.grab(*this);
		selClear();
	}
}


void MathCursor::selCut()
{
	dump("selCut");
	if (selection_) {
		theSelection.grab(*this);
		theSelection.erase(*this);
		selClear();
	} else {
		theSelection.clear();
	}
}


void MathCursor::selDel()
{
	dump("selDel");
	if (selection_) {
		theSelection.erase(*this);
		if (pos() > size())
			pos() = size();
		selClear();
	}
}


void MathCursor::selPaste()
{
	dump("selPaste");
	theSelection.paste(*this);
	//theSelection.grab(*this);
	//selClear();
}


void MathCursor::selHandle(bool sel)
{
	if (sel == selection_)
		return;
	//theSelection.clear();
	Anchor_    = Cursor_;
	selection_ = sel;
}


void MathCursor::selStart()
{
	dump("selStart 1");
	//theSelection.clear();
	Anchor_ = Cursor_;
	selection_ = true;
	dump("selStart 2");
}


void MathCursor::selClear()
{
	dump("selClear 1");
	selection_ = false;
	dump("selClear 2");
}


void MathCursor::selGet(MathArray & ar)
{
	dump("selGet");
	if (!selection_)
		return;

	theSelection.grab(*this);
	ar = theSelection.glue();
}



void MathCursor::drawSelection(Painter & pain) const
{
	if (!selection_)
		return;

	MathCursorPos i1;
	MathCursorPos i2;
	getSelection(i1, i2);

	if (i1.idx_ == i2.idx_) {
		MathXArray & c = i1.xcell();
		int x1 = c.xo() + c.pos2x(i1.pos_);
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(i2.pos_);
		int y2 = c.yo() + c.descent();
		pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		vector<MathInset::idx_type> indices
			= i1.par_->idxBetween(i1.idx_, i2.idx_);
		for (unsigned i = 0; i < indices.size(); ++i) {
			MathXArray & c = i1.xcell(indices[i]);
			int x1 = c.xo();
			int y1 = c.yo() - c.ascent();
			int x2 = c.xo() + c.width();
			int y2 = c.yo() + c.descent();
			pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
		}
	}

#if 0
	// draw anchor if different from selection boundary
	MathCursorPos anc = Anchor_.back();
	if (anc != i1 && anc != i2) {
		MathXArray & c = anc.xcell();
		int x  = c.xo() + c.pos2x(anc.pos_);
		int y1 = c.yo() - c.ascent();
		int y2 = c.yo() + c.descent();
		pain.line(x, y1, x, y2, LColor::math);
	}
#endif
}


void MathCursor::handleFont(MathTextCodes t)
{
	macroModeClose();
	if (selection_) {
		MathCursorPos i1;
		MathCursorPos i2;
		getSelection(i1, i2);
		if (i1.idx_ == i2.idx_) {
			MathArray & ar = i1.cell();
			for (MathInset::pos_type pos = i1.pos_; pos != i2.pos_; ++pos)
				ar.at(pos)->handleFont(t);
		}
	} else
		lastcode_ = (lastcode_ == t) ? LM_TC_VAR : t;
}


void MathCursor::handleDelim(string const & l, string const & r)
{
	handleNest(new MathDelimInset(l, r));
}


void MathCursor::handleNest(MathInset * p)
{
	if (selection_) {
		selCut();
		p->cell(0) = theSelection.glue();
	}
	insert(MathAtom(p)); // this invalidates p!
	pushRight(prevAtom());
}


void MathCursor::getPos(int & x, int & y)
{
#ifdef WITH_WARNINGS
#warning This should probably take cellXOffset and cellYOffset into account
#endif
	x = xarray().xo() + xarray().pos2x(pos());
	y = xarray().yo();
}


MathInset * MathCursor::par() const
{
	return cursor().par_;
}


InsetFormulaBase * MathCursor::formula()
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
	return macroNamePos() != -1;
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
	for (MathInset::difference_type i = Cursor_.size() - 1; i >= 0; --i) {
		MathGridInset * p = Cursor_[i].par_->asGridInset();
		if (p) {
			idx = Cursor_[i].idx_;
			return p;
			lyxerr << "found grid and idx: " << idx << "\n";
		}
	}
	return 0;
}


void MathCursor::popToEnclosingGrid()
{
	while (Cursor_.size() && !Cursor_.back().par_->asGridInset())
		Cursor_.pop_back();
}


void MathCursor::pullArg(bool goright)
{
	dump("pullarg");
	MathArray a = array();

	MathScriptInset const * p = par()->asScriptInset();
	if (p) {
		// special handling for scripts
		const bool up = p->hasUp();
		popLeft();
		MathScriptInset * q = nextAtom()->asScriptInset();
		if (q)
			q->removeScript(up);
		++pos();
		array().insert(pos(), a);
		return;
	}

	if (popLeft()) {
		plainErase();
		array().insert(pos(), a);
		if (goright)
			pos() += a.size();
	} else {
		formula()->mutateToText();
	}
}


void MathCursor::touch()
{
	cursor_type::const_iterator it = Cursor_.begin();
	cursor_type::const_iterator et = Cursor_.end();
	for ( ; it != et; ++it)
		it->xcell().touch();
}


void MathCursor::normalize()
{
#if 0
	// rebreak
	{
		MathIterator it = ibegin(formula()->par().nucleus());
		MathIterator et = iend(formula()->par().nucleus());
		for (; it != et; ++it)
			if (it.par()->asBoxInset())
				it.par()->asBoxInset()->rebreak();
	}
#endif

	if (idx() >= par()->nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << " " << par()->nargs() << "\n";
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
			MathScriptInset * p = array().at(i)->asScriptInset();
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
	return Cursor_[0].par_->asGridInset()->col(Cursor_[0].idx_);
}


MathCursor::row_type MathCursor::hullRow() const
{
	return Cursor_[0].par_->asGridInset()->row(Cursor_[0].idx_);
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
	return array().at(pos() - 1);
}


MathAtom & MathCursor::prevAtom()
{
	lyx::Assert(pos() > 0);
	return array().at(pos() - 1);
}


MathAtom const & MathCursor::nextAtom() const
{
	lyx::Assert(pos() < size());
	return array().at(pos());
}


MathAtom & MathCursor::nextAtom()
{
	lyx::Assert(pos() < size());
	return array().at(pos());
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;

	if (idx() >= par()->nargs()) {
		lyxerr << "############  idx_ " << idx() << " not valid\n";
		return dummy;
	}

	if (Cursor_.size() == 0) {
		lyxerr << "############  Cursor_.size() == 0 not valid\n";
		return dummy;
	}

	return cursor().cell();
}


MathXArray & MathCursor::xarray() const
{
	static MathXArray dummy;

	if (Cursor_.size() == 0) {
		lyxerr << "############  Cursor_.size() == 0 not valid\n";
		return dummy;
	}

	return cursor().xcell();
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

	MathHullInset * p = formula()->par()->asHullInset();
	if (!p)
		return;

	if (p->getType() == LM_OT_SIMPLE || p->getType() == LM_OT_EQUATION) {
		p->mutate(LM_OT_EQNARRAY);
		idx() = 0;
		pos() = size();
	} else {
		p->addRow(hullRow());

		// split line
		const row_type r = hullRow();
		for (col_type c = hullCol() + 1; c < p->ncols(); ++c)
			p->cell(p->index(r, c)).swap(p->cell(p->index(r + 1, c)));

		// split cell
		splitCell();
		p->cell(idx()).swap(p->cell(idx() + p->ncols() - 1));
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
	lyx::Assert(Cursor_.size());
	return Cursor_.back();
}


MathCursorPos const & MathCursor::cursor() const
{
	lyx::Assert(Cursor_.size());
	return Cursor_.back();
}


bool MathCursor::goUpDown(bool up)
{
	// Be warned: The 'logic' implemented in this function is highly fragile.
	// A distance of one pixel or a '<' vs '<=' _really_ matters.
	// So fiddle around with it only if you know what you are doing!
	int xlow, xhigh, ylow, yhigh;

  int xo, yo;
	getPos(xo, yo);

	// try neigbouring script insets
	// try left
	if (hasPrevAtom()) {
		MathScriptInset * p = prevAtom()->asScriptInset();
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
		MathScriptInset * p = nextAtom()->asScriptInset();
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
		if (par()->idxUpDown(idx(), up)) {
			// we found a cell that thinks it has something "below" us.
			///lyxerr << "updown: found inset that handles UpDown\n";
			xarray().boundingBox(xlow, xhigh, ylow, yhigh);
			// project (xo,yo) onto proper box
			///lyxerr << "\n   xo: " << xo << " yo: " << yo
			///       << "\n   xlow: " << xlow << " ylow: " << ylow
			///       << "\n   xhigh: " << xhigh << " yhigh: " << yhigh;
			xo = min(max(xo, xlow), xhigh);
			yo = min(max(yo, ylow), yhigh);
			///lyxerr << "\n   xo2: " << xo << " yo2: " << yo << "\n";
			bruteFind(xo, yo, xlow, xhigh, ylow, yhigh);
			///lyxerr << "updown: handled by final brute find\n";
			return true;
		}

		// leave inset
		if (!popLeft()) {
			// no such inset found, just take something "above"
			///lyxerr << "updown: handled by strange case\n";
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


bool MathCursor::bruteFind
	(int x, int y, int xlow, int xhigh, int ylow, int yhigh)
{
	cursor_type best_cursor;
	double best_dist = 1e10;

	MathIterator it = ibegin(formula()->par().nucleus());
	MathIterator et = iend(formula()->par().nucleus());
	while (1) {
		// avoid invalid nesting when selecting
		if (!selection_ || positionable(it.cursor(), Anchor_)) {
			MathCursorPos const & top = it.position();
			int xo = top.xpos();
			int yo = top.ypos();
			if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
				double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
				// '<=' in order to take the last possible position
				// this is important for clicking behind \sum in e.g. '\sum_i a'
				if (d <= best_dist) {
					best_dist   = d;
					best_cursor = it.cursor();
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
	//owner_->getIntl()->getTrans().TranslateAndInsert(s[0], lt);
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
		MathArray ar = array();
		MathAtom t(createMathInset(name));
		t->asNestInset()->cell(0).swap(array());
		pos() = 0;
		niceInsert(t);
		popRight();
		left();
		return true;
	}

	latexkeys const * l = in_word_set(name);
	if (l && (l->token == LM_TK_FONT || l->token == LM_TK_OLDFONT)) {
		lastcode_ = static_cast<MathTextCodes>(l->id);
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
	macroModeClose();
	selCut();
	if (hasPrevAtom() && prevAtom()->asScriptInset()) {
		prevAtom()->asScriptInset()->ensure(up);
		pushRight(prevAtom());
		idx() = up;
		pos() = size();
	} else if (hasNextAtom() && nextAtom()->asScriptInset()) {
		nextAtom()->asScriptInset()->ensure(up);
		pushLeft(nextAtom());
		idx() = up;
		pos() = 0;
	} else {
		plainInsert(MathAtom(new MathScriptInset(up)));
		prevAtom()->asScriptInset()->ensure(up);
		pushRight(prevAtom());
		idx() = up;
		pos() = 0;
	}
	selPaste();
	dump("1");
	return true;
}


bool MathCursor::interpret(char c)
{
	//lyxerr << "interpret 2: '" << c << "'\n";
	if (inMacroArgMode()) {
		--pos();
		plainErase();
		int n = c - '0';
		MathMacroTemplate * p = formula()->par()->asMacroTemplate();
		if (p && 1 <= n && n <= p->numargs())
			insert(MathAtom(new MathMacroArgument(c - '0', lastcode_)));
		else {
			insert(MathAtom(new MathSpecialCharInset('#')));
			interpret(c); // try again
		}
		return true;
	}

	// handle macroMode
	if (inMacroMode()) {
		string name = macroName();
		//lyxerr << "interpret name: '" << name << "'\n";

		// extend macro name if possible
		if (isalpha(c)) {
			insert(c, LM_TC_TEX);
			return true;
		}

		// leave macro mode if explicitly requested
		if (c == ' ') {
			macroModeClose();
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

		// leave macro mode and try again
		macroModeClose();
		interpret(c);
		return true;
	}

	if (selection_) {
		selClear();
		if (c == ' ')
			return true;
		// fall through in the other cases
	}

	if (lastcode_ == LM_TC_TEXTRM || par()->asBoxInset()) {
		// suppress direct insertion of two spaces in a row
		// the still allows typing  '<space>a<space>' and deleting the 'a', but
		// it is better than nothing...
		if (c == ' ' && hasPrevAtom() && prevAtom()->getChar() == ' ')
			return true;
		insert(c, LM_TC_TEXTRM);
		return true;
	}

	if (c == ' ') {
		if (hasPrevAtom() && prevAtom()->asSpaceInset()) {
			prevAtom()->asSpaceInset()->incSpace();
			return true;
		}
		if (popRight())
			return true;
		// if are at the very end, leave the formula
		return pos() != size();
	}

	if (c == '#') {
		insert(c, LM_TC_TEX);
		return true;
	}

/*
	if (c == '{' || c == '}', c)) {
		insert(c, LM_TC_TEX);
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

	if (c == '$' || c == '%') {
		insert(MathAtom(new MathSpecialCharInset(c)));
		lastcode_ = LM_TC_VAR;
		return true;
	}

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

	// no special circumstances, so insert the character without any fuss
	insert(c, lastcode_ == LM_TC_MIN ? MathCharInset::nativeCode(c) : lastcode_);
	lastcode_ = LM_TC_MIN;
	return true;
}



MathCursorPos MathCursor::normalAnchor() const
{
	if (Anchor_.size() < Cursor_.size()) {
		Anchor_ = Cursor_;
		lyxerr << "unusual Anchor size\n";
		dump("1");
	}
	//lyx::Assert(Anchor_.size() >= Cursor_.size());
	// use Anchor on the same level as Cursor
	MathCursorPos normal = Anchor_[Cursor_.size() - 1];
	if (Cursor_.size() < Anchor_.size() && !(normal < cursor())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
	return normal;
}


void MathCursor::stripFromLastEqualSign()
{
	// find position of last '=' in the array
	MathArray & ar = cursor().cell();
	MathArray::const_iterator et = ar.end();
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		if ((*it)->getChar() == '=')
			et = it;

	// delete everything behind this position
	ar.erase(et - ar.begin(), ar.size());
	pos() = ar.size();
}


void MathCursor::setSelection(cursor_type const & where, size_type n)
{
	selection_ = true;
	Anchor_ = where;
	Cursor_ = where;
	cursor().pos_ += n;
}


string MathCursor::info() const
{
	ostringstream os;
	if (pos() > 0)
		prevAtom()->infoize(os);
	return os.str().c_str(); // .c_str() needed for lyxstring
}
