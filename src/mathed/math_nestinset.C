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


namespace {

// local global
int first_x;
int first_y;

} // namespace anon




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


void MathNestInset::drawSelection(PainterInfo & pi, int, int) const
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
		MathArray const & c = cell(s1.idx());
		int x1 = c.xo() + c.pos2x(s1.pos());
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(s2.pos());
		int y2 = c.yo() + c.descent();
		pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		for (idx_type i = 0; i < nargs(); ++i) {
			if (idxBetween(i, s1.idx(), s2.idx())) {
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


void MathNestInset::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	lyxerr << "MathNestInset: request: " << cmd << std::endl;

	switch (cmd.action) {

	case LFUN_PASTE:
		if (!cmd.argument.empty()) {
			MathArray ar;
			mathed_parse_cell(ar, cmd.argument);
			cur.cell().insert(cur.pos(), ar);
			cur.pos() += ar.size();
		}
		break;
/*
	case LFUN_PASTE: {
		size_t n = 0;
		istringstream is(cmd.argument.c_str());
		is >> n;
		if (was_macro)
			cur.macroModeClose();
		//recordUndo(cur, Undo::ATOMIC);
		cur.selPaste(n);
		break;
	}
*/

	case LFUN_PASTESELECTION:
		dispatch(cur, FuncRequest(LFUN_PASTE, cur.bv().getClipboard()));
		break;

	case LFUN_MOUSE_PRESS:
		lfunMousePress(cur, cmd);
		break;

	case LFUN_MOUSE_MOTION:
		lfunMouseMotion(cur, cmd);
		break;

	case LFUN_MOUSE_RELEASE:
		lfunMouseRelease(cur, cmd);
		break;

	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		//lyxerr << "Mouse double" << endl;
		//lyxerr << "Mouse triple" << endl;
		dispatch(cur, FuncRequest(LFUN_WORDSEL));
		break;

	case LFUN_FINISHED_LEFT:
		cur.pop(cur.currentDepth());
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_RIGHT:
		cur.pop(cur.currentDepth());
		++cur.pos();
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_UP:
		cur.pop(cur.currentDepth());
		//idxUpDown(cur, true);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_DOWN:
		cur.pop(cur.currentDepth());
		//idxUpDown(cur, false);
		cur.bv().cursor() = cur;
		break;

	case LFUN_RIGHTSEL:
	case LFUN_RIGHT:
		cur.selHandle(cmd.action == LFUN_RIGHTSEL);
		if (!cur.right()) 
			cur.dispatched(FINISHED_RIGHT);
		break;

	case LFUN_LEFTSEL:
	case LFUN_LEFT:
		cur.selHandle(cmd.action == LFUN_LEFTSEL);
		if (!cur.left())
			cur.dispatched(FINISHED);
		break;

	case LFUN_UPSEL:
	case LFUN_UP:
		cur.selHandle(cmd.action == LFUN_UPSEL);
		if (!cur.up())
			cur.dispatched(FINISHED_UP);
		break;

	case LFUN_DOWNSEL:
	case LFUN_DOWN:
		cur.selHandle(cmd.action == LFUN_DOWNSEL);
		if (!cur.down())
			cur.dispatched(FINISHED_DOWN);
		break;

	case LFUN_WORDSEL:
		cur.home();
		cur.resetAnchor();
		cur.selection() = true;
		cur.end();
		break;

	case LFUN_UP_PARAGRAPHSEL:
	case LFUN_UP_PARAGRAPH:
	case LFUN_DOWN_PARAGRAPHSEL:
	case LFUN_DOWN_PARAGRAPH:
		break;

	case LFUN_WORDLEFTSEL:
	case LFUN_WORDLEFT:
		cur.selHandle(cmd.action == LFUN_WORDLEFTSEL);
		if (!cur.home())
			cur.dispatched(FINISHED);
		break;

	case LFUN_WORDRIGHTSEL:
	case LFUN_WORDRIGHT:
		cur.selHandle(cmd.action == LFUN_WORDRIGHTSEL);
		if (!cur.end())
			cur.dispatched(FINISHED_RIGHT);
		break;

	case LFUN_HOMESEL:
	case LFUN_HOME:
		cur.selHandle(cmd.action == LFUN_HOMESEL);
		if (!cur.home())
			cur.dispatched(FINISHED_RIGHT);
		break;

	case LFUN_ENDSEL:
	case LFUN_END:
		cur.selHandle(cmd.action == LFUN_ENDSEL);
		if (!cur.end())
			cur.dispatched(FINISHED_RIGHT);
		break;

	case LFUN_PRIORSEL:
	case LFUN_PRIOR:
	case LFUN_BEGINNINGBUFSEL:
	case LFUN_BEGINNINGBUF:
		cur.dispatched(FINISHED);
		break;

	case LFUN_NEXTSEL:
	case LFUN_NEXT:
	case LFUN_ENDBUFSEL:
	case LFUN_ENDBUF:
		cur.dispatched(FINISHED_RIGHT);
		break;

	case LFUN_CELL_FORWARD:
		cur.inset()->idxNext(cur);
		break;

	case LFUN_CELL_BACKWARD:
		cur.inset()->idxPrev(cur);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
	case LFUN_BACKSPACE:
		//recordUndo(cur, Undo::ATOMIC);
		cur.backspace();
		break;

	case LFUN_DELETE_WORD_FORWARD:
	case LFUN_DELETE:
		//recordUndo(cur, Undo::ATOMIC);
		cur.erase();
		cur.dispatched(FINISHED);
		break;

	case LFUN_ESCAPE:
		if (cur.selection()) 
			cur.selClear();
		else 
			cur.dispatched(FINISHED);
		break;

	case LFUN_INSET_TOGGLE:
		cur.lockToggle();
		break;

	case LFUN_SELFINSERT:
		if (cmd.argument.empty()) {
			cur.dispatched(FINISHED_RIGHT);
			break;
		}
		//recordUndo(cur, Undo::ATOMIC);
		if (cmd.argument.size() != 1) {
			cur.insert(cmd.argument);
			break;
		}
		if (!cur.interpret(cmd.argument[0]))
			cur.dispatched(FINISHED_RIGHT);
		break;

#if 0
//
// this needs to be incorporated
//
	// delete empty mathbox (LFUN_BACKSPACE and LFUN_DELETE)
	bool remove_inset = false;

	DispatchResult result(true);
	bool was_macro     = cur.inMacroMode();

	cur.normalize();
	cur.touch();
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
		break;
	}

	case LFUN_CUT:
		//recordUndo(cur, Undo::DELETE);
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
			//recordUndo(cur, Undo::ATOMIC);
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

	case LFUN_BOLD:
		handleFont(cur, cmd.argument, "mathbf");
		break;
	case LFUN_SANS:
		handleFont(cur, cmd.argument, "mathsf");
		break;
	case LFUN_EMPH:
		handleFont(cur, cmd.argument, "mathcal");
		break;
	case LFUN_ROMAN:
		handleFont(cur, cmd.argument, "mathrm");
		break;
	case LFUN_CODE:
		handleFont(cur, cmd.argument, "texttt");
		break;
	case LFUN_FRAK:
		handleFont(cur, cmd.argument, "mathfrak");
		break;
	case LFUN_ITAL:
		handleFont(cur, cmd.argument, "mathit");
		break;
	case LFUN_NOUN:
		handleFont(cur, cmd.argument, "mathbb");
		break;
	//case LFUN_FREEFONT_APPLY:
		handleFont(cur, cmd.argument, "textrm");
		break;
	case LFUN_DEFAULT:
		handleFont(cur, cmd.argument, "textnormal");
		break;

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
		break;

	case LFUN_MATH_SIZE:
#if 0
		if (!arg.empty()) {
			//recordUndo(cur, Undo::ATOMIC);
			cur.setSize(arg);
		}
#endif
		break;

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
		break;
	}

	case LFUN_MATH_DELIM: {
		lyxerr << "MathNestInset::LFUN_MATH_DELIM" << endl;
		string ls;
		string rs = lyx::support::split(cmd.argument, ls, ' ');
		// Reasonable default values
		if (ls.empty())
			ls = '(';
		if (rs.empty())
			rs = ')';
		//recordUndo(cur, Undo::ATOMIC);
		cur.handleNest(MathAtom(new MathDelimInset(ls, rs)));
		break;
	}

	case LFUN_SPACE_INSERT:
	case LFUN_MATH_SPACE:
		//recordUndo(cur, Undo::ATOMIC);
		cur.insert(MathAtom(new MathSpaceInset(",")));
		break;

	case LFUN_UNDO:
#warning look here
		//cur.bv().owner()->message(_("Invalid action in math mode!"));
		break;

	case LFUN_INSET_ERT:
		// interpret this as if a backslash was typed
		//recordUndo(cur, Undo::ATOMIC);
		cur.interpret('\\');
		break;

// FIXME: We probably should swap parts of "math-insert" and "self-insert"
// handling such that "self-insert" works on "arbitrary stuff" too, and
// math-insert only handles special math things like "matrix".
	case LFUN_INSERT_MATH:
		//recordUndo(cur, Undo::ATOMIC);
		cur.niceInsert(cmd.argument);
		break;

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		string const & name = cmd.argument;
		string data;
#if 0
		if (name == "ref") {
			RefInset tmp(name);
			data = tmp.createDialogStr(name);
		}
#endif
		cur.bv().owner()->getDialogs().show(name, data, 0);
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		InsetBase * base = cur.bv().owner()->getDialogs().getOpenInset(name);

		if (base) {
			base->dispatch(cur, FuncRequest(LFUN_INSET_MODIFY, cmd.argument));
			break;
		}
		MathArray ar;
		if (createMathInset_fromDialogStr(cmd.argument, ar)) {
			cur.insert(ar);
			break;
		}
		cur.notdispatched();
		break;
	}

