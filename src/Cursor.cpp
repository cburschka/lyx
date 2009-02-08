/**
 * \file Cursor.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Alfredo Braunstein
 * \author Dov Feldstern
 * \author André Pönitz
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Bidi.h"
#include "Buffer.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "Font.h"
#include "FuncCode.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LyXFunc.h" // only for setMessage()
#include "LyXRC.h"
#include "paragraph_funcs.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "Row.h"
#include "Text.h"
#include "TextMetrics.h"
#include "TocBackend.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/docstream.h"

#include "insets/InsetTabular.h"
#include "insets/InsetText.h"

#include "mathed/InsetMath.h"
#include "mathed/InsetMathBrace.h"
#include "mathed/InsetMathScript.h"
#include "mathed/MacroTable.h"
#include "mathed/MathData.h"
#include "mathed/MathMacro.h"

#include <boost/bind.hpp>

#include <sstream>
#include <limits>
#include <map>

using namespace std;

namespace lyx {

namespace {

bool positionable(DocIterator const & cursor, DocIterator const & anchor)
{
	// avoid deeper nested insets when selecting
	if (cursor.depth() > anchor.depth())
		return false;

	// anchor might be deeper, should have same path then
	for (size_t i = 0; i < cursor.depth(); ++i)
		if (&cursor[i].inset() != &anchor[i].inset())
			return false;

	// position should be ok.
	return true;
}


// Find position closest to (x, y) in cell given by iter.
// Used only in mathed
DocIterator bruteFind2(Cursor const & c, int x, int y)
{
	double best_dist = numeric_limits<double>::max();

	DocIterator result;

	DocIterator it = c;
	it.top().pos() = 0;
	DocIterator et = c;
	et.top().pos() = et.top().asInsetMath()->cell(et.top().idx()).size();
	for (size_t i = 0;; ++i) {
		int xo;
		int yo;
		Inset const * inset = &it.inset();
		map<Inset const *, Geometry> const & data =
			c.bv().coordCache().getInsets().getData();
		map<Inset const *, Geometry>::const_iterator I = data.find(inset);

		// FIXME: in the case where the inset is not in the cache, this
		// means that no part of it is visible on screen. In this case
		// we don't do elaborate search and we just return the forwarded
		// DocIterator at its beginning.
		if (I == data.end()) {
			it.top().pos() = 0;
			return it;
		}

		Point o = I->second.pos;
		inset->cursorPos(c.bv(), it.top(), c.boundary(), xo, yo);
		// Convert to absolute
		xo += o.x_;
		yo += o.y_;
		double d = (x - xo) * (x - xo) + (y - yo) * (y - yo);
		// '<=' in order to take the last possible position
		// this is important for clicking behind \sum in e.g. '\sum_i a'
		LYXERR(Debug::DEBUG, "i: " << i << " d: " << d
			<< " best: " << best_dist);
		if (d <= best_dist) {
			best_dist = d;
			result = it;
		}
		if (it == et)
			break;
		it.forwardPos();
	}
	return result;
}


/*
/// moves position closest to (x, y) in given box
bool bruteFind(Cursor & cursor,
	int x, int y, int xlow, int xhigh, int ylow, int yhigh)
{
	LASSERT(!cursor.empty(), return false);
	Inset & inset = cursor[0].inset();
	BufferView & bv = cursor.bv();

	CoordCache::InnerParPosCache const & cache =
		bv.coordCache().getParPos().find(cursor.bottom().text())->second;
	// Get an iterator on the first paragraph in the cache
	DocIterator it(inset);
	it.push_back(CursorSlice(inset));
	it.pit() = cache.begin()->first;
	// Get an iterator after the last paragraph in the cache
	DocIterator et(inset);
	et.push_back(CursorSlice(inset));
	et.pit() = boost::prior(cache.end())->first;
	if (et.pit() >= et.lastpit())
		et = doc_iterator_end(inset);
	else
		++et.pit();

	double best_dist = numeric_limits<double>::max();;
	DocIterator best_cursor = et;

	for ( ; it != et; it.forwardPos(true)) {
		// avoid invalid nesting when selecting
		if (!cursor.selection() || positionable(it, cursor.anchor_)) {
			Point p = bv.getPos(it, false);
			int xo = p.x_;
			int yo = p.y_;
			if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
				double const dx = xo - x;
				double const dy = yo - y;
				double const d = dx * dx + dy * dy;
				// '<=' in order to take the last possible position
				// this is important for clicking behind \sum in e.g. '\sum_i a'
				if (d <= best_dist) {
					//	lyxerr << "*" << endl;
					best_dist   = d;
					best_cursor = it;
				}
			}
		}
	}

	if (best_cursor != et) {
		cursor.setCursor(best_cursor);
		return true;
	}

	return false;
}
*/


/// moves position closest to (x, y) in given box
bool bruteFind3(Cursor & cur, int x, int y, bool up)
{
	BufferView & bv = cur.bv();
	int ylow  = up ? 0 : y + 1;
	int yhigh = up ? y - 1 : bv.workHeight();
	int xlow = 0;
	int xhigh = bv.workWidth();

// FIXME: bit more work needed to get 'from' and 'to' right.
	pit_type from = cur.bottom().pit();
	//pit_type to = cur.bottom().pit();
	//lyxerr << "Pit start: " << from << endl;

	//lyxerr << "bruteFind3: x: " << x << " y: " << y
	//	<< " xlow: " << xlow << " xhigh: " << xhigh
	//	<< " ylow: " << ylow << " yhigh: " << yhigh
	//	<< endl;
	DocIterator it = doc_iterator_begin(cur.buffer());
	it.pit() = from;
	DocIterator et = doc_iterator_end(cur.buffer());

	double best_dist = numeric_limits<double>::max();
	DocIterator best_cursor = et;

	for ( ; it != et; it.forwardPos()) {
		// avoid invalid nesting when selecting
		if (bv.cursorStatus(it) == CUR_INSIDE
				&& (!cur.selection() || positionable(it, cur.anchor_))) {
			Point p = bv.getPos(it, false);
			int xo = p.x_;
			int yo = p.y_;
			if (xlow <= xo && xo <= xhigh && ylow <= yo && yo <= yhigh) {
				double const dx = xo - x;
				double const dy = yo - y;
				double const d = dx * dx + dy * dy;
				//lyxerr << "itx: " << xo << " ity: " << yo << " d: " << d
				//	<< " dx: " << dx << " dy: " << dy
				//	<< " idx: " << it.idx() << " pos: " << it.pos()
				//	<< " it:\n" << it
				//	<< endl;
				// '<=' in order to take the last possible position
				// this is important for clicking behind \sum in e.g. '\sum_i a'
				if (d <= best_dist) {
					//lyxerr << "*" << endl;
					best_dist   = d;
					best_cursor = it;
				}
			}
		}
	}

	//lyxerr << "best_dist: " << best_dist << " cur:\n" << best_cursor << endl;
	if (best_cursor == et)
		return false;
	cur.setCursor(best_cursor);
	return true;
}

docstring parbreak(Paragraph const & par)
{
	odocstringstream os;
	os << '\n';
	// only add blank line if we're not in an ERT or Listings inset
	if (par.ownerCode() != ERT_CODE && par.ownerCode() != LISTINGS_CODE)
		os << '\n';
	return os.str();
}

} // namespace anon


// be careful: this is called from the bv's constructor, too, so
// bv functions are not yet available!
Cursor::Cursor(BufferView & bv)
	: DocIterator(&bv.buffer()), bv_(&bv), anchor_(),
	  x_target_(-1), textTargetOffset_(0),
	  selection_(false), mark_(false), logicalpos_(false),
	  current_font(inherit_font)
{}


