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
#include <cctype>

#include "math_inset.h"
#include "math_arrayinset.h"
#include "math_parser.h"
#include "math_cursor.h"
#include "math_macro.h"
#include "math_macroarg.h"
#include "math_macrotable.h"
#include "math_root.h"
#include "support/lstrings.h"
#include "debug.h"
#include "LColor.h"
#include "Painter.h"
#include "math_matrixinset.h"
#include "math_grid.h"
#include "math_spaceinset.h"
#include "math_funcinset.h"
#include "math_bigopinset.h"
#include "math_fracinset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_deliminset.h"
#include "math_accentinset.h"
#include "math_macrotemplate.h"
#include "math_sqrtinset.h"
#include "math_scriptinset.h"
#include "mathed/support.h"
#include "formulabase.h"


using std::endl;
using std::min;
using std::max;
using std::isalnum;


namespace {

MathArray selarray;

bool IsMacro(short tok, int id)
{
	return tok != LM_TK_STACK &&
	       tok != LM_TK_FRAC &&
	       tok != LM_TK_SQRT &&
	       tok != LM_TK_WIDE &&
	       tok != LM_TK_SPACE &&
	       tok != LM_TK_DOTS &&
	       tok != LM_TK_FUNCLIM &&
	       tok != LM_TK_BIGSYM &&
	       tok != LM_TK_ACCENT &&
	       !(tok == LM_TK_SYM && id < 255);
}

}

MathCursor::MathCursor(InsetFormulaBase * formula)
	: formula_(formula)
{
	lastcode   = LM_TC_MIN;
	macro_mode = false;
	first();
}


void MathCursor::push(MathInset * par, bool first)
{
	path_.push_back(MathIter());
	path_.back().par_    = par_;
	path_.back().idx_    = idx_;
	path_.back().cursor_ = cursor_;
	dump("Pushed:");
	par_ = par;
	first ? par_->idxFirst(idx_, cursor_) : par_->idxLast(idx_, cursor_);
}


bool MathCursor::pop()
{
	if (path_.empty())
		return false;
	par_    = path_.back().par_;
	idx_    = path_.back().idx_;
	cursor_ = path_.back().cursor_;
	dump("Popped:");
	path_.pop_back();
	return true;
}


MathInset * MathCursor::parInset(int i) const
{
	return path_[i].par_;
}

void MathCursor::dump(char const * what) const
{
	return;

	lyxerr << "MC: " << what
		<< " cursor: " << cursor_
		<< " anchor: " << anchor_
		<< " idx: " << idx_
		<< " par: " << par_
		<< " sel: " << selection
		<< " data: " << array()
		<< "\n";
}

void MathCursor::seldump(char const * str) const
{
	lyxerr << "SEL: " << str << ": '" << selarray << "'\n";
	dump("   Pos");
	return;

	lyxerr << "\n\n\\n=================vvvvvvvvvvvvv=======================   "
		<<  str << "\nselarray: " << selarray;
	for (unsigned int i = 0; i < path_.size(); ++i) 
		lyxerr << path_[i].par_ << "\n'" << path_[i].par_->cell(0) << "'\n";
	lyxerr << "\ncursor: " << cursor_;
	lyxerr << "\nanchor: " << anchor_;
	lyxerr << "\n===================^^^^^^^^^^^^=====================\n\n\n";
}


bool MathCursor::isInside(MathInset * p) const
{
	for (unsigned i = 0; i < path_.size(); ++i) 
		if (parInset(i) == p) 
			return true;
	return par_ == p;
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
	clearLastCode();
	SelHandle(sel);

	bool result = false;

	if (selection) {
		result = array().prev(cursor_);
		if (!result && pop()) {
			anchor_ = cursor_;
			result = array().next(anchor_);
		}
	} else {
		MathInset * p = prevInset();
		if (p && p->isActive()) {
			// We have to move deeper into the previous inset
			array().prev(cursor_);
			push(p, false);
			result = true;
		} else {
			// The common case, where we are not 
			// entering a deeper inset
			result = array().prev(cursor_);
			if (!result) {
				if (par_->idxLeft(idx_, cursor_)) {
					result = true;
				} else if (pop()) {
					result = true;
				}
			}
		}
	}
	dump("Left 2");
	return result;
}


bool MathCursor::plainRight()
{
	return array().next(cursor_);
}


