/**
 * \file formulabase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "cursor.h"
#include "formulabase.h"
#include "formula.h"
#include "formulamacro.h"
#include "math_support.h"
#include "math_arrayinset.h"
#include "math_deliminset.h"
#include "math_cursor.h"
#include "math_factory.h"
#include "math_hullinset.h"
#include "math_parser.h"
#include "math_spaceinset.h"
#include "ref_inset.h"

#include "BufferView.h"
#include "bufferview_funcs.h"
#include "dispatchresult.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LColor.h"
#include "lyxtext.h"
#include "undo.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

#include "support/std_sstream.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

using lyx::support::atoi;
using lyx::support::split;
using lyx::support::token;

using std::string;
using std::abs;
using std::endl;
using std::max;
using std::istringstream;
using std::ostringstream;


namespace {

// local global
int first_x;
int first_y;

bool openNewInset(LCursor & cur, UpdatableInset * inset)
{
	if (!cur.bv().insertInset(inset)) {
		delete inset;
		return false;
	}
	inset->edit(cur, true);
	return true;
}


} // namespace anon



InsetFormulaBase::InsetFormulaBase()
{
	// This is needed as long the math parser is not re-entrant
	initMath();
	//lyxerr << "sizeof MathInset: " << sizeof(MathInset) << endl;
	//lyxerr << "sizeof MetricsInfo: " << sizeof(MetricsInfo) << endl;
	//lyxerr << "sizeof MathCharInset: " << sizeof(MathCharInset) << endl;
	//lyxerr << "sizeof LyXFont: " << sizeof(LyXFont) << endl;
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
	//view_->owner()->dispatch(LFUN_ESCAPE);
#endif
}


void InsetFormulaBase::handleFont
	(LCursor & cur, string const & arg, string const & font)
{
	// this whole function is a hack and won't work for incremental font
	// changes...
	recordUndo(cur, Undo::ATOMIC);

	if (cur.inset()->asMathInset()->name() == font)
		mathcursor::handleFont(cur, font);
	else {
		mathcursor::handleNest(cur, createMathInset(font));
		mathcursor::insert(cur, arg);
	}
}


void InsetFormulaBase::handleFont2(LCursor & cur, string const & arg)
{
	recordUndo(cur, Undo::ATOMIC);
	LyXFont font;
	bool b;
	bv_funcs::string2font(arg, font, b);
	if (font.color() != LColor::inherit) {
		MathAtom at = createMathInset("color");
		asArray(lcolor.getGUIName(font.color()), at.nucleus()->cell(0));
		mathcursor::handleNest(cur, at, 1);
	}
}



void InsetFormulaBase::validate(LaTeXFeatures &) const
{}


string const InsetFormulaBase::editMessage() const
{
	return _("Math editor mode");
}


void InsetFormulaBase::insetUnlock(BufferView & bv)
{
	if (inMathed()) {
		if (mathcursor::inMacroMode(bv.cursor()))
			mathcursor::macroModeClose(bv.cursor());
		releaseMathCursor(bv.cursor());
	}
	if (bv.buffer())
		generatePreview(*bv.buffer());
	bv.update();
}


void InsetFormulaBase::getCursorPos(BufferView & bv, int & x, int & y) const
{
	if (inMathed()) {
		mathcursor::getScreenPos(bv.cursor(), x, y);
		x = mathcursor::targetX(bv.cursor());
		x -= xo_;
		y -= yo_;
		lyxerr << "InsetFormulaBase::getCursorPos: " << x << ' ' << y << endl;
	} else {
		x = 0;
		y = 0;
		lyxerr << "getCursorPos - should not happen";
	}
}


void InsetFormulaBase::getCursorDim(int & asc, int & desc) const
{
	if (inMathed()) {
		asc = 10;
		desc = 2;
		//math_font_max_dim(font_, asc, des);
	}
}


DispatchResult
InsetFormulaBase::lfunMouseRelease(LCursor & cur, FuncRequest const & cmd)
{
	if (!inMathed())
		return DispatchResult(false);
	cur.bv().update();
	//lyxerr << "lfunMouseRelease: buttons: " << cmd.button() << endl;

	if (cmd.button() == mouse_button::button3) {
		// try to dispatch to enclosed insets first
		if (!mathcursor::dispatch(cur, cmd).dispatched()) {
			// launch math panel for right mouse button
			lyxerr << "lfunMouseRelease: undispatched: " << cmd.button() << endl;
			cur.bv().owner()->getDialogs().show("mathpanel");
		}
		return DispatchResult(true, true);
	}

	if (cmd.button() == mouse_button::button2) {
		MathArray ar;
		asArray(cur.bv().getClipboard(), ar);
		mathcursor::selClear(cur);
		mathcursor::setScreenPos(cur, cmd.x + xo_, cmd.y + yo_);
		mathcursor::insert(cur, ar);
		cur.bv().update();
		return DispatchResult(true, true);
	}

	if (cmd.button() == mouse_button::button1) {
		// try to dispatch to enclosed insets first
		mathcursor::dispatch(cur, cmd);
		cur.bv().stuffClipboard(mathcursor::grabSelection(cur));
		// try to set the cursor
		//delete mathcursor;
		//mathcursor = new MathCursor(bv, this, x == 0);
		//metrics(bv);
		//mathcursor::setScreenPos(x + xo_, y + yo_);
		return DispatchResult(true, true);
	}

	return DispatchResult(false);
}


DispatchResult
InsetFormulaBase::lfunMousePress(LCursor & cur, FuncRequest const & cmd)
{
	//lyxerr << "lfunMousePress: buttons: " << cmd.button() << endl;

	if (!inMathed() || mathcursor::formula() != this) {
		lyxerr[Debug::MATHED] << "re-create cursor" << endl;
		releaseMathCursor(cur);
		mathcursor::formula_ = this;
		cur.idx() = 0;
		//metrics(bv);
		mathcursor::setScreenPos(cur, cmd.x + xo_, cmd.y + yo_);
	}

	if (cmd.button() == mouse_button::button3) {
		mathcursor::dispatch(cur, cmd);
		return DispatchResult(true, true);
	}

	if (cmd.button() == mouse_button::button1) {
		first_x = cmd.x;
		first_y = cmd.y;
		mathcursor::selClear(cur);
		mathcursor::setScreenPos(cur, cmd.x + xo_, cmd.y + yo_);
		mathcursor::dispatch(cur, cmd);
		return DispatchResult(true, true);
	}

	cur.bv().update();
	return DispatchResult(true, true);
}


DispatchResult
InsetFormulaBase::lfunMouseMotion(LCursor & cur, FuncRequest const & cmd)
{
	if (!inMathed())
		return DispatchResult(true, true);

	if (mathcursor::dispatch(cur, FuncRequest(cmd)).dispatched())
		return DispatchResult(true, true);

	// only select with button 1
	if (cmd.button() != mouse_button::button1)
		return DispatchResult(true, true);

	if (abs(cmd.x - first_x) < 2 && abs(cmd.y - first_y) < 2)
		return DispatchResult(true, true);

	first_x = cmd.x;
	first_y = cmd.y;

	if (!cur.selection())
		mathcursor::selStart(cur);

	mathcursor::setScreenPos(cur, cmd.x + xo_, cmd.y + yo_);
	cur.bv().update();
	return DispatchResult(true, true);
}


void InsetFormulaBase::edit(LCursor & cur, bool /*left*/)
{
	lyxerr << "Called FormulaBase::edit" << endl;
	mathcursor::formula_ = this;
	cur.push(this);
	cur.idx() = 0;
	cur.pos() = 0;
#warning FIXME
	cur.push(par().nucleus()->asHullInset());
	//cur.idx() = left ? 0 : cur.lastidx();
	cur.idx() = 0;
	cur.pos() = 0;
	cur.resetAnchor();
}


