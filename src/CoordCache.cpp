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

// just a helper to be able to set a breakpoint
void lyxbreaker(void const * data, const char * hint, int size)
{
	LYXERR0("break on pointer: " << data << " hint: " << hint
		<< " size: " << size);
	LBUFERR(false);
}


void CoordCache::clear()
{
	arrays_.clear();
	insets_.clear();
}


void CoordCache::dump() const
{
	if (getInsets().data_.empty()) {
		LYXERR0("InsetCache is empty.");
		return;
	}

	LYXERR0("InsetCache contains:");
	CoordCache::Insets::cache_type::const_iterator it =
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