void Cursor::reset(Inset & inset)
{
	clear();
	push_back(CursorSlice(inset));
	anchor_ = doc_iterator_begin(&inset.buffer(), &inset);
	anchor_.clear();
	clearTargetX();
	selection_ = false;
	mark_ = false;
}


// this (intentionally) does neither touch anchor nor selection status
void Cursor::setCursor(DocIterator const & cur)
{
	DocIterator::operator=(cur);
}


void Cursor::dispatch(FuncRequest const & cmd0)
{
	LYXERR(Debug::DEBUG, "cmd: " << cmd0 << '\n' << *this);
	if (empty())
		return;

	fixIfBroken();
	FuncRequest cmd = cmd0;
	Cursor safe = *this;
	
	// store some values to be used inside of the handlers
	beforeDispatchCursor_ = *this;
	for (; depth(); pop(), boundary(false)) {
		LYXERR(Debug::DEBUG, "Cursor::dispatch: cmd: "
			<< cmd0 << endl << *this);
		LASSERT(pos() <= lastpos(), /**/);
		LASSERT(idx() <= lastidx(), /**/);
		LASSERT(pit() <= lastpit(), /**/);

		// The common case is 'LFUN handled, need update', so make the
		// LFUN handler's life easier by assuming this as default value.
		// The handler can reset the update and val flags if necessary.
		disp_.update(Update::FitCursor | Update::Force);
		disp_.dispatched(true);
		inset().dispatch(*this, cmd);
		if (disp_.dispatched())
			break;
	}
	
	// it completely to get a 'bomb early' behaviour in case this
	// object will be used again.
	if (!disp_.dispatched()) {
		LYXERR(Debug::DEBUG, "RESTORING OLD CURSOR!");
		operator=(safe);
		disp_.update(Update::None);
		disp_.dispatched(false);
	} else {
		// restore the previous one because nested Cursor::dispatch calls
		// are possible which would change it
		beforeDispatchCursor_ = safe.beforeDispatchCursor_;
	}
}


DispatchResult Cursor::result() const
{
	return disp_;
}


BufferView & Cursor::bv() const
{
	LASSERT(bv_, /**/);
	return *bv_;
}


void Cursor::pop()
{
	LASSERT(depth() >= 1, /**/);
	pop_back();
}


void Cursor::push(Inset & p)
{
	push_back(CursorSlice(p));
	p.setBuffer(*buffer());
}


void Cursor::pushBackward(Inset & p)
{
	LASSERT(!empty(), return);
	//lyxerr << "Entering inset " << t << " front" << endl;
	push(p);
	p.idxFirst(*this);
}


bool Cursor::popBackward()
{
	LASSERT(!empty(), return false);
	if (depth() == 1)
		return false;
	pop();
	return true;
}


bool Cursor::popForward()
{
	LASSERT(!empty(), return false);
	//lyxerr << "Leaving inset from in back" << endl;
	const pos_type lp = (depth() > 1) ? (*this)[depth() - 2].lastpos() : 0;
	if (depth() == 1)
		return false;
	pop();
	pos() += lastpos() - lp + 1;
	return true;
}


int Cursor::currentMode()
{
	LASSERT(!empty(), /**/);
	for (int i = depth() - 1; i >= 0; --i) {
		int res = operator[](i).inset().currentMode();
		if (res != Inset::UNDECIDED_MODE)
			return res;
	}
	return Inset::TEXT_MODE;
}


void Cursor::getPos(int & x, int & y) const
{
	Point p = bv().getPos(*this, boundary());
	x = p.x_;
	y = p.y_;
}


Row const & Cursor::textRow() const
{
	CursorSlice const & cs = innerTextSlice();
	ParagraphMetrics const & pm = bv().parMetrics(cs.text(), cs.pit());
	LASSERT(!pm.rows().empty(), /**/);
	return pm.getRow(pos(), boundary());
}


void Cursor::resetAnchor()
{
	anchor_ = *this;
}



bool Cursor::posBackward()
{
	if (pos() == 0)
		return false;
	--pos();
	return true;
}


bool Cursor::posForward()
{
	if (pos() == lastpos())
		return false;
	++pos();
	return true;
}


bool Cursor::posVisRight(bool skip_inset)
{
	Cursor new_cur = *this; // where we will move to
	pos_type left_pos; // position visually left of current cursor
	pos_type right_pos; // position visually right of current cursor
	bool new_pos_is_RTL; // is new position we're moving to RTL?

	getSurroundingPos(left_pos, right_pos);

	LYXERR(Debug::RTL, left_pos <<"|"<< right_pos << " (pos: "<< pos() <<")");

	// Are we at an inset?
	new_cur.pos() = right_pos;
	new_cur.boundary(false);
	if (!skip_inset &&
		text()->checkAndActivateInsetVisual(new_cur, right_pos >= pos(), false)) {
		// we actually move the cursor at the end of this function, for now
		// we just keep track of the new position in new_cur...
		LYXERR(Debug::RTL, "entering inset at: " << new_cur.pos());
	}

	// Are we already at rightmost pos in row?
	else if (text()->empty() || right_pos == -1) {
		
		new_cur = *this;
		if (!new_cur.posVisToNewRow(false)) {
			LYXERR(Debug::RTL, "not moving!");
			return false;
		}
		
		// we actually move the cursor at the end of this function, for now 
		// just keep track of the new position in new_cur...
		LYXERR(Debug::RTL, "right edge, moving: " << int(new_cur.pit()) << "," 
			<< int(new_cur.pos()) << "," << (new_cur.boundary() ? 1 : 0));

	}
	// normal movement to the right
	else {
		new_cur = *this;
		// Recall, if the cursor is at position 'x', that means *before* 
		// the character at position 'x'. In RTL, "before" means "to the 
		// right of", in LTR, "to the left of". So currently our situation
		// is this: the position to our right is 'right_pos' (i.e., we're 
		// currently to the left of 'right_pos'). In order to move to the 
		// right, it depends whether or not the character at 'right_pos' is RTL.
		new_pos_is_RTL = paragraph().getFontSettings(
			buffer()->params(), right_pos).isVisibleRightToLeft();
		// If the character at 'right_pos' *is* LTR, then in order to move to
		// the right of it, we need to be *after* 'right_pos', i.e., move to
		// position 'right_pos' + 1.
		if (!new_pos_is_RTL) {
			new_cur.pos() = right_pos + 1;
			// set the boundary to true in two situations:
			if (
			// 1. if new_pos is now lastpos, and we're in an RTL paragraph
			// (this means that we're moving right to the end of an LTR chunk
			// which is at the end of an RTL paragraph);
				(new_cur.pos() == lastpos()
				 && paragraph().isRTL(buffer()->params()))
			// 2. if the position *after* right_pos is RTL (we want to be 
			// *after* right_pos, not before right_pos + 1!)
				|| paragraph().getFontSettings(buffer()->params(),
						new_cur.pos()).isVisibleRightToLeft()
			)
				new_cur.boundary(true);
			else // set the boundary to false
				new_cur.boundary(false);
		}
		// Otherwise (if the character at position 'right_pos' is RTL), then
		// moving to the right of it is as easy as setting the new position
		// to 'right_pos'.
		else {
			new_cur.pos() = right_pos;
			new_cur.boundary(false);
		}
	
	}

	bool moved = (new_cur.pos() != pos()
				  || new_cur.pit() != pit()
				  || new_cur.boundary() != boundary()
				  || &new_cur.inset() != &inset());
	
	if (moved) {
		LYXERR(Debug::RTL, "moving to: " << new_cur.pos() 
			<< (new_cur.boundary() ? " (boundary)" : ""));
		*this = new_cur;
	}

	return moved;
}