void InsetFormulaBase::edit(LCursor & cur, int x, int y)
{
	lyxerr << "Called FormulaBase::EDIT with '" << x << ' ' << y << "'" << endl;
	releaseMathCursor(cur);
	//metrics(bv);
	cur.push(this);
	cur.idx() = 0;
	cur.pos() = 0;
	mathcursor::setScreenPos(cur, x + xo_, y + yo_);
	cur.push(par().nucleus()->asHullInset());
	//cur.idx() = left ? 0 : cur.lastidx();
	cur.idx() = 0;
	cur.pos() = 0;
	// if that is removed, we won't get the magenta box when entering an
	// inset for the first time
	cur.bv().update();
}


DispatchResult
InsetFormulaBase::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	return par().nucleus()->dispatch(cur, cmd);

	//lyxerr << "InsetFormulaBase::localDispatch: act: " << cmd.action
	//	<< " arg: '" << cmd.argument
	//	<< "' x: '" << cmd.x
	//	<< " y: '" << cmd.y
	//	<< "' button: " << cmd.button() << endl;

#if 0
	// delete empty mathbox (LFUN_BACKSPACE and LFUN_DELETE)
	bool remove_inset = false;

	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			//lyxerr << "Mouse single press" << endl;
			return lfunMousePress(cur, cmd);
		case LFUN_MOUSE_MOTION:
			//lyxerr << "Mouse motion" << endl;
			return lfunMouseMotion(cur, cmd);
		case LFUN_MOUSE_RELEASE:
			//lyxerr << "Mouse single release" << endl;
			return lfunMouseRelease(cur, cmd);
		case LFUN_MOUSE_DOUBLE:
			//lyxerr << "Mouse double" << endl;
			return dispatch(cur, FuncRequest(LFUN_WORDSEL));
		default:
			break;
	}

	DispatchResult result(true);
	string argument    = cmd.argument;
	bool sel           = false;
	bool was_macro     = mathcursor::inMacroMode(cur);

	mathcursor::normalize(cur);
	mathcursor::touch();

	switch (cmd.action) {

	case LFUN_MATH_MUTATE:
	case LFUN_MATH_DISPLAY:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_NONUMBER:
	case LFUN_CELL_SPLIT:
	case LFUN_BREAKLINE:
	case LFUN_DELETE_LINE_FORWARD:
	case LFUN_INSERT_LABEL:
	case LFUN_MATH_EXTERN:
	case LFUN_TABULAR_FEATURE:
	case LFUN_PASTESELECTION:
	case LFUN_MATH_LIMITS:
		recordUndo(cur, Undo::ATOMIC);
		mathcursor::dispatch(cur, cmd);
		break;

	case LFUN_WORDSEL:
		mathcursor::home(cur, false);
		mathcursor::end(cur, true);
		break;

	case LFUN_UP_PARAGRAPHSEL:
	case LFUN_UP_PARAGRAPH:
	case LFUN_DOWN_PARAGRAPHSEL:
	case LFUN_DOWN_PARAGRAPH:
		result = DispatchResult(true, FINISHED);
		break;

	case LFUN_HOMESEL:
	case LFUN_WORDLEFTSEL:
		sel = true; // fall through
	case LFUN_HOME:
	case LFUN_WORDLEFT:
		result = mathcursor::home(cur, sel)
			? DispatchResult(true, true) : DispatchResult(true, FINISHED);
		break;

	case LFUN_ENDSEL:
	case LFUN_WORDRIGHTSEL:
		sel = true; // fall through
	case LFUN_END:
	case LFUN_WORDRIGHT:
		result = mathcursor::end(cur, sel)
			? DispatchResult(true, true) : DispatchResult(false, FINISHED_RIGHT);
		break;

	case LFUN_PRIORSEL:
	case LFUN_PRIOR:
	case LFUN_BEGINNINGBUFSEL:
	case LFUN_BEGINNINGBUF:
		result = DispatchResult(true, FINISHED);
		break;

	case LFUN_NEXTSEL:
	case LFUN_NEXT:
	case LFUN_ENDBUFSEL:
	case LFUN_ENDBUF:
		result = DispatchResult(false, FINISHED_RIGHT);
		break;

	case LFUN_CELL_FORWARD:
		mathcursor::idxNext(cur);
		break;

	case LFUN_CELL_BACKWARD:
		mathcursor::idxPrev(cur);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
	case LFUN_BACKSPACE:
		recordUndo(cur, Undo::ATOMIC);
		if (!mathcursor::backspace(cur)) {
			result = DispatchResult(true, FINISHED);
			remove_inset = true;
		}
		break;

	case LFUN_DELETE_WORD_FORWARD:
	case LFUN_DELETE:
		recordUndo(cur, Undo::ATOMIC);
		if (!mathcursor::erase(cur)) {
			result = DispatchResult(true, FINISHED);
			remove_inset = true;
		}
		break;

	//    case LFUN_GETXY:
	//      sprintf(dispatch_buffer, "%d %d",);
	//      DispatchResult= dispatch_buffer;
	//      break;
	case LFUN_SETXY: {
		lyxerr << "LFUN_SETXY broken!" << endl;
		int x = 0;
		int y = 0;
		istringstream is(cmd.argument.c_str());
		is >> x >> y;
		mathcursor::setScreenPos(cur, x, y);
		break;
	}

	case LFUN_PASTE: {
		size_t n = 0;
		istringstream is(cmd.argument.c_str());
		is >> n;
		if (was_macro)
			mathcursor::macroModeClose(cur);
		recordUndo(cur, Undo::ATOMIC);
		mathcursor::selPaste(cur, n);
		break;
	}

	case LFUN_CUT:
		recordUndo(cur, Undo::DELETE);
		mathcursor::selCut(cur);
		break;

	case LFUN_COPY:
		mathcursor::selCopy(cur);
		break;


	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_CIRCUMFLEX:
		if (cmd.argument.empty()) {
			// do superscript if LyX handles
			// deadkeys
			recordUndo(cur, Undo::ATOMIC);
			mathcursor::script(cur, true);
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
	case LFUN_FREEFONT_APPLY:
	case LFUN_FREEFONT_UPDATE:
		handleFont2(cur, cmd.argument);
		break;

	case LFUN_BOLD:         handleFont(cur, cmd.argument, "mathbf"); break;
	case LFUN_SANS:         handleFont(cur, cmd.argument, "mathsf"); break;
	case LFUN_EMPH:         handleFont(cur, cmd.argument, "mathcal"); break;
	case LFUN_ROMAN:        handleFont(cur, cmd.argument, "mathrm"); break;
	case LFUN_CODE:         handleFont(cur, cmd.argument, "texttt"); break;
	case LFUN_FRAK:         handleFont(cur, cmd.argument, "mathfrak"); break;
	case LFUN_ITAL:         handleFont(cur, cmd.argument, "mathit"); break;
	case LFUN_NOUN:         handleFont(cur, cmd.argument, "mathbb"); break;
	//case LFUN_FREEFONT_APPLY:  handleFont(cur, cmd.argument, "textrm"); break;
	case LFUN_DEFAULT:      handleFont(cur, cmd.argument, "textnormal"); break;

	case LFUN_MATH_MODE:
		if (mathcursor::currentMode(cur) == MathInset::TEXT_MODE)
			mathcursor::niceInsert(cur, MathAtom(new MathHullInset("simple")));
		else
			handleFont(cur, cmd.argument, "textrm");
		//cur.owner()->message(_("math text mode toggled"));
		break;

	case LFUN_MATH_SIZE:
#if 0
		if (!arg.empty()) {
			recordUndo(cur, Undo::ATOMIC);
			mathcursor::setSize(arg);
		}
#endif
		break;

	case LFUN_INSERT_MATRIX: {
		recordUndo(cur, Undo::ATOMIC);
		unsigned int m = 1;
		unsigned int n = 1;
		string v_align;
		string h_align;
		istringstream is(argument);
		is >> m >> n >> v_align >> h_align;
		m = max(1u, m);
		n = max(1u, n);
		v_align += 'c';
		mathcursor::niceInsert(cur,
			MathAtom(new MathArrayInset("array", m, n, v_align[0], h_align)));
		break;
	}

	case LFUN_MATH_DELIM: {
		//lyxerr << "formulabase::LFUN_MATH_DELIM, arg: '" << arg << "'" << endl;
		string ls;
		string rs = split(cmd.argument, ls, ' ');
		// Reasonable default values
		if (ls.empty())
			ls = '(';
		if (rs.empty())
			rs = ')';
		recordUndo(cur, Undo::ATOMIC);
		mathcursor::handleNest(cur, MathAtom(new MathDelimInset(ls, rs)));
		break;
	}

	case LFUN_SPACE_INSERT:
	case LFUN_MATH_SPACE:
		recordUndo(cur, Undo::ATOMIC);
		mathcursor::insert(cur, MathAtom(new MathSpaceInset(",")));
		break;

	case LFUN_UNDO:
		cur.bv().owner()->message(_("Invalid action in math mode!"));
		break;


	case LFUN_EXEC_COMMAND:
		result = DispatchResult(false);
		break;

	case LFUN_INSET_ERT:
		// interpret this as if a backslash was typed
		recordUndo(cur, Undo::ATOMIC);
		mathcursor::interpret(cur, '\\');
		break;

	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	case LFUN_BREAKPARAGRAPH_SKIP:
		argument = "\n";
		// fall through

// FIXME: We probably should swap parts of "math-insert" and "self-insert"
// handling such that "self-insert" works on "arbitrary stuff" too, and
// math-insert only handles special math things like "matrix".
	case LFUN_INSERT_MATH:
		recordUndo(cur, Undo::ATOMIC);
		mathcursor::niceInsert(cur, argument);
		break;

	case -1:
	case LFUN_SELFINSERT:
		if (!argument.empty()) {
			recordUndo(cur, Undo::ATOMIC);
			if (argument.size() == 1)
				result = mathcursor::interpret(cur, argument[0])
					? DispatchResult(true, true) : DispatchResult(false, FINISHED_RIGHT);
			else
				mathcursor::insert(cur, argument);
		}
		break;

	case LFUN_ESCAPE:
		if (cur.selection())
			mathcursor::selClear(cur);
		else
			result = DispatchResult(false);
		break;

	case LFUN_INSET_TOGGLE:
		mathcursor::insetToggle(cur);
		break;

	case LFUN_DIALOG_SHOW:
		result = DispatchResult(false);
		break;

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		string const & name = argument;
		string data;
		if (name == "ref") {
			RefInset tmp(name);
			data = tmp.createDialogStr(name);
		}

		if (data.empty())
			result = DispatchResult(false);
		else
			cur.bv().owner()->getDialogs().show(name, data, 0);
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		InsetBase * base = cur.bv().owner()->getDialogs().getOpenInset(name);

		if (base) {
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument);
			result = base->dispatch(cur, fr);
		} else {
			MathArray ar;
			if (createMathInset_fromDialogStr(cmd.argument, ar)) {
				mathcursor::insert(cur, ar);
				result = DispatchResult(true, true);
			} else {
				result = DispatchResult(false);
			}
		}
		break;
	}

	case LFUN_WORD_REPLACE:
	case LFUN_WORD_FIND: {
		result = 
			searchForward(&cur.bv(), cmd.getArg(0), false, false)
				? DispatchResult(true, true) : DispatchResult(false);
		break;
	}

	default:
		result = DispatchResult(false);
	}

	if (result == DispatchResult(true, true))
		cur.bv().update();

	mathcursor::normalize(cur);
	mathcursor::touch();

	BOOST_ASSERT(inMathed());

	if (result.dispatched()) {
		revealCodes(cur);
		cur.bv().stuffClipboard(mathcursor::grabSelection(cur));
	} else {
		releaseMathCursor(cur);
		if (remove_inset)
			cur.bv().owner()->dispatch(FuncRequest(LFUN_DELETE));
	}

	return result;  // original version
