/**
 * \file InsetMathNest.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathNest.h"

#include "InsetMathArray.h"
#include "InsetMathAMSArray.h"
#include "InsetMathBig.h"
#include "InsetMathBox.h"
#include "InsetMathBrace.h"
#include "InsetMathChar.h"
#include "InsetMathColor.h"
#include "InsetMathComment.h"
#include "InsetMathDelim.h"
#include "InsetMathEnsureMath.h"
#include "InsetMathHull.h"
#include "InsetMathRef.h"
#include "InsetMathScript.h"
#include "InsetMathSpace.h"
#include "InsetMathSymbol.h"
#include "InsetMathUnknown.h"
#include "MathAutoCorrect.h"
#include "MathCompletionList.h"
#include "MathData.h"
#include "MathFactory.h"
#include "MathMacro.h"
#include "MathMacroArgument.h"
#include "MathParser.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TexRow.h"
#include "Text.h"

#include "frontends/Application.h"
#include "frontends/Clipboard.h"
#include "frontends/Painter.h"
#include "frontends/Selection.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <algorithm>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

using cap::copySelection;
using cap::grabAndEraseSelection;
using cap::cutSelection;
using cap::replaceSelection;
using cap::selClearOrDel;


InsetMathNest::InsetMathNest(Buffer * buf, idx_type nargs)
	: InsetMath(buf), cells_(nargs), lock_(false)
{
	setBuffer(*buf);
}


InsetMathNest::InsetMathNest(InsetMathNest const & inset)
	: InsetMath(inset), cells_(inset.cells_), lock_(inset.lock_)
{}


InsetMathNest::~InsetMathNest()
{
	map<BufferView const *, bool>::iterator it = mouse_hover_.begin();
	map<BufferView const *, bool>::iterator end = mouse_hover_.end();
	for (; it != end; ++it)
		if (it->second)
			it->first->clearLastInset(this);
}


InsetMathNest & InsetMathNest::operator=(InsetMathNest const & inset)
{
	cells_ = inset.cells_;
	lock_ = inset.lock_;
	mouse_hover_.clear();
	InsetMath::operator=(inset);
	return *this;
}


void InsetMathNest::setBuffer(Buffer & buffer)
{
	InsetMath::setBuffer(buffer);
	for (idx_type i = 0, n = nargs(); i != n; ++i) {
		MathData & data = cell(i);
		for (size_t j = 0; j != data.size(); ++j)
			data[j].nucleus()->setBuffer(buffer);
	}
}


InsetMath::idx_type InsetMathNest::nargs() const
{
	return cells_.size();
}


void InsetMathNest::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool /*boundary*/,
		int & x, int & y) const
{
// FIXME: This is a hack. Ideally, the coord cache should not store
// absolute positions, but relative ones. This would mean to call
// setXY() not in MathData::draw(), but in the parent insets' draw()
// with the correctly adjusted x,y values. But this means that we'd have
// to touch all (math)inset's draw() methods. Right now, we'll store
// absolute value, and make them here relative, only to make them
// absolute again when actually drawing the cursor. What a mess.
	LASSERT(&sl.inset() == this, return);
	MathData const & ar = sl.cell();
	CoordCache const & coord_cache = bv.coordCache();
	if (!coord_cache.getArrays().has(&ar)) {
		// this can (semi-)legally happen if we just created this cell
		// and it never has been drawn before. So don't ASSERT.
		//lyxerr << "no cached data for array " << &ar << endl;
		x = 0;
		y = 0;
		return;
	}
	Point const pt = coord_cache.getArrays().xy(&ar);
	if (!coord_cache.getInsets().has(this)) {
		// same as above
		//lyxerr << "no cached data for inset " << this << endl;
		x = 0;
		y = 0;
		return;
	}
	Point const pt2 = coord_cache.getInsets().xy(this);
	//lyxerr << "retrieving position cache for MathData "
	//	<< pt.x_ << ' ' << pt.y_ << endl;
	x = pt.x_ - pt2.x_ + ar.pos2x(&bv, sl.pos());
	y = pt.y_ - pt2.y_;
//	lyxerr << "pt.y_ : " << pt.y_ << " pt2_.y_ : " << pt2.y_
//		<< " asc: " << ascent() << "  des: " << descent()
//		<< " ar.asc: " << ar.ascent() << " ar.des: " << ar.descent() << endl;
	// move cursor visually into empty cells ("blue rectangles");
	if (ar.empty())
		x += 2;
}


void InsetMathNest::metrics(MetricsInfo const & mi) const
{
	MetricsInfo m = mi;
	for (idx_type i = 0, n = nargs(); i != n; ++i) {
		Dimension dim;
		cell(i).metrics(m, dim);
	}
}


void InsetMathNest::updateBuffer(ParIterator const & it, UpdateType utype)
{
	for (idx_type i = 0, n = nargs(); i != n; ++i)
		cell(i).updateBuffer(it, utype);
}



bool InsetMathNest::idxNext(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	if (cur.idx() == cur.lastidx())
		return false;
	++cur.idx();
	cur.pos() = 0;
	return true;
}


bool InsetMathNest::idxForward(Cursor & cur) const
{
	return idxNext(cur);
}


bool InsetMathNest::idxPrev(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	if (cur.idx() == 0)
		return false;
	--cur.idx();
	cur.pos() = cur.lastpos();
	return true;
}


bool InsetMathNest::idxBackward(Cursor & cur) const
{
	return idxPrev(cur);
}


bool InsetMathNest::idxFirst(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	if (nargs() == 0)
		return false;
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathNest::idxLast(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	if (nargs() == 0)
		return false;
	cur.idx() = cur.lastidx();
	cur.pos() = cur.lastpos();
	return true;
}


void InsetMathNest::dump() const
{
	odocstringstream oss;
	otexrowstream ots(oss, false);
	WriteStream os(ots);
	os << "---------------------------------------------\n";
	write(os);
	os << "\n";
	for (idx_type i = 0, n = nargs(); i != n; ++i)
		os << cell(i) << "\n";
	os << "---------------------------------------------\n";
	lyxerr << to_utf8(oss.str());
}


void InsetMathNest::draw(PainterInfo & pi, int x, int y) const
{
#if 0
	if (lock_)
		pi.pain.fillRectangle(x, y - ascent(), width(), height(),
					Color_mathlockbg);
#endif
	setPosCache(pi, x, y);
}


void InsetMathNest::drawSelection(PainterInfo & pi, int x, int y) const
{
	BufferView & bv = *pi.base.bv;
	// this should use the x/y values given, not the cached values
	Cursor & cur = bv.cursor();
	if (!cur.selection())
		return;
	if (&cur.inset() != this)
		return;

	// FIXME: hack to get position cache warm
	bool const original_drawing_state = pi.pain.isDrawingEnabled();
	pi.pain.setDrawingEnabled(false);
	draw(pi, x, y);
	pi.pain.setDrawingEnabled(original_drawing_state);

	CursorSlice s1 = cur.selBegin();
	CursorSlice s2 = cur.selEnd();

	//lyxerr << "InsetMathNest::drawing selection: "
	//	<< " s1: " << s1 << " s2: " << s2 << endl;
	if (s1.idx() == s2.idx()) {
		MathData const & c = cell(s1.idx());
		Geometry const & g = bv.coordCache().getArrays().geometry(&c);
		int x1 = g.pos.x_ + c.pos2x(pi.base.bv, s1.pos());
		int y1 = g.pos.y_ - g.dim.ascent();
		int x2 = g.pos.x_ + c.pos2x(pi.base.bv, s2.pos());
		int y2 = g.pos.y_ + g.dim.descent();
		pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, Color_selection);
	//lyxerr << "InsetMathNest::drawing selection 3: "
	//	<< " x1: " << x1 << " x2: " << x2
	//	<< " y1: " << y1 << " y2: " << y2 << endl;
	} else {
		for (idx_type i = 0; i < nargs(); ++i) {
			if (idxBetween(i, s1.idx(), s2.idx())) {
				MathData const & c = cell(i);
				Geometry const & g = bv.coordCache().getArrays().geometry(&c);
				int x1 = g.pos.x_;
				int y1 = g.pos.y_ - g.dim.ascent();
				int x2 = g.pos.x_ + g.dim.width();
				int y2 = g.pos.y_ + g.dim.descent();
				pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, Color_selection);
			}
		}
	}
}


void InsetMathNest::validate(LaTeXFeatures & features) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}


void InsetMathNest::replace(ReplaceData & rep)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).replace(rep);
}


bool InsetMathNest::contains(MathData const & ar) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		if (cell(i).contains(ar))
			return true;
	return false;
}


