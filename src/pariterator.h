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

#include "dociterator.h"

#include "support/types.h"

#include <boost/optional.hpp>

#include <vector>



class InsetBase;
class LyXText;
class ParagraphList;


class ParIterator : public std::iterator<std::forward_iterator_tag, Paragraph>,
		    public DocumentIterator
{
public:
	///
	ParIterator(InsetBase &, lyx::par_type pit);
	///
	ParIterator(ParIterator const &);
	///
	ParIterator(DocumentIterator const &);

	///
	void operator=(ParIterator const &);
	///
	ParIterator & operator++();
	///
	ParIterator & operator--();
	///
	Paragraph & operator*() const;
	///
	Paragraph * operator->() const;
	/// This gives us the top-most parent paragraph
	lyx::par_type outerPar() const;
	///
	lyx::par_type pit() const;
	///
	ParagraphList & plist() const;
};


DocumentIterator makeDocumentIterator(ParIterator const &, lyx::pos_type);




///
bool operator==(ParIterator const & iter1, ParIterator const & iter2);

///
bool operator!=(ParIterator const & iter1, ParIterator const & iter2);


class ParConstIterator : public std::iterator<std::forward_iterator_tag,
			 Paragraph>,
			 public DocumentIterator
{
public:
	///
	ParConstIterator(InsetBase const &, lyx::par_type pit);
	///
	ParConstIterator(ParConstIterator const &);
	///
	ParConstIterator(DocumentIterator const &);
	///

	ParConstIterator & operator++();
	///
	ParConstIterator & operator--();
	///
	Paragraph const & operator*() const;
	///
	Paragraph const * operator->() const;
	///
	ParagraphList const & plist() const;
};

bool operator==(ParConstIterator const & iter1,
		ParConstIterator const & iter2);

bool operator!=(ParConstIterator const & iter1,
		ParConstIterator const & iter2);

#endif
