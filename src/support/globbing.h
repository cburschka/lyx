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

/** Given a string such as
 *      "<glob> <glob> ... *.{abc,def} <glob>",
 *  convert the csh-style brace expresions:
 *      "<glob> <glob> ... *.abc *.def <glob>".
 *  Requires no system support, so should work equally on Unix, Mac, Win32.
 */
std::string const convert_brace_glob(std::string const & glob);

/** A wrapper for the Posix function 'glob'.
 *  \param pattern the glob to be expanded. Eg "*.[Ch]".
 *  \param flags flags to be passed to the system function. See 'man glob'.
 *  \returns a vector of the files found to match \c pattern.
 */
std::vector<std::string> const glob(std::string const & pattern, int flags = 0);

/** Given a string "<glob> <glob> <glob>", expand each glob in turn.
 *  Any glob that cannot be expanded is ignored silently.
 *  Invokes \c expand_brace_glob and \c glob internally, so use only
 *  on systems supporting the Posix function 'glob'.
 * \param mask the string "<glob> <glob> <glob>".
 * \param directory (if not empty) the current working directory from
 * which \c glob is invoked.
 *  \returns a vector of all matching file names.
 */
std::vector<std::string> const
expand_globs(std::string const & mask,
	     std::string const & directory = std::string());

} // namespace support
} // namespace lyx

#endif // NOT GLOBBING_H
