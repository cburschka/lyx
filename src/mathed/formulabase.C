/*
*  File:        formula.C
*  Purpose:     Implementation of formula inset
*  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
*  Created:     January 1996
*  Description: Allows the edition of math paragraphs inside Lyx.
*
*  Copyright: 1996-1998 Alejandro Aguilar Sierra
*
*  Version: 0.4, Lyx project.
*
*   You are free to use and modify this code under the terms of
*   the GNU General Public Licence version 2 or later.
*/

#include <config.h>
#include <fstream>

#include "Lsstream.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "formula.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "support/LOstream.h"
#include "LyXView.h"
#include "Painter.h"
#include "font.h"
#include "math_arrayinset.h"
#include "math_spaceinset.h"
#include "math_deliminset.h"
#include "support/lyxlib.h"
#include "mathed/support.h"

using namespace std;

extern char const * latex_special_chars;

int greek_kb_flag = 0;
extern char const * latex_mathenv[];
LyXFont           * Math_Fonts = 0;
MathCursor        * mathcursor = 0;


namespace {


// local global
int sel_x;
int sel_y;
bool sel_flag;

void mathed_init_fonts();

string nicelabel(string const & label)
{
	return label.empty() ? string("(#)") : "(" + label + ")";
}

} // namespaces



LyXFont WhichFont(short type, int size)
{
	LyXFont f;
	
	if (!Math_Fonts)
		mathed_init_fonts();

	switch (type) {
	case LM_TC_SYMB:	
		f = Math_Fonts[2];
		break;

	case LM_TC_BSYM:	
		f = Math_Fonts[2];
		break;

	case LM_TC_VAR:
	case LM_TC_IT:
		f = Math_Fonts[0];
		break;

	case LM_TC_BF:
		f = Math_Fonts[3];
		break;

	case LM_TC_SF:
		f = Math_Fonts[7];
		break;

	case LM_TC_CAL:
		f = Math_Fonts[4];
		break;

	case LM_TC_TT:
		f = Math_Fonts[5];
		break;

	case LM_TC_SPECIAL: //f = Math_Fonts[0]; break;
	case LM_TC_TEXTRM:
	case LM_TC_RM:
		f = Math_Fonts[6];
		break;

	default:
		f = Math_Fonts[1];
		break;
	}

	switch (size) {
	case LM_ST_DISPLAY:
		if (type == LM_TC_BSYM) {
			f.incSize();
			f.incSize();
		}
		break;

	case LM_ST_TEXT:
		break;

	case LM_ST_SCRIPT:
		f.decSize();
		break;

	case LM_ST_SCRIPTSCRIPT:
		f.decSize();
		f.decSize();
		break;

	default:
		lyxerr << "Math Error: wrong font size: " << size << endl;
		break;
	}

	if (type != LM_TC_TEXTRM)
		f.setColor(LColor::math);

	return f;
}


namespace {

void mathed_init_fonts()
{
	Math_Fonts = new LyXFont[8]; //DEC cxx cannot initialize all fonts
	//at once (JMarc) rc

	for (int i = 0 ; i < 8 ; ++i) {
		Math_Fonts[i] = LyXFont(LyXFont::ALL_SANE);
	}

	Math_Fonts[0].setShape(LyXFont::ITALIC_SHAPE);

	Math_Fonts[1].setFamily(LyXFont::SYMBOL_FAMILY);

	Math_Fonts[2].setFamily(LyXFont::SYMBOL_FAMILY);
	Math_Fonts[2].setShape(LyXFont::ITALIC_SHAPE);

	Math_Fonts[3].setSeries(LyXFont::BOLD_SERIES);

	Math_Fonts[4].setFamily(LyXFont::SANS_FAMILY);
	Math_Fonts[4].setShape(LyXFont::ITALIC_SHAPE);

	Math_Fonts[5].setFamily(LyXFont::TYPEWRITER_FAMILY);

	Math_Fonts[6].setFamily(LyXFont::ROMAN_FAMILY);

	Math_Fonts[7].setFamily(LyXFont::SANS_FAMILY);
}


// returns the nearest enclosing matrix
MathArrayInset * matrixpar(int & idx)
{
	idx = 0;
	return
		static_cast<MathArrayInset *> 
			(mathcursor ? mathcursor->enclosing(LM_OT_MATRIX, idx) : 0); 
}


} // namespace anon


