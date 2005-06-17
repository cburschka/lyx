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
#include "math_boxinset.h"
#include "math_braceinset.h"
#include "math_colorinset.h"
#include "math_commentinset.h"
#include "math_data.h"
#include "math_deliminset.h"
#include "math_factory.h"
#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "math_macroarg.h"
//#include "math_mboxinset.h"
#include "math_parser.h"
#include "math_scriptinset.h"
#include "math_spaceinset.h"
#include "math_symbolinset.h"
#include "math_support.h"
#include "math_unknowninset.h"

#include "BufferView.h"
#include "CutAndPaste.h"
#include "FuncStatus.h"
#include "LColor.h"
#include "bufferview_funcs.h"
#include "coordcache.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "outputparams.h"
#include "undo.h"

#include "support/lstrings.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include <sstream>

using lyx::cap::copySelection;
using lyx::cap::grabAndEraseSelection;
using lyx::cap::cutSelection;
using lyx::cap::pasteSelection;
using lyx::cap::replaceSelection;
using lyx::cap::selClearOrDel;

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


void MathNestInset::getCursorPos(CursorSlice const & sl,
	int & x, int & y) const
{
// FIXME: This is a hack. Ideally, the coord cache should not store
// absolute positions, but relative ones. This would mean to call
// setXY() not in MathArray::draw(), but in the parent insets' draw()
// with the correctly adjusted x,y values. But this means that we'd have
// to touch all (math)inset's draw() methods. Right now, we'll store
// absolute value, and make them here relative, only to make them
// absolute again when actually drawing the cursor. What a mess.
	BOOST_ASSERT(ptr_cmp(&sl.inset(), this));
	MathArray const & ar = sl.cell();
	if (!theCoords.getArrays().has(&ar)) {
		// this can (semi-)legally happen if we just created this cell
		// and it never has been drawn before. So don't ASSERT.
		//lyxerr << "no cached data for array " << &ar << endl;
		x = 0;
		y = 0;
		return;
	}
	Point const pt = theCoords.getArrays().xy(&ar);
	if (!theCoords.getInsets().has(this)) {
		// same as above
		//lyxerr << "no cached data for inset " << this << endl;
		x = 0;
		y = 0;
		return;
	}
	Point const pt2 = theCoords.getInsets().xy(this);
	//lyxerr << "retrieving position cache for MathArray "
	//	<< pt.x_ << ' ' << pt.y_ << std::endl;
	x = pt.x_ - pt2.x_ + ar.pos2x(sl.pos());
	y = pt.y_ - pt2.y_;
//	lyxerr << "pt.y_ : " << pt.y_ << " pt2_.y_ : " << pt2.y_
//		<< " asc: " << ascent() << "  des: " << descent()
//		<< " ar.asc: " << ar.ascent() << " ar.des: " << ar.descent() << endl;
	// move cursor visually into empty cells ("blue rectangles");
	if (ar.empty())
		x += 2;
}


void MathNestInset::metrics(MetricsInfo const & mi) const
{
	MetricsInfo m = mi;
	for (idx_type i = 0, n = nargs(); i != n; ++i)
		cell(i).metrics(m);
}