bool MathCursor::Right(bool sel)
{
	dump("Right 1");
	if (macro_mode) {
		MacroModeClose();
		return true;
	}

	clearLastCode();
	SelHandle(sel);

	bool result = false;

	if (selection) {
		result = array().next(cursor_);
		if (!result && pop()) {
			anchor_ = cursor_;
			result = array().next(cursor_);
		}
	} else {
		MathInset * p = nextInset();
		if (p && p->isActive()) {
			push(p, true);
			result = true;
		} else {
			result = array().next(cursor_);
			if (!result) {
				if (par_->idxRight(idx_, cursor_)) {
					result = true;
				} else if (pop()) {
					result = true;
					array().next(cursor_);
				}
			}
		}
	}
	dump("Right 2");
	return result;
}


void MathCursor::first()
{
	selection  = false;
	par_       = formula_->par();
	idx_       = 0;
	cursor_    = 0;
	anchor_    = 0;
	par_->idxFirst(idx_, cursor_);
}


void MathCursor::last()
{
	selection  = false;
	par_       = formula_->par();
	idx_       = 0;
	cursor_    = 0;
	anchor_    = 0;
	par_->idxLast(idx_, cursor_);
}


void MathCursor::SetPos(int x, int y)
{
	dump("SetPos 1");
	//lyxerr << "MathCursor::SetPos x: " << x << " y: " << y << "\n";

	MacroModeClose();
	lastcode = LM_TC_MIN;
	path_.clear();

	par_    = formula()->par();

	while (1) {
		idx_    = -1;
		cursor_ = -1;
		//lyxerr << "found idx: " << idx_ << " cursor: " << cursor_  << "\n";
		int distmin = 1 << 30; // large enough
		for (int i = 0; i < par_->nargs(); ++i) {
			MathXArray const & ar = par_->xcell(i);
			int x1 = x - ar.xo();
			int y1 = y - ar.yo();
			int c  = ar.x2pos(x1);
			int xx = abs(x1 - ar.pos2x(c));
			int yy = abs(y1);
			//lyxerr << "idx: " << i << " xx: " << xx << " yy: " << yy
			//	<< " c: " << c  << " xo: " << ar.xo() << "\n";
			if (yy + xx <= distmin) {
				distmin = yy + xx;
				idx_     = i;
				cursor_  = c;
			}
		}
		lyxerr << "found idx: " << idx_ << " cursor: " << cursor_  << "\n";
		MathInset * n = nextInset();
		MathInset * p = prevInset();
		if (n && (n->isActive() || n->isUpDownInset()) && n->covers(x, y))
			push(n, true);
		else if (p && (p->isActive() || p->isUpDownInset()) && p->covers(x, y)) {
			array().prev(cursor_);
			push(p, false);
		} else 
			break;
	}
	dump("SetPos 2");
}


void MathCursor::Home()
{
	dump("Home 1");
	if (macro_mode)
		MacroModeClose();
	clearLastCode();
	if (!par_->idxHome(idx_, cursor_)) {
		pop();
	}
	dump("Home 2");
}


void MathCursor::End()
{
	dump("End 1");
	if (macro_mode)
		MacroModeClose();
	clearLastCode();
	if (!par_->idxEnd(idx_, cursor_)) {
		pop();
		array().next(cursor_);
	}
	dump("End 2");
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

	array().insert(cursor_, c, t);
	array().next(cursor_);

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

	array().insert(cursor_, p);
	array().next(cursor_);
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

	if (cursor_ < array().size())
		array().erase(cursor_);

	// delete empty cells if necessary
	if (cursor_ == 0 && array().size() == 0) {
		bool popit;
		bool removeit;
		par_->idxDelete(idx_, popit, removeit);
		if (popit && pop() && removeit)
			Delete();
	}

#ifdef WITH_WARNINGS
#warning pullArg disabled
#endif
	//if (cursor_ == 0 && !path_.empty()) {
	//	lyxerr << "Delete: popping...\n";
	//	pop();
	//}

	dump("Delete 2");
}


void MathCursor::DelLine()
{
	MacroModeClose();

	if (selection) {
		SelDel();
		return;
	}

	if (par_->nrows() > 1)
		par_->delRow(row());
}


