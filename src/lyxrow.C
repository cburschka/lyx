/**
 * \file lyxrow.C
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

#include "lyxrow.h"
#include "debug.h"

using lyx::pos_type;


RowMetrics::RowMetrics()
	: separator(0), hfill(0), label_hfill(0), x(0)
{}


Row::Row()
	: pos_(0), end_(0), ascent_(0), descent_(0), width_(0)
{}


Row::Row(pos_type pos)
	: pos_(pos), end_(0), ascent_(0), descent_(0), width_(0)
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


void Row::width(int w)
{
	width_ = w;
}


int Row::width() const
{
	return width_;
}


void Row::ascent(int b)
{
	ascent_ = b;
}


int Row::ascent() const
{
	return ascent_;
}


void Row::dump(const char * s) const
{
	lyxerr << s << " pos: " << pos_ << " end: " << end_
		<< " width: " << width_
		<< " ascent: " << ascent_
		<< " descent: " << descent_
		<< std::endl;
}