bool MathNestInset::idxNext(LCursor & cur) const
{
	BOOST_ASSERT(ptr_cmp(&cur.inset(), this));
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
	BOOST_ASSERT(ptr_cmp(&cur.inset(), this));
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
	BOOST_ASSERT(ptr_cmp(&cur.inset(), this));
	if (nargs() == 0)
		return false;
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool MathNestInset::idxLast(LCursor & cur) const
{
	BOOST_ASSERT(ptr_cmp(&cur.inset(), this));
	if (nargs() == 0)
		return false;
	cur.idx() = cur.lastidx();
	cur.pos() = cur.lastpos();
	return true;
}


void MathNestInset::dump() const
{
	WriteStream os(lyxerr);
	os << "---------------------------------------------\n";
	write(os);
	os << "\n";
	for (idx_type i = 0, n = nargs(); i != n; ++i)
		os << cell(i) << "\n";
	os << "---------------------------------------------\n";
}


void MathNestInset::draw(PainterInfo & pi, int x, int y) const
{
#if 0
	if (lock_)
		pi.pain.fillRectangle(x, y - ascent(), width(), height(),
					LColor::mathlockbg);
#endif
	setPosCache(pi, x, y);
}


void MathNestInset::drawSelection(PainterInfo & pi, int x, int y) const
{
	// FIXME: hack to get position cache warm
	draw(pi, x, y);

	// this should use the x/y values given, not the cached values
	LCursor & cur = pi.base.bv->cursor();
	if (!cur.selection())
		return;
	if (!ptr_cmp(&cur.inset(), this))
		return;

	CursorSlice s1 = cur.selBegin();
	CursorSlice s2 = cur.selEnd();
	//lyxerr << "MathNestInset::drawing selection: "
	//	<< " s1: " << s1 << " s2: " << s2 << endl;
	if (s1.idx() == s2.idx()) {
		MathArray const & c = cell(s1.idx());
		int x1 = c.xo() + c.pos2x(s1.pos());
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(s2.pos());
		int y2 = c.yo() + c.descent();
		pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	//lyxerr << "MathNestInset::drawing selection 3: "
	//	<< " x1: " << x1 << " x2: " << x2
	//	<< " y1: " << y1 << " y2: " << y2 << endl;
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


int MathNestInset::latex(Buffer const &, std::ostream & os,
			OutputParams const & runparams) const
{
	WriteStream wi(os, runparams.moving_arg, true);
	write(wi);
	return wi.line();
}


void MathNestInset::notifyCursorLeaves(LCursor & /*cur*/)
{
#ifdef WITH_WARNINGS
#warning look here
#endif
#if 0
	MathArray & ar = cur.cell();
	// remove base-only "scripts"
	for (pos_type i = 0; i + 1 < ar.size(); ++i) {
		MathScriptInset * p = operator[](i).nucleus()->asScriptInset();
		if (p && p->nargs() == 1) {
			MathArray ar = p->nuc();
			erase(i);
			insert(i, ar);
			cur.adjust(i, ar.size() - 1);
		}
	}

	// glue adjacent font insets of the same kind
	for (pos_type i = 0; i + 1 < size(); ++i) {
		MathFontInset * p = operator[](i).nucleus()->asFontInset();
		MathFontInset const * q = operator[](i + 1)->asFontInset();
		if (p && q && p->name() == q->name()) {
			p->cell(0).append(q->cell(0));
			erase(i + 1);
			cur.adjust(i, -1);
		}
	}
#endif
}


void MathNestInset::handleFont
	(LCursor & cur, string const & arg, string const & font)
{
	// this whole function is a hack and won't work for incremental font
	// changes...
	recordUndo(cur, Undo::ATOMIC);

	if (cur.inset().asMathInset()->name() == font)
		cur.handleFont(font);
	else {
		cur.handleNest(createMathInset(font));
		cur.insert(arg);
	}
}


void MathNestInset::handleFont2(LCursor & cur, string const & arg)
{
	recordUndo(cur, Undo::ATOMIC);
	LyXFont font;
	bool b;
	bv_funcs::string2font(arg, font, b);
	if (font.color() != LColor::inherit) {
		MathAtom at = MathAtom(new MathColorInset(true, font.color()));
		cur.handleNest(at, 0);
	}
}


void MathNestInset::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "MathNestInset: request: " << cmd << std::endl;
	//CursorSlice sl = cur.current();

	switch (cmd.action) {

	case LFUN_PASTE: {
		recordUndo(cur);
		cur.message(_("Paste"));
		replaceSelection(cur);
		size_t n = 0;
		istringstream is(cmd.argument);
		is >> n;
		pasteSelection(cur, n);
		cur.clearSelection(); // bug 393
		cur.bv().switchKeyMap();
		finishUndo();
		break;
	}

	case LFUN_CUT:
		cutSelection(cur, true, true);
		cur.message(_("Cut"));
		break;

	case LFUN_COPY:
		copySelection(cur);
		cur.message(_("Copy"));
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

	case LFUN_FINISHED_LEFT:
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_RIGHT:
		++cur.pos();
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_UP:
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_DOWN:
		cur.bv().cursor() = cur;
		break;

	case LFUN_RIGHTSEL:
	case LFUN_RIGHT:
		cur.selHandle(cmd.action == LFUN_RIGHTSEL);
		cur.autocorrect() = false;
		cur.clearTargetX();
		cur.macroModeClose();
		if (cur.pos() != cur.lastpos() && cur.openable(cur.nextAtom())) {
			cur.pushLeft(*cur.nextAtom().nucleus());
			cur.inset().idxFirst(cur);
		} else if (cur.posRight() || idxRight(cur)
			|| cur.popRight() || cur.selection())
			;
		else
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		break;

	case LFUN_LEFTSEL:
	case LFUN_LEFT:
		cur.selHandle(cmd.action == LFUN_LEFTSEL);
		cur.autocorrect() = false;
		cur.clearTargetX();
		cur.macroModeClose();
		if (cur.pos() != 0 && cur.openable(cur.prevAtom())) {
			cur.posLeft();
			cur.push(*cur.nextAtom().nucleus());
			cur.inset().idxLast(cur);
		} else if (cur.posLeft() || idxLeft(cur)
			|| cur.popLeft() || cur.selection())
			;
		else
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		break;

	case LFUN_UPSEL:
	case LFUN_UP:
		// FIXME Tried to use clearTargetX and macroModeClose, crashed on cur.up()
		if (cur.inMacroMode()) {
			// Make Helge happy
			cur.macroModeClose();
			break;
		}
		cur.selHandle(cmd.action == LFUN_UPSEL);
		if (!cur.up())
			cmd = FuncRequest(LFUN_FINISHED_UP);
		// fixes bug 1598. Please check!
		cur.normalize();
		break;

	case LFUN_DOWNSEL:
	case LFUN_DOWN:
		if (cur.inMacroMode()) {
			cur.macroModeClose();
			break;
		}
		cur.selHandle(cmd.action == LFUN_DOWNSEL);
		if (!cur.down())
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
		// fixes bug 1598. Please check!
		cur.normalize();
		break;

	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
	case LFUN_WORDSEL:
		cur.pos() = 0;
		cur.idx() = 0;
		cur.resetAnchor();
		cur.selection() = true;
		cur.pos() = cur.lastpos();
		cur.idx() = cur.lastidx();
		break;

	case LFUN_UP_PARAGRAPHSEL:
	case LFUN_UP_PARAGRAPH:
	case LFUN_DOWN_PARAGRAPHSEL:
	case LFUN_DOWN_PARAGRAPH:
		break;

	case LFUN_HOMESEL:
	case LFUN_HOME:
	case LFUN_WORDLEFTSEL:
	case LFUN_WORDLEFT:
		cur.selHandle(cmd.action == LFUN_WORDLEFTSEL || cmd.action == LFUN_HOMESEL);
		cur.macroModeClose();
		if (cur.pos() != 0) {
			cur.pos() = 0;
		} else if (cur.col() != 0) {
			cur.idx() -= cur.col();
			cur.pos() = 0;
		} else if (cur.idx() != 0) {
			cur.idx() = 0;
			cur.pos() = 0;
		} else {
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
			cur.undispatched();
		}
		break;

	case LFUN_WORDRIGHTSEL:
	case LFUN_WORDRIGHT:
	case LFUN_ENDSEL:
	case LFUN_END:
		cur.selHandle(cmd.action == LFUN_WORDRIGHTSEL || cmd.action == LFUN_ENDSEL);
		cur.macroModeClose();
		cur.clearTargetX();
		if (cur.pos() != cur.lastpos()) {
			cur.pos() = cur.lastpos();
		} else if (ncols() && (cur.col() != cur.lastcol())) {
			cur.idx() = cur.idx() - cur.col() + cur.lastcol();
			cur.pos() = cur.lastpos();
		} else if (cur.idx() != cur.lastidx()) {
			cur.idx() = cur.lastidx();
			cur.pos() = cur.lastpos();
		} else {
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			cur.undispatched();
		}
		break;

	case LFUN_PRIORSEL:
	case LFUN_PRIOR:
		cmd = FuncRequest(LFUN_FINISHED_LEFT);
		break;

	case LFUN_NEXTSEL:
	case LFUN_NEXT:
		cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		break;

	case LFUN_CELL_FORWARD:
		cur.inset().idxNext(cur);
		break;

	case LFUN_CELL_BACKWARD:
		cur.inset().idxPrev(cur);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
	case LFUN_BACKSPACE:
		recordUndo(cur, Undo::ATOMIC);
		cur.backspace();
		break;

	case LFUN_DELETE_WORD_FORWARD:
	case LFUN_DELETE:
		recordUndo(cur);
		cur.erase();
		cmd = FuncRequest(LFUN_FINISHED_LEFT);
		break;

	case LFUN_ESCAPE:
		if (cur.selection())
			cur.clearSelection();
		else
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		break;

	case LFUN_INSET_TOGGLE:
		recordUndo(cur);
		//lockToggle();
		if (cur.pos() != cur.lastpos()) {
			// toggle previous inset ...
			cur.nextAtom().nucleus()->lock(!cur.nextAtom()->lock());
		} else if (cur.popLeft() && cur.pos() != cur.lastpos()) {
			// ... or enclosing inset if we are in the last inset position
			cur.nextAtom().nucleus()->lock(!cur.nextAtom()->lock());
			++cur.pos();
		}
		break;

	case LFUN_SELFINSERT:
		recordUndo(cur);
		if (cmd.argument.size() != 1) {
			cur.insert(cmd.argument);
			break;
		}
		if (!interpret(cur, cmd.argument[0]))
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		break;

	//case LFUN_GETXY:
	//	sprintf(dispatch_buffer, "%d %d",);
	//	break;

	case LFUN_SETXY: {
		lyxerr << "LFUN_SETXY broken!" << endl;
		int x = 0;
		int y = 0;
		istringstream is(cmd.argument);
		is >> x >> y;
		cur.setScreenPos(x, y);
		break;
	}

	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_CIRCUMFLEX:
		if (cmd.argument.empty()) {
			// do superscript if LyX handles
			// deadkeys
			recordUndo(cur, Undo::ATOMIC);
			script(cur, true);
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
		// ignore math-mode on when already in math mode
		if (currentMode() == InsetBase::MATH_MODE && cmd.argument == "on")
			break;
		cur.macroModeClose();
		selClearOrDel(cur);
		//cur.plainInsert(MathAtom(new MathMBoxInset(cur.bv())));
		cur.plainInsert(MathAtom(new MathBoxInset("mbox")));
		cur.posLeft();
		cur.pushLeft(*cur.nextInset());
#else
		if (currentMode() == InsetBase::TEXT_MODE) {
			cur.niceInsert(MathAtom(new MathHullInset("simple")));
			cur.message(_("create new math text environment ($...$)"));
		} else {
			handleFont(cur, cmd.argument, "textrm");
			cur.message(_("entered math text mode (textrm)"));
		}
#endif
		break;

	case LFUN_MATH_SIZE:
#if 0
		recordUndo(cur);
		cur.setSize(arg);
#endif
		break;

	case LFUN_INSERT_MATRIX: {
		recordUndo(cur, Undo::ATOMIC);
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
		recordUndo(cur, Undo::ATOMIC);
		cur.handleNest(MathAtom(new MathDelimInset(ls, rs)));
		break;
	}

	case LFUN_SPACE_INSERT:
	case LFUN_MATH_SPACE:
		recordUndo(cur, Undo::ATOMIC);
		cur.insert(MathAtom(new MathSpaceInset(",")));
		break;

	case LFUN_INSET_ERT:
		// interpret this as if a backslash was typed
		recordUndo(cur, Undo::ATOMIC);
		interpret(cur, '\\');
		break;

	case LFUN_SUBSCRIPT:
		// interpret this as if a _ was typed
		recordUndo(cur, Undo::ATOMIC);
		interpret(cur, '_');
		break;

	case LFUN_SUPERSCRIPT:
		// interpret this as if a ^ was typed
		recordUndo(cur, Undo::ATOMIC);
		interpret(cur, '^');
		break;

// FIXME: We probably should swap parts of "math-insert" and "self-insert"
// handling such that "self-insert" works on "arbitrary stuff" too, and
// math-insert only handles special math things like "matrix".
	case LFUN_INSERT_MATH: {
		recordUndo(cur, Undo::ATOMIC);
		MathArray ar;
		asArray(cmd.argument, ar);
		int cell(0);
		if (cmd.argument == "\\root")
			cell = 1;
		if (ar.size() == 1 && (ar[0].nucleus()->asNestInset())) {
			cur.handleNest(ar[0], cell);
		} else
			cur.niceInsert(cmd.argument);
		break;
		}

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
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument);
			base->dispatch(cur, fr);
			break;
		}
		MathArray ar;
		if (createMathInset_fromDialogStr(cmd.argument, ar)) {
			cur.insert(ar);
			break;
		}
		cur.undispatched();
		break;
	}

	default:
		MathDimInset::doDispatch(cur, cmd);
		break;
	}
}


