
#include "coordcache.h"
#include "debug.h"

CoordCache theCoords;

// just a helper to be able to set a breakpoint
void lyxbreaker(void const * data, const char * hint, int size)
{
	lyxerr << "break on pointer: " << data << " hint: " << hint 
		<< " size: " << size << std::endl;
}


void CoordCache::clear() 
{
	lyxerr << "CoordCache: removing " << arrays_.data_.size()
		<< " arrays" << std::endl;
	lyxerr << "CoordCache: removing " << insets_.data_.size()
		<< " insets" << std::endl;
	arrays_.clear();
	insets_.clear();
}