InsetFormulaBase::InsetFormulaBase(MathInset * par)
	: par_(par)
{}

InsetFormulaBase::InsetFormulaBase(InsetFormulaBase const & f)
	: UpdatableInset(f), par_(static_cast<MathInset *>(f.par_->Clone()))
{}

InsetFormulaBase::~InsetFormulaBase()
{
#ifdef WITH_WARNINGS
#warning leak this for a while...
#endif
	//delete par_;
}


void InsetFormulaBase::Write(Buffer const * buf, ostream & os) const
{
	os << "Formula ";
	Latex(buf, os, false, false);
}


int InsetFormulaBase::Ascii(Buffer const *, ostream & os, int) const
{
	par_->Write(os, false);
	return 0;
}


int InsetFormulaBase::Linuxdoc(Buffer const * buf, ostream & os) const
{
	return Ascii(buf, os, 0);
}


int InsetFormulaBase::DocBook(Buffer const * buf, ostream & os) const
{
	return Ascii(buf, os, 0);
}


// Check if uses AMS macros
void InsetFormulaBase::Validate(LaTeXFeatures &) const
{}


string const InsetFormulaBase::EditMessage() const
{
	return _("Math editor mode");
}


void InsetFormulaBase::Edit(BufferView * bv, int x, int /*y*/, unsigned int)
{
	mathcursor = new MathCursor(this);

	if (!bv->lockInset(this))
		lyxerr[Debug::MATHED] << "Cannot lock inset!!!" << endl;

	par_->Metrics(LM_ST_TEXT);
	bv->updateInset(this, false);
	if (x == 0) {
		mathcursor->first();
	} else {
		mathcursor->last();
	}
	sel_x = 0;
	sel_y = 0;
	sel_flag = false;
}


void InsetFormulaBase::InsetUnlock(BufferView * bv)
{
	if (mathcursor) {
		if (mathcursor->InMacroMode()) {
			mathcursor->MacroModeClose();
			UpdateLocal(bv);
		}
		delete mathcursor;
	}
	mathcursor = 0;
	bv->updateInset(this, false);
}




void InsetFormulaBase::GetCursorPos(BufferView *, int & x, int & y) const
{
	mathcursor->GetPos(x, y);
	x -= par_->xo();
	y -= par_->yo();
}


void InsetFormulaBase::ToggleInsetCursor(BufferView * bv)
{
	if (!mathcursor)
		return;

	if (isCursorVisible())
		bv->hideLockedInsetCursor();
	else {
		int x;
		int y;
		mathcursor->GetPos(x, y);
		//x -= par_->xo();
		y -= par_->yo();

		LyXFont   font = WhichFont(LM_TC_TEXTRM, LM_ST_TEXT);
		int const asc  = lyxfont::maxAscent(font);
		int const desc = lyxfont::maxDescent(font);

		bv->showLockedInsetCursor(x, y, asc, desc);
	}

	toggleCursorVisible();
}


void InsetFormulaBase::ShowInsetCursor(BufferView * bv, bool)
{
	if (!isCursorVisible()) {
		if (mathcursor) {
			int x;
			int y;
			mathcursor->GetPos(x, y);
			x -= par_->xo();
			y -= par_->yo();
			LyXFont font   = WhichFont(LM_TC_TEXTRM, LM_ST_TEXT);
			int const asc  = lyxfont::maxAscent(font);
			int const desc = lyxfont::maxDescent(font);
			bv->fitLockedInsetCursor(x, y, asc, desc);
		}
		ToggleInsetCursor(bv);
	}
}


void InsetFormulaBase::HideInsetCursor(BufferView * bv)
{
	if (isCursorVisible())
		ToggleInsetCursor(bv);
}


void InsetFormulaBase::ToggleInsetSelection(BufferView * bv)
{
	if (!mathcursor)
		return;

	bv->updateInset(this, false);
}



vector<string> const InsetFormulaBase::getLabelList() const
{
  return std::vector<string>();
}


void InsetFormulaBase::UpdateLocal(BufferView * bv)
{
	par_->Metrics(LM_ST_TEXT);
	bv->updateInset(this, true);
}