bool Cursor::posVisLeft(bool skip_inset)
{
	Cursor new_cur = *this; // where we will move to
	pos_type left_pos; // position visually left of current cursor
	pos_type right_pos; // position visually right of current cursor
	bool new_pos_is_RTL; // is new position we're moving to RTL?

	getSurroundingPos(left_pos, right_pos);

	LYXERR(Debug::RTL, left_pos <<"|"<< right_pos << " (pos: "<< pos() <<")");

	// Are we at an inset?
	new_cur.pos() = left_pos;
	new_cur.boundary(false);
	if (!skip_inset && 
		text()->checkAndActivateInsetVisual(new_cur, left_pos >= pos(), true)) {
		// we actually move the cursor at the end of this function, for now 
		// we just keep track of the new position in new_cur...
		LYXERR(Debug::RTL, "entering inset at: " << new_cur.pos());
	}

	// Are we already at leftmost pos in row?
	else if (text()->empty() || left_pos == -1) {
		
		new_cur = *this;
		if (!new_cur.posVisToNewRow(true)) {
			LYXERR(Debug::RTL, "not moving!");
			return false;
		}
		
		// we actually move the cursor at the end of this function, for now 
		// just keep track of the new position in new_cur...
		LYXERR(Debug::RTL, "left edge, moving: " << int(new_cur.pit()) << "," 
			<< int(new_cur.pos()) << "," << (new_cur.boundary() ? 1 : 0));

	}
	// normal movement to the left
	else {
		new_cur = *this;
		// Recall, if the cursor is at position 'x', that means *before* 
		// the character at position 'x'. In RTL, "before" means "to the 
		// right of", in LTR, "to the left of". So currently our situation
		// is this: the position to our left is 'left_pos' (i.e., we're 
		// currently to the right of 'left_pos'). In order to move to the 
		// left, it depends whether or not the character at 'left_pos' is RTL.
		new_pos_is_RTL = paragraph().getFontSettings(
			buffer()->params(), left_pos).isVisibleRightToLeft();
		// If the character at 'left_pos' *is* RTL, then in order to move to
		// the left of it, we need to be *after* 'left_pos', i.e., move to
		// position 'left_pos' + 1.
		if (new_pos_is_RTL) {
			new_cur.pos() = left_pos + 1;
			// set the boundary to true in two situations:
			if (
			// 1. if new_pos is now lastpos and we're in an LTR paragraph
			// (this means that we're moving left to the end of an RTL chunk
			// which is at the end of an LTR paragraph);
				(new_cur.pos() == lastpos()
				 && !paragraph().isRTL(buffer()->params()))
			// 2. if the position *after* left_pos is not RTL (we want to be 
			// *after* left_pos, not before left_pos + 1!)
				|| !paragraph().getFontSettings(buffer()->params(),
						new_cur.pos()).isVisibleRightToLeft()
			)
				new_cur.boundary(true);
			else // set the boundary to false
				new_cur.boundary(false);
		}
		// Otherwise (if the character at position 'left_pos' is LTR), then
		// moving to the left of it is as easy as setting the new position
		// to 'left_pos'.
		else {
			new_cur.pos() = left_pos;
			new_cur.boundary(false);
		}
	
	}

	bool moved = (new_cur.pos() != pos() 
				  || new_cur.pit() != pit()
				  || new_cur.boundary() != boundary());

	if (moved) {
		LYXERR(Debug::RTL, "moving to: " << new_cur.pos() 
			<< (new_cur.boundary() ? " (boundary)" : ""));
		*this = new_cur;
	}
		
	return moved;
}


void Cursor::getSurroundingPos(pos_type & left_pos, pos_type & right_pos)
{
	// preparing bidi tables
	Paragraph const & par = paragraph();
	Buffer const & buf = *buffer();
	Row const & row = textRow();
	Bidi bidi;
	bidi.computeTables(par, buf, row);

	LYXERR(Debug::RTL, "bidi: " << row.pos() << "--" << row.endpos());

	// The cursor is painted *before* the character at pos(), or, if 'boundary'
	// is true, *after* the character at (pos() - 1). So we already have one
	// known position around the cursor:
	pos_type known_pos = boundary() ? pos() - 1 : pos();
	
	// edge case: if we're at the end of the paragraph, things are a little 
	// different (because lastpos is a position which does not really "exist" 
	// --- there's no character there yet).
	if (known_pos == lastpos()) {
		if (par.isRTL(buf.params())) {
			left_pos = -1;
			right_pos = bidi.vis2log(row.pos());
		}
		else { // LTR paragraph
			right_pos = -1;
			left_pos = bidi.vis2log(row.endpos() - 1);
		}
		return;
	}
	
	// Whether 'known_pos' is to the left or to the right of the cursor depends
	// on whether it is an RTL or LTR character...
	bool const cur_is_RTL = 
		par.getFontSettings(buf.params(), known_pos).isVisibleRightToLeft();
	// ... in the following manner:
	// For an RTL character, "before" means "to the right" and "after" means
	// "to the left"; and for LTR, it's the reverse. So, 'known_pos' is to the
	// right of the cursor if (RTL && boundary) or (!RTL && !boundary):
	bool known_pos_on_right = (cur_is_RTL == boundary());

	// So we now know one of the positions surrounding the cursor. Let's 
	// determine the other one:
	
	if (known_pos_on_right) {
		right_pos = known_pos;
		// *visual* position of 'left_pos':
		pos_type v_left_pos = bidi.log2vis(right_pos) - 1;
		// If the position we just identified as 'left_pos' is a "skipped 
		// separator" (a separator which is at the logical end of a row,
		// except for the last row in a paragraph; such separators are not
		// painted, so they "are not really there"; note that in bidi text,
		// such a separator could appear visually in the middle of a row),
		// set 'left_pos' to the *next* position to the left.
		if (bidi.inRange(v_left_pos) 
				&& bidi.vis2log(v_left_pos) + 1 == row.endpos() 
				&& row.endpos() < lastpos()
				&& par.isSeparator(bidi.vis2log(v_left_pos))) {
			--v_left_pos;
		}
		// calculate the logical position of 'left_pos', if in row
		if (!bidi.inRange(v_left_pos))
			left_pos = -1;
		else
			left_pos = bidi.vis2log(v_left_pos);
		// If the position we identified as 'right_pos' is a "skipped 
		// separator", set 'right_pos' to the *next* position to the right.
		if (right_pos + 1 == row.endpos() && row.endpos() < lastpos() 
				&& par.isSeparator(right_pos)) {
			pos_type v_right_pos = bidi.log2vis(right_pos) + 1;
			if (!bidi.inRange(v_right_pos))
				right_pos = -1;
			else
				right_pos = bidi.vis2log(v_right_pos);
		}
	} 
	else { // known_pos is on the left
		left_pos = known_pos;
		// *visual* position of 'right_pos'
		pos_type v_right_pos = bidi.log2vis(left_pos) + 1;
		// If the position we just identified as 'right_pos' is a "skipped 
		// separator", set 'right_pos' to the *next* position to the right.
		if (bidi.inRange(v_right_pos) 
				&& bidi.vis2log(v_right_pos) + 1 == row.endpos() 
				&& row.endpos() < lastpos()
				&& par.isSeparator(bidi.vis2log(v_right_pos))) {
			++v_right_pos;
		}
		// calculate the logical position of 'right_pos', if in row
		if (!bidi.inRange(v_right_pos)) 
			right_pos = -1;
		else
			right_pos = bidi.vis2log(v_right_pos);
		// If the position we identified as 'left_pos' is a "skipped 
		// separator", set 'left_pos' to the *next* position to the left.
		if (left_pos + 1 == row.endpos() && row.endpos() < lastpos() 
				&& par.isSeparator(left_pos)) {
			pos_type v_left_pos = bidi.log2vis(left_pos) - 1;
			if (!bidi.inRange(v_left_pos))
				left_pos = -1;
			else
				left_pos = bidi.vis2log(v_left_pos);
		}
	}
	return;
}


