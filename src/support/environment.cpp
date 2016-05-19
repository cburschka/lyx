/**
 * \file environment.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author João Luis M. Assirati
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/environment.h"

#include "support/docstring.h"
#include "support/lstrings.h"
#include "support/os.h"

#include <algorithm> // for remove
#include <cstdlib>
#include <map>
#include <sstream>

using namespace std;

namespace lyx {
namespace support {


bool hasEnv(string const & name)
{
	return getenv(name.c_str());
}


string const getEnv(string const & name)
{
	// f.ex. what about error checking?
	char const * const ch = getenv(name.c_str());
	return ch ? to_utf8(from_local8bit(ch)) : string();
}


vector<string> const getEnvPath(string const & name)
{
	string const env_var = getEnv(name);
	string const separator(1, os::path_separator());

	return getVectorFromString(env_var, separator);
}


bool setEnv(string const & name, string const & value)
{
	// CHECK Look at and fix this.
	// f.ex. what about error checking?

	string const encoded = to_local8bit(from_utf8(value));
#if defined (HAVE_SETENV)
	return ::setenv(name.c_str(), encoded.c_str(), 1) == 0;
#elif defined (HAVE_PUTENV)
	// According to http://pubs.opengroup.org/onlinepubs/9699919799/functions/putenv.html
	// the argument of putenv() needs to be static, because changing its
	// value will change the environment. Therefore we need a different static
	// storage for each variable.
	// FIXME THREAD
	static map<string, string> varmap;
	varmap[name] = name + '=' + encoded;
	return ::putenv(const_cast<char*>(varmap[name].c_str())) == 0;
#else
#error No environment-setting function has been defined.
#endif
	return false;
}


void setEnvPath(string const & name, vector<string> const & env)
{
	char const separator(os::path_separator());
	ostringstream ss;
	vector<string>::const_iterator const begin = env.begin();
	vector<string>::const_iterator const end = env.end();
	vector<string>::const_iterator it = begin;
	for (; it != end; ++it) {
		if (it != begin)
			ss << separator;
		ss << os::external_path(*it);
	}
	setEnv(name, ss.str());
}


void prependEnvPath(string const & name, string const & prefix)
{
	string const separator(1, os::path_separator());
	vector<string> reversed_tokens
		= getVectorFromString(prefix, separator);
	vector<string> env_var = getEnvPath(name);

	// Prepend each new element to the list, removing identical elements
	// that occur later in the list.
	typedef vector<string>::const_reverse_iterator token_iterator;
	token_iterator it = reversed_tokens.rbegin();
	token_iterator const end = reversed_tokens.rend();
	for (; it != end; ++it) {
		vector<string>::iterator remove_it =
			remove(env_var.begin(), env_var.end(), *it);
		env_var.erase(remove_it, env_var.end());
		env_var.insert(env_var.begin(), *it);
	}

	setEnvPath(name, env_var);
}


bool unsetEnv(string const & name)
{
#if defined(HAVE_UNSETENV)
	// FIXME: does it leak?
	return ::unsetenv(name.c_str()) == 0;
#elif defined(HAVE_PUTENV)
	// This is OK with MSVC and MinGW at least.
	// The argument of putenv() does not need to be a static variable in this
	// case, since the variable is removed from the environment.
	return ::putenv(const_cast<char*>((name + "=").c_str())) == 0;
#else
#error No environment-unsetting function has been defined.
#endif
}


} // namespace support
} // namespace lyx
