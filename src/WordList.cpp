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
#include "support/lassert.h"
#include "support/weighted_btree.h"

#include <QThreadStorage>

#include <map>

using namespace std;

namespace lyx {

///
typedef map<string, WordList *> GlobalWordList;
// Each thread uses its own word list, but only the one of the GUI thread is
// used to do real work. The others are only neded to prevent simultanous
// write access e.g. from a cloned buffer and a true document buffer.
QThreadStorage<GlobalWordList *> theGlobalWordList;


WordList * theWordList(string const & lang)
{
	if (!theGlobalWordList.hasLocalData())
		theGlobalWordList.setLocalData(new GlobalWordList);
	GlobalWordList * globalWordList = theGlobalWordList.localData();
	GlobalWordList::iterator it = globalWordList->find(lang);
	if (it != globalWordList->end())
		return it->second;
	else {
		WordList * wl = new WordList;
		(*globalWordList)[lang] = wl;
		return wl;
	}
}


void WordList::cleanupWordLists()
{
	if (!theGlobalWordList.hasLocalData())
		return;
	GlobalWordList * globalWordList = theGlobalWordList.localData();
	GlobalWordList::const_iterator it = globalWordList->begin();
	for (; it != globalWordList->end(); ++it)
		delete it->second;
	globalWordList->clear();
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
	LASSERT(it != d->words_.end(), { static docstring dummy; return dummy; });
	
	// We use the key() method here, and not something like it->first
	// because the btree only returns (iterator-) temporary value pairs.
	// If we returned the first component of those here, we get an
	// invalid reference and therefore strange crashes.
	return it.key();
}


size_t WordList::size() const
{
	return d->words_.summed_weight();
}


void WordList::insert(docstring const & w)
{
	Impl::Words::iterator it = d->words_.find(w);
	if (it == d->words_.end())
		d->words_.insert(w, size_t(1), 1);
	else {
		it.data()++;
		d->words_.change_weight(it, 1);
	}
}


void WordList::remove(docstring const & w)
{
	Impl::Words::iterator it = d->words_.find(w);
	if (it != d->words_.end()) {
		it.data()--;
		d->words_.change_weight(it, 0);
		// We will not erase here, but instead we just leave it
		// in the btree with weight 0. This avoid too much
		// reorganisation of the tree all the time.
		//if (it.data() == 0)
		//	d->words_.erase(w);
	}
}

} // namespace lyx
