/**
 * \file TocBackend.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 * \author Guillaume Munch
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

#include "insets/InsetArgument.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/lassert.h"
#include "support/lstrings.h"

using namespace std;


namespace lyx {


///////////////////////////////////////////////////////////////////////////
//
// TocItem implementation
//
///////////////////////////////////////////////////////////////////////////

TocItem::TocItem(DocIterator const & dit, int d, docstring const & s,
			bool output_active, docstring const & t, FuncRequest action) :
	dit_(dit), depth_(d), str_(s), tooltip_(t), output_(output_active),
	action_(action)
{
}


int TocItem::id() const
{
	return dit_.paragraph().id();
}


docstring const & TocItem::tooltip() const
{
	return tooltip_.empty() ? str_ : tooltip_;
}


docstring const TocItem::asString() const
{
	// U+2327 X IN A RECTANGLE BOX
	// char_type const cross = 0x2327;
	// U+274E NEGATIVE SQUARED CROSS MARK
	char_type const cross = 0x274e;
	docstring prefix;
	if (!output_) {
		prefix += cross;
		prefix += " ";
	}
	return prefix + str_;
}

namespace {

// convert a DocIterator into an argument to LFUN_PARAGRAPH_GOTO 
docstring paragraph_goto_arg(DocIterator const & dit)
{
	CursorSlice const & s = dit.innerTextSlice();
	return convert<docstring>(s.paragraph().id()) + ' ' +
		convert<docstring>(s.pos());
}

} // namespace anon

FuncRequest TocItem::action() const
{
	if (action_.action() == LFUN_UNKNOWN_ACTION) {
		return FuncRequest(LFUN_PARAGRAPH_GOTO, paragraph_goto_arg(dit_));
	} else
		return action_;
}


///////////////////////////////////////////////////////////////////////////
//
// Toc implementation
//
///////////////////////////////////////////////////////////////////////////

TocIterator Toc::item(DocIterator const & dit) const
{
	TocIterator last = begin();
	TocIterator it = end();
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


Toc::iterator Toc::item(int depth, docstring const & str)
{
	if (empty())
		return end();
	iterator it = begin();
	iterator itend = end();
	for (; it != itend; ++it) {
		if (it->depth() == depth && it->str() == str)
			break;
	}
	return it;
}


///////////////////////////////////////////////////////////////////////////
//
// TocBuilder implementation
//
///////////////////////////////////////////////////////////////////////////

TocBuilder::TocBuilder(shared_ptr<Toc> toc)
	: toc_(toc ? toc : lyx::make_shared<Toc>()),
	  stack_()
{
	LATTEST(toc);
}

void TocBuilder::pushItem(DocIterator const & dit, docstring const & s,
						  bool output_active, bool is_captioned)
{
	toc_->push_back(TocItem(dit, stack_.size(), s, output_active));
	frame f = {
		toc_->size() - 1, //pos
		is_captioned, //is_captioned
	};
	stack_.push(f);
}

void TocBuilder::captionItem(DocIterator const & dit, docstring const & s,
							 bool output_active)
{
	// first show the float before moving to the caption
	docstring arg = "paragraph-goto " + paragraph_goto_arg(dit);
	if (!stack_.empty())
		arg = "paragraph-goto " +
			paragraph_goto_arg((*toc_)[stack_.top().pos].dit_) + ";" + arg;
	FuncRequest func(LFUN_COMMAND_SEQUENCE, arg);
	
	if (!stack_.empty() && !stack_.top().is_captioned) {
		// The float we entered has not yet been assigned a caption.
		// Assign the caption string to it.
		TocItem & captionable = (*toc_)[stack_.top().pos];
		captionable.str(s);
		captionable.setAction(func);
		stack_.top().is_captioned = true;
	} else {
		// This is a new entry.
		pop();
		// the dit is at the float's level, e.g. for the contextual menu of
		// outliner entries
		DocIterator captionable_dit = dit;
		captionable_dit.pop_back();
		pushItem(captionable_dit, s, output_active, true);
		(*toc_)[stack_.top().pos].setAction(func);
	}
}

void TocBuilder::pop()
{
	if (!stack_.empty())
		stack_.pop();
}



///////////////////////////////////////////////////////////////////////////
//
// TocBuilderStore implementation
//
///////////////////////////////////////////////////////////////////////////

shared_ptr<TocBuilder> TocBuilderStore::get(string const & type,
											shared_ptr<Toc> toc)
{
	map_t::const_iterator it = map_.find(type);
	if (it == map_.end()) {
		it = map_.insert(std::make_pair(type,
									lyx::make_shared<TocBuilder>(toc))).first;
	}
	return it->second;
}



///////////////////////////////////////////////////////////////////////////
//
// TocBackend implementation
//
///////////////////////////////////////////////////////////////////////////

shared_ptr<Toc const> TocBackend::toc(string const & type) const
{
	// Is the type already supported?
	TocList::const_iterator it = tocs_.find(type);
	LASSERT(it != tocs_.end(), { return lyx::make_shared<Toc>(); });
	return it->second;
}


shared_ptr<Toc> TocBackend::toc(string const & type)
{
	TocList::const_iterator it = tocs_.find(type);
	if (it == tocs_.end()) {
		it = tocs_.insert(std::make_pair(type, lyx::make_shared<Toc>())).first;
	}
	return it->second;
}


shared_ptr<TocBuilder> TocBackend::builder(string const & type)
{
	return builders_.get(type, toc(type));
}


// FIXME: This function duplicates functionality from InsetText::iterateForToc.
// Both have their own way of computing the TocItem for "tableofcontents". The
// TocItem creation and update should be made in a dedicated function and
// updateItem should be rewritten to uniformly update the matching items from
// all TOCs.
bool TocBackend::updateItem(DocIterator const & dit)
{
	if (dit.text()->getTocLevel(dit.pit()) == Layout::NOT_IN_TOC)
		return false;

	if (toc("tableofcontents")->empty()) {
		// FIXME: should not happen, 
		// a call to TocBackend::update() is missing somewhere
		LYXERR0("TocBackend::updateItem called but the TOC is empty!");
		return false;
	}

	BufferParams const & bufparams = buffer_->params();
	const int min_toclevel = bufparams.documentClass().min_toclevel();

	TocIterator toc_item = item("tableofcontents", dit);

	docstring tocstring;

	// For each paragraph, traverse its insets and let them add
	// their toc items
	//
	// FIXME: This is supposed to accomplish the same as the body of
	// InsetText::iterateForToc(), probably
	Paragraph & par = toc_item->dit_.paragraph();
	InsetList::const_iterator it = par.insetList().begin();
	InsetList::const_iterator end = par.insetList().end();
	for (; it != end; ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == ARG_CODE) {
			tocstring = par.labelString();
			if (!tocstring.empty())
				tocstring += ' ';
			inset.asInsetText()->text().forOutliner(tocstring,TOC_ENTRY_LENGTH);
			break;
		}
	}

	int const toclevel = toc_item->dit_.text()->
		getTocLevel(toc_item->dit_.pit());
	if (toclevel != Layout::NOT_IN_TOC && toclevel >= min_toclevel
		&& tocstring.empty())
		par.forOutliner(tocstring, TOC_ENTRY_LENGTH);

	support::truncateWithEllipsis(tocstring, TOC_ENTRY_LENGTH);
	const_cast<TocItem &>(*toc_item).str(tocstring);

	buffer_->updateTocItem("tableofcontents", dit);
	return true;
}


void TocBackend::update(bool output_active, UpdateType utype)
{
	for (TocList::iterator it = tocs_.begin(); it != tocs_.end(); ++it)
		it->second->clear();
	tocs_.clear();
	builders_.clear();
	if (!buffer_->isInternal()) {
		DocIterator dit;
		buffer_->inset().addToToc(dit, output_active, utype);
	}
}


TocIterator TocBackend::item(string const & type,
		DocIterator const & dit) const
{
	TocList::const_iterator toclist_it = tocs_.find(type);
	// Is the type supported?
	// We will try to make the best of it in release mode
	LASSERT(toclist_it != tocs_.end(), toclist_it = tocs_.begin());
	return toclist_it->second->item(dit);
}


void TocBackend::writePlaintextTocList(string const & type,
        odocstringstream & os, size_t max_length) const
{
	TocList::const_iterator cit = tocs_.find(type);
	if (cit != tocs_.end()) {
		TocIterator ccit = cit->second->begin();
		TocIterator end = cit->second->end();
		for (; ccit != end; ++ccit) {
			os << ccit->asString() << from_utf8("\n");
			if (os.str().size() > max_length)
				break;
		}
	}
}


} // namespace lyx
