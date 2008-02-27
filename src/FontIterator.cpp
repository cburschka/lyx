/**
 * \file src/FontIterator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 *
 */

#include <config.h>

#include "FontIterator.h"

#include "TextMetrics.h"
#include "Paragraph.h"


namespace lyx {


FontIterator::FontIterator(TextMetrics const & tm,
		Paragraph const & par, pit_type pit, pos_type pos)
	: tm_(tm), par_(par), pit_(pit), pos_(pos),
	  font_(tm.displayFont(pit, pos)),
	  endspan_(par.fontSpan(pos).last),
	  bodypos_(par.beginOfBody())
{}


Font const & FontIterator::operator*() const
{
	return font_;
}


Font * FontIterator::operator->()
{
	return &font_;
}


FontIterator & FontIterator::operator++()
{
	++pos_;
	if (pos_ > endspan_ || pos_ == bodypos_) {
		font_ = tm_.displayFont(pit_, pos_);
		endspan_ = par_.fontSpan(pos_).last;
	}
	return *this;
}


} // namespace lyx
