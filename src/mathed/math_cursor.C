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
*   Version: 0.8beta, Mathed & Lyx project.
*
*   You are free to use and modify this code under the terms of
*   the GNU General Public Licence version 2 or later.
*/

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include FORMS_H_LOCATION
#include "math_inset.h"
#include "math_parser.h"
#include "math_cursor.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_root.h"
#include "support/lstrings.h"
#include "debug.h"
#include "LColor.h"
#include "Painter.h"
#include "math_matrixinset.h"
#include "math_rowst.h"
#include "math_spaceinset.h"
#include "math_funcinset.h"
#include "math_bigopinset.h"
#include "math_fracinset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_accentinset.h"
#include "mathed/support.h"

static MathedArray selarray;

using std::endl;

// This was very smaller, I'll change it later
static inline
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

static int const MAX_STACK_ITEMS = 32;

struct MathStackXIter {
	int i, imax;
	MathedXIter * item;

	MathStackXIter(int n = MAX_STACK_ITEMS): imax(n) {
		item = new MathedXIter[imax];
		i = 0;
	}

	MathStackXIter(MathStackXIter & stk);

	~MathStackXIter() {
		delete[] item;
	}

	void push(MathedXIter ** a) {
		*a = &item[i++];
	}

	MathedXIter * pop() {
		--i;
		return &item[i - 1];
	}

	MathedXIter * Item(int idx) {
		return (idx + 1 <= i) ? &item[i - idx - 1] : 0;
	}

	void Reset() {
		i = 0;
	}

	bool Full() {
		return i >= MAX_STACK_ITEMS;
	}

	bool Empty() {
		return i <= 1;
	}

	int Level() { return i; }

} mathstk, *selstk = 0;


MathStackXIter::MathStackXIter(MathStackXIter & stk)
{
	imax = stk.imax;
	item = new MathedXIter[imax];
	i = stk.i;
	for (int k = 0; k < i; ++k) {
		item[k].SetData(stk.item[k].getPar());
		item[k].GoBegin();
		item[k].goPosAbs(stk.item[k].getPos());
	}
}


/***----------------  Mathed Cursor  ---------------------------***/

MathedCursor::MathedCursor(MathParInset * p) // : par(p)
{
	accent   = 0;
	anchor   = 0;
	lastcode = LM_TC_MIN;
	SetPar(p);
	if (!MathMacroTable::built)
		MathMacroTable::mathMTable.builtinMacros();
}


void MathedCursor::SetPar(MathParInset * p)
{
	macro_mode = false;
	selection  = false; // not SelClear() ?
	mathstk.Reset();
	mathstk.push(&cursor);
	par = p;
	cursor->SetData(par);
}


void MathedCursor::draw(Painter & pain, int x, int y)
{
	//    lyxerr << "Cursor[" << x << " " << y << "] ";
	//win = pm;    // win = (mathedCanvas) ? mathedCanvas: pm;

	par->Metrics();
	int w = par->Width() + 2;
	int a = par->Ascent() + 1;
	int h = par->Height() + 1;

	if (par->GetType() > LM_OT_PAR) {
		a += 4;
		h += 8;
	}

	pain.rectangle(x - 1, y - a, w, h, LColor::mathframe);

	par->draw(pain, x, y);
	cursor->Adjust();
}


void MathedCursor::Redraw(Painter & pain)
{
	lyxerr[Debug::MATHED] << "Mathed: Redrawing!" << endl;
	par->Metrics();
	int w = par->Width();
	int h = par->Height();
	int x;
	int y;
	par->GetXY(x, y);
	//mathed_set_font(LM_TC_VAR, 1);
	pain.fillRectangle(x, y - par->Ascent(),
	x + w, y - par->Ascent() + h,
	LColor::mathbg);
	par->draw(pain, x, y);
}


