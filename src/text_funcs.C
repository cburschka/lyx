/**
 * \file text_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 *
 * This file contains some utility functions for actually mutating
 * the text contents of a document
 */

#include <config.h>

#include "debug.h"
#include "lyxcursor.h"
#include "ParagraphList.h"

#include "support/types.h"

#include <boost/next_prior.hpp>

using lyx::pos_type;
using lyx::word_location;

using std::endl;


bool transposeChars(LyXCursor const & cursor)
{
	ParagraphList::iterator tmppit = cursor.par();
	pos_type tmppos = cursor.pos();

	// First decide if it is possible to transpose at all

	if (tmppos == 0 || tmppos == tmppit->size())
		return false;

	if (isDeletedText(*tmppit, tmppos - 1)
		|| isDeletedText(*tmppit, tmppos))
		return false;

	unsigned char c1 = tmppit->getChar(tmppos);
	unsigned char c2 = tmppit->getChar(tmppos - 1);

	// We should have an implementation that handles insets
	// as well, but that will have to come later. (Lgb)
	if (c1 == Paragraph::META_INSET || c2 == Paragraph::META_INSET)
		return false;

	bool const erased = tmppit->erase(tmppos - 1, tmppos + 1);
	size_t const ipos = erased ? tmppos - 1 : tmppos + 1;

	tmppit->insertChar(ipos, c1);
	tmppit->insertChar(ipos + 1, c2);
	return true;
}


void cursorLeftOneWord(LyXCursor & cursor, ParagraphList const & pars)
{
	// treat HFills, floats and Insets as words

	ParagraphList::iterator pit = cursor.par();
	size_t pos = cursor.pos();

	while (pos &&
	       (pit->isSeparator(pos - 1) ||
	        pit->isKomma(pos - 1) ||
	        pit->isNewline(pos - 1)) &&
	       !(pit->isHfill(pos - 1) ||
		 pit->isInset(pos - 1)))
		--pos;

	if (pos &&
	    (pit->isInset(pos - 1) ||
	     pit->isHfill(pos - 1))) {
		--pos;
	} else if (!pos) {
		// cast only for BSD's g++ 2.95
		if (pit != const_cast<ParagraphList &>(pars).begin()) {
			--pit;
			pos = pit->size();
		}
	} else {		// Here, cur != 0
		while (pos > 0 && pit->isWord(pos - 1))
			--pos;
	}

	cursor.par(pit);
	cursor.pos(pos);
}


void cursorRightOneWord(LyXCursor & cursor, ParagraphList const & pars)
{
	// treat floats, HFills and Insets as words
	ParagraphList::iterator pit = cursor.par();
	pos_type pos = cursor.pos();

	// CHECK See comment on top of text.C

	// cast only for BSD's g++ 2.95
	if (pos == pit->size() &&
		boost::next(pit) != const_cast<ParagraphList &>(pars).end()) {
		++pit;
		pos = 0;
	} else {
		// Skip through initial nonword stuff.
		while (pos < pit->size() && !pit->isWord(pos)) {
			++pos;
		}
		// Advance through word.
		while (pos < pit->size() && pit->isWord(pos)) {
			++pos;
		}
	}

	cursor.par(pit);
	cursor.pos(pos);
}


// Select current word. This depends on behaviour of
// CursorLeftOneWord(), so it is patched as well.
void getWord(LyXCursor & from, LyXCursor & to, word_location const loc,
	ParagraphList const & pars)
{
	switch (loc) {
	case lyx::WHOLE_WORD_STRICT:
		if (from.pos() == 0 || from.pos() == from.par()->size()
		    || from.par()->isSeparator(from.pos())
		    || from.par()->isKomma(from.pos())
		    || from.par()->isNewline(from.pos())
		    || from.par()->isSeparator(from.pos() - 1)
		    || from.par()->isKomma(from.pos() - 1)
		    || from.par()->isNewline(from.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case lyx::WHOLE_WORD:
		// Move cursor to the beginning, when not already there.
		if (from.pos() && !from.par()->isSeparator(from.pos() - 1)
		    && !(from.par()->isKomma(from.pos() - 1)
			 || from.par()->isNewline(from.pos() - 1)))
			cursorLeftOneWord(from, pars);
		break;
	case lyx::PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		cursorLeftOneWord(from, pars);
		break;
	case lyx::NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet"
		       << endl;
		break;
	case lyx::PARTIAL_WORD:
		break;
	}
	to = from;
	while (to.pos() < to.par()->size()
	       && !to.par()->isSeparator(to.pos())
	       && !to.par()->isKomma(to.pos())
	       && !to.par()->isNewline(to.pos())
	       && !to.par()->isHfill(to.pos())
	       && !to.par()->isInset(to.pos()))
	{
		to.pos(to.pos() + 1);
	}
}
