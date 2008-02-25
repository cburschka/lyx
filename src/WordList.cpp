/**
 * \file WordList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "WordList.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring.h"
#include "support/weighted_btree.h"

#include <boost/assert.hpp>

namespace lyx {

///
WordList theGlobalWordList;

WordList & theWordList()
{
	return theGlobalWordList;
}
	
///
struct WordList::Impl {
	///
	size_t c_;
	///
	typedef stx::weighted_btree<docstring, size_t, int> Words;
	///
	Words words_;
};


WordList::WordList()
{
	d = new Impl;
	d->c_ = 0;

#if 0
	for (size_t i = 1000000; i > 0; --i) {
		d->words_.insert("a" + convert<docstring>(i), size_t(1), stx::Void());
	}
#endif
}


WordList::~WordList()
{
	delete d;
}


docstring const & WordList::word(size_t idx) const
{
	Impl::Words::const_iterator it = d->words_.find_summed_weight(idx);
	BOOST_ASSERT(it != d->words_.end());
	return it->first;
}


size_t WordList::size() const
{
	return d->words_.size();
}


void WordList::insert(docstring const & w)
{
	Impl::Words::iterator it = d->words_.find(w);
	if (it == d->words_.end())
		d->words_.insert(w, size_t(1), 1);
	else
		it.data()++;
}


void WordList::remove(docstring const & w)
{
	Impl::Words::iterator it = d->words_.find(w);
	if (it != d->words_.end()) {
		it.data()--;
		if (it.data() == 0)
			d->words_.erase(w);
	}
}

} // namespace lyx