#endif
}


void InsetFormulaBase::revealCodes(LCursor & cur) const
{
	if (!inMathed())
		return;
	ostringstream os;
	cur.info(os);
	cur.bv().owner()->message(os.str());
/*
	// write something to the minibuffer
	// translate to latex
	mathcursor::markInsert(bv);
	ostringstream os;
	write(NULL, os);
	string str = os.str();
	mathcursor::markErase(bv);
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
	bv.owner()->message(res);
*/
}


InsetOld::Code InsetFormulaBase::lyxCode() const
{
	return InsetOld::MATH_CODE;
}


int InsetFormulaBase::ylow() const
{
	return yo_ - dim_.asc;
}


int InsetFormulaBase::yhigh() const
{
	return yo_ + dim_.des;
}


int InsetFormulaBase::xlow() const
{
	return xo_;
}


int InsetFormulaBase::xhigh() const
{
	return xo_ + dim_.wid;
}


/////////////////////////////////////////////////////////////////////


bool InsetFormulaBase::searchForward(BufferView * bv, string const & str,
				     bool, bool)
{
	return false;
#ifdef WITH_WARNINGS
#warning pretty ugly
#endif
	static InsetFormulaBase * lastformula = 0;
	static CursorBase current = CursorBase(ibegin(par().nucleus()));
	static MathArray ar;
	static string laststr;

	if (lastformula != this || laststr != str) {
		//lyxerr << "reset lastformula to " << this << endl;
		lastformula = this;
		laststr = str;
		current	= ibegin(par().nucleus());
		ar.clear();
		mathed_parse_cell(ar, str);
	} else {
		increment(current);
	}
	//lyxerr << "searching '" << str << "' in " << this << ar << endl;

	for (CursorBase it = current; it != iend(par().nucleus()); increment(it)) {
		CursorSlice & top = it.back();
		MathArray const & a = top.asMathInset()->cell(top.idx_);
		if (a.matchpart(ar, top.pos_)) {
			mathcursor::formula_ = this;
			mathcursor::setSelection(bv->cursor(), it, ar.size());
			current = it;
			top.pos_ += ar.size();
			bv->update();
			return true;
		}
	}

	//lyxerr << "not found!" << endl;
	lastformula = 0;
	return false;
}