bool Cursor::posVisToNewRow(bool movingLeft)
{
	Paragraph const & par = paragraph();
	Buffer const & buf = *buffer();
	Row const & row = textRow();
	bool par_is_LTR = !par.isRTL(buf.params());

	// Inside a table, determining whether to move to the next or previous row
	// should be done based on the table's direction. 
	int s = depth() - 1;
	if (s >= 1 && (*this)[s].inset().asInsetTabular()) {
		par_is_LTR = !(*this)[s].inset().asInsetTabular()->isRightToLeft(*this);
		LYXERR(Debug::RTL, "Inside table! par_is_LTR=" << (par_is_LTR ? 1 : 0));
	}
	
	// if moving left in an LTR paragraph or moving right in an RTL one, 
	// move to previous row
	if (par_is_LTR == movingLeft) {
		if (row.pos() == 0) { // we're at first row in paragraph
			if (pit() == 0) // no previous paragraph! don't move
				return false;
			// move to last pos in previous par
			--pit();
			pos() = lastpos();
			boundary(false);
		} else { // move to previous row in this par
			pos() = row.pos() - 1; // this is guaranteed to be in previous row
			boundary(false);
		}
	}
	// if moving left in an RTL paragraph or moving right in an LTR one, 
	// move to next row
	else {
		if (row.endpos() == lastpos()) { // we're at last row in paragraph
			if (pit() == lastpit()) // last paragraph! don't move
				return false;
			// move to first row in next par
			++pit();
			pos() = 0;
			boundary(false);
		} else { // move to next row in this par
			pos() = row.endpos();
			boundary(false);
		}
	}
	
	// make sure we're at left-/right-most pos in new row
	posVisToRowExtremity(!movingLeft);

	return true;
}


void Cursor::posVisToRowExtremity(bool left)  
{
	// prepare bidi tables
	Paragraph const & par = paragraph();
	Buffer const & buf = *buffer();
	Row const & row = textRow();
	Bidi bidi;
	bidi.computeTables(par, buf, row);

	LYXERR(Debug::RTL, "entering extremity: " << pit() << "," << pos() << ","
		<< (boundary() ? 1 : 0));

	if (left) { // move to leftmost position
		// if this is an RTL paragraph, and we're at the last row in the
		// paragraph, move to lastpos
		if (par.isRTL(buf.params()) && row.endpos() == lastpos())
			pos() = lastpos();
		else {
			pos() = bidi.vis2log(row.pos());

			// Moving to the leftmost position in the row, the cursor should
			// normally be placed to the *left* of the leftmost position.
			// A very common exception, though, is if the leftmost character 
			// also happens to be the separator at the (logical) end of the row
			// --- in this case, the separator is positioned beyond the left 
			// margin, and we don't want to move the cursor there (moving to 
			// the left of the separator is equivalent to moving to the next
			// line). So, in this case we actually want to place the cursor 
			// to the *right* of the leftmost position (the separator). 
			// Another exception is if we're moving to the logically last 
			// position in the row, which is *not* a separator: this means
			// that the entire row has no separators (if there were any, the 
			// row would have been broken there); and therefore in this case
			// we also move to the *right* of the last position (this indicates
			// to the user that there is no space after this position, and is 
			// consistent with the behavior in the middle of a row --- moving
			// right or left moves to the next/previous character; if we were
			// to move to the *left* of this position, that would simulate 
			// a separator which is not really there!). 
			// Finally, there is an exception to the previous exception: if 
			// this non-separator-but-last-position-in-row is an inset, then
			// we *do* want to stay to the left of it anyway: this is the 
			// "boundary" which we simulate at insets.
			
			bool right_of_pos = false; // do we want to be to the right of pos?

			// as explained above, if at last pos in row, stay to the right
			if ((pos() == row.endpos() - 1) && !par.isInset(pos()))
				right_of_pos = true;

			// Now we know if we want to be to the left or to the right of pos,
			// let's make sure we are where we want to be.
			bool new_pos_is_RTL = 
				par.getFontSettings(buf.params(), pos()).isVisibleRightToLeft();

			if (new_pos_is_RTL == !right_of_pos) {
				++pos();
				boundary(true);
			}
			
		}
	}
	else { // move to rightmost position
		// if this is an LTR paragraph, and we're at the last row in the
		// paragraph, move to lastpos
		if (!par.isRTL(buf.params()) && row.endpos() == lastpos())
			pos() = lastpos();
		else {
			pos() = bidi.vis2log(row.endpos() - 1);

			// Moving to the rightmost position in the row, the cursor should
			// normally be placed to the *right* of the rightmost position.
			// A very common exception, though, is if the rightmost character 
			// also happens to be the separator at the (logical) end of the row
			// --- in this case, the separator is positioned beyond the right 
			// margin, and we don't want to move the cursor there (moving to 
			// the right of the separator is equivalent to moving to the next
			// line). So, in this case we actually want to place the cursor 
			// to the *left* of the rightmost position (the separator). 
			// Another exception is if we're moving to the logically last 
			// position in the row, which is *not* a separator: this means
			// that the entire row has no separators (if there were any, the 
			// row would have been broken there); and therefore in this case
			// we also move to the *left* of the last position (this indicates
			// to the user that there is no space after this position, and is 
			// consistent with the behavior in the middle of a row --- moving
			// right or left moves to the next/previous character; if we were
			// to move to the *right* of this position, that would simulate 
			// a separator which is not really there!). 
			// Finally, there is an exception to the previous exception: if 
			// this non-separator-but-last-position-in-row is an inset, then
			// we *do* want to stay to the right of it anyway: this is the 
			// "boundary" which we simulate at insets.
			
			bool left_of_pos = false; // do we want to be to the left of pos?

			// as explained above, if at last pos in row, stay to the left
			if ((pos() == row.endpos() - 1) && !par.isInset(pos()))
				left_of_pos = true;

			// Now we know if we want to be to the left or to the right of pos,
			// let's make sure we are where we want to be.
			bool new_pos_is_RTL = 
				par.getFontSettings(buf.params(), pos()).isVisibleRightToLeft();

			if (new_pos_is_RTL == left_of_pos) {
				++pos();
				boundary(true);
			}
		}
	}
	LYXERR(Debug::RTL, "leaving extremity: " << pit() << "," << pos() << ","
		<< (boundary() ? 1 : 0));
}


CursorSlice Cursor::anchor() const
{
	LASSERT(anchor_.depth() >= depth(), /**/);
	CursorSlice normal = anchor_[depth() - 1];
	if (depth() < anchor_.depth() && top() <= normal) {
		// anchor is behind cursor -> move anchor behind the inset
		++normal.pos();
	}
	return normal;
}


CursorSlice Cursor::selBegin() const
{
	if (!selection())
		return top();
	return anchor() < top() ? anchor() : top();
}


CursorSlice Cursor::selEnd() const
{
	if (!selection())
		return top();
	return anchor() > top() ? anchor() : top();
}


DocIterator Cursor::selectionBegin() const
{
	if (!selection())
		return *this;

	DocIterator di;
	// FIXME: This is a work-around for the problem that
	// CursorSlice doesn't keep track of the boundary.
	if (anchor() == top())
		di = anchor_.boundary() > boundary() ? anchor_ : *this;
	else
		di = anchor() < top() ? anchor_ : *this;
	di.resize(depth());
	return di;
}


DocIterator Cursor::selectionEnd() const
{
	if (!selection())
		return *this;

	DocIterator di;
	// FIXME: This is a work-around for the problem that
	// CursorSlice doesn't keep track of the boundary.
	if (anchor() == top())
		di = anchor_.boundary() < boundary() ? anchor_ : *this;
	else
		di = anchor() > top() ? anchor_ : *this;

	if (di.depth() > depth()) {
		di.resize(depth());
		++di.pos();
	}
	return di;
}


