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
#include "formulamacro.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "support.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "Painter.h"
#include "font.h"
#include "math_arrayinset.h"
#include "math_spaceinset.h"
#include "math_scopeinset.h"
#include "math_macrotable.h"
#include "math_factory.h"
#include "support/lyxlib.h"
#include "undo_funcs.h"

using std::endl;
using std::ostream;
using std::vector;

extern char const * latex_mathenv[];
MathCursor        * mathcursor = 0;


namespace {


// local global
int sel_x;
int sel_y;
bool sel_flag;

void mathed_init_fonts();

string nicelabel(string const & label)
{
	return "(" + (label.empty() ? "#" : label) + ")";
}

void handleFont(BufferView * bv, MathTextCodes t) 
{
	if (mathcursor->selection())
		bv->lockedInsetStoreUndo(Undo::EDIT);
	mathcursor->handleFont(t);
}


void handleAccent(BufferView * bv, string const & name)
{
	bv->lockedInsetStoreUndo(Undo::EDIT);
	mathcursor->insert(createMathInset(name));
}


bool openNewInset(BufferView * bv, UpdatableInset * new_inset)
{
	LyXText * lt = bv->getLyXText();
	
	bv->beforeChange(lt);
	finishUndo();
	if (!bv->insertInset(new_inset)) {
		delete new_inset;
		return false;
	}
	new_inset->edit(bv, 0, 0, 0);
	return true;
}


// returns the nearest enclosing grid
MathArrayInset * matrixpar(int & idx)
{
	idx = 0;
	return (mathcursor ? mathcursor->enclosingArray(idx) : 0); 
}


} // namespace anon



InsetFormulaBase::InsetFormulaBase()
{
#ifdef WITH_WARNINGS
#warning This is needed as long the math parser is not re-entrant
#endif
	MathMacroTable::builtinMacros();
	//lyxerr << "sizeof MathInset: " << sizeof(MathInset) << "\n";
}


void InsetFormulaBase::read(Buffer const *, LyXLex & lex)
{
	read(lex);
}


void InsetFormulaBase::write(Buffer const *, ostream & os) const
{
	write(os);
}


int InsetFormulaBase::latex(Buffer const *, ostream & os,
	bool fragile, bool spacing) const
{
	return latex(os, fragile, spacing);
}


int InsetFormulaBase::ascii(Buffer const *, ostream & os, int spacing) const
{
	return ascii(os, spacing);
}


int InsetFormulaBase::linuxdoc(Buffer const *, ostream & os) const
{
	return linuxdoc(os);
}


int InsetFormulaBase::docBook(Buffer const *, ostream & os) const
{
	return docBook(os);
}



// Check if uses AMS macros
void InsetFormulaBase::validate(LaTeXFeatures &) const
{}


string const InsetFormulaBase::editMessage() const
{
	return _("Math editor mode");
}


void InsetFormulaBase::edit(BufferView * bv, int x, int /*y*/, unsigned int)
{
	mathcursor = new MathCursor(this);

	if (!bv->lockInset(this))
		lyxerr[Debug::MATHED] << "Cannot lock inset!!!" << endl;

	metrics();
	// if that is removed, we won't get the magenta box when entering an
	// inset for the first time
	bv->updateInset(this, false);
	if (x == 0)
		mathcursor->first();
	else
		mathcursor->last();
	sel_x = 0;
	sel_y = 0;
	sel_flag = false;
}


void InsetFormulaBase::edit(BufferView * bv, bool front)
{
	// looks hackish but seems to work
	edit(bv, front ? 0 : 1, 0, 0);
}


void InsetFormulaBase::insetUnlock(BufferView * bv)
{
	if (mathcursor) {
		if (mathcursor->inMacroMode()) {
			mathcursor->macroModeClose();
			updateLocal(bv, true);
		}
		delete mathcursor;
		mathcursor = 0;
	}
	bv->updateInset(this, false);
}


void InsetFormulaBase::getCursorPos(BufferView *, int & x, int & y) const
{
	mathcursor->getPos(x, y);
	x -= par()->xo();
	y -= par()->yo();
}


void InsetFormulaBase::toggleInsetCursor(BufferView * bv)
{
	if (!mathcursor)
		return;

	if (isCursorVisible())
		bv->hideLockedInsetCursor();
	else {
		int x;
		int y;
		mathcursor->getPos(x, y);
		//x -= par()->xo();
		y -= par()->yo();
		int asc;
		int desc;
		math_font_max_dim(LM_TC_TEXTRM, LM_ST_TEXT, asc, desc);
		bv->showLockedInsetCursor(x, y, asc, desc);
	}

	toggleCursorVisible();
}


