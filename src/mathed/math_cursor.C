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

#include "debug.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "formulabase.h"
#include "math_cursor.h"
#include "math_arrayinset.h"
#include "math_bigopinset.h"
#include "math_decorationinset.h"
#include "math_deliminset.h"
#include "math_dotsinset.h"
#include "math_fracinset.h"
#include "math_funcinset.h"
#include "math_gridinset.h"
#include "math_macro.h"
#include "math_macroarg.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_matrixinset.h"
#include "math_rootinset.h"
#include "math_spaceinset.h"
#include "math_sqrtinset.h"
#include "support/lstrings.h"
#include "math_scriptinset.h"
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
#ifdef RECTANGULAR_SELECT
			std::vector<int> indices = i1.par_->idxBetween(i1.idx_, i2.idx_);
			for (unsigned i = 0; i < indices.size(); ++i)
				data_.push_back(i1.cell(indices[i]));
#else
			data_.push_back(MathArray(i1.cell(), i1.pos_, i1.cell().size()));
			for (int i = i1.idx_ + 1; i < i2.idx_; ++i)
				data_.push_back(i1.cell(i));
			data_.push_back(MathArray(i2.cell(), 0, i2.pos_));
#endif
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
	: formula_(formula)
{
	lastcode   = LM_TC_MIN;
	macro_mode = false;
	selection  = false;
	first();
}


void MathCursor::push(MathInset * par, bool first)
{
	MathCursorPos p;
	p.par_ = par;
	if (first)
		par->idxFirst(p.idx_, p.pos_);
	else
		par->idxLast(p.idx_, p.pos_);
	Cursor_.push_back(p);
}


bool MathCursor::pop()
{
	if (Cursor_.size() <= 1)
		return false;
	Cursor_.pop_back();
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

	//lyxerr	<< " sel: " << selection << " data: " << array() << "\n";
}


void MathCursor::seldump(char const *) const
{
	//lyxerr << "SEL: " << str << ": '" << theSelection << "'\n";
	//dump("   Pos");
	return;

	//lyxerr << "\n\n\\n=================vvvvvvvvvvvvv=======================   "
	//	<<  str << "\ntheSelection: " << theSelection;
	//for (unsigned int i = 0; i < Cursor_.size(); ++i) 
	//	lyxerr << Cursor_[i].par_ << "\n'" << Cursor_[i].cell() << "'\n";
	//lyxerr << "\ncursor.pos_: " << cursor().pos_;
	//lyxerr << "\nanchor.pos_: " << anchor().pos_;
	//lyxerr << "\n===================^^^^^^^^^^^^=====================\n\n\n";
}


bool MathCursor::isInside(MathInset * p) const
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


bool MathCursor::plainLeft()
{
	return array().prev(cursor().pos_);
}


bool MathCursor::Left(bool sel)
{
	dump("Left 1");
	if (macro_mode) {
		// was MacroModeBack()
		if (!imacro->name().empty()) {
			imacro->SetName(imacro->name().substr(0, imacro->name().length()-1));
			imacro->Metrics(imacro->size());
		} else
			MacroModeClose();
		return true;
	}
	SelHandle(sel);
	clearLastCode();

	MathInset * p = prevInset();
	if (openable(p, sel, false)) {
		plainLeft();
		push(p, false);
		return true;
	} 
	if (plainLeft())
		return true;
	if (cursor().par_->idxLeft(cursor().idx_, cursor().pos_))
		return true;
	if (pop())
		return true;
	return false;
}


bool MathCursor::plainRight()
{
	return array().next(cursor().pos_);
}


bool MathCursor::Right(bool sel)
{
	dump("Right 1");
	if (macro_mode) {
		MacroModeClose();
		return true;
	}
	SelHandle(sel);
	clearLastCode();

	MathInset * p = nextInset();
	if (openable(p, sel, false)) {
		push(p, true);
		return true;
	}
	if (array().next(cursor().pos_))
		return true;
	if (cursor().par_->idxRight(cursor().idx_, cursor().pos_))
		return true;
	if (!pop())
		return false;
	array().next(cursor().pos_);
	return true;
}


