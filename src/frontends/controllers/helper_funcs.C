/**
 * \file helper_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <vector>

#include <config.h>
#include "LString.h"
#include "helper_funcs.h"

#include "buffer.h"
#include "gettext.h"
#include "lyxrc.h"

#include "frontends/Alert.h"
#include "frontends/FileDialog.h"
#include "frontends/LyXView.h"

#include "support/filetools.h" // OnlyPath, OnlyFilename
#include "support/lstrings.h"

using std::pair;
using std::vector;
using std::make_pair;


namespace {

string const get_invalid_chars_latex()
{
	string invalid_chars("#$%{}()[]:\"^");
	if (!lyxrc.tex_allows_spaces)
		invalid_chars += ' ';
	return invalid_chars;
}


string const printable_list(string const & invalid_chars)
{
	ostringstream ss;
	string::const_iterator const begin = invalid_chars.begin();
	string::const_iterator const end = invalid_chars.end();
	string::const_iterator it = begin;

	for (; it != end; ++it) {
		if (it != begin)
			ss << ", ";
		if (*it == ' ')
			ss << _("space");
		else
			ss << *it;
	}

	return STRCONV(ss.str());
}

} // namespace anon


string const browseFile(LyXView * lv,
			bool check_returned_filename,
			string const & filename,
			string const & title,
			string const & pattern,
			bool save,
			pair<string,string> const & dir1,
			pair<string,string> const & dir2)
{
	string lastPath(".");
	if (!filename.empty())
		lastPath = OnlyPath(filename);

	FileDialog fileDlg(lv, title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;
	string const & result_path = result.second;
	string const invalid_chars = lv->buffer()->isLatex() ?
		get_invalid_chars_latex() : string();

	while (true) {
		if (save)
			result = fileDlg.save(lastPath, pattern,
				OnlyFilename(filename));
		else
			result = fileDlg.open(lastPath, pattern,
				OnlyFilename(filename));

		if (!check_returned_filename)
			break;
		if (invalid_chars.empty())
			break;
		if (result_path.empty())
			break;
		if (result_path.find_first_of(invalid_chars) == string::npos)
			break;

		Alert::alert(_("Invalid filename"),
			     _("No LaTeX support for paths containing any of these characters:\n") +
			     printable_list(invalid_chars));
	}

	return result_path;
}


string const browseRelFile(LyXView * lv,
			   bool check_returned_filename,
			   string const & filename,
			   string const & refpath,
			   string const & title,
			   string const & pattern,
			   bool save,
			   pair<string,string> const & dir1,
			   pair<string,string> const & dir2)
{
	string const fname = MakeAbsPath(filename, refpath);

	string const outname = browseFile(lv, check_returned_filename,
					  fname, title, pattern, save,
					  dir1, dir2);
	string const reloutname = MakeRelPath(outname, refpath);
	if (prefixIs(reloutname, "../"))
		return outname;
	else
		return reloutname;
}


string const browseDir(LyXView * lv,
		       bool check_returned_pathname,
		       string const & pathname,
		       string const & title,
		       pair<string,string> const & dir1,
		       pair<string,string> const & dir2)
{
	string lastPath(".");
	if (!pathname.empty())
		lastPath = OnlyPath(pathname);

	FileDialog fileDlg(lv, title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;
	string const & result_path = result.second;
	string const invalid_chars = lv->buffer()->isLatex() ?
		get_invalid_chars_latex() : string();

	while (true) {
		result = fileDlg.opendir(lastPath,
				OnlyFilename(pathname));

		if (!check_returned_pathname)
			break;
		if (invalid_chars.empty())
			break;
		if (result_path.empty())
			break;
		if (result_path.find_first_of(invalid_chars) == string::npos)
			break;

		lastPath = OnlyPath(result_path);
		Alert::alert(_("Invalid directory name"),
			     _("No LaTeX support for paths containing any of these characters:\n") +
			     printable_list(invalid_chars));
	}

	return result_path;
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
