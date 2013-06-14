/**
 * \file Row.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 *
 * Metrics for an on-screen text row.
 */

#include <config.h>

#include "Row.h"

#include "DocIterator.h"

#include "support/debug.h"


namespace lyx {


Row::Row()
	: separator(0), label_hfill(0), x(0),
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


void Row::dump(char const * s) const
{
	LYXERR0(s << " pos: " << pos_ << " end: " << end_
		<< " width: " << dim_.wid
		<< " ascent: " << dim_.asc
		<< " descent: " << dim_.des);
}


} // namespace lyx
