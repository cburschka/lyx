/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 */


#include <config.h>

#include <fstream>
#include <iterator>
#include <algorithm>
#include <boost/crc.hpp>

#include "support/lyxlib.h"


namespace {

template<typename InputIterator>
inline
unsigned long do_crc(InputIterator first, InputIterator last)
{
	boost::crc_32_type crc;
	crc = std::for_each(first, last, crc);
	return crc.checksum();
}

} // namespace


// And this would be the file interface.
unsigned long lyx::sum(string const & file)
{
	std::ifstream ifs(file.c_str());
	if (!ifs) return 0;
	
#ifdef HAVE_DECL_ISTREAMBUF_ITERATOR
	// This is a lot faster...
	std::istreambuf_iterator<char> beg(ifs);
	std::istreambuf_iterator<char> end;
#else
	// than this.
	ifs.unsetf(std::ios::skipws);
	std::istream_iterator<char> beg(ifs);
	std::istream_iterator<char> end;
#endif

	return do_crc(beg, end);
}
