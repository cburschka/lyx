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
		return cursor;
	return selection.cursor < cursor ? selection.cursor : cursor;
}


LyXCursor const & TextCursor::selEnd() const
{
	if (!selection.set())
		return cursor;
	return selection.cursor < cursor ? cursor : selection.cursor;
}


LyXCursor & TextCursor::selStart()
{
	TextCursor const & t = *this;
	return const_cast<LyXCursor &>(t.selStart());
}


LyXCursor & TextCursor::selEnd()
{
	TextCursor const & t = *this;
	return const_cast<LyXCursor &>(t.selEnd());
}


void TextCursor::setSelection()
{
	selection.set(true);
	// a selection with no contents is not a selection
	if (cursor.par() == selection.cursor.par() &&
	    cursor.pos() == selection.cursor.pos())
	{
		selection.set(false);
	}
}


void TextCursor::clearSelection()
{
	selection.set(false);
	selection.mark(false);
	selection.cursor = cursor;
}