bool MathedCursor::Left(bool sel)
{
	if (macro_mode) {
		// was MacroModeBack()
		if (!imacro->GetName().empty()) {
			imacro->SetName(imacro->GetName()
					.substr(0, imacro->GetName()
						.length() - 1));
			imacro->Metrics();
		} else
			MacroModeClose();
		return true;
	}

	clearLastCode();

	if (sel && !selection)
		SelStart();

	if (!sel && selection)
		SelClear();

	bool result = cursor->Prev();

	if (!result && !mathstk.Empty()) {
		cursor = mathstk.pop();
		cursor->Adjust();
		result = true;
		if (selection)
			SelClear();
	} else if (result && cursor->IsActive()) {
		if (cursor->IsScript()) {
			cursor->Prev();
			if (!cursor->IsScript())
				cursor->Next();
			cursor->Adjust();
			return true;
		}

		if (!selection) {
			MathParInset * p = cursor->GetActiveInset();
			if (!p)
				return result;

			p->setArgumentIdx(p->getMaxArgumentIdx());
			mathstk.push(&cursor);
			cursor->SetData(p);
			cursor->GoLast();
		}
	}
	return result;
}


// Leave the inset
bool MathedCursor::Pop()
{
	if (!mathstk.Empty()) {
		cursor = mathstk.pop();
		cursor->Next();
		return true;
	}
	return false;
}


// Go to the inset
bool MathedCursor::Push()
{
	if (cursor->IsActive()) {
		MathParInset * p = cursor->GetActiveInset();
		if (!p)
			return false;
		mathstk.push(&cursor);
		cursor->SetData(p);
		return true;
	}
	return false;
}


bool MathedCursor::Right(bool sel)
{
	if (macro_mode) {
		MacroModeClose();
		return true;
	}

	clearLastCode();

	if (sel && !selection)
		SelStart();

	if (!sel && selection)
		SelClear();

	bool result = false;

	if (cursor->IsActive()) {
		if (cursor->IsScript()) {
			cursor->Next();
			// A script may be followed by another script
			if (cursor->IsScript())
				cursor->Next();
			return true;
		}

		if (!selection) {
			MathParInset *p = cursor->GetActiveInset();
			if (!p) {
			lyxerr << "Math error: Inset expected." << endl;
			return cursor->Next();
			}
			p->setArgumentIdx(0);
			mathstk.push(&cursor);
			cursor->SetData(p);
			result = true;
		} else
			result = cursor->Next();

	} else {
		if (cursor->GetChar()!= LM_TC_CR)
		result = cursor->Next();
		if (!result && !mathstk.Empty()) {
			cursor = mathstk.pop();
			cursor->Next();
			cursor->Adjust();
			result = true;
			if (selection)
				SelClear();
		}
	}
	return result;
}


void MathedCursor::SetPos(int x, int y)
{
	int xp = 0;

	if (macro_mode)
		MacroModeClose();

	lastcode = LM_TC_MIN;
	mathstk.Reset();
	mathstk.push(&cursor);
	cursor->SetData(par);
	cursor->fitCoord(x, y);

	while (cursor->GetX()<x && cursor->OK()) {
		if (cursor->IsActive()) {
			MathParInset * p = cursor->GetActiveInset();
			if (p->Inside(x, y)) {
				p->SetFocus(x, y);
				mathstk.push(&cursor);
				cursor->SetData(p);
				cursor->fitCoord(x, y);
				continue;
			}
		}
		xp = cursor->GetX();
		cursor->ipush();
		if (!cursor->Next() && !Pop())
			break;
	}
	if (x-xp < cursor->GetX()-x) cursor->ipop();
	cursor->Adjust();
}


void MathedCursor::Home()
{
	if (macro_mode)
		MacroModeClose();
	clearLastCode();
	mathstk.Reset();
	mathstk.push(&cursor);
	cursor->GoBegin();
}


void MathedCursor::End()
{
	if (macro_mode)
		MacroModeClose();
	clearLastCode();
	mathstk.Reset();
	mathstk.push(&cursor);
	cursor->GoLast();
}