void MathCursor::first()
{
	Cursor_.clear();
	push(formula_->par(), true);
}


void MathCursor::last()
{
	Cursor_.clear();
	push(formula_->par(), false);
}


void MathCursor::SetPos(int x, int y)
{
	dump("SetPos 1");
	//lyxerr << "MathCursor::SetPos x: " << x << " y: " << y << "\n";

	MacroModeClose();
	lastcode = LM_TC_MIN;
	first();

	cursor().par_  = formula()->par();

	while (1) {
		cursor().idx_ = -1;
		cursor().pos_ = -1;
		//lyxerr << "found idx: " << idx_ << " cursor: " << cursor().pos_  << "\n";
		int distmin = 1 << 30; // large enough
		for (int i = 0; i < cursor().par_->nargs(); ++i) {
			MathXArray const & ar = cursor().par_->xcell(i);
			int x1 = x - ar.xo();
			int y1 = y - ar.yo();
			int c  = ar.x2pos(x1);
			int xx = abs(x1 - ar.pos2x(c));
			int yy = abs(y1);
			//lyxerr << "idx: " << i << " xx: " << xx << " yy: " << yy
			//	<< " c: " << c  << " xo: " << ar.xo() << "\n";
			if (yy + xx <= distmin) {
				distmin        = yy + xx;
				cursor().idx_  = i;
				cursor().pos_  = c;
			}
		}
		//lyxerr << "found idx: " << cursor().idx_ << " cursor: "
		//	<< cursor().pos_  << "\n";
		MathInset * n = nextInset();
		MathInset * p = prevInset();
		if (openable(n, selection, true) && n->covers(x, y))
			push(n, true);
		else if (openable(p, selection, true) && p->covers(x, y)) {
			plainLeft();
			push(p, false);
		} else 
			break;
	}
	dump("SetPos 2");
}


void MathCursor::Home()
{
	dump("Home 1");
	MacroModeClose();
	clearLastCode();
	if (!cursor().par_->idxHome(cursor().idx_, cursor().pos_)) 
		pop();
	dump("Home 2");
}


void MathCursor::End()
{
	dump("End 1");
	MacroModeClose();
	clearLastCode();
	if (!cursor().par_->idxEnd(cursor().idx_, cursor().pos_)) {
		pop();
		array().next(cursor().pos_);
	}
	dump("End 2");
}


void MathCursor::erase()
{
	array().erase(cursor().pos_);
}


void MathCursor::insert(char c, MathTextCodes t)
{
	//lyxerr << "inserting '" << c << "'\n";
	if (selection)
		SelDel();

	if (t == LM_TC_MIN)
		t = lastcode;

	if (macro_mode && !(MathIsAlphaFont(t) || t == LM_TC_MIN))
		MacroModeClose();

	if (macro_mode) {
		if (MathIsAlphaFont(t) || t == LM_TC_MIN) {
			// was MacroModeinsert(c);
			imacro->SetName(imacro->name() + static_cast<char>(c));
			return;
		}
	}

	array().insert(cursor().pos_, c, t);
	array().next(cursor().pos_);

	lastcode = t;
}


void MathCursor::insert(MathInset * p)
{
	MacroModeClose();

	if (selection) {
		if (p->nargs())
			SelCut();
		else
			SelDel();
	}

	array().insert(cursor().pos_, p);
	array().next(cursor().pos_);
}


void MathCursor::insert(MathArray const & ar)
{
	MacroModeClose();
	if (selection)
		SelCut();

	array().insert(cursor().pos_, ar);
	cursor().pos_ += ar.size();
}


void MathCursor::Delete()
{
	dump("Delete 1");
	if (macro_mode)
		return;

	if (selection) {
		SelDel();
		return;
	}

	// delete empty cells if necessary
	if (cursor().pos_ == 0 && array().size() == 0) {
		bool popit;
		bool removeit;
		cursor().par_->idxDelete(cursor().idx_, popit, removeit);
		if (popit && pop() && removeit)
			Delete();
		return;
	}

	if (cursor().pos_ < array().size())
		array().erase(cursor().pos_);

	dump("Delete 2");
}