bool MathCursor::Up(bool sel)
{
	dump("Up 1");
	MacroModeClose();
	SelHandle(sel);
	SelClear();

	// check whether we could move into an inset on the right or on the left
	MathInset * p = nextInset();
	if (p) {
		int idx, cursor;
		if (p->idxFirstUp(idx, cursor)) {
			push(p, true);
			par_ = p;
			idx_    = idx;
			cursor_ = cursor;
			dump("Up 3");
			return true;
		}
	}

	p = prevInset();
	if (p) {
		int idx, cursor;
		if (p->idxLastUp(idx, cursor)) {
			array().prev(cursor_);
			push(p, false);
			par_ = p;
			idx_    = idx;
			cursor_ = cursor;
			dump("Up 4");
			return true;
		}
	}


	int x = xarray().pos2x(cursor_);
	bool result = par_->idxUp(idx_, cursor_);
	if (!result && pop()) {
		result = par_->idxUp(idx_, cursor_);
	}
	cursor_ = xarray().x2pos(x);

	dump("Up 2");
	return result;
}


bool MathCursor::Down(bool sel)
{
	dump("Down 1");
	MacroModeClose();
	SelHandle(sel);
	SelClear();

	// check whether we could move into an inset on the right or on the left
	MathInset * p = nextInset();
	if (p) {
		int idx, cursor;
		if (p->idxFirstDown(idx, cursor)) {
			push(p, true);
			idx_    = idx;
			cursor_ = cursor;
			dump("Down 3");
			return true;
		}
	}

	p = prevInset();
	if (p) {
		int idx, cursor;
		if (p->idxLastDown(idx, cursor)) {
			array().prev(cursor_);
			push(p, false);
			idx_    = idx;
			cursor_ = cursor;
			dump("Down 4");
			return true;
		}
	}

	int x = xarray().pos2x(cursor_);
	bool result = par_->idxDown(idx_, cursor_);
	if (!result && pop()) {
		result = par_->idxDown(idx_, cursor_);
	}
	cursor_ = xarray().x2pos(x);

	dump("Down 2");
	return result;
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
	par_->UserSetSize(size);
}