void InsetFormulaBase::showInsetCursor(BufferView * bv, bool)
{
	if (!isCursorVisible()) {
		if (mathcursor) {
			int x;
			int y;
			mathcursor->getPos(x, y);
			x -= par()->xo();
			y -= par()->yo();
			int asc;
			int desc;
			math_font_max_dim(LM_TC_TEXTRM, LM_ST_TEXT, asc, desc);
			bv->fitLockedInsetCursor(x, y, asc, desc);
		}
		toggleInsetCursor(bv);
	}
}


void InsetFormulaBase::hideInsetCursor(BufferView * bv)
{
	if (isCursorVisible())
		toggleInsetCursor(bv);
}


void InsetFormulaBase::toggleInsetSelection(BufferView * bv)
{
	if (mathcursor)
		bv->updateInset(this, false);
}


vector<string> const InsetFormulaBase::getLabelList() const
{
  return std::vector<string>();
}


void InsetFormulaBase::updateLocal(BufferView * bv, bool dirty)
{
	metrics();
	bv->updateInset(this, dirty);
}


void InsetFormulaBase::insetButtonRelease(BufferView * bv,
					  int x, int y, int /*button*/)
{
	if (mathcursor) {
		hideInsetCursor(bv);
		x += par()->xo();
		y += par()->yo();
		mathcursor->setPos(x, y);
		showInsetCursor(bv);
		if (sel_flag) {
			sel_flag = false;
			sel_x = 0;
			sel_y = 0;
		}
		bv->updateInset(this, false);
	}
}


void InsetFormulaBase::insetButtonPress(BufferView * bv,
					int x, int y, int /*button*/)
{
	sel_flag = false;
	sel_x = x;
	sel_y = y;
	if (mathcursor && mathcursor->selection()) {
		mathcursor->selClear();
		bv->updateInset(this, false);
	}
}


void InsetFormulaBase::insetMotionNotify(BufferView * bv,
					 int x, int y, int /*button*/)
{
	if (sel_x && sel_y && abs(x-sel_x) > 4 && !sel_flag) {
		sel_flag = true;
		hideInsetCursor(bv);
		mathcursor->setPos(sel_x + par()->xo(), sel_y + par()->yo());
		mathcursor->selStart();
		showInsetCursor(bv);
		mathcursor->getPos(sel_x, sel_y);
	} else if (sel_flag) {
		hideInsetCursor(bv);
		x += par()->xo();
		y += par()->yo();
		mathcursor->setPos(x, y);
		showInsetCursor(bv);
		mathcursor->getPos(x, y);
		if (sel_x != x || sel_y != y)
			bv->updateInset(this, false);
		sel_x = x;
		sel_y = y;
	}
}


void InsetFormulaBase::insetKeyPress(XKeyEvent *)
{
	lyxerr[Debug::MATHED] << "Used InsetFormulaBase::InsetKeyPress." << endl;
}


int greek_kb_flag = 0;

