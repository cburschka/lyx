/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file helper_funcs.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "LString.h"
#include "helper_funcs.h"

using std::vector;

string const getStringFromVector(vector<string> const & vec,
				 string const & delim)
{
	string str;
	for (vector<string>::size_type i=0; i<vec.size(); ++i) {
		if (i > 0) str += delim;
		str += vec[i];
	}
	return str;
}

vector<string> const getVectorFromString(string const & str,
					 string const & delim)
{
	vector<string> vec;
	string keys(str);

	for(;;) {
		string::size_type const idx = keys.find(delim);
		if (idx == string::npos) break;
		
		vec.push_back(keys.substr(0, idx));

		string::size_type const start = idx + delim.size();
		keys = keys.substr(start);
	}

	return vec;
}

