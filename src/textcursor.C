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


void TextCursor::setSelection()
{
	if (!selection.set()) {
		selection.start = selection.cursor;
		selection.end = selection.cursor;
	}

	selection.set(true);

	// and now the whole selection
	if (selection.cursor.par() == cursor.par())
		if (selection.cursor.pos() < cursor.pos()) {
			selection.end = cursor;
			selection.start = selection.cursor;
		} else {
			selection.end = selection.cursor;
			selection.start = cursor;
		}
	else if (selection.cursor.y() < cursor.y() ||
		 (selection.cursor.y() == cursor.y()
		  && selection.cursor.x() < cursor.x())) {
		selection.end = cursor;
		selection.start = selection.cursor;
	}
	else {
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


string const TextCursor::selectionAsString(Buffer const & buffer,
					bool label) const
{
	if (!selection.set())
		return string();

	// should be const ...
	ParagraphList::iterator startpit = selection.start.par();
	ParagraphList::iterator endpit = selection.end.par();
	size_t const startpos = selection.start.pos();
	size_t const endpos = selection.end.pos();

	if (startpit == endpit)
		return startpit->asString(buffer, startpos, endpos, label);

	// First paragraph in selection
	string result =
		startpit->asString(buffer, startpos, startpit->size(), label) + "\n\n";

	// The paragraphs in between (if any)
	ParagraphList::iterator pit = startpit;
	for (++pit; pit != endpit; ++pit)
		result += pit->asString(buffer, 0, pit->size(), label) + "\n\n";

	// Last paragraph in selection
	result += endpit->asString(buffer, 0, endpos, label);

	return result;
}
