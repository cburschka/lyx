/**
 * \file frontend_helpers.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontend_helpers.h"

#include "gettext.h"
#include "Language.h"

#include "frontends/FileDialog.h"
#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#include <boost/cregex.hpp>

#include <algorithm>
#include <fstream>

using std::string;
using std::vector;
using std::pair;
using std::endl;

namespace lyx {
namespace frontend {

using support::addName;
using support::bformat;
using support::FileFilterList;
using support::FileName;
using support::getExtension;
using support::getFileContents;
using support::getVectorFromString;
using support::libFileSearch;
using support::makeAbsPath;
using support::makeRelPath;
using support::onlyFilename;
using support::onlyPath;
using support::package;
using support::prefixIs;
using support::quoteName;
using support::removeExtension;
using support::Systemcall;
using support::token;


namespace {

struct Sorter
{
	bool operator()(LanguagePair const & lhs, LanguagePair const & rhs) const {
		return lhs.first < rhs.first;
	}
};


} // namespace anon


vector<LanguagePair> const getLanguageData(bool character_dlg)
{
	vector<LanguagePair>::size_type const size = character_dlg ?
		languages.size() + 2 : languages.size();

	vector<LanguagePair> langs(size);

	if (character_dlg) {
		langs[0].first = _("No change");
		langs[0].second = "ignore";
		langs[1].first = _("Reset");
		langs[1].second = "reset";
	}

	vector<string>::size_type i = character_dlg ? 2 : 0;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[i].first  = _(cit->second.display());
		langs[i].second = cit->second.lang();
		++i;
	}

	// Don't sort "ignore" and "reset"
	vector<LanguagePair>::iterator begin = character_dlg ?
		langs.begin() + 2 : langs.begin();

	std::sort(begin, langs.end(), Sorter());

	return langs;
}


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
		from_utf8(addName(package().system_support().absFilename(), to_utf8(dir))));

	pair<docstring, docstring> const dir2(_("User files|#U#u"),
		from_utf8(addName(package().user_support().absFilename(), to_utf8(dir))));

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


void rescanTexStyles()
{
	// Run rescan in user lyx directory
	support::Path p(package().user_support());
	FileName const command = libFileSearch("scripts", "TeXFiles.py");
	Systemcall one;
	int const status = one.startscript(Systemcall::Wait,
			lyx::support::os::python() + ' ' +
			quoteName(command.toFilesystemEncoding()));
	if (status == 0)
		return;
	// FIXME UNICODE
	Alert::error(_("Could not update TeX information"),
		bformat(_("The script `%s' failed."), from_utf8(command.absFilename())));
}


void getTexFileList(string const & filename, std::vector<string> & list)
{
	list.clear();
	FileName const file = libFileSearch("", filename);
	if (file.empty())
		return;

	list = getVectorFromString(getFileContents(file), "\n");

	// Normalise paths like /foo//bar ==> /foo/bar
	boost::RegEx regex("/{2,}");
	std::vector<string>::iterator it  = list.begin();
	std::vector<string>::iterator end = list.end();
	for (; it != end; ++it)
		*it = regex.Merge((*it), "/");

	// remove empty items and duplicates
	list.erase(std::remove(list.begin(), list.end(), ""), list.end());
	eliminate_duplicates(list);
}

} // namespace frontend
} // namespace lyx
