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
#include "math_decorationinset.h"
#include "math_deliminset.h"
#include "math_funcinset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_matrixinset.h"
#include "math_scopeinset.h"
#include "math_scriptinset.h"
#include "math_spaceinset.h"
#include "math_parser.h"

using std::endl;
using std::min;
using std::max;
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
			std::vector<int> indices = i1.par_->idxBetween(i1.idx_, i2.idx_);
			for (unsigned i = 0; i < indices.size(); ++i)
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
			std::vector<int> indices = i1.par_->idxBetween(i1.idx_, i2.idx_);
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


std::ostream & operator<<(std::ostream & os, MathCursorPos const & p)
{
	os << "(par: " << p.par_ << " idx: " << p.idx_
	   << " pos: " << p.pos_ << ")";
	return os;
}

}


MathCursor::MathCursor(InsetFormulaBase * formula)
	: formula_(formula), lastcode_(LM_TC_VAR), imacro_(0), selection_(false)
{
	first();
}


MathCursor::~MathCursor()
{
	delete imacro_;
}


void MathCursor::pushLeft(MathInset * par)
{
	MathCursorPos p;
	p.par_ = par;
	par->idxFirst(p.idx_, p.pos_);
	Cursor_.push_back(p);
}


void MathCursor::pushRight(MathInset * par)
{
	posLeft();
	MathCursorPos p;
	p.par_ = par;
	par->idxLast(p.idx_, p.pos_);
	Cursor_.push_back(p);
}


bool MathCursor::popLeft()
{
	if (Cursor_.size() <= 1)
		return false;
	Cursor_.pop_back();
	return true;
}

bool MathCursor::popRight()
{
	if (Cursor_.size() <= 1)
		return false;
	Cursor_.pop_back();
	posRight();
	return true;
}


MathInset * MathCursor::parInset(int i) const
{
	return Cursor_[i].par_;
}


