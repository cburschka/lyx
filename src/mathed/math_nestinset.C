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

#include "math_arrayinset.h"
#include "math_data.h"
#include "math_deliminset.h"
#include "math_factory.h"
#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "math_spaceinset.h"
#include "math_support.h"
#include "math_mboxinset.h"

#include "BufferView.h"
#include "bufferview_funcs.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LColor.h"
#include "undo.h"

#include "support/std_sstream.h"
#include "support/lstrings.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"


using std::endl;
using std::string;
using std::istringstream;


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


void MathNestInset::getCursorPos(CursorSlice const & cur,
	int & x, int & y) const
{
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
	MathArray const & ar = cur.cell();
	x = ar.xo() + ar.pos2x(cur.pos());
	y = ar.yo();
	// move cursor visually into empty cells ("blue rectangles");
	if (cur.cell().empty())
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
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
	if (cur.idx() == cur.lastidx())
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
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
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
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
	if (nargs() == 0)
		return false;
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool MathNestInset::idxLast(LCursor & cur) const
{
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
	if (nargs() == 0)
		return false;
	cur.idx() = cur.lastidx();
	cur.pos() = cur.lastpos();
	return true;
}


bool MathNestInset::idxHome(LCursor & cur) const
{
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
	if (cur.pos() == 0)
		return false;
	cur.pos() = 0;
	return true;
}


bool MathNestInset::idxEnd(LCursor & cur) const
{
	BOOST_ASSERT(ptr_cmp(cur.inset(), this));
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


void MathNestInset::drawSelection(PainterInfo & pi, int x, int y) const
{
	// this should use the x/y values given, not the cached values
	LCursor & cur = pi.base.bv->cursor();
	if (!cur.selection())
		return;
	if (!ptr_cmp(cur.inset(), this))
		return;
	CursorSlice & s1 = cur.selBegin();
	CursorSlice & s2 = cur.selEnd();
	if (s1.idx() == s2.idx()) {
		MathArray const & c = s1.cell();
		lyxerr << "###### c.xo(): " << c.xo() << " c.yo(): " << c.yo() << endl;
		int x1 = c.xo() + c.pos2x(s1.pos());
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(s2.pos());
		int y2 = c.yo() + c.descent();
		//pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
		pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::red);
	} else {
		for (idx_type i = 0; i < nargs(); ++i) {
			if (idxBetween(i, s1.idx(), s2.idx())) {
				MathArray const & c = cell(i);
				int x1 = c.xo();
				int y1 = c.yo() - c.ascent();
				int x2 = c.xo() + c.width();
				int y2 = c.yo() + c.descent();
				//pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
				pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::red);
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


void MathNestInset::handleFont
	(LCursor & cur, string const & arg, string const & font)
{
	// this whole function is a hack and won't work for incremental font
	// changes...
	//recordUndo(cur, Undo::ATOMIC);

	if (cur.inset()->asMathInset()->name() == font)
		cur.handleFont(font);
	else {
		cur.handleNest(createMathInset(font));
		cur.insert(arg);
	}
}


void MathNestInset::handleFont2(LCursor & cur, string const & arg)
{
	//recordUndo(cur, Undo::ATOMIC);
	LyXFont font;
	bool b;
	bv_funcs::string2font(arg, font, b);
	if (font.color() != LColor::inherit) {
		MathAtom at = createMathInset("color");
		asArray(lcolor.getGUIName(font.color()), at.nucleus()->cell(0));
		cur.handleNest(at, 1);
	}
}


DispatchResult
MathNestInset::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	lyxerr << "*** MathNestInset: request: " << cmd << std::endl;
	//lyxerr << "InsetFormulaBase::localDispatch: act: " << cmd.action
	//	<< " arg: '" << cmd.argument
	//	<< "' x: '" << cmd.x
	//	<< " y: '" << cmd.y
	//	<< "' button: " << cmd.button() << endl;

	switch (cmd.action) {

	case LFUN_PASTE:
		if (!cmd.argument.empty()) {
			MathArray ar;
			mathed_parse_cell(ar, cmd.argument);
			cur.cell().insert(cur.pos(), ar);
			cur.pos() += ar.size();
		}
		return DispatchResult(true, true);
/*
	case LFUN_PASTE: {
		size_t n = 0;
		istringstream is(cmd.argument.c_str());
		is >> n;
		if (was_macro)
			cur.macroModeClose();
		//recordUndo(cur, Undo::ATOMIC);
		cur.selPaste(n);
		return DispatchResult(true, true);
	}
*/

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
		//recordUndo(cur, Undo::ATOMIC);
		cur.backspace();
		return DispatchResult(true, true);

	case LFUN_DELETE_WORD_FORWARD:
	case LFUN_DELETE:
		//recordUndo(cur, Undo::ATOMIC);
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
			//recordUndo(cur, Undo::ATOMIC);
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
// this needs to be incorporated
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
	bool was_macro     = cur.inMacroMode();

	cur.normalize();
	cur.touch();

	switch (cmd.action) {

	case LFUN_MATH_LIMITS:
		//recordUndo(cur, Undo::ATOMIC);
		cur.dispatch(cmd);
		break;
#endif

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
		return DispatchResult(true, true);
	}

	case LFUN_CUT:
		//recordUndo(cur, Undo::DELETE);
		cur.selCut();
		return DispatchResult(true, true);

	case LFUN_COPY:
		cur.selCopy();
		return DispatchResult(true, true);


	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_CIRCUMFLEX:
		if (cmd.argument.empty()) {
			// do superscript if LyX handles
			// deadkeys
			//recordUndo(cur, Undo::ATOMIC);
			cur.script(true);
		}
		return DispatchResult(true, true);

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
		return DispatchResult(true, true);

	//  Math fonts
	case LFUN_FREEFONT_APPLY:
	case LFUN_FREEFONT_UPDATE:
		handleFont2(cur, cmd.argument);
		return DispatchResult(true, true);

	case LFUN_BOLD:
		handleFont(cur, cmd.argument, "mathbf");
		return DispatchResult(true, true);
	case LFUN_SANS:
		handleFont(cur, cmd.argument, "mathsf");
		return DispatchResult(true, true);
	case LFUN_EMPH:
		handleFont(cur, cmd.argument, "mathcal");
		return DispatchResult(true, true);
	case LFUN_ROMAN:
		handleFont(cur, cmd.argument, "mathrm");
		return DispatchResult(true, true);
	case LFUN_CODE:
		handleFont(cur, cmd.argument, "texttt");
		return DispatchResult(true, true);
	case LFUN_FRAK:
		handleFont(cur, cmd.argument, "mathfrak");
		return DispatchResult(true, true);
	case LFUN_ITAL:
		handleFont(cur, cmd.argument, "mathit");
		return DispatchResult(true, true);
	case LFUN_NOUN:
		handleFont(cur, cmd.argument, "mathbb");
		return DispatchResult(true, true);
	//case LFUN_FREEFONT_APPLY:
		handleFont(cur, cmd.argument, "textrm");
		return DispatchResult(true, true);
	case LFUN_DEFAULT:
		handleFont(cur, cmd.argument, "textnormal");
		return DispatchResult(true, true);

	case LFUN_MATH_MODE:
#if 1
		cur.macroModeClose();
		cur.selClearOrDel();
		cur.plainInsert(MathAtom(new MathMBoxInset(cur.bv())));
		cur.posLeft();
		cur.pushLeft(cur.nextInset());
#else
		if (cur.currentMode() == InsetBase::TEXT_MODE)
			cur.niceInsert(MathAtom(new MathHullInset("simple")));
		else
			handleFont(cur, cmd.argument, "textrm");
		//cur.owner()->message(_("math text mode toggled"));
#endif
		return DispatchResult(true, true);

	case LFUN_MATH_SIZE:
#if 0
		if (!arg.empty()) {
			//recordUndo(cur, Undo::ATOMIC);
			cur.setSize(arg);
		}
#endif
		return DispatchResult(true, true);

	case LFUN_INSERT_MATRIX: {
		//recordUndo(cur, Undo::ATOMIC);
		unsigned int m = 1;
		unsigned int n = 1;
		string v_align;
		string h_align;
		istringstream is(cmd.argument);
		is >> m >> n >> v_align >> h_align;
		if (m < 1)
			m = 1;
		if (n < 1)
			n = 1;
		v_align += 'c';
		cur.niceInsert(
			MathAtom(new MathArrayInset("array", m, n, v_align[0], h_align)));
		return DispatchResult(true, true);
	}

	case LFUN_MATH_DELIM: {
		//lyxerr << "formulabase::LFUN_MATH_DELIM, arg: '" << arg << "'" << endl;
		string ls;
		string rs = lyx::support::split(cmd.argument, ls, ' ');
		// Reasonable default values
		if (ls.empty())
			ls = '(';
		if (rs.empty())
			rs = ')';
		//recordUndo(cur, Undo::ATOMIC);
		cur.handleNest(MathAtom(new MathDelimInset(ls, rs)));
		return DispatchResult(true, true);
	}

	case LFUN_SPACE_INSERT:
	case LFUN_MATH_SPACE:
		//recordUndo(cur, Undo::ATOMIC);
		cur.insert(MathAtom(new MathSpaceInset(",")));
		return DispatchResult(true, true);

	case LFUN_UNDO:
#warning look here
		//cur.bv().owner()->message(_("Invalid action in math mode!"));
		return DispatchResult(true, true);

	case LFUN_INSET_ERT:
		// interpret this as if a backslash was typed
		//recordUndo(cur, Undo::ATOMIC);
		cur.interpret('\\');
		return DispatchResult(true, true);

#if 0
	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	case LFUN_BREAKPARAGRAPH_SKIP:
		cmd.argument = "\n";
		//recordUndo(cur, Undo::ATOMIC);
		cur.niceInsert(argument);
		return DispatchResult(true, true);
#endif

// FIXME: We probably should swap parts of "math-insert" and "self-insert"
// handling such that "self-insert" works on "arbitrary stuff" too, and
// math-insert only handles special math things like "matrix".
	case LFUN_INSERT_MATH:
		//recordUndo(cur, Undo::ATOMIC);
		cur.niceInsert(cmd.argument);
		return DispatchResult(true, true);

	case LFUN_DIALOG_SHOW:
		return DispatchResult(false);

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		string const & name = cmd.argument;
		string data;
#if 0
		if (name == "ref") {
			RefInset tmp(name);
			data = tmp.createDialogStr(name);
		}
#endif
		if (data.empty())
			return DispatchResult(false);
		cur.bv().owner()->getDialogs().show(name, data, 0);
		return DispatchResult(true, true);
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		InsetBase * base = cur.bv().owner()->getDialogs().getOpenInset(name);

		if (base) {
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument);
			return base->dispatch(cur, fr);
		}
		MathArray ar;
		if (createMathInset_fromDialogStr(cmd.argument, ar)) {
			cur.insert(ar);
			return DispatchResult(true, true);
		}
		return DispatchResult(false);
	}

#warning look here
#if 0

	case LFUN_WORD_REPLACE:
	case LFUN_WORD_FIND:
		return
			searchForward(&cur.bv(), cmd.getArg(0), false, false)
				? DispatchResult(true, true) : DispatchResult(false);

	case LFUN_INSERT_MATH:
	case LFUN_INSERT_MATRIX:
	case LFUN_MATH_DELIM: {
		MathHullInset * f = new MathHullInset;
		if (openNewInset(cur, f)) {
			cur.inset()->dispatch(cur, FuncRequest(LFUN_MATH_MUTATE, "simple"));
			cur.inset()->dispatch(cur, cmd);
		}
		return DispatchResult(true, true);
	}

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
		return MathDimInset::priv_dispatch(cur, cmd);
	}
}


void MathNestInset::edit(LCursor & cur, int x, int y)
{
	lyxerr << "Called MathNestInset::edit with '" << x << ' ' << y << "'" << endl;
	cur.push(this);
	int idx_min = 0;
	int dist_min = 1000000;
	for (idx_type i = 0; i < nargs(); ++i) {
		int d = cell(i).dist(x, y);
		if (d < dist_min) {
			dist_min = d;
			idx_min = i;
		}
	}
	MathArray & ar = cell(idx_min);
	cur.push(this);
	cur.idx() = idx_min;
	cur.pos() = ar.x2pos(x - ar.xo());
	lyxerr << "found cell : " << idx_min << " pos: " << cur.pos() << endl;
	if (dist_min == 0) {
		// hit inside cell
		for (pos_type i = 0, n = ar.size(); i < n; ++i)
			if (ar[i]->covers(x, y))
				ar[i].nucleus()->edit(cur, x, y);
	}
}
