// -*- C++ -*-
/* \file iterators.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ITERATORS_H
#define ITERATORS_H

#include "ParagraphList_fwd.h"
#include "InsetList.h"

#include "support/types.h"

#include <boost/optional.hpp>

#include <vector>

class LyXText;
class InsetBase;
class Cursor;
class Buffer;
class PosIterator;


class ParPosition {
public:
	///
	ParPosition(ParagraphList::iterator p, ParagraphList const & pl);
	///
	ParagraphList::iterator pit;
	///
	ParagraphList const * plist;
	///
	boost::optional<InsetList::iterator> it;
	///
	boost::optional<int> index;
};


class ParIterator  : public std::iterator<
	std::forward_iterator_tag,
	ParagraphList::value_type> {
public:
	///
	ParIterator(ParagraphList::iterator pit, ParagraphList const & pl);
	///
	~ParIterator();
	///
	ParIterator(ParIterator const &);
	///
	ParIterator(PosIterator const &);
	///
	void operator=(ParIterator const &);
	///
	ParIterator & operator++();
	///
	Paragraph & operator*() const;
	///
	ParagraphList::iterator operator->() const;
	/// This gives us the top-most parent paragraph
	ParagraphList::iterator outerPar() const;
	///
	ParagraphList::iterator pit() const;
	///
	ParagraphList & plist() const;
	/// returns 'innermost' LyXText
	LyXText * text(Buffer &) const;
	/// returns innermost inset
	InsetBase * inset() const;
	/// returns index of cell in innermost inset
	int index() const;
	///
	size_t size() const;

	typedef std::vector<ParPosition> PosHolder;
	PosHolder const & positions() const
	{
		return positions_;
	}
private:
	PosHolder positions_;
};

///
bool operator==(ParIterator const & iter1, ParIterator const & iter2);

///
bool operator!=(ParIterator const & iter1, ParIterator const & iter2);


class ParConstIterator : public std::iterator<
	std::forward_iterator_tag,
	ParagraphList::value_type> {
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
	ParagraphList::const_iterator pit() const;
	///
	Paragraph const & operator*() const;
	///
	ParagraphList::const_iterator operator->() const;
	///
	ParagraphList const & plist() const;

	/// depth of nesting
	size_t size() const;
	typedef std::vector<ParPosition> PosHolder;
	PosHolder const & positions() const
	{
		return positions_;
	}
private:
	PosHolder positions_;
};

bool operator==(ParConstIterator const & iter1,
		ParConstIterator const & iter2);

bool operator!=(ParConstIterator const & iter1,
		ParConstIterator const & iter2);

#endif