void MathCursor::DelLine()
{
	MacroModeClose();

	if (selection) {
		SelDel();
		return;
	}

	if (cursor().par_->nrows() > 1)
		cursor().par_->delRow(row());
}


bool MathCursor::Up(bool sel)
{
	dump("Up 1");
	MacroModeClose();
	SelHandle(sel);

	if (selection) {
		int x = xarray().pos2x(cursor().pos_);
		if (cursor().idxDown()) {
			cursor().pos_ = xarray().x2pos(x);
			return true;
		}
		if (pop()) 
			return true;
		return false;
	}

	// check whether we could move into an inset on the right or on the left
	MathInset * p = nextInset();
	if (p) {
		int idx, pos;
		if (p->idxFirstUp(idx, pos)) {
			push(p, true);
			cursor().idx_ = idx;
			cursor().pos_ = pos;
			dump("Up 3");
			return true;
		}
	}

	p = prevInset();
	if (p) {
		int idx, pos;
		if (p->idxLastUp(idx, pos)) {
			plainLeft();
			push(p, false);
			cursor().idx_ = idx;
			cursor().pos_ = pos;
			dump("Up 4");
			return true;
		}
	}

	int x = xarray().pos2x(cursor().pos_);
	if (cursor().idxUp()) {
		cursor().pos_ = xarray().x2pos(x);
		return true;
	}
	if (pop())
		return true;
	return false;
}


bool MathCursor::Down(bool sel)
{
	dump("Down 1");
	MacroModeClose();
	SelHandle(sel);

	if (selection) {
		int x = xarray().pos2x(cursor().pos_);
		if (cursor().idxDown()) {
			cursor().pos_ = xarray().x2pos(x);
			return true;
		}
		if (pop()) 
			return true;
		return false;
	}

	// check whether we could move into an inset on the right or on the left
	MathInset * p = nextInset();
	if (p) {
		int idx, pos;
		if (p->idxFirstDown(idx, pos)) {
			push(p, true);
			cursor().idx_ = idx;
			cursor().pos_ = pos;
			dump("Down 3");
			return true;
		}
	}

	p = prevInset();
	if (p) {
		int idx, pos;
		if (p->idxLastDown(idx, pos)) {
			plainLeft();
			push(p, false);
			cursor().idx_ = idx;
			cursor().pos_ = pos;
			dump("Down 4");
			return true;
		}
	}

	int x = xarray().pos2x(cursor().pos_);
	if (cursor().idxDown()) {
		cursor().pos_ = xarray().x2pos(x);
		return true;
	}
	if (pop())
		return true;
	return false;
}


bool MathCursor::toggleLimits()
{
	if (!prevIsInset())
		return false;
	MathInset * p = prevInset();
	int old = p->limits();
	p->limits(old < 0 ? 1 : -1);
	return old != p->limits();
}


void MathCursor::SetSize(MathStyles size)
{
	cursor().par_->UserSetSize(size);
}