bool InsetMathNest::lock() const
{
	return lock_;
}


void InsetMathNest::lock(bool l)
{
	lock_ = l;
}


bool InsetMathNest::isActive() const
{
	return nargs() > 0;
}


MathData InsetMathNest::glue() const
{
	MathData ar;
	for (size_t i = 0; i < nargs(); ++i)
		ar.append(cell(i));
	return ar;
}


void InsetMathNest::write(WriteStream & os) const
{
	MathEnsurer ensurer(os, currentMode() == MATH_MODE);
	ModeSpecifier specifier(os, currentMode(), lockedMode());
	docstring const latex_name = name();
	os << '\\' << latex_name;
	for (size_t i = 0; i < nargs(); ++i) {
		Changer dummy = os.changeRowEntry(TexRow::mathEntry(id(),i));
		os << '{' << cell(i) << '}';
	}
	if (nargs() == 0)
		os.pendingSpace(true);
	if (lock_ && !os.latex()) {
		os << "\\lyxlock";
		os.pendingSpace(true);
	}
}


void InsetMathNest::normalize(NormalStream & os) const
{
	os << '[' << name();
	for (size_t i = 0; i < nargs(); ++i)
		os << ' ' << cell(i);
	os << ']';
}


void InsetMathNest::latex(otexstream & os, OutputParams const & runparams) const
{
	WriteStream wi(os, runparams.moving_arg, true,
			runparams.dryrun ? WriteStream::wsDryrun : WriteStream::wsDefault,
			runparams.encoding);
	wi.canBreakLine(os.canBreakLine());
	Changer dummy = wi.changeRowEntry(TexRow::textEntry(runparams.lastid,
	                                                    runparams.lastpos));
	write(wi);
	// Reset parbreak status after a math inset.
	os.lastChar(0);
	os.canBreakLine(wi.canBreakLine());
}


bool InsetMathNest::setMouseHover(BufferView const * bv, bool mouse_hover)
	const
{
	mouse_hover_[bv] = mouse_hover;
	return true;
}


bool InsetMathNest::notifyCursorLeaves(Cursor const & /*old*/, Cursor & /*cur*/)
{
	// FIXME: look here
#if 0
	MathData & ar = cur.cell();
	// remove base-only "scripts"
	for (pos_type i = 0; i + 1 < ar.size(); ++i) {
		InsetMathScript * p = operator[](i).nucleus()->asScriptInset();
		if (p && p->nargs() == 1) {
			MathData ar = p->nuc();
			erase(i);
			insert(i, ar);
			cur.adjust(i, ar.size() - 1);
		}
	}

	// glue adjacent font insets of the same kind
	for (pos_type i = 0; i + 1 < size(); ++i) {
		InsetMathFont * p = operator[](i).nucleus()->asFontInset();
		InsetMathFont const * q = operator[](i + 1)->asFontInset();
		if (p && q && p->name() == q->name()) {
			p->cell(0).append(q->cell(0));
			erase(i + 1);
			cur.adjust(i, -1);
		}
	}
#endif
	return false;
}


void InsetMathNest::handleFont
	(Cursor & cur, docstring const & arg, char const * const font)
{
	handleFont(cur, arg, from_ascii(font));
}


void InsetMathNest::handleFont(Cursor & cur, docstring const & arg,
	docstring const & font)
{
	cur.recordUndoSelection();

	// this whole function is a hack and won't work for incremental font
	// changes...
	if (cur.inset().asInsetMath()->name() == font)
		cur.handleFont(to_utf8(font));
	else
		handleNest(cur, createInsetMath(font, cur.buffer()), arg);
}


void InsetMathNest::handleNest(Cursor & cur, MathAtom const & nest)
{
	handleNest(cur, nest, docstring());
}


void InsetMathNest::handleNest(Cursor & cur, MathAtom const & nest,
	docstring const & arg)
{
	CursorSlice i1 = cur.selBegin();
	CursorSlice i2 = cur.selEnd();
	if (!i1.inset().asInsetMath())
		return;
	if (i1.idx() == i2.idx()) {
		// the easy case where only one cell is selected
		cur.handleNest(nest);
		cur.insert(arg);
		return;
	}

	// multiple selected cells in a simple non-grid inset
	if (i1.asInsetMath()->nrows() == 0 || i1.asInsetMath()->ncols() == 0) {
		for (idx_type i = i1.idx(); i <= i2.idx(); ++i) {
			// select cell
			cur.idx() = i;
			cur.pos() = 0;
			cur.resetAnchor();
			cur.pos() = cur.lastpos();
			cur.setSelection();

			// change font of cell
			cur.handleNest(nest);
			cur.insert(arg);

			// cur is in the font inset now. If the loop continues,
			// we need to get outside again for the next cell
			if (i + 1 <= i2.idx())
				cur.pop_back();
		}
		return;
	}

	// the complicated case with multiple selected cells in a grid
	row_type r1, r2;
	col_type c1, c2;
	cap::region(i1, i2, r1, r2, c1, c2);
	for (row_type row = r1; row <= r2; ++row) {
		for (col_type col = c1; col <= c2; ++col) {
			// select cell
			cur.idx() = i1.asInsetMath()->index(row, col);
			cur.pos() = 0;
			cur.resetAnchor();
			cur.pos() = cur.lastpos();
			cur.setSelection();

			//
			cur.handleNest(nest);
			cur.insert(arg);

			// cur is in the font inset now. If the loop continues,
			// we need to get outside again for the next cell
			if (col + 1 <= c2 || row + 1 <= r2)
				cur.pop_back();
		}
	}
}


void InsetMathNest::handleFont2(Cursor & cur, docstring const & arg)
{
	cur.recordUndoSelection();
	Font font;
	bool b;
	font.fromString(to_utf8(arg), b);
	if (font.fontInfo().color() != Color_inherit &&
	    font.fontInfo().color() != Color_ignore)
		handleNest(cur, MathAtom(new InsetMathColor(buffer_, true, font.fontInfo().color())));

	// FIXME: support other font changes here as well?
}