MathMatrixInset * create_multiline(short int type, int cols)
{
	int columns;
	string align;
	if (cols < 1)
		cols = 1;

	switch (type) {
		case LM_OT_ALIGN:
		case LM_OT_ALIGNN:
			columns = 2*cols;
			for (int i = 0; i < cols; ++i)
			align += "Rl";
			break;

		case LM_OT_ALIGNAT:
		case LM_OT_ALIGNATN:
			columns = 2*cols;
			for (int i = 0; i < cols; ++i)
			align += "rl";
			break;

		case LM_OT_MULTLINE:
		case LM_OT_MULTLINEN:
			columns = 1;
			align = "C";
			break;

		case LM_OT_MPAR:
		case LM_OT_MPARN:
		default:
			columns = 3;
			align = "rcl";
			break;
	}

	MathMatrixInset * mt = new MathMatrixInset(columns, -1);
	mt->SetAlign(' ', align);
	return mt;
}


void MathedCursor::Insert(byte c, MathedTextCodes t)
{
	if (selection)
		SelDel();

	if (t == LM_TC_MIN)
		t = lastcode;

	if (macro_mode && !(MathIsAlphaFont(t) || t == LM_TC_MIN))
		MacroModeClose();

	if (t == LM_TC_CR) {
		MathParInset * p = cursor->getPar();
		if (p == par && p->GetType()<LM_OT_MPAR && p->GetType()>LM_OT_MIN) {
			short int type = LM_OT_MPAR;
			int cols = 1;
			if (c >= '1' && c <= '9') {
				type = LM_OT_ALIGN;
				cols = c - '1' + 1;
			} else if (c >= 'A' && c <= 'I') {
				type = LM_OT_ALIGNAT;
				cols = c - 'A' + 1;
			} else if (c == 'm')
				type = LM_OT_MULTLINE;
			else if (c == 'e')
				type = LM_OT_MPAR;

			if (p->GetType() == LM_OT_PARN)
				++type;
			MathMatrixInset * mt = create_multiline(type, cols);
			mt->SetStyle(LM_ST_DISPLAY);
			mt->SetType(type);
			mt->setData(p->GetData());
			delete p;
			par = mt;
			p = mt;
			p->Metrics();
			int pos = cursor->getPos();
			cursor->SetData(par);
			cursor->goPosAbs(pos);
		}
		if (p &&  p->Permit(LMPF_ALLOW_CR)) {
			cursor->addRow();
		}
	} else if (t == LM_TC_TAB) {
		MathParInset * p = cursor->getPar();
		if (p &&  p->Permit(LMPF_ALLOW_TAB)) {
			if (c) {
				cursor->insert(c, t);
				cursor->checkTabs();
			} else
				cursor->goNextColumn();
		} else // Navigate between arguments
		if (p && p->GetType() == LM_OT_MACRO) {
			if (p->getArgumentIdx() < p->getMaxArgumentIdx()) {
				p->setArgumentIdx(p->getArgumentIdx() + 1);
				cursor->SetData(p);
				return;
			}
		}
	} else {
		if (macro_mode) {
			if (MathIsAlphaFont(t) || t == LM_TC_MIN) {
				// was MacroModeInsert(c);
				imacro->SetName(imacro->GetName() + static_cast<char>(c));
				return;
			}
		}

		if (accent) {
			doAccent(c, t);
		} else
			cursor->insert(c, t);

		lastcode = t;
		return;
	}
	clearLastCode();
}


void MathedCursor::insertInset(MathedInset * p, int t)
{
	if (macro_mode)
		MacroModeClose();

	if (selection) {
		if (MathIsActive(t)) {
			SelCut();
			static_cast<MathParInset*>(p)->setData(selarray);
		} else
			SelDel();
	}

	if (mathstk.i < MAX_STACK_ITEMS - 1) {
		if (accent && !MathIsActive(t)) {	
			doAccent(p);
		} else {
			cursor->insertInset(p, t);
			if (MathIsActive(t)) {
				cursor->Prev();
				Push();
			}
		}
	} else
		lyxerr << "Math error: Full stack." << endl;
}


void MathedCursor::Delete()
{
	if (macro_mode)
		return;

	if (selection) {
		SelDel();
		return;
	}

	if (cursor->Empty() && !mathstk.Empty()) 
		cursor = mathstk.pop();

	//   if (cursor->GetChar()!= LM_TC_TAB)
	cursor->Delete();
	cursor->checkTabs();
}


