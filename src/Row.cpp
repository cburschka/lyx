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


RowMetrics::RowMetrics()
	: separator(0), hfill(0), label_hfill(0), x(0)
{}


Row::Row()
	: pos_(0), end_(0)
{}


Row::Row(pos_type pos)
	: pos_(pos), end_(0)
{}


void Row::pos(pos_type p)
{
	pos_ = p;
}


pos_type Row::pos() const
{
	return pos_;
}


void Row::endpos(pos_type p)
{
	end_ = p;
}


pos_type Row::endpos() const
{
	return end_;
}


void Row::dump(const char * s) const
{
	lyxerr << s << " pos: " << pos_ << " end: " << end_
		<< " width: " << dim_.wid
		<< " ascent: " << dim_.asc
		<< " descent: " << dim_.des
		<< std::endl;
}


} // namespace lyx