#warning look here
#if 0

	case LFUN_WORD_REPLACE:
	case LFUN_WORD_FIND:
		if (!searchForward(&cur.bv(), cmd.getArg(0), false, false))
			cur.notdispatched();
		break;

	cur.normalize();
	cur.touch();

	BOOST_ASSERT(cur.inMathed());

	if (result.dispatched()) {
		revealCodes(cur);
		cur.bv().stuffClipboard(cur.grabSelection());
	} else {
		if (remove_inset)
			cur.bv().owner()->dispatch(FuncRequest(LFUN_DELETE));
	}
	break;
#endif

	default:
		MathDimInset::priv_dispatch(cur, cmd);
		break;
	}
}


void MathNestInset::edit(LCursor & cur, bool left)
{
	cur.push(this);
	cur.idx() = left ? 0 : cur.lastidx();
	cur.pos() = left ? 0 : cur.lastpos();
	cur.resetAnchor();
}


InsetBase * MathNestInset::editXY(LCursor & cur, int x, int y)
{
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
				return ar[i].nucleus()->editXY(cur, x, y);
	}
	return this;
}


void MathNestInset::lfunMouseRelease(LCursor & cur, FuncRequest const & cmd)
{
	//lyxerr << "lfunMouseRelease: buttons: " << cmd.button() << endl;

	if (cmd.button() == mouse_button::button1) {
		// try to dispatch to enclosed insets first
		//cur.bv().stuffClipboard(cur.grabSelection());
		return;
	}

	if (cmd.button() == mouse_button::button2) {
		MathArray ar;
		asArray(cur.bv().getClipboard(), ar);
		cur.selClear();
		cur.setScreenPos(cmd.x, cmd.y);
		cur.insert(ar);
		cur.bv().update();
		return;
	}

	if (cmd.button() == mouse_button::button3) {
		// try to dispatch to enclosed insets first
		cur.bv().owner()->getDialogs().show("mathpanel");
		return;
	}

	cur.notdispatched();
}


