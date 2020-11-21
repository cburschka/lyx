// -*- C++ -*-
/**
 * \file checksum.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yuriy Skalko
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_CHECKSUM_H
#define LYX_CHECKSUM_H

#include <fstream>
#include <string>

namespace lyx {

namespace support {

unsigned long checksum(std::string const & s);
unsigned long checksum(std::ifstream & ifs);
unsigned long checksum(char const * beg, char const * end);

} // namespace support

} // namespace lyx

#endif // LYX_CHECKSUM_H