void MathCursor::dump(char const * what) const
{
	return;

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
	return;

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


bool MathCursor::isInside(MathInset const * p) const
{
	for (unsigned i = 0; i < Cursor_.size(); ++i) 
		if (parInset(i) == p) 
			return true;
	return false;
}


bool MathCursor::openable(MathInset * p, bool sel, bool useupdown) const
{
	if (!p)
		return false;

	if (!(p->isActive() || (useupdown && p->isScriptInset())))
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
	if (imacro_) {
		// was MacroModeBack()
		if (!imacro_->name().empty()) {
			imacro_->setName(imacro_->name().substr(0, imacro_->name().length()-1));
			imacro_->metrics(imacro_->size());
		} else
			macroModeClose();
		return true;
	}
	selHandle(sel);
	lastcode_ = LM_TC_VAR;

	MathInset * p = prevInset();
	if (openable(p, sel, false)) {
		pushRight(p);
		return true;
	} 
	
	return posLeft() || idxLeft() || popLeft();
}


bool MathCursor::right(bool sel)
{
	dump("Right 1");
	if (imacro_) {
		macroModeClose();
		return true;
	}
	selHandle(sel);
	lastcode_ = LM_TC_VAR;

	MathInset * p = nextInset();
	if (openable(p, sel, false)) {
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
	dump("setPos 1");
	//lyxerr << "MathCursor::setPos x: " << x << " y: " << y << "\n";

	macroModeClose();
	lastcode_ = LM_TC_VAR;
	first();

	cursor().par_  = outerPar();

	while (1) {
		idx() = -1;
		cursor().pos_ = -1;
		//lyxerr << "found idx: " << idx_ << " cursor: " << pos()  << "\n";
		int distmin = 1 << 30; // large enough
		for (int i = 0; i < par()->nargs(); ++i) {
			MathXArray const & ar = par()->xcell(i);
			int x1 = x - ar.xo();
			int y1 = y - ar.yo();
			int c  = ar.x2pos(x1);
			int xx = abs(x1 - ar.pos2x(c));
			int yy = abs(y1);
			//lyxerr << "idx: " << i << " xx: " << xx << " yy: " << yy
			//	<< " c: " << c  << " xo: " << ar.xo() << "\n";
			if (yy + xx <= distmin) {
				distmin        = yy + xx;
				idx()  = i;
				pos()  = c;
			}
		}
		//lyxerr << "found idx: " << idx() << " cursor: "
		//	<< pos()  << "\n";
		MathInset * n = nextInset();
		MathInset * p = prevInset();
		if (openable(n, selection_, true) && n->covers(x, y))
			pushLeft(n);
		else if (openable(p, selection_, true) && p->covers(x, y))
			pushRight(p);
		else 
			break;
	}
	dump("setPos 2");
}


void MathCursor::home()
{
	dump("home 1");
	macroModeClose();
	lastcode_ = LM_TC_VAR;
	if (!par()->idxHome(idx(), pos())) 
		popLeft();
	dump("home 2");
}


void MathCursor::end()
{
	dump("end 1");
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


void MathCursor::insert(char c, MathTextCodes t)
{
	//lyxerr << "inserting '" << c << "'\n";
	if (selection_)
		selDel();

	if (t != LM_TC_VAR)
		lastcode_ = t;

	if (imacro_ && !(MathIsAlphaFont(t) || t == LM_TC_VAR))
		macroModeClose();

	if (imacro_) {
		if (MathIsAlphaFont(t) || t == LM_TC_VAR) {
			// was MacroModeinsert(c);
			imacro_->setName(imacro_->name() + c);
			return;
		}
	}

	array().insert(pos(), new MathCharInset(c, t));
	posRight();
}


void MathCursor::insert(MathInset * p)
{
	macroModeClose();

	if (selection_) {
		if (p->nargs())
			selCut();
		else
			selDel();
	}

	array().insert(pos(), p);
	posRight();
}


void MathCursor::niceInsert(MathInset * p) 
{
	if (!p) {
		lyxerr << "should not happen\n";
		return;
	}
	selCut();
	insert(p);
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


void MathCursor::backspace()
{
	if (inMacroMode()) {
		left();
		return;
	}

	if (posLeft()) {
		plainErase();
		return;
	}

	if (size()) {
		pullArg(false);
		return;
	}

	erase();
}


void MathCursor::erase()
{
	dump("erase 1");
	if (imacro_)
		return;

	if (selection_) {
		selDel();
		return;
	}

	// delete empty cells if necessary
	if (pos() == 0 && array().empty()) {
		bool popit;
		bool removeit;
		par()->idxDelete(idx(), popit, removeit);
		if (popit && popLeft() && removeit)
			plainErase();
		return;
	}

	if (pos() < size())
		plainErase();

	dump("erase 2");
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

	// check whether we could move into an inset on the right or on the left
	MathInset * p = nextInset();
	if (p) {
		int idxx, poss;
		if (p->idxFirstUp(idxx, poss)) {
			pushLeft(p);
			idx() = idxx;
			pos() = poss;
			dump("up 3");
			return true;
		}
	}

	p = prevInset();
	if (p) {
		int idxx, poss;
		if (p->idxLastUp(idxx, poss)) {
			pushRight(p);
			idx() = idxx;
			pos() = poss;
			dump("up 4");
			return true;
		}
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

	// check whether we could move into an inset on the right or on the left
	MathInset * p = nextInset();
	if (p) {
		int idxx = 0;
		int poss = 0;
		if (p->idxFirstDown(idxx, poss)) {
			pushLeft(p);
			idx() = idxx;
			pos() = poss;
			dump("Down 3");
			return true;
		}
	}

	p = prevInset();
	if (p) {
		int idxx = 0;
		int poss = 0;
		if (p->idxLastDown(idxx, poss)) {
			pushRight(p);
			idx() = idxx;
			pos() = poss;
			dump("Down 4");
			return true;
		}
	}

	return goDown();
}


bool MathCursor::toggleLimits()
{
	MathScriptInset * p = prevScriptInset();
	if (!p)
		return false;
	int old = p->limits();
	p->limits(old < 0 ? 1 : -1);
	return old != p->limits();
}


void MathCursor::setSize(MathStyles size)
{
	par()->userSetSize(size);
}



void MathCursor::macroModeOpen()
{
	if (!imacro_) {
		imacro_ = new MathFuncInset("");
		array().insert(pos(), imacro_);
		++pos();
		//insert(imacro_);
	} else
		lyxerr << "Math Warning: Already in macro mode" << endl;
}


void MathCursor::macroModeClose()
{
	if (imacro_) {
		string name = imacro_->name();
		posLeft();
		plainErase();
		imacro_ = 0;
		interpret("\\" + name);
	}
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
		int x1 = c.xo() + c.pos2x(i1.pos_);
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(i2.pos_);
		int y2 = c.yo() + c.descent();
		pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		std::vector<int> indices = i1.par_->idxBetween(i1.idx_, i2.idx_);
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
			for (int pos = i1.pos_; pos != i2.pos_; ++pos) 
				ar.nextInset(pos)->handleFont(t);
		}
	} else 
		lastcode_ = (lastcode_ == t) ? LM_TC_VAR : t;
}


void MathCursor::handleAccent(string const & name)
{
	latexkeys const * l = in_word_set(name);
	if (l)
		handleNest(new MathDecorationInset(l));
}


void MathCursor::handleDelim(latexkeys const * l, latexkeys const * r)
{
	handleNest(new MathDelimInset(l, r));
}


void MathCursor::handleNest(MathInset * p)
{
	if (selection_) {
		selCut();
		p->cell(0) = theSelection.glue();
	}
	insert(p);
	pushRight(p);
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


InsetFormulaBase const * MathCursor::formula()
{
	return formula_;
}


int MathCursor::idx() const
{
	return cursor().idx_;
}


int & MathCursor::idx()
{
	return cursor().idx_;
}


int MathCursor::pos() const
{
	return cursor().pos_;
}


int & MathCursor::pos()
{
	return cursor().pos_;
}


bool MathCursor::inMacroMode() const
{
	return imacro_;
}


bool MathCursor::selection() const
{
	return selection_;
}


MathArrayInset * MathCursor::enclosingArray(int & idx) const
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

	if (idx() < 0)
		lyxerr << "this should not really happen - 1: " << idx() << "\n";
 	if (idx() >= par()->nargs()) {
		lyxerr << "this should not really happen - 2: "
		       << idx() << " " << par()->nargs() << "\n";
		dump("error 2");
	}
	it->idx()    = max(idx(), 0);
 	it->idx()    = min(idx(), par()->nargs() - 1);

	if (pos() < 0)
		lyxerr << "this should not really happen - 3: " << pos() << "\n";
	if (pos() > size()) {
		lyxerr << "this should not really happen - 4: "
		       << pos() << " " << size() << "\n";
		dump("error 4");
	}
	it->pos() = max(pos(), 0);
	it->pos() = min(pos(), size());
}


int MathCursor::size() const
{
	return array().size();
}


int MathCursor::col() const
{
	return par()->col(idx());
}


int MathCursor::row() const
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
	normalize();
	if (!pos())
		return 0;
	return array().nextInset(pos() - 1);
}


MathInset * MathCursor::nextInset() const
{
	normalize();
	return array().nextInset(pos());
}


MathScriptInset * MathCursor::prevScriptInset() const
{
	normalize();
	MathInset * p = prevInset();
	return (p && p->isScriptInset()) ? static_cast<MathScriptInset *>(p) : 0;
}


MathSpaceInset * MathCursor::prevSpaceInset() const
{
	normalize();
	MathInset * p = prevInset();
	return (p && p->isSpaceInset()) ? static_cast<MathSpaceInset *>(p) : 0;
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;
	if (!par()) {
		lyxerr << "############  par_ not valid\n";
		return dummy;
	}

	if (idx() < 0 || idx() >= par()->nargs()) {
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
		const int r = row();
		for (int c = col() + 1; c < p->ncols(); ++c) {
			const int i1 = p->index(r, c);
			const int i2 = p->index(r + 1, c);	
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
	int idx;
	MathArrayInset * p = enclosingArray(idx);
	return p ? p->valign() : 0;
}


char MathCursor::halign() const
{
	int idx;
	MathArrayInset * p = enclosingArray(idx);
	return p ? p->halign(idx % p->ncols()) : 0;
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


MathArray & MathCursorPos::cell(int idx) const
{
	return par_->cell(idx);
}


MathArray & MathCursorPos::cell() const
{
	return par_->cell(idx_);
}


MathXArray & MathCursorPos::xcell(int idx) const
{
	return par_->xcell(idx);
}


MathXArray & MathCursorPos::xcell() const
{
	return par_->xcell(idx_);
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


void MathCursor::interpret(string const & s)
{
	//lyxerr << "interpret: '" << s << "'\n";
	//lyxerr << "in: " << in_word_set(s) << " \n";

	if (s.empty())
		return;

	char c = s[0];

	lyxerr << "char: '" << c << "'  int: " << int(c) << endl;
	//owner_->getIntl()->getTrans().TranslateAndInsert(c, lt);	
	//lyxerr << "trans: '" << c << "'  int: " << int(c) << endl;

	latexkeys const * l = in_word_set(s.substr(1));
	if (l) {
		lastcode_ = LM_TC_VAR;
		niceInsert(createMathInset(l));
		return;
	}

	if (MathMacroTable::hasTemplate(s.substr(1))) {
		niceInsert(new MathMacro(MathMacroTable::provideTemplate(s.substr(1))));
		return;
	}

	if (s.size() > 8 && s.substr(0, 8) == "\\matrix ") {
		int m = 1;
		int n = 1;
		string v_align;
		string h_align;
		istringstream is(s.substr(8).c_str());
		is >> m >> n >> v_align >> h_align;
		m = std::max(1, m);
		n = std::max(1, n);
		v_align += 'c';
		MathArrayInset * pp = new MathArrayInset(m, n);
		pp->valign(v_align[0]);
		pp->halign(h_align);
		niceInsert(pp);
		return;
	}

	if (s.size() > 1)
		return niceInsert(new MathFuncInset(s.substr(1)));


	// we got just a single char now

	if (c == '^' || c == '_') {
		bool const up = (s[0] == '^');
		selCut();	
		MathScriptInset * p = prevScriptInset();
		if (!p) {
			MathInset * b = prevInset();
			if (b && b->isScriptable()) {
				p = new MathScriptInset(up, !up, b->clone());
				posLeft();
				plainErase();
			} else 
				p = new MathScriptInset(up, !up);
			insert(p);
		}
		pushRight(p);
		if (up)
			p->up(true);
		else
			p->down(true);
		idx() = up ? 0 : 1;
		pos() = 0;
		selPaste();
		return;
	}

	if (c == '{') {
		niceInsert(new MathScopeInset);
		return;
	}

	if (isalpha(c) && (lastcode_ == LM_TC_GREEK || lastcode_ == LM_TC_GREEK1)) {
		static char const greek[26] =
			{'A', 'B', 'X',  0 , 'E',  0 ,  0 , 'H', 'I',  0 ,
			 'K',  0 , 'M', 'N', 'O',  0 ,  0 , 'P',  0 , 'T',
			 0,  0,   0,   0,   0 , 'Z' };
			
		MathTextCodes code = LM_TC_SYMB;
		if ('A' <= c && c <= 'Z' && greek[c - 'A']) {
			code = LM_TC_RM;
			c = greek[c - 'A'];
		}
		insert(c, code);
	
#warning greek insert problem? look here!
		//if (lastcode_ == LM_TC_GREEK1)
			lastcode_ = LM_TC_VAR;
		return;	
	}

	if (c == '_' && lastcode_ == LM_TC_TEX) {
		lastcode_ = LM_TC_VAR;
		insert(c, LM_TC_SPECIAL);
		return;
	}

	if ('0' <= c && c <= '9' && (lastcode_ == LM_TC_TEX || imacro_)) {
		macroModeOpen();
		lastcode_ = LM_TC_VAR;
		insert(c, lastcode_);
		return;
	}

	if (('0' <= c && c <= '9') || strchr(";:!|[]().,?", c)) {
		if (lastcode_ != LM_TC_TEXTRM)
			lastcode_ = LM_TC_CONST;
		insert(c, lastcode_);
		return;
	}

	if (strchr("+/-*<>=", c)) {
		if (lastcode_ != LM_TC_TEXTRM)
			lastcode_ = LM_TC_BOP;
		insert(c, lastcode_);
		return;
	}

	if (strchr("#$%{|}", c)) {
		if (lastcode_ != LM_TC_TEXTRM)
			lastcode_ = LM_TC_SPECIAL;
		insert(c, lastcode_);
		return;
	}

	if (c == ' ') {
		if (imacro_) {
			lastcode_ = LM_TC_VAR;
			macroModeClose();
			return;
		}

		if (lastcode_ == LM_TC_TEXTRM) {
			insert(c, LM_TC_TEXTRM);
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

	if (c == '\'' || c == '@') {
		insert(c, LM_TC_VAR);
		return;
	}

	if (c == '\\') {
		if (imacro_)
			macroModeClose();
		//bv->owner()->message(_("TeX mode"));
		lastcode_ = LM_TC_TEX;
		return;	
	}

	if (isalpha(c)) {
		if (lastcode_ == LM_TC_TEX) {
			macroModeOpen();
			lastcode_ = LM_TC_VAR;
		}
		insert(c, lastcode_);
		return;	
	}

}

