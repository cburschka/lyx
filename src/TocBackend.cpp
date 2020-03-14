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
#include "IndicesList.h"
#include "InsetList.h"
#include "Paragraph.h"
#include "TextClass.h"

#include "insets/InsetArgument.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
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
                 bool output_active, FuncRequest const & action)
	: dit_(dit), depth_(d), str_(s), output_(output_active),
	  action_(action)
{
}


int TocItem::id() const
{
	return dit_.paragraph().id();
}


docstring const TocItem::asString() const
{
	static char_type const cross = 0x2716; // ✖ U+2716 HEAVY MULTIPLICATION X
	static char_type const thin = 0x2009; // U+2009 THIN SPACE
	docstring prefix;
	if (!output_) {
		prefix += cross;
		prefix += thin;
	}
	return prefix + str_;
}


FuncRequest TocItem::action() const
{
	if (action_.action() == LFUN_UNKNOWN_ACTION) {
		return FuncRequest(LFUN_PARAGRAPH_GOTO, dit_.paragraphGotoArgument());
	} else
		return action_;
}


///////////////////////////////////////////////////////////////////////////
//
// TocBackend implementation
//
///////////////////////////////////////////////////////////////////////////

Toc::const_iterator TocBackend::findItem(Toc const & toc,
                                         DocIterator const & dit)
{
	Toc::const_iterator last = toc.begin();
	Toc::const_iterator it = toc.end();
	if (it == last)
		return it;
	--it;
	DocIterator dit_text = dit.getInnerText();

	for (; it != last; --it) {
		// We verify that we don't compare contents of two
		// different document. This happens when you
		// have parent and child documents.
		if (&it->dit()[0].inset() != &dit_text[0].inset())
			continue;
		if (it->dit() <= dit_text)
			return it;
	}

	// We are before the first Toc Item:
	return last;
}


Toc::iterator TocBackend::findItem(Toc & toc, int depth, docstring const & str)
{
	if (toc.empty())
		return toc.end();
	Toc::iterator it = toc.begin();
	Toc::iterator itend = toc.end();
	for (; it != itend; ++it) {
		if (it->depth() == depth && it->str() == str)
			break;
	}
	return it;
}


shared_ptr<Toc const> TocBackend::toc(string const & type) const
{
	// Is the type already supported?
	TocList::const_iterator it = tocs_.find(type);
	LASSERT(it != tocs_.end(), { return make_shared<Toc>(); });
	return it->second;
}


shared_ptr<Toc> TocBackend::toc(string const & type)
{
	// std::map::insert only really performs the insertion if the key is not
	// already bound, and otherwise returns an iterator to the element already
	// there, see manual.
	return tocs_.insert({type, make_shared<Toc>()}).first->second;
}


TocBuilder & TocBackend::builder(string const & type)
{
	auto p = make_unique<TocBuilder>(toc(type));
	return * builders_.insert(make_pair(type, move(p))).first->second;
}


// FIXME: This function duplicates functionality from InsetText::iterateForToc.
// Both have their own way of computing the TocItem for "tableofcontents". The
// TocItem creation and update should be made in a dedicated function and
// updateItem should be rewritten to uniformly update the matching items from
// all TOCs.
bool TocBackend::updateItem(DocIterator const & dit_in)
{
	// we need a text
	DocIterator dit = dit_in.getInnerText();

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

	Toc::const_iterator toc_item = item("tableofcontents", dit);

	docstring tocstring;

	// For each paragraph, traverse its insets and let them add
	// their toc items
	//
	// FIXME: This is supposed to accomplish the same as the body of
	// InsetText::iterateForToc(), probably
	Paragraph & par = toc_item->dit().paragraph();
	for (auto const & table : par.insetList())
		if (InsetArgument const * arg = table.inset->asInsetArgument()) {
			tocstring = par.labelString();
			if (!tocstring.empty())
				tocstring += ' ';
			arg->text().forOutliner(tocstring,TOC_ENTRY_LENGTH);
			break;
		}

	int const toclevel = toc_item->dit().text()->
		getTocLevel(toc_item->dit().pit());
	if (toclevel != Layout::NOT_IN_TOC && toclevel >= min_toclevel
		&& tocstring.empty())
		par.forOutliner(tocstring, TOC_ENTRY_LENGTH);

	support::truncateWithEllipsis(tocstring, TOC_ENTRY_LENGTH);
	const_cast<TocItem &>(*toc_item).str(tocstring);

	buffer_->updateTocItem("tableofcontents", dit);
	return true;
}


