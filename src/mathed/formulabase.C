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
#include "lyxrc.h"
#include "funcrequest.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "math_support.h"
#include "math_metricsinfo.h"
#include "support/lstrings.h"
#include "frontends/LyXView.h"
#include "frontends/font_metrics.h"
#include "frontends/mouse_state.h"
#include "Lsstream.h"
#include "math_arrayinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_cursor.h"
#include "math_factory.h"
#include "math_fontinset.h"
#include "math_hullinset.h"
#include "math_iterator.h"
#include "math_macrotable.h"
#include "math_parser.h"
#include "math_pos.h"
#include "math_spaceinset.h"
#include "undo_funcs.h"
#include "textpainter.h"
#include "frontends/Dialogs.h"
#include "intl.h"
#include "insets/insetcommandparams.h"
#include "ref_inset.h"

using std::endl;
using std::ostream;
using std::vector;
using std::abs;

MathCursor * mathcursor = 0;


namespace {

// local global
int first_x;
int first_y;



bool openNewInset(BufferView * bv, UpdatableInset * new_inset)
{
	if (!bv->insertInset(new_inset)) {
		delete new_inset;
		return false;
	}
	new_inset->edit(bv, true);
	return true;
}


} // namespace anon



InsetFormulaBase::InsetFormulaBase()
	: font_(), xo_(0), yo_(0)
{
	// This is needed as long the math parser is not re-entrant
	initMath();
	//lyxerr << "sizeof MathInset: " << sizeof(MathInset) << "\n";
	//lyxerr << "sizeof MathMetricsInfo: " << sizeof(MathMetricsInfo) << "\n";
	//lyxerr << "sizeof MathCharInset: " << sizeof(MathCharInset) << "\n";
	//lyxerr << "sizeof LyXFont: " << sizeof(LyXFont) << "\n";
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
		view_->owner()->getIntl()->getTransManager().TranslateAndInsert(*cit, lt);

	// remove ourselves
	//view_->owner()->getLyXFunc()->dispatch(LFUN_ESCAPE);
#endif
}


void InsetFormulaBase::handleFont
	(BufferView * bv, string const & arg, string const & font)
{
	bv->lockedInsetStoreUndo(Undo::EDIT);
	bool sel = mathcursor->selection();
	if (sel)
		updateLocal(bv, true);
	mathcursor->handleNest(createMathInset(font));
	mathcursor->insert(arg);
	if (!sel)
		updateLocal(bv, false);
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
		view_ = bv->owner()->view();
	MathMetricsInfo mi;
	mi.base.style = LM_ST_TEXT;
	mi.base.font  = font_;
	mi.base.font.setColor(LColor::math);
	par()->metrics(mi);
}


string const InsetFormulaBase::editMessage() const
{
	return _("Math editor mode");
}


void InsetFormulaBase::edit(BufferView * bv, int x, int y, mouse_button::state)
{
	if (!bv->lockInset(this))
		lyxerr[Debug::MATHED] << "Cannot lock inset!!!" << endl;
	releaseMathCursor(bv);
	mathcursor = new MathCursor(this, true);
	metrics(bv);
	mathcursor->setPos(x, y);
	//lyxerr << "setting pos to " << x << "," << y << "\n";

	// if that is removed, we won't get the magenta box when entering an
	// inset for the first time
	bv->updateInset(this, false);
}


void InsetFormulaBase::edit(BufferView * bv, bool front)
{
	if (!bv->lockInset(this))
		lyxerr << "Cannot lock math inset in edit call!\n";
	releaseMathCursor(bv);
	mathcursor = new MathCursor(this, front);
	metrics(bv);
	bv->updateInset(this, false);
}


void InsetFormulaBase::insetUnlock(BufferView * bv)
{
	if (mathcursor) {
		if (mathcursor->inMacroMode()) {
			mathcursor->macroModeClose();
			updateLocal(bv, true);
		}
		releaseMathCursor(bv);
	}
	generatePreview();
	bv->updateInset(this, false);
}


void InsetFormulaBase::getCursorPos(BufferView *, int & x, int & y) const
{
	// calling metrics here destroys the cached xo,yo positions e.g. in
	// MathParboxinset. And it would be too expensive anyway...
	//metrics(bv);
	mathcursor->getPos(x, y);
	//x -= xo_;
	y -= yo_;
	//lyxerr << "getCursorPos: " << x << " " << y << "\n";
}


