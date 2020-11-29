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

#include "support/checksum.h"
#include "boost/crc.hpp"
#include <algorithm>

namespace lyx {

namespace support {

unsigned long checksum(std::string const & s)
{
	boost::crc_32_type crc;
	crc.process_bytes(s.c_str(), s.size());
	return crc.checksum();
}

unsigned long checksum(std::ifstream & ifs)
{
	std::istreambuf_iterator<char> beg(ifs);
	std::istreambuf_iterator<char> end;

	boost::crc_32_type crc;
	crc = for_each(beg, end, crc);
	return crc.checksum();
}

unsigned long checksum(char const * beg, char const * end)
{
	boost::crc_32_type crc;
	crc.process_block(beg, end);
	return crc.checksum();
}

} // namespace support

} // namespace lyx
