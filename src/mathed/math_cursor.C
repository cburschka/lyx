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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include <algorithm>
#include <cctype>

#include "support/lstrings.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LColor.h"
#include "Painter.h"
#include "support.h"
#include "formulabase.h"
#include "math_cursor.h"
#include "math_factory.h"
#include "math_arrayinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_matrixinset.h"
#include "math_scriptinset.h"
#include "math_spaceinset.h"
#include "math_specialcharinset.h"

#define FILEDEBUG 0

using std::endl;
using std::min;
using std::max;
using std::swap;
using std::isalnum;

namespace {

struct Selection
{
	void grab(MathCursor const & cursor)
	{
		data_.clear();
		MathCursorPos i1;
		MathCursorPos i2;
		cursor.getSelection(i1, i2); 
		if (i1.idx_ == i2.idx_)
			data_.push_back(MathArray(i1.cell(), i1.pos_, i2.pos_));
		else {
			std::vector<MathInset::idx_type> indices =
				(*i1.par_)->idxBetween(i1.idx_, i2.idx_);
			for (MathInset::idx_type i = 0; i < indices.size(); ++i)
				data_.push_back(i1.cell(indices[i]));
		}
	}

	void erase(MathCursor & cursor)
	{
		MathCursorPos i1;
		MathCursorPos i2;
		cursor.getSelection(i1, i2); 
		if (i1.idx_ == i2.idx_) {
			i1.cell().erase(i1.pos_, i2.pos_);
		} else {
			std::vector<MathInset::idx_type> indices =
				(*i1.par_)->idxBetween(i1.idx_, i2.idx_);
			for (unsigned i = 0; i < indices.size(); ++i)
				i1.cell(indices[i]).erase();
		}
		cursor.cursor() = i1;
	}

	void paste(MathCursor & cursor) const
	{
		MathArray ar = glue();
		cursor.paste(ar);
	}

	// glues selection to one cell
	MathArray glue() const
	{
		MathArray ar;
		for (unsigned i = 0; i < data_.size(); ++i)
			ar.push_back(data_[i]);
		return ar;
	}

	void clear()
	{
		data_.clear();
	}

	std::vector<MathArray> data_;
};


Selection theSelection;


#if FILEDEBUG
std::ostream & operator<<(std::ostream & os, MathCursorPos const & p)
{
	os << "(par: " << p.par_ << " idx: " << p.idx_ << " pos: " << p.pos_ << ")";
	return os;
}
#endif

}


MathCursor::MathCursor(InsetFormulaBase * formula, bool left)
	: formula_(formula), lastcode_(LM_TC_VAR), selection_(false)
{
	left ? first() : last();
}


void MathCursor::push(MathAtom & t)
{
	MathCursorPos p;
	p.par_ = &t;
	p.idx_ = 0;
	p.pos_ = 0;
	Cursor_.push_back(p);
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
	//cerr << "Leaving atom "; par()->write(cerr, false); cerr << " left\n";
	if (Cursor_.size() <= 1)
		return false;
	//if (nextInset())
	//	nextInset()->removeEmptyScripts();
	Cursor_.pop_back();
	//if (nextAtom())
	//	nextAtom()->removeEmptyScripts();
	return true;
}


bool MathCursor::popRight()
{
	//cerr << "Leaving atom "; par()->write(cerr, false); cerr << " right\n";
	if (Cursor_.size() <= 1)
		return false;
	//if (nextInset())
	//	nextInset()->removeEmptyScripts();
	Cursor_.pop_back();
	//if (nextInset())
	//	nextInset()->removeEmptyScripts();
	posRight();
	return true;
}



