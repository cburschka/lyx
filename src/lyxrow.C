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
#include "paragraph.h"
#include "layout.h"
#include "lyxlayout.h"

using lyx::pos_type;

using std::max;
using std::min;

Row::Row()
	: par_(0), pos_(0), fill_(0), height_(0), width_(0),
	  ascent_of_text_(0), baseline_(0), next_(0), previous_(0)
{}


Row::Row(Paragraph * pa, pos_type po)
	: par_(pa), pos_(po), fill_(0), height_(0), width_(0),
	  ascent_of_text_(0), baseline_(0), next_(0), previous_(0)
{}


Paragraph * Row::par()
{
	return par_;
}


Paragraph * Row::par() const
{
	return par_;
}


unsigned short Row::height() const
{
	return height_;
}


Row * Row::next() const
{
	return next_;
}


void Row::par(Paragraph * p)
{
	par_ = p;
}


void Row::pos(pos_type p)
{
	pos_ = p;
}


pos_type Row::pos() const
{
	return pos_;
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


void Row::next(Row * r)
{
	next_ = r;
}


void Row::previous(Row * r)
{
	previous_ = r;
}


Row * Row::previous() const
{
	return previous_;
}


bool Row::isParStart() const
{
	return !pos();
}


bool Row::isParEnd() const
{
	return !next() || next()->par() != par();
}


pos_type Row::lastPos() const
{
	if (par()->empty())
		return 0;

	if (isParEnd()) {
		return par()->size() - 1;
	} else {
		return next()->pos() - 1;
	}
}


namespace {

bool nextRowIsAllInset(Row const & row, pos_type last)
{
	if (last + 1 >= row.par()->size())
		return false;

	if (!row.par()->isInset(last + 1))
		return false;

	Inset * i = row.par()->getInset(last + 1);
	return i->needFullRow() || i->display();
}

};


pos_type Row::lastPrintablePos() const
{
	pos_type const last = lastPos();

	// if this row is an end of par, just act like lastPos()
	if (isParEnd())
		return last;

	bool const nextrownotinset = !nextRowIsAllInset(*this, last);

	if (nextrownotinset && par()->isSeparator(last))
		return last - 1;

	return last;
}


int Row::numberOfSeparators() const
{
	pos_type const last = lastPrintablePos();
	pos_type p = max(pos(), par()->beginningOfBody());

	int n = 0;
	for (; p < last; ++p) {
		if (par()->isSeparator(p)) {
			++n;
		}
	}
	return n;
}


int Row::numberOfHfills() const
{
	pos_type const last = lastPos();
	pos_type first = pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par()->isHfill(first)) {
			++first;
		}
	}

	first = max(first, par()->beginningOfBody());

	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p) {
		if (par()->isHfill(p))
			++n;
	}
	return n;
}


int Row::numberOfLabelHfills() const
{
	pos_type last = lastPos();
	pos_type first = pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par()->isHfill(first))
			++first;
	}

	last = min(last, par()->beginningOfBody());
	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p) {
		if (par()->isHfill(p))
			++n;
	}
	return n;
}


bool Row::hfillExpansion(pos_type pos) const
{
	if (!par()->isHfill(pos))
		return false;

	// at the end of a row it does not count
	// unless another hfill exists on the line
	if (pos >= lastPos()) {
		pos_type i = this->pos();
		while (i < pos && !par()->isHfill(i)) {
			++i;
		}
		if (i == pos) {
			return false;
		}
	}

	// at the beginning of a row it does not count, if it is not
	// the first row of a paragaph
	if (isParStart())
		return true;

	// in some labels it does not count
	if (par()->layout()->margintype != MARGIN_MANUAL
	    && pos < par()->beginningOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	pos_type i = this->pos();
	while (i < pos && (par()->isNewline(i) || par()->isHfill(i)))
		++i;

	return i != pos;
}
