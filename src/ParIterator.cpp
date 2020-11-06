/* \file ParIterator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ParIterator.h"

#include "CursorSlice.h"
#include "Text.h"

#include "insets/Inset.h"


namespace lyx {

//////////////////////////////////////////////////////////////////////////
//
// ParIterator
//
//////////////////////////////////////////////////////////////////////////


ParIterator par_iterator_begin(Inset & inset)
{
	return ParIterator(doc_iterator_begin(&inset.buffer(), &inset));
}


ParIterator par_iterator_end(Inset & inset)
{
	return ParIterator(doc_iterator_end(&inset.buffer(), &inset));
}


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
	// FIXME: look here
//	DocIterator::backwardPar();
	return *this;
}
#endif


Paragraph & ParIterator::operator*() const
{
	return const_cast<Paragraph&>(text()->getPar(pit()));
}


pit_type ParIterator::pit() const
{
	return DocIterator::pit();
}


Paragraph * ParIterator::operator->() const
{
	return const_cast<Paragraph*>(&text()->getPar(pit()));
}


pit_type ParIterator::outerPar() const
{
	return bottom().pit();
}


ParagraphList & ParIterator::plist() const
{
	return const_cast<ParagraphList&>(text()->paragraphs());
}


//////////////////////////////////////////////////////////////////////////
//
// ParConstIterator
//
//////////////////////////////////////////////////////////////////////////


void ParConstIterator::push_back(Inset const & inset)
{
	DocIterator::push_back(CursorSlice(const_cast<Inset &>(inset)));
}


ParConstIterator & ParConstIterator::operator++()
{
	DocIterator::forwardPar();
	return *this;
}


Paragraph const & ParConstIterator::operator*() const
{
	return text()->getPar(pit());
}


Paragraph const * ParConstIterator::operator->() const
{
	return &text()->getPar(pit());
}


ParagraphList const & ParConstIterator::plist() const
{
	return text()->paragraphs();
}

bool operator==(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	DocIterator const * dit1 = &iter1;
	DocIterator const * dit2 = &iter2;
	return *dit1 == *dit2;
}

#if 0
bool operator!=(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return !(iter1 == iter2);
}
#endif


} // namespace lyx
