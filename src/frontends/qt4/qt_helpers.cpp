/**
 * \file qt_helpers.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Jürgen Spitzmüller
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qt_helpers.h"

#include "FileDialog.h"
#include "LengthCombo.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "Language.h"
#include "Length.h"

#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#include <QComboBox>
#include <QCheckBox>
#include <QPalette>
#include <QLineEdit>

#include <boost/cregex.hpp>

#include <algorithm>
#include <fstream>
#include <locale>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

string widgetsToLength(QLineEdit const * input, LengthCombo const * combo)
{
	QString const length = input->text();
	if (length.isEmpty())
		return string();

	// Don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(fromqstr(length)))
		return fromqstr(length);

	Length::UNIT const unit = combo->currentLengthItem();

	return Length(length.toDouble(), unit).asString();
}


Length widgetsToLength(QLineEdit const * input, QComboBox const * combo)
{
	QString const length = input->text();
	if (length.isEmpty())
		return Length();

	// don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(fromqstr(length)))
		return Length(fromqstr(length));

	Length::UNIT const unit = unitFromString(fromqstr(combo->currentText()));

	return Length(length.toDouble(), unit);
}


void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
                     Length const & len, Length::UNIT /*defaultUnit*/)
{
	combo->setCurrentItem(len.unit());
	input->setText(QString::number(Length(len).value()));
}


void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	string const & len, Length::UNIT defaultUnit)
{
	if (len.empty()) {
		// no length (UNIT_NONE)
		combo->setCurrentItem(defaultUnit);
		input->setText("");
	} else if (!isValidLength(len) && !isStrDbl(len)) {
		// use input field only for gluelengths
		combo->setCurrentItem(defaultUnit);
		input->setText(toqstr(len));
	} else {
		lengthToWidgets(input, combo, Length(len), defaultUnit);
	}
}


void lengthAutoToWidgets(QLineEdit * input, LengthCombo * combo,
	Length const & len, Length::UNIT defaultUnit)
{
	if (len.value() == 0)
		lengthToWidgets(input, combo, "auto", defaultUnit);
	else
		lengthToWidgets(input, combo, len, defaultUnit);
}


void setValid(QWidget * widget, bool valid)
{
	if (valid) {
		widget->setPalette(QPalette());
	} else {
		QPalette pal = widget->palette();
		pal.setColor(QPalette::Active, QPalette::Foreground, QColor(255, 0, 0));
		widget->setPalette(pal);
	}
}

} // namespace frontend

QString const qt_(char const * str, const char *)
{
	return toqstr(_(str));
}


QString const qt_(string const & str)
{
	return toqstr(_(str));
}

namespace {

class Sorter
{
public:
#if !defined(USE_WCHAR_T) && defined(__GNUC__)
	bool operator()(LanguagePair const & lhs, LanguagePair const & rhs) const {
		return lhs.first < rhs.first;
	}
#else
	Sorter() : loc_ok(true)
	{
		try {
			loc_ = locale("");
		} catch (...) {
			loc_ok = false;
		}
	};

	bool operator()(LanguagePair const & lhs,
			LanguagePair const & rhs) const {
		if (loc_ok)
			return loc_(lhs.first, rhs.first);
		else
			return lhs.first < rhs.first;
	}
private:
	locale loc_;
	bool loc_ok;
#endif
};


} // namespace anon


vector<LanguagePair> const getLanguageData(bool character_dlg)
{
	size_t const size = languages.size() + (character_dlg ? 2 : 0);

	vector<LanguagePair> langs(size);

	if (character_dlg) {
		langs[0].first = _("No change");
		langs[0].second = "ignore";
		langs[1].first = _("Reset");
		langs[1].second = "reset";
	}

	size_t i = character_dlg ? 2 : 0;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[i].first  = _(cit->second.display());
		langs[i].second = cit->second.lang();
		++i;
	}

	// Don't sort "ignore" and "reset"
	vector<LanguagePair>::iterator begin = character_dlg ?
		langs.begin() + 2 : langs.begin();

	sort(begin, langs.end(), Sorter());

	return langs;
}


