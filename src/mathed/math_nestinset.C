/**
 * \file math_nestinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_nestinset.h"

#include "BufferView.h"
#include "LColor.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "undo.h"

#include "frontends/Painter.h"


MathNestInset::MathNestInset(idx_type nargs)
	: cells_(nargs), lock_(false)
{}


MathInset::idx_type MathNestInset::nargs() const
{
	return cells_.size();
}


MathArray & MathNestInset::cell(idx_type i)
{
	return cells_[i];
}


MathArray const & MathNestInset::cell(idx_type i) const
{
	return cells_[i];
}


void MathNestInset::getScreenPos(idx_type idx, pos_type pos, int & x, int & y) const
{
	MathArray const & ar = cell(idx);
	x = ar.xo() + ar.pos2x(pos);
	y = ar.yo();
	// move cursor visually into empty cells ("blue rectangles");
	if (cell(idx).empty())
		x += 2;
}


void MathNestInset::substitute(MathMacro const & m)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).substitute(m);
}


void MathNestInset::metrics(MetricsInfo const & mi) const
{
	MetricsInfo m = mi;
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).metrics(m);
}


bool MathNestInset::idxNext(LCursor & cur) const
{
	if (cur.idx() + 1 >= nargs())
		return false;
	++cur.idx();
	cur.pos() = 0;
	return true;
}


bool MathNestInset::idxRight(LCursor & cur) const
{
	return idxNext(cur);
}


bool MathNestInset::idxPrev(LCursor & cur) const
{
	if (cur.idx() == 0)
		return false;
	--cur.idx();
	cur.pos() = cur.lastpos();
	return true;
}


bool MathNestInset::idxLeft(LCursor & cur) const
{
	return idxPrev(cur);
}


bool MathNestInset::idxFirst(LCursor & cur) const
{
	if (nargs() == 0)
		return false;
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool MathNestInset::idxLast(LCursor & cur) const
{
	if (nargs() == 0)
		return false;
	cur.idx() = nargs() - 1;
	cur.pos() = cur.lastpos();
	return true;
}


bool MathNestInset::idxHome(LCursor & cur) const
{
	if (cur.pos() == 0)
		return false;
	cur.pos() = 0;
	return true;
}


bool MathNestInset::idxEnd(LCursor & cur) const
{
	if (cur.lastpos() == cur.lastpos())
		return false;
	cur.pos() = cur.lastpos();
	return true;
}


void MathNestInset::dump() const
{
	WriteStream os(lyxerr);
	os << "---------------------------------------------\n";
	write(os);
	os << "\n";
	for (idx_type i = 0; i < nargs(); ++i)
		os << cell(i) << "\n";
	os << "---------------------------------------------\n";
}


//void MathNestInset::draw(PainterInfo & pi, int x, int y) const
void MathNestInset::draw(PainterInfo &, int, int) const
{
#if 0
	if (lock_)
		pi.pain.fillRectangle(x, y - ascent(), width(), height(),
					LColor::mathlockbg);
#endif
}


void MathNestInset::drawSelection(PainterInfo & pi,
		idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const
{
	if (idx1 == idx2) {
		MathArray const & c = cell(idx1);
		int x1 = c.xo() + c.pos2x(pos1);
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(pos2);
		int y2 = c.yo() + c.descent();
		pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		for (idx_type i = 0; i < nargs(); ++i) {
			if (idxBetween(i, idx1, idx2)) {
				MathArray const & c = cell(i);
				int x1 = c.xo();
				int y1 = c.yo() - c.ascent();
				int x2 = c.xo() + c.width();
				int y2 = c.yo() + c.descent();
				pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
			}
		}
	}
}


void MathNestInset::validate(LaTeXFeatures & features) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}


void MathNestInset::replace(ReplaceData & rep)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).replace(rep);
}


bool MathNestInset::contains(MathArray const & ar) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		if (cell(i).contains(ar))
			return true;
	return false;
}


bool MathNestInset::editing(BufferView * bv) const
{
	return bv->cursor().isInside(this);
}


bool MathNestInset::lock() const
{
	return lock_;
}


void MathNestInset::lock(bool l)
{
	lock_ = l;
}


bool MathNestInset::isActive() const
{
	return nargs() > 0;
}


MathArray MathNestInset::glue() const
{
	MathArray ar;
	for (size_t i = 0; i < nargs(); ++i)
		ar.append(cell(i));
	return ar;
}


void MathNestInset::write(WriteStream & os) const
{
	os << '\\' << name().c_str();
	for (size_t i = 0; i < nargs(); ++i)
		os << '{' << cell(i) << '}';
	if (nargs() == 0)
		os.pendingSpace(true);
	if (lock_ && !os.latex()) {
		os << "\\lyxlock";
		os.pendingSpace(true);
	}
}


void MathNestInset::normalize(NormalStream & os) const
{
	os << '[' << name().c_str();
	for (size_t i = 0; i < nargs(); ++i)
		os << ' ' << cell(i);
	os << ']';
}


void MathNestInset::notifyCursorLeaves(idx_type idx)
{
	cell(idx).notifyCursorLeaves();
}


DispatchResult
MathNestInset::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	//lyxerr << "InsetFormulaBase::localDispatch: act: " << cmd.action
	//	<< " arg: '" << cmd.argument
	//	<< "' x: '" << cmd.x
	//	<< " y: '" << cmd.y
	//	<< "' button: " << cmd.button() << endl;

	switch (cmd.action) {

	case LFUN_PASTE: {
		MathArray ar;
		mathed_parse_cell(ar, cmd.argument);
		cur.cell().insert(cur.pos(), ar);
		cur.pos() += ar.size();
		return DispatchResult(true, true);
	}

	case LFUN_PASTESELECTION:
		return dispatch(cur, FuncRequest(LFUN_PASTE, cur.bv().getClipboard())); 

	case LFUN_MOUSE_PRESS:
		if (cmd.button() == mouse_button::button2)
			return priv_dispatch(cur, FuncRequest(LFUN_PASTESELECTION));
		return DispatchResult(false);

	case LFUN_RIGHTSEL:
		cur.selection() = true; // fall through...
	case LFUN_RIGHT:
		return cur.right() ?
			DispatchResult(true, true) : DispatchResult(false, FINISHED_RIGHT);
		//lyxerr << "calling scroll 20" << endl;
		//scroll(&cur.bv(), 20);
		// write something to the minibuffer
		//cur.bv().owner()->message(cur.info());

	case LFUN_LEFTSEL:
		cur.selection() = true; // fall through
	case LFUN_LEFT:
		return cur.left() ?
			DispatchResult(true, true) : DispatchResult(false, FINISHED);

	case LFUN_UPSEL:
		cur.selection() = true; // fall through
	case LFUN_UP:
		return cur.up() ?
			DispatchResult(true, true) : DispatchResult(false, FINISHED_UP);

	case LFUN_DOWNSEL:
		cur.selection() = true; // fall through
	case LFUN_DOWN:
		return cur.down() ?
			DispatchResult(true, true) : DispatchResult(false, FINISHED_DOWN);

	case LFUN_WORDSEL:
		cur.home();
		cur.selection() = true;
		cur.end();
		return DispatchResult(true, true);

	case LFUN_UP_PARAGRAPHSEL:
	case LFUN_UP_PARAGRAPH:
	case LFUN_DOWN_PARAGRAPHSEL:
	case LFUN_DOWN_PARAGRAPH:
		return DispatchResult(true, FINISHED);

	case LFUN_HOMESEL:
	case LFUN_WORDLEFTSEL:
		cur.selection() = true; // fall through
	case LFUN_HOME:
	case LFUN_WORDLEFT:
		return cur.home()
			? DispatchResult(true, true) : DispatchResult(true, FINISHED);

	case LFUN_ENDSEL:
	case LFUN_WORDRIGHTSEL:
		cur.selection() = true; // fall through
	case LFUN_END:
	case LFUN_WORDRIGHT:
		return cur.end()
			? DispatchResult(true, true) : DispatchResult(false, FINISHED_RIGHT);

	case LFUN_PRIORSEL:
	case LFUN_PRIOR:
	case LFUN_BEGINNINGBUFSEL:
	case LFUN_BEGINNINGBUF:
		return DispatchResult(true, FINISHED);

	case LFUN_NEXTSEL:
	case LFUN_NEXT:
	case LFUN_ENDBUFSEL:
	case LFUN_ENDBUF:
		return DispatchResult(false, FINISHED_RIGHT);

	case LFUN_CELL_FORWARD:
		cur.inset()->idxNext(cur);
		return DispatchResult(true, true);

	case LFUN_CELL_BACKWARD:
		cur.inset()->idxPrev(cur);
		return DispatchResult(true, true);

	case LFUN_DELETE_WORD_BACKWARD:
	case LFUN_BACKSPACE:
		recordUndo(cur, Undo::ATOMIC);
		cur.backspace();
		return DispatchResult(true, true);

	case LFUN_DELETE_WORD_FORWARD:
	case LFUN_DELETE:
		recordUndo(cur, Undo::ATOMIC);
		cur.erase();
		return DispatchResult(true, FINISHED);

	case LFUN_ESCAPE:
		if (!cur.selection())
			return DispatchResult(true, true);
		cur.selClear();
		return DispatchResult(false);

	case LFUN_INSET_TOGGLE:
		cur.lockToggle();
		return DispatchResult(true, true);

	case LFUN_SELFINSERT:
		if (!cmd.argument.empty()) {
			recordUndo(cur, Undo::ATOMIC);
			if (cmd.argument.size() == 1) {
				if (cur.interpret(cmd.argument[0]))
					return DispatchResult(true, true);
				else
					return DispatchResult(false, FINISHED_RIGHT);
			}
			cur.insert(cmd.argument);
		}
		return DispatchResult(false, FINISHED_RIGHT);


#if 0
//
// this needs to bee incorporated
//
	//lyxerr << "InsetFormulaBase::localDispatch: act: " << cmd.action
	//	<< " arg: '" << cmd.argument
	//	<< "' x: '" << cmd.x
	//	<< " y: '" << cmd.y
	//	<< "' button: " << cmd.button() << endl;

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
	bool was_macro     = cur.inMacroMode();

	cur.normalize();
	cur.touch();

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
		cur.dispatch(cmd);
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
		cur.setScreenPos(x, y);
		break;
	}

	case LFUN_PASTE: {
		size_t n = 0;
		istringstream is(cmd.argument.c_str());
		is >> n;
		if (was_macro)
			cur.macroModeClose();
		recordUndo(cur, Undo::ATOMIC);
		cur.selPaste(n);
		break;
	}

	case LFUN_CUT:
		recordUndo(cur, Undo::DELETE);
		cur.selCut();
		break;

	case LFUN_COPY:
		cur.selCopy();
		break;


	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_CIRCUMFLEX:
		if (cmd.argument.empty()) {
			// do superscript if LyX handles
			// deadkeys
			recordUndo(cur, Undo::ATOMIC);
			cur.script(true);
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
		if (cur.currentMode() == InsetBase::TEXT_MODE)
			cur.niceInsert(MathAtom(new MathHullInset("simple")));
		else
			handleFont(cur, cmd.argument, "textrm");
		//cur.owner()->message(_("math text mode toggled"));
		break;

	case LFUN_MATH_SIZE:
#if 0
		if (!arg.empty()) {
			recordUndo(cur, Undo::ATOMIC);
			cur.setSize(arg);
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
		cur.niceInsert(
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
		cur.handleNest(MathAtom(new MathDelimInset(ls, rs)));
		break;
	}

	case LFUN_SPACE_INSERT:
	case LFUN_MATH_SPACE:
		recordUndo(cur, Undo::ATOMIC);
		cur.insert(MathAtom(new MathSpaceInset(",")));
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
		cur.interpret('\\');
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
		cur.niceInsert(argument);
		break;

	case LFUN_INSET_TOGGLE:
		cur.lockToggle();
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
				cur.insert(ar);
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

	case LFUN_INSERT_MATH:
	case LFUN_INSERT_MATRIX:
	case LFUN_MATH_DELIM: {
		MathHullInset * f = new MathHullInset;
		if (openNewInset(cur, f)) {
			cur.inset()->dispatch(cur, FuncRequest(LFUN_MATH_MUTATE, "simple"));
			cur.inset()->dispatch(cur, cmd);
		}
		break;
	}

	default:
		result = DispatchResult(false);
	}

	if (result == DispatchResult(true, true))
		cur.bv().update();

	cur.normalize();
	cur.touch();

	BOOST_ASSERT(cur.inMathed());

	if (result.dispatched()) {
		revealCodes(cur);
		cur.bv().stuffClipboard(cur.grabSelection());
	} else {
		cur.releaseMathCursor();
		if (remove_inset)
			cur.bv().owner()->dispatch(FuncRequest(LFUN_DELETE));
	}

	return result;  // original version
#endif

	default:
		return MathInset::priv_dispatch(cur, cmd);
	}
}


void MathNestInset::metricsMarkers(int) const
{
	dim_.wid += 2;
	dim_.asc += 1;
}


void MathNestInset::metricsMarkers2(int) const
{
	dim_.wid += 2;
	dim_.asc += 1;
	dim_.des += 1;
}


void MathNestInset::drawMarkers(PainterInfo & pi, int x, int y) const
{
	if (!editing(pi.base.bv))
		return;
	int t = x + dim_.width() - 1;
	int d = y + dim_.descent();
	pi.pain.line(x, d - 3, x, d, LColor::mathframe);
	pi.pain.line(t, d - 3, t, d, LColor::mathframe);
	pi.pain.line(x, d, x + 3, d, LColor::mathframe);
	pi.pain.line(t - 3, d, t, d, LColor::mathframe);
}


void MathNestInset::drawMarkers2(PainterInfo & pi, int x, int y) const
{
	if (!editing(pi.base.bv))
		return;
	drawMarkers(pi, x, y);
	int t = x + dim_.width() - 1;
	int a = y - dim_.ascent();
	pi.pain.line(x, a + 3, x, a, LColor::mathframe);
	pi.pain.line(t, a + 3, t, a, LColor::mathframe);
	pi.pain.line(x, a, x + 3, a, LColor::mathframe);
	pi.pain.line(t - 3, a, t, a, LColor::mathframe);
}
