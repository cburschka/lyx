/*
*  File:        formulabase.C
*  Purpose:     Implementation of common parts of the LyX  math insets
*  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
*  Created:     January 1996
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
#include "support/LAssert.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "formula.h"
#include "formulamacro.h"
#include "commandtags.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "math_support.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "Painter.h"
#include "font.h"
#include "Lsstream.h"
#include "math_arrayinset.h"
#include "math_cursor.h"
#include "math_factory.h"
#include "math_hullinset.h"
#include "math_iterator.h"
#include "math_macrotable.h"
#include "math_parser.h"
#include "math_pos.h"
#include "math_spaceinset.h"
#include "undo_funcs.h"
#include "frontends/Dialogs.h"
#include "intl.h"

using std::endl;
using std::ostream;
using std::vector;

MathCursor * mathcursor = 0;


namespace {


// local global
int first_x;
int first_y;
int hack_x;
int hack_y;
int hack_button;


void handleFont(BufferView * bv, string const & arg, MathTextCodes t) 
{
	if (mathcursor->selection())
		bv->lockedInsetStoreUndo(Undo::EDIT);
	mathcursor->handleFont(t);
	for (string::const_iterator it = arg.begin(); it != arg.end(); ++it)
		mathcursor->insert(*it);
}


bool openNewInset(BufferView * bv, UpdatableInset * new_inset)
{
	if (!bv->insertInset(new_inset)) {
		delete new_inset;
		return false;
	}
	new_inset->edit(bv, 0, 0, 0);
	return true;
}


} // namespace anon



InsetFormulaBase::InsetFormulaBase()
	: view_(0), font_(), xo_(0), yo_(0)
{
	// This is needed as long the math parser is not re-entrant
	MathMacroTable::builtinMacros();
	//lyxerr << "sizeof MathInset: " << sizeof(MathInset) << "\n";
}


// simply scrap this function if you want
void InsetFormulaBase::mutateToText()
{
#if 0
	// translate to latex
	ostringstream os;
	latex(NULL, os, false, false);
	string str = os.str();

	// insert this text
	LyXText * lt = view_->getLyXText();
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit)
		view_->owner()->getIntl()->getTrans().TranslateAndInsert(*cit, lt);

	// remove ourselves
	//view_->owner()->getLyXFunc()->dispatch(LFUN_ESCAPE);
#endif
}


// Check if uses AMS macros
void InsetFormulaBase::validate(LaTeXFeatures &) const
{}


void InsetFormulaBase::metrics(BufferView * bv, LyXFont const & f) const 
{
	font_ = f;
	metrics(bv);
}


void InsetFormulaBase::metrics(BufferView * bv) const 
{
	if (bv)
		view_ = bv;
	MathMetricsInfo mi(view_, font_, display() ? LM_ST_DISPLAY : LM_ST_TEXT);
	par()->metrics(mi);
}


string const InsetFormulaBase::editMessage() const
{
	return _("Math editor mode");
}


void InsetFormulaBase::edit(BufferView * bv, int x, int /*y*/, unsigned int)
{
	if (!bv->lockInset(this))
		lyxerr[Debug::MATHED] << "Cannot lock inset!!!" << endl;

	//lyxerr << "edit: " << x  << " " << y << " button: " << button << "\n";
	if (!mathcursor) {
		mathcursor = new MathCursor(this, x == 0);
		metrics(bv);
		// handle ignored click
		if (hack_x || hack_y) {
			insetButtonPress(bv, hack_x, hack_y, hack_button);
			hack_x = hack_y = 0;
		}
	} else
		metrics(bv);
	// if that is removed, we won't get the magenta box when entering an
	// inset for the first time
	bv->updateInset(this, false);
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
	x += xo_;
	y += yo_;
	//lyxerr << "getCursorPos: " << x << " " << y << "\n";
}


