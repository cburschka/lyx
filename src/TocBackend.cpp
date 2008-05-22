/**
 * \file TocBackend.cpp
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

#include "TocBackend.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "InsetList.h"
#include "Layout.h"
#include "LyXAction.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "TextClass.h"

#include "insets/InsetOptArg.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"

#include "support/lassert.h"

using namespace std;


namespace lyx {

///////////////////////////////////////////////////////////////////////////
//
// TocItem implementation
//
///////////////////////////////////////////////////////////////////////////

TocItem::TocItem(DocIterator const & dit, int d, docstring const & s)
	: dit_(dit), depth_(d), str_(s)
{
}


int TocItem::id() const
{
	return dit_.paragraph().id();
}


int TocItem::depth() const
{
	return depth_;
}


docstring const & TocItem::str() const
{
	return str_;
}


docstring const TocItem::asString() const
{
	return docstring(4 * depth_, ' ') + str_;
}


FuncRequest TocItem::action() const
{
	string const arg = convert<string>(dit_.paragraph().id())
		+ ' ' + convert<string>(dit_.pos());
	return FuncRequest(LFUN_PARAGRAPH_GOTO, arg);
}


///////////////////////////////////////////////////////////////////////////
//
// TocBackend implementation
//
///////////////////////////////////////////////////////////////////////////

Toc const & TocBackend::toc(string const & type) const
{
	// Is the type already supported?
	TocList::const_iterator it = tocs_.find(type);
	LASSERT(it != tocs_.end(), /**/);

	return it->second;
}


Toc & TocBackend::toc(string const & type)
{
	return tocs_[type];
}


void TocBackend::updateItem(DocIterator const & dit)
{
	if (toc("tableofcontents").empty()) {
		// FIXME: should not happen, 
		// a call to TocBackend::update() is missing somewhere
		LYXERR0("TocBackend::updateItem called but the TOC is empty!");
		return;
	}

	BufferParams const & bufparams = buffer_->params();
	const int min_toclevel = bufparams.documentClass().min_toclevel();

	TocIterator toc_item = item("tableofcontents", dit);

	docstring tocstring;

	// For each paragraph, traverse its insets and let them add
	// their toc items
	Paragraph & par = toc_item->dit_.paragraph();
	InsetList::const_iterator it = par.insetList().begin();
	InsetList::const_iterator end = par.insetList().end();
	for (; it != end; ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == OPTARG_CODE) {
			if (!tocstring.empty())
				break;
			Paragraph const & inset_par =
				*static_cast<InsetOptArg&>(inset).paragraphs().begin();
			if (!par.labelString().empty())
				tocstring = par.labelString() + ' ';
			tocstring += inset_par.asString();
			break;
		}
	}

	int const toclevel = par.layout().toclevel;
	if (toclevel != Layout::NOT_IN_TOC && toclevel >= min_toclevel
		&& tocstring.empty())
			tocstring = par.asString(AS_STR_LABEL);

	const_cast<TocItem &>(*toc_item).str_ = tocstring;
}


void TocBackend::update()
{
	tocs_.clear();

	BufferParams const & bufparams = buffer_->params();
	const int min_toclevel = bufparams.documentClass().min_toclevel();

	Toc & toc = tocs_["tableofcontents"];
	ParConstIterator pit = buffer_->par_iterator_begin();
	ParConstIterator end = buffer_->par_iterator_end();
	for (; pit != end; ++pit) {

		// the string that goes to the toc (could be the optarg)
		docstring tocstring;

		// For each paragraph, traverse its insets and let them add
		// their toc items
		InsetList::const_iterator it = pit->insetList().begin();
		InsetList::const_iterator end = pit->insetList().end();
		for (; it != end; ++it) {
			Inset & inset = *it->inset;
			pit.pos() = it->pos;
			//lyxerr << (void*)&inset << " code: " << inset.lyxCode() << std::endl;
			inset.addToToc(pit);
			switch (inset.lyxCode()) {
			case OPTARG_CODE: {
				if (!tocstring.empty())
					break;
				pit.pos() = 0;
				Paragraph const & par =
					*static_cast<InsetOptArg&>(inset).paragraphs().begin();
				if (!pit->labelString().empty())
					tocstring = pit->labelString() + ' ';
				tocstring += par.asString();
				break;
			}
			default:
				break;
			}
		}

		/// now the toc entry for the paragraph
		int const toclevel = pit->layout().toclevel;
		if (toclevel != Layout::NOT_IN_TOC
		    && toclevel >= min_toclevel) {
			pit.pos() = 0;
			// insert this into the table of contents
			if (tocstring.empty())
				tocstring = pit->asString(AS_STR_LABEL);
			toc.push_back(TocItem(pit, toclevel - min_toclevel,
				tocstring));
		}
	}
}


TocIterator TocBackend::item(string const & type,
		DocIterator const & dit) const
{
	TocList::const_iterator toclist_it = tocs_.find(type);
	// Is the type supported?
	LASSERT(toclist_it != tocs_.end(), /**/);

	Toc const & toc_vector = toclist_it->second;
	TocIterator last = toc_vector.begin();
	TocIterator it = toc_vector.end();
	if (it == last)
		return it;

	--it;

	DocIterator dit_text = dit;
	if (dit_text.inMathed()) {
		// We are only interested in text so remove the math CursorSlice.
		while (dit_text.inMathed())
			dit_text.pop_back();
	}

	for (; it != last; --it) {
		// We verify that we don't compare contents of two
		// different document. This happens when you
		// have parent and child documents.
		if (&it->dit_[0].inset() != &dit_text[0].inset())
			continue;
		if (it->dit_ <= dit_text)
			return it;
	}

	// We are before the first Toc Item:
	return last;
}


void TocBackend::writePlaintextTocList(string const & type, odocstream & os) const
{
	TocList::const_iterator cit = tocs_.find(type);
	if (cit != tocs_.end()) {
		TocIterator ccit = cit->second.begin();
		TocIterator end = cit->second.end();
		for (; ccit != end; ++ccit)
			os << ccit->asString() << from_utf8("\n");
	}
}


} // namespace lyx