void InsetMathNest::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//LYXERR0("InsetMathNest: request: " << cmd);

	Parse::flags parseflg = Parse::QUIET | Parse::USETEXT;

	FuncCode const act = cmd.action();
	switch (act) {

	case LFUN_CLIPBOARD_PASTE:
		parseflg |= Parse::VERBATIM;
		// fall through
	case LFUN_PASTE: {
		if (cur.currentMode() <= TEXT_MODE)
			parseflg |= Parse::TEXTMODE;
		cur.recordUndoSelection();
		cur.message(_("Paste"));
		replaceSelection(cur);
		docstring topaste;
		if (cmd.argument().empty() && !theClipboard().isInternal())
			topaste = theClipboard().getAsText(frontend::Clipboard::PlainTextType);
		else {
			size_t n = 0;
			idocstringstream is(cmd.argument());
			is >> n;
			topaste = cap::selection(n, buffer().params().documentClassPtr());
		}
		cur.niceInsert(topaste, parseflg, false);
		cur.clearSelection(); // bug 393
		cur.forceBufferUpdate();
		cur.finishUndo();
		break;
	}

	case LFUN_CUT:
		cur.recordUndo();
		cutSelection(cur, true, true);
		cur.message(_("Cut"));
		// Prevent stale position >= size crash
		// Probably not necessary anymore, see eraseSelection (gb 2005-10-09)
		cur.normalize();
		cur.forceBufferUpdate();
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

	case LFUN_FINISHED_LEFT: // in math, left is backwards
	case LFUN_FINISHED_BACKWARD:
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_RIGHT: // in math, right is forward
	case LFUN_FINISHED_FORWARD:
		++cur.pos();
		cur.bv().cursor() = cur;
		break;

	case LFUN_CHAR_RIGHT:
	case LFUN_CHAR_LEFT:
	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_FORWARD:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		// fall through
	case LFUN_CHAR_RIGHT_SELECT:
	case LFUN_CHAR_LEFT_SELECT:
	case LFUN_CHAR_BACKWARD_SELECT:
	case LFUN_CHAR_FORWARD_SELECT: {
		// are we in a selection?
		bool select = (act == LFUN_CHAR_RIGHT_SELECT
					   || act == LFUN_CHAR_LEFT_SELECT
					   || act == LFUN_CHAR_BACKWARD_SELECT
					   || act == LFUN_CHAR_FORWARD_SELECT);
		// are we moving forward or backwards?
		// If the command was RIGHT or LEFT, then whether we're moving forward
		// or backwards depends on the cursor movement mode (logical or visual):
		//  * in visual mode, since math is always LTR, right -> forward,
		//    left -> backwards
		//  * in logical mode, the mapping is determined by the
		//    reverseDirectionNeeded() function

		bool forward;
		FuncCode finish_lfun;

		if (act == LFUN_CHAR_FORWARD
				|| act == LFUN_CHAR_FORWARD_SELECT) {
			forward = true;
			finish_lfun = LFUN_FINISHED_FORWARD;
		}
		else if (act == LFUN_CHAR_BACKWARD
				|| act == LFUN_CHAR_BACKWARD_SELECT) {
			forward = false;
			finish_lfun = LFUN_FINISHED_BACKWARD;
		}
		else {
			bool right = (act == LFUN_CHAR_RIGHT_SELECT
						  || act == LFUN_CHAR_RIGHT);
			if (lyxrc.visual_cursor || !cur.reverseDirectionNeeded())
				forward = right;
			else
				forward = !right;

			if (right)
				finish_lfun = LFUN_FINISHED_RIGHT;
			else
				finish_lfun = LFUN_FINISHED_LEFT;
		}
		// Now that we know exactly what we want to do, let's do it!
		cur.selHandle(select);
		cur.clearTargetX();
		cur.macroModeClose();
		// try moving forward or backwards as necessary...
		if (!(forward ? cursorMathForward(cur) : cursorMathBackward(cur))) {
			// ... and if movement failed, then finish forward or backwards
			// as necessary
			cmd = FuncRequest(finish_lfun);
			cur.undispatched();
		}
		break;
	}

	case LFUN_DOWN:
	case LFUN_UP:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		// fall through
	case LFUN_DOWN_SELECT:
	case LFUN_UP_SELECT: {
		// close active macro
		if (cur.inMacroMode()) {
			cur.macroModeClose();
			break;
		}

		// stop/start the selection
		bool select = act == LFUN_DOWN_SELECT ||
			act == LFUN_UP_SELECT;
		cur.selHandle(select);

		// handle autocorrect:
		if (lyxrc.autocorrection_math && cur.autocorrect()) {
			cur.autocorrect() = false;
			cur.message(_("Autocorrect Off ('!' to enter)"));
		}

		// go up/down
		bool up = act == LFUN_UP || act == LFUN_UP_SELECT;
		bool successful = cur.upDownInMath(up);
		if (successful)
			break;

		if (cur.fixIfBroken())
			// FIXME: Something bad happened. We pass the corrected Cursor
			// instead of letting things go worse.
			break;

		// We did not manage to move the cursor.
		cur.undispatched();
		break;
	}

	case LFUN_MOUSE_DOUBLE:
	case LFUN_WORD_SELECT:
		cur.pos() = 0;
		cur.resetAnchor();
		cur.selection(true);
		cur.pos() = cur.lastpos();
		cur.bv().cursor() = cur;
		break;

	case LFUN_MOUSE_TRIPLE:
		cur.idx() = 0;
		cur.pos() = 0;
		cur.resetAnchor();
		cur.selection(true);
		cur.idx() = cur.lastidx();
		cur.pos() = cur.lastpos();
		cur.bv().cursor() = cur;
		break;

	case LFUN_PARAGRAPH_UP:
	case LFUN_PARAGRAPH_DOWN:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		// fall through
	case LFUN_PARAGRAPH_UP_SELECT:
	case LFUN_PARAGRAPH_DOWN_SELECT:
		break;

	case LFUN_LINE_BEGIN:
	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_LEFT:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		// fall through
	case LFUN_LINE_BEGIN_SELECT:
	case LFUN_WORD_BACKWARD_SELECT:
	case LFUN_WORD_LEFT_SELECT:
		cur.selHandle(act == LFUN_WORD_BACKWARD_SELECT ||
				act == LFUN_WORD_LEFT_SELECT ||
				act == LFUN_LINE_BEGIN_SELECT);
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
			cmd = FuncRequest(LFUN_FINISHED_BACKWARD);
			cur.undispatched();
		}
		break;

	case LFUN_WORD_FORWARD:
	case LFUN_WORD_RIGHT:
	case LFUN_LINE_END:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		// fall through
	case LFUN_WORD_FORWARD_SELECT:
	case LFUN_WORD_RIGHT_SELECT:
	case LFUN_LINE_END_SELECT:
		cur.selHandle(act == LFUN_WORD_FORWARD_SELECT ||
				act == LFUN_WORD_RIGHT_SELECT ||
				act == LFUN_LINE_END_SELECT);
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
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		break;

	case LFUN_CELL_FORWARD:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		cur.inset().idxNext(cur);
		break;

	case LFUN_CELL_BACKWARD:
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor);
		cur.inset().idxPrev(cur);
		break;

	case LFUN_WORD_DELETE_BACKWARD:
	case LFUN_CHAR_DELETE_BACKWARD:
		if (cur.pos() == 0)
			// May affect external cell:
			cur.recordUndoInset();
		else if (!cur.inMacroMode())
			cur.recordUndoSelection();
		// if the inset can not be removed from within, delete it
		if (!cur.backspace()) {
			FuncRequest cmd = FuncRequest(LFUN_CHAR_DELETE_FORWARD);
			cur.innerText()->dispatch(cur, cmd);
		}
		break;

	case LFUN_WORD_DELETE_FORWARD:
	case LFUN_CHAR_DELETE_FORWARD:
		if (cur.pos() == cur.lastpos())
			// May affect external cell:
			cur.recordUndoInset();
		else
			cur.recordUndoSelection();
		// if the inset can not be removed from within, delete it
		if (!cur.erase()) {
			FuncRequest cmd = FuncRequest(LFUN_CHAR_DELETE_FORWARD);
			cur.innerText()->dispatch(cur, cmd);
		}
		break;

	case LFUN_ESCAPE:
		if (cur.selection())
			cur.clearSelection();
		else  {
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		break;

	// 'Locks' the math inset. A 'locked' math inset behaves as a unit
	// that is traversed by a single <CursorLeft>/<CursorRight>.
	case LFUN_INSET_TOGGLE:
		cur.recordUndo();
		lock(!lock());
		cur.popForward();
		break;

	case LFUN_SELF_INSERT:
		if (cmd.argument().size() != 1) {
			cur.recordUndoSelection();
			docstring const arg = cmd.argument();
			if (!interpretString(cur, arg))
				cur.insert(arg);
			break;
		}
		// Don't record undo steps if we are in macro mode and thus
		// cmd.argument is the next character of the macro name.
		// Otherwise we'll get an invalid cursor if we undo after
		// the macro was finished and the macro is a known command,
		// e.g. sqrt. Cursor::macroModeClose replaces in this case
		// the InsetMathUnknown with name "frac" by an empty
		// InsetMathFrac -> a pos value > 0 is invalid.
		// A side effect is that an undo before the macro is finished
		// undoes the complete macro, not only the last character.
		// At the time we hit '\' we are not in macro mode, still.
		if (!cur.inMacroMode())
			cur.recordUndoSelection();

		// spacial handling of space. If we insert an inset
		// via macro mode, we want to put the cursor inside it
		// if relevant. Think typing "\frac<space>".
		if (cmd.argument()[0] == ' '
		    && cur.inMacroMode() && cur.macroName() != "\\"
		    && cur.macroModeClose() && cur.pos() > 0) {
			MathAtom const atom = cur.prevAtom();
			if (atom->asNestInset() && atom->isActive()) {
				cur.posBackward();
				cur.pushBackward(*cur.nextInset());
			}
		} else if (!interpretChar(cur, cmd.argument()[0])) {
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		break;

	//case LFUN_SERVER_GET_XY:
	//	break;

	case LFUN_SERVER_SET_XY: {
		lyxerr << "LFUN_SERVER_SET_XY broken!" << endl;
		int x = 0;
		int y = 0;
		istringstream is(to_utf8(cmd.argument()));
		is >> x >> y;
		cur.setScreenPos(x, y);
		break;
	}

	// Special casing for superscript in case of LyX handling
	// dead-keys:
	case LFUN_ACCENT_CIRCUMFLEX:
		if (cmd.argument().empty()) {
			// do superscript if LyX handles
			// deadkeys
			cur.recordUndoSelection();
			script(cur, true, grabAndEraseSelection(cur));
		}
		break;

	case LFUN_ACCENT_UMLAUT:
	case LFUN_ACCENT_ACUTE:
	case LFUN_ACCENT_GRAVE:
	case LFUN_ACCENT_BREVE:
	case LFUN_ACCENT_DOT:
	case LFUN_ACCENT_MACRON:
	case LFUN_ACCENT_CARON:
	case LFUN_ACCENT_TILDE:
	case LFUN_ACCENT_CEDILLA:
	case LFUN_ACCENT_CIRCLE:
	case LFUN_ACCENT_UNDERDOT:
	case LFUN_ACCENT_TIE:
	case LFUN_ACCENT_OGONEK:
	case LFUN_ACCENT_HUNGARIAN_UMLAUT:
		break;

	//  Math fonts
	case LFUN_TEXTSTYLE_APPLY:
	case LFUN_TEXTSTYLE_UPDATE:
		handleFont2(cur, cmd.argument());
		break;

	case LFUN_FONT_BOLD:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "textbf");
		else
			handleFont(cur, cmd.argument(), "mathbf");
		break;
	case LFUN_FONT_BOLDSYMBOL:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "textbf");
		else
			handleFont(cur, cmd.argument(), "boldsymbol");
		break;
	case LFUN_FONT_SANS:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "textsf");
		else
			handleFont(cur, cmd.argument(), "mathsf");
		break;
	case LFUN_FONT_EMPH:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "emph");
		else
			handleFont(cur, cmd.argument(), "mathcal");
		break;
	case LFUN_FONT_ROMAN:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "textrm");
		else
			handleFont(cur, cmd.argument(), "mathrm");
		break;
	case LFUN_FONT_TYPEWRITER:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "texttt");
		else
			handleFont(cur, cmd.argument(), "mathtt");
		break;
	case LFUN_FONT_FRAK:
		handleFont(cur, cmd.argument(), "mathfrak");
		break;
	case LFUN_FONT_ITAL:
		if (currentMode() <= TEXT_MODE)
			handleFont(cur, cmd.argument(), "textit");
		else
			handleFont(cur, cmd.argument(), "mathit");
		break;
	case LFUN_FONT_NOUN:
		if (currentMode() <= TEXT_MODE)
			// FIXME: should be "noun"
			handleFont(cur, cmd.argument(), "textsc");
		else
			handleFont(cur, cmd.argument(), "mathbb");
		break;
	case LFUN_FONT_DEFAULT:
		handleFont(cur, cmd.argument(), "textnormal");
		break;
	case LFUN_FONT_UNDERLINE:
		cur.recordUndo();
		cur.handleNest(createInsetMath("underline", cur.buffer()));
		break;

	case LFUN_MATH_MODE: {
#if 1
		// ignore math-mode on when already in math mode
		if (currentMode() == Inset::MATH_MODE && cmd.argument() == "on")
			break;
		cur.recordUndoSelection();
		cur.macroModeClose();
		docstring const save_selection = grabAndEraseSelection(cur);
		selClearOrDel(cur);
		if (currentMode() <= Inset::TEXT_MODE)
			cur.plainInsert(MathAtom(new InsetMathEnsureMath(buffer_)));
		else
			cur.plainInsert(createInsetMath("text", buffer_));
		cur.posBackward();
		cur.pushBackward(*cur.nextInset());
		cur.niceInsert(save_selection);
		cur.forceBufferUpdate();
#else
		if (currentMode() == Inset::TEXT_MODE) {
			cur.recordUndoSelection();
			cur.niceInsert(MathAtom(new InsetMathHull("simple", cur.buffer())));
			cur.message(_("create new math text environment ($...$)"));
		} else {
			handleFont(cur, cmd.argument(), "textrm");
			cur.message(_("entered math text mode (textrm)"));
		}
#endif
		break;
	}

	case LFUN_REGEXP_MODE: {
		InsetMath * im = cur.inset().asInsetMath();
		if (im) {
			InsetMathHull * i = im->asHullInset();
			if (i && i->getType() == hullRegexp) {
				cur.message(_("Already in regular expression mode"));
				break;
			}
		}
		cur.macroModeClose();
		docstring const save_selection = grabAndEraseSelection(cur);
		selClearOrDel(cur);
		cur.plainInsert(MathAtom(new InsetMathHull(buffer_, hullRegexp)));
		cur.posBackward();
		cur.pushBackward(*cur.nextInset());
		cur.niceInsert(save_selection);
		cur.message(_("Regular expression editor mode"));
		break;
	}

	case LFUN_MATH_FONT_STYLE: {
		FuncRequest fr = FuncRequest(LFUN_MATH_INSERT, '\\' + cmd.argument());
		doDispatch(cur, fr);
		break;
	}

	case LFUN_MATH_SIZE: {
		FuncRequest fr = FuncRequest(LFUN_MATH_INSERT, cmd.argument());
		doDispatch(cur, fr);
		break;
	}

	case LFUN_MATH_MATRIX: {
		cur.recordUndo();
		unsigned int m = 1;
		unsigned int n = 1;
		docstring v_align;
		docstring h_align;
		idocstringstream is(cmd.argument());
		is >> m >> n >> v_align >> h_align;
		if (m < 1)
			m = 1;
		if (n < 1)
			n = 1;
		v_align += 'c';
		cur.niceInsert(MathAtom(new InsetMathArray(buffer_,
			from_ascii("array"), m, n, (char)v_align[0], h_align)));
		break;
	}

	case LFUN_MATH_AMS_MATRIX: {
		cur.recordUndo();
		unsigned int m = 1;
		unsigned int n = 1;
		docstring name = from_ascii("matrix");
		idocstringstream is(cmd.argument());
		is >> m >> n >> name;
		if (m < 1)
			m = 1;
		if (n < 1)
			n = 1;
		// check if we have a valid decoration
		if (name != "pmatrix" && name != "bmatrix"
			&& name != "Bmatrix" && name != "vmatrix"
			&& name != "Vmatrix" && name != "matrix")
			name = from_ascii("matrix");

		cur.niceInsert(
			MathAtom(new InsetMathAMSArray(buffer_, name, m, n)));
		break;
	}

	case LFUN_MATH_DELIM: {
		docstring ls;
		docstring rs = split(cmd.argument(), ls, ' ');
		// Reasonable default values
		if (ls.empty())
			ls = '(';
		if (rs.empty())
			rs = ')';
		cur.recordUndo();
		cur.handleNest(MathAtom(new InsetMathDelim(buffer_, ls, rs)));
		break;
	}

	case LFUN_MATH_BIGDELIM: {
		docstring const lname  = from_utf8(cmd.getArg(0));
		docstring const ldelim = from_utf8(cmd.getArg(1));
		docstring const rname  = from_utf8(cmd.getArg(2));
		docstring const rdelim = from_utf8(cmd.getArg(3));
		latexkeys const * l = in_word_set(lname);
		bool const have_l = l && l->inset == "big" &&
				    InsetMathBig::isBigInsetDelim(ldelim);
		l = in_word_set(rname);
		bool const have_r = l && l->inset == "big" &&
				    InsetMathBig::isBigInsetDelim(rdelim);
		// We mimic LFUN_MATH_DELIM in case we have an empty left
		// or right delimiter.
		if (have_l || have_r) {
			cur.recordUndo();
			docstring const selection = grabAndEraseSelection(cur);
			selClearOrDel(cur);
			if (have_l)
				cur.insert(MathAtom(new InsetMathBig(lname,
								ldelim)));
			// first insert the right delimiter and then go back
			// and re-insert the selection (bug 7088)
			if (have_r) {
				cur.insert(MathAtom(new InsetMathBig(rname,
								rdelim)));
				cur.posBackward();
			}
			cur.niceInsert(selection);
		}
		// Don't call cur.undispatched() if we did nothing, this would
		// lead to infinite recursion via Text::dispatch().
		break;
	}

	case LFUN_SPACE_INSERT: {
		cur.recordUndoSelection();
		string const name = cmd.getArg(0);
		if (name == "normal")
			cur.insert(MathAtom(new InsetMathSpace(" ", "")));
		else if (name == "protected")
			cur.insert(MathAtom(new InsetMathSpace("~", "")));
		else if (name == "thin" || name == "med" || name == "thick")
			cur.insert(MathAtom(new InsetMathSpace(name + "space", "")));
		else if (name == "hfill*")
			cur.insert(MathAtom(new InsetMathSpace("hspace*{\\fill}", "")));
		else if (name == "quad" || name == "qquad" ||
		         name == "enspace" || name == "enskip" ||
		         name == "negthinspace" || name == "negmedspace" ||
		         name == "negthickspace" || name == "hfill")
			cur.insert(MathAtom(new InsetMathSpace(name, "")));
		else if (name == "hspace" || name == "hspace*") {
			string const len = cmd.getArg(1);
			if (len.empty() || !isValidLength(len)) {
				lyxerr << "LyX function 'space-insert " << name << "' "
				          "needs a valid length argument." << endl;
				break;
			}
			cur.insert(MathAtom(new InsetMathSpace(name, len)));
		} else
			cur.insert(MathAtom(new InsetMathSpace));
		break;
	}

	case LFUN_MATH_SPACE:
		cur.recordUndoSelection();
		if (cmd.argument().empty())
			cur.insert(MathAtom(new InsetMathSpace));
		else {
			string const name = cmd.getArg(0);
			string const len = cmd.getArg(1);
			cur.insert(MathAtom(new InsetMathSpace(name, len)));
		}
		break;

	case LFUN_ERT_INSERT:
		// interpret this as if a backslash was typed
		cur.recordUndo();
		interpretChar(cur, '\\');
		break;

	case LFUN_MATH_SUBSCRIPT:
		// interpret this as if a _ was typed
		cur.recordUndoSelection();
		interpretChar(cur, '_');
		break;

	case LFUN_MATH_SUPERSCRIPT:
		// interpret this as if a ^ was typed
		cur.recordUndoSelection();
		interpretChar(cur, '^');
		break;

	case LFUN_MATH_MACRO_FOLD:
	case LFUN_MATH_MACRO_UNFOLD: {
		Cursor it = cur;
		bool fold = act == LFUN_MATH_MACRO_FOLD;
		bool found = findMacroToFoldUnfold(it, fold);
		if (found) {
			MathMacro * macro = it.nextInset()->asInsetMath()->asMacro();
			cur.recordUndoInset();
			if (fold)
				macro->fold(cur);
			else
				macro->unfold(cur);
		}
		break;
	}

	case LFUN_QUOTE_INSERT:
		// interpret this as if a straight " was typed
		cur.recordUndoSelection();
		interpretChar(cur, '\"');
		break;