void MathedCursor::DelLine()
{
	if (macro_mode)
		MacroModeClose();

	if (selection) {
		SelDel();
		return;
	}

	MathParInset * p = cursor->getPar();

	if (p && p->GetType() <= LM_OT_MATRIX && p->GetType() >= LM_OT_MPAR) {
		cursor->delRow();
	}
}


bool MathedCursor::Up(bool sel)
{
	bool result = false;

	if (macro_mode)
		MacroModeClose();

	if (sel && !selection)
		SelStart();

	if (!sel && selection)
		SelClear();

	if (cursor->IsScript()) {
		char cd = cursor->GetChar();
		if (MathIsUp(cd)) {
			Push();
			return true;
		} else {
			// A subscript may be followed by a superscript
			cursor->ipush();
			cursor->Next();
			if (MathIsUp(cursor->GetChar())) {
				Push();
				return true;
			} else  // return to the previous state
				cursor->ipop();
		}
	}

	result = cursor->Up();
	if (!result && cursor->getPar()) {
		MathParInset * p = cursor->getPar();

		if (p->GetType() == LM_OT_SCRIPT) {
			MathedXIter * cx = mathstk.Item(1);
			bool is_down = (cx->GetChar() == LM_TC_DOWN);
			cursor = mathstk.pop();
			cursor->Next();
			result =  (is_down) ? true: Up();
		} else {
			result = (p->getArgumentIdx() > 0);
			if (result) {
				p->setArgumentIdx(p->getArgumentIdx() - 1);
				cursor->SetData(p);
			}
		}

		if (!result && !mathstk.Empty()) {
			cursor = mathstk.pop();
			return Up();
		}
	}
	return result;
}


bool MathedCursor::Down(bool sel)
{
	bool result = false;

	if (macro_mode)
		MacroModeClose();

	if (sel && !selection)
		SelStart();

	if (!sel && selection)
		SelClear();

	if (cursor->IsScript()) {
		char cd = cursor->GetChar();
		if (MathIsDown(cd)) {
			Push();
			return true;
		} else {
		// A superscript may be followed by a subscript
		cursor->ipush();
		cursor->Next();
		if (MathIsDown(cursor->GetChar())) {
			Push();
			return true;
		} else
			cursor->ipop();
		}
	}

	result = cursor->Down();
	if (!result && cursor->getPar()) {
	MathParInset * p= cursor->getPar();
	if (p->GetType() == LM_OT_SCRIPT) {
	MathedXIter * cx = mathstk.Item(1);
	bool is_up = (cx->GetChar() == LM_TC_UP);
	cursor = mathstk.pop();
	cursor->Next();
	result = (is_up) ? true: Down();
	} else {
	result = (p->getArgumentIdx() < p->getMaxArgumentIdx());
	if (result) {
	p->setArgumentIdx(p->getArgumentIdx() + 1);
	cursor->SetData(p);
	}
	}
	if (!result && !mathstk.Empty()) {
	cursor = mathstk.pop();
	return Down(sel);
	}
	}
	return result;
}


bool MathedCursor::Limits()
{
	if (cursor->IsInset()) {
		MathedInset * p = cursor->GetInset();
		bool ol = p->GetLimits();
		p->SetLimits(!ol);
		return (ol!= p->GetLimits());
	}
	return false;
}


void MathedCursor::SetSize(short size)
{
	MathParInset * p = cursor->getPar();
	p->UserSetSize(size);
	cursor->SetData(p);
}


void MathedCursor::setLabel(string const & label)
{
	// ugly hack and possible bug
	if (!cursor->setLabel(label))
		lyxerr << "MathErr: Bad place to set labels." << endl;
}


void MathedCursor::setNumbered()
{
	// another ugly hack
	MathedRowSt * crow = cursor->currentRow();
	if (crow)
		crow->setNumbered(!crow->isNumbered());
}


