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
 *  \param matches files found to match \c pattern are appended.
 *  \param pattern the glob to be expanded. Eg "*.[Ch]".
 *  \param working_dir the starting directory from which \c pattern
 *  is to be expanded. Used only if \c pattern is a relative path.
 *  \param flags flags to be passed to the system function. See 'man glob'.
 */
void glob(std::vector<std::string> & matches,
	  std::string const & pattern,
	  std::string const & working_dir,
	  int flags = 0);

} // namespace support
} // namespace lyx

#endif // NOT GLOBBING_H
