// -*- C++ -*-
/* \file pariterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARITERATOR_H
#define PARITERATOR_H

#include "dociterator.h"

#include "support/types.h"

#include <boost/optional.hpp>

#include <vector>



class InsetBase;
class LyXText;
class ParagraphList;


class ParIterator : public std::iterator<std::forward_iterator_tag, Paragraph>,
		    public DocIterator
{
public:
	typedef std::iterator<std::forward_iterator_tag, Paragraph> StdIt;

	typedef StdIt::value_type value_type;
	typedef StdIt::difference_type difference_type;
	typedef StdIt::pointer pointer;
	typedef StdIt::reference reference;

	///
	ParIterator() : DocIterator() {}


	///
	ParIterator(InsetBase &, lyx::par_type pit);
	///
	ParIterator(ParIterator const &);
	///
	ParIterator(DocIterator const &);

	/// This really should be implemented...
	//void operator=(ParIterator const &);
	///
	ParIterator & operator++();
	///
	ParIterator operator++(int);
	/// See comment in pariterator.C
	//ParIterator & operator--();
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


DocIterator makeDocIterator(ParIterator const &, lyx::pos_type);

ParIterator par_iterator_begin(InsetBase & inset);

ParIterator par_iterator_end(InsetBase & inset);


///
bool operator==(ParIterator const & iter1, ParIterator const & iter2);

///
bool operator!=(ParIterator const & iter1, ParIterator const & iter2);


class ParConstIterator : public std::iterator<std::forward_iterator_tag,
			 Paragraph>,
			 public DocIterator
{
public:
	///
	ParConstIterator(ParConstIterator const &);
	///
	ParConstIterator(DocIterator const &);
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


ParConstIterator par_const_iterator_begin(InsetBase const & inset);

ParConstIterator par_const_iterator_end(InsetBase const & inset);


#endif