void MathedCursor::Interpret(string const & s)
{
	MathedInset * p = 0;
	latexkeys const * l = 0;
	MathedTextCodes tcode = LM_TC_INSET;

	if (s[0] == '^' || s[0] == '_') {
		char c = cursor->GetChar();
		if (MathIsUp(c) && s[0] == '^' || MathIsDown(c) && s[0] == '_') {
			Push();
			return;
		} else

		// A script may be followed by a script
		if (MathIsUp(c)  || MathIsDown(c)) {
			cursor->ipush();
			cursor->Next();
			c = cursor->GetChar();
			if (MathIsUp(c) && s[0] == '^' || MathIsDown(c) && s[0] == '_') {
				Push();
				return;
			} else
				cursor->ipop();
		}
		p = new MathParInset(LM_ST_SCRIPT, "", LM_OT_SCRIPT);
		insertInset(p, (s[0] == '_') ? LM_TC_DOWN: LM_TC_UP);
		return;
	} else
	if (s[0] == '!' || s[0] == ','  || s[0] == ':' || s[0] == ';') {
		int sp = ((s[0] == ',') ? 1:((s[0] == ':') ? 2:((s[0] == ';') ? 3: 0)));
		p = new MathSpaceInset(sp);
		insertInset(p, LM_TC_INSET);
		return;
	} else
		l = in_word_set(s);

	if (!l) {
		p = MathMacroTable::mathMTable.getMacro(s);
		if (!p) {
		lyxerr[Debug::MATHED] << "Macro2 " << s << ' ' << tcode << endl;
		if (s == "root") {
			p = new MathRootInset;
			tcode = LM_TC_ACTIVE_INSET;
		} else
			p = new MathFuncInset(s, LM_OT_UNDEF);
		} else {
			tcode = static_cast<MathMacro*>(p)->getTCode();
			lyxerr[Debug::MATHED] << "Macro2 " << s << ' ' << tcode << endl;
		}
	} else {
		MathedInsetTypes fractype = LM_OT_FRAC;
		switch (l->token) {
			case LM_TK_BIGSYM: 
					p = new MathBigopInset(l->name, l->id);
					break;
				
			case LM_TK_SYM: 
					if (l->id<255) {
						Insert(static_cast<byte>(l->id), MathIsBOPS(l->id) ?
						LM_TC_BOPS: LM_TC_SYMB); 	
					} else {
						p = new MathFuncInset(l->name);
					}
					break;
				
			case LM_TK_STACK:
				fractype = LM_OT_STACKREL;
				lyxerr[Debug::MATHED] << " i:stackrel " << endl;

			case LM_TK_FRAC:
				p = new MathFracInset(fractype);
				tcode = LM_TC_ACTIVE_INSET;
				break;

			case LM_TK_SQRT:
				p = new MathSqrtInset;
				tcode = LM_TC_ACTIVE_INSET;
				break;

			case LM_TK_WIDE:
				p = new MathDecorationInset(l->id);
				tcode = LM_TC_ACTIVE_INSET;
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
				setAccent(l->id);
				break;

			case LM_TK_MACRO:
				p = MathMacroTable::mathMTable.getMacro(s);
				tcode = static_cast<MathMacro*>(p)->getTCode();
				lyxerr[Debug::MATHED] << "Macro " << s << ' ' << tcode << endl;
				break;

			default:
				p = new MathFuncInset(l->name);
				break;
		}
	}

	if (p) {
		insertInset(p, tcode);
		par->Metrics();
	}
}


bool MathedCursor::pullArg()
{
	if (cursor->IsActive()) {
		MathParInset * p = cursor->GetActiveInset();
		if (!p) 
			return false;
		
		MathedArray a = p->GetData();
		p->clear();
		Delete();
		if (!a.empty()) {
			cursor->Merge(a);
			cursor->Adjust();
		}

		return true;
	}
	return false;
}


void MathedCursor::MacroModeOpen()
{
	if (!macro_mode) {
		imacro = new MathFuncInset("");
		insertInset(imacro, LM_TC_INSET);
		macro_mode = true;
	} else
		lyxerr << "Mathed Warning: Already in macro mode" << endl;
}


