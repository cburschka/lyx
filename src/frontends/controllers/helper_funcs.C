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
using std::string;
using std::vector;

namespace lyx {

using support::addName;
using support::FileFilterList;
using support::getExtension;
using support::libFileSearch;
using support::makeAbsPath;
using support::makeRelPath;
using support::onlyFilename;
using support::onlyPath;
using support::package;
using support::prefixIs;
using support::removeExtension;

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
		lastPath = from_utf8(onlyPath(to_utf8(filename)));

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;

	if (save)
		result = fileDlg.save(lastPath, filters,
				      from_utf8(onlyFilename(to_utf8(filename))));
	else
		result = fileDlg.open(lastPath, filters,
				      from_utf8(onlyFilename(to_utf8(filename))));

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
	docstring const fname = from_utf8(makeAbsPath(
		to_utf8(filename), to_utf8(refpath)).absFilename());

	docstring const outname = browseFile(fname, title, filters, save,
					  dir1, dir2);
	docstring const reloutname = makeRelPath(outname, refpath);
	if (prefixIs(reloutname, from_ascii("../")))
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
				       from_utf8(addName(package().system_support(), to_utf8(dir))));

	pair<docstring, docstring> const dir2(_("User files|#U#u"),
				       from_utf8(addName(package().user_support(), to_utf8(dir))));

	docstring const result = browseFile(from_utf8(
		libFileSearch(to_utf8(dir), to_utf8(name), to_utf8(ext)).absFilename()),
		title, filters, false, dir1, dir2);

	// remove the extension if it is the default one
	docstring noextresult;
	if (from_utf8(getExtension(to_utf8(result))) == ext)
		noextresult = from_utf8(removeExtension(to_utf8(result)));
	else
		noextresult = result;

	// remove the directory, if it is the default one
	docstring const file = from_utf8(onlyFilename(to_utf8(noextresult)));
	if (from_utf8(libFileSearch(to_utf8(dir), to_utf8(file), to_utf8(ext)).absFilename()) == result)
		return file;
	else
		return noextresult;
}


docstring const browseDir(docstring const & pathname,
		       docstring const & title,
		       pair<docstring,docstring> const & dir1,
		       pair<docstring,docstring> const & dir2)
{
	docstring lastPath = from_ascii(".");
	if (!pathname.empty())
		lastPath = from_utf8(onlyPath(to_utf8(pathname)));

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result const result =
		fileDlg.opendir(lastPath, from_utf8(onlyFilename(to_utf8(pathname))));

	return result.second;
}


vector<docstring> const getLatexUnits()
{
	vector<docstring> units;
	int i = 0;
	char const * str = stringFromUnit(i);
	for (; str != 0; ++i, str = stringFromUnit(i))
		units.push_back(from_ascii(str));

	return units;
}

} // namespace frontend
} // namespace lyx
