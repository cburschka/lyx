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
#include "paragraph.h"
#include "ParagraphList_fwd.h"
#include "debug.h"

#include <string>

using std::string;


void TextCursor::setSelection()
{
	selection.set(true);

	if (selection.cursor.par() == cursor.par())
		if (selection.cursor.pos() < cursor.pos()) {
			selection.end = cursor;
			selection.start = selection.cursor;
		} else {
			selection.end = selection.cursor;
			selection.start = cursor;
		}
	else if (selection.cursor.par() < cursor.par() ||
		 (selection.cursor.par() == cursor.par()
		  && selection.cursor.pos() < cursor.pos())) {
		selection.end = cursor;
		selection.start = selection.cursor;
	} else {
		selection.end = selection.cursor;
		selection.start = cursor;
	}

	// a selection with no contents is not a selection
	if (selection.start.par() == selection.end.par() &&
	    selection.start.pos() == selection.end.pos())
	{
		selection.set(false);
	}
}


void TextCursor::clearSelection()
{
	selection.set(false);
	selection.mark(false);
	selection.end    = cursor;
	selection.start  = cursor;
	selection.cursor = cursor;
}