void TocBackend::reset()
{
	for (auto const & t: tocs_)
		t.second->clear();
	tocs_.clear();
	builders_.clear();
	resetOutlinerNames();
}


void TocBackend::update(bool output_active, UpdateType utype)
{
	reset();
	if (buffer_->isInternal())
		return;

	DocIterator dit;
	buffer_->inset().addToToc(dit, output_active, utype, *this);
}


Toc::const_iterator TocBackend::item(string const & type,
                                     DocIterator const & dit) const
{
	TocList::const_iterator toclist_it = tocs_.find(type);
	// Is the type supported?
	// We will try to make the best of it in release mode
	LASSERT(toclist_it != tocs_.end(), toclist_it = tocs_.begin());
	return findItem(*toclist_it->second, dit);
}


void TocBackend::writePlaintextTocList(string const & type,
        odocstringstream & os, size_t max_length) const
{
	TocList::const_iterator cit = tocs_.find(type);
	if (cit != tocs_.end()) {
		Toc::const_iterator ccit = cit->second->begin();
		Toc::const_iterator end = cit->second->end();
		for (; ccit != end; ++ccit) {
			os << ccit->asString() << from_utf8("\n");
			if (os.str().size() > max_length)
				break;
		}
	}
}


docstring TocBackend::outlinerName(string const & type) const
{
	map<string, docstring>::const_iterator const it
		= outliner_names_.find(type);
	if (it != outliner_names_.end())
		return it->second;

	// Legacy treatment of index:... type
	if (support::prefixIs(type, "index:")) {
		string const itype = support::split(type, ':');
		IndicesList const & indiceslist = buffer_->params().indiceslist();
		Index const * index = indiceslist.findShortcut(from_utf8(itype));
		docstring indextype = _("unknown type!");
		if (index)
			indextype = index->index();
		return support::bformat(_("Index Entries (%1$s)"), indextype);
	}

	LYXERR0("Missing OutlinerName for " << type << "!");
	return from_utf8(type);
}


void TocBackend::resetOutlinerNames()
{
	outliner_names_.clear();
	// names from this document class
	for (auto const & name
		     : buffer_->params().documentClass().outlinerNames())
		addName(name.first, translateIfPossible(name.second));
	// Hardcoded types
	addName("tableofcontents", _("Table of Contents"));
	addName("change", _("Changes"));
	addName("senseless", _("Senseless"));
	addName("citation", _("Citations"));
	addName("label", _("Labels and References"));
	addName("brokenrefs", _("Broken References and Citations"));
	// Customizable, but the corresponding insets have no layout definition
	addName("child", _("Child Documents"));
	addName("graphics", _("Graphics"));
	addName("equation", _("Equations"));
	addName("external", _("External Material"));
	addName("math-macro", _("Math Macros"));
	addName("nomencl", _("Nomenclature Entries"));
}


void TocBackend::addName(string const & type, docstring const & name)
{
	if (name.empty())
		return;
	// only inserts if the key does not exist
	outliner_names_.insert({type, name});
}


bool TocBackend::isOther(std::string const & type)
{
	return type == "graphics"
		|| type == "note"
		|| type == "branch"
		|| type == "change"
		|| type == "label"
		|| type == "citation"
		|| type == "equation"
		|| type == "footnote"
		|| type == "marginalnote"
		|| type == "nomencl"
		|| type == "listings"
		|| type == "math-macro"
		|| type == "external"
		|| type == "senseless"
		|| type == "index"
		|| type.substr(0,6) == "index:";
}


} // namespace lyx
