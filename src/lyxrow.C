/**
 * \file lyxrow.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 *
 * Metrics for an on-screen text row.
 */

#include <config.h>

#include "lyxrow.h"
#include "debug.h"

using lyx::pos_type;

using std::max;
using std::min;

Row::Row()
	: pos_(0), end_(0), fill_(0), height_(0), width_(0), y_(0),
	  ascent_of_text_(0), baseline_(0)
{}


Row::Row(pos_type pos)
	: pos_(pos), end_(0), fill_(0), height_(0), width_(0), y_(0),
	  ascent_of_text_(0), baseline_(0)
{}


void Row::y(unsigned int newy)
{
	y_ = newy;
}


unsigned int Row::y() const
{
	return y_;
}


unsigned short Row::height() const
{
	return height_;
}


void Row::pos(pos_type p)
{
	pos_ = p;
}


pos_type Row::pos() const
{
	return pos_;
}


void Row::end(pos_type p)
{
	end_ = p;
}


pos_type Row::end() const
{
	return end_;
}


void Row::fill(int f)
{
	fill_ = f;
}


int Row::fill() const
{
	return fill_;
}


void Row::height(unsigned short h)
{
	height_ = h;
}


void Row::width(unsigned int w)
{
	width_ = w;
}


unsigned int Row::width() const
{
	return width_;
}


void Row::ascent_of_text(unsigned short a)
{
	ascent_of_text_ = a;
}


unsigned short Row::ascent_of_text() const
{
	return ascent_of_text_;
}


void Row::top_of_text(unsigned int top)
{
	top_of_text_ = top;
}


unsigned int Row::top_of_text() const
{
	return top_of_text_;
}


void Row::baseline(unsigned int b)
{
	baseline_ = b;
}


unsigned int Row::baseline() const
{
	return baseline_;
}


bool Row::isParStart() const
{
	return !pos();
}


void Row::dump(const char * s) const
{
	lyxerr << s << " pos: " << pos_ << " width: " << width_
		<< " height: " << height_
		<< " fill: " << fill_
		<< " ascent_of_text: " << ascent_of_text_
		<< " top_of_text: " << top_of_text_
		<< " y: " << y_ << std::endl;
}