void Cursor::setSelection()
{
	setSelection(true);
	// A selection with no contents is not a selection
	// FIXME: doesnt look ok
	if (idx() == anchor().idx() && 
	    pit() == anchor().pit() && 
	    pos() == anchor().pos())
		setSelection(false);
}


void Cursor::setSelection(DocIterator const & where, int n)
{
	setCursor(where);
	setSelection(true);
	anchor_ = where;
	pos() += n;
}


void Cursor::clearSelection()
{
	setSelection(false);
	setMark(false);
	resetAnchor();
}


void Cursor::setTargetX(int x)
{
	x_target_ = x;
	textTargetOffset_ = 0;
}


int Cursor::x_target() const
{
	return x_target_;
}


void Cursor::clearTargetX()
{
	x_target_ = -1;
	textTargetOffset_ = 0;
}


void Cursor::updateTextTargetOffset()
{
	int x;
	int y;
	getPos(x, y);
	textTargetOffset_ = x - x_target_;
}


void Cursor::info(odocstream & os) const
{
	for (int i = 1, n = depth(); i < n; ++i) {
		operator[](i).inset().infoize(os);
		os << "  ";
	}
	if (pos() != 0) {
		Inset const * inset = prevInset();
		// prevInset() can return 0 in certain case.
		if (inset)
			prevInset()->infoize2(os);
	}
	// overwite old message
	os << "                    ";
}


bool Cursor::selHandle(bool sel)
{
	//lyxerr << "Cursor::selHandle" << endl;
	if (mark())
		sel = true;
	if (sel == selection())
		return false;

	if (!sel)
		cap::saveSelection(*this);

	resetAnchor();
	setSelection(sel);
	return true;
}


ostream & operator<<(ostream & os, Cursor const & cur)
{
	os << "\n cursor:                                | anchor:\n";
	for (size_t i = 0, n = cur.depth(); i != n; ++i) {
		os << " " << cur[i] << " | ";
		if (i < cur.anchor_.depth())
			os << cur.anchor_[i];
		else
			os << "-------------------------------";
		os << "\n";
	}
	for (size_t i = cur.depth(), n = cur.anchor_.depth(); i < n; ++i) {
		os << "------------------------------- | " << cur.anchor_[i] << "\n";
	}
	os << " selection: " << cur.selection_
	   << " x_target: " << cur.x_target_ << endl;
	return os;
}


LyXErr & operator<<(LyXErr & os, Cursor const & cur)
{
	os.stream() << cur;
	return os;
}


} // namespace lyx


///////////////////////////////////////////////////////////////////
//
// FIXME: Look here
// The part below is the non-integrated rest of the original math
// cursor. This should be either generalized for texted or moved
// back to mathed (in most cases to InsetMathNest).
//
///////////////////////////////////////////////////////////////////

#include "mathed/InsetMathChar.h"
#include "mathed/InsetMathGrid.h"
#include "mathed/InsetMathScript.h"
#include "mathed/InsetMathUnknown.h"
#include "mathed/MathFactory.h"
#include "mathed/MathStream.h"
#include "mathed/MathSupport.h"


