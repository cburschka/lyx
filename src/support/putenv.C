/**
 * \file putenv.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include <cstdlib>
#include <string>
#include <map>

using std::string;
using std::map;

bool lyx::support::putenv(string const & varname, string const & value)
{
	static map<string, char *> varmap;

	string str = varname + '=' + value;
	char * newptr = new char[str.size() + 1];
	newptr[str.copy(newptr, string::npos)] = '\0';
	bool status = (::putenv(newptr) == 0);

	char * oldptr = varmap[varname];
	if (oldptr)
		delete oldptr;
	varmap[varname] = newptr;

	return status;
}
