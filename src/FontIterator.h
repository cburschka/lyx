// -*- C++ -*-
/**
 * \file src/FontIterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 *
 *
 * Calling Text::getFont is slow. While rebreaking we scan a
 * paragraph from left to right calling getFont for every char.  This
 * simple class address this problem by hidding an optimization trick
 * (not mine btw -AB): the font is reused in the whole font span.  The
 * class handles transparently the "hidden" (not part of the fontlist)
 * label font (as getFont does).
 */

#ifndef FONTITERATOR_H
#define FONTITERATOR_H

#include "Font.h"

#include "support/types.h"


namespace lyx {

class Paragraph;
class TextMetrics;


class FontIterator : std::iterator<std::forward_iterator_tag, Font>
{
public:
	///
	FontIterator(TextMetrics const & tm,
		Paragraph const & par, pos_type pos);
	///
	Font const & operator*() const;
	///
	FontIterator & operator++();
	///
	Font * operator->();

private:
	///
	TextMetrics const & tm_;
	///
	Paragraph const & par_;
	///
	pos_type pos_;
	///
	Font font_;
	///
	pos_type endspan_;
	///
	pos_type bodypos_;
};


} // namespace lyx

#endif // FONTITERATOR_H