namespace lyx {

//#define FILEDEBUG 1


bool Cursor::isInside(Inset const * p) const
{
	for (size_t i = 0; i != depth(); ++i)
		if (&operator[](i).inset() == p)
			return true;
	return false;
}


void Cursor::leaveInset(Inset const & inset)
{
	for (size_t i = 0; i != depth(); ++i) {
		if (&operator[](i).inset() == &inset) {
			resize(i);
			return;
		}
	}
}


bool Cursor::openable(MathAtom const & t) const
{
	if (!t->isActive())
		return false;

	if (t->lock())
		return false;

	if (!selection())
		return true;

	// we can't move into anything new during selection
	if (depth() >= anchor_.depth())
		return false;
	if (t.nucleus() != &anchor_[depth()].inset())
		return false;

	return true;
}


void Cursor::setScreenPos(int x, int /*y*/)
{
	setTargetX(x);
	//bruteFind(*this, x, y, 0, bv().workWidth(), 0, bv().workHeight());
}



void Cursor::plainErase()
{
	cell().erase(pos());
}


void Cursor::markInsert()
{
	insert(char_type(0));
}


void Cursor::markErase()
{
	cell().erase(pos());
}


void Cursor::plainInsert(MathAtom const & t)
{
	cell().insert(pos(), t);
	++pos();
	inset().setBuffer(bv_->buffer());
	inset().initView();
}


void Cursor::insert(docstring const & str)
{
	for_each(str.begin(), str.end(),
		 boost::bind(static_cast<void(Cursor::*)(char_type)>
			     (&Cursor::insert), this, _1));
}


void Cursor::insert(char_type c)
{
	//lyxerr << "Cursor::insert char '" << c << "'" << endl;
	LASSERT(!empty(), /**/);
	if (inMathed()) {
		cap::selClearOrDel(*this);
		insert(new InsetMathChar(c));
	} else {
		text()->insertChar(*this, c);
	}
}


void Cursor::insert(MathAtom const & t)
{
	//lyxerr << "Cursor::insert MathAtom '" << t << "'" << endl;
	macroModeClose();
	cap::selClearOrDel(*this);
	plainInsert(t);
}


void Cursor::insert(Inset * inset0)
{
	LASSERT(inset0, /**/);
	if (inMathed())
		insert(MathAtom(inset0));
	else {
		text()->insertInset(*this, inset0);
		inset0->setBuffer(bv_->buffer());
		inset0->initView();
	}
}


void Cursor::niceInsert(docstring const & t, Parse::flags f)
{
	MathData ar;
	asArray(t, ar, f);
	if (ar.size() == 1)
		niceInsert(ar[0]);
	else
		insert(ar);
}


void Cursor::niceInsert(MathAtom const & t)
{
	macroModeClose();
	docstring const safe = cap::grabAndEraseSelection(*this);
	plainInsert(t);
	// enter the new inset and move the contents of the selection if possible
	if (t->isActive()) {
		posBackward();
		// be careful here: don't use 'pushBackward(t)' as this we need to
		// push the clone, not the original
		pushBackward(*nextInset());
		// We may not use niceInsert here (recursion)
		MathData ar;
		asArray(safe, ar);
		insert(ar);
	}
}


void Cursor::insert(MathData const & ar)
{
	macroModeClose();
	if (selection())
		cap::eraseSelection(*this);
	cell().insert(pos(), ar);
	pos() += ar.size();
}


bool Cursor::backspace()
{
	autocorrect() = false;

	if (selection()) {
		cap::eraseSelection(*this);
		return true;
	}

	if (pos() == 0) {
		// If empty cell, and not part of a big cell
		if (lastpos() == 0 && inset().nargs() == 1) {
			popBackward();
			// Directly delete empty cell: [|[]] => [|]
			if (inMathed()) {
				plainErase();
				resetAnchor();
				return true;
			}
			// [|], can not delete from inside
			return false;
		} else {
			if (inMathed())
				pullArg();
			else
				popBackward();
			return true;
		}
	}

	if (inMacroMode()) {
		InsetMathUnknown * p = activeMacro();
		if (p->name().size() > 1) {
			p->setName(p->name().substr(0, p->name().size() - 1));
			return true;
		}
	}

	if (pos() != 0 && prevAtom()->nargs() > 0) {
		// let's require two backspaces for 'big stuff' and
		// highlight on the first
		resetAnchor();
		setSelection(true);
		--pos();
	} else {
		--pos();
		plainErase();
	}
	return true;
}


bool Cursor::erase()
{
	autocorrect() = false;
	if (inMacroMode())
		return true;

	if (selection()) {
		cap::eraseSelection(*this);
		return true;
	}

	// delete empty cells if possible
	if (pos() == lastpos() && inset().idxDelete(idx()))
		return true;

	// special behaviour when in last position of cell
	if (pos() == lastpos()) {
		bool one_cell = inset().nargs() == 1;
		if (one_cell && lastpos() == 0) {
			popBackward();
			// Directly delete empty cell: [|[]] => [|]
			if (inMathed()) {
				plainErase();
				resetAnchor();
				return true;
			}
			// [|], can not delete from inside
			return false;
		}
		// remove markup
		if (!one_cell)
			inset().idxGlue(idx());
		return true;
	}

	// 'clever' UI hack: only erase large items if previously slected
	if (pos() != lastpos() && nextAtom()->nargs() > 0) {
		resetAnchor();
		setSelection(true);
		++pos();
	} else {
		plainErase();
	}

	return true;
}


bool Cursor::up()
{
	macroModeClose();
	DocIterator save = *this;
	FuncRequest cmd(selection() ? LFUN_UP_SELECT : LFUN_UP, docstring());
	this->dispatch(cmd);
	if (disp_.dispatched())
		return true;
	setCursor(save);
	autocorrect() = false;
	return false;
}


bool Cursor::down()
{
	macroModeClose();
	DocIterator save = *this;
	FuncRequest cmd(selection() ? LFUN_DOWN_SELECT : LFUN_DOWN, docstring());
	this->dispatch(cmd);
	if (disp_.dispatched())
		return true;
	setCursor(save);
	autocorrect() = false;
	return false;
}


bool Cursor::macroModeClose()
{
	if (!inMacroMode())
		return false;
	InsetMathUnknown * p = activeMacro();
	p->finalize();
	MathData selection;
	asArray(p->selection(), selection);
	docstring const s = p->name();
	--pos();
	cell().erase(pos());

	// do nothing if the macro name is empty
	if (s == "\\")
		return false;

	// trigger updates of macros, at least, if no full
	// updates take place anyway
	updateFlags(Update::Force);

	docstring const name = s.substr(1);
	InsetMathNest * const in = inset().asInsetMath()->asNestInset();
	if (in && in->interpretString(*this, s))
		return true;
	MathAtom atom = createInsetMath(name);

	// try to put argument into macro, if we just inserted a macro
	bool macroArg = false;
	MathMacro * atomAsMacro = atom.nucleus()->asMacro();
	if (atomAsMacro) {
		// macros here are still unfolded (in init mode in fact). So
		// we have to resolve the macro here manually and check its arity
		// to put the selection behind it if arity > 0.
		MacroData const * data = buffer()->getMacro(atomAsMacro->name());
		if (selection.size() > 0 && data && data->numargs() - data->optionals() > 0) {
			macroArg = true;
			atomAsMacro->setDisplayMode(MathMacro::DISPLAY_INTERACTIVE_INIT, 1);
		} else
			// non-greedy case. Do not touch the arguments behind
			atomAsMacro->setDisplayMode(MathMacro::DISPLAY_INTERACTIVE_INIT, 0);
	}

	// insert remembered selection into first argument of a non-macro
	else if (atom.nucleus()->nargs() > 0)
		atom.nucleus()->cell(0).append(selection);
	
	plainInsert(atom);

	// finally put the macro argument behind, if needed
	if (macroArg) {
		if (selection.size() > 1)
			plainInsert(MathAtom(new InsetMathBrace(selection)));
		else
			insert(selection);
	}
	
	return true;
}


docstring Cursor::macroName()
{
	return inMacroMode() ? activeMacro()->name() : docstring();
}


void Cursor::handleNest(MathAtom const & a, int c)
{
	//lyxerr << "Cursor::handleNest: " << c << endl;
	MathAtom t = a;
	asArray(cap::grabAndEraseSelection(*this), t.nucleus()->cell(c));
	insert(t);
	posBackward();
	pushBackward(*nextInset());
}


int Cursor::targetX() const
{
	if (x_target() != -1)
		return x_target();
	int x = 0;
	int y = 0;
	getPos(x, y);
	return x;
}


int Cursor::textTargetOffset() const
{
	return textTargetOffset_;
}


void Cursor::setTargetX()
{
	int x;
	int y;
	getPos(x, y);
	setTargetX(x);
}


bool Cursor::inMacroMode() const
{
	if (!inMathed())
		return false;
	if (pos() == 0)
		return false;
	InsetMathUnknown const * p = prevAtom()->asUnknownInset();
	return p && !p->final();
}


InsetMathUnknown * Cursor::activeMacro()
{
	return inMacroMode() ? prevAtom().nucleus()->asUnknownInset() : 0;
}


InsetMathUnknown const * Cursor::activeMacro() const
{
	return inMacroMode() ? prevAtom().nucleus()->asUnknownInset() : 0;
}


void Cursor::pullArg()
{
	// FIXME: Look here
	MathData ar = cell();
	if (popBackward() && inMathed()) {
		plainErase();
		cell().insert(pos(), ar);
		resetAnchor();
	} else {
		//formula()->mutateToText();
	}
}


void Cursor::touch()
{
	// FIXME: look here
#if 0
	DocIterator::const_iterator it = begin();
	DocIterator::const_iterator et = end();
	for ( ; it != et; ++it)
		it->cell().touch();
#endif
}


void Cursor::normalize()
{
	if (idx() > lastidx()) {
		lyxerr << "this should not really happen - 1: "
		       << idx() << ' ' << nargs()
		       << " in: " << &inset() << endl;
		idx() = lastidx();
	}

	if (pos() > lastpos()) {
		lyxerr << "this should not really happen - 2: "
			<< pos() << ' ' << lastpos() <<  " in idx: " << idx()
		       << " in atom: '";
		odocstringstream os;
		WriteStream wi(os, false, true, false);
		inset().asInsetMath()->write(wi);
		lyxerr << to_utf8(os.str()) << endl;
		pos() = lastpos();
	}
}


bool Cursor::upDownInMath(bool up)
{
	// Be warned: The 'logic' implemented in this function is highly
	// fragile. A distance of one pixel or a '<' vs '<=' _really
	// matters. So fiddle around with it only if you think you know
	// what you are doing!
	int xo = 0;
	int yo = 0;
	getPos(xo, yo);
	xo = theLyXFunc().cursorBeforeDispatchX();
	
	// check if we had something else in mind, if not, this is the future
	// target
	if (x_target_ == -1)
		setTargetX(xo);
	else if (inset().asInsetText() && xo - textTargetOffset() != x_target()) {
		// In text mode inside the line (not left or right) possibly set a new target_x,
		// but only if we are somewhere else than the previous target-offset.
		
		// We want to keep the x-target on subsequent up/down movements
		// that cross beyond the end of short lines. Thus a special
		// handling when the cursor is at the end of line: Use the new
		// x-target only if the old one was before the end of line
		// or the old one was after the beginning of the line
		bool inRTL = isWithinRtlParagraph(*this);
		bool left;
		bool right;
		if (inRTL) {
			left = pos() == textRow().endpos();
			right = pos() == textRow().pos();
		} else {
			left = pos() == textRow().pos();
			right = pos() == textRow().endpos();
		}
		if ((!left && !right) ||
				(left && !right && xo < x_target_) ||
				(!left && right && x_target_ < xo))
			setTargetX(xo);
		else
			xo = targetX();
	} else
		xo = targetX();

	// try neigbouring script insets
	Cursor old = *this;
	if (inMathed() && !selection()) {
		// try left
		if (pos() != 0) {
			InsetMathScript const * p = prevAtom()->asScriptInset();
			if (p && p->has(up)) {
				--pos();
				push(*const_cast<InsetMathScript*>(p));
				idx() = p->idxOfScript(up);
				pos() = lastpos();
				
				// we went in the right direction? Otherwise don't jump into the script
				int x;
				int y;
				getPos(x, y);
				int oy = theLyXFunc().cursorBeforeDispatchY();
				if ((!up && y <= oy) ||
						(up && y >= oy))
					operator=(old);
				else
					return true;
			}
		}
		
		// try right
		if (pos() != lastpos()) {
			InsetMathScript const * p = nextAtom()->asScriptInset();
			if (p && p->has(up)) {
				push(*const_cast<InsetMathScript*>(p));
				idx() = p->idxOfScript(up);
				pos() = 0;
				
				// we went in the right direction? Otherwise don't jump into the script
				int x;
				int y;
				getPos(x, y);
				int oy = theLyXFunc().cursorBeforeDispatchY();
				if ((!up && y <= oy) ||
						(up && y >= oy))
					operator=(old);
				else
					return true;
			}
		}
	}
		
	// try to find an inset that knows better then we,
	if (inset().idxUpDown(*this, up)) {
		//lyxerr << "idxUpDown triggered" << endl;
		// try to find best position within this inset
		if (!selection())
			setCursor(bruteFind2(*this, xo, yo));
		return true;
	}
	
	// any improvement going just out of inset?
	if (popBackward() && inMathed()) {
		//lyxerr << "updown: popBackward succeeded" << endl;
		int xnew;
		int ynew;
		int yold = theLyXFunc().cursorBeforeDispatchY();
		getPos(xnew, ynew);
		if (up ? ynew < yold : ynew > yold)
			return true;
	}
	
	// no success, we are probably at the document top or bottom
	operator=(old);
	return false;
}


bool Cursor::atFirstOrLastRow(bool up)
{
	TextMetrics const & tm = bv_->textMetrics(text());
	ParagraphMetrics const & pm = tm.parMetrics(pit());
	
	int row;
	if (pos() && boundary())
		row = pm.pos2row(pos() - 1);
	else
		row = pm.pos2row(pos());
	
	if (up) {
		if (pit() == 0 && row == 0)
			return true;
	} else {
		if (pit() + 1 >= int(text()->paragraphs().size()) &&
				row + 1 >= int(pm.rows().size()))
			return true;
	}
	return false;
}

bool Cursor::upDownInText(bool up, bool & updateNeeded)
{
	LASSERT(text(), /**/);

	// where are we?
	int xo = 0;
	int yo = 0;
	getPos(xo, yo);
	xo = theLyXFunc().cursorBeforeDispatchX();

	// update the targetX - this is here before the "return false"
	// to set a new target which can be used by InsetTexts above
	// if we cannot move up/down inside this inset anymore
	if (x_target_ == -1)
		setTargetX(xo);
	else if (xo - textTargetOffset() != x_target() &&
					 depth() == beforeDispatchCursor_.depth()) {
		// In text mode inside the line (not left or right) possibly set a new target_x,
		// but only if we are somewhere else than the previous target-offset.
		
		// We want to keep the x-target on subsequent up/down movements
		// that cross beyond the end of short lines. Thus a special
		// handling when the cursor is at the end of line: Use the new
		// x-target only if the old one was before the end of line
		// or the old one was after the beginning of the line
		bool inRTL = isWithinRtlParagraph(*this);
		bool left;
		bool right;
		if (inRTL) {
			left = pos() == textRow().endpos();
			right = pos() == textRow().pos();
		} else {
			left = pos() == textRow().pos();
			right = pos() == textRow().endpos();
		}
		if ((!left && !right) ||
				(left && !right && xo < x_target_) ||
				(!left && right && x_target_ < xo))
			setTargetX(xo);
		else
			xo = targetX();
	} else
		xo = targetX();
		
	// first get the current line
	TextMetrics & tm = bv_->textMetrics(text());
	ParagraphMetrics const & pm = tm.parMetrics(pit());
	int row;
	if (pos() && boundary())
		row = pm.pos2row(pos() - 1);
	else
		row = pm.pos2row(pos());
		
	if (atFirstOrLastRow(up))
		return false;

	// with and without selection are handled differently
	if (!selection()) {
		int yo = bv().getPos(*this, boundary()).y_;
		Cursor old = *this;
		// To next/previous row
		if (up)
			tm.editXY(*this, xo, yo - textRow().ascent() - 1);
		else
			tm.editXY(*this, xo, yo + textRow().descent() + 1);
		clearSelection();
		
		// This happens when you move out of an inset.
		// And to give the DEPM the possibility of doing
		// something we must provide it with two different
		// cursors. (Lgb)
		Cursor dummy = *this;
		if (dummy == old)
			++dummy.pos();
		if (bv().checkDepm(dummy, old)) {
			updateNeeded = true;
			// Make sure that cur gets back whatever happened to dummy(Lgb)
			operator=(dummy);
		}
	} else {
		// if there is a selection, we stay out of any inset, and just jump to the right position:
		Cursor old = *this;
		if (up) {
			if (row > 0) {
				top().pos() = min(tm.x2pos(pit(), row - 1, xo), top().lastpos());
			} else if (pit() > 0) {
				--pit();
				TextMetrics & tm = bv_->textMetrics(text());
				if (!tm.contains(pit()))
					tm.newParMetricsUp();
				ParagraphMetrics const & pmcur = tm.parMetrics(pit());
				top().pos() = min(tm.x2pos(pit(), pmcur.rows().size() - 1, xo), top().lastpos());
			}
		} else {
			if (row + 1 < int(pm.rows().size())) {
				top().pos() = min(tm.x2pos(pit(), row + 1, xo), top().lastpos());
			} else if (pit() + 1 < int(text()->paragraphs().size())) {
				++pit();
				TextMetrics & tm = bv_->textMetrics(text());
				if (!tm.contains(pit()))
					tm.newParMetricsDown();
				top().pos() = min(tm.x2pos(pit(), 0, xo), top().lastpos());
			}
		}

		updateNeeded |= bv().checkDepm(*this, old);
	}

	updateTextTargetOffset();
	return true;
}	


void Cursor::handleFont(string const & font)
{
	LYXERR(Debug::DEBUG, font);
	docstring safe;
	if (selection()) {
		macroModeClose();
		safe = cap::grabAndEraseSelection(*this);
	}

	recordUndoInset();

	if (lastpos() != 0) {
		// something left in the cell
		if (pos() == 0) {
			// cursor in first position
			popBackward();
		} else if (pos() == lastpos()) {
			// cursor in last position
			popForward();
		} else {
			// cursor in between. split cell
			MathData::iterator bt = cell().begin();
			MathAtom at = createInsetMath(from_utf8(font));
			at.nucleus()->cell(0) = MathData(bt, bt + pos());
			cell().erase(bt, bt + pos());
			popBackward();
			plainInsert(at);
		}
	} else {
		// nothing left in the cell
		popBackward();
		plainErase();
		resetAnchor();
	}
	insert(safe);
}


void Cursor::message(docstring const & msg) const
{
	theLyXFunc().setMessage(msg);
}


void Cursor::errorMessage(docstring const & msg) const
{
	theLyXFunc().setErrorMessage(msg);
}


docstring Cursor::selectionAsString(bool with_label) const
{
	if (!selection())
		return docstring();

	int const label = with_label
		? AS_STR_LABEL | AS_STR_INSETS : AS_STR_INSETS;

	if (inTexted()) {
		idx_type const startidx = selBegin().idx();
		idx_type const endidx = selEnd().idx();
		if (startidx != endidx) {
			// multicell selection
			InsetTabular * table = inset().asInsetTabular();
			LASSERT(table, return docstring());
			return table->asString(startidx, endidx);
		}
		
		ParagraphList const & pars = text()->paragraphs();

		pit_type const startpit = selBegin().pit();
		pit_type const endpit = selEnd().pit();
		size_t const startpos = selBegin().pos();
		size_t const endpos = selEnd().pos();

		if (startpit == endpit)
			return pars[startpit].asString(startpos, endpos, label);

		// First paragraph in selection
		docstring result = pars[startpit].
			asString(startpos, pars[startpit].size(), label)
				 + parbreak(pars[startpit]);

		// The paragraphs in between (if any)
		for (pit_type pit = startpit + 1; pit != endpit; ++pit) {
			Paragraph const & par = pars[pit];
			result += par.asString(0, par.size(), label)
				  + parbreak(pars[pit]);
		}

		// Last paragraph in selection
		result += pars[endpit].asString(0, endpos, label);

		return result;
	}

	if (inMathed())
		return cap::grabSelection(*this);

	return docstring();
}


docstring Cursor::currentState() const
{
	if (inMathed()) {
		odocstringstream os;
		info(os);
		return os.str();
	}

	if (inTexted())
		return text()->currentState(*this);

	return docstring();
}


docstring Cursor::getPossibleLabel() const
{
	return inMathed() ? from_ascii("eq:") : text()->getPossibleLabel(*this);
}


Encoding const * Cursor::getEncoding() const
{
	if (empty())
		return 0;
	CursorSlice const & sl = innerTextSlice();
	Text const & text = *sl.text();
	Font font = text.getPar(sl.pit()).getFont(
		bv().buffer().params(), sl.pos(), outerFont(sl.pit(), text.paragraphs()));
	return font.language()->encoding();
}


void Cursor::undispatched()
{
	disp_.dispatched(false);
}


void Cursor::dispatched()
{
	disp_.dispatched(true);
}


void Cursor::updateFlags(Update::flags f)
{
	disp_.update(f);
}


void Cursor::noUpdate()
{
	disp_.update(Update::None);
}


Font Cursor::getFont() const
{
	// The logic here should more or less match to the Cursor::setCurrentFont
	// logic, i.e. the cursor height should give a hint what will happen
	// if a character is entered.
	
	// HACK. far from being perfect...

	CursorSlice const & sl = innerTextSlice();
	Text const & text = *sl.text();
	Paragraph const & par = text.getPar(sl.pit());
	
	// on boundary, so we are really at the character before
	pos_type pos = sl.pos();
	if (pos > 0 && boundary())
		--pos;
	
	// on space? Take the font before (only for RTL boundary stay)
	if (pos > 0) {
		TextMetrics const & tm = bv().textMetrics(&text);
		if (pos == sl.lastpos()
			|| (par.isSeparator(pos) 
			&& !tm.isRTLBoundary(sl.pit(), pos)))
			--pos;
	}
	
	// get font at the position
	Font font = par.getFont(buffer()->params(), pos,
		outerFont(sl.pit(), text.paragraphs()));

	return font;
}


bool Cursor::fixIfBroken()
{
	if (DocIterator::fixIfBroken()) {
			clearSelection();
			return true;
	}
	return false;
}


bool notifyCursorLeavesOrEnters(Cursor const & old, Cursor & cur)
{
	// find inset in common
	size_type i;
	for (i = 0; i < old.depth() && i < cur.depth(); ++i) {
		if (&old[i].inset() != &cur[i].inset())
			break;
	}

	// update words if we just moved to another paragraph
	if (i == old.depth() && i == cur.depth()
	    && !cur.buffer()->isClean()
	    && cur.inTexted() && old.inTexted()
	    && cur.pit() != old.pit()) {
		old.paragraph().updateWords(old.top());
	}

	// notify everything on top of the common part in old cursor,
	// but stop if the inset claims the cursor to be invalid now
	for (size_type j = i; j < old.depth(); ++j) {
		Cursor insetPos = old;
		insetPos.cutOff(j);
		if (old[j].inset().notifyCursorLeaves(insetPos, cur))
			return true;
	}

	// notify everything on top of the common part in new cursor,
	// but stop if the inset claims the cursor to be invalid now
	for (; i < cur.depth(); ++i) {
		if (cur[i].inset().notifyCursorEnters(cur))
			return true;
	}
	
	return false;
}


void Cursor::setCurrentFont()
{
	CursorSlice const & cs = innerTextSlice();
	Paragraph const & par = cs.paragraph();
	pos_type cpit = cs.pit();
	pos_type cpos = cs.pos();
	Text const & ctext = *cs.text();
	TextMetrics const & tm = bv().textMetrics(&ctext);

	// are we behind previous char in fact? -> go to that char
	if (cpos > 0 && boundary())
		--cpos;

	// find position to take the font from
	if (cpos != 0) {
		// paragraph end? -> font of last char
		if (cpos == lastpos())
			--cpos;
		// on space? -> look at the words in front of space
		else if (cpos > 0 && par.isSeparator(cpos))	{
			// abc| def -> font of c
			// abc |[WERBEH], i.e. boundary==true -> font of c
			// abc [WERBEH]| def, font of the space
			if (!tm.isRTLBoundary(cpit, cpos))
				--cpos;
		}
	}

	// get font
	BufferParams const & bufparams = buffer()->params();
	current_font = par.getFontSettings(bufparams, cpos);
	real_current_font = tm.displayFont(cpit, cpos);

	// special case for paragraph end
	if (cs.pos() == lastpos()
	    && tm.isRTLBoundary(cpit, cs.pos())
	    && !boundary()) {
		Language const * lang = par.getParLanguage(bufparams);
		current_font.setLanguage(lang);
		current_font.fontInfo().setNumber(FONT_OFF);
		real_current_font.setLanguage(lang);
		real_current_font.fontInfo().setNumber(FONT_OFF);
	}
}


bool Cursor::textUndo()
{
	DocIterator dit = *this;
	// Undo::textUndo() will modify dit.
	if (!buffer()->undo().textUndo(dit))
		return false;
	// Set cursor
	setCursor(dit);
	clearSelection();
	fixIfBroken();
	return true;
}


bool Cursor::textRedo()
{
	DocIterator dit = *this;
	// Undo::textRedo() will modify dit.
	if (!buffer()->undo().textRedo(dit))
		return false;
	// Set cursor
	setCursor(dit);
	clearSelection();
	fixIfBroken();
	return true;
}


void Cursor::finishUndo() const
{
	buffer()->undo().finishUndo();
}


void Cursor::beginUndoGroup() const
{
	buffer()->undo().beginUndoGroup();
}


void Cursor::endUndoGroup() const
{
	buffer()->undo().endUndoGroup();
}


void Cursor::recordUndo(UndoKind kind, pit_type from, pit_type to) const
{
	buffer()->undo().recordUndo(*this, kind, from, to);
}


void Cursor::recordUndo(UndoKind kind, pit_type from) const
{
	buffer()->undo().recordUndo(*this, kind, from);
}


void Cursor::recordUndo(UndoKind kind) const
{
	buffer()->undo().recordUndo(*this, kind);
}


void Cursor::recordUndoInset(UndoKind kind) const
{
	buffer()->undo().recordUndoInset(*this, kind);
}


void Cursor::recordUndoFullDocument() const
{
	buffer()->undo().recordUndoFullDocument(*this);
}


void Cursor::recordUndoSelection() const
{
	if (inMathed()) {
		if (cap::multipleCellsSelected(*this))
			recordUndoInset();
		else
			recordUndo();
	} else {
		buffer()->undo().recordUndo(*this, ATOMIC_UNDO,
			selBegin().pit(), selEnd().pit());
	}
}


void Cursor::checkBufferStructure()
{
	Buffer const * master = buffer()->masterBuffer();
	master->tocBackend().updateItem(*this);
}


} // namespace lyx