void InsetFormulaBase::toggleInsetCursor(BufferView * bv)
{
	//lyxerr << "toggleInsetCursor: " << isCursorVisible() << "\n";
	if (isCursorVisible())
		hideInsetCursor(bv);
	else
		showInsetCursor(bv);
}


void InsetFormulaBase::showInsetCursor(BufferView * bv, bool)
{
	if (isCursorVisible())
		return;
	fitInsetCursor(bv);
	int x, y, asc, des;
	getCursorPos(bv, x, y);
	math_font_max_dim(font_, asc, des);
	bv->showLockedInsetCursor(x, y, asc, des);
	setCursorVisible(true);
	//lyxerr << "showInsetCursor: " << x << " " << y << "\n";
}


void InsetFormulaBase::hideInsetCursor(BufferView * bv)
{
	if (!isCursorVisible())
		return;
	bv->hideLockedInsetCursor();
	setCursorVisible(false);
	//lyxerr << "hideInsetCursor: \n";
}


void InsetFormulaBase::fitInsetCursor(BufferView * bv) const
{
	if (!mathcursor)
		return;
	int x, y, asc, des;
	math_font_max_dim(font_, asc, des);
	getCursorPos(bv, x, y);
	//y += yo_;
	//lyxerr << "fitInsetCursor: x: " << x << " y: " << y << " yo: " << yo_ << "\n";
	bv->fitLockedInsetCursor(x, y, asc, des);
}


void InsetFormulaBase::toggleInsetSelection(BufferView * bv)
{
	if (mathcursor)
		bv->updateInset(this, false);
}


vector<string> const InsetFormulaBase::getLabelList() const
{
  return vector<string>();
}


void InsetFormulaBase::updateLocal(BufferView * bv, bool dirty)
{
	bv->updateInset(this, dirty);
}


bool InsetFormulaBase::insetButtonRelease(BufferView * bv,
	int /*x*/, int /*y*/, mouse_button::state button)
{
	if (!mathcursor)
		return false;

	//lyxerr << "insetButtonRelease: " << x << " " << y << "\n";
	hideInsetCursor(bv);
	showInsetCursor(bv);
	bv->updateInset(this, false);

	if (button == mouse_button::button3) {
		// try to dispatch to enclosed insets first
		if (mathcursor->dispatch("mouse-3-release"))
			return true;

		// launch math panel for right mouse button
		bv->owner()->getDialogs()->showMathPanel();
		return true;
	}

	if (button == mouse_button::button1) {
		// try to dispatch to enclosed insets first
		if (mathcursor->dispatch("mouse-1-release"))
			return true;

		// try to set the cursor
		//delete mathcursor;
		//mathcursor = new MathCursor(this, x == 0);
		//metrics(bv);
		//mathcursor->setPos(x + xo_, y + yo_);
		return true;
	}
	return false;
}


void InsetFormulaBase::insetButtonPress(BufferView * bv,
					int x, int y, mouse_button::state button)
{
	//lyxerr << "insetButtonPress: "
	//	<< x << " " << y << " but: " << button << "\n";
	//lyxerr << "formula: ";
	par()->dump();

	releaseMathCursor(bv);
	mathcursor = new MathCursor(this, x == 0);

	if (button == mouse_button::button1) {
		// just set the cursor here
		//lyxerr << "setting cursor\n";
		metrics(bv);
		first_x = x;
		first_y = y;
		mathcursor->selClear();
		mathcursor->setPos(x + xo_, y + yo_);

		if (mathcursor->dispatch("mouse-1-press")) {
			//delete mathcursor;
			return;
		}

	}
	if (button == mouse_button::button3) {
		if (mathcursor->dispatch("mouse-3-press")) {
			//delete mathcursor;
			return;
		}
	}
	bv->updateInset(this, false);
}