#if FILEDEBUG
void MathCursor::dump(char const * what) const
{
	lyxerr << "MC: " << what << "\n";
	for (unsigned i = 0; i < Cursor_.size(); ++i)
		lyxerr << "  i: " << i 
			<< " Cursor: pos: " << Cursor_[i].pos_
			<< " idx: " << Cursor_[i].idx_
			<< " par: " << Cursor_[i].par_ << "\n";

	for (unsigned i = 0; i < Anchor_.size(); ++i)
		lyxerr << "  i: " << i 
			<< " Anchor: pos: " << Anchor_[i].pos_
			<< " idx: " << Anchor_[i].idx_
			<< " par: " << Anchor_[i].par_ << "\n";

	lyxerr	<< " sel: " << selection_ << "\n";
}


void MathCursor::seldump(char const * str) const
{
	//lyxerr << "SEL: " << str << ": '" << theSelection << "'\n";
	//dump("   Pos");

	lyxerr << "\n\n\n=================vvvvvvvvvvvvv=======================   "
		<<  str << "\ntheSelection: " << selection_
		<< " '" << theSelection.glue() << "'\n";
	for (unsigned int i = 0; i < Cursor_.size(); ++i) 
		lyxerr << Cursor_[i].par_ << "\n'" << Cursor_[i].cell() << "'\n";
	lyxerr << "\n";
	for (unsigned int i = 0; i < Anchor_.size(); ++i) 
		lyxerr << Anchor_[i].par_ << "\n'" << Anchor_[i].cell() << "'\n";
	//lyxerr << "\ncursor.pos_: " << pos();
	//lyxerr << "\nanchor.pos_: " << anchor().pos_;
	lyxerr << "\n===================^^^^^^^^^^^^=====================\n\n\n";
}

#else

void MathCursor::seldump(char const *) const {}
void MathCursor::dump(char const *) const {}

#endif


bool MathCursor::isInside(MathInset const * p) const
{
	for (unsigned i = 0; i < Cursor_.size(); ++i) 
		if (Cursor_[i].par_->nucleus() == p) 
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
		if (&t != Anchor_[Cursor_.size()].par_)
			return false;
	}
	return true;
}


bool MathCursor::positionable(MathAtom const & t, int x, int y) const
{
	if (selection_) {
		// we can't move into anything new during selection
		if (Cursor_.size() == Anchor_.size())
			return 0;
		//if (t != Anchor_[Cursor_.size()].par_)
		//	return 0;
	}

	return t->nargs() && t->covers(x, y);
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
		lastcode_ = LM_TC_VAR;
		return true;
	}
	selHandle(sel);
	lastcode_ = LM_TC_VAR;

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
		lastcode_ = LM_TC_VAR;
		return true;
	}
	selHandle(sel);
	lastcode_ = LM_TC_VAR;

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


void MathCursor::setPos(int x, int y)
{
	//dump("setPos 1");
	//lyxerr << "MathCursor::setPos x: " << x << " y: " << y << "\n";

	macroModeClose();
	lastcode_ = LM_TC_VAR;
	first();

	cursor().par_  = &formula_->par();

	while (1) {
		idx() = 0;
		cursor().pos_ = 0;
		//lyxerr << "found idx: " << idx() << " cursor: " << pos()  << "\n";
		int distmin = 1 << 30; // large enough
		for (unsigned int i = 0; i < par()->nargs(); ++i) {
			MathXArray const & ar = par()->xcell(i);
			int x1 = x - ar.xo();
			int y1 = y - ar.yo();
			MathXArray::size_type c  = ar.x2pos(x1);
			int xx = abs(x1 - ar.pos2x(c));
			int yy = abs(y1);
			//lyxerr << "idx: " << i << " xx: " << xx << " yy: " << yy
			//	<< " c: " << c  << " xo: " << ar.xo() << "\n";
			if (yy + xx <= distmin) {
				distmin = yy + xx;
				idx()   = i;
				pos()   = c;
			}
		}
		//lyxerr << "found idx: " << idx() << " cursor: "
		//	<< pos()  << "\n";
		if (hasNextAtom() && positionable(nextAtom(), x, y))
			pushLeft(nextAtom());
		else if (hasPrevAtom() && positionable(prevAtom(), x, y))
			pushRight(prevAtom());
		else 
			break;
	}
	//dump("setPos 2");
}


