/* \file iterators.C
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

ParIterator::ParIterator(DocumentIterator const & cur) : DocumentIterator(cur)
{}


ParIterator::ParIterator(InsetBase & in, par_type pit) : DocumentIterator(in)
{
	par() = pit;
}


ParIterator::ParIterator(ParIterator const & pi)
	: DocumentIterator(DocumentIterator(pi))
{}


ParIterator & ParIterator::operator++()
{
	forwardPar();
	return *this;
}


ParIterator & ParIterator::operator--()
{
//	DocumentIterator::backwardPar();
	return *this;
}


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
	return DocumentIterator(iter1) == DocumentIterator(iter2);
}


bool operator!=(ParIterator const & iter1, ParIterator const & iter2)
{
	return !(iter1 == iter2);
}

DocumentIterator
makeDocumentIterator(ParIterator const & par, lyx::pos_type pos)
{
	DocumentIterator dit(par);
	dit.pos() = pos;
	return dit;
}

///
/// ParConstIterator
///


ParConstIterator::ParConstIterator(InsetBase const & in, par_type pit)
	: DocumentIterator(const_cast<InsetBase &>(in))
{
	par() = pit;
}


ParConstIterator::ParConstIterator(DocumentIterator const & dit)
	: DocumentIterator(dit)
{}


ParConstIterator::ParConstIterator(ParConstIterator const & pi)
	: DocumentIterator(DocumentIterator(pi))
{}


ParConstIterator & ParConstIterator::operator++()
{
	DocumentIterator::forwardPar();
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
	return DocumentIterator(iter1) == DocumentIterator(iter2);
}


bool operator!=(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return !(iter1 == iter2);
}