void MathCursor::Interpret(string const & s)
{
	//lyxerr << "Interpret: '" << s << "'\n";
	//lyxerr << "in: " << in_word_set(s) << " \n";

	if (s.size() && (s[0] == '^' || s[0] == '_')) {
		bool const up = (s[0] == '^');
		SelCut();	
		MathScriptInset * p = prevScriptInset();
		if (!p) {
			MathInset * b = prevInset();
			if (b && b->isScriptable()) {
				p = new MathScriptInset(up, !up, b->clone());
				plainLeft();
				erase();
			} else {
				p = new MathScriptInset(up, !up);
			}
			insert(p);
			plainLeft();
		}
		push(p, true);
		if (up)
			p->up(true);
		else
			p->down(true);
		cursor().idx_ = up ? 0 : 1;
		cursor().pos_ = 0;
		SelPaste();
		return;
	}

	if (s[0] == '!' || s[0] == ','  || s[0] == ':' || s[0] == ';') {
		int sp = (s[0] == ',') ? 1:((s[0] == ':') ? 2:((s[0] == ';') ? 3: 0));
		insert(new MathSpaceInset(sp));
		return;
	}

	MathInset * p = 0;
	latexkeys const * l = in_word_set(s);

	if (l == 0) {
		if (s == "root") 
			p = new MathRootInset;
		else if (MathMacroTable::hasTemplate(s))
			p = new MathMacro(MathMacroTable::provideTemplate(s));
		else if (s.size() > 7 && s.substr(0, 7) == "matrix ") {
			int m = 1;
			int n = 1;
			string v_align;
			string h_align;
			istringstream is(s.substr(7).c_str());
			is >> m >> n >> v_align >> h_align;
			m = std::max(1, m);
			n = std::max(1, n);
			v_align += 'c';
			MathArrayInset * pp = new MathArrayInset(m, n);
			pp->valign(v_align[0]);
			pp->halign(h_align);
			p = pp;
		}
		else
			p = new MathFuncInset(s, LM_OT_UNDEF);
	} else {
		switch (l->token) {
			case LM_TK_BIGSYM: 
				p = new MathBigopInset(s, l->id);
				break;
				
			case LM_TK_SYM: {
				MathTextCodes code = static_cast<MathTextCodes>(l->id);
				if (code < 255)
					insert(l->id, MathIsBOPS(code) ? LM_TC_BOPS : LM_TC_SYMB);
				else
					p = new MathFuncInset(l->name);
				break;
			}

			case LM_TK_STACK:
				p = new MathFracInset("stackrel");
				break;

			case LM_TK_FRAC:
				p = new MathFracInset("frac");
				break;

			case LM_TK_SQRT:
				p = new MathSqrtInset;
				break;

			case LM_TK_DECORATION:
				p = new MathDecorationInset(l->name, l->id);
				break;

			case  LM_TK_FUNCLIM:
				p = new MathFuncInset(l->name, LM_OT_FUNCLIM);
				break;

			case LM_TK_SPACE:
				p = new MathSpaceInset(l->id);
				break;

			case LM_TK_DOTS:
				p = new MathDotsInset(l->name, l->id);
				break;

			case LM_TK_MACRO:
				p = new MathMacro(MathMacroTable::provideTemplate(s));
				break;

			default:
				p = new MathFuncInset(l->name);
				break;
		}
	}

	if (p) {
		bool oldsel = selection;
		if (oldsel) 
			SelCut();
		insert(p);
		if (p->nargs()) {
			plainLeft();
			//push(p, true);
			Right();  // do not push for e.g. MathBigopInset
			if (oldsel) 
				SelPaste();
		}
		p->Metrics(p->size());
	}
}


void MathCursor::MacroModeOpen()
{
	if (!macro_mode) {
		imacro = new MathFuncInset("");
		insert(imacro);
		macro_mode = true;
	} else
		lyxerr << "Math Warning: Already in macro mode" << endl;
}


void MathCursor::MacroModeClose()
{
	if (macro_mode) {
		string name = imacro->name();
		plainLeft();
		erase();
		delete imacro;
		macro_mode = false;
		imacro = 0;
		Interpret(name);
	}
}


void MathCursor::SelCopy()
{
	seldump("SelCopy");
	if (selection) {
		theSelection.grab(*this);
		SelClear();
	}
}


void MathCursor::SelCut()
{
	seldump("SelCut");
	if (selection) {
		theSelection.grab(*this);
		theSelection.erase(*this);
		SelClear();
	}
}


void MathCursor::SelDel()
{
	seldump("SelDel");
	if (selection) {
		theSelection.erase(*this);
		SelClear();
	}
}


void MathCursor::SelPaste()
{
	seldump("SelPaste");
	theSelection.paste(*this);
	SelClear();
}


void MathCursor::SelHandle(bool sel)
{
	if (sel && !selection)
		SelStart();
	if (!sel && selection)
		SelClear();
}


void MathCursor::SelStart()
{
	seldump("SelStart");
	if (selection)
		return;

	Anchor_ = Cursor_;
	selection = true;
}


void MathCursor::SelClear()
{
	selection = false;
}