void MathCursor::home(bool sel)
{
	dump("home 1");
	selHandle(sel);
	macroModeClose();
	lastcode_ = LM_TC_VAR;
	if (!par()->idxHome(idx(), pos())) 
		popLeft();
	dump("home 2");
}


void MathCursor::end(bool sel)
{
	dump("end 1");
	selHandle(sel);
	macroModeClose();
	lastcode_ = LM_TC_VAR;
	if (!par()->idxEnd(idx(), pos()))
		popRight();
	dump("end 2");
}


void MathCursor::plainErase()
{
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
#ifdef WITH_WARNINGS
#warning "redraw disabled"
#endif
	//p->metrics(p->size());
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

	// delete empty cells if necessary
	if (array().empty()) {
		bool popit;
		bool removeit;
		par()->idxDelete(idx(), popit, removeit);
		if (popit && popLeft() && removeit)
			plainErase();
		return;
	}

	if (pos() == size())
		return;

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

	if (par()->nrows() > 1)
		par()->delRow(row());

	if (pos() > size())
		pos() = size();
}


bool MathCursor::up(bool sel)
{
	dump("up 1");
	macroModeClose();
	selHandle(sel);

	if (!selection_) {
		// check whether we could move into a superscript 
		if (hasPrevAtom()) {
			MathAtom & p = prevAtom();
			if (p->asScriptInset() && p->asScriptInset()->hasUp()) {
				pushRight(p);
				idx() = 1;
				pos() = size();
				return true;
			}
		}

		if (hasNextAtom()) {
			MathAtom & n = nextAtom();
			if (n->asScriptInset() && n->asScriptInset()->hasUp()) {
				pushLeft(n);
				idx() = 1;
				pos() = 0;
				return true;
			}
		}
	}

	return goUp() || selection_;
}


bool MathCursor::down(bool sel)
{
	dump("down 1");
	macroModeClose();
	selHandle(sel);

	if (!selection_) {
		// check whether we could move into a subscript 
		if (hasPrevAtom()) {
			MathAtom & p = prevAtom();
			if (p->asScriptInset() && p->asScriptInset()->hasDown()) {
				pushRight(p);
				idx() = 0;
				pos() = size();
				return true;
			}
		}

		if (hasNextAtom()) {
			MathAtom & n = nextAtom();
			if (n->asScriptInset() && n->asScriptInset()->hasDown()) {
				pushLeft(n);
				idx() = 0;
				pos() = 0;
				return true;
			}
		}
	}

	return goDown() || selection_;
}


bool MathCursor::toggleLimits()
{
	if (!hasPrevAtom())
		return false;
	MathScriptInset * t = prevAtom()->asScriptInset();
	if (!t)
		return false;
	int old = t->limits();
	t->limits(old < 0 ? 1 : -1);
	return old != t->limits();
}


void MathCursor::macroModeClose()
{
	string s = macroName();
	if (s.size()) {
		size_type old = pos();
		pos() -= s.size();
		array().erase(pos(), old);
		interpret(s);
	}
}


int MathCursor::macroNamePos() const
{
	for (int i = pos() - 1; i >= 0; --i) { 
		MathAtom & p = array().at(i);
		if (p->code() == LM_TC_TEX && p->getChar() == '\\')
			return i;
	}
	return -1;
}


string MathCursor::macroName() const
{
	string s;
	for (int i = macroNamePos(); i >= 0 && i < int(pos()); ++i) 
		s += array().at(i)->getChar();
	return s;
}


void MathCursor::selCopy()
{
	seldump("selCopy");
	if (selection_) {
		theSelection.grab(*this);
		selClear();
	}
}


