// -*- C++ -*-
/* \file iterators.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef ITERATORS_H
#define ITERATORS_H

#include "ParagraphList.h"

#include <boost/scoped_ptr.hpp>

class ParIterator {
public:
	///
	ParIterator(ParagraphList::iterator pit, ParagraphList const & pl);
	///
	~ParIterator();
	///
	ParIterator(ParIterator const &);
	///
	void operator=(ParIterator const &);
	///
	ParIterator & operator++();
	///
	ParagraphList::iterator operator*() const;
	///
	ParagraphList::iterator operator->() const;
	///
	ParagraphList::iterator outerPar() const;
	///
	ParagraphList & plist() const;
	///
	size_t size() const;
	///
	friend
	bool operator==(ParIterator const & iter1, ParIterator const & iter2);
private:
	struct Pimpl;
	boost::scoped_ptr<Pimpl> pimpl_;
};

///
bool operator==(ParIterator const & iter1, ParIterator const & iter2);

///
bool operator!=(ParIterator const & iter1, ParIterator const & iter2);


class ParConstIterator {
public:
	///
	ParConstIterator(ParagraphList::iterator pit, ParagraphList const & pl);
	///
	~ParConstIterator();
	///
	ParConstIterator(ParConstIterator const &);
	///
	ParConstIterator & operator++();
	///
	ParagraphList::iterator operator*() const;
	///
	ParagraphList::iterator operator->() const;

	///
	size_t size() const;
	///
	friend
	bool operator==(ParConstIterator const & iter1,
			ParConstIterator const & iter2);
private:
	struct Pimpl;
	boost::scoped_ptr<Pimpl> pimpl_;
};

bool operator==(ParConstIterator const & iter1,
		ParConstIterator const & iter2);

bool operator!=(ParConstIterator const & iter1,
		ParConstIterator const & iter2);

#endif
