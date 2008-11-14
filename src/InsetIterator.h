// -*- C++ -*-
/**
 * \file InsetIterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETITERATOR_H
#define INSETITERATOR_H

#include "DocIterator.h"


namespace lyx {


class InsetIterator : public DocIterator
{
public:
	///
	explicit InsetIterator(Inset & inset);
	///
	void operator++() { forwardInset(); }
	///
	void operator--();
	///
	Inset * operator->() { return nextInset(); }
	///
	Inset & operator*() { return *nextInset(); }
};

InsetIterator inset_iterator_begin(Inset & inset);

InsetIterator inset_iterator_end(Inset & inset);



} // namespace lyx

#endif
