
#include "coordcache.h"
#include "debug.h"

#include "lyxtext.h"

#include "mathed/math_data.h"
#include "insets/insetbase.h"

#include <boost/assert.hpp>


CoordCache theCoords;

// just a helper to be able to set a breakpoint
void lyxbreaker(void const * data, const char * hint, int size)
{
	lyxerr << "break on pointer: " << data << " hint: " << hint
		<< " size: " << size << std::endl;
	BOOST_ASSERT(false);
}


void lyxaborter(int x, int y)
{
	lyxerr << "abort on x: " << x << " y: " << y << std::endl;
	BOOST_ASSERT(false);
}


void CoordCache::clear()
{
	arrays_.clear();
	insets_.clear();
	pars_.clear();
}


Point CoordCache::get(LyXText const * text, lyx::pit_type pit)
{
	ParPosCache::iterator const it = pars_.find(text);
	BOOST_ASSERT(it != pars_.end());
	InnerParPosCache::iterator const posit = it->second.find(pit);
	BOOST_ASSERT(posit != it->second.end());
	return posit->second;
}