void InsetFormulaBase::InsetButtonRelease(BufferView * bv,
				      int x, int y, int /*button*/)
{
	if (mathcursor) {
		HideInsetCursor(bv);
		x += par_->xo();
		y += par_->yo();
		mathcursor->SetPos(x, y);
		ShowInsetCursor(bv);
		if (sel_flag) {
			sel_flag = false;
			sel_x = 0;
			sel_y = 0;
			bv->updateInset(this, false);
		}
	}
}


void InsetFormulaBase::InsetButtonPress(BufferView * bv,
		int x, int y, int /*button*/)
{
	sel_flag = false;
	sel_x = x;
	sel_y = y;
	if (mathcursor && mathcursor->Selection()) {
		mathcursor->SelClear();
		bv->updateInset(this, false);
	}
}


void InsetFormulaBase::InsetMotionNotify(BufferView * bv,
		int x, int y, int /*button*/)
{
	if (sel_x && sel_y && abs(x-sel_x) > 4 && !sel_flag) {
		sel_flag = true;
		HideInsetCursor(bv);
		mathcursor->SetPos(sel_x + par_->xo(), sel_y + par_->yo());
		mathcursor->SelStart();
		ShowInsetCursor(bv);
		mathcursor->GetPos(sel_x, sel_y);
	} else if (sel_flag) {
		HideInsetCursor(bv);
		x += par_->xo();
		y += par_->yo();
		mathcursor->SetPos(x, y);
		ShowInsetCursor(bv);
		mathcursor->GetPos(x, y);
		if (sel_x != x || sel_y != y)
			bv->updateInset(this, false);
		sel_x = x;
		sel_y = y;
	}
}


void InsetFormulaBase::InsetKeyPress(XKeyEvent *)
{
	lyxerr[Debug::MATHED] << "Used InsetFormulaBase::InsetKeyPress." << endl;
}


