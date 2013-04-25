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

#include "Text.h"

#include "support/debug.h"
#include "support/docstring.h"

#include "insets/Inset.h"

#include "support/lassert.h"


namespace lyx {

Point::Point(int x, int y)
	: x_(x), y_(y)
{
	LASSERT(x > -1000000, x = -1000000);
	LASSERT(x <  1000000, x =  1000000);
	LASSERT(y > -1000000, y = -1000000);
	LASSERT(y <  1000000, y =  1000000);
}


// just a helper to be able to set a breakpoint
void lyxbreaker(void const * data, const char * hint, int size)
{
	LYXERR0("break on pointer: " << data << " hint: " << hint
		<< " size: " << size);
	LASSERT(false, return);
}


void CoordCache::clear()
{
	arrays_.clear();
	insets_.clear();
}


void CoordCache::dump() const
{
	LYXERR0("InsetCache contains:");
	CoordCacheBase<Inset>::cache_type::const_iterator it =
		getInsets().data_.begin();
	for (; it != getInsets().data_.end(); ++it) {
		// Warning: it is not guaranteed that inset is a valid pointer
		// (therefore it has type 'void *') (see bug #7376).	
		void const * inset = it->first;
		Point const p = it->second.pos;
		LYXERR0("Inset " << inset << " has point " << p.x_ << "," << p.y_);
	}
}

} // namespace lyx