void MathCursor::selCut()
{
	seldump("selCut");
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
	seldump("selDel");
	if (selection_) {
		theSelection.erase(*this);
		pos() = 0;
		selClear();
	}
}


void MathCursor::selPaste()
{
	seldump("selPaste");
	theSelection.paste(*this);
	theSelection.grab(*this);
	//selClear();
}


void MathCursor::selHandle(bool sel)
{
	if (sel == selection_)
		return;

	theSelection.clear();
	Anchor_    = Cursor_;
	selection_ = sel;
}


void MathCursor::selStart()
{
	seldump("selStart");
	if (selection_)
		return;

	theSelection.clear();
	Anchor_ = Cursor_;
	selection_ = true;
}


void MathCursor::selClear()
{
	seldump("selClear");
	selection_ = false;
}


void MathCursor::selGet(MathArray & ar)
{
	seldump("selGet");
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

	//lyxerr << "selection from: " << i1 << " to " << i2 << "\n";

	if (i1.idx_ == i2.idx_) {
		MathXArray & c = i1.xcell();
		int x1 = c.xo() + c.pos2x(i1.pos_);
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(i2.pos_);
		int y2 = c.yo() + c.descent();
		pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		std::vector<MathInset::idx_type> indices
			= (*i1.par_)->idxBetween(i1.idx_, i2.idx_);
		for (unsigned i = 0; i < indices.size(); ++i) {
			MathXArray & c = i1.xcell(indices[i]);
			int x1 = c.xo();
			int y1 = c.yo() - c.ascent();
			int x2 = c.xo() + c.width();
			int y2 = c.yo() + c.descent();
			pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
		}
	}
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


MathAtom & MathCursor::par() const
{
	return *cursor().par_;
}


InsetFormulaBase const * MathCursor::formula()
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


bool MathCursor::selection() const
{
	return selection_;
}


MathArrayInset * MathCursor::enclosingArray(MathCursor::idx_type & idx) const
{
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		MathArrayInset * p = (*Cursor_[i].par_)->asArrayInset();
		if (p) {
			idx = Cursor_[i].idx_;
			return p;
		}
	}
	return 0;
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
	}
}


void MathCursor::normalize() const
{
#ifdef WITH_WARNINGS
#warning This is evil!
#endif
	MathCursor * it = const_cast<MathCursor *>(this);

 	if (idx() >= par()->nargs()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << " " << par()->nargs() << "\n";
		dump("error 2");
	}
 	it->idx()    = min(idx(), par()->nargs() - 1);

	if (pos() > size()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << " " << size() <<  " in idx: " << it->idx()
			<< " in atom: '";
		MathWriteInfo wi(0, lyxerr, false);
		it->par()->write(wi);
		lyxerr << "\n";
		dump("error 4");
	}
	it->pos() = min(pos(), size());
}


MathCursor::size_type MathCursor::size() const
{
	return array().size();
}


MathCursor::col_type MathCursor::col() const
{
	return par()->col(idx());
}


MathCursor::row_type MathCursor::row() const
{
	return par()->row(idx());
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

	return cursor().cell();
}


MathXArray & MathCursor::xarray() const
{
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
	if (idx() == par()->nargs() - 1) 
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

	MathMatrixInset * p = formula()->par()->asMatrixInset();
	if (!p)
		return;

	if (p->getType() == LM_OT_SIMPLE || p->getType() == LM_OT_EQUATION) {
		p->mutate(LM_OT_EQNARRAY);
		idx() = 0;
		pos() = size();
	} else {
		p->addRow(row());

		// split line
		const row_type r = row();
		for (col_type c = col() + 1; c < p->ncols(); ++c) {
			const MathMatrixInset::idx_type i1 = p->index(r, c);
			const MathMatrixInset::idx_type i2 = p->index(r + 1, c);	
			//lyxerr << "swapping cells " << i1 << " and " << i2 << "\n";
			p->cell(i1).swap(p->cell(i2));
		}

		// split cell
		splitCell();
		p->cell(idx()).swap(p->cell(idx() + p->ncols() - 1));
	}
}


