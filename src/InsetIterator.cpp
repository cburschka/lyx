/**
 * \file InsetIterator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetIterator.h"

#include <boost/assert.hpp>


namespace lyx {


InsetIterator::InsetIterator(InsetBase & inset)
	: DocIterator(inset)
{
}


InsetIterator inset_iterator_begin(InsetBase & inset)
{
	InsetIterator it = InsetIterator(inset);
	it.forwardInset();
	return it;
}


InsetIterator inset_iterator_end(InsetBase & inset)
{
	return InsetIterator(inset);
}


} // namespace lyx
