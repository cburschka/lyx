
#include "insetiterator.h"

#include <boost/assert.hpp>


InsetIterator::InsetIterator(InsetBase & inset)
	: DocumentIterator(inset)
{
	if (size() && !nextInset())
		forwardInset();
}