void MathCursor::drawSelection(Painter & pain) const
{
	if (!selection)
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


void MathCursor::handleFont(MathTextCodes t)
{
	if (selection) {
		MathCursorPos i1;
		MathCursorPos i2;
		getSelection(i1, i2); 
		if (i1.idx_ == i2.idx_) {
			MathArray & ar = i1.cell();
			for (int pos = i1.pos_; pos != i2.pos_; ar.next(pos))
				if (!ar.isInset(pos) && isalnum(ar.GetChar(pos))) { 
					MathTextCodes c = ar.GetCode(pos) == t ? LM_TC_VAR : t;
					ar.setCode(pos, c);
				}
		}
	} else {
		lastcode = (lastcode == t) ? LM_TC_VAR : t;
	}
}


void MathCursor::handleAccent(string const & name, int code)
{
	MathDecorationInset * p = new MathDecorationInset(name, code);
	if (selection) {
		SelCut();
		p->cell(0) = theSelection.glue();
	}
	insert(p);
	push(p, true);
}


void MathCursor::handleDelim(int l, int r)
{
	MathDelimInset * p = new MathDelimInset(l, r);
	if (selection) {
		SelCut();
		p->cell(0) = theSelection.glue();
	}
	insert(p);
	plainLeft();
	push(p, true);
}


void MathCursor::GetPos(int & x, int & y)
{
	x = xarray().xo() + xarray().pos2x(cursor().pos_);
	y = xarray().yo();
}


MathTextCodes MathCursor::nextCode() const
{
	return array().GetCode(cursor().pos_); 
}


MathTextCodes MathCursor::prevCode() const
{
	return array().GetCode(cursor().pos_ - 1); 
}


MathInset * MathCursor::par() const
{
	return cursor().par_;
}


InsetFormulaBase const * MathCursor::formula()
{
	return formula_;
}


int MathCursor::pos() const
{
	return cursor().pos_;
}


bool MathCursor::InMacroMode() const
{
	return macro_mode;
}


bool MathCursor::Selection() const
{
	return selection;
}


void MathCursor::clearLastCode()
{
	lastcode = LM_TC_MIN;
}


void MathCursor::setLastCode(MathTextCodes t)
{
	lastcode = t;
}


MathTextCodes MathCursor::getLastCode() const
{
	return lastcode;
}


MathInset * MathCursor::enclosing(MathInsetTypes t, int & idx) const
{
	for (int i = Cursor_.size() - 1; i >= 0; --i) {
		//lyxerr << "checking level " << i << "\n";
		if (Cursor_[i].par_->GetType() == t) {
			idx = Cursor_[i].idx_;
			return Cursor_[i].par_;
		}
	}
	return 0;
}


void MathCursor::pullArg(bool goright)
{
	// pullArg
	dump("pullarg");
	MathArray a = array();
	if (pop()) {
		array().erase(cursor().pos_);
		array().insert(cursor().pos_, a);
		if (goright) 
			cursor().pos_ += a.size();
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

	if (cursor().idx_ < 0 || cursor().idx_ > cursor().par_->nargs())
		lyxerr << "this should not really happen - 1\n";
	it->cursor().idx_    = max(cursor().idx_, 0);
 	it->cursor().idx_    = min(cursor().idx_, cursor().par_->nargs());

	if (cursor().pos_ < 0 || cursor().pos_ > array().size())
		lyxerr << "this should not really happen - 2\n";
	it->cursor().pos_ = max(cursor().pos_, 0);
	it->cursor().pos_ = min(cursor().pos_, array().size());
}


int MathCursor::col() const
{
	return par()->col(cursor().idx_);
}


int MathCursor::row() const
{
	return par()->row(cursor().idx_);
}


/*
char MathCursorPos::GetChar() const
{
	return array().GetChar(cursor().pos_);
}


string MathCursorPos::readString()
{
	string s;
	int code = nextCode();
	for ( ; OK() && nextCode() == code; Next()) 
		s += GetChar();

	return s;
}
*/


MathInset * MathCursor::prevInset() const
{
	normalize();
	int c = cursor().pos_;
	if (!array().prev(c))
		return 0;
	return array().nextInset(c);
}


MathInset * MathCursor::nextInset() const
{
	normalize();
	return array().nextInset(cursor().pos_);
}


MathScriptInset * MathCursor::prevScriptInset() const
{
	normalize();
	MathInset * p = array().prevInset(cursor().pos_);
	return (p && p->isScriptInset()) ? static_cast<MathScriptInset *>(p) : 0;
}


MathSpaceInset * MathCursor::prevSpaceInset() const
{
	normalize();
	MathInset * p = array().prevInset(cursor().pos_);
	return (p && p->isSpaceInset()) ? static_cast<MathSpaceInset *>(p) : 0;
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;
	if (!cursor().par_) {
		lyxerr << "############  par_ not valid\n";
		return dummy;
	}

	if (cursor().idx_ < 0 || cursor().idx_ >= cursor().par_->nargs()) {
		lyxerr << "############  idx_ " << cursor().idx_ << " not valid\n";
		return dummy;
	}

	return cursor().cell();
}


MathXArray & MathCursor::xarray() const
{
	return cursor().xcell();
}


bool MathCursor::nextIsInset() const
{
	return cursor().pos_ < array().size() && MathIsInset(nextCode());
}


bool MathCursor::prevIsInset() const
{
	return cursor().pos_ > 0 && MathIsInset(prevCode());
}


int MathCursor::xpos() const 
{
	normalize();
	return xarray().pos2x(cursor().pos_);
}


void MathCursor::gotoX(int x)
{
	cursor().pos_ = xarray().x2pos(x);	
}


void MathCursor::idxNext()
{
	cursor().par_->idxNext(cursor().idx_, cursor().pos_);
}


void MathCursor::idxPrev()
{
	cursor().par_->idxPrev(cursor().idx_, cursor().pos_);
}


void MathCursor::splitCell()
{
	if (cursor().idx_ == cursor().par_->nargs() - 1) 
		return;
	MathArray ar = array();
	ar.erase(0, cursor().pos_);
	array().erase(cursor().pos_, array().size());
	++cursor().idx_;
	cursor().pos_ = 0;
	array().insert(0, ar);
}


void MathCursor::breakLine()
{
	MathMatrixInset * p = static_cast<MathMatrixInset *>(formula()->par());
	if (p->GetType() == LM_OT_SIMPLE || p->GetType() == LM_OT_EQUATION) {
		p->mutate(LM_OT_EQNARRAY);
		p->addRow(0);
		cursor().idx_ = p->nrows();
		cursor().pos_ = 0;
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
		p->cell(cursor().idx_).swap(p->cell(cursor().idx_ + p->ncols() - 1));
	}
}


char MathCursor::valign() const
{
	int idx;
	MathGridInset * p =
		static_cast<MathGridInset *>(enclosing(LM_OT_MATRIX, idx));
	return p ? p->valign() : 0;
}


char MathCursor::halign() const
{
	int idx;
	MathGridInset * p =
		static_cast<MathGridInset *>(enclosing(LM_OT_MATRIX, idx));
	return p ? p->halign(idx % p->ncols()) : 0;
}


MathCursorPos MathCursor::firstSelectionPos() const
{
	MathCursorPos anc = normalAnchor();
	return anc < cursor() ? anc : cursor(); 
}


MathCursorPos MathCursor::lastSelectionPos() const
{
	MathCursorPos anc = normalAnchor();
	return anc < cursor() ? cursor() : anc; 
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
		normal.cell().next(normal.pos_);
	}
	//lyxerr << "normalizing: from " << Anchor_[Anchor_.size() - 1] << " to "
	//	<< normal << "\n";
	return normal;
}


bool MathCursorPos::idxUp()
{
	return par_->idxUp(idx_, pos_);
}


bool MathCursorPos::idxDown()
{
	return par_->idxDown(idx_, pos_);
}


bool MathCursorPos::idxLeft()
{
	return par_->idxLeft(idx_, pos_);
}


bool MathCursorPos::idxRight()
{
	return par_->idxRight(idx_, pos_);
}