UpdatableInset::RESULT
InsetFormulaBase::LocalDispatch(BufferView * bv, kb_action action,
			    string const & arg)
{
	//lyxerr << "InsetFormulaBase::LocalDispatch: act: " << action
	//	<< " arg: '" << arg << "' cursor: " << mathcursor << "\n";
	//   extern char *dispatch_result;

	if (!mathcursor) 
		return UNDISPATCHED;

	MathTextCodes varcode = LM_TC_MIN;
	bool was_macro = mathcursor->InMacroMode();
	bool sel = false;
	bool space_on = false;
	bool was_selection = mathcursor->Selection();
	RESULT result = DISPATCHED;
	static MathSpaceInset * sp = 0;

	HideInsetCursor(bv);

	if (mathcursor->getLastCode() == LM_TC_TEX)
		varcode = LM_TC_TEX;

	mathcursor->normalize();

	switch (action) {

		// --- Cursor Movements ---------------------------------------------

	case LFUN_RIGHTSEL:
		sel = true; // fall through...

	case LFUN_RIGHT:
		result = DISPATCH_RESULT(mathcursor->Right(sel));
		UpdateLocal(bv);
		break;


	case LFUN_LEFTSEL:
		sel = true; // fall through

	case LFUN_LEFT:
		result = DISPATCH_RESULT(mathcursor->Left(sel));
		UpdateLocal(bv);
		break;


	case LFUN_UPSEL:
		sel = true;

	case LFUN_UP:
		result = DISPATCH_RESULT(mathcursor->Up(sel));
		UpdateLocal(bv);
		break;


	case LFUN_DOWNSEL:
		sel = true;

	case LFUN_DOWN:
		result = DISPATCH_RESULT(mathcursor->Down(sel));
		UpdateLocal(bv);
		break;


	case LFUN_HOME:
		mathcursor->Home();
		result = DISPATCHED_NOUPDATE;
		break;

	case LFUN_END:
		mathcursor->End();
		result = DISPATCHED_NOUPDATE;
		break;

	case LFUN_DELETE_LINE_FORWARD:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->DelLine();
		UpdateLocal(bv);
		break;

	case LFUN_TAB:
		bv->lockedInsetStoreUndo(Undo::INSERT);
		mathcursor->idxRight();
		UpdateLocal(bv);
		break;

	case LFUN_TABINSERT:
		bv->lockedInsetStoreUndo(Undo::INSERT);
		mathcursor->idxRight();
		UpdateLocal(bv);
		break;

	case LFUN_BACKSPACE:
		if (!mathcursor->InMacroMode() && mathcursor->pos() == 0) {
			bv->lockedInsetStoreUndo(Undo::DELETE);
			mathcursor->pullArg();
			bv->updateInset(this, true);
			break;
		}
		if (!mathcursor->Left())
			break;

		// fall through...

	case LFUN_DELETE:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->Delete();
		bv->updateInset(this, true);
		break;

		//    case LFUN_GETXY:
		//      sprintf(dispatch_buffer, "%d %d",);
		//      dispatch_result = dispatch_buffer;
		//      break;
	case LFUN_SETXY:
	{
		lyxerr << "LFUN_SETXY broken!\n";
		int x, y, x1, y1;
		istringstream is(arg.c_str());
		is >> x >> y;
		lyxerr << "LFUN_SETXY: x: " << x << " y: " << y << "\n";
		par_->GetXY(x1, y1);
		mathcursor->SetPos(x1 + x, y1 + y);
	}
	break;

		// cursor selection ---------------------------- 

	case LFUN_PASTE:
		if (was_macro)
			mathcursor->MacroModeClose();
		bv->lockedInsetStoreUndo(Undo::INSERT);
		mathcursor->SelPaste();
		UpdateLocal(bv);
		break;

	case LFUN_CUT:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->SelCut();
		UpdateLocal(bv);
		break;

	case LFUN_COPY:
		mathcursor->SelCopy();
		break;

	case LFUN_HOMESEL:
	case LFUN_ENDSEL:
	case LFUN_WORDRIGHTSEL:
	case LFUN_WORDLEFTSEL:
		break;

		// --- accented characters ------------------------------

	case LFUN_UMLAUT:     mathcursor->setAccent(LM_ddot); break;
	case LFUN_CIRCUMFLEX: mathcursor->setAccent(LM_hat); break;
	case LFUN_GRAVE:      mathcursor->setAccent(LM_grave); break;
	case LFUN_ACUTE:      mathcursor->setAccent(LM_acute); break;
	case LFUN_TILDE:      mathcursor->setAccent(LM_tilde); break;
	case LFUN_MACRON:     mathcursor->setAccent(LM_bar); break;
	case LFUN_DOT:        mathcursor->setAccent(LM_dot); break;
	case LFUN_CARON:      mathcursor->setAccent(LM_check); break;
	case LFUN_BREVE:      mathcursor->setAccent(LM_breve); break;
	case LFUN_VECTOR:     mathcursor->setAccent(LM_vec); break;

		// Greek mode
	case LFUN_GREEK:
		if (!greek_kb_flag) {
			greek_kb_flag = 1;
			bv->owner()->message(_("Math greek mode on"));
		} else
			greek_kb_flag = 0;
		break;

		// Greek keyboard
	case LFUN_GREEK_TOGGLE:
		greek_kb_flag = greek_kb_flag ? 0 : 2;
		if (greek_kb_flag)
			bv->owner()->message(_("Math greek keyboard on"));
		else
			bv->owner()->message(_("Math greek keyboard off"));
		break;

		//  Math fonts
	case LFUN_BOLD:  mathcursor->toggleLastCode(LM_TC_BF); break;
	case LFUN_SANS:  mathcursor->toggleLastCode(LM_TC_SF); break;
	case LFUN_EMPH:  mathcursor->toggleLastCode(LM_TC_CAL); break;
	case LFUN_ROMAN: mathcursor->toggleLastCode(LM_TC_RM); break;
	case LFUN_CODE:  mathcursor->toggleLastCode(LM_TC_TT); break;
	case LFUN_DEFAULT:  mathcursor->setLastCode(LM_TC_VAR); break;

	case LFUN_TEX:
		// varcode = LM_TC_TEX;
		mathcursor->setLastCode(LM_TC_TEX);
		bv->owner()->message(_("TeX mode"));
		break;

	case LFUN_MATH_LIMITS:
		bv->lockedInsetStoreUndo(Undo::INSERT);
		if (mathcursor->toggleLimits())
			UpdateLocal(bv);
		break;

	case LFUN_MATH_SIZE:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::INSERT);
			latexkeys const * l = in_word_set(arg);
			mathcursor->SetSize(MathStyles(l ? l->id : -1));
			UpdateLocal(bv);
		}
		break;

	case LFUN_INSERT_MATH:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::INSERT);
			mathcursor->Interpret(arg);
			UpdateLocal(bv);
		}
		break;


	case LFUN_INSERT_MATRIX:
		if (mathcursor) {
			bv->lockedInsetStoreUndo(Undo::INSERT);
			int m = 1;
			int n = 1;
			string v_align, h_align;
			istringstream is(arg.c_str());
			is >> m >> n >> v_align >> h_align;
			MathArrayInset * p = new MathArrayInset(m, n);
			p->valign(v_align[0]);
			p->halign(h_align);
			mathcursor->insert(p);
			UpdateLocal(bv);
		}
		break;

	case LFUN_MATH_DELIM:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		int ilt = '(';
		int irt = '.';
		static const string vdelim("(){}[]./|");
		lyxerr << "formulabase::LFUN_MATH_DELIM, arg: '" << arg << "'\n";

		if (arg.empty())
			break;

		istringstream is(arg.c_str());
		string lt, rt;
		is >> lt >> rt;
		lyxerr << "formulabase::LFUN_MATH_DELIM, lt: '" << lt << "'\n";
		lyxerr << "formulabase::LFUN_MATH_DELIM, rt: '" << rt << "'\n";

		if (lt.size() > 1) {
			latexkeys const * l = in_word_set(lt);
			if (l)
				ilt = l->id;
		} else if (vdelim.find(lt[0]) != string::npos)
				ilt = lt[0];

		if (rt.size() > 1) {
			latexkeys const * l = in_word_set(rt);
			if (l)
				irt = l->id;
		} else if (vdelim.find(rt[0]) != string::npos)
				irt = rt[0];

		if (mathcursor->selection) {
			MathDelimInset * p = new MathDelimInset(ilt, irt);
			MathArray ar;
		 	mathcursor->selArray(ar);
			lyxerr << "selarray: " << ar << "\n";
			p->cell(0) = ar; 
			mathcursor->insert(p);
		} else {
			mathcursor->insert(new MathDelimInset(ilt, irt));
		}
		UpdateLocal(bv);
		break;
	}

	case LFUN_PROTECTEDSPACE:
		bv->lockedInsetStoreUndo(Undo::INSERT);
		mathcursor->insert(new MathSpaceInset(1));
		space_on = true;
		UpdateLocal(bv);
		break;

		// Invalid actions under math mode
	case LFUN_MATH_MODE:
		if (mathcursor->getLastCode() != LM_TC_TEXTRM) {
			bv->owner()->message(_("math text mode"));
			varcode = LM_TC_TEXTRM;
		} else 
			varcode = LM_TC_VAR;
		mathcursor->setLastCode(varcode);
		break;

	case LFUN_UNDO:
		bv->owner()->message(_("Invalid action in math mode!"));
		break;


	case LFUN_MATH_HALIGN:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		lyxerr << "handling halign '" << arg << "'\n";
		int idx;
		MathArrayInset * p = matrixpar(idx);
		if (!p)
			break; 
		p->halign(arg.size() ? arg[0] : 'c', p->col(idx));
		UpdateLocal(bv);
		break;
	}

	case LFUN_MATH_VALIGN:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		lyxerr << "handling valign '" << arg << "'\n";
		int idx;
		MathArrayInset * p = matrixpar(idx);
		if (!p)
			break; 
		p->valign(arg.size() ? arg[0] : 'c');
		UpdateLocal(bv);
		break;
	}

	case LFUN_MATH_ROW_INSERT:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		int idx;
		MathArrayInset * p = matrixpar(idx);
		lyxerr << " calling LFUN_MATH_ROW_INSERT on " << p << endl;
		if (!p)
			break; 
		p->addRow(p->row(idx));
		UpdateLocal(bv);
		break;
	}

	case LFUN_MATH_ROW_DELETE:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		int idx;
		MathArrayInset * p = matrixpar(idx);
		lyxerr << " calling LFUN_MATH_ROW_DELETE on " << p << endl;
		if (!p)
			break; 
		p->delRow(p->row(idx));
		UpdateLocal(bv);
		break;
	}

	case LFUN_MATH_COLUMN_INSERT:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		int idx;
		MathArrayInset * p = matrixpar(idx);
		if (!p)
			break; 
		p->addCol(p->col(idx));
		UpdateLocal(bv);
		break;
	}

	case LFUN_MATH_COLUMN_DELETE:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		int idx;
		MathArrayInset * p = matrixpar(idx);
		if (!p)
			break; 
		p->delCol(p->col(idx));
		UpdateLocal(bv);
		break;
	}

	case LFUN_EXEC_COMMAND:
		result = UNDISPATCHED;
		break;


		//------- dummy actions
