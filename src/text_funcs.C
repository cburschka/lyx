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

using lyx::pos_type;

void transposeChars(LyXText & text, LyXCursor const & cursor)
{
	Paragraph * tmppar = &*cursor.par();

	setUndo(text.bv(), Undo::FINISH, tmppar, tmppar->next());

	pos_type tmppos = cursor.pos();

	// First decide if it is possible to transpose at all

	if (tmppos == 0 || tmppos == tmppar->size())
		return;

	if (isDeletedText(*tmppar, tmppos - 1)
		|| isDeletedText(*tmppar, tmppos))
		return;

	unsigned char c1 = tmppar->getChar(tmppos);
	unsigned char c2 = tmppar->getChar(tmppos - 1);

	// We should have an implementation that handles insets
	// as well, but that will have to come later. (Lgb)
	if (c1 == Paragraph::META_INSET || c2 == Paragraph::META_INSET)
		return;

	bool const erased = tmppar->erase(tmppos - 1, tmppos + 1);
	pos_type const ipos(erased ? tmppos - 1 : tmppos + 1);

	tmppar->insertChar(ipos, c1);
	tmppar->insertChar(ipos + 1, c2);

	text.checkParagraph(tmppar, tmppos);
}
