/**
 * \file src/FontIterator.C
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

#include "lyxtext.h"
#include "paragraph.h"


FontIterator::FontIterator(LyXText const & text, lyx::par_type pit,
			   lyx::pos_type pos)
	: text_(text), pit_(pit), pos_(pos),
	  font_(text.getFont(pit, pos)),
	  endspan_(text.getPar(pit).getEndPosOfFontSpan(pos)),
	  bodypos_(text.getPar(pit).beginOfBody())
{}


LyXFont FontIterator::operator*() const
{
	return font_;
}


LyXFont * FontIterator::operator->()
{
	return &font_;
}


FontIterator & FontIterator::operator++()
{
	++pos_;
	if (pos_ > endspan_ || pos_ == bodypos_) {
		font_ = text_.getFont(pit_, pos_);
		endspan_ = text_.getPar(pit_).getEndPosOfFontSpan(pos_);
	}
	return *this;
}