void InsetFormulaBase::insetMotionNotify(BufferView * bv,
	int x, int y, mouse_button::state button)
{
	if (!mathcursor)
		return;

	if (button == mouse_button::button1)
		if (mathcursor->dispatch("mouse-1-motion"))
			return;

	if (button == mouse_button::button3)
		if (mathcursor->dispatch("mouse-3-motion"))
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


UpdatableInset::RESULT
InsetFormulaBase::localDispatch(BufferView * bv, FuncRequest const & ev)
{
	//lyxerr << "InsetFormulaBase::localDispatch: act: " << action
	//	<< " arg: '" << arg
	//	<< "' cursor: " << mathcursor << "\n";

	if (!mathcursor)
		return UNDISPATCHED;

	string argument    = ev.argument;
	RESULT result      = DISPATCHED;
	bool sel           = false;
	bool was_macro     = mathcursor->inMacroMode();
	bool was_selection = mathcursor->selection();

	hideInsetCursor(bv);

	mathcursor->normalize();
	mathcursor->touch();

	switch (ev.action) {

	case LFUN_WORDRIGHTSEL:
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

	case LFUN_WORDLEFTSEL:
	case LFUN_LEFTSEL:
		sel = true; // fall through
	case LFUN_LEFT:
		result = mathcursor->left(sel) ? DISPATCHED : FINISHED;
		updateLocal(bv, false);
		break;

	case LFUN_UPSEL:
		sel = true; // fall through
	case LFUN_UP:
		result = mathcursor->up(sel) ? DISPATCHED : FINISHED_UP;
		updateLocal(bv, false);
		break;

	case LFUN_DOWNSEL:
		sel = true; // fall through
	case LFUN_DOWN:
		result = mathcursor->down(sel) ? DISPATCHED : FINISHED_DOWN;
		updateLocal(bv, false);
		break;

	case LFUN_HOMESEL:
		sel = true; // fall through
	case LFUN_HOME:
		result = mathcursor->home(sel) ? DISPATCHED : FINISHED;
		updateLocal(bv, false);
		break;

	case LFUN_ENDSEL:
		sel = true; // fall through
	case LFUN_END:
		result = mathcursor->end(sel) ? DISPATCHED : FINISHED_RIGHT;
		updateLocal(bv, false);
		break;

	case LFUN_PRIORSEL:
	case LFUN_PRIOR:
		result = FINISHED_UP;
		updateLocal(bv, false);
		break;

	case LFUN_NEXTSEL:
	case LFUN_NEXT:
		result = FINISHED_DOWN;
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
		updateLocal(bv, true);
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
		istringstream is(ev.argument.c_str());
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


	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_CIRCUMFLEX:
		if (ev.argument.empty()) {
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
	case LFUN_GREEK_TOGGLE: handleFont(bv, ev.argument, "lyxgreek"); break;
	case LFUN_BOLD:         handleFont(bv, ev.argument, "textbf"); break;
	case LFUN_SANS:         handleFont(bv, ev.argument, "textsf"); break;
	case LFUN_EMPH:         handleFont(bv, ev.argument, "mathcal"); break;
	case LFUN_ROMAN:        handleFont(bv, ev.argument, "mathrm"); break;
	case LFUN_CODE:         handleFont(bv, ev.argument, "texttt"); break;
	case LFUN_FRAK:         handleFont(bv, ev.argument, "mathfrak"); break;
	case LFUN_ITAL:         handleFont(bv, ev.argument, "mathit"); break;
	case LFUN_NOUN:         handleFont(bv, ev.argument, "mathbb"); break;
	case LFUN_DEFAULT:      handleFont(bv, ev.argument, "textnormal"); break;
	case LFUN_FREE:         handleFont(bv, ev.argument, "textrm"); break;

	case LFUN_GREEK:
		handleFont(bv, ev.argument, "lyxgreek1");
		if (ev.argument.size())
			mathcursor->interpret(ev.argument);
		break;

	case LFUN_MATH_MODE:
		if (mathcursor->currentMode()) {
			handleFont(bv, ev.argument, "textrm");
		} else {
			mathcursor->niceInsert(MathAtom(new MathHullInset("simple")));
			updateLocal(bv, true);
		}
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
		if (!ev.argument.empty()) {
			bv->lockedInsetStoreUndo(Undo::EDIT);
			mathcursor->interpret("matrix " + ev.argument);
			updateLocal(bv, true);
		}
		break;

	case LFUN_SUPERSCRIPT:
	case LFUN_SUBSCRIPT:
	{
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->script(ev.action == LFUN_SUPERSCRIPT);
		updateLocal(bv, true);
		break;
	}

	case LFUN_MATH_DELIM:
	{
		//lyxerr << "formulabase::LFUN_MATH_DELIM, arg: '" << arg << "'\n";
		string ls;
		string rs = split(ev.argument, ls, ' ');
		// Reasonable default values
		if (ls.empty())
			ls = '(';
		if (rs.empty())
			rs = ')';

		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->handleNest(MathAtom(new MathDelimInset(ls, rs)));
		updateLocal(bv, true);
		break;
	}

	case LFUN_PROTECTEDSPACE:
	case LFUN_MATH_SPACE:
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->insert(MathAtom(new MathSpaceInset(",")));
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
			mathcursor->popToEnclosingGrid();
			bv->lockedInsetStoreUndo(Undo::EDIT);
			char align = ev.argument.size() ? ev.argument[0] : 'c';
			switch (ev.action) {
				case LFUN_MATH_HALIGN: p->halign(align, p->col(idx)); break;
				case LFUN_MATH_VALIGN: p->valign(align); break;
				case LFUN_MATH_ROW_INSERT: p->addRow(p->row(idx)); break;
				case LFUN_MATH_ROW_DELETE: p->delRow(p->row(idx)); break;
				case LFUN_MATH_COLUMN_INSERT: p->addFancyCol(p->col(idx)); break;
				case LFUN_MATH_COLUMN_DELETE: p->delFancyCol(p->col(idx)); break;
				default: ;
			}
			updateLocal(bv, true);
		}
		break;
	}

	case LFUN_EXEC_COMMAND:
		result = UNDISPATCHED;
		break;

	case LFUN_INSET_ERT:
		// interpret this as if a backslash was typed
		bv->lockedInsetStoreUndo(Undo::EDIT);
		mathcursor->interpret('\\');
		updateLocal(bv, true);
		break;

	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	case LFUN_BREAKPARAGRAPH_SKIP:
		argument = "\n";
		// fall through

	case -1:
	case LFUN_INSERT_MATH:
	case LFUN_SELFINSERT:
		if (!argument.empty()) {
			bv->lockedInsetStoreUndo(Undo::EDIT);
			if (argument.size() == 1)
				result = mathcursor->interpret(argument[0]) ? DISPATCHED : FINISHED_RIGHT;
			else
				result = mathcursor->interpret(argument) ? DISPATCHED : FINISHED_RIGHT;
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

	case LFUN_INSET_TOGGLE:
		mathcursor->insetToggle();
		updateLocal(bv, true);
		break;

	case LFUN_REF_INSERT:
		//if (argument.empty()) {
		//	InsetCommandParams p("ref");
		//	owner_->getDialogs()->createRef(p.getAsString());
		//} else {
		//	InsetCommandParams p;
		//	p.setFromString(argument);

		//	InsetRef * inset = new InsetRef(p, *buffer_);
		//	if (!insertInset(inset))
		//		delete inset;
		//	else
		//		updateInset(inset, true);
		//}
		//
		if (ev.argument.empty()) {
			InsetCommandParams p("ref");
			bv->owner()->getDialogs()->createRef(p.getAsString());
		} else {
			//mathcursor->handleNest(new InsetRef2);
			//mathcursor->insert(arg);
			mathcursor->insert(MathAtom(new RefInset(ev.argument)));
		}
		updateLocal(bv, true);
		break;

	default:
		result = UNDISPATCHED;
	}

	mathcursor->normalize();
	mathcursor->touch();

	lyx::Assert(mathcursor);

	if (mathcursor->selection() || was_selection)
		toggleInsetSelection(bv);

	if (result == DISPATCHED || result == DISPATCHED_NOUPDATE ||
	    result == UNDISPATCHED) {
		fitInsetCursor(bv);
		showInsetCursor(bv);
		revealCodes(bv);
	} else {
		releaseMathCursor(bv);
		bv->unlockInset(this);
	}

	return result;  // original version
}


void InsetFormulaBase::revealCodes(BufferView * bv) const
{
	if (!mathcursor)
		return;
	bv->owner()->message(mathcursor->info());

#if 0
	// write something to the minibuffer
	// translate to latex
	mathcursor->markInsert();
	ostringstream os;
	write(NULL, os);
	string str = os.str();
	mathcursor->markErase();
	string::size_type pos = 0;
	string res;
	for (string::iterator it = str.begin(); it != str.end(); ++it) {
		if (*it == '\n')
			res += ' ';
		else if (*it == '\0') {
			res += "  -X-  ";
			pos = it - str.begin();
		}
		else
			res += *it;
	}
	if (pos > 30)
		res = res.substr(pos - 30);
	if (res.size() > 60)
		res = res.substr(0, 60);
	bv->owner()->message(res);
#endif
}


Inset::Code InsetFormulaBase::lyxCode() const
{
	return Inset::MATH_CODE;
}


int InsetFormulaBase::ylow() const
{
	return yo_ - ascent(view(), font_);
}


int InsetFormulaBase::yhigh() const
{
	return yo_ + descent(view(), font_);
}


int InsetFormulaBase::xlow() const
{
	return xo_;
}


int InsetFormulaBase::xhigh() const
{
	return xo_ + width(view(), font_);
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
		if (it.cell().matchpart(ar, it.back().pos_)) {
			bv->unlockInset(bv->theLockingInset());
			if (!bv->lockInset(this)) {
				lyxerr << "Cannot lock inset" << endl;
				return false;
			}
			delete mathcursor;
			mathcursor = new MathCursor(this, true);
			metrics(bv);
			mathcursor->setSelection(it, ar.size());
			current = it;
			it.jump(ar.size());
			updateLocal(bv, false);
			return true;
		}
	}

	//lyxerr << "not found!\n";
	lastformula = 0;
	return false;
}