docstring browseFile(docstring const & filename, docstring const & title,
	FileFilterList const & filters, bool save,
	docstring const & label1, docstring const & dir1,
	docstring const & label2, docstring const & dir2)
{
	docstring lastPath = from_ascii(".");
	if (!filename.empty())
		lastPath = from_utf8(onlyPath(to_utf8(filename)));

	FileDialog dlg(title, LFUN_SELECT_FILE_SYNC);
	dlg.setButton1(label1, dir1);
	dlg.setButton2(label2, dir2);

	FileDialog::Result result;

	if (save)
		result = dlg.save(lastPath, filters,
				      from_utf8(onlyFilename(to_utf8(filename))));
	else
		result = dlg.open(lastPath, filters,
				      from_utf8(onlyFilename(to_utf8(filename))));

	return result.second;
}


docstring browseRelFile(docstring const & filename, docstring const & refpath,
	docstring const & title, FileFilterList const & filters, bool save,
	docstring const & label1, docstring const & dir1,
	docstring const & label2, docstring const & dir2)
{
	docstring const fname = from_utf8(makeAbsPath(
		to_utf8(filename), to_utf8(refpath)).absFilename());

	docstring const outname = browseFile(fname, title, filters, save,
					  label1, dir1, label2, dir2);
	docstring const reloutname = makeRelPath(outname, refpath);
	if (prefixIs(reloutname, from_ascii("../")))
		return outname;
	else
		return reloutname;
}


docstring browseLibFile(docstring const & dir, docstring const & name,
	docstring const & ext, docstring const & title,
	FileFilterList const & filters)
{
	// FIXME UNICODE
	docstring const label1 = _("System files|#S#s");
	docstring const dir1 =
		from_utf8(addName(package().system_support().absFilename(), to_utf8(dir)));

	docstring const label2 = _("User files|#U#u");
	docstring const dir2 =
		from_utf8(addName(package().user_support().absFilename(), to_utf8(dir)));

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


docstring browseDir(docstring const & pathname, docstring const & title,
	docstring const & label1, docstring const & dir1,
	docstring const & label2, docstring const & dir2)
{
	docstring lastPath = from_ascii(".");
	if (!pathname.empty())
		lastPath = from_utf8(onlyPath(to_utf8(pathname)));

	FileDialog dlg(title, LFUN_SELECT_FILE_SYNC);
	dlg.setButton1(label1, dir1);
	dlg.setButton2(label2, dir2);

	FileDialog::Result const result =
		dlg.opendir(lastPath, from_utf8(onlyFilename(to_utf8(pathname))));

	return result.second;
}


void rescanTexStyles()
{
	// Run rescan in user lyx directory
	PathChanger p(package().user_support());
	FileName const command = libFileSearch("scripts", "TeXFiles.py");
	Systemcall one;
	int const status = one.startscript(Systemcall::Wait,
			os::python() + ' ' +
			quoteName(command.toFilesystemEncoding()));
	if (status == 0)
		return;
	// FIXME UNICODE
	frontend::Alert::error(_("Could not update TeX information"),
		bformat(_("The script `%s' failed."), from_utf8(command.absFilename())));
}


void getTexFileList(string const & filename, vector<string> & list)
{
	list.clear();
	FileName const file = libFileSearch("", filename);
	if (file.empty())
		return;

	// FIXME Unicode.
	vector<docstring> doclist = 
		getVectorFromString(file.fileContents("UTF-8"), from_ascii("\n"));

	// Normalise paths like /foo//bar ==> /foo/bar
	boost::RegEx regex("/{2,}");
	vector<docstring>::iterator it  = doclist.begin();
	vector<docstring>::iterator end = doclist.end();
	for (; it != end; ++it)
		list.push_back(regex.Merge(to_utf8(*it), "/"));

	// remove empty items and duplicates
	list.erase(remove(list.begin(), list.end(), ""), list.end());
	eliminate_duplicates(list);
}

} // namespace lyx
