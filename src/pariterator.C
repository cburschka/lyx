/* \file pariterator.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "pariterator.h"

#include "ParagraphList_fwd.h"
#include "paragraph.h"
#include "lyxtext.h"

#include "insets/inset.h"

using lyx::par_type;


///
/// ParIterator
///

ParIterator::ParIterator(DocIterator const & cur) : DocIterator(cur)
{}


ParIterator par_iterator_begin(InsetBase & inset)
{
	return ParIterator(doc_iterator_begin(inset));
}


ParIterator par_iterator_end(InsetBase & inset)
{
	return ParIterator(doc_iterator_end(inset));
}


ParIterator::ParIterator(ParIterator const & pi)
	: DocIterator(DocIterator(pi))
{}


ParIterator & ParIterator::operator++()
{
	forwardPar();
	return *this;
}


ParIterator ParIterator::operator++(int)
{
	ParIterator tmp(*this);
	forwardPar();
	return tmp;
}


#if 0
// Unused member functions. Also having this makes ParIterator not be
// an forward iterator anymore. So unless we change that, this function
// should not be compiled/used. (Lgb)
ParIterator & ParIterator::operator--()
{
#ifdef WITH_WARNINGS
#warning look here
#endif
//	DocIterator::backwardPar();
	return *this;
}
#endif


Paragraph & ParIterator::operator*() const
{
	return text()->getPar(par());
}


par_type ParIterator::pit() const
{
	return par();
}


Paragraph * ParIterator::operator->() const
{
	return &text()->getPar(par());
}


par_type ParIterator::outerPar() const
{
	return bottom().par();
}


ParagraphList & ParIterator::plist() const
{
	return text()->paragraphs();
}


bool operator==(ParIterator const & iter1, ParIterator const & iter2)
{
	return DocIterator(iter1) == DocIterator(iter2);
}


bool operator!=(ParIterator const & iter1, ParIterator const & iter2)
{
	return !(iter1 == iter2);
}


DocIterator makeDocIterator(ParIterator const & par, lyx::pos_type pos)
{
	DocIterator dit(par);
	dit.pos() = pos;
	return dit;
}



///
/// ParConstIterator
///


ParConstIterator::ParConstIterator(DocIterator const & dit)
	: DocIterator(dit)
{}


ParConstIterator::ParConstIterator(ParConstIterator const & pi)
	: DocIterator(DocIterator(pi))
{}


ParConstIterator & ParConstIterator::operator++()
{
	DocIterator::forwardPar();
	return *this;
}


Paragraph const & ParConstIterator::operator*() const
{
	return text()->getPar(par());
}


Paragraph const * ParConstIterator::operator->() const
{
	return &text()->getPar(par());
}


ParagraphList const & ParConstIterator::plist() const
{
	return text()->paragraphs();
}


bool operator==(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return DocIterator(iter1) == DocIterator(iter2);
}


bool operator!=(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return !(iter1 == iter2);
}


#ifdef WITH_WARNINGS
#warning const correctness!
#endif

ParConstIterator par_const_iterator_begin(InsetBase const & inset)
{
	return ParConstIterator(doc_iterator_begin(const_cast<InsetBase &>(inset)));
}


ParConstIterator par_const_iterator_end(InsetBase const & inset)
{
	return ParConstIterator(doc_iterator_end(const_cast<InsetBase &>(inset)));
}
