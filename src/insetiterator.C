
#include "insetiterator.h"

#include <boost/assert.hpp>


InsetIterator::InsetIterator(InsetBase & inset)
	: DocumentIterator(inset)
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