void InsetFormulaBase::toggleInsetCursor(BufferView * bv)
{
	if (!mathcursor)
		return;

	if (isCursorVisible())
		bv->hideLockedInsetCursor();
	else {
		metrics(bv);
		int x;
		int y;
		mathcursor->getPos(x, y);
		y -= yo_;
		int asc = 0;
		int des = 0;
		MathMetricsInfo mi(bv, font_, display() ? LM_ST_DISPLAY : LM_ST_TEXT);
		math_font_max_dim(LM_TC_TEXTRM, mi, asc, des);
		bv->showLockedInsetCursor(x, y, asc, des);
		//lyxerr << "toggleInsetCursor: " << x << " " << y << "\n";
	}

	toggleCursorVisible();
}


void InsetFormulaBase::showInsetCursor(BufferView * bv, bool)
{
	if (isCursorVisible())
		return;
	if (mathcursor) {
		metrics(bv);
		int x;
		int y;
		mathcursor->getPos(x, y);
		y -= yo_;
		int asc = 0;
		int des = 0;
		MathMetricsInfo mi(bv, font_, display() ? LM_ST_DISPLAY : LM_ST_TEXT);
		math_font_max_dim(LM_TC_TEXTRM, mi, asc, des);
		bv->fitLockedInsetCursor(x, y, asc, des);
		//lyxerr << "showInsetCursor: x: " << x << " y: " << y << " yo: " << yo_ << "\n";
	}
	toggleInsetCursor(bv);
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
	metrics(bv);
	bv->updateInset(this, dirty);
}


bool InsetFormulaBase::insetButtonRelease(BufferView * bv,
                                          int /*x*/, int /*y*/, int /*button*/)
{
	if (!mathcursor)
		return false;
	//lyxerr << "insetButtonRelease: " << x << " " << y << "\n";
	hideInsetCursor(bv);
	showInsetCursor(bv);
	bv->updateInset(this, false);
	return false;
}


void InsetFormulaBase::insetButtonPress(BufferView * bv,
					int x, int y, int button)
{
	// hack to cope with mouseclick that comes before the call to edit()
	if (!mathcursor) {
		hack_x = x;
		hack_y = y;
		hack_button = button;
		return;
	}

	lyxerr << "insetButtonPress: " << x + xo_ << " " << y + yo_
		<< " but: " << button << "\n";
	switch (button) {
		default:
		case 1:
			// just click
			first_x = x;
			first_y = y;
			if (mathcursor) {
				mathcursor->selClear();
				mathcursor->setPos(x + xo_, y + yo_);
			}
			break;
/*
		case 2:
			lyxerr << "insetButtonPress: 2\n";
			// insert stuff
			if (mathcursor) {
				bv->lockedInsetStoreUndo(Undo::EDIT);
				MathArray ar;
				mathcursor->selGet(ar);
				mathcursor->setPos(x + xo_, y + yo_);
				string sel =
					bv->getLyXText()->selectionAsString(bv->buffer(), false);
				mathed_parse_cell(ar, sel);
				mathcursor->insert(ar);
			}	
			break;
*/
		case 3:
			// launch math panel for right mouse button
			bv->owner()->getDialogs()->showMathPanel();
			break;
	}
	bv->updateInset(this, false);
}


void InsetFormulaBase::insetMotionNotify(BufferView * bv,
					 int x, int y, int /*button*/)
{
	if (!mathcursor)
		return;

	if (abs(x - first_x) < 2 && abs(y - first_y) < 2) {
		//lyxerr << "insetMotionNotify: ignored\n";
		return;
	}
	first_x = x;
	first_y = y;

	if (!mathcursor->selection()) 
		mathcursor->selStart();
	
	//lyxerr << "insetMotionNotify: " << x + xo_ << ' ' << y + yo_
	//	<< ' ' << button << "\n";
	hideInsetCursor(bv);
	mathcursor->setPos(x + xo_, y + yo_);
	showInsetCursor(bv);
	bv->updateInset(this, false);
}