bool MathNestInset::getStatus(LCursor & /*cur*/, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	// the font related toggles
	//string tc = "mathnormal";
	bool ret = true;
	string const arg = cmd.argument;
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE:
		flag.enabled(false);
		break;
#if 0
	case LFUN_TABULAR_FEATURE:
		// FIXME: check temporarily disabled
		// valign code
		char align = mathcursor::valign();
		if (align == '\0') {
			enable = false;
			break;
		}
		if (cmd.argument.empty()) {
			flag.clear();
			break;
		}
		if (!contains("tcb", cmd.argument[0])) {
			enable = false;
			break;
		}
		flag.setOnOff(cmd.argument[0] == align);
		break;
	case LFUN_BOLD:
		flag.setOnOff(tc == "mathbf");
		break;
	case LFUN_SANS:
		flag.setOnOff(tc == "mathsf");
		break;
	case LFUN_EMPH:
		flag.setOnOff(tc == "mathcal");
		break;
	case LFUN_ROMAN:
		flag.setOnOff(tc == "mathrm");
		break;
	case LFUN_CODE:
		flag.setOnOff(tc == "mathtt");
		break;
	case LFUN_NOUN:
		flag.setOnOff(tc == "mathbb");
		break;
	case LFUN_DEFAULT:
		flag.setOnOff(tc == "mathnormal");
		break;
#endif
	case LFUN_MATH_MUTATE:
		//flag.setOnOff(mathcursor::formula()->hullType() == cmd.argument);
		flag.setOnOff(false);
		break;

	// we just need to be in math mode to enable that
	case LFUN_MATH_SIZE:
	case LFUN_MATH_SPACE:
	case LFUN_MATH_LIMITS:
	case LFUN_MATH_NONUMBER:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_EXTERN:
		flag.enabled(true);
		break;
	case LFUN_INSERT_MATH: {
		bool const textarg =
			arg == "\\textbf"   || arg == "\\textsf" ||
			arg == "\\textrm"   || arg == "\\textmd" ||
			arg == "\\textit"   || arg == "\\textsc" ||
			arg == "\\textsl"   || arg == "\\textup" ||
			arg == "\\texttt"   || arg == "\\textbb" ||
			arg == "\\textnormal";
		flag.enabled((currentMode() == MATH_MODE && !textarg)
			||   (currentMode() == TEXT_MODE && textarg));
		break;
	}
	case LFUN_INSERT_MATRIX:
		flag.enabled(currentMode() == MATH_MODE);
		break;
	default:
		ret = false;
		break;
	}
	return ret;
}


