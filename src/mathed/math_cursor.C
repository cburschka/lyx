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
#include "math_parser.h"

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
		if (i1.idx_ == i2.idx_) {
			MathArray ar;
			if (i1.inner_) {
				ar.push_back(*i1.at());
				ar.back().removeNucleus();
				++i1.pos_;
			}
			ar.push_back(MathArray(i1.cell(), i1.pos_, i2.pos_));
			if (i2.inner_) {
				ar.push_back(*i2.at());
				ar.back().removeUp();
				ar.back().removeDown();
			}
			data_.push_back(ar);
		}
		else {
			std::vector<MathInset::idx_type> indices =
				i1.par_->idxBetween(i1.idx_, i2.idx_);
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
			if (i1.inner_) {
				i1.inner_ = false;
				i1.at()->removeUp();
				i1.at()->removeDown();
				++i1.pos_;
			}
			if (i2.inner_) {
				i2.inner_ = false;
				i2.at()->removeNucleus();
			}
			i1.cell().erase(i1.pos_, i2.pos_);
		} else {
			std::vector<MathInset::idx_type> indices =
				i1.par_->idxBetween(i1.idx_, i2.idx_);
			for (unsigned i = 0; i < indices.size(); ++i)
				i1.cell(indices[i]).erase();
		}
		cursor.cursor() = i1;
	}

	void paste(MathCursor & cursor) const
	{
		cursor.insert(glue());
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
	os << "(par: " << p.par_ << " idx: " << p.idx_
	   << " pos: " << p.pos_ << " inner: " << p.inner_ << ")";
	return os;
}
#endif

}


MathCursor::MathCursor(InsetFormulaBase * formula)
	: formula_(formula), lastcode_(LM_TC_VAR), selection_(false)
{
	first();
}


void MathCursor::pushLeft(MathInset * par)
{
	MathCursorPos p;
	p.par_   = par;
	p.inner_ = false;
	par->idxFirst(p.idx_, p.pos_);
	Cursor_.push_back(p);
}


void MathCursor::pushRight(MathInset * par)
{
	posLeft();
	MathCursorPos p;
	p.par_   = par;
	p.inner_ = false;
	par->idxLast(p.idx_, p.pos_);
	Cursor_.push_back(p);
}


bool MathCursor::popLeft()
{
	if (Cursor_.size() <= 1)
		return false;
	if (nextAtom())
		nextAtom()->removeEmptyScripts();
	Cursor_.pop_back();
	if (nextAtom())
		nextAtom()->removeEmptyScripts();
	return true;
}


bool MathCursor::popRight()
{
	if (Cursor_.size() <= 1)
		return false;
	if (nextAtom())
		nextAtom()->removeEmptyScripts();
	Cursor_.pop_back();
	if (nextAtom())
		nextAtom()->removeEmptyScripts();
	posRight();
	return true;
}



#if FILEDEBUG
void MathCursor::dump(char const *) const
{
	lyxerr << "MC: " << what << "\n";
	for (unsigned i = 0; i < Cursor_.size(); ++i)
		lyxerr << "  i: " << i 
			<< " pos: " << Cursor_[i].pos_
			<< " idx: " << Cursor_[i].idx_
			<< " par: " << Cursor_[i].par_ << "\n";
	//lyxerr	<< " sel: " << selection_ << " data: " << array() << "\n";
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
		if (Cursor_[i].par_ == p) 
			return true;
	return false;
}


bool MathCursor::openable(MathInset * p, bool sel) const
{
	if (!p)
		return false;

	if (!p->isActive())
		return false;

	if (sel) {
		// we can't move into anything new during selection
		if (Cursor_.size() == Anchor_.size())
			return false;
		if (p != Anchor_[Cursor_.size()].par_)
			return false;
	}
	return true;
}


MathInset * MathCursor::positionable(MathAtom * t, int x, int y) const
{
	if (!t)
		return 0;

	if (selection_) {
		// we can't move into anything new during selection
		if (Cursor_.size() == Anchor_.size())
			return 0;
		//if (t != Anchor_[Cursor_.size()].par_)
		//	return 0;
	}

	MathInset * p;

	p = t->nucleus();
	if (p && p->nargs() && p->covers(x, y))
		return p;

	p = t->up();
	if (p && p->nargs() && p->covers(x, y))
		return p;

	p = t->down();
	if (p && p->nargs() && p->covers(x, y))
		return p;

	return 0;
}


bool MathCursor::posLeft()
{
	if (inner()) {
		inner() = false;
		return true;
	}

	if (pos() == 0)
		return false;

	--pos();

	if (nextAtom()->hasInner())
		inner() = true;

	return true;
}