#ifdef WITH_WARNINGS
#warning Is this needed here? Shouldnt the main dispatch handle this? (Lgb)
#endif
		//case LFUN_EXEC_COMMAND:
		//bv->owner()->getMiniBuffer()->PrepareForCommand();
		//break;

	default:
		if ((action == -1  || action == LFUN_SELFINSERT)
		    && !arg.empty())  {
			unsigned char c = arg[0];
			bv->lockedInsetStoreUndo(Undo::INSERT);

			if (c == ' ' && mathcursor->getAccent() == LM_hat) {
				c = '^';
				mathcursor->setAccent(0);
			}

			if (c == 0) {      // Dead key, do nothing
				//lyxerr << "deadkey" << endl;
				break;
			}

			if (isalpha(c)) {
				if (mathcursor->getLastCode() == LM_TC_TEX) {
					mathcursor->MacroModeOpen();
					mathcursor->clearLastCode();
					varcode = LM_TC_MIN;
				} else if (!varcode) {		
					short f = mathcursor->getLastCode() ?
						mathcursor->getLastCode() :
						mathcursor->nextCode();
					varcode = MathIsAlphaFont(f) ?
						static_cast<MathTextCodes>(f) :
						LM_TC_VAR;
				}
				
				//	     lyxerr << "Varcode << vardoce;
				MathTextCodes char_code = varcode;
				if (greek_kb_flag) {
					char greek[26] =
					{'A', 'B', 'X',  0 , 'E',  0 ,  0 , 'H', 'I',  0 ,
					 'K',  0 , 'M', 'N', 'O',  0 ,  0 , 'P',  0 , 'T',
					 'Y',  0,   0,   0,   0 , 'Z' };
					
					if ('A' <= c && c <= 'Z' && greek[c - 'A']) {
						char_code = LM_TC_RM;
						c = greek[c - 'A'];
					} else
						char_code = LM_TC_SYMB;
				}
				
				mathcursor->insert(c, char_code);
				
				if (greek_kb_flag && char_code == LM_TC_RM )
					mathcursor->setLastCode(LM_TC_VAR);
				
				varcode = LM_TC_MIN;
				
				if (greek_kb_flag < 2)
					greek_kb_flag = 0;
				
			} else if (strchr("!,:;{}", c) && (varcode == LM_TC_TEX||was_macro)) {
				mathcursor->insert(c, LM_TC_TEX);
				if (c == '{') {
					mathcursor->insert('}', LM_TC_TEX);
					mathcursor->Left();
				}
				mathcursor->clearLastCode();
				//	       varcode = LM_TC_MIN;
			} else if (c == '_' && varcode == LM_TC_TEX) {
				mathcursor->insert(c, LM_TC_SPECIAL);
				mathcursor->clearLastCode();
				//	       varcode = LM_TC_MIN;
			} else if ('0' <= c && c <= '9' && (varcode == LM_TC_TEX||was_macro)) {
				mathcursor->MacroModeOpen();
				mathcursor->clearLastCode();
				mathcursor->insert(c, LM_TC_MIN);
			} else if (('0' <= c && c <= '9') || strchr(";:!|[]().,?", c)) {
				mathcursor->insert(c, LM_TC_CONST);
			} else if (strchr("+/-*<>=", c)) {
				mathcursor->insert(c, LM_TC_BOP);
			} else if (strchr(latex_special_chars, c) && c!= '_') {
				mathcursor->insert(c, LM_TC_SPECIAL);
			} else if (c == '_' || c == '^') {
				char s[2];
				s[0] = c;
				s[1] = 0;
				mathcursor->Interpret(s);
			} else if (c == ' ') {	
				if (!varcode) {	
					short f = (mathcursor->getLastCode()) ?
						mathcursor->getLastCode() :
						mathcursor->nextCode();
					varcode = MathIsAlphaFont(f) ?
						static_cast<MathTextCodes>(f) :
						LM_TC_VAR;
				}
				
				if (varcode == LM_TC_TEXTRM) {
					mathcursor->insert(c, LM_TC_TEXTRM);
				} else if (was_macro) {
					mathcursor->MacroModeClose();
				} else if (sp) {
					int isp = (sp->GetSpace()<5) ? sp->GetSpace()+1: 0;
					sp->SetSpace(isp);
					space_on = true;
				} else {
					lyxerr << "look here!\n";
					//if (!mathcursor->cursor.pop() && !mathcursor->cursor.OK())
					result = FINISHED;
				}
			} else if (c == '\'' || c == '@') {
				mathcursor->insert (c, LM_TC_VAR);
			} else if (c == '\\') {
				if (was_macro)
					mathcursor->MacroModeClose();
				bv->owner()->message(_("TeX mode"));
				mathcursor->setLastCode(LM_TC_TEX);
			}
			UpdateLocal(bv);
		} else if (action == LFUN_MATH_PANEL) {
			result = UNDISPATCHED;
		} else {
			lyxerr << "Closed by action " << action << endl;
			result =  FINISHED;
		}
	}

	if (mathcursor)
		mathcursor->normalize();

	if (mathcursor && was_macro != mathcursor->InMacroMode()
				&& action >= 0
				&& action != LFUN_BACKSPACE) 
  		UpdateLocal(bv);
	
	//if (mathcursor)
	//		UpdateLocal(bv);

	if (sp && !space_on)
		sp = 0;

	if (mathcursor && (mathcursor->Selection() || was_selection))
		ToggleInsetSelection(bv);

	if (result == DISPATCHED || result == DISPATCHED_NOUPDATE ||
	    result == UNDISPATCHED)
		ShowInsetCursor(bv);
	else
		bv->unlockInset(this);

	return result;  // original version
}



