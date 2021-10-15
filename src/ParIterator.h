// -*- C++ -*-
/* \file ParIterator.h
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

#include "DocIterator.h"

#include "support/types.h"


namespace lyx {

class Buffer;
class Inset;
class Paragraph;
class ParagraphList;


class ParIterator : public DocIterator
{
public:
	///
	ParIterator(Buffer * buf) : DocIterator(buf) {}
	///
	ParIterator(ParIterator const & pi) :
		DocIterator(DocIterator(pi)) {}
	///
	explicit ParIterator(DocIterator const & dit) :
		DocIterator(dit) {}

	/// This really should be implemented...
	//ParIterator & operator=(ParIterator const &);
	///
	ParIterator & operator++();
	///
	ParIterator operator++(int);
	/// See comment in ParIterator.cpp
	//ParIterator & operator--();
	///
	Paragraph & operator*() const;
	///
	Paragraph * operator->() const;
	/// This gives us the top-most parent paragraph
	pit_type outerPar() const;
	///
	pit_type pit() const;
	///
	/// return the paragraph this cursor is in
	pit_type & pit() { return DocIterator::pit(); }

	ParagraphList & plist() const;
};


ParIterator par_iterator_begin(Inset & inset);

ParIterator par_iterator_end(Inset & inset);


///
//bool operator==(ParIterator const & it1, ParIterator const & it2);

// FIXME: Unfortunately operator!=(ParIterator &, ParIterator &) is
// implemented with operator!=(DocIterator &, DocIterator &) that gives
// false if the positions are different, even if the pars are the same.
// So ultimately it's a bug in operator!=(ParIterator &, ParIterator &)
// I'd say (nevertheless, I would be reluctant to change it, because I
// fear that some part of the code could rely on this "bug". --Alfredo
//bool operator!=(ParIterator const & it1, ParIterator const & it2);


class ParConstIterator : public DocIterator
{
public:
	///
	ParConstIterator(Buffer const * buf)
		: DocIterator(const_cast<Buffer *>(buf)) {}
	///
	ParConstIterator(ParConstIterator const & pi)
		: DocIterator(DocIterator(pi)) {}
	///
	explicit ParConstIterator(DocIterator const & dit)
		: DocIterator(dit) {}
	///
	void push_back(Inset const &);

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

//bool operator==(ParConstIterator const & it1, ParConstIterator const & it2);

//bool operator!=(ParConstIterator const & it1, ParConstIterator const & it2);


} // namespace lyx

#endif