void MathNestInset::lfunMousePress(LCursor & cur, FuncRequest const & cmd)
{
	lyxerr << "lfunMousePress: buttons: " << cmd.button() << endl;
	if (cmd.button() == mouse_button::button1) {
		first_x = cmd.x;
		first_y = cmd.y;
		cur.selClear();
		//cur.setScreenPos(cmd.x + xo_, cmd.y + yo_);
		lyxerr << "lfunMousePress: setting cursor to: " << cur << endl;
		cur.bv().cursor() = cur;
	}

	if (cmd.button() == mouse_button::button2) {
		priv_dispatch(cur, FuncRequest(LFUN_PASTESELECTION));
	}
}


void MathNestInset::lfunMouseMotion(LCursor & cur, FuncRequest const & cmd)
{
	// only select with button 1
	if (cmd.button() != mouse_button::button1)
		return;

	if (abs(cmd.x - first_x) < 2 && abs(cmd.y - first_y) < 2)
		return;

	first_x = cmd.x;
	first_y = cmd.y;

	if (!cur.selection())
		cur.selBegin();

	//cur.setScreenPos(cmd.x + xo_, cmd.y + yo_);
	cur.bv().cursor().cursor_ = cur.cursor_;
	cur.bv().cursor().selection() = true;
	return;
}
