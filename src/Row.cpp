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
	: changed_(false), crc_(0), pos_(0), end_(0), separator(0), hfill(0),
	label_hfill(0), x(0)
{}


Row::Row(pos_type pos)
	: changed_(false), crc_(0), pos_(pos), end_(0), separator(0), hfill(0),
	label_hfill(0), x(0)
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
	changed_ |= p != pos_;
	pos_ = p;
}


void Row::endpos(pos_type p)
{
	changed_ |= p != end_;
	end_ = p;
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