UpdatableInset::RESULT
InsetFormulaBase::localDispatch(BufferView * bv, kb_action action,
			    string const & arg)
{
	//lyxerr << "InsetFormulaBase::LocalDispatch: act: " << action
	//	<< " arg: '" << arg << "' cursor: " << mathcursor << "\n";


	if (!mathcursor) 
		return UNDISPATCHED;

	RESULT result      = DISPATCHED;
	bool sel           = false;
	bool was_macro     = mathcursor->inMacroMode();
	bool was_selection = mathcursor->selection();

	hideInsetCursor(bv);

	mathcursor->normalize();

	switch (action) {

		// --- Cursor Movements ---------------------------------------------

	case LFUN_RIGHTSEL:
		sel = true; // fall through...

	case LFUN_RIGHT:
		result = mathcursor->right(sel) ? DISPATCHED : FINISHED_RIGHT;
		updateLocal(bv, false);
		break;


	case LFUN_LEFTSEL:
		sel = true; // fall through

	case LFUN_LEFT:
		result = mathcursor->left(sel) ? DISPATCHED : FINISHED;
		updateLocal(bv, false);
		break;


	case LFUN_UPSEL:
		sel = true;

	case LFUN_UP:
		result = mathcursor->up(sel) ? DISPATCHED : FINISHED_UP;
		updateLocal(bv, false);
		break;


	case LFUN_DOWNSEL:
		sel = true;

	case LFUN_DOWN:
		result = mathcursor->down(sel) ? DISPATCHED : FINISHED_DOWN;
		updateLocal(bv, false);
		break;

	case LFUN_HOME:
		mathcursor->home();
		updateLocal(bv, false);
		break;

	case LFUN_END:
		mathcursor->end();
		updateLocal(bv, false);
		break;

	case LFUN_DELETE_LINE_FORWARD:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->delLine();
		updateLocal(bv, true);
		break;

	case LFUN_TAB:
		mathcursor->idxNext();
		updateLocal(bv, false);
		break;

	case LFUN_SHIFT_TAB:
		mathcursor->idxPrev();
		updateLocal(bv, false);
		break;

	case LFUN_TABINSERT:
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->splitCell();
		updateLocal(bv, true);
		break;

	case LFUN_BACKSPACE:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->backspace();
		bv->updateInset(this, true);
		break;

	case LFUN_DELETE:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->erase();
		bv->updateInset(this, true);
		break;

		//    case LFUN_GETXY:
		//      sprintf(dispatch_buffer, "%d %d",);
		//      dispatch_result = dispatch_buffer;
		//      break;
	case LFUN_SETXY: {
		lyxerr << "LFUN_SETXY broken!\n";
		int x;
		int y;
		int x1;
		int y1;
		istringstream is(arg.c_str());
		is >> x >> y;
		par()->getXY(x1, y1);
		mathcursor->setPos(x1 + x, y1 + y);
		updateLocal(bv, false);
		break;
	}

	case LFUN_PASTE:
		if (was_macro)
			mathcursor->macroModeClose();
		bv->lockedInsetStoreUndo(Undo::INSERT);
		mathcursor->selPaste();
		updateLocal(bv, true);
		break;

	case LFUN_CUT:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->selCut();
		updateLocal(bv, true);
		break;

	case LFUN_COPY:
		mathcursor->selCopy();
		break;

	case LFUN_HOMESEL:
	case LFUN_ENDSEL:
	case LFUN_WORDRIGHTSEL:
	case LFUN_WORDLEFTSEL:
		break;

		// --- accented characters ------------------------------

	case LFUN_UMLAUT:       handleAccent(bv, "ddot"); break;
	case LFUN_CIRCUMFLEX:   handleAccent(bv, "hat"); break;
	case LFUN_GRAVE:        handleAccent(bv, "grave"); break;
	case LFUN_ACUTE:        handleAccent(bv, "acute"); break;
	case LFUN_TILDE:        handleAccent(bv, "tilde"); break;
	case LFUN_MACRON:       handleAccent(bv, "bar"); break;
	case LFUN_DOT:          handleAccent(bv, "dot"); break;
	case LFUN_CARON:        handleAccent(bv, "check"); break;
	case LFUN_BREVE:        handleAccent(bv, "breve"); break;
	case LFUN_VECTOR:       handleAccent(bv, "vec"); break;

	//  Math fonts
	case LFUN_GREEK:        handleFont(bv, LM_TC_GREEK1); break;
	case LFUN_GREEK_TOGGLE: handleFont(bv, LM_TC_GREEK); break;
	case LFUN_BOLD:         handleFont(bv, LM_TC_BF); break;
	case LFUN_SANS:         handleFont(bv, LM_TC_SF); break;
	case LFUN_EMPH:         handleFont(bv, LM_TC_CAL); break;
	case LFUN_ROMAN:        handleFont(bv, LM_TC_RM); break;
	case LFUN_CODE:         handleFont(bv, LM_TC_TT); break;
	case LFUN_DEFAULT:      handleFont(bv, LM_TC_VAR); break;

	case LFUN_MATH_MODE:
		handleFont(bv, LM_TC_TEXTRM);
		//bv->owner()->message(_("math text mode toggled"));
		break;

	case LFUN_MATH_LIMITS:
		bv->lockedInsetStoreUndo(Undo::INSERT);
		if (mathcursor->toggleLimits())
			updateLocal(bv, true);
		break;

	case LFUN_MATH_SIZE:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::INSERT);
			latexkeys const * l = in_word_set(arg);
			mathcursor->setSize(MathStyles(l ? l->id : static_cast<unsigned int>(-1)));
			updateLocal(bv, true);
		}
		break;

	case LFUN_INSERT_MATRIX:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::INSERT);
			mathcursor->interpret("matrix " + arg);
			updateLocal(bv, true);
		}
		break;

	case LFUN_MATH_SPACE:
	{
		bv->lockedInsetStoreUndo(Undo::EDIT);
		MathSpaceInset * p = mathcursor->prevSpaceInset();
		if (p) 
			p->incSpace();
		else
			mathcursor->insert(new MathSpaceInset(1));
		updateLocal(bv, true);
		break;
	}

	case LFUN_MATH_DELIM:
	{
		bv->lockedInsetStoreUndo(Undo::INSERT);
		//lyxerr << "formulabase::LFUN_MATH_DELIM, arg: '" << arg << "'\n";
		string ls;
		string rs;
		istringstream is(arg.c_str());
		is >> ls >> rs;
		latexkeys const * l = in_word_set(ls);
		latexkeys const * r = in_word_set(rs);
		if (!is || !l || !r) {
			lyxerr << "can't parse delimeters from '" << arg << "'\n";
			break;
		}
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->handleDelim(l, r);
		updateLocal(bv, true);
		break;
	}

	case LFUN_PROTECTEDSPACE:
		//lyxerr << " called LFUN_PROTECTEDSPACE\n";
		bv->lockedInsetStoreUndo(Undo::INSERT);
		mathcursor->insert(new MathSpaceInset(1));
		updateLocal(bv, true);
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
		updateLocal(bv, true);
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
		updateLocal(bv, true);
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
		updateLocal(bv, true);
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
		updateLocal(bv, true);
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
		updateLocal(bv, true);
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
		updateLocal(bv, true);
		break;
	}

	case LFUN_EXEC_COMMAND:
		result = UNDISPATCHED;
		break;

	case -1:
	case LFUN_INSERT_MATH:
	case LFUN_SELFINSERT:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::INSERT);
			mathcursor->interpret(arg);
			updateLocal(bv, true);
		}
		break;

	case LFUN_MATH_PANEL:
		result = UNDISPATCHED;
		break;

	default:
		lyxerr << "Closed by action " << action << endl;
		result = FINISHED_RIGHT;
	}

	mathcursor->normalize();

	if (was_macro != mathcursor->inMacroMode()
				&& action >= 0 && action != LFUN_BACKSPACE) 
  		updateLocal(bv, true);
	
	if (mathcursor->selection() || was_selection)
		toggleInsetSelection(bv);

	if (result == DISPATCHED || result == DISPATCHED_NOUPDATE ||
	    result == UNDISPATCHED)
		showInsetCursor(bv);
	else
		bv->unlockInset(this);

	return result;  // original version
}


