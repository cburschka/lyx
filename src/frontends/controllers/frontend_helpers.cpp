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

#include "Buffer.h"
#include "BufferParams.h"
#include "Color.h"
#include "debug.h"
#include "gettext.h"
#include "Language.h"
#include "Length.h"

#include "frontends/FileDialog.h"
#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#include <boost/cregex.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <fstream>

using std::string;
using std::vector;
using std::pair;
using std::endl;

namespace lyx {

namespace frontend {

vector<FamilyPair> const getFamilyData()
{
	vector<FamilyPair> family(5);

	FamilyPair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_FAMILY;
	family[0] = pr;

	pr.first = _("Roman");
	pr.second = Font::ROMAN_FAMILY;
	family[1] = pr;

	pr.first = _("Sans Serif");
	pr.second = Font::SANS_FAMILY;
	family[2] = pr;

	pr.first = _("Typewriter");
	pr.second = Font::TYPEWRITER_FAMILY;
	family[3] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_FAMILY;
	family[4] = pr;

	return family;
}


vector<SeriesPair> const getSeriesData()
{
	vector<SeriesPair> series(4);

	SeriesPair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_SERIES;
	series[0] = pr;

	pr.first = _("Medium");
	pr.second = Font::MEDIUM_SERIES;
	series[1] = pr;

	pr.first = _("Bold");
	pr.second = Font::BOLD_SERIES;
	series[2] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_SERIES;
	series[3] = pr;

	return series;
}


vector<ShapePair> const getShapeData()
{
	vector<ShapePair> shape(6);

	ShapePair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_SHAPE;
	shape[0] = pr;

	pr.first = _("Upright");
	pr.second = Font::UP_SHAPE;
	shape[1] = pr;

	pr.first = _("Italic");
	pr.second = Font::ITALIC_SHAPE;
	shape[2] = pr;

	pr.first = _("Slanted");
	pr.second = Font::SLANTED_SHAPE;
	shape[3] = pr;

	pr.first = _("Small Caps");
	pr.second = Font::SMALLCAPS_SHAPE;
	shape[4] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_SHAPE;
	shape[5] = pr;

	return shape;
}


vector<SizePair> const getSizeData()
{
	vector<SizePair> size(14);

	SizePair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_SIZE;
	size[0] = pr;

	pr.first = _("Tiny");
	pr.second = Font::SIZE_TINY;
	size[1] = pr;

	pr.first = _("Smallest");
	pr.second = Font::SIZE_SCRIPT;
	size[2] = pr;

	pr.first = _("Smaller");
	pr.second = Font::SIZE_FOOTNOTE;
	size[3] = pr;

	pr.first = _("Small");
	pr.second = Font::SIZE_SMALL;
	size[4] = pr;

	pr.first = _("Normal");
	pr.second = Font::SIZE_NORMAL;
	size[5] = pr;

	pr.first = _("Large");
	pr.second = Font::SIZE_LARGE;
	size[6] = pr;

	pr.first = _("Larger");
	pr.second = Font::SIZE_LARGER;
	size[7] = pr;

	pr.first = _("Largest");
	pr.second = Font::SIZE_LARGEST;
	size[8] = pr;

	pr.first = _("Huge");
	pr.second = Font::SIZE_HUGE;
	size[9] = pr;

	pr.first = _("Huger");
	pr.second = Font::SIZE_HUGER;
	size[10] = pr;

	pr.first = _("Increase");
	pr.second = Font::INCREASE_SIZE;
	size[11] = pr;

	pr.first = _("Decrease");
	pr.second = Font::DECREASE_SIZE;
	size[12] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_SIZE;
	size[13] = pr;

	return size;
}


vector<BarPair> const getBarData()
{
	vector<BarPair> bar(5);

	BarPair pr;

	pr.first = _("No change");
	pr.second = IGNORE;
	bar[0] = pr;

	pr.first = _("Emph");
	pr.second = EMPH_TOGGLE;
	bar[1] = pr;

	pr.first = _("Underbar");
	pr.second = UNDERBAR_TOGGLE;
	bar[2] = pr;

	pr.first = _("Noun");
	pr.second = NOUN_TOGGLE;
	bar[3] = pr;

	pr.first = _("Reset");
	pr.second = INHERIT;
	bar[4] = pr;

	return bar;
}


vector<ColorPair> const getColorData()
{
	vector<ColorPair> color(11);

	ColorPair pr;

	pr.first = _("No change");
	pr.second = Color::ignore;
	color[0] = pr;

	pr.first = _("No color");
	pr.second = Color::none;
	color[1] = pr;

	pr.first = _("Black");
	pr.second = Color::black;
	color[2] = pr;

	pr.first = _("White");
	pr.second = Color::white;
	color[3] = pr;

	pr.first = _("Red");
	pr.second = Color::red;
	color[4] = pr;

	pr.first = _("Green");
	pr.second = Color::green;
	color[5] = pr;

	pr.first = _("Blue");
	pr.second = Color::blue;
	color[6] = pr;

	pr.first = _("Cyan");
	pr.second = Color::cyan;
	color[7] = pr;

	pr.first = _("Magenta");
	pr.second = Color::magenta;
	color[8] = pr;

	pr.first = _("Yellow");
	pr.second = Color::yellow;
	color[9] = pr;

	pr.first = _("Reset");
	pr.second = Color::inherit;
	color[10] = pr;

	return color;
}



namespace {

class Sorter
	: public std::binary_function<LanguagePair,
				      LanguagePair, bool>
{
public:
	bool operator()(LanguagePair const & lhs,
			LanguagePair const & rhs) const {
		return lhs.first < rhs.first;
	}
};


class ColorSorter
{
public:
	bool operator()(Color::color const & lhs,
			Color::color const & rhs) const {
		return lcolor.getGUIName(lhs) < lcolor.getGUIName(rhs);
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


vector<Color_color> const getSortedColors(vector<Color_color> colors)
{
	// sort the colors
	std::sort(colors.begin(), colors.end(), ColorSorter());
	return colors;
}

} // namespace frontend

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


using support::bformat;
using support::contains;
using support::FileName;
using support::getExtension;
using support::getFileContents;
using support::getVectorFromString;
using support::libFileSearch;
using support::onlyFilename;
using support::package;
using support::quoteName;
using support::split;
using support::Systemcall;
using support::token;

namespace frontend {

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
		     bformat(_("The script `%s' failed."), lyx::from_utf8(command.absFilename())));
}


void texhash()
{
	// Run texhash in user lyx directory
	support::Path p(package().user_support());

	//path to texhash through system
	Systemcall one;
	one.startscript(Systemcall::Wait,"texhash");
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
	for (; it != end; ++it) {
		*it = regex.Merge((*it), "/");
	}

	// remove empty items and duplicates
	list.erase(std::remove(list.begin(), list.end(), ""), list.end());
	eliminate_duplicates(list);
}


string const getListOfOptions(string const & classname, string const & type)
{
	FileName const filename(getTexFileFromList(classname, type));
	if (filename.empty())
		return string();
	string optionList = string();
	std::ifstream is(filename.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		is >> s;
		if (contains(s,"DeclareOption")) {
			s = s.substr(s.find("DeclareOption"));
			s = split(s,'{');		// cut front
			s = token(s,'}',0);		// cut end
			optionList += (s + '\n');
		}
	}
	return optionList;
}


string const getTexFileFromList(string const & file,
			    string const & type)
{
	string file_ = file;
	// do we need to add the suffix?
	if (!(getExtension(file) == type))
		file_ += '.' + type;

	lyxerr << "Searching for file " << file_ << endl;

	string lstfile;
	if (type == "cls")
		lstfile = "clsFiles.lst";
	else if (type == "sty")
		lstfile = "styFiles.lst";
	else if (type == "bst")
		lstfile = "bstFiles.lst";
	else if (type == "bib")
		lstfile = "bibFiles.lst";
	FileName const abslstfile = libFileSearch(string(), lstfile);
	if (abslstfile.empty()) {
		lyxerr << "File `'" << lstfile << "' not found." << endl;
		return string();
	}
	string const allClasses = getFileContents(abslstfile);
	int entries = 0;
	string classfile = token(allClasses, '\n', entries);
	int count = 0;
	while ((!contains(classfile, file) ||
		(onlyFilename(classfile) != file)) &&
		(++count < 1000)) {
		classfile = token(allClasses, '\n', ++entries);
	}

	// now we have filename with full path
	lyxerr << "with full path: " << classfile << endl;

	return classfile;
}

} // namespace frontend
} // namespace lyx
