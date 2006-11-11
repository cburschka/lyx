/**
 * \file toc.C
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

#include "buffer.h"
#include "bufferparams.h"
#include "funcrequest.h"
#include "lyxtext.h"
#include "LyXAction.h"
#include "paragraph.h"
#include "pariterator.h"
#include "cursor.h"
#include "debug.h"
#include "undo.h"


namespace lyx {
namespace toc {

void outline(OutlineOp mode,  LCursor & cur)
{
	recordUndo(cur);
	Buffer * buf = & cur.buffer();
	pit_type & pit = cur.pit();
	ParagraphList & pars = buf->text().paragraphs();
	ParagraphList::iterator bgn = pars.begin();
	ParagraphList::iterator s = boost::next(bgn, pit);
	ParagraphList::iterator p = s;
	ParagraphList::iterator end = pars.end();

	LyXTextClass::const_iterator lit =
		buf->params().getLyXTextClass().begin();
	LyXTextClass::const_iterator const lend =
		buf->params().getLyXTextClass().end();

	int const thistoclevel = s->layout()->toclevel;
	int toclevel;
	switch (mode) {
		case Up: {
			if (p != end)
				++p;
			for (; p != end; ++p) {
				toclevel = p->layout()->toclevel;
				if (toclevel != LyXLayout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			ParagraphList::iterator q = s;
			if (q != bgn)
				--q;
			else
				break;
			for (; q != bgn; --q) {
				toclevel = q->layout()->toclevel;
				if (toclevel != LyXLayout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			pit_type const newpit = std::distance(pars.begin(), q);
			pit_type const len = std::distance(s, p);
			pit += len;
			pars.insert(q, s, p);
			s = boost::next(pars.begin(), pit);
			ParagraphList::iterator t = boost::next(s, len);
			pit = newpit;
			pars.erase(s, t);
		break;
		}
		case Down: {
			   if (p != end)
				++p;
			for (; p != end; ++p) {
				toclevel = p->layout()->toclevel;
				if (toclevel != LyXLayout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			ParagraphList::iterator q = p;
			if (q != end)
				++q;
			else
				break;
			for (; q != end; ++q) {
				toclevel = q->layout()->toclevel;
				if (toclevel != LyXLayout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			pit_type const newpit = std::distance(pars.begin(), q);
			pit_type const len = std::distance(s, p);
			pars.insert(q, s, p);
			s = boost::next(pars.begin(), pit);
			ParagraphList::iterator t = boost::next(s, len);
			pit = newpit - len;
			pars.erase(s, t);
		break;
		}
		case In:
			for (; lit != lend; ++lit) {
				if ((*lit)->toclevel == thistoclevel + 1 &&
				    s->layout()->labeltype == (*lit)->labeltype) {
					s->layout((*lit));
					break;
				}
			}
		break;
		case Out:
			for (; lit != lend; ++lit) {
				if ((*lit)->toclevel == thistoclevel - 1 &&
				    s->layout()->labeltype == (*lit)->labeltype) {
					s->layout((*lit));
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
