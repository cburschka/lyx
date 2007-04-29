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

#include "LyXText.h"

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
	pars_.clear();
	slices0_.clear();
	slices1_.clear();
}


Point CoordCache::get(LyXText const * text, pit_type pit) const
{
	ParPosCache::const_iterator const it = pars_.find(text);
	BOOST_ASSERT(it != pars_.end());
	InnerParPosCache::const_iterator const posit = it->second.find(pit);
	BOOST_ASSERT(posit != it->second.end());
	return posit->second;
}


void CoordCache::dump() const
{
	lyxerr << "ParPosCache contains:" << std::endl;
	for (ParPosCache::const_iterator it = getParPos().begin(); it != getParPos().end(); ++it) {
		LyXText const * lt = it->first;
		InnerParPosCache const & cache = it->second;
		lyxerr << "LyXText:" << lt << std::endl;
		for (InnerParPosCache::const_iterator jt = cache.begin(); jt != cache.end(); ++jt) {
			pit_type pit = jt->first;
			Paragraph const & par = lt->getPar(pit);
			Point p = jt->second;
			lyxerr << "Paragraph " << pit << ": \"";
			int const n = std::min(static_cast<lyx::pos_type>(10), par.size());
			for (int k = 0; k < n; ++k)
				lyxerr << to_utf8(docstring(1, par.getChar(k)));
			lyxerr << "\" has point " << p.x_ << "," << p.y_ << std::endl;
		}
	}

	lyxerr << "InsetCache contains:" << std::endl;
	for (CoordCacheBase<Inset>::cache_type::const_iterator it = getInsets().getData().begin(); it != getInsets().getData().end(); ++it) {
		Inset const * inset = it->first;
		Point const p = it->second;
		lyxerr << "Inset " << inset << "(" << to_utf8(inset->insetName())
			<< ") has point " << p.x_ << "," << p.y_ << std::endl;
	}
}

} // namespace lyx
