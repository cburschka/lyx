/**
 * \file helper_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "helper_funcs.h"

#include "gettext.h"

#include "frontends/Alert.h"
#include "frontends/FileDialog.h"

#include "support/filetools.h" // OnlyPath, OnlyFilename
#include "support/globbing.h"

using lyx::support::FileFilterList;
using lyx::support::MakeAbsPath;
using lyx::support::MakeRelPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::prefixIs;

using std::pair;
using std::vector;
using std::string;


string const browseFile(string const & filename,
			string const & title,
			FileFilterList const & filters,
			bool save,
			pair<string,string> const & dir1,
			pair<string,string> const & dir2)
{
	string lastPath(".");
	if (!filename.empty())
		lastPath = OnlyPath(filename);

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;

	while (true) {
		if (save)
			result = fileDlg.save(lastPath, filters,
				OnlyFilename(filename));
		else
			result = fileDlg.open(lastPath, filters,
				OnlyFilename(filename));

		if (result.second.empty())
			return result.second;

		lastPath = OnlyPath(result.second);

		if (result.second.find_first_of("#~$% ") == string::npos)
			break;

		Alert::error(_("Invalid filename"),
			_("Filename can't contain any "
			"of these characters:\n"
			"space, '#', '~', '$' or '%'."));
	}

	return result.second;
}


string const browseRelFile(string const & filename,
			   string const & refpath,
			   string const & title,
			   FileFilterList const & filters,
			   bool save,
			   pair<string,string> const & dir1,
			   pair<string,string> const & dir2)
{
	string const fname = MakeAbsPath(filename, refpath);

	string const outname = browseFile(fname, title, filters, save,
					  dir1, dir2);
	string const reloutname = MakeRelPath(outname, refpath);
	if (prefixIs(reloutname, "../"))
		return outname;
	else
		return reloutname;
}


string const browseDir(string const & pathname,
			string const & title,
			pair<string,string> const & dir1,
			pair<string,string> const & dir2)
{
	string lastPath(".");
	if (!pathname.empty())
		lastPath = OnlyPath(pathname);

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;

	while (true) {
		result = fileDlg.opendir(lastPath,
				OnlyFilename(pathname));

		if (result.second.empty())
			return result.second;

		lastPath = OnlyPath(result.second);

		if (result.second.find_first_of("#~$% ") == string::npos)
			break;

		Alert::error(_("Invalid filename"),
			_("Filename can't contain any "
			"of these characters:\n"
			"space, '#', '~', '$' or '%'."));
	}

	return result.second;
}


// sorry this is just a temporary hack we should include vspace.h! (Jug)
extern const char * stringFromUnit(int);

vector<string> const getLatexUnits()
{
	vector<string> units;
	char const * str;
	for (int i = 0; (str = stringFromUnit(i)); ++i)
	    units.push_back(str);

	return units;
}