// FIXME: We probably should swap parts of "math-insert" and "self-insert"
// handling such that "self-insert" works on "arbitrary stuff" too, and
// math-insert only handles special math things like "matrix".
	case LFUN_MATH_INSERT: {
		cur.recordUndoSelection();
		if (cmd.argument() == "^" || cmd.argument() == "_")
			interpretChar(cur, cmd.argument()[0]);
		else {
			MathData ar;
			asArray(cmd.argument(), ar);
			if (cur.selection() && ar.size() == 1
			    && ar[0]->asNestInset()
			    && ar[0]->asNestInset()->nargs() > 1)
				handleNest(cur, ar[0]);
			else
				cur.niceInsert(cmd.argument());
		}
		break;
	}

	case LFUN_UNICODE_INSERT: {
		if (cmd.argument().empty())
			break;
		docstring hexstring = cmd.argument();
		if (isHex(hexstring)) {
			char_type c = hexToInt(hexstring);
			if (c >= 32 && c < 0x10ffff) {
				docstring s = docstring(1, c);
				FuncCode code = currentMode() == MATH_MODE ?
					LFUN_MATH_INSERT : LFUN_SELF_INSERT;
				lyx::dispatch(FuncRequest(code, s));
			}
		}
		break;
	}

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		docstring const & name = cmd.argument();
		string data;
		if (name == "ref") {
			InsetMathRef tmp(buffer_, name);
			data = tmp.createDialogStr();
			cur.bv().showDialog(to_utf8(name), data);
		} else if (name == "mathspace") {
			cur.bv().showDialog(to_utf8(name));
		}
		break;
	}

	case LFUN_INSET_INSERT: {
		MathData ar;
		if (createInsetMath_fromDialogStr(cmd.argument(), ar)) {
			cur.recordUndoSelection();
			cur.insert(ar);
			cur.forceBufferUpdate();
		} else
			cur.undispatched();
		break;
	}
	case LFUN_INSET_DISSOLVE:
		if (!asHullInset()) {
			cur.recordUndoInset();
			cur.pullArg();
		}
		break;

	default:
		InsetMath::doDispatch(cur, cmd);
		break;
	}
}