void InsetFormulaBase::insetKeyPress(XKeyEvent *)
{
	lyxerr[Debug::MATHED] << "Used InsetFormulaBase::InsetKeyPress." << endl;
}


UpdatableInset::RESULT
InsetFormulaBase::localDispatch(BufferView * bv, kb_action action,
			    string const & arg)
{
	//lyxerr << "InsetFormulaBase::localDispatch: act: " << action
	//	<< " arg: '" << arg << "' cursor: " << mathcursor << "\n";

	if (!mathcursor) 
		return UNDISPATCHED;

	if (mathcursor->asHyperActiveInset()) {
		lyxerr << " uurr.... getting dificult now\n";
		return mathcursor->asHyperActiveInset()->localDispatch(bv, action, arg);
	}

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
		//lyxerr << "calling scroll 20\n";
		//scroll(bv, 20);
		updateLocal(bv, false);
		// write something to the minibuffer
		//bv->owner()->message(mathcursor->info());
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

	case LFUN_HOMESEL:
		sel = true;

	case LFUN_HOME:
		mathcursor->home(sel);
		updateLocal(bv, false);
		break;

	case LFUN_ENDSEL:
		sel = true;

	case LFUN_END:
		mathcursor->end(sel);
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

	case LFUN_DELETE_WORD_BACKWARD:
	case LFUN_BACKSPACE:
		bv->lockedInsetStoreUndo(Undo::DELETE);
		mathcursor->backspace();
		bv->updateInset(this, true);
		break;

	case LFUN_DELETE_WORD_FORWARD:
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
		int x = 0;
		int y = 0;
		istringstream is(arg.c_str());
		is >> x >> y;
		mathcursor->setPos(x, y);
		updateLocal(bv, false);
		break;
	}

	case LFUN_PASTE:
		if (was_macro)
			mathcursor->macroModeClose();
		bv->lockedInsetStoreUndo(Undo::EDIT);
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

	case LFUN_WORDRIGHTSEL:
	case LFUN_WORDLEFTSEL:
		break;

	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_CIRCUMFLEX:
		if (arg.empty()) {
			// do superscript if LyX handles
			// deadkeys
			bv->lockedInsetStoreUndo(Undo::EDIT);
			mathcursor->script(true);
			updateLocal(bv, true);
		}
		break;
	case LFUN_UMLAUT:
	case LFUN_ACUTE:
	case LFUN_GRAVE:
	case LFUN_BREVE:
	case LFUN_DOT:
	case LFUN_MACRON:
	case LFUN_CARON:
	case LFUN_TILDE:
	case LFUN_CEDILLA:
	case LFUN_CIRCLE:
	case LFUN_UNDERDOT:
	case LFUN_TIE:
	case LFUN_OGONEK:
	case LFUN_HUNG_UMLAUT:
		break;

	//  Math fonts
	case LFUN_GREEK_TOGGLE: handleFont(bv, arg, LM_TC_GREEK); break;
	case LFUN_BOLD:         handleFont(bv, arg, LM_TC_BF); break;
	case LFUN_SANS:         handleFont(bv, arg, LM_TC_SF); break;
	case LFUN_EMPH:         handleFont(bv, arg, LM_TC_CAL); break;
	case LFUN_ROMAN:        handleFont(bv, arg, LM_TC_RM); break;
	case LFUN_CODE:         handleFont(bv, arg, LM_TC_TT); break;
	case LFUN_FRAK:         handleFont(bv, arg, LM_TC_EUFRAK); break;
	case LFUN_ITAL:         handleFont(bv, arg, LM_TC_IT); break;
	case LFUN_NOUN:         handleFont(bv, arg, LM_TC_BB); break;
	case LFUN_DEFAULT:      handleFont(bv, arg, LM_TC_VAR); break;
	case LFUN_FREE:         handleFont(bv, arg, LM_TC_TEXTRM); break;

	case LFUN_GREEK: 
		handleFont(bv, arg, LM_TC_GREEK1);
		if (arg.size())
			mathcursor->interpret(arg);
		break;

	case LFUN_MATH_MODE:
		//handleFont(bv, arg, LM_TC_TEXTRM);

		//mathcursor->niceInsert(MathAtom(new MathHullInset(LM_OT_SIMPLE)));
		//updateLocal(bv, true);

		//bv->owner()->message(_("math text mode toggled"));
		break;

	case LFUN_MATH_LIMITS:
		bv->lockedInsetStoreUndo(Undo::EDIT);
		if (mathcursor->toggleLimits())
			updateLocal(bv, true);
		break;

	case LFUN_MATH_SIZE:
#if 0
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::EDIT);
			mathcursor->setSize(arg);
			updateLocal(bv, true);
		}
#endif
		break;

	case LFUN_INSERT_MATRIX:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::EDIT);
			mathcursor->interpret("matrix " + arg);
			updateLocal(bv, true);
		}
		break;

	case LFUN_MATH_SPACE:
	{
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->insert(MathAtom(new MathSpaceInset(1)));
		updateLocal(bv, true);
		break;
	}
	
	case LFUN_SUPERSCRIPT:
	case LFUN_SUBSCRIPT:
	{
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->script((action == LFUN_SUPERSCRIPT));
		updateLocal(bv, true);
		break;
	}
	
	case LFUN_MATH_DELIM:
	{
		lyxerr << "formulabase::LFUN_MATH_DELIM, arg: '" << arg << "'\n";
		string ls;
		string rs;
		istringstream is(arg.c_str());
		is >> ls >> rs;
		if (!is) {
			lyxerr << "can't parse delimeters from '" << arg << "'\n";
			break;
		}
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->handleDelim(ls, rs);
		updateLocal(bv, true);
		break;
	}

	case LFUN_PROTECTEDSPACE:
		//lyxerr << " called LFUN_PROTECTEDSPACE\n";
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->insert(MathAtom(new MathSpaceInset(1)));
		updateLocal(bv, true);
		break;

	case LFUN_UNDO:
		bv->owner()->message(_("Invalid action in math mode!"));
		break;


	case LFUN_MATH_HALIGN:
	case LFUN_MATH_VALIGN:
	case LFUN_MATH_ROW_INSERT:
	case LFUN_MATH_ROW_DELETE:
	case LFUN_MATH_COLUMN_INSERT:
	case LFUN_MATH_COLUMN_DELETE:
	{
		MathInset::idx_type idx = 0;
		MathGridInset * p = mathcursor ? mathcursor->enclosingGrid(idx) : 0;
		if (p) {
			bv->lockedInsetStoreUndo(Undo::EDIT);
			char al = arg.size() ? arg[0] : 'c';
			switch (action) {
				case LFUN_MATH_HALIGN: p->halign(al, p->col(idx)); break;
				case LFUN_MATH_VALIGN: p->valign(al); break;
				case LFUN_MATH_ROW_INSERT: p->addRow(p->row(idx)); break;
				case LFUN_MATH_ROW_DELETE: p->delRow(p->row(idx)); break;
				case LFUN_MATH_COLUMN_INSERT: p->addCol(p->col(idx)); break;
				case LFUN_MATH_COLUMN_DELETE: p->delCol(p->col(idx)); break;
				default: ;
			}
			updateLocal(bv, true);
		}
		break;
	}

	case LFUN_EXEC_COMMAND:
		result = UNDISPATCHED;
		break;

	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		//lyxerr << "LFUN ignored\n";
		break;

	case LFUN_INSET_ERT:
		// interpret this as if a backslash was typed
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->interpret("\\");
		updateLocal(bv, true);
		break;

	case -1:
	case LFUN_INSERT_MATH:
	case LFUN_SELFINSERT:
		if (!arg.empty()) {
			bv->lockedInsetStoreUndo(Undo::EDIT);
			result = mathcursor->interpret(arg) ? DISPATCHED : FINISHED_RIGHT;
			updateLocal(bv, true);
		}
		break;

	case LFUN_MATH_PANEL:
		result = UNDISPATCHED;
		break;

	case LFUN_ESCAPE:
		if (mathcursor->selection())
			mathcursor->selClear();
		else
			result = UNDISPATCHED;
		break;

	default:
		result = UNDISPATCHED;
	}

	mathcursor->normalize();

	lyx::Assert(mathcursor);

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