bool MathCursor::posRight()
{
	if (inner()) {
		++pos();
		inner() = false;
		return true;
	}

	if (pos() == size())
		return false;

	if (nextAtom()->hasInner())
		inner() = true;
	else 
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

	MathInset * p = prevInset();
	if (openable(p, sel)) {
		pushRight(p);
		return true;
	} 
	
	return posLeft() || idxLeft() || popLeft();
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

	MathInset * p = nextInset();
	if (openable(p, sel)) {
		pushLeft(p);
		return true;
	}

	return posRight() || idxRight() || popRight();
}


void MathCursor::first()
{
	Cursor_.clear();
	pushLeft(outerPar());
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

	cursor().par_  = outerPar();

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
			int xx = abs(x1 - ar.pos2x(c, false));
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
		if (MathInset * p = positionable(nextAtom(), x, y))
			pushLeft(p);
		else if (MathInset * p = positionable(prevAtom(), x, y))
			pushRight(p);
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


void MathCursor::plainInsert(MathInset * p)
{
	if (inner()) {
		array().insert(pos(), p);
		++pos();
		swap(prevAtom()->nucleus(), nextAtom()->nucleus());
		return;
	}

	MathAtom * n = nextAtom();

	if (n && !n->nucleus()) {
		n->nucleus() = p;
		inner() = true;
		return;
	}

	array().insert(pos(), p); // this invalidates the pointer!
	++pos();
}


void MathCursor::insert(char c, MathTextCodes t)
{
	//lyxerr << "inserting '" << c << "'\n";
	plainInsert(new MathCharInset(c, t));
}


void MathCursor::insert(MathInset * p)
{
	macroModeClose();

	if (p && selection_) {
		if (p->nargs())
			selCut();
		else
			selDel();
	}

	plainInsert(p);
}


void MathCursor::niceInsert(MathInset * p) 
{
	if (!p) {
		lyxerr << "should not happen\n";
		return;
	}
	selCut();
	//cerr << "\n2: "; p->write(cerr, true); cerr << "\n";
	insert(p); // inserting invalidates the pointer!
	p = prevAtom()->nucleus();
	//cerr << "\n3: "; p->write(cerr, true); cerr << "\n";
	if (p->nargs()) {
		posLeft();
		right();  // do not push for e.g. MathSymbolInset
		selPaste();
	}
	p->metrics(p->size());
}


void MathCursor::insert(MathArray const & ar)
{
	macroModeClose();
	if (selection_)
		selCut();

	array().insert(pos(), ar);
	pos() += ar.size();
}


void MathCursor::glueAdjacentAtoms()
{
	MathAtom * p = prevAtom();
	if (!p)
		return;

	MathAtom * n = nextAtom();
	if (!n)
		return;

	if (p->up() && n->up())
		return;

	if (p->down() && n->down())
		return;

	// move everything to the previous atom
	if (n->up())
		swap(p->up(), n->up());

	if (n->down())
		swap(p->down(), n->down());
	
	plainErase();
	--pos();
	inner() = nextAtom()->hasInner();
}


void MathCursor::backspace()
{
	if (inner()) {
		nextAtom()->removeNucleus();
		inner() = false;
		glueAdjacentAtoms();
		return;
	}

	if (pos() == 0) {
		if (size())
			pullArg(false);
		return;
	}	

	if (prevAtom()->hasInner()) {
		--pos();
		inner() = true;
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

	MathAtom * n = nextAtom();

	if (!n)
		return;

	if (inner()) {
		if (n->up())
			n->removeUp();
		else if (n->down())
			n->removeDown();
		if (!n->up() && !n->down()) {
			++pos();
			inner() = false;
		}
		return;
	}

	if (n->hasInner()) {
		n->removeNucleus();
		inner() = true;
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
}


bool MathCursor::up(bool sel)
{
	dump("up 1");
	macroModeClose();
	selHandle(sel);

	if (selection_)
		return goUp();

	// check whether we could move into a superscript on the right or on the left
	if (prevAtom() && prevAtom()->up()) {
		pushRight(prevAtom()->up());		
		return true;
	}

	if (nextAtom() && nextAtom()->up()) {
		pushLeft(nextAtom()->up());		
		return true;
	}

	return goUp();
}


bool MathCursor::down(bool sel)
{
	dump("down 1");
	macroModeClose();
	selHandle(sel);

	if (selection_) 
		return goDown();

	// check whether we could move into an subscript on the right or on the left
	if (prevAtom() && prevAtom()->down()) {
		pushRight(prevAtom()->down());		
		return true;
	}

	if (nextAtom() && nextAtom()->down()) {
		pushLeft(nextAtom()->down());	
		return true;
	}

	return goDown();
}


bool MathCursor::toggleLimits()
{
	MathAtom * t = prevAtom();
	if (!t)
		return false;
	int old = t->limits();
	t->limits(old < 0 ? 1 : -1);
	return old != t->limits();
}


void MathCursor::setSize(MathStyles size)
{
	par()->userSetSize(size);
}


void MathCursor::macroModeClose()
{
	string s = macroName();
	if (s.size()) {
		pos() = pos() - s.size() - 1;
		for (unsigned i = 0; i <= s.size(); ++i)
			plainErase();
		lastcode_ = LM_TC_VAR;
		interpret("\\" + s);
	}
}


string MathCursor::macroName() const
{
	string s;
	for (int i = pos() - 1; i >= 0; --i) {
		MathInset * p = array().at(i)->nucleus();
		if (!p || p->code() != LM_TC_TEX || p->getChar() == '\\')
			break;
		s = p->getChar() + s;
	}
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
		selClear();
	}
}


void MathCursor::selPaste()
{
	seldump("selPaste");
	theSelection.paste(*this);
	selClear();
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
		int x1 = c.xo() + c.pos2x(i1.pos_, i1.inner_);
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(i2.pos_, i2.inner_);
		int y2 = c.yo() + c.descent();
		pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		std::vector<MathInset::idx_type> indices = i1.par_->idxBetween(i1.idx_, i2.idx_);
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


MathTextCodes MathCursor::nextCode() const
{
	//return (pos() == size()) ? LM_TC_VAR : nextInset()->code();
	return LM_TC_VAR;
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
			for (MathInset::pos_type pos = i1.pos_;
			     pos != i2.pos_; ++pos) {
				MathInset * p = ar.at(pos)->nucleus();
				if (p)
					p->handleFont(t);
			}
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
	insert(p); // this invalidates p!
	p = prevAtom()->nucleus();	
	pushRight(p);
}


void MathCursor::getPos(int & x, int & y)
{
#ifdef WITH_WARNINGS
#warning This should probably take cellXOffset and cellYOffset into account
#endif
	x = xarray().xo() + xarray().pos2x(pos(), inner());
	y = xarray().yo();
}


MathInset * MathCursor::par() const
{
	return cursor().par_;
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


bool MathCursor::inner() const
{
	return cursor().inner_;
}


bool & MathCursor::inner()
{
	return cursor().inner_;
}


bool MathCursor::inMacroMode() const
{
	return lastcode_ == LM_TC_TEX;
}


bool MathCursor::selection() const
{
	return selection_;
}


MathArrayInset * MathCursor::enclosingArray(MathCursor::idx_type & idx) const
{
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		if (Cursor_[i].par_->isArray()) {
			idx = Cursor_[i].idx_;
			return static_cast<MathArrayInset *>(Cursor_[i].par_);
		}
	}
	return 0;
}


void MathCursor::pullArg(bool goright)
{
	// pullArg
	dump("pullarg");
	MathArray a = array();
	if (popLeft()) {
		plainErase();
		array().insert(pos(), a);
		if (goright) 
			pos() += a.size();
	}
}


MathStyles MathCursor::style() const
{
	return xarray().style();
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
		       << pos() << " " << size() << "\n";
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


/*
char MathCursorPos::getChar() const
{
	return array().getChar(pos());
}


string MathCursorPos::readString()
{
	string s;
	int code = nextCode();
	for ( ; OK() && nextCode() == code; Next()) 
		s += getChar();

	return s;
}
*/


MathInset * MathCursor::prevInset() const
{
	return prevAtom() ? prevAtom()->nucleus() : 0;
}


MathInset * MathCursor::nextInset() const
{
	return nextAtom() ? nextAtom()->nucleus() : 0;
}


MathSpaceInset * MathCursor::prevSpaceInset() const
{
	MathInset * p = prevInset();
	return (p && p->isSpaceInset()) ? static_cast<MathSpaceInset *>(p) : 0;
}


MathAtom const * MathCursor::prevAtom() const
{
	return array().at(pos() - 1);
}


MathAtom * MathCursor::prevAtom()
{
	return array().at(pos() - 1);
}


MathAtom const * MathCursor::nextAtom() const
{
	return array().at(pos());
}


MathAtom * MathCursor::nextAtom()
{
	return array().at(pos());
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;
	if (!par()) {
		lyxerr << "############  par_ not valid\n";
		return dummy;
	}

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

	MathMatrixInset * p = outerPar();
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
			lyxerr << "swapping cells " << i1 << " and " << i2 << "\n";
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
	return cellXOffset() + xarray().pos2x(pos(), inner());
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
	if (par()->asScriptInset() && par()->asScriptInset()->down()) {
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
	for (int y = y0 - 4; y > outerPar()->yo() - outerPar()->ascent(); y -= 4) {
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
	if (par()->asScriptInset() && par()->asScriptInset()->up()) {
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
	for (int y = y0 + 4; y < outerPar()->yo() + outerPar()->descent(); y += 4) {
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


MathMatrixInset * MathCursor::outerPar() const
{
	return
		static_cast<MathMatrixInset *>(const_cast<MathInset *>(formula_->par()));
}


void MathCursor::interpret(string const & s)
{
	//lyxerr << "interpret: '" << s << "'\n";
	//lyxerr << "in: " << in_word_set(s) << " \n";

	if (s.empty())
		return;

	char c = s[0];

	//lyxerr << "char: '" << c << "'  int: " << int(c) << endl;
	//owner_->getIntl()->getTrans().TranslateAndInsert(c, lt);	
	//lyxerr << "trans: '" << c << "'  int: " << int(c) << endl;

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
		MathArrayInset * p = new MathArrayInset(m, n);
		p->valign(v_align[0]);
		p->halign(h_align);
		niceInsert(p);
		return;
	}

	if (s == "\\over" || s == "\\choose" || s == "\\atop") {
		MathArray ar = array();
		MathInset * p = createMathInset(s.substr(1));
		p->cell(0).swap(array());
		pos() = 0;
		niceInsert(p);
		popRight();
		left();
		return;
	}

	if (s.size() > 1) {
		niceInsert(createMathInset(s.substr(1)));
		return;
	}


	// we got just a single char now

	if (c == '^' || c == '_') {
		const bool up = (s[0] == '^');
		selCut();
		if (inner())
			++pos();
		if (!prevAtom())
			insert(0);
		MathInset * par = prevAtom()->ensure(up);
		pushRight(par);
		selPaste();
		return;
	}

	if (selection_)
		selDel();

	if (lastcode_ == LM_TC_TEXTRM) {
		insert(c, LM_TC_TEXTRM);
		return;
	}

	if (c == ' ') {
		if (inMacroMode()) {
			macroModeClose();
			lastcode_ = LM_TC_VAR;
			return;
		}

		MathSpaceInset * p = prevSpaceInset();
		if (p) {
			p->incSpace();
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

	if (lastcode_ != LM_TC_TEX && strchr("{}", c)) {
		insert(c, LM_TC_TEX);
		return;
	}

	if (lastcode_ != LM_TC_TEX && strchr("#$%", c)) {
		insert(new MathSpecialCharInset(c));	
		lastcode_ = LM_TC_VAR;
		return;
	}

	if (lastcode_ == LM_TC_TEX) {
		if (macroName().empty()) {
			insert(c, LM_TC_TEX);
			if (!isalpha(c) && c != '#') {
				macroModeClose();
				lastcode_ = LM_TC_VAR;
			}
		} else {
			if ('1' <= c && c <= '9' && macroName() == "#") {
				insert(c, LM_TC_TEX);
				macroModeClose();
				lastcode_ = LM_TC_VAR;
			}
			else if (isalpha(c)) {
				insert(c, LM_TC_TEX);
			}
			else {
				macroModeClose();
				lastcode_ = LM_TC_VAR;
			}
		}
		return;
	}

	if (isalpha(c) && (lastcode_ == LM_TC_GREEK || lastcode_ == LM_TC_GREEK1)) {
		static char const greekl[][26] =
			{"alpha", "beta", "chi", "delta", "epsilon", "phi",
			 "gamma", "eta", "iota", "iota", "kappa", "lambda", "mu",
			 "nu", "omikron", "pi", "omega", "rho", "sigma",
			 "tau", "upsilon", "theta", "omega", "xi", "upsilon", "zeta"};
		static char const greeku[][26] =
			{"Alpha", "Beta", "Chi", "Delta", "Epsilon", "Phi",
			 "Gamma", "Eta", "Iota", "Iota", "Kappa", "Lambda", "Mu",
			 "Nu", "Omikron", "Pi", "Omega", "Rho", "Sigma", "Tau",
			 "Upsilon", "Theta", "Omega", "xi", "Upsilon", "Zeta"};
	
		latexkeys const * l = 0;	
		if ('a' <= c && c <= 'z')
			l = in_word_set(greekl[c - 'a']);
		if ('A' <= c && c <= 'Z')
			l = in_word_set(greeku[c - 'A']);
	
		if (l)
			insert(createMathInset(l));
		else
			insert(c, LM_TC_VAR);

#warning greek insert problem? look here!
		if (lastcode_ == LM_TC_GREEK1)
			lastcode_ = LM_TC_VAR;
		return;	
	}

	if (c == '\\') {
		lastcode_ = LM_TC_TEX;
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
	return par_->cell(idx);
}


MathArray & MathCursorPos::cell() const
{
	return par_->cell(idx_);
}


MathXArray & MathCursorPos::xcell(MathCursor::idx_type idx) const
{
	return par_->xcell(idx);
}


MathXArray & MathCursorPos::xcell() const
{
	return par_->xcell(idx_);
}


MathAtom * MathCursorPos::at() const
{
	return cell().at(pos_);
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

