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
 * Calling LyXText::getFont is slow. While rebreaking we scan a
 * paragraph from left to right calling getFont for every char.  This
 * simple class address this problem by hidding an optimization trick
 * (not mine btw -AB): the font is reused in the whole font span.  The
 * class handles transparently the "hidden" (not part of the fontlist)
 * label font (as getFont does).
 */

#ifndef FONTITERATOR_H
#define FONTITERATOR_H

#include "lyxfont.h"

#include "support/types.h"

class LyXText;


class FontIterator : std::iterator<std::forward_iterator_tag, LyXFont>
{
public:
	///
	FontIterator(LyXText const & text, lyx::pit_type pit, lyx::pos_type pos);
	///
	LyXFont operator*() const;
	///
	FontIterator & operator++();
	///
	LyXFont * operator->();

private:
	///
	LyXText const & text_;
	///
	lyx::pit_type pit_;
	///
	lyx::pos_type pos_;
	///
	LyXFont font_;
	///
	lyx::pos_type endspan_;
	///
	lyx::pos_type bodypos_;
};

#endif // FONTITERATOR_H