bool InsetMathNest::findMacroToFoldUnfold(Cursor & it, bool fold) const {
	// look for macro to open/close, but stay in mathed
	for (; !it.empty(); it.pop_back()) {

		// go backward through the current cell
		Inset * inset = it.nextInset();
		while (inset && inset->asInsetMath()) {
			MathMacro * macro = inset->asInsetMath()->asMacro();
			if (macro) {
				// found the an macro to open/close?
				if (macro->folded() != fold)
					return true;

				// Wrong folding state.
				// If this was the first we see in this slice, look further left,
				// otherwise go up.
				if (inset != it.nextInset())
					break;
			}

			// go up if this was the left most position
			if (it.pos() == 0)
				break;

			// go left
			it.pos()--;
			inset = it.nextInset();
		}
	}

	return false;
}


bool InsetMathNest::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	// the font related toggles
	//string tc = "mathnormal";
	bool ret = true;
	string const arg = to_utf8(cmd.argument());
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		flag.setEnabled(false);
		break;
#if 0
	case LFUN_INSET_MODIFY:
		// FIXME: check temporarily disabled
		// valign code
		char align = mathcursor::valign();
		if (align == '\0') {
			enable = false;
			break;
		}
		if (cmd.argument().empty()) {
			flag.clear();
			break;
		}
		if (!contains("tcb", cmd.argument()[0])) {
			enable = false;
			break;
		}
		flag.setOnOff(cmd.argument()[0] == align);
		break;
#endif
	/// We have to handle them since 1.4 blocks all unhandled actions
	case LFUN_FONT_ITAL:
	case LFUN_FONT_BOLD:
	case LFUN_FONT_BOLDSYMBOL:
	case LFUN_FONT_SANS:
	case LFUN_FONT_EMPH:
	case LFUN_FONT_TYPEWRITER:
	case LFUN_FONT_NOUN:
	case LFUN_FONT_ROMAN:
	case LFUN_FONT_DEFAULT:
		flag.setEnabled(true);
		break;

	// we just need to be in math mode to enable that
	case LFUN_MATH_SIZE:
	case LFUN_MATH_SPACE:
	case LFUN_MATH_EXTERN:
		flag.setEnabled(true);
		break;

	case LFUN_FONT_UNDERLINE:
	case LFUN_FONT_FRAK:
		flag.setEnabled(currentMode() != TEXT_MODE);
		break;

	case LFUN_MATH_FONT_STYLE: {
		bool const textarg =
			arg == "textbf"   || arg == "textsf" ||
			arg == "textrm"   || arg == "textmd" ||
			arg == "textit"   || arg == "textsc" ||
			arg == "textsl"   || arg == "textup" ||
			arg == "texttt"   || arg == "textbb" ||
			arg == "textnormal";
		flag.setEnabled(currentMode() != TEXT_MODE || textarg);
		break;
	}

	case LFUN_MATH_MODE:
		// forbid "math-mode on" in math mode to prevent irritating
		// behaviour of menu entries (bug 6709)
		flag.setEnabled(currentMode() == TEXT_MODE || arg != "on");
		break;

	case LFUN_MATH_INSERT:
		flag.setEnabled(currentMode() != TEXT_MODE);
		break;

	case LFUN_MATH_AMS_MATRIX:
	case LFUN_MATH_MATRIX:
		flag.setEnabled(currentMode() == MATH_MODE);
		break;

	case LFUN_INSET_INSERT: {
		// Don't test createMathInset_fromDialogStr(), since
		// getStatus is not called with a valid reference and the
		// dialog would not be applyable.
		string const name = cmd.getArg(0);
		flag.setEnabled(name == "ref" || name == "mathspace");
		break;
	}

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		docstring const & name = cmd.argument();
		if (name == "space")
			flag.setEnabled(false);
		break;
	}


	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		// Don't do this with multi-cell selections
		flag.setEnabled(cur.selBegin().idx() == cur.selEnd().idx());
		break;

	case LFUN_MATH_MACRO_FOLD:
	case LFUN_MATH_MACRO_UNFOLD: {
		Cursor it = cur;
		bool found = findMacroToFoldUnfold(it, cmd.action() == LFUN_MATH_MACRO_FOLD);
		flag.setEnabled(found);
		break;
	}

	case LFUN_SPECIALCHAR_INSERT:
	case LFUN_SCRIPT_INSERT:
		// FIXME: These would probably make sense in math-text mode
		flag.setEnabled(false);
		break;

	case LFUN_CAPTION_INSERT:
		flag.setEnabled(false);
		break;

	case LFUN_SPACE_INSERT: {
		docstring const & name = cmd.argument();
		if (name == "visible")
			flag.setEnabled(false);
		break;
	}

	case LFUN_INSET_DISSOLVE:
		flag.setEnabled(!asHullInset());
		break;

	case LFUN_PASTE: {
		docstring const & name = cmd.argument();
		if (name == "html" || name == "latex")
			flag.setEnabled(false);
		break;
	}

	default:
		ret = false;
		break;
	}
	return ret;
}


