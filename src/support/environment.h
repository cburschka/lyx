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

/// @returns true if the environment variable @c name exists.
bool hasEnv(std::string const & name);

/// @returns the contents of the environment variable @c name encoded in utf8.
std::string const getEnv(std::string const & name);

/** @returns the contents of the environment variable @c name,
 *  split into path elements using the OS-dependent separator token
 *  and encoded in utf8.
 *  Each element is then passed through os::internal_path() to
 *  guarantee that it is in the form of a unix-style path.
 *  If the environment variable is not set, then the function returns
 *  an empty vector.
 */
std::vector<std::string> const getEnvPath(std::string const & name);

/** Set the contents of the environment variable @c name to @c value.
 *  \p value is encoded in utf8.
 *  @returns true if the variable was set successfully.
 */
bool setEnv(std::string const & name, std::string const & value);

/** Set the contents of the environment variable @c name
 *  using the paths stored in the @c env vector (encoded in utf8).
 *  Each element is passed through os::external_path().
 *  Multiple elements are concatenated into a single string using
 *  os::path_separator().
 */
void setEnvPath(std::string const & name, std::vector<std::string> const & env);

/** Prepend a list of paths to that returned by the environment variable.
 *  Identical paths occurring later in the list are removed.
 *  @param name the name of the environment variable (encoded in utf8).
 *  @prefix the list of paths in OS-native syntax (encoded in utf8).
 *  Eg "/foo/bar:/usr/bin:/usr/local/bin" on *nix,
 *     "C:\foo\bar;C:\windows" on Windows.
 */
void prependEnvPath(std::string const & name, std::string const & prefix);

/** Remove the variable @c name from the environment.
 *  @returns true if the variable was unset successfully.
 */
bool unsetEnv(std::string const & name);


/** Utility class to change temporarily an environment variable. The
 * variable is reset to its original state when the dummy EnvChanger
 * variable is deleted.
 */
class EnvChanger {
public:
	///
	EnvChanger(std::string const & name, std::string const & value)
	: name_(name), set_(hasEnv(name)), value_(getEnv(name))
	{
			setEnv(name, value);
	}
	///
	~EnvChanger()
	{
		if (set_)
			setEnv(name_, value_);
		else
			unsetEnv(name_);
	}

private:
	/// the name of the variable
	std::string name_;
	/// was the variable set?
	bool set_;
	///
	std::string value_;
};

} // namespace support
} // namespace lyx

#endif // LYX_ENVIRONMENT_H
