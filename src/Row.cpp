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
#include "debug.h"


namespace lyx {


Row::Row()
	: separator(0), hfill(0), label_hfill(0), x(0),
	sel_beg(-1), sel_end(-1), changed_(false), crc_(0), pos_(0), end_(0)
{}


Row::Row(pos_type pos)
	: separator(0), hfill(0), label_hfill(0), x(0),
	sel_beg(-1), sel_end(-1), changed_(false), crc_(0), pos_(pos), end_(0)
{}


void Row::setCrc(size_type crc)
{
	changed_ |= crc != crc_;
	crc_ = crc;
}


void Row::setDimension(Dimension const & dim)
{
	changed_ |= dim != dim_;
	dim_ = dim;
}


void Row::pos(pos_type p)
{
	pos_ = p;
}


void Row::endpos(pos_type p)
{
	end_ = p;
}


void Row::setSelection(pos_type beg, pos_type end)
{
	pos_type sel_beg_b = sel_beg;
	if (pos_ >= beg && pos_ <= end)
		sel_beg = pos_;
	else if (beg > pos_ && beg <= end_)
		sel_beg = beg;
	else
		sel_beg = -1;

	pos_type sel_end_b = sel_end;
	if (end_ >= beg && end_ <= end)
		sel_end = end_;
	else if (end < end_ && end >= pos_)
		sel_end = end;
	else
		sel_end = -1;
/*
			&& ((rit->pos() >= beg.pos() && rit->pos() <= end.pos())
				|| (rit->endpos() >= beg.pos() && rit->endpos() <= end.pos())
				|| (beg.pos() >= rit->pos() && beg.pos() <= rit->endpos())
				|| (end.pos() >= rit->pos() && end.pos() <= rit->endpos()));
*/
	changed_ |= sel_beg_b != sel_beg;
	changed_ |= sel_end_b != sel_end;
}


void Row::dump(char const * s) const
{
	lyxerr << s << " pos: " << pos_ << " end: " << end_
		<< " width: " << dim_.wid
		<< " ascent: " << dim_.asc
		<< " descent: " << dim_.des
		<< std::endl;
}


} // namespace lyx
