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


FontIterator::FontIterator(LyXText const & text, Paragraph const & par,
			   lyx::pos_type pos)
	: text_(text), par_(par), pos_(pos),
	  font_(text.getFont(par, pos)),
	  endspan_(par.getEndPosOfFontSpan(pos)),
	  bodypos_(par.beginOfBody())
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
		font_ = text_.getFont(par_, pos_);
		endspan_ = par_.getEndPosOfFontSpan(pos_);
	}
	return *this;
}