char MathCursor::valign() const
{
	idx_type idx;
	MathArrayInset * p = enclosingArray(idx);
	return p ? p->valign() : '\0';
}


char MathCursor::halign() const
{
	idx_type idx;
	MathArrayInset * p = enclosingArray(idx);
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
	return Cursor_.back();
}


MathCursorPos const & MathCursor::cursor() const
{
	return Cursor_.back();
}


int MathCursor::cellXOffset() const
{
	return par()->cellXOffset(idx());
}


int MathCursor::cellYOffset() const
{
	return par()->cellYOffset(idx());
}


int MathCursor::xpos() const
{
	return cellXOffset() + xarray().pos2x(pos());
}


int MathCursor::ypos() const
{
	return cellYOffset();
}



void MathCursor::gotoX(int x) 
{
	pos() = xarray().x2pos(x - cellXOffset());
}


bool MathCursor::goUp()
{
	// first ask the inset if it knows better then we
	if (par()->idxUp(idx(), pos()))
		return true;

	// leave subscript to the nearest side	
	MathScriptInset * p = par()->asScriptInset();
	if (p && idx() == 0) {
		if (pos() <= size() / 2)
			popLeft();
		else
			popRight();		
		return true;
	}

	// if not, apply brute force.
	int x0;
	int y0;
	getPos(x0, y0);
	std::vector<MathCursorPos> save = Cursor_;
	y0 -= xarray().ascent();
	for (int y = y0 - 4; y > formula()->upperY(); y -= 4) {
		setPos(x0, y);
		if (save != Cursor_ && xarray().yo() < y0)
			return true;	
	}
	Cursor_ = save;
	return false;
}


bool MathCursor::goDown()
{
	// first ask the inset if it knows better then we
	if (par()->idxDown(idx(), pos()))
		return true;

	// leave superscript to the nearest side	
	MathScriptInset * p = par()->asScriptInset();
	if (p && idx() == 1) {
		if (pos() <= size() / 2)
			popLeft();
		else
			popRight();		
		return true;
	}

	// if not, apply brute force.
	int x0;
	int y0;
	getPos(x0, y0);
	std::vector<MathCursorPos> save = Cursor_;
	y0 += xarray().descent();
	for (int y = y0 + 4; y < formula()->lowerY(); y += 4) {
		setPos(x0, y);
		if (save != Cursor_ && xarray().yo() > y0)
			return true;	
	}
	Cursor_ = save;
	return false;
}


bool MathCursor::idxLeft()
{
	return par()->idxLeft(idx(), pos());
}


bool MathCursor::idxRight()
{
	return par()->idxRight(idx(), pos());
}


void MathCursor::interpret(string const & s)
{
	//lyxerr << "interpret 1: '" << s << "'\n";
	if (s.empty())
		return;

	if (s.size() == 1) {
		interpret(s[0]);
		return;
	}

	//lyxerr << "char: '" << s[0] << "'  int: " << int(s[0]) << endl;
	//owner_->getIntl()->getTrans().TranslateAndInsert(s[0], lt);	
	//lyxerr << "trans: '" << s[0] << "'  int: " << int(s[0]) << endl;

	if (s.size() > 7 && s.substr(0, 7) == "matrix ") {
		unsigned int m = 1;
		unsigned int n = 1;
		string v_align;
		string h_align;
		istringstream is(s.substr(7).c_str());
		is >> m >> n >> v_align >> h_align;
		m = std::max(1u, m);
		n = std::max(1u, n);
		v_align += 'c';
		niceInsert(MathAtom(new MathArrayInset(m, n, v_align[0], h_align)));
		return;
	}

	if (s == "\\over" || s == "\\choose" || s == "\\atop") {
		MathArray ar = array();
		MathAtom t = createMathInset(s.substr(1));
		t->asNestInset()->cell(0).swap(array());
		pos() = 0;
		niceInsert(t);
		popRight();
		left();
		return;
	}

	niceInsert(createMathInset(s.substr(1)));
}


