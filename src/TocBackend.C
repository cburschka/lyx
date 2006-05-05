/**
 * \file TocBackend.C
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

#include "frontends/LyXView.h"

#include "insets/insetfloat.h"
#include "insets/insetoptarg.h"
#include "insets/insetwrap.h"

#include "support/convert.h"

#include <iostream>

using std::vector;
using std::max;
using std::ostream;
using std::string;
using std::cout;
using std::endl;

namespace lyx {

///////////////////////////////////////////////////////////////////////////
// TocBackend::Item implementation

TocBackend::Item::Item(ParConstIterator const & par_it, int d,
					   std::string const & s)
		: par_it_(par_it), depth_(d), str_(s)
{
/*
	if (!uid_.empty())
		return;

	size_t pos = s.find(" ");
	if (pos == string::npos) {
		// Non labelled item
		uid_ = s;
		return;
	}

	string s2 = s.substr(0, pos);

	if (s2 == "Chapter" || s2 == "Part") {
		size_t pos2 = s.find(" ", pos + 1);
		if (pos2 == string::npos) {
			// Unnumbered Chapter?? This should not happen.
			uid_ = s.substr(pos + 1);
			return;
		}
		// Chapter or Part
		uid_ = s.substr(pos2 + 1);
		return;
	}
	// Numbered Item.
	uid_ = s.substr(pos + 1);
	*/
}

bool const TocBackend::Item::isValid() const
{
	return depth_ != -1;
}


int const TocBackend::Item::id() const
{
	return par_it_->id();
}


int const TocBackend::Item::depth() const
{
	return depth_;
}


std::string const & TocBackend::Item::str() const
{
	return str_;
}


string const TocBackend::Item::asString() const
{
	return string(4 * depth_, ' ') + str_;
}


void TocBackend::Item::goTo(LyXView & lv_) const
{
	string const tmp = convert<string>(id());
	lv_.dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
}

FuncRequest TocBackend::Item::action() const
{
	return FuncRequest(LFUN_PARAGRAPH_GOTO, convert<string>(id()));
}





///////////////////////////////////////////////////////////////////////////
// TocBackend implementation

TocBackend::Toc const & TocBackend::toc(std::string const & type)
{
	// Is the type already supported?
	TocList::const_iterator it = tocs_.find(type);
	BOOST_ASSERT(it != tocs_.end());

	return it->second;
}


bool TocBackend::addType(std::string const & type)
{
	// Is the type already supported?
	TocList::iterator toclist_it = tocs_.find(type);
	if (toclist_it != tocs_.end())
		return false;

	tocs_.insert(make_pair(type, Toc()));
	types_.push_back(type);

	return true;
}


void TocBackend::update()
{
	tocs_.clear();
	types_.clear();

	BufferParams const & bufparams = buffer_->params();
	const int min_toclevel = bufparams.getLyXTextClass().min_toclevel();

	ParConstIterator pit = buffer_->par_iterator_begin();
	ParConstIterator end = buffer_->par_iterator_end();
	for (; pit != end; ++pit) {

		// the string that goes to the toc (could be the optarg)
		string tocstring;

		// For each paragraph, traverse its insets and look for
		// FLOAT_CODE or WRAP_CODE
		InsetList::const_iterator it = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			switch (it->inset->lyxCode()) {
			case InsetBase::FLOAT_CODE:
				static_cast<InsetFloat*>(it->inset)
					->addToToc(tocs_, *buffer_);
				break;
			case InsetBase::WRAP_CODE:
				static_cast<InsetWrap*>(it->inset)
					->addToToc(tocs_, *buffer_);
				break;
			case InsetBase::OPTARG_CODE: {
				if (!tocstring.empty())
					break;
				Paragraph const & par = *static_cast<InsetOptArg*>(it->inset)->paragraphs().begin();
				if (!pit->getLabelstring().empty())
					tocstring = pit->getLabelstring()
						+ ' ';
				tocstring += par.asString(*buffer_, false);
				break;
			}
			default:
				break;
			}
		}

		/// now the toc entry for the paragraph
		int const toclevel = pit->layout()->toclevel;
		if (toclevel != LyXLayout::NOT_IN_TOC
		    && toclevel >= min_toclevel
		    && toclevel <= bufparams.tocdepth) {
			// insert this into the table of contents
			if (tocstring.empty())
				tocstring = pit->asString(*buffer_, true);
			Item const item(pit, toclevel - min_toclevel, tocstring);
			tocs_["TOC"].push_back(item);
			//cout << "item inserted str " << item.str()
			//	<< "  id " << item.id() << endl;
		}
	}

	TocList::iterator it = tocs_.begin();
	for (; it != tocs_.end(); ++it)
		types_.push_back(it->first);
}


TocBackend::TocIterator const TocBackend::item(std::string const & type, ParConstIterator const & par_it)
{
	TocList::iterator toclist_it = tocs_.find(type);
	// Is the type supported?
	BOOST_ASSERT(toclist_it != tocs_.end());

	Toc const & toc_vector = toclist_it->second;
	TocBackend::TocIterator last = toc_vector.begin();
	TocBackend::TocIterator it = toc_vector.end();
	--it;

	for (; it != last; --it) {
		
		// A good solution for Items inside insets would be to do:
		//
		//if (std::distance(it->par_it_, current) <= 0)
		//	return it;
		//
		// But for an unknown reason, std::distance(current, it->par_it_) always
		// returns  a positive value and std::distance(it->par_it_, current) takes forever...
		// So for now, we do:
		if (it->par_it_.pit() <= par_it.pit())
			return it;
	}

	// We are before the first Toc Item:
	return last;
}


void TocBackend::asciiTocList(string const & type, ostream & os) const
{
	TocList::const_iterator cit = tocs_.find(type);
	if (cit != tocs_.end()) {
		Toc::const_iterator ccit = cit->second.begin();
		Toc::const_iterator end = cit->second.end();
		for (; ccit != end; ++ccit)
			os << ccit->asString() << '\n';
	}
}


} // namespace lyx