void InsetMathNest::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.push(*this);
	bool enter_front = (entry_from == Inset::ENTRY_DIRECTION_RIGHT ||
		(entry_from == Inset::ENTRY_DIRECTION_IGNORE && front));
	cur.idx() = enter_front ? 0 : cur.lastidx();
	cur.pos() = enter_front ? 0 : cur.lastpos();
	cur.resetAnchor();
	//lyxerr << "InsetMathNest::edit, cur:\n" << cur << endl;
}


Inset * InsetMathNest::editXY(Cursor & cur, int x, int y)
{
	int idx_min = 0;
	int dist_min = 1000000;
	for (idx_type i = 0, n = nargs(); i != n; ++i) {
		int const d = cell(i).dist(cur.bv(), x, y);
		if (d < dist_min) {
			dist_min = d;
			idx_min = i;
		}
	}
	MathData & ar = cell(idx_min);
	cur.push(*this);
	cur.idx() = idx_min;
	cur.pos() = ar.x2pos(&cur.bv(), x - ar.xo(cur.bv()));

	//lyxerr << "found cell : " << idx_min << " pos: " << cur.pos() << endl;
	if (dist_min == 0) {
		// hit inside cell
		for (pos_type i = 0, n = ar.size(); i < n; ++i)
			if (ar[i]->covers(cur.bv(), x, y))
				return ar[i].nucleus()->editXY(cur, x, y);
	}
	return this;
}


void InsetMathNest::lfunMousePress(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "## lfunMousePress: buttons: " << cmd.button() << endl;
	BufferView & bv = cur.bv();
	if (cmd.button() == mouse_button::button3) {
		// Don't do anything if we right-click a
		// selection, a context menu will popup.
		if (bv.cursor().selection() && cur >= bv.cursor().selectionBegin()
		      && cur < bv.cursor().selectionEnd()) {
	 		cur.noScreenUpdate();
			return;
		}
	}
	bool do_selection = cmd.button() == mouse_button::button1
		&& cmd.modifier() == ShiftModifier;
	bv.mouseSetCursor(cur, do_selection);
	if (cmd.button() == mouse_button::button1) {
		//lyxerr << "## lfunMousePress: setting cursor to: " << cur << endl;
		// Update the cursor update flags as needed:
		//
		// Update::Decoration: tells to update the decoration
		//                     (visual box corners that define
		//                     the inset)/
		// Update::FitCursor: adjust the screen to the cursor
		//                    position if needed
		// cur.result().update(): don't overwrite previously set flags.
		cur.screenUpdateFlags(Update::Decoration | Update::FitCursor
				| cur.result().screenUpdate());
	} else if (cmd.button() == mouse_button::button2 && lyxrc.mouse_middlebutton_paste) {
		if (cap::selection()) {
			// See comment in Text::dispatch why we do this
			cap::copySelectionToStack();
			cmd = FuncRequest(LFUN_PASTE, "0");
			doDispatch(bv.cursor(), cmd);
		} else {
			MathData ar;
			asArray(theSelection().get(), ar);
			bv.cursor().insert(ar);
		}
	}
}


void InsetMathNest::lfunMouseMotion(Cursor & cur, FuncRequest & cmd)
{
	// only select with button 1
	if (cmd.button() != mouse_button::button1)
		return;

	Cursor & bvcur = cur.bv().cursor();

	// ignore motions deeper nested than the real anchor
	if (!bvcur.realAnchor().hasPart(cur)) {
		cur.undispatched();
		return;
	}

	CursorSlice old = bvcur.top();

	// We continue with our existing selection or start a new one, so don't
	// reset the anchor.
	bvcur.setCursor(cur);
	// Did we actually move?
	if (cur.top() == old)
		// We didn't move one iota, so no need to change selection status
		// or update the screen.
		cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
	else
		bvcur.setSelection();
}


void InsetMathNest::lfunMouseRelease(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "## lfunMouseRelease: buttons: " << cmd.button() << endl;

	if (cmd.button() == mouse_button::button1) {
		if (!cur.selection())
			cur.noScreenUpdate();
		else {
			Cursor & bvcur = cur.bv().cursor();
			bvcur.selection(true);
		}
		return;
	}

	cur.undispatched();
}