void MathCursor::interpret(char c)
{
	//lyxerr << "interpret 2: '" << c << "'\n";

	if (inMacroMode()) {
		string name = macroName();

		if (name == "\\" && c == '#') {
			insert(c, LM_TC_TEX);
			return;
		}

		if (name == "\\" && c == '\\') {
			backspace();
			interpret("\\backslash");
			return;
		}

		if (name == "\\#" && '1' <= c && c <= '9') {
			insert(c, LM_TC_TEX);
			macroModeClose();
			return;
		}

		if (isalpha(c)) {
			insert(c, LM_TC_TEX);
			return;
		}

		if (name == "\\") {
			insert(c, LM_TC_TEX);
			macroModeClose();
			return;
		}

		macroModeClose();
		return;
	}

	// no macro mode
	if (c == '^' || c == '_') {
		const bool up = (c == '^');
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
		return;
	}

	if (selection_)
		selDel();

	if (lastcode_ == LM_TC_TEXTRM) {
		insert(c, LM_TC_TEXTRM);
		return;
	}

	if (c == ' ') {
		if (hasPrevAtom() && prevAtom()->asSpaceInset()) {
			prevAtom()->asSpaceInset()->incSpace();
			return;
		}

		if (mathcursor->popRight())
			return;

#warning look here
			// this would not work if the inset is in an table!
			//bv->text->cursorRight(bv, true);
			//result = FINISHED;
		return;
	}

	if (strchr("{}", c)) {
		insert(c, LM_TC_TEX);
		return;
	}

	if (strchr("#$%", c)) {
		insert(MathAtom(new MathSpecialCharInset(c)));	
		lastcode_ = LM_TC_VAR;
		return;
	}

	if (isalpha(c) && lastcode_ == LM_TC_GREEK) {
		insert(c, LM_TC_VAR);
		return;	
	}

	if (isalpha(c) && lastcode_ == LM_TC_GREEK1) {
		insert(c, LM_TC_VAR);
		lastcode_ = LM_TC_VAR;
		return;	
	}

	if (c == '\\') {
		insert(c, LM_TC_TEX);
		//bv->owner()->message(_("TeX mode"));
		return;	
	}

	// no special circumstances, so insert the character without any fuss
	insert(c, LM_TC_MIN);
}



////////////////////////////////////////////////////////////////////////


bool operator==(MathCursorPos const & ti, MathCursorPos const & it)
{
	return ti.par_ == it.par_ && ti.idx_ == it.idx_ && ti.pos_ == it.pos_;
}


bool operator<(MathCursorPos const & ti, MathCursorPos const & it)
{
	if (ti.par_ != it.par_) {
		lyxerr << "can't compare cursor and anchor in different insets\n";
		return true;
	}
	if (ti.idx_ != it.idx_)
		return ti.idx_ < it.idx_;
	return ti.pos_ < it.pos_;
}


MathArray & MathCursorPos::cell(MathCursor::idx_type idx) const
{
	return (*par_)->cell(idx);
}


MathArray & MathCursorPos::cell() const
{
	return (*par_)->cell(idx_);
}


MathXArray & MathCursorPos::xcell(MathCursor::idx_type idx) const
{
	return (*par_)->xcell(idx);
}


MathXArray & MathCursorPos::xcell() const
{
	return (*par_)->xcell(idx_);
}


MathCursorPos MathCursor::normalAnchor() const
{
	// use Anchor on the same level as Cursor
	MathCursorPos normal = Anchor_[Cursor_.size() - 1];
	if (Cursor_.size() < Anchor_.size() && !(normal < cursor())) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos_;
	}
	return normal;
}