bool InsetFormulaBase::searchBackward(BufferView * bv, string const & what,
				      bool a, bool b)
{
	lyxerr[Debug::MATHED] << "searching backward not implemented in mathed\n";
	return searchForward(bv, what, a, b);
}


/////////////////////////////////////////////////////////////////////


void mathDispatchCreation(BufferView * bv, string const & arg, bool display)
{
	if (!bv->available())
		return;

	// use selection if available..
	//string sel;
	//if (action == LFUN_MATH_IMPORT_SELECTION)
	//	sel = "";
	//else

	string sel = bv->getLyXText()->selectionAsString(bv->buffer(), false);

	if (sel.empty()) {
		InsetFormula * f = new InsetFormula(bv);
		if (openNewInset(bv, f)) {
			f->mutate("simple");
			// don't do that also for LFUN_MATH_MODE unless you want end up with
			// always changing to mathrm when opening an inlined inset
			// -- I really hate "LyXfunc overloading"...
			if (display)
				f->localDispatch(bv, FuncRequest(LFUN_MATH_DISPLAY));
			f->localDispatch(bv, FuncRequest(LFUN_INSERT_MATH, arg));
		}
	} else {
		// create a macro if we see "\\newcommand" somewhere, and an ordinary
		// formula otherwise
		InsetFormulaBase * f;
		if (sel.find("\\newcommand") == string::npos &&
				sel.find("\\def") == string::npos)
			f = new InsetFormula(sel);
		else
			f = new InsetFormulaMacro(sel);
		bv->getLyXText()->cutSelection(bv);
		openNewInset(bv, f);
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
	if (!bv->available())
		return;
	if (arg.empty())
		bv->owner()->getLyXFunc()->setErrorMessage(N_("Missing argument"));
	else {
		string s = arg;
		string const s1 = token(s, ' ', 1);
		int const na = s1.empty() ? 0 : lyx::atoi(s1);
		openNewInset(bv, new InsetFormulaMacro(token(s, ' ', 0), na));
	}
}


void mathDispatchMathDelim(BufferView * bv, string const & arg)
{
	if (!bv->available())
		return;
	InsetFormula * f = new InsetFormula(bv);
	if (openNewInset(bv, f)) {
		f->mutate("simple");
		bv->theLockingInset()->localDispatch(bv, FuncRequest(LFUN_MATH_DELIM, arg));
	}
}


void mathDispatchInsertMatrix(BufferView * bv, string const & arg)
{
	if (!bv->available())
		return;
	InsetFormula * f = new InsetFormula(bv);
	if (openNewInset(bv, f)) {
		f->mutate("simple");
		bv->theLockingInset()->localDispatch(bv, FuncRequest(LFUN_INSERT_MATRIX, arg));
	}
}


void mathDispatchInsertMath(BufferView * bv, string const & arg)
{
	if (!bv->available())
		return;
	InsetFormula * f = new InsetFormula(bv);
	if (openNewInset(bv, f)) {
		f->mutate("simple");
    bv->theLockingInset()->localDispatch(bv, FuncRequest(LFUN_INSERT_MATH, arg));
	}
}


void mathDispatchGreek(BufferView * bv, string const & arg)
{
	if (!bv->available())
		return;
	InsetFormula * f = new InsetFormula(bv);
	if (openNewInset(bv, f)) {
		f->mutate("simple");
		bv->theLockingInset()->localDispatch(bv, FuncRequest(LFUN_GREEK, arg));
		bv->unlockInset(f);
	}
}


void mathDispatch(BufferView *, kb_action, string const &)
{}


void mathDispatch(BufferView *, string const &)
{}
