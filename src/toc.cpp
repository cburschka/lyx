/**
 * \file toc.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "toc.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "Text.h"
#include "LyXAction.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "Cursor.h"
#include "debug.h"
#include "Undo.h"


namespace lyx {
namespace toc {

void outline(OutlineOp mode,  Cursor & cur)
{
	Buffer & buf = cur.buffer();
	pit_type & pit = cur.pit();
	ParagraphList & pars = buf.text().paragraphs();
	ParagraphList::iterator bgn = pars.begin();
	// The first paragraph of the area to be copied:
	ParagraphList::iterator start = boost::next(bgn, pit);
	// The final paragraph of area to be copied:
	ParagraphList::iterator finish = start;
	ParagraphList::iterator end = pars.end();

	TextClass::const_iterator lit =
		buf.params().getTextClass().begin();
	TextClass::const_iterator const lend =
		buf.params().getTextClass().end();

	int const thistoclevel = start->layout()->toclevel;
	int toclevel;
	switch (mode) {
		case Up: {
			// Move out (down) from this section header
			if (finish != end)
				++finish;
			// Seek the one (on same level) below
			for (; finish != end; ++finish) {
				toclevel = finish->layout()->toclevel;
				if (toclevel != Layout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			ParagraphList::iterator dest = start;
			// Move out (up) from this header
			if (dest == bgn)
				break;
			// Search previous same-level header above
			do {
				--dest;
				toclevel = dest->layout()->toclevel;
			} while(dest != bgn
				&& (toclevel == Layout::NOT_IN_TOC
				    || toclevel > thistoclevel));
			// Not found; do nothing
			if (toclevel == Layout::NOT_IN_TOC
			    || toclevel > thistoclevel)
				break;
			pit_type const newpit = std::distance(bgn, dest);
			pit_type const len = std::distance(start, finish);
			pit_type const deletepit = pit + len;
			recordUndo(cur, Undo::ATOMIC, newpit, deletepit - 1);
			pars.insert(dest, start, finish);
			start = boost::next(pars.begin(), deletepit);
			pit = newpit;
			pars.erase(start, finish);
		break;
		}
		case Down: {
			// Go down out of current header:
			if (finish != end)
				++finish;
			// Find next same-level header:
			for (; finish != end; ++finish) {
				toclevel = finish->layout()->toclevel;
				if (toclevel != Layout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			ParagraphList::iterator dest = finish;
			// Go one down from *this* header:
			if (dest != end)
				++dest;
			else
				break;
			// Go further down to find header to insert in front of:
			for (; dest != end; ++dest) {
				toclevel = dest->layout()->toclevel;
				if (toclevel != Layout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			// One such was found:
			pit_type newpit = std::distance(bgn, dest);
			pit_type const len = std::distance(start, finish);
			recordUndo(cur, Undo::ATOMIC, pit, newpit - 1);
			pars.insert(dest, start, finish);
			start = boost::next(bgn, pit);
			pit = newpit - len;
			pars.erase(start, finish);
		break;
		}
		case In:
			recordUndo(cur);
			for (; lit != lend; ++lit) {
				if ((*lit)->toclevel == thistoclevel + 1 &&
				    start->layout()->labeltype == (*lit)->labeltype) {
					start->layout((*lit));
					break;
				}
			}
		break;
		case Out:
			recordUndo(cur);
			for (; lit != lend; ++lit) {
				if ((*lit)->toclevel == thistoclevel - 1 &&
				    start->layout()->labeltype == (*lit)->labeltype) {
					start->layout((*lit));
					break;
				}
			}
		break;
		default:
		break;
	}
}


} // namespace toc
} // namespace lyx