void MathedCursor::MacroModeClose()
{
	if (macro_mode)  {
		macro_mode = false;
		latexkeys const * l = in_word_set(imacro->GetName());
		if (!imacro->GetName().empty()
		&& (!l || (l && IsMacro(l->token, l->id))) &&
		!MathMacroTable::mathMTable.getMacro(imacro->GetName())) {
			if (!l) {
				//imacro->SetName(macrobf);
				// This guarantees that the string will be removed by destructor
				imacro->SetType(LM_OT_UNDEF);
			} else
				imacro->SetName(l->name);
		} else {
			Left();
			if (cursor->GetInset()->GetType() == LM_OT_ACCENT) {
				setAccent(
					static_cast<MathAccentInset*>(cursor->GetInset())->getAccentCode());
			}
			cursor->Delete();
			if (l || MathMacroTable::mathMTable.getMacro(imacro->GetName())) {
				Interpret(imacro->GetName());
			}
			imacro->SetName("");
		}
		imacro = 0;
	}
}


void MathedCursor::SelCopy()
{
	if (selection) {
		int p1 = (cursor->getPos() < selpos) ? cursor->getPos() : selpos;
		int p2 = (cursor->getPos() > selpos) ?
			cursor->getPos() : selpos;
		selarray = *(cursor->GetData());
		selarray.shrink(p1, p2);
		cursor->Adjust();
		SelClear();
	}
}


void MathedCursor::SelCut()
{
	if (selection) {
		if (cursor->getPos() == selpos)
			return;

		int p1 = (cursor->getPos() < selpos) ? cursor->getPos() : selpos;
		int p2 = (cursor->getPos() > selpos) ? cursor->getPos() : selpos;
		selarray = *(cursor->GetData());
		selarray.shrink(p1, p2);
		cursor->Clean(selpos);
		cursor->Adjust();
		SelClear();
	}
}


void MathedCursor::SelDel()
{
	//  lyxerr << "Deleting sel "
	if (selection) {
		if (cursor->getPos() == selpos)
			return;
		cursor->Clean(selpos);
		cursor->Adjust();
		SelClear();
	}
}


void MathedCursor::SelPaste()
{
	// lyxerr << "paste " << selarray << " " << curor->pos;
	if (selection)
		SelDel();

	if (!selarray.empty()) {
		cursor->Merge(selarray);
		cursor->Adjust();
	}
}


void MathedCursor::SelStart()
{
	lyxerr[Debug::MATHED] << "Starting sel " << endl;
	if (!anchor) {
		selpos = cursor->getPos();
		selstk = new MathStackXIter(mathstk);
		anchor = selstk->Item(-1);
		anchor->SetData(cursor->getPar());
		anchor->GoBegin();
		anchor->goPosAbs(selpos);
		selection = true;
	}
}


void MathedCursor::SelClear()
{
	lyxerr[Debug::MATHED] << "Clearing sel " << endl;
	selection = false;
	delete selstk;
	selstk = 0;
	anchor = 0;
}



// Anchor position must be at the same level that stack.
void MathedCursor::SelBalance()
{
	int d = mathstk.Level() - selstk->Level();

	// If unbalanced, balance them
	while (d != 0) {
		if (d < 0) {
			// lyxerr << "b[" << mathstk.Level() << " " << selstk->Level
			//  << " " << anchor->GetX() << " " << cursor->GetX() << "]";
			anchor = selstk->pop();
			if (anchor->GetX() >= cursor->GetX())
			anchor->Next();
		} else {
			// lyxerr <<"a[" << mathstk.Level() << " " << selstk->Level() <<"]";
			Pop();
		}
		d = mathstk.Level() - selstk->Level();
	}

	// Once balanced the levels, check that they are at the same paragraph
	selpos = anchor->getPos();
}


