/**
 * \file textcursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "textcursor.h"


LyXCursor const & TextCursor::selStart() const
{
	if (!selection.set())
		return cursor_;
	// can't use std::min as this creates a new object
	return anchor_ < cursor_ ? anchor_ : cursor_;
}


LyXCursor const & TextCursor::selEnd() const
{
	if (!selection.set())
		return cursor_;
	return anchor_ > cursor_ ? anchor_ : cursor_;
}


LyXCursor & TextCursor::selStart()
{
	if (!selection.set())
		return cursor_;
	return anchor_ < cursor_ ? anchor_ : cursor_;
}


LyXCursor & TextCursor::selEnd()
{
	if (!selection.set())
		return cursor_;
	return anchor_ > cursor_ ? anchor_ : cursor_;
}


void TextCursor::setSelection()
{
	selection.set(true);
	// a selection with no contents is not a selection
	if (cursor_.par() == anchor_.par() && cursor_.pos() == anchor_.pos())
		selection.set(false);
}


void TextCursor::clearSelection()
{
	selection.set(false);
	selection.mark(false);
	anchor_ = cursor_;
}