bool InsetFormulaBase::searchBackward(BufferView * bv, string const & what,
				      bool a, bool b)
{
	lyxerr[Debug::MATHED] << "searching backward not implemented in mathed" << endl;
	return searchForward(bv, what, a, b);
}


bool InsetFormulaBase::display() const
{
	return par()->asHullInset() && par()->asHullInset()->display();
}


string InsetFormulaBase::selectionAsString(BufferView & bv) const
{
	return inMathed() ? mathcursor::grabSelection(bv.cursor()) : string();
}

/////////////////////////////////////////////////////////////////////


void mathDispatchCreation(LCursor & cur, FuncRequest const & cmd,
	bool display)
{
	// use selection if available..
	//string sel;
	//if (action == LFUN_MATH_IMPORT_SELECTION)
	//	sel = "";
	//else

	string sel =
		cur.bv().getLyXText()->selectionAsString(*cur.bv().buffer(), false);

	if (sel.empty()) {
		InsetFormula * f = new InsetFormula;
		if (openNewInset(cur, f)) {
			cur.inset()->dispatch(cur, FuncRequest(LFUN_MATH_MUTATE, "simple"));
			// don't do that also for LFUN_MATH_MODE unless you want end up with
			// always changing to mathrm when opening an inlined inset
			// -- I really hate "LyXfunc overloading"...
			if (display)
				f->dispatch(cur, FuncRequest(LFUN_MATH_DISPLAY));
			f->dispatch(cur, FuncRequest(LFUN_INSERT_MATH, cmd.argument));
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
		cur.bv().getLyXText()->cutSelection(true, false);
		openNewInset(cur, f);
	}
	cmd.message(N_("Math editor mode"));
}


void mathDispatch(LCursor & cur, FuncRequest const & cmd)
{
	if (!cur.bv().available())
		return;

	switch (cmd.action) {

		case LFUN_MATH_DISPLAY:
			mathDispatchCreation(cur, cmd, true);
			break;

		case LFUN_MATH_MODE:
			mathDispatchCreation(cur, cmd, false);
			break;

		case LFUN_MATH_IMPORT_SELECTION:
			mathDispatchCreation(cur, cmd, false);
			break;

		case LFUN_MATH_MACRO:
			if (cmd.argument.empty())
				cmd.errorMessage(N_("Missing argument"));
			else {
				string s = cmd.argument;
				string const s1 = token(s, ' ', 1);
				int const nargs = s1.empty() ? 0 : atoi(s1);
				string const s2 = token(s, ' ', 2);
				string const type = s2.empty() ? "newcommand" : s2;
				openNewInset(cur, new InsetFormulaMacro(token(s, ' ', 0), nargs, s2));
			}
			break;

		case LFUN_INSERT_MATH:
		case LFUN_INSERT_MATRIX:
		case LFUN_MATH_DELIM: {
			InsetFormula * f = new InsetFormula;
			if (openNewInset(cur, f)) {
				cur.inset()->dispatch(cur, FuncRequest(LFUN_MATH_MUTATE, "simple"));
				cur.inset()->dispatch(cur, cmd);
			}
			break;
		}

		default:
			break;
	}
}
