/* This file is part of
* ======================================================
*
*           LyX, The Document Processor
*
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2001 the LyX Team.
*
* ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "Variables.h"
#endif

#include "Variables.h"
#include "support/LRegex.h"


void Variables::set(string const & var, string const & val)
{
	// We want to use const_iterator (Lgb)
	Vars::iterator cit = vars_.find(var);
	if (cit != vars_.end())
		vars_.erase(var);
	vars_[var] = val;;
}


string const Variables::get(string const & var) const
{
	Vars::const_iterator cit = vars_.find(var);
	if (cit != vars_.end())
		return cit->second;
	else
		return string();
}


bool Variables::isSet(string const & var) const
{
	Vars::const_iterator cit = vars_.find(var);
	return (cit != vars_.end());
}


string const Variables::expand(string const & s) const
{
	string str(s);
	LRegex reg("\\$\\{\\(.*\\)\\}");

	if (!reg.exact_match(str))
		return str;

	LRegex::MatchPair match;
	string var;

	do {
		match = reg.first_match(str);
		var = str.substr(match.first,match.second);
		// we correct the match to take ${} in account.
		str.replace(match.first - 2, match.second + 3, get(var));
	} while (reg.exact_match(str));

	return str;
}

#ifdef TEST

#include <iostream>
using std::endl;
using std::cout;

int main() {
	Variables vars;
	vars.set("x", "hello");
	vars.set("y", "world");
	cout << vars.expand("${x}") << endl;
}

#endif