int InsetFormulaBase::ylow() const
{
	return yo_ - ascent(view_, font_);
}


int InsetFormulaBase::yhigh() const
{
	return yo_ + descent(view_, font_);
}


int InsetFormulaBase::xlow() const
{
	return xo_;
}


int InsetFormulaBase::xhigh() const
{
	return xo_ + width(view_, font_);
}


/////////////////////////////////////////////////////////////////////


bool InsetFormulaBase::searchForward(BufferView * bv, string const & str,
				     bool, bool)
{
#ifdef WITH_WARNINGS
#warning pretty ugly
#endif
	static InsetFormulaBase * lastformula = 0;
	static MathIterator current = MathIterator(ibegin(par().nucleus()));
	static MathArray ar;
	static string laststr;

	if (lastformula != this || laststr != str) {
		//lyxerr << "reset lastformula to " << this << "\n";
		lastformula = this;
		laststr = str;
 		current	= ibegin(par().nucleus());
		ar.clear();
		mathed_parse_cell(ar, str);
	} else {
		++current;
	}
	//lyxerr << "searching '" << str << "' in " << this << ar << endl;

	for (MathIterator it = current; it != iend(par().nucleus()); ++it) {
		if (it.cell().matchpart(ar, it.position().pos_)) {
			mathcursor->setSelection(it.cursor(), ar.size());
			current = it;
			it.jump(ar.size());
			// I guess some of the following can go
			bv->toggleSelection(true);
			hideInsetCursor(bv);
			updateLocal(bv, true);
			showInsetCursor(bv);
			metrics(bv);
			return true;
		}
	}

	//lyxerr << "not found!\n";
	lastformula = 0;
	// we have to unlock ourself in this function by default!
	// don't ask me why...
	bv->unlockInset(this);
	return false;
}


bool InsetFormulaBase::searchBackward(BufferView * bv, string const & what,
				      bool a, bool b)
{
	lyxerr << "searching backward not implemented in mathed" << endl;
	return searchForward(bv, what, a, b);
}


/////////////////////////////////////////////////////////////////////


void mathDispatchCreation(BufferView * bv, string const & arg, bool display)
{
	if (bv->available()) {
		// use selection if available..
		//string sel;
		//if (action == LFUN_MATH_IMPORT_SELECTION)
		//	sel = "";
		//else

		string sel = bv->getLyXText()->selectionAsString(bv->buffer(), false);

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
			else {
				string name;
				if (!mathed_parse_macro(name, sel))
					return;
				f = new InsetFormulaMacro(sel);
			}
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
		if (arg.size() && arg[0] == '\\') {
			InsetFormula * f = new InsetFormula(arg);
			if (!bv->insertInset(f))
				delete f;
		} else
			mathDispatchMathMode(bv, arg);
	}
}


void mathDispatchGreek(BufferView * bv, string const & arg)
{ 	   
	if (bv->available()) { 
		InsetFormula * f = new InsetFormula;
		if (openNewInset(bv, f)) {
			bv->theLockingInset()->localDispatch(bv, LFUN_GREEK, arg);
			bv->unlockInset(f);
		}
	}
}	   


void mathDispatch(BufferView *, kb_action, string const &)
{}	   
