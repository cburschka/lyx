/* \file CoordCache.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CoordCache.h"
#include "debug.h"

#include "Text.h"

#include "insets/Inset.h"

#include <boost/assert.hpp>


namespace lyx {

Point::Point(int x, int y)
	: x_(x), y_(y)
{
	BOOST_ASSERT(x > -1000000);
	BOOST_ASSERT(x <  1000000);
	BOOST_ASSERT(y > -1000000);
	BOOST_ASSERT(y <  1000000);
}

// just a helper to be able to set a breakpoint
void lyxbreaker(void const * data, const char * hint, int size)
{
	lyxerr << "break on pointer: " << data << " hint: " << hint
		<< " size: " << size << std::endl;
	BOOST_ASSERT(false);
}


void CoordCache::clear()
{
	arrays_.clear();
	insets_.clear();
	slices0_.clear();
	slices1_.clear();
}


void CoordCache::dump() const
{
	lyxerr << "InsetCache contains:" << std::endl;
	for (CoordCacheBase<Inset>::cache_type::const_iterator it = getInsets().getData().begin(); it != getInsets().getData().end(); ++it) {
		Inset const * inset = it->first;
		Point const p = it->second;
		lyxerr << "Inset " << inset << "(" << to_utf8(inset->name())
			<< ") has point " << p.x_ << "," << p.y_ << std::endl;
	}
}

} // namespace lyx