bool InsetMathNest::interpretChar(Cursor & cur, char_type const c)
{
	//lyxerr << "interpret 2: '" << c << "'" << endl;
	docstring save_selection;
	if (c == '^' || c == '_')
		save_selection = grabAndEraseSelection(cur);

	cur.clearTargetX();
	Buffer * buf = cur.buffer();

	// handle macroMode
	if (cur.inMacroMode()) {
		docstring name = cur.macroName();

		/// are we currently typing '#1' or '#2' or...?
		if (name == "\\#") {
			cur.backspace();
			int n = c - '0';
			if (n >= 1 && n <= 9)
				cur.insert(new MathMacroArgument(n));
			return true;
		}

		// do not finish macro for known * commands
		bool star_macro = c == '*'
			&& (in_word_set(name.substr(1) + '*')
			    || cur.buffer()->getMacro(name.substr(1) + "*", cur, true));
		if (isAlphaASCII(c) || star_macro) {
			cur.activeMacro()->setName(name + docstring(1, c));
			return true;
		}

		// handle 'special char' macros
		if (name == "\\") {
			// remove the '\\'
			if (c == '\\') {
				cur.backspace();
				if (currentMode() <= InsetMath::TEXT_MODE)
					cur.niceInsert(createInsetMath("textbackslash", buf));
				else
					cur.niceInsert(createInsetMath("backslash", buf));
			} else if (c == '^' && currentMode() == InsetMath::MATH_MODE) {
				cur.backspace();
				cur.niceInsert(createInsetMath("mathcircumflex", buf));
			} else if (c == '{' || c == '%') {
				//using the saved selection as argument
				InsetMathUnknown * p = cur.activeMacro();
				p->finalize();
				MathData sel(cur.buffer());
				asArray(p->selection(), sel);
				cur.backspace();
				if (c == '{')
					cur.niceInsert(MathAtom(new InsetMathBrace(sel)));
				else
					cur.niceInsert(MathAtom(new InsetMathComment(sel)));
			} else if (c == '#') {
				LASSERT(cur.activeMacro(), return false);
				cur.activeMacro()->setName(name + docstring(1, c));
			} else {
				cur.backspace();
				cur.niceInsert(createInsetMath(docstring(1, c), buf));
			}
			return true;
		}

		// One character big delimiters. The others are handled in
		// interpretString().
		latexkeys const * l = in_word_set(name.substr(1));
		if (name[0] == '\\' && l && l->inset == "big") {
			docstring delim;
			switch (c) {
			case '{':
				delim = from_ascii("\\{");
				break;
			case '}':
				delim = from_ascii("\\}");
				break;
			default:
				delim = docstring(1, c);
				break;
			}
			if (InsetMathBig::isBigInsetDelim(delim)) {
				// name + delim ared a valid InsetMathBig.
				// We can't use cur.macroModeClose() because
				// it does not handle delim.
				InsetMathUnknown * p = cur.activeMacro();
				p->finalize();
				--cur.pos();
				cur.cell().erase(cur.pos());
				cur.plainInsert(MathAtom(
					new InsetMathBig(name.substr(1), delim)));
				return true;
			}
		} else if (name == "\\smash" && c == '[') {
			// We can't use cur.macroModeClose() because
			// it would create an InsetMathPhantom
			InsetMathUnknown * p = cur.activeMacro();
			p->finalize();
			interpretChar(cur, c);
			return true;
		}

		// leave macro mode and try again if necessary
		if (cur.macroModeClose()) {
			MathAtom const atom = cur.prevAtom();
			if (atom->asNestInset() && atom->isActive()) {
				cur.posBackward();
				cur.pushBackward(*cur.nextInset());
			}
		}
		if (c == '{')
			cur.niceInsert(MathAtom(new InsetMathBrace(buf)));
		else if (c != ' ')
			interpretChar(cur, c);
		return true;
	}


	// leave autocorrect mode if necessary
	if (lyxrc.autocorrection_math && c == ' ' && cur.autocorrect()) {
		cur.autocorrect() = false;
		cur.message(_("Autocorrect Off ('!' to enter)"));
		return true;
	}
	if (lyxrc.autocorrection_math && c == '!' && !cur.autocorrect()) {
		cur.autocorrect() = true;
		cur.message(_("Autocorrect On (<space> to exit)"));
		return true;
	}

	// just clear selection on pressing the space bar
	if (cur.selection() && c == ' ') {
		cur.selection(false);
		return true;
	}

	if (c == '\\') {
		//lyxerr << "starting with macro" << endl;
		bool reduced = cap::reduceSelectionToOneCell(cur);
		if (reduced || !cur.selection()) {
			cur.recordUndoInset();
			docstring const safe = cap::grabAndEraseSelection(cur);
			if (!cur.inRegexped())
				cur.insert(MathAtom(new InsetMathUnknown(from_ascii("\\"), safe, false)));
			else
				cur.niceInsert(createInsetMath("backslash", buf));
		}
		return true;
	}

	selClearOrDel(cur);

	if (c == '\n') {
		if (currentMode() <= InsetMath::TEXT_MODE)
			cur.insert(c);
		return true;
	}

	if (c == ' ') {
		if (currentMode() <= InsetMath::TEXT_MODE) {
			// insert spaces in text or undecided mode,
			// but suppress direct insertion of two spaces in a row
			// the still allows typing  '<space>a<space>' and deleting the 'a', but
			// it is better than nothing...
			if (cur.pos() == 0 || cur.prevAtom()->getChar() != ' ') {
				cur.insert(c);
				// FIXME: we have to enable full redraw here because of the
				// visual box corners that define the inset. If we know for
				// sure that we stay within the same cell we can optimize for
				// that using:
				//cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
			}
			return true;
		}
		if (cur.pos() != 0 && cur.prevAtom()->asSpaceInset()) {
			cur.prevAtom().nucleus()->asSpaceInset()->incSpace();
			// FIXME: we have to enable full redraw here because of the
			// visual box corners that define the inset. If we know for
			// sure that we stay within the same cell we can optimize for
			// that using:
			//cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
			return true;
		}

		if (cur.popForward()) {
			// FIXME: we have to enable full redraw here because of the
			// visual box corners that define the inset. If we know for
			// sure that we stay within the same cell we can optimize for
			// that using:
			//cur.screenUpdateFlags(Update::FitCursor);
			return true;
		}

		// if we are at the very end, leave the formula
		return cur.pos() != cur.lastpos();
	}

	// These should be treated differently when not in text mode:
	if (cur.inRegexped()) {
		switch (c) {
		case '^':
			cur.niceInsert(createInsetMath("mathcircumflex", buf));
			break;
		case '{':
		case '}':
		case '#':
		case '%':
		case '_':
			cur.niceInsert(createInsetMath(docstring(1, c), buf));
			break;
		case '~':
			cur.niceInsert(createInsetMath("sim", buf));
			break;
		default:
			cur.insert(c);
		}
		return true;
	} else if (currentMode() != InsetMath::TEXT_MODE) {
		if (c == '_') {
			script(cur, false, save_selection);
			return true;
		}
		if (c == '^') {
			script(cur, true, save_selection);
			return true;
		}
		if (c == '~') {
			cur.niceInsert(createInsetMath("sim", buf));
			return true;
		}
		if (currentMode() == InsetMath::MATH_MODE && !isAsciiOrMathAlpha(c)) {
			MathAtom at = createInsetMath("text", buf);
			at.nucleus()->cell(0).push_back(MathAtom(new InsetMathChar(c)));
			cur.niceInsert(at);
			cur.posForward();
			return true;
		}
	} else {
		if (c == '^') {
			cur.niceInsert(createInsetMath("textasciicircum", buf));
			return true;
		}
		if (c == '~') {
			cur.niceInsert(createInsetMath("textasciitilde", buf));
			return true;
		}
	}

	if (c == '{' || c == '}' || c == '&' || c == '$' || c == '#' ||
	    c == '%' || c == '_') {
		cur.niceInsert(createInsetMath(docstring(1, c), buf));
		return true;
	}


	// try auto-correction
	if (lyxrc.autocorrection_math && cur.autocorrect() && cur.pos() != 0
		  && math_autocorrect(cur.prevAtom(), c))
		return true;

	// no special circumstances, so insert the character without any fuss
	cur.insert(c);
	if (lyxrc.autocorrection_math) {
		if (!cur.autocorrect())
			cur.message(_("Autocorrect Off ('!' to enter)"));
		else
			cur.message(_("Autocorrect On (<space> to exit)"));
	}
	return true;
}


bool InsetMathNest::interpretString(Cursor & cur, docstring const & str)
{
	// Create a InsetMathBig from cur.cell()[cur.pos() - 1] and t if
	// possible
	if (!cur.empty() && cur.pos() > 0 &&
	    cur.cell()[cur.pos() - 1]->asUnknownInset()) {
		if (InsetMathBig::isBigInsetDelim(str)) {
			docstring prev = asString(cur.cell()[cur.pos() - 1]);
			if (prev[0] == '\\') {
				prev = prev.substr(1);
				latexkeys const * l = in_word_set(prev);
				if (l && l->inset == "big") {
					cur.cell()[cur.pos() - 1] =
						MathAtom(new InsetMathBig(prev, str));
					return true;
				}
			}
		}
	}
	return false;
}


bool InsetMathNest::script(Cursor & cur, bool up)
{
	return script(cur, up, docstring());
}


bool InsetMathNest::script(Cursor & cur, bool up,
		docstring const & save_selection)
{
	// Hack to get \^ and \_ working
	//lyxerr << "handling script: up: " << up << endl;
	if (cur.inMacroMode() && cur.macroName() == "\\") {
		if (up)
			cur.niceInsert(createInsetMath("mathcircumflex", cur.buffer()));
		else
			interpretChar(cur, '_');
		return true;
	}

	cur.macroModeClose();
	if (asScriptInset() && cur.idx() == 0) {
		// we are in a nucleus of a script inset, move to _our_ script
		InsetMathScript * inset = asScriptInset();
		//lyxerr << " going to cell " << inset->idxOfScript(up) << endl;
		inset->ensure(up);
		cur.idx() = inset->idxOfScript(up);
		cur.pos() = 0;
	} else if (cur.pos() != 0 && cur.prevAtom()->asScriptInset()) {
		--cur.pos();
		InsetMathScript * inset = cur.nextAtom().nucleus()->asScriptInset();
		cur.push(*inset);
		inset->ensure(up);
		cur.idx() = inset->idxOfScript(up);
		cur.pos() = cur.lastpos();
	} else {
		// convert the thing to our left to a scriptinset or create a new
		// one if in the very first position of the array
		if (cur.pos() == 0) {
			//lyxerr << "new scriptinset" << endl;
			cur.insert(new InsetMathScript(buffer_, up));
		} else {
			//lyxerr << "converting prev atom " << endl;
			cur.prevAtom() = MathAtom(new InsetMathScript(buffer_, cur.prevAtom(), up));
		}
		--cur.pos();
		InsetMathScript * inset = cur.nextAtom().nucleus()->asScriptInset();
		// See comment in MathParser.cpp for special handling of {}-bases

		cur.push(*inset);
		cur.idx() = 1;
		cur.pos() = 0;
	}
	//lyxerr << "inserting selection 1:\n" << save_selection << endl;
	cur.niceInsert(save_selection);
	cur.resetAnchor();
	//lyxerr << "inserting selection 2:\n" << save_selection << endl;
	return true;
}


bool InsetMathNest::completionSupported(Cursor const & cur) const
{
	return cur.inMacroMode();
}


bool InsetMathNest::inlineCompletionSupported(Cursor const & cur) const
{
	return cur.inMacroMode();
}


bool InsetMathNest::automaticInlineCompletion() const
{
	return lyxrc.completion_inline_math;
}


bool InsetMathNest::automaticPopupCompletion() const
{
	return lyxrc.completion_popup_math;
}


CompletionList const *
InsetMathNest::createCompletionList(Cursor const & cur) const
{
	if (!cur.inMacroMode())
		return 0;

	return new MathCompletionList(cur);
}


docstring InsetMathNest::completionPrefix(Cursor const & cur) const
{
	if (!cur.inMacroMode())
		return docstring();

	return cur.activeMacro()->name();
}


bool InsetMathNest::insertCompletion(Cursor & cur, docstring const & s,
				     bool finished)
{
	if (!cur.inMacroMode())
		return false;

	// append completion to active macro
	InsetMathUnknown * inset = cur.activeMacro();
	inset->setName(inset->name() + s);

	// finish macro
	if (finished) {
#if 0
		// FIXME: this creates duplicates in the completion popup
		// which looks ugly. Moreover the changes the list lengths
		// which seems to
		confuse the popup as well.
		MathCompletionList::addToFavorites(inset->name());
#endif
		lyx::dispatch(FuncRequest(LFUN_SELF_INSERT, " "));
	}

	return true;
}


