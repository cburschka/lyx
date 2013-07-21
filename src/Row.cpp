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

using namespace std;

namespace lyx {

using frontend::FontMetrics;

double Row::Element::pos2x(pos_type const i) const
{
	bool const rtl = font.isVisibleRightToLeft();

	// handle first the two bounds of the element
	if ((!rtl && pos >= i) || (rtl && endpos <= i))
		return 0;
	if ((!rtl && endpos <= i) || (rtl && pos >= i))
		return width();

	FontMetrics const & fm = theFontMetrics(font);
	// FIXME Avoid caching of metrics there?
	int const w = fm.width(str.substr(0, i - pos));
	if (rtl)
		return width() - w;
	else
		return w;
}


pos_type Row::Element::x2pos(double &x) const
{
	//lyxerr << "x2pos: x=" << x << " w=" << width() << " " << *this;
	// if element is rtl, flip x value
	bool const rtl = font.isVisibleRightToLeft();
	double x2 = rtl ? (width() - x) : x;

	FontMetrics const & fm = theFontMetrics(font);
	double last_w = 0;
	double w = 0;
	size_t i = 1;
	// non-STRING element only contain one position
	if (type != STRING) {
		i = 0;
		w = width();
	} else {
		// FIXME: implement dichotomy search?
		for ( ; i <= str.size() ; ++i) {
			last_w = w;
			w = fm.width(str.substr(0,i));
			if (w > x2) {
				--i;
				break;
			}
		}
		// if (i == str.size())
		// 	lyxerr << " NOT FOUND ";
	}

	// round to the closest side
	if (x2 - last_w > w - x2) {
		x2 = w;
		++i;
	} else
		x2 = last_w;

	// is element is rtl, flip values
	if (rtl) {
		x = last_w - x2;
		i = endpos - i;
	} else {
		x = x2;
		i = pos + i;
	}

	//lyxerr << "=> p=" << i << " x=" << x << endl;
	return i;
}


Row::Row()
	: separator(0), label_hfill(0), x(0), right_margin(0),
	sel_beg(-1), sel_end(-1),
	begin_margin_sel(false), end_margin_sel(false),
	changed_(false), crc_(0), pos_(0), end_(0)
{}


void Row::setCrc(size_type crc) const
{
	changed_ = crc != crc_;
	crc_ = crc;
}


void Row::pos(pos_type p)
{
	pos_ = p;
}


void Row::endpos(pos_type p)
{
	end_ = p;
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
		os << "STRING: `" << to_utf8(e.str) << "' " << e.dim.wid;
		break;
	case Row::VIRTUAL:
		os << "VIRTUAL: `" << to_utf8(e.str) << "'";
		break;
	case Row::INSET:
		os << "INSET: " << to_utf8(e.inset->layoutName());
		break;
	case Row::SEPARATOR:
		os << "SEPARATOR: " << e.dim.wid << "+" << e.extra;
		break;
	case Row::SPACE:
		os << "SPACE: " << e.dim.wid;
		break;
	}
	return os;
}


ostream & operator<<(ostream & os, Row const & row)
{
	os << " pos: " << row.pos_ << " end: " << row.end_
	   << " x: " << row.x
	   << " width: " << row.dim_.wid
	   << " ascent: " << row.dim_.asc
	   << " descent: " << row.dim_.des
	   << " separator: " << row.separator
	   << " label_hfill : " << row.label_hfill << "\n";
	Row::Elements::const_iterator it = row.elements_.begin();
	for ( ; it != row.elements_.end() ; ++it) {
		os << "** " << *it << endl;
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
	//lyxerr << "FONT " <<back().font.language() << endl;
	back().str += c;
	back().endpos = pos + 1;
}


void Row::addVirtual(pos_type const pos, docstring const & s,
		     Font const & f, Change const & ch)
{
	finalizeLast();
	Element e(VIRTUAL, pos, f, ch);
	e.str = s;
	// A completion has no size
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


void Row::separate_back(pos_type const keep)
{
	if (empty())
		return;
	int i = elements_.size();
	int new_end = end_;
	int new_wid = dim_.wid;
	if (i > 0 && elements_[i - 1].type == SEPARATOR && new_end > keep) {
		--i;
		new_end = elements_[i].pos;
		new_wid -= elements_[i].dim.wid;
	}

	while (i > 0 && elements_[i - 1].type != SEPARATOR && new_end > keep) {
		--i;
		new_end = elements_[i].pos;
		new_wid -= elements_[i].dim.wid;
	}
	if (i == 0)
		return;
	end_ = new_end;
	dim_.wid = new_wid;
	elements_.erase(elements_.begin() + i, elements_.end());
}


void Row::reverseRtL()
{
	pos_type i = 0;
	pos_type const end = elements_.size();
	while (i < end) {
		// skip LtR elements
		while (i < end && !elements_[i].font.isRightToLeft())
			++i;
		if (i >= end)
			break;

		// look for a RtL sequence
		pos_type j = i;
		while (j < end && elements_[j].font.isRightToLeft())
			++j;
		reverse(elements_.begin() + i, elements_.begin() + j);
		i = j;
	}
}

} // namespace lyx
