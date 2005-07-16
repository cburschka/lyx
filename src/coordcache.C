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


void CoordCache::clear()
{
	BOOST_ASSERT(updating);
	arrays_.clear();
	insets_.clear();
	pars_.clear();
}


void CoordCache::startUpdating()
{
	BOOST_ASSERT(!updating);
	updating = true;
}


void CoordCache::doneUpdating()
{
	BOOST_ASSERT(updating);
	updating = false;
}


Point CoordCache::get(LyXText const * text, lyx::pit_type pit)
{
	ParPosCache::iterator const it = pars_.find(text);
	BOOST_ASSERT(it != pars_.end());
	InnerParPosCache::iterator const posit = it->second.find(pit);
	BOOST_ASSERT(posit != it->second.end());
	return posit->second;
}