void InsetMathNest::completionPosAndDim(Cursor const & cur, int & x, int & y,
					Dimension & dim) const
{
	Inset const * inset = cur.activeMacro();
	if (!inset)
		return;

	// get inset dimensions
	dim = cur.bv().coordCache().insets().dim(inset);
	// FIXME: these 3 are no accurate, but should depend on the font.
	// Now the popup jumps down if you enter a char with descent > 0.
	dim.des += 3;
	dim.asc += 3;

	// and position
	Point xy = cur.bv().coordCache().insets().xy(inset);
	x = xy.x_;
	y = xy.y_;
}


bool InsetMathNest::cursorMathForward(Cursor & cur)
{
	if (cur.pos() != cur.lastpos() && cur.openable(cur.nextAtom())) {
		cur.pushBackward(*cur.nextAtom().nucleus());
		cur.inset().idxFirst(cur);
		return true;
	}
	if (cur.posForward() || idxForward(cur))
		return true;
	// try to pop forwards --- but don't pop out of math! leave that to
	// the FINISH lfuns
	int s = cur.depth() - 2;
	if (s >= 0 && cur[s].inset().asInsetMath())
		return cur.popForward();
	return false;
}


bool InsetMathNest::cursorMathBackward(Cursor & cur)
{
	if (cur.pos() != 0 && cur.openable(cur.prevAtom())) {
		cur.posBackward();
		cur.push(*cur.nextAtom().nucleus());
		cur.inset().idxLast(cur);
		return true;
	}
	if (cur.posBackward() || idxBackward(cur))
		return true;
	// try to pop backwards --- but don't pop out of math! leave that to
	// the FINISH lfuns
	int s = cur.depth() - 2;
	if (s >= 0 && cur[s].inset().asInsetMath())
		return cur.popBackward();
	return false;
}


////////////////////////////////////////////////////////////////////

MathCompletionList::MathCompletionList(Cursor const & cur)
{
	// fill it with macros from the buffer
	MacroNameSet macros;
	cur.buffer()->listMacroNames(macros);
	MacroNameSet::const_iterator it;
	for (it = macros.begin(); it != macros.end(); ++it) {
		if (cur.buffer()->getMacro(*it, cur, false))
			locals.push_back("\\" + *it);
	}
	sort(locals.begin(), locals.end());

	if (!globals.empty())
		return;

	// fill in global macros
	macros.clear();
	MacroTable::globalMacros().getMacroNames(macros, false);
	//lyxerr << "Globals completion macros: ";
	for (it = macros.begin(); it != macros.end(); ++it) {
		//lyxerr << "\\" + *it << " ";
		globals.push_back("\\" + *it);
	}
	//lyxerr << std::endl;

	// fill in global commands
	globals.push_back(from_ascii("\\boxed"));
	globals.push_back(from_ascii("\\fbox"));
	globals.push_back(from_ascii("\\framebox"));
	globals.push_back(from_ascii("\\makebox"));
	globals.push_back(from_ascii("\\kern"));
	globals.push_back(from_ascii("\\xhookrightarrow"));
	globals.push_back(from_ascii("\\xhookleftarrow"));
	globals.push_back(from_ascii("\\xrightarrow"));
	globals.push_back(from_ascii("\\xRightarrow"));
	globals.push_back(from_ascii("\\xrightharpoondown"));
	globals.push_back(from_ascii("\\xrightharpoonup"));
	globals.push_back(from_ascii("\\xrightleftharpoons"));
	globals.push_back(from_ascii("\\xleftarrow"));
	globals.push_back(from_ascii("\\xLeftarrow"));
	globals.push_back(from_ascii("\\xleftharpoondown"));
	globals.push_back(from_ascii("\\xleftharpoonup"));
	globals.push_back(from_ascii("\\xleftrightarrow"));
	globals.push_back(from_ascii("\\xLeftrightarrow"));
	globals.push_back(from_ascii("\\xleftrightharpoons"));
	globals.push_back(from_ascii("\\xmapsto"));
	globals.push_back(from_ascii("\\split"));
	globals.push_back(from_ascii("\\gathered"));
	globals.push_back(from_ascii("\\aligned"));
	globals.push_back(from_ascii("\\alignedat"));
	globals.push_back(from_ascii("\\cases"));
	globals.push_back(from_ascii("\\substack"));
	globals.push_back(from_ascii("\\xymatrix"));
	globals.push_back(from_ascii("\\Diagram"));
	globals.push_back(from_ascii("\\subarray"));
	globals.push_back(from_ascii("\\array"));
	globals.push_back(from_ascii("\\sqrt"));
	globals.push_back(from_ascii("\\root"));
	globals.push_back(from_ascii("\\tabular"));
	globals.push_back(from_ascii("\\sideset"));
	globals.push_back(from_ascii("\\stackrel"));
	globals.push_back(from_ascii("\\stackrelthree"));
	globals.push_back(from_ascii("\\binom"));
	globals.push_back(from_ascii("\\choose"));
	globals.push_back(from_ascii("\\brace"));
	globals.push_back(from_ascii("\\brack"));
	globals.push_back(from_ascii("\\frac"));
	globals.push_back(from_ascii("\\over"));
	globals.push_back(from_ascii("\\nicefrac"));
	globals.push_back(from_ascii("\\unitfrac"));
	globals.push_back(from_ascii("\\unitfracthree"));
	globals.push_back(from_ascii("\\unitone"));
	globals.push_back(from_ascii("\\unittwo"));
	globals.push_back(from_ascii("\\infer"));
	globals.push_back(from_ascii("\\atop"));
	globals.push_back(from_ascii("\\lefteqn"));
	globals.push_back(from_ascii("\\boldsymbol"));
	globals.push_back(from_ascii("\\bm"));
	globals.push_back(from_ascii("\\color"));
	globals.push_back(from_ascii("\\normalcolor"));
	globals.push_back(from_ascii("\\textcolor"));
	globals.push_back(from_ascii("\\cfrac"));
	globals.push_back(from_ascii("\\cfracleft"));
	globals.push_back(from_ascii("\\cfracright"));
	globals.push_back(from_ascii("\\dfrac"));
	globals.push_back(from_ascii("\\tfrac"));
	globals.push_back(from_ascii("\\dbinom"));
	globals.push_back(from_ascii("\\tbinom"));
	globals.push_back(from_ascii("\\hphantom"));
	globals.push_back(from_ascii("\\phantom"));
	globals.push_back(from_ascii("\\vphantom"));
	globals.push_back(from_ascii("\\cancel"));
	globals.push_back(from_ascii("\\bcancel"));
	globals.push_back(from_ascii("\\xcancel"));
	globals.push_back(from_ascii("\\cancelto"));
	globals.push_back(from_ascii("\\smash"));
	globals.push_back(from_ascii("\\mathclap"));
	globals.push_back(from_ascii("\\mathllap"));
	globals.push_back(from_ascii("\\mathrlap"));
	globals.push_back(from_ascii("\\ensuremath"));
	MathWordList const & words = mathedWordList();
	MathWordList::const_iterator it2;
	//lyxerr << "Globals completion commands: ";
	for (it2 = words.begin(); it2 != words.end(); ++it2) {
		if (it2->second.inset != "macro" && !it2->second.hidden) {
			// macros are already read from MacroTable::globalMacros()
			globals.push_back('\\' + it2->first);
			//lyxerr << '\\' + it2->first << ' ';
		}
	}
	//lyxerr << std::endl;
	sort(globals.begin(), globals.end());
}


MathCompletionList::~MathCompletionList()
{
}


size_type MathCompletionList::size() const
{
	return locals.size() + globals.size();
}


docstring const & MathCompletionList::data(size_t idx) const
{
	size_t lsize = locals.size();
	if (idx >= lsize)
		return globals[idx - lsize];
	else
		return locals[idx];
}


std::string MathCompletionList::icon(size_t idx) const
{
	// get the latex command
	docstring cmd;
	size_t lsize = locals.size();
	if (idx >= lsize)
		cmd = globals[idx - lsize];
	else
		cmd = locals[idx];

	// get the icon resource name by stripping the backslash
	docstring icon_name = frontend::Application::mathIcon(cmd.substr(1));
	if (icon_name.empty())
		return std::string();
	return "images/math/" + to_utf8(icon_name);
}

std::vector<docstring> MathCompletionList::globals;

} // namespace lyx
