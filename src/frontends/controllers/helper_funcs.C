/**
 * \file helper_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "helper_funcs.h"

#include "gettext.h"

#include "frontends/Alert.h"
#include "frontends/FileDialog.h"

#include "support/filetools.h"
#include "support/path_defines.h"
#include "support/globbing.h"

using std::pair;
using std::vector;
using std::string;

// sorry this is just a temporary hack we should include vspace.h! (Jug)
extern const char * stringFromUnit(int);

namespace lyx {

using support::AddName;
using support::ChangeExtension;
using support::FileFilterList;
using support::GetExtension;
using support::LibFileSearch;
using support::MakeAbsPath;
using support::MakeRelPath;
using support::OnlyFilename;
using support::OnlyPath;
using support::prefixIs;
using support::system_lyxdir;
using support::user_lyxdir;

namespace frontend {


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



string const browseLibFile(string const & dir,
			   string const & name,
			   string const & ext,
			   string const & title,
			   FileFilterList const & filters)
{
	pair<string,string> const dir1(_("System files|#S#s"),
				       AddName(system_lyxdir(), dir));

	pair<string,string> const dir2(_("User files|#U#u"),
				       AddName(user_lyxdir(), dir));

	string const result = browseFile(LibFileSearch(dir, name, ext), title,
				   filters, false, dir1, dir2);

	// remove the extension if it is the default one
	string noextresult;
	if (GetExtension(result) == ext)
		noextresult = ChangeExtension(result, string());
	else
		noextresult = result;

	// remove the directory, if it is the default one
	string const file = OnlyFilename(noextresult);
	if (LibFileSearch(dir, file, ext) == result)
		return file;
	else
		return noextresult;
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


vector<string> const getLatexUnits()
{
	vector<string> units;
	char const * str;
	for (int i = 0; (str = stringFromUnit(i)); ++i)
	    units.push_back(str);

	return units;
}

} // namespace frontend
} // namespace lyx