Inset::Code InsetFormulaBase::lyxCode() const
{
	return Inset::MATH_CODE;
}


void mathDispatchCreation(BufferView * bv, string const & arg, bool display)
{
	if (bv->available()) {
		// use selection if available..
		//string sel;
		//if (action == LFUN_MATH_IMPORT_SELECTION)
		//	sel = "";
		//else

		string sel = bv->getLyXText()->selectionAsString(bv->buffer());

		InsetFormulaBase * f;
		if (sel.empty()) {
				f = new InsetFormula;
				if (openNewInset(bv, f)) {
					// don't do that also for LFUN_MATH_MODE unless you want end up with
					// always changing to mathrm when opening an inlined inset
					// -- I really hate "LyXfunc overloading"...
					if (display)
						f->localDispatch(bv, LFUN_MATH_DISPLAY, string());
					f->localDispatch(bv, LFUN_INSERT_MATH, arg);
				}
		} else {
			// create a macro if we see "\\newcommand" somewhere, and an ordinary
			// formula otherwise
			if (sel.find("\\newcommand") == string::npos) 
				f = new InsetFormula(sel);
			else
				f = new InsetFormulaMacro(sel);
			bv->getLyXText()->cutSelection(bv);
			openNewInset(bv, f);
		}
	}
	bv->owner()->getLyXFunc()->setMessage(N_("Math editor mode"));
}

void mathDispatchMathDisplay(BufferView * bv, string const & arg)
{
	mathDispatchCreation(bv, arg, true);
}

	
void mathDispatchMathMode(BufferView * bv, string const & arg)
{
	mathDispatchCreation(bv, arg, false);
}


void mathDispatchMathImportSelection(BufferView * bv, string const & arg)
{
	mathDispatchCreation(bv, arg, true);
}


void mathDispatchMathMacro(BufferView * bv, string const & arg)
{
	if (bv->available()) {
		if (arg.empty())
			bv->owner()->getLyXFunc()->setErrorMessage(N_("Missing argument"));
		else {
			string s(arg);
			string const s1 = token(s, ' ', 1);
			int const na = s1.empty() ? 0 : lyx::atoi(s1);
			openNewInset(bv, new InsetFormulaMacro(token(s, ' ', 0), na));
		}
	}
}


void mathDispatchMathDelim(BufferView * bv, string const & arg)
{ 	   
	if (bv->available()) { 
		if (openNewInset(bv, new InsetFormula))
			bv->theLockingInset()->localDispatch(bv, LFUN_MATH_DELIM, arg);
	}
}	   


void mathDispatchInsertMatrix(BufferView * bv, string const & arg)
{ 	   
	if (bv->available()) { 
		if (openNewInset(bv, new InsetFormula))
			bv->theLockingInset()->localDispatch(bv, LFUN_INSERT_MATRIX, arg);
	}
}	   


void mathDispatchInsertMath(BufferView * bv, string const & arg)
{
	if (bv->available()) {
		if (arg.size() && arg[0] == '\\')
			openNewInset(bv, new InsetFormula(arg));
		else
			mathDispatchMathMode(bv, arg);
	}
}

