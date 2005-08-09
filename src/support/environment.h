// -*- C++ -*-
/**
 * \file environment.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_ENVIRONMENT_H
#define LYX_ENVIRONMENT_H

#include <string>
#include <vector>

namespace lyx {
namespace support {

/// @returns the contents of the environment variable @c name.
std::string const getEnv(std::string const & envname);

/** @returns the contents of the environment variable @c name,
 *  split into path elements using the OS-dependent separator token.
 *  Each element is then passed through os::internal_path() to
 *  guarantee that it is in the form of a unix-style path.
 *  If the environment variable is not set, then the function returns
 *  an empty vector.
 */
std::vector<std::string> const getEnvPath(std::string const & name);

/** Set the contents of the environment variable @c name to @c value.
 *  @returns true if the variable was set successfully.
 */
bool setEnv(std::string const & name, std::string const & value);

/** Set the contents of the environment variable @c name
 *  using the paths stored in the @c env vector.
 *  Each element is passed through os::external_path().
 *  Multiple elements are concatenated into a single string using
 *  os::path_separator().
 */
void setEnvPath(std::string const & name, std::vector<std::string> const & env);

/** Prepend a list of paths to that returned by the environment variable.
 *  Identical paths occurring later in the list are removed.
 *  @param name the name of the environment variable.
 *  @prefix the list of paths in OS-native syntax.
 *  Eg "/foo/bar:/usr/bin:/usr/local/bin" on *nix,
 *     "C:\foo\bar;C:\windows" on Windows.
 */
void prependEnvPath(std::string const & name, std::string const & prefix);

} // namespace support
} // namespace lyx

#endif // LYX_ENVIRONMENT_H