void MathNestInset::edit(LCursor & cur, bool left)
{
	cur.push(*this);
	cur.idx() = left ? 0 : cur.lastidx();
	cur.pos() = left ? 0 : cur.lastpos();
	cur.resetAnchor();
	lyxerr << "MathNestInset::edit, cur:\n" << cur << endl;
}


InsetBase * MathNestInset::editXY(LCursor & cur, int x, int y) const
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
	MathArray const & ar = cell(idx_min);
	cur.push(const_cast<MathNestInset&>(*this));
	cur.idx() = idx_min;
	cur.pos() = ar.x2pos(x - ar.xo());
	lyxerr << "found cell : " << idx_min << " pos: " << cur.pos() << endl;
	if (dist_min == 0) {
		// hit inside cell
		for (pos_type i = 0, n = ar.size(); i < n; ++i)
			if (ar[i]->covers(x, y))
				return ar[i].nucleus()->editXY(cur, x, y);
	}
	return const_cast<MathNestInset*>(this);
}


void MathNestInset::lfunMousePress(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "## lfunMousePress: buttons: " << cmd.button() << endl;
	if (cmd.button() == mouse_button::button1) {
		//lyxerr << "## lfunMousePress: setting cursor to: " << cur << endl;
		cur.resetAnchor();
		cur.bv().cursor() = cur;
	}

	if (cmd.button() == mouse_button::button2) {
		cur.dispatch(FuncRequest(LFUN_PASTESELECTION));
	}
}


