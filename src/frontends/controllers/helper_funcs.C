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
#include "lyxlength.h"

#include "frontends/FileDialog.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/package.h"

using std::pair;
using std::vector;
using std::string;

namespace lyx {

using support::addName;
using support::changeExtension;
using support::FileFilterList;
using support::getExtension;
using support::libFileSearch;
using support::makeAbsPath;
using support::makeRelPath;
using support::onlyFilename;
using support::onlyPath;
using support::package;
using support::prefixIs;

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
		lastPath = onlyPath(filename);

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;

	if (save)
		result = fileDlg.save(lastPath, filters,
				      onlyFilename(filename));
	else
		result = fileDlg.open(lastPath, filters,
				      onlyFilename(filename));

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
	string const fname = makeAbsPath(filename, refpath);

	string const outname = browseFile(fname, title, filters, save,
					  dir1, dir2);
	string const reloutname = makeRelPath(outname, refpath);
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
				       addName(package().system_support(), dir));

	pair<string,string> const dir2(_("User files|#U#u"),
				       addName(package().user_support(), dir));

	string const result = browseFile(libFileSearch(dir, name, ext), title,
					 filters, false, dir1, dir2);

	// remove the extension if it is the default one
	string noextresult;
	if (getExtension(result) == ext)
		noextresult = changeExtension(result, string());
	else
		noextresult = result;

	// remove the directory, if it is the default one
	string const file = onlyFilename(noextresult);
	if (libFileSearch(dir, file, ext) == result)
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
		lastPath = onlyPath(pathname);

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result const result =
		fileDlg.opendir(lastPath, onlyFilename(pathname));

	return result.second;
}


vector<string> const getLatexUnits()
{
	vector<string> units;
	int i = 0;
	char const * str = stringFromUnit(i);
	for (; str != 0; ++i, str = stringFromUnit(i))
		units.push_back(str);

	return units;
}

} // namespace frontend
} // namespace lyx
