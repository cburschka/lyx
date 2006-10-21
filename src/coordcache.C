/* \file coordcache.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "coordcache.h"
#include "debug.h"

#include "lyxtext.h"

#include "insets/insetbase.h"

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


Point CoordCache::get(LyXText const * text, pit_type pit)
{
	ParPosCache::iterator const it = pars_.find(text);
	BOOST_ASSERT(it != pars_.end());
	InnerParPosCache::iterator const posit = it->second.find(pit);
	BOOST_ASSERT(posit != it->second.end());
	return posit->second;
}

void
CoordCache::dump() const {
	lyxerr << "ParPosCache contains:" << std::endl;
	for (ParPosCache::const_iterator i = getParPos().begin(); i != getParPos().end(); ++i) {
		LyXText const * lt = (*i).first;
		InnerParPosCache const & cache = (*i).second;
		lyxerr << "LyXText:" << lt << std::endl;
		for (InnerParPosCache::const_iterator j = cache.begin(); j != cache.end(); ++j) {
			pit_type pit = (*j).first;
			Paragraph const & par = lt->getPar(pit);
			Point p = (*j).second;
			lyxerr << "Paragraph " << pit << ": \"";
			for (int k = 0; k < std::min(10, par.size()); ++k) {
				lyxerr << to_utf8(docstring(1,par.getChar(k)));
			}
			lyxerr << "\" has point " << p.x_ << "," << p.y_ << std::endl;
		}
	}
}

} // namespace lyx
