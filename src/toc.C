// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2002 The LyX Team.
 *
 * ======================================================
 *
 * \file toc.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 * \author Jean-Marc Lasgouttes <lasgouttes@freesurf.fr>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
#include "toc.h"
#include "buffer.h"
#include "frontends/LyXView.h"
#include "lyxfunc.h"
#include "LyXAction.h"
#include "paragraph.h"
#include "insets/insetfloat.h"
#include "debug.h"

using std::vector;
using std::max;
using std::endl;
using std::ostream;

namespace toc
{

string const TocItem::asString() const
{
	return string(4 * depth, ' ') + str;
}


void TocItem::goTo(LyXView & lv_) const
{
	string const tmp = tostr(par->id());
	lv_.getLyXFunc()->dispatch(FuncRequest(LFUN_GOTO_PARAGRAPH, tmp));
}


int TocItem::action() const
{
	return lyxaction.getPseudoAction(LFUN_GOTO_PARAGRAPH,
					 tostr(par->id()));
}


string const getType(string const & cmdName)
{
	// special case
	if (cmdName == "tableofcontents")
		return "TOC";
	else
		return cmdName;
}


TocList const getTocList(Buffer const * buf)
{
	TocList toclist;
	if (!buf)
		return toclist;
	Paragraph * par = buf->paragraph;

	LyXTextClass const & textclass = buf->params.getLyXTextClass();

	while (par) {
#ifdef WITH_WARNINGS
#warning bogus type (Lgb)
#endif
		char const labeltype = par->layout()->labeltype;

		if (labeltype >= LABEL_COUNTER_CHAPTER
		    && labeltype <= LABEL_COUNTER_CHAPTER + buf->params.tocdepth) {
				// insert this into the table of contents
			const int depth = max(0, labeltype - textclass.maxcounter());
			TocItem const item(par, depth,
					   par->asString(buf, true));
			toclist["TOC"].push_back(item);
		}

		// For each paragraph, traverse its insets and look for
		// FLOAT_CODE
		Paragraph::inset_iterator it = par->inset_iterator_begin();
		Paragraph::inset_iterator end =	par->inset_iterator_end();
		for (; it != end; ++it) {
			if ((*it)->lyxCode() == Inset::FLOAT_CODE) {
				InsetFloat * il =
					static_cast<InsetFloat*>(*it);
				il->addToToc(toclist, buf);
			}
		}

		par = par->next();
	}
	return toclist;
}


vector<string> const getTypes(Buffer const * buffer)
{
	vector<string> types;

	TocList const tmp = getTocList(buffer);

	TocList::const_iterator cit = tmp.begin();
	TocList::const_iterator end = tmp.end();

	for (; cit != end; ++cit) {
		types.push_back(cit->first);
	}

	return types;
}


void asciiTocList(string const & type, Buffer const * buffer, ostream & os)
{
	TocList const toc_list = getTocList(buffer);
	TocList::const_iterator cit = toc_list.find(type);
	if (cit != toc_list.end()) {
		Toc::const_iterator ccit = cit->second.begin();
		Toc::const_iterator end = cit->second.end();
		for (; ccit != end; ++ccit)
			os << ccit->asString() << '\n';
	}
}


} // namespace toc