void MathedCursor::SelGetArea(int ** xp, int ** yp, int & np)
{
	static int xpoint[10];
	static int ypoint[10];

	if (!selection) {
		np = 0;
		xpoint[0] = 0;
		ypoint[0] = 0;
		*xp = &xpoint[0];
		*yp = &ypoint[0];
		return;
	}

	// Balance anchor and cursor
	SelBalance();

	int xo;
	int yo;
	cursor->getPar()->GetXY(xo, yo);
	int w = cursor->getPar()->Width();
	int x1;
	int y1;
	cursor->GetPos(x1, y1);
	int a1;
	int d1;
	cursor->getAD(a1, d1);
	int x;
	int y;
	anchor->GetPos(x, y);
	int a;
	int d;
	anchor->getAD(a, d);

	// single row selection
	int i = 0;
	xpoint[i]   = x;
	ypoint[i++] = y + d;
	xpoint[i]   = x;
	ypoint[i++] = y - a;

	if (y != y1) {
		xpoint[i]   = xo + w;
		ypoint[i++] = y - a;

		if (x1 < xo + w) {
		xpoint[i]   = xo + w;
		ypoint[i++] = y1 - a;
	}
	}

	xpoint[i]   = x1;
	ypoint[i++] = y1 - a;
	xpoint[i]   = x1;
	ypoint[i++] = y1 + d;

	if (y != y1) {
		xpoint[i]   = xo;
		ypoint[i++] = y1 + d;
		if (x > xo) {
			xpoint[i]   = xo;
			ypoint[i++] = y + d;
		}
	}
	xpoint[i]   = xpoint[0];
	ypoint[i++] = ypoint[0];

	*xp = &xpoint[0];
	*yp = &ypoint[0];
	np = i;
	//    lyxerr << "AN[" << x << " " << y << " " << x1 << " " << y1 << "] ";
	//    lyxerr << "MT[" << a << " " << d << " " << a1 << " " << d1 << "] ";
	//    for (i = 0; i < np; ++i)
	//      lyxerr << "XY[" << point[i].x << " " << point[i].y << "] ";
}


void MathedCursor::setAccent(int ac)
{
	if (ac > 0 && accent < 8) {
		nestaccent[accent++] = ac;
	} else
		accent = 0;  // consumed!
}


int MathedCursor::getAccent() const
{
	return (accent > 0) ? nestaccent[accent - 1]: 0;
}


void MathedCursor::doAccent(byte c, MathedTextCodes t)
{
	MathedInset * ac = 0;

	for (int i = accent - 1; i >= 0; --i) {
		if (i == accent - 1)
			ac = new MathAccentInset(c, t, nestaccent[i]);
		else
			ac = new MathAccentInset(ac, nestaccent[i]);
	}
	
	if (ac)
		cursor->insertInset(ac, LM_TC_INSET);

	accent = 0;  // consumed!
}


void MathedCursor::doAccent(MathedInset * p)
{
	MathedInset * ac = 0;

	for (int i = accent - 1; i >= 0; --i) {
		if (i == accent - 1)
			ac = new MathAccentInset(p, nestaccent[i]);
		else
			ac = new MathAccentInset(ac, nestaccent[i]);
	}

	if (ac)
		cursor->insertInset(ac, LM_TC_INSET);

	accent = 0;  // consumed!
}


void MathedCursor::toggleLastCode(MathedTextCodes t)
{
	if (lastcode == t)
		lastcode = LM_TC_VAR;
	else
		lastcode = t;
}


void MathedCursor::GetPos(int & x, int & y)
{
	cursor->GetPos(x, y);
}


short MathedCursor::GetFCode()
{
	return cursor->fcode();
}


MathParInset * MathedCursor::GetPar()
{
	return par;
}


MathParInset * MathedCursor::getCurrentPar() const
{
	return cursor->getPar();
}


string const & MathedCursor::getLabel() const
{
	return cursor->getLabel();
}


bool MathedCursor::IsEnd() const
{
	return !cursor->OK();
}


bool MathedCursor::InMacroMode()
{
	return macro_mode;
}


bool MathedCursor::Selection()
{
	return selection;
}


void MathedCursor::clearLastCode()
{
	lastcode = LM_TC_MIN;
}


void MathedCursor::setLastCode(MathedTextCodes t)
{
	lastcode = t;
}


MathedTextCodes MathedCursor::getLastCode() const
{
	return lastcode;
}
