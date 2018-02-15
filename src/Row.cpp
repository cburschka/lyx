/**
 * \file Row.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 *
 * Metrics for an on-screen text row.
 */

#include <config.h>

#include "Row.h"

#include "DocIterator.h"

#include "frontends/FontMetrics.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"

#include <ostream>

using namespace std;

namespace lyx {

using support::rtrim;
using frontend::FontMetrics;


// Maximum length that a space can be stretched when justifying text
static double const MAX_SPACE_STRETCH = 1.5; //em


int Row::Element::countSeparators() const
{
	if (type != STRING)
		return 0;
	return count(str.begin(), str.end(), ' ');
}


int Row::Element::countExpanders() const
{
	if (type != STRING)
		return 0;
	return theFontMetrics(font).countExpanders(str);
}


int Row::Element::expansionAmount() const
{
	if (type != STRING)
		return 0;
	return countExpanders() * theFontMetrics(font).em();
}


void Row::Element::setExtra(double extra_per_em)
{
	if (type != STRING)
		return;
	extra = extra_per_em * theFontMetrics(font).em();
}


double Row::Element::pos2x(pos_type const i) const
{
	// This can happen with inline completion when clicking on the
	// row after the completion.
	if (i < pos || i > endpos)
		return 0;

	double w = 0;
	//handle first the two bounds of the element
	if (i == endpos && type != VIRTUAL)
		w = isRTL() ? 0 : full_width();
	else if (i == pos || type != STRING)
		w = isRTL() ? full_width() : 0;
	else {
		FontMetrics const & fm = theFontMetrics(font);
		w = fm.pos2x(str, i - pos, isRTL(), extra);
	}

	return w;
}


pos_type Row::Element::x2pos(int &x) const
{
	//lyxerr << "x2pos: x=" << x << " w=" << width() << " " << *this;
	size_t i = 0;

	switch (type) {
	case STRING: {
		FontMetrics const & fm = theFontMetrics(font);
		i = fm.x2pos(str, x, isRTL(), extra);
		break;
	}
	case VIRTUAL:
		// those elements are actually empty (but they have a width)
		i = 0;
		x = isRTL() ? int(full_width()) : 0;
		break;
	case INSET:
	case SPACE:
		// those elements contain only one position. Round to
		// the closest side.
		if (x > (full_width() + 1) / 2) {
			x = int(full_width());
			i = !isRTL();
		} else {
			x = 0;
			i = isRTL();
		}
	}
	//lyxerr << "=> p=" << pos + i << " x=" << x << endl;
	return pos + i;
}


bool Row::Element::breakAt(int w, bool force)
{
	if (type != STRING || dim.wid <= w)
		return false;

	FontMetrics const & fm = theFontMetrics(font);
	int x = w;
	if(fm.breakAt(str, x, isRTL(), force)) {
		dim.wid = x;
		endpos = pos + str.length();
		//lyxerr << "breakAt(" << w << ")  Row element Broken at " << x << "(w(str)=" << fm.width(str) << "): e=" << *this << endl;
		return true;
	}

	return false;
}


pos_type Row::Element::left_pos() const
{
	return isRTL() ? endpos : pos;
}


pos_type Row::Element::right_pos() const
{
	return isRTL() ? pos : endpos;
}


Row::Row()
	: separator(0), label_hfill(0), left_margin(0), right_margin(0),
	  sel_beg(-1), sel_end(-1),
	  begin_margin_sel(false), end_margin_sel(false),
	  changed_(true),
	  pit_(0), pos_(0), end_(0),
	  right_boundary_(false), flushed_(false), rtl_(false),
	  changebar_(false)
{}


bool Row::isMarginSelected(bool left_margin, DocIterator const & beg,
		DocIterator const & end) const
{
	pos_type const sel_pos = left_margin ? sel_beg : sel_end;
	pos_type const margin_pos = left_margin ? pos_ : end_;

	// Is the chosen margin selected ?
	if (sel_pos == margin_pos) {
		if (beg.pos() == end.pos())
			// This is a special case in which the space between after
			// pos i-1 and before pos i is selected, i.e. the margins
			// (see DocIterator::boundary_).
			return beg.boundary() && !end.boundary();
		else if (end.pos() == margin_pos)
			// If the selection ends around the margin, it is only
			// drawn if the cursor is after the margin.
			return !end.boundary();
		else if (beg.pos() == margin_pos)
			// If the selection begins around the margin, it is
			// only drawn if the cursor is before the margin.
			return beg.boundary();
		else
			return true;
	}
	return false;
}


void Row::setSelectionAndMargins(DocIterator const & beg,
		DocIterator const & end) const
{
	setSelection(beg.pos(), end.pos());

	if (selection()) {
		change(end_margin_sel, isMarginSelected(false, beg, end));
		change(begin_margin_sel, isMarginSelected(true, beg, end));
	}
}


void Row::setSelection(pos_type beg, pos_type end) const
{
	if (pos_ >= beg && pos_ <= end)
		change(sel_beg, pos_);
	else if (beg > pos_ && beg <= end_)
		change(sel_beg, beg);
	else
		change(sel_beg, -1);

	if (end_ >= beg && end_ <= end)
		change(sel_end,end_);
	else if (end < end_ && end >= pos_)
		change(sel_end, end);
	else
		change(sel_end, -1);
}


bool Row::selection() const
{
	return sel_beg != -1 && sel_end != -1;
}


ostream & operator<<(ostream & os, Row::Element const & e)
{
	if (e.isRTL())
		os << e.endpos << "<<" << e.pos << " ";
	else
		os << e.pos << ">>" << e.endpos << " ";

	switch (e.type) {
	case Row::STRING:
		os << "STRING: `" << to_utf8(e.str) << "' ("
		   << e.countExpanders() << " expanders.), ";
		break;
	case Row::VIRTUAL:
		os << "VIRTUAL: `" << to_utf8(e.str) << "', ";
		break;
	case Row::INSET:
		os << "INSET: " << to_utf8(e.inset->layoutName()) << ", ";
		break;
	case Row::SPACE:
		os << "SPACE: ";
		break;
	}
	os << "width=" << e.full_width();
	return os;
}


ostream & operator<<(ostream & os, Row const & row)
{
	os << " pos: " << row.pos_ << " end: " << row.end_
	   << " left_margin: " << row.left_margin
	   << " width: " << row.dim_.wid
	   << " right_margin: " << row.right_margin
	   << " ascent: " << row.dim_.asc
	   << " descent: " << row.dim_.des
	   << " separator: " << row.separator
	   << " label_hfill: " << row.label_hfill
	   << " row_boundary: " << row.right_boundary() << "\n";
	double x = row.left_margin;
	Row::Elements::const_iterator it = row.elements_.begin();
	for ( ; it != row.elements_.end() ; ++it) {
		os << "x=" << x << " => " << *it << endl;
		x += it->full_width();
	}
	return os;
}


int Row::left_x() const
{
	double x = left_margin;
	const_iterator const end = elements_.end();
	const_iterator cit = elements_.begin();
	while (cit != end && cit->isVirtual()) {
		x += cit->full_width();
		++cit;
	}
	return int(x + 0.5);
}


int Row::right_x() const
{
	double x = dim_.wid;
	const_iterator const begin = elements_.begin();
	const_iterator cit = elements_.end();
	while (cit != begin) {
		--cit;
		if (cit->isVirtual())
			x -= cit->full_width();
		else
			break;
	}
	return int(x + 0.5);
}


int Row::countSeparators() const
{
	int n = 0;
	const_iterator const end = elements_.end();
	for (const_iterator cit = elements_.begin() ; cit != end ; ++cit)
		n += cit->countSeparators();
	return n;
}


bool Row::setExtraWidth(int w)
{
	if (w < 0)
		// this is not expected to happen (but it does)
		return false;
	// amount of expansion: number of expanders time the em value for each
	// string element
	int exp_amount = 0;
	for (Row::Element const & e : elements_)
		exp_amount += e.expansionAmount();
	if (!exp_amount)
		return false;
	// extra length per expander per em
	double extra_per_em = double(w) / exp_amount;
	if (extra_per_em > MAX_SPACE_STRETCH)
		// do not stretch more than MAX_SPACE_STRETCH em per expander
		return false;
	// add extra length to each element proportionally to its em.
	for (Row::Element & e : elements_)
		if (e.type == Row::STRING)
			e.setExtra(extra_per_em);
	// update row dimension
	dim_.wid += w;
	return true;
}


bool Row::sameString(Font const & f, Change const & ch) const
{
	if (elements_.empty())
		return false;
	Element const & elt = elements_.back();
	return elt.type == STRING && !elt.final
		   && elt.font == f && elt.change == ch;
}


void Row::finalizeLast()
{
	if (elements_.empty())
		return;
	Element & elt = elements_.back();
	if (elt.final)
		return;
	elt.final = true;
	if (elt.change.changed())
		changebar_ = true;

	if (elt.type == STRING) {
		dim_.wid -= elt.dim.wid;
		elt.dim.wid = theFontMetrics(elt.font).width(elt.str);
		dim_.wid += elt.dim.wid;
	}
}


void Row::add(pos_type const pos, Inset const * ins, Dimension const & dim,
	      Font const & f, Change const & ch)
{
	finalizeLast();
	Element e(INSET, pos, f, ch);
	e.inset = ins;
	e.dim = dim;
	elements_.push_back(e);
	dim_.wid += dim.wid;
}


void Row::add(pos_type const pos, char_type const c,
	      Font const & f, Change const & ch)
{
	if (!sameString(f, ch)) {
		finalizeLast();
		Element e(STRING, pos, f, ch);
		elements_.push_back(e);
	}
	if (back().str.length() % 30 == 0) {
		dim_.wid -= back().dim.wid;
		back().str += c;
		back().endpos = pos + 1;
		back().dim.wid = theFontMetrics(back().font).width(back().str);
		dim_.wid += back().dim.wid;
	} else {
		back().str += c;
		back().endpos = pos + 1;
	}
}


void Row::addVirtual(pos_type const pos, docstring const & s,
		     Font const & f, Change const & ch)
{
	finalizeLast();
	Element e(VIRTUAL, pos, f, ch);
	e.str = s;
	e.dim.wid = theFontMetrics(f).width(s);
	dim_.wid += e.dim.wid;
	e.endpos = pos;
	elements_.push_back(e);
	finalizeLast();
}


void Row::addSpace(pos_type const pos, int const width,
		   Font const & f, Change const & ch)
{
	finalizeLast();
	Element e(SPACE, pos, f, ch);
	e.dim.wid = width;
	elements_.push_back(e);
	dim_.wid += e.dim.wid;
}


void Row::pop_back()
{
	dim_.wid -= elements_.back().dim.wid;
	elements_.pop_back();
}


bool Row::shortenIfNeeded(pos_type const keep, int const w, int const next_width)
{
	if (empty() || width() <= w)
		return false;

	Elements::iterator const beg = elements_.begin();
	Elements::iterator const end = elements_.end();
	int wid = left_margin;

	// Search for the first element that goes beyond right margin
	Elements::iterator cit = beg;
	for ( ; cit != end ; ++cit) {
		if (wid + cit->dim.wid > w)
			break;
		wid += cit->dim.wid;
	}

	if (cit == end) {
		// This should not happen since the row is too long.
		LYXERR0("Something is wrong cannot shorten row: " << *this);
		return false;
	}

	// Iterate backwards over breakable elements and try to break them
	Elements::iterator cit_brk = cit;
	int wid_brk = wid + cit_brk->dim.wid;
	++cit_brk;
	while (cit_brk != beg) {
		--cit_brk;
		// make a copy of the element to work on it.
		Element brk = *cit_brk;
		wid_brk -= brk.dim.wid;
		if (brk.countSeparators() == 0 || brk.pos < keep)
			continue;
		/* We have found a suitable separable element. This is the common case.
		 * Try to break it cleanly (at word boundary) at a length that is both
		 * - less than the available space on the row
		 * - shorter than the natural width of the element, in order to enforce
		 *   break-up.
		 */
		if (brk.breakAt(min(w - wid_brk, brk.dim.wid - 2), false)) {
			/* if this element originally did not cause a row overflow
			 * in itself, and the remainder of the row would still be
			 * too large after breaking, then we will have issues in
			 * next row. Thus breaking does not help.
			 */
			if (wid_brk + cit_brk->dim.wid < w
			    && dim_.wid - (wid_brk + brk.dim.wid) >= next_width) {
				break;
			}
			end_ = brk.endpos;
			/* after breakAt, there may be spaces at the end of the
			 * string, but they are not counted in the string length
			 * (QTextLayout feature, actually). We remove them, but do
			 * not change the end of the row, since spaces at row
			 * break are invisible.
			 */
			brk.str = rtrim(brk.str);
			brk.endpos = brk.pos + brk.str.length();
			*cit_brk = brk;
			dim_.wid = wid_brk + brk.dim.wid;
			// If there are other elements, they should be removed.
			elements_.erase(cit_brk + 1, end);
			return true;
		}
	}

	if (cit != beg && cit->type == VIRTUAL) {
		// It is not possible to separate a virtual element from the
		// previous one.
		--cit;
		wid -= cit->dim.wid;
	}

	if (cit != beg) {
		// There is no usable separator, but several elements have
		// been added. We can cut right here.
		end_ = cit->pos;
		dim_.wid = wid;
		elements_.erase(cit, end);
		return true;
	}

	/* If we are here, it means that we have not found a separator to
	 * shorten the row. Let's try to break it again, but not at word
	 * boundary this time.
	 */
	if (cit->breakAt(w - wid, true)) {
		end_ = cit->endpos;
		// See comment above.
		cit->str = rtrim(cit->str);
		cit->endpos = cit->pos + cit->str.length();
		dim_.wid = wid + cit->dim.wid;
		// If there are other elements, they should be removed.
		elements_.erase(next(cit, 1), end);
		return true;
	}
	return false;
}


void Row::reverseRTL(bool const rtl_par)
{
	pos_type i = 0;
	pos_type const end = elements_.size();
	while (i < end) {
		// gather a sequence of elements with the same direction
		bool const rtl = elements_[i].isRTL();
		pos_type j = i;
		while (j < end && elements_[j].isRTL() == rtl)
			++j;
		// if the direction is not the same as the paragraph
		// direction, the sequence has to be reverted.
		if (rtl != rtl_par)
			reverse(elements_.begin() + i, elements_.begin() + j);
		i = j;
	}
	// If the paragraph itself is RTL, reverse everything
	if (rtl_par)
		reverse(elements_.begin(), elements_.end());
	rtl_ = rtl_par;
}

Row::const_iterator const
Row::findElement(pos_type const pos, bool const boundary, double & x) const
{
	/**
	 * When boundary is true, position i is in the row element (pos, endpos)
	 * if
	 *    pos < i <= endpos
	 * whereas, when boundary is false, the test is
	 *    pos <= i < endpos
	 * The correction below allows to handle both cases.
	*/
	int const boundary_corr = (boundary && pos) ? -1 : 0;

	x = left_margin;

	/** Early return in trivial cases
	 * 1) the row is empty
	 * 2) the position is the left-most position of the row; there
	 * is a quirk here however: if the first element is virtual
	 * (end-of-par marker for example), then we have to look
	 * closer
	 */
	if (empty()
	    || (pos == begin()->left_pos() && !boundary
			&& !begin()->isVirtual()))
		return begin();

	Row::const_iterator cit = begin();
	for ( ; cit != end() ; ++cit) {
		/** Look whether the cursor is inside the element's
		 * span. Note that it is necessary to take the
		 * boundary into account, and to accept virtual
		 * elements, which have pos == endpos.
		 */
		if (pos + boundary_corr >= cit->pos
		    && (pos + boundary_corr < cit->endpos || cit->isVirtual())) {
				x += cit->pos2x(pos);
				break;
		}
		x += cit->full_width();
	}

	if (cit == end())
		--cit;

	return cit;
}


} // namespace lyx
