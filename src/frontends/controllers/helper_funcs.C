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

#include "frontends/FileDialog.h"
#include "support/filetools.h" // OnlyPath, OnlyFilename
#include "support/lstrings.h"
#include "gettext.h" // _()
#include "lyx_gui_misc.h" // WriteAlert

using std::pair;
using std::vector;
using std::make_pair;


string const getStringFromVector(vector<string> const & vec,
				 string const & delim)
{
	string str;
	int i = 0;
	for (vector<string>::const_iterator it = vec.begin();
	     it != vec.end(); ++it) {
		string item = strip(frontStrip(*it));
		if (item.empty()) continue;

		if (i++ > 0) str += delim;
		str += item;
	}
	return str;
}

vector<string> const getVectorFromString(string const & str,
					 string const & delim)
{
	vector<string> vec;
	if (str.empty())
		return vec;

	string keys(strip(str));

	for(;;) {
		string::size_type const idx = keys.find(delim);
		if (idx == string::npos) {
			vec.push_back(frontStrip(keys));
			break;
		}

		string const key = strip(frontStrip(keys.substr(0, idx)));
		if (!key.empty())
			vec.push_back(key);

		string::size_type const start = idx + delim.size();
		keys = keys.substr(start);
	}

	return vec;
}

string const browseFile(LyXView * lv, string const & filename,
			string const & title,
			string const & pattern, 
			pair<string,string> const & dir1,
			pair<string,string> const & dir2)
{
	string lastPath = ".";
	if (!filename.empty()) lastPath = OnlyPath(filename);

	FileDialog fileDlg(lv, title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;
 
	while (1) {
		result = fileDlg.Select(lastPath, pattern, OnlyFilename(filename));

		if (result.second.empty()) 
			return result.second;

		lastPath = OnlyPath(result.second);

		if (result.second.find_first_of("#~$% ") == string::npos)
			break; 
 
		WriteAlert(_("Filename can't contain any "
			"of these characters:"),
			_("space, '#', '~', '$' or '%'."));
	}

	return result.second;
}


// sorry this is just a temporary hack we should include vspace.h! (Jug)
extern const char * stringFromUnit(int);

vector<string> const getLatexUnits()
{
	vector<string> units;
	const char * str;
	for(int i=0; (str = stringFromUnit(i)); ++i)
	    units.push_back(str);

	return units;
}
