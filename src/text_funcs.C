/**
 * \file text_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 *
 * This file contains some utility functions for actually mutating
 * the text contents of a document
 */

#include <config.h>

#include "lyxtext.h"
#include "paragraph.h"
#include "lyxcursor.h"
#include "undo_funcs.h"

#include <boost/next_prior.hpp>

using lyx::pos_type;

void transposeChars(LyXText & text, LyXCursor const & cursor)
{
	ParagraphList::iterator tmppit = cursor.par();

	setUndo(text.bv(), Undo::FINISH, &*tmppit, &*boost::next(tmppit));

	pos_type tmppos = cursor.pos();

	// First decide if it is possible to transpose at all

	if (tmppos == 0 || tmppos == tmppit->size())
		return;

	if (isDeletedText(*tmppit, tmppos - 1)
		|| isDeletedText(*tmppit, tmppos))
		return;

	unsigned char c1 = tmppit->getChar(tmppos);
	unsigned char c2 = tmppit->getChar(tmppos - 1);

	// We should have an implementation that handles insets
	// as well, but that will have to come later. (Lgb)
	if (c1 == Paragraph::META_INSET || c2 == Paragraph::META_INSET)
		return;

	bool const erased = tmppit->erase(tmppos - 1, tmppos + 1);
	pos_type const ipos(erased ? tmppos - 1 : tmppos + 1);

	tmppit->insertChar(ipos, c1);
	tmppit->insertChar(ipos + 1, c2);

	text.checkParagraph(tmppit, tmppos);
}