void MathNestInset::lfunMouseMotion(LCursor & cur, FuncRequest & cmd)
{
	// only select with button 1
	if (cmd.button() == mouse_button::button1) {
		LCursor & bvcur = cur.bv().cursor();
		if (bvcur.anchor_.hasPart(cur)) {
			//lyxerr << "## lfunMouseMotion: cursor: " << cur << endl;
			bvcur.setCursor(cur);
			bvcur.selection() = true;
			//lyxerr << "MOTION " << bvcur << endl;
		}
		else {
			cur.undispatched();
		}
	}
}


void MathNestInset::lfunMouseRelease(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "## lfunMouseRelease: buttons: " << cmd.button() << endl;

	if (cmd.button() == mouse_button::button1) {
		//cur.bv().stuffClipboard(cur.grabSelection());
		return;
	}

	if (cmd.button() == mouse_button::button2) {
		MathArray ar;
		asArray(cur.bv().getClipboard(), ar);
		cur.clearSelection();
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

	cur.undispatched();
}


bool MathNestInset::interpret(LCursor & cur, char c)
{
	lyxerr << "interpret 2: '" << c << "'" << endl;
	cur.clearTargetX();

	// handle macroMode
	if (cur.inMacroMode()) {
		string name = cur.macroName();

		/// are we currently typing '#1' or '#2' or...?
		if (name == "\\#") {
			cur.backspace();
			int n = c - '0';
			if (n >= 1 && n <= 9)
				cur.insert(new MathMacroArgument(n));
			return true;
		}

		if (isalpha(c)) {
			cur.activeMacro()->setName(name + c);
			return true;
		}

		// handle 'special char' macros
		if (name == "\\") {
			// remove the '\\'
			if (c == '\\') {
				cur.backspace();
				if (currentMode() == MathInset::TEXT_MODE)
					cur.niceInsert(createMathInset("textbackslash"));
				else
					cur.niceInsert(createMathInset("backslash"));
			} else if (c == '{') {
				cur.backspace();
				cur.niceInsert(MathAtom(new MathBraceInset));
			} else if (c == '%') {
				cur.backspace();
				cur.niceInsert(MathAtom(new MathCommentInset));
			} else if (c == '#') {
				BOOST_ASSERT(cur.activeMacro());
				cur.activeMacro()->setName(name + c);
			} else {
				cur.backspace();
				cur.niceInsert(createMathInset(string(1, c)));
			}
			return true;
		}

		// leave macro mode and try again if necessary
		cur.macroModeClose();
		if (c == '{')
			cur.niceInsert(MathAtom(new MathBraceInset));
		else if (c != ' ')
			interpret(cur, c);
		return true;
	}

	// This is annoying as one has to press <space> far too often.
	// Disable it.

#if 0
		// leave autocorrect mode if necessary
		if (autocorrect() && c == ' ') {
			autocorrect() = false;
			return true;
		}
#endif

	// just clear selection on pressing the space bar
	if (cur.selection() && c == ' ') {
		cur.selection() = false;
		return true;
	}

	selClearOrDel(cur);

	if (c == '\\') {
		//lyxerr << "starting with macro" << endl;
		cur.insert(MathAtom(new MathUnknownInset("\\", false)));
		return true;
	}

	if (c == '\n') {
		if (currentMode() == MathInset::TEXT_MODE)
			cur.insert(c);
		return true;
	}

	if (c == ' ') {
		if (currentMode() == MathInset::TEXT_MODE) {
			// insert spaces in text mode,
			// but suppress direct insertion of two spaces in a row
			// the still allows typing  '<space>a<space>' and deleting the 'a', but
			// it is better than nothing...
			if (!cur.pos() != 0 || cur.prevAtom()->getChar() != ' ')
				cur.insert(c);
			return true;
		}
		if (cur.pos() != 0 && cur.prevAtom()->asSpaceInset()) {
			cur.prevAtom().nucleus()->asSpaceInset()->incSpace();
			return true;
		}
		if (cur.popRight())
			return true;
		// if are at the very end, leave the formula
		return cur.pos() != cur.lastpos();
	}

	// These shouldn't work in text mode:
	if (currentMode() != MathInset::TEXT_MODE) {
		if (c == '_') {
			script(cur, false);
			return true;
		}

		if (c == '^') {
			script(cur, true);
			return true;
		}
		if (c == '~') {
			cur.niceInsert(createMathInset("sim"));
			return true;
		}
	}

	if (c == '{' || c == '}' || c == '&' || c == '$' || c == '#' || c == '%') {
		cur.niceInsert(createMathInset(string(1, c)));
		return true;
	}


	// try auto-correction
	//if (autocorrect() && hasPrevAtom() && math_autocorrect(prevAtom(), c))
	//	return true;

	// no special circumstances, so insert the character without any fuss
	cur.insert(c);
	cur.autocorrect() = true;
	return true;
}


bool MathNestInset::script(LCursor & cur, bool up)
{
	// Hack to get \^ and \_ working
	lyxerr << "handling script: up: " << up << endl;
	if (cur.inMacroMode() && cur.macroName() == "\\") {
		if (up)
			cur.niceInsert(createMathInset("mathcircumflex"));
		else
			interpret(cur, '_');
		return true;
	}

	cur.macroModeClose();
	string safe = grabAndEraseSelection(cur);
	if (asScriptInset() && cur.idx() == 0) {
		// we are in a nucleus of a script inset, move to _our_ script
		MathScriptInset * inset = asScriptInset();
		lyxerr << " going to cell " << inset->idxOfScript(up) << endl;
		inset->ensure(up);
		cur.idx() = inset->idxOfScript(up);
		cur.pos() = 0;
	} else if (cur.pos() != 0 && cur.prevAtom()->asScriptInset()) {
		--cur.pos();
		MathScriptInset * inset = cur.nextAtom().nucleus()->asScriptInset();
		cur.push(*inset);
		cur.idx() = inset->idxOfScript(up);
		cur.pos() = cur.lastpos();
	} else {
		// convert the thing to our left to a scriptinset or create a new
		// one if in the very first position of the array
		if (cur.pos() == 0) {
			lyxerr << "new scriptinset" << endl;
			cur.insert(new MathScriptInset(up));
		} else {
			lyxerr << "converting prev atom " << endl;
			cur.prevAtom() = MathAtom(new MathScriptInset(cur.prevAtom(), up));
		}
		--cur.pos();
		MathScriptInset * inset = cur.nextAtom().nucleus()->asScriptInset();
		cur.push(*inset);
		cur.idx() = 1;
		cur.pos() = 0;
	}
	lyxerr << "pasting 1: safe:\n" << safe << endl;
	cur.paste(safe);
	cur.resetAnchor();
	lyxerr << "pasting 2: safe:\n" << safe << endl;
	return true;
}
