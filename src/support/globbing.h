// -*- C++ -*-
/**
 * \file globbing.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GLOBBING_H
#define GLOBBING_H

#include <string>
#include <vector>

namespace lyx {
namespace support {

/** A wrapper for the Posix function 'glob'.
 *  \param pattern the glob to be expanded. Eg "*.[Ch]".
 *  \param flags flags to be passed to the system function. See 'man glob'.
 *  \returns a vector of the files found to match \c pattern.
 */
std::vector<std::string> const glob(std::string const & pattern, int flags = 0);

} // namespace support
} // namespace lyx

#endif // NOT GLOBBING_H
