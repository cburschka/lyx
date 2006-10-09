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

using lyx::docstring;

using std::pair;
using std::string;
using std::vector;

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


docstring const browseFile(docstring const & filename,
			docstring const & title,
			FileFilterList const & filters,
			bool save,
			pair<docstring,docstring> const & dir1,
			pair<docstring,docstring> const & dir2)
{
	docstring lastPath = from_ascii(".");
	if (!filename.empty())
		lastPath = lyx::from_utf8(onlyPath(lyx::to_utf8(filename)));

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;

	if (save)
		result = fileDlg.save(lastPath, filters,
				      lyx::from_utf8(onlyFilename(lyx::to_utf8(filename))));
	else
		result = fileDlg.open(lastPath, filters,
				      lyx::from_utf8(onlyFilename(lyx::to_utf8(filename))));

	return result.second;
}


docstring const browseRelFile(docstring const & filename,
			   docstring const & refpath,
			   docstring const & title,
			   FileFilterList const & filters,
			   bool save,
			   pair<docstring,docstring> const & dir1,
			   pair<docstring,docstring> const & dir2)
{
	docstring const fname = lyx::from_utf8(
		makeAbsPath(lyx::to_utf8(filename), lyx::to_utf8(refpath)));

	docstring const outname = browseFile(fname, title, filters, save,
					  dir1, dir2);
	docstring const reloutname = lyx::from_utf8(
		makeRelPath(lyx::to_utf8(outname), lyx::to_utf8(refpath)));
	if (prefixIs(lyx::to_utf8(reloutname), "../"))
		return outname;
	else
		return reloutname;
}



docstring const browseLibFile(docstring const & dir,
			   docstring const & name,
			   docstring const & ext,
			   docstring const & title,
			   FileFilterList const & filters)
{
	// FIXME UNICODE
	pair<docstring, docstring> const dir1(_("System files|#S#s"),
				       lyx::from_utf8(addName(package().system_support(), lyx::to_utf8(dir))));

	pair<docstring, docstring> const dir2(_("User files|#U#u"),
				       lyx::from_utf8(addName(package().user_support(), lyx::to_utf8(dir))));

	docstring const result = browseFile(lyx::from_utf8(
		libFileSearch(lyx::to_utf8(dir), lyx::to_utf8(name), lyx::to_utf8(ext))),
		title, filters, false, dir1, dir2);

	// remove the extension if it is the default one
	docstring noextresult;
	if (lyx::from_utf8(getExtension(lyx::to_utf8(result))) == ext)
		noextresult = lyx::from_utf8(changeExtension(lyx::to_utf8(result), string()));
	else
		noextresult = result;

	// remove the directory, if it is the default one
	docstring const file = lyx::from_utf8(onlyFilename(lyx::to_utf8(noextresult)));
	if (lyx::from_utf8(libFileSearch(lyx::to_utf8(dir), lyx::to_utf8(file), lyx::to_utf8(ext))) == result)
		return file;
	else
		return noextresult;
}


docstring const browseDir(docstring const & pathname,
		       docstring const & title,
		       pair<docstring,docstring> const & dir1,
		       pair<docstring,docstring> const & dir2)
{
	docstring lastPath = lyx::from_ascii(".");
	if (!pathname.empty())
		lastPath = lyx::from_utf8(onlyPath(lyx::to_utf8(pathname)));

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result const result =
		fileDlg.opendir(lastPath, lyx::from_utf8(onlyFilename(lyx::to_utf8(pathname))));

	return result.second;
}


vector<docstring> const getLatexUnits()
{
	vector<docstring> units;
	int i = 0;
	char const * str = stringFromUnit(i);
	for (; str != 0; ++i, str = stringFromUnit(i))
		units.push_back(lyx::from_ascii(str));

	return units;
}

} // namespace frontend
} // namespace lyx
