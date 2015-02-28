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

#include <algorithm>
#include <ostream>

#include <boost/next_prior.hpp>

using namespace std;

namespace lyx {

using frontend::FontMetrics;

double Row::Element::pos2x(pos_type const i) const
{
	// This can happen with inline completion when clicking on the
	// row after the completion.
	if (i < pos || i > endpos)
		return 0;

	bool const rtl = font.isVisibleRightToLeft();

	double w = 0;
	//handle first the two bounds of the element
	if (i == endpos && type != VIRTUAL
		&& !(inset && inset->lyxCode() == SEPARATOR_CODE))
		w = rtl ? 0 : full_width();
	else if (i == pos || type != STRING)
		w = rtl ? full_width() : 0;
	else {
		FontMetrics const & fm = theFontMetrics(font);
		w = fm.pos2x(str, i - pos, font.isVisibleRightToLeft());
	}

	return w;
}


pos_type Row::Element::x2pos(int &x) const
{
	//lyxerr << "x2pos: x=" << x << " w=" << width() << " " << *this;
	bool const rtl = font.isVisibleRightToLeft();
	size_t i = 0;

	switch (type) {
	case STRING: {
		FontMetrics const & fm = theFontMetrics(font);
		i = fm.x2pos(str, x, rtl);
		break;
	}
	case VIRTUAL:
		// those elements are actually empty (but they have a width)
		i = 0;
		x = rtl ? int(full_width()) : 0;
		break;
	case SEPARATOR:
	case INSET:
	case SPACE:
		// those elements contain only one position. Round to
		// the closest side.
		if (x > full_width()) {
			x = int(full_width());
			i = !rtl;
		} else {
			x = 0;
			i = rtl;
		}

	}
	//lyxerr << "=> p=" << pos + i << " x=" << x << endl;
	return pos + i;

}


bool Row::Element::breakAt(int w)
{
	if (type != STRING || dim.wid <= w)
		return false;

	bool const rtl = font.isVisibleRightToLeft();
	if (rtl)
		w = dim.wid - w;
	pos_type new_pos = x2pos(w);
	if (new_pos == pos)
		return false;
	str = str.substr(0, new_pos - pos);
	if (rtl)
		dim.wid -= w;
	else
		dim.wid = w;
	endpos = new_pos;
	return true;
}


pos_type Row::Element::left_pos() const
{
	return font.isVisibleRightToLeft() ? endpos : pos;
}


pos_type Row::Element::right_pos() const
{
	return font.isVisibleRightToLeft() ? pos : endpos;
}


Row::Row()
	: separator(0), label_hfill(0), left_margin(0), right_margin(0),
	  sel_beg(-1), sel_end(-1),
	  begin_margin_sel(false), end_margin_sel(false),
	  changed_(false), crc_(0), pos_(0), end_(0), right_boundary_(false)
{}


void Row::setCrc(size_type crc) const
{
	changed_ = crc != crc_;
	crc_ = crc;
}


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
		end_margin_sel = isMarginSelected(false, beg, end);
		begin_margin_sel = isMarginSelected(true, beg, end);
	}
}


void Row::setSelection(pos_type beg, pos_type end) const
{
	if (pos_ >= beg && pos_ <= end)
		sel_beg = pos_;
	else if (beg > pos_ && beg <= end_)
		sel_beg = beg;
	else
		sel_beg = -1;

	if (end_ >= beg && end_ <= end)
		sel_end = end_;
	else if (end < end_ && end >= pos_)
		sel_end = end;
	else
		sel_end = -1;
}


bool Row::selection() const
{
	return sel_beg != -1 && sel_end != -1;
}


ostream & operator<<(ostream & os, Row::Element const & e)
{
	if (e.font.isVisibleRightToLeft())
		os << e.endpos << "<<" << e.pos << " ";
	else
		os << e.pos << ">>" << e.endpos << " ";

	switch (e.type) {
	case Row::STRING:
		os << "STRING: `" << to_utf8(e.str) << "', ";
		break;
	case Row::VIRTUAL:
		os << "VIRTUAL: `" << to_utf8(e.str) << "', ";
		break;
	case Row::INSET:
		os << "INSET: " << to_utf8(e.inset->layoutName()) << ", ";
		break;
	case Row::SEPARATOR:
		os << "SEPARATOR: extra=" << e.extra << ", ";
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

	if (elt.type == STRING) {
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
	back().str += c;
	back().endpos = pos + 1;
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


void Row::addSeparator(pos_type const pos, char_type const c,
		       Font const & f, Change const & ch)
{
	finalizeLast();
	Element e(SEPARATOR, pos, f, ch);
	e.str += c;
	e.dim.wid = theFontMetrics(f).width(c);
	elements_.push_back(e);
	dim_.wid += e.dim.wid;
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


void Row::shortenIfNeeded(pos_type const keep, int const w)
{
	if (empty() || width() <= w)
		return;

	Elements::iterator const beg = elements_.begin();
	Elements::iterator const end = elements_.end();
	Elements::iterator last_sep = elements_.end();
	int last_width = 0;
	int wid = left_margin;

	Elements::iterator cit = beg;
	for ( ; cit != end ; ++cit) {
		if (cit->type == SEPARATOR && cit->pos >= keep) {
			last_sep = cit;
			last_width = wid;
		}
		if (wid + cit->dim.wid > w)
			break;
		wid += cit->dim.wid;
	}

	if (last_sep != end) {
		// We have found a suitable separator. This is the
		// common case.
		end_ = last_sep->endpos;
		dim_.wid = last_width;
		elements_.erase(last_sep, end);
		return;
	}

	if (cit == end) {
		// This should not happen since the row is too long.
		LYXERR0("Something is wrong cannot shorten row: " << *this);
		return;
	}

	if (cit != beg && cit->type == VIRTUAL) {
		// It is not possible to separate a virtual element from the
		// previous one.
		--cit;
		wid -= cit->dim.wid;
	}

	if (cit != beg) {
		// There is no separator, but several elements (probably
		// insets) have been added. We can cut at this place.
		end_ = cit->pos;
		dim_.wid = wid;
		elements_.erase(cit, end);
		return;
	}

	/* If we are here, it means that we have not found a separator
	 * to shorten the row. There is one case where we can do
	 * something: when we have one big string, maybe with some
	 * other things after it.
	 */
	if (cit->breakAt(w - left_margin)) {
		end_ = cit->endpos;
		dim_.wid = left_margin + cit->dim.wid;
		// If there are other elements, they should be removed.
		elements_.erase(boost::next(cit), end);
	}
}


void Row::reverseRTL(bool const rtl_par)
{
	pos_type i = 0;
	pos_type const end = elements_.size();
	while (i < end) {
		// gather a sequence of elements with the same direction
		bool const rtl = elements_[i].font.isVisibleRightToLeft();
		pos_type j = i;
		while (j < end && elements_[j].font.isVisibleRightToLeft() == rtl)
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
}

} // namespace lyx