/* FIXME: math-greek-toggle seems to work OK, but math-greek doesn't turn
 * on greek mode */
bool math_insert_greek(BufferView * bv, char c)
{
	if (!bv->available())
		return false;

	if (!isalpha(c))
		return false;

	string tmp;
	tmp = c;
	if (!bv->theLockingInset() || bv->theLockingInset()->IsTextInset()) {
		int greek_kb_flag_save = greek_kb_flag;
		InsetFormula * new_inset = new InsetFormula();
		bv->beforeChange(bv->text);
		if (!bv->insertInset(new_inset)) {
			delete new_inset;
			return false;
		}
		//Update(1);//BUG
		new_inset->Edit(bv, 0, 0, 0);
		new_inset->LocalDispatch(bv, LFUN_SELFINSERT, tmp);
		if (greek_kb_flag_save < 2) {
			bv->unlockInset(new_inset); // bv->theLockingInset());
#warning someone broke this in bolzano
			//bv->text->cursorRight(bv, true);
		}
	} else
		if (bv->theLockingInset()->LyxCode() == Inset::MATH_CODE ||
				bv->theLockingInset()->LyxCode() == Inset::MATHMACRO_CODE)
			static_cast<InsetFormula*>(bv->theLockingInset())->LocalDispatch(bv, LFUN_SELFINSERT, tmp);
		else
			lyxerr << "Math error: attempt to write on a wrong "
				"class of inset." << endl;
	return true;
}



Inset::Code InsetFormulaBase::LyxCode() const
{
	return Inset::MATH_CODE;
}


LyXFont const InsetFormulaBase::ConvertFont(LyXFont const & f) const
{
	// We have already discussed what was here
	LyXFont font(f);
	font.setLatex(LyXFont::OFF);
	return font;
}

MathInset * InsetFormulaBase::par() const
{
	return par_;
}

