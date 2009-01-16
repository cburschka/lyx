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
#include "support/os.h"

#include <boost/tokenizer.hpp>

#include <cstdlib>
#include <map>
#include <sstream>

using namespace std;

namespace lyx {
namespace support {

string const getEnv(string const & envname)
{
	// f.ex. what about error checking?
	char const * const ch = getenv(envname.c_str());
	return ch ? to_utf8(from_local8bit(ch)) : string();
}


vector<string> const getEnvPath(string const & name)
{
	typedef boost::char_separator<char> Separator;
	typedef boost::tokenizer<Separator> Tokenizer;

	string const env_var = getEnv(name);
	Separator const separator(string(1, os::path_separator()).c_str());
	Tokenizer const tokens(env_var, separator);
	Tokenizer::const_iterator it = tokens.begin();
	Tokenizer::const_iterator const end = tokens.end();

	vector<string> vars;
	for (; it != end; ++it)
		vars.push_back(os::internal_path(*it));

	return vars;
}


bool setEnv(string const & name, string const & value)
{
	// CHECK Look at and fix this.
	// f.ex. what about error checking?

	string const encoded = to_local8bit(from_utf8(value));
#if defined (HAVE_SETENV)
	return ::setenv(name.c_str(), encoded.c_str(), 1) == 0;
#elif defined (HAVE_PUTENV)
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
	vector<string> env_var = getEnvPath(name);

	typedef boost::char_separator<char> Separator;
	typedef boost::tokenizer<Separator> Tokenizer;

	Separator const separator(string(1, os::path_separator()).c_str());

	// Prepend each new element to the list, removing identical elements
	// that occur later in the list.
	Tokenizer const tokens(prefix, separator);
	vector<string> reversed_tokens(tokens.begin(), tokens.end());

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

} // namespace support
} // namespace lyx
