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
#include "FloatList.h"
#include "funcrequest.h"
#include "LyXAction.h"
#include "paragraph.h"
#include "cursor.h"
#include "debug.h"
#include "undo.h"

#include "insets/insetfloat.h"
#include "insets/insetoptarg.h"
#include "insets/insetwrap.h"

#include "support/convert.h"

#include <iostream>
#include <map>

using std::map;
using std::pair;
using std::make_pair;
using std::vector;
using std::max;
using std::ostream;
using std::string;
using std::endl;

namespace lyx {
namespace toc {

typedef map<Buffer const *, TocBackend> TocMap;
static TocMap toc_backend_;

///////////////////////////////////////////////////////////////////////////
// Interface to toc_backend_

void updateToc(Buffer const & buf)
{
	TocMap::iterator it = toc_backend_.find(&buf);
	if (it == toc_backend_.end()) {
		pair<TocMap::iterator, bool> result
			= toc_backend_.insert(make_pair(&buf, TocBackend(&buf)));
		if (!result.second)
			return;

		it = result.first;
	}

	it->second.update();
}


TocList const & getTocList(Buffer const & buf)
{
	return toc_backend_[&buf].tocs();
}


Toc const & getToc(Buffer const & buf, std::string const & type)
{
	return toc_backend_[&buf].toc(type);
}


TocIterator const getCurrentTocItem(Buffer const & buf, LCursor const & cur,
								std::string const & type)
{
	return toc_backend_[&buf].item(type, ParConstIterator(cur));
}


vector<string> const & getTypes(Buffer const & buf)
{
	return toc_backend_[&buf].types();
}


void asciiTocList(string const & type, Buffer const & buf, odocstream & os)
{
	toc_backend_[&buf].asciiTocList(type, os);
}

///////////////////////////////////////////////////////////////////////////
// Other functions

string const getType(string const & cmdName)
{
	// special case
	if (cmdName == "tableofcontents")
		return "TOC";
	else
		return cmdName;
}


string const getGuiName(string const & type, Buffer const & buffer)
{
	FloatList const & floats =
		buffer.params().getLyXTextClass().floats();
	if (floats.typeExist(type))
		return floats.getType(type).name();
	else
		return type;
}


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