void MathCursor::Interpret(string const & s)
{
	lyxerr << "Interpret: '" << s << "'  ('" << s.substr(0, 7)  << "' " <<
in_word_set(s) << " \n";

	if (s[0] == '^') {
		MathUpDownInset * p = nearbyUpDownInset();
		if (!p) {
			p = new MathScriptInset(true, false);
			insert(p);
			array().prev(cursor_);
		}
		push(p, true);
		p->up(true);
		idx_ = 0;
		return;
	}

	if (s[0] == '_') {
		MathUpDownInset * p = nearbyUpDownInset();
		if (!p) {
			p = new MathScriptInset(false, true);
			insert(p);
			array().prev(cursor_);
		}
		push(p, true);
		p->down(true);
		idx_ = 1;
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
			std::istringstream is(s.substr(7).c_str());
			is >> m >> n >> v_align >> h_align;
			m = std::max(1, m);
			n = std::max(1, n);
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
					p = new MathBigopInset(l->name, l->id);
					break;
				
			case LM_TK_SYM: 
				if (l->id < 255) {
					insert(static_cast<byte>(l->id), 
					       MathIsBOPS(l->id) ?
						LM_TC_BOPS : LM_TC_SYMB);
					
				} else {
					p = new MathFuncInset(l->name);
				}
				break;

			case LM_TK_STACK:
				p = new MathFracInset("stackrel");
				break;

			case LM_TK_FRAC:
				p = new MathFracInset("frac");
				break;

			case LM_TK_SQRT:
				p = new MathSqrtInset;
				break;

			case LM_TK_WIDE:
				p = new MathDecorationInset(l->id);
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

			case LM_TK_ACCENT:
				p = new MathAccentInset(l->id);
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
			array().prev(cursor_);
			push(p, true);
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
	if (macro_mode)  {
		macro_mode = false;
		latexkeys const * l = in_word_set(imacro->name());
		if (!imacro->name().empty()
				&& (!l || (l && IsMacro(l->token, l->id)))
				&& !MathMacroTable::hasTemplate(imacro->name()))
		{
			if (!l) {
				//imacro->SetName(macrobf);
				// This guarantees that the string will be removed by destructor
				imacro->SetType(LM_OT_UNDEF);
			} else
				imacro->SetName(l->name);
		} else {
			Left();
			array().erase(cursor_);
			if (l || MathMacroTable::hasTemplate(imacro->name())) 
				Interpret(imacro->name());
			imacro->SetName(string());
		}
		imacro = 0;
	}
}


void MathCursor::SelCopy()
{
	seldump("SelCopy");
	if (selection) {
		int const p1 = min(cursor_, anchor_);
		int const p2 = max(cursor_, anchor_);
		selarray = array();
		selarray.erase(p2, selarray.size());
		selarray.erase(0, p1);
		SelClear();
	}
}

void MathCursor::SelCut()
{
	seldump("SelCut");
	if (selection) {
		int const p1 = min(cursor_, anchor_);
		int const p2 = max(cursor_, anchor_);
		cursor_ = p1;  // move cursor to a same position
		selarray = array();
		selarray.erase(p2, selarray.size());
		selarray.erase(0, p1);
		array().erase(p1, p2);
		SelClear();
	}
}


void MathCursor::SelDel()
{
	seldump("SelDel");
	if (selection) {
		int const p1 = min(cursor_, anchor_);
		int const p2 = max(cursor_, anchor_);
		array().erase(p1, p2);
		SelClear();
	}
}


void MathCursor::SelPaste()
{
	seldump("SelPaste");
	array().insert(cursor_, selarray);
	cursor_ += selarray.size();
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

	anchor_   = cursor_;
	selection = true;
}


void MathCursor::SelClear()
{
	selection = false;
}



void MathCursor::SelGetArea(int * xpoint, int * ypoint, int & n)
{
	if (!selection) {
		n = 0;
		xpoint[0] = 0;
		ypoint[0] = 0;
		return;
	}

	// Balance anchor and cursor
	int xo;
	int yo;
	par()->GetXY(xo, yo);
	int w = par()->width();
	// cursor
	int x1 = xarray().xo() + xarray().pos2x(cursor_);
	int y1 = xarray().yo();
	//int a1 = xarray().ascent();
	//int d1 = xarray().descent();

	// anchor
	int x  = xarray().xo() + xarray().pos2x(anchor_);
	int y  = xarray().yo();
	int a  = xarray().ascent();
	int d  = xarray().descent();

	// single row selection
	n = 0;
	xpoint[n]   = x;
	ypoint[n++] = y + d;
	xpoint[n]   = x;
	ypoint[n++] = y - a;

	if (y != y1) {
		xpoint[n]   = xo + w;
		ypoint[n++] = y - a;

		if (x1 < xo + w) {
			xpoint[n]   = xo + w;
			ypoint[n++] = y1 - a;
		}
	}

	xpoint[n]   = x1;
	ypoint[n++] = y1 - a;
	xpoint[n]   = x1;
	ypoint[n++] = y1 + d;

	if (y != y1) {
		xpoint[n]   = xo;
		ypoint[n++] = y1 + d;
		if (x > xo) {
			xpoint[n]   = xo;
			ypoint[n++] = y + d;
		}
	}
	xpoint[n]   = xpoint[0];
	ypoint[n++] = ypoint[0];

	//lyxerr << "AN[" << x << " " << y << " " << x1 << " " << y1 << "]\n";
	//lyxerr << "MT[" << a << " " << d << " " << a1 << " " << d1 << "]\n";
	//for (i = 0; i < np; ++i)
	//	lyxerr << "XY[" << xpoint[i] << " " << ypoint[i] << "]\n";
}


void MathCursor::handleFont(MathTextCodes t)
{
	if (selection)	{
		int const p1 = std::min(cursor_, anchor_);
		int const p2 = std::max(cursor_, anchor_);
		MathArray & ar = array();
		for (int pos = p1; pos != p2; ar.next(pos))
			if (!ar.isInset(pos) && isalnum(ar.GetChar(pos))) { 
				MathTextCodes c = ar.GetCode(pos) == t ? LM_TC_VAR : t;
				ar.setCode(pos, c);
			}
	} else {
		if (lastcode == t)
			lastcode = LM_TC_VAR;
		else
			lastcode = t;
	}
}


void MathCursor::handleAccent(int code)
{
	MathAccentInset * p = new MathAccentInset(code);
	if (selection) {
		SelCut();
		p->cell(0) = selarray;
	}
	insert(p);
}

void MathCursor::handleDelim(int l, int r)
{
	MathDelimInset * p = new MathDelimInset(l, r);
	if (selection) {
		SelCut();
		p->cell(0) = selarray;
	}
	insert(p);
}


void MathCursor::GetPos(int & x, int & y)
{
	x = xarray().xo() + xarray().pos2x(cursor_);
	y = xarray().yo();
}


MathTextCodes MathCursor::nextCode() const
{
	return array().GetCode(cursor_); 
}


MathTextCodes MathCursor::prevCode() const
{
	return array().GetCode(cursor_ - 1); 
}


MathInset * MathCursor::par() const
{
	return par_;
}


InsetFormulaBase const * MathCursor::formula()
{
	return formula_;
}


int MathCursor::pos() const
{
	return cursor_;
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
	if (par_->GetType() == t) {
		//lyxerr << "enclosing par is current\n";
		idx = idx_;
		return par_;
	}
	for (int i = path_.size() - 1; i >= 0; --i) {
		lyxerr << "checking level " << i << "\n";
		if (path_[i].par_->GetType() == t) {
			idx = path_[i].idx_;
			return path_[i].par_;
		}
	}
	return 0;
}

void MathCursor::pullArg()
{
	// pullArg
	MathArray a = array();
	if (!Left())
		return;
	normalize();
	array().erase(cursor_);
	array().insert(cursor_, a);
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

	if (idx_ < 0 || idx_ > par_->nargs())
		lyxerr << "this should not really happen - 1\n";
	it->idx_    = max(idx_, 0);
 	it->idx_    = min(idx_, par_->nargs());

	if (cursor_ < 0 || cursor_ > array().size())
		lyxerr << "this should not really happen - 2\n";
	it->cursor_ = max(cursor_, 0);
	it->cursor_ = min(cursor_, array().size());
}


int MathCursor::col() const
{
	return par_->col(idx_);
}


int MathCursor::row() const
{
	return par_->row(idx_);
}


/*
char MathIter::GetChar() const
{
	return array().GetChar(cursor_);
}


string MathIter::readString()
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
	int c = cursor_;
	if (!array().prev(c))
		return 0;
	return array().nextInset(c);
}


MathInset * MathCursor::nextInset() const
{
	normalize();
	return array().nextInset(cursor_);
}


MathUpDownInset * MathCursor::nearbyUpDownInset() const
{
	normalize();
	MathInset * p = array().prevInset(cursor_);
	if (p && p->isUpDownInset())
		return static_cast<MathUpDownInset *>(p);
	p = array().nextInset(cursor_);
	if (p && p->isUpDownInset())
		return static_cast<MathUpDownInset *>(p);
	return 0;
}


MathArray & MathCursor::array() const
{
	static MathArray dummy;
	if (!par_) {
		lyxerr << "############  par_ not valid\n";
		return dummy;
	}

	if (idx_ < 0 || idx_ >= par_->nargs()) {
		lyxerr << "############  idx_ " << idx_ << " not valid\n";
		return dummy;
	}

	return par_->cell(idx_);
}


MathXArray & MathCursor::xarray() const
{
	return par_->xcell(idx_);
}



bool MathCursor::nextIsInset() const
{
	return cursor_ < array().size() && MathIsInset(nextCode());
}


bool MathCursor::prevIsInset() const
{
	return cursor_ > 0 && MathIsInset(prevCode());
}


int MathCursor::xpos() const 
{
	normalize();
	return xarray().pos2x(cursor_);
}

void MathCursor::gotoX(int x)
{
	cursor_ = xarray().x2pos(x);	
}

void MathCursor::idxNext()
{
	par_->idxNext(idx_, cursor_);
}

void MathCursor::idxPrev()
{
	par_->idxPrev(idx_, cursor_);
}

void MathCursor::splitCell()
{
	if (idx_ == par_->nargs() - 1) 
		return;
	MathArray ar = array();
	ar.erase(0, cursor_);
	array().erase(cursor_, array().size());
	++idx_;
	cursor_ = 0;
	array().insert(0, ar);
}

void MathCursor::breakLine()
{
	MathMatrixInset * p = static_cast<MathMatrixInset *>(formula()->par());
	if (p->GetType() == LM_OT_SIMPLE || p->GetType() == LM_OT_EQUATION) {
		p->mutate(LM_OT_EQNARRAY);
		p->addRow(row());
		idx_ = p->nrows();
		cursor_ = 0;
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
		p->cell(idx_).swap(p->cell(idx_ + p->ncols() - 1));
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
