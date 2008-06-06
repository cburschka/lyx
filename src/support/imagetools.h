// -*- C++ -*-
/**
 * \file imagetools.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef IMAGETOOLS_H
#define IMAGETOOLS_H

#include <string>
#include <vector>

namespace lyx {
namespace support {

/// Return the list of loadable formats.
std::vector<std::string> loadableImageFormats();

} // namespace support
} // namespace lyx

#endif // IMAGETOOLS_H
