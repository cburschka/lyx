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

#include "text_funcs.h"
#include "debug.h"
#include "lyxcursor.h"
#include "lyxtext.h"
#include "paragraph.h"

#include <boost/next_prior.hpp>

using lyx::pos_type;
using lyx::word_location;

using std::endl;


bool transposeChars(LyXText & text, LyXCursor const & cursor)
{
	ParagraphList::iterator tmppit = text.cursorPar();
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


void cursorLeftOneWord(LyXText & text,
	LyXCursor & cursor, ParagraphList const & pars)
{
	// treat HFills, floats and Insets as words

	ParagraphList::iterator pit = text.cursorPar();
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

	cursor.par(text.parOffset(pit));
	cursor.pos(pos);
}


void cursorRightOneWord(LyXText & text,
	LyXCursor & cursor, ParagraphList const & pars)
{
	// treat floats, HFills and Insets as words
	ParagraphList::iterator pit = text.cursorPar();
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

	cursor.par(text.parOffset(pit));
	cursor.pos(pos);
}


// Select current word. This depends on behaviour of
// CursorLeftOneWord(), so it is patched as well.
void getWord(LyXText & text, LyXCursor & from, LyXCursor & to,
	word_location const loc, ParagraphList const & pars)
{
	ParagraphList::iterator from_par = text.getPar(from);
	ParagraphList::iterator to_par = text.getPar(to);
	switch (loc) {
	case lyx::WHOLE_WORD_STRICT:
		if (from.pos() == 0 || from.pos() == from_par->size()
		    || from_par->isSeparator(from.pos())
		    || from_par->isKomma(from.pos())
		    || from_par->isNewline(from.pos())
		    || from_par->isSeparator(from.pos() - 1)
		    || from_par->isKomma(from.pos() - 1)
		    || from_par->isNewline(from.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case lyx::WHOLE_WORD:
		// Move cursor to the beginning, when not already there.
		if (from.pos() && !from_par->isSeparator(from.pos() - 1)
		    && !(from_par->isKomma(from.pos() - 1)
			 || from_par->isNewline(from.pos() - 1)))
			cursorLeftOneWord(text, from, pars);
		break;
	case lyx::PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		cursorLeftOneWord(text, from, pars);
		break;
	case lyx::NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet"
		       << endl;
		break;
	case lyx::PARTIAL_WORD:
		break;
	}
	to = from;
	while (to.pos() < to_par->size()
	       && !to_par->isSeparator(to.pos())
	       && !to_par->isKomma(to.pos())
	       && !to_par->isNewline(to.pos())
	       && !to_par->isHfill(to.pos())
	       && !to_par->isInset(to.pos()))
	{
		to.pos(to.pos() + 1);
	}
}
