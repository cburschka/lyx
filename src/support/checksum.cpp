// -*- C++ -*-
/**
 * \file checksum.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yuriy Skalko
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include "support/checksum.h"

#include <zlib.h>

namespace lyx {

namespace support {

unsigned long checksum(std::string const & s)
{
	auto p = reinterpret_cast<unsigned char const *>(s.c_str());
	return crc32(0, p, s.size());
}

unsigned long checksum(std::ifstream & ifs)
{
	std::istreambuf_iterator<char> beg(ifs);
	std::istreambuf_iterator<char> end;

	unsigned long sum = 0;
	for (auto & it = beg; beg != end; ++it) {
		unsigned char c = *it;
		sum = crc32(sum, &c, 1);
	}
	return sum;
}

unsigned long checksum(unsigned char const * beg, unsigned char const * end)
{
	return crc32(0, beg, end - beg);
}

} // namespace support

} // namespace lyx
