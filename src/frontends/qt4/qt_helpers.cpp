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

#include "frontends/alert.h"

#include "BufferParams.h"
#include "FloatList.h"
#include "Language.h"
#include "Length.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/foreach.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLocale>
#include <QPalette>
#include <QSet>

#include <algorithm>
#include <fstream>
#include <locale>

// for FileFilter.
// FIXME: Remove
#include "support/regex.h"
#include <boost/tokenizer.hpp>


using namespace std;
using namespace lyx::support;

namespace lyx {

FileName libFileSearch(QString const & dir, QString const & name,
				QString const & ext)
{
	return support::libFileSearch(fromqstr(dir), fromqstr(name), fromqstr(ext));
}


FileName imageLibFileSearch(QString & dir, QString const & name,
				QString const & ext)
{
	string tmp = fromqstr(dir);
	FileName fn = support::imageLibFileSearch(tmp, fromqstr(name), fromqstr(ext));
	dir = toqstr(tmp);
	return fn;
}

namespace {

double locstringToDouble(QString const & str)
{
	QLocale loc;
	bool ok;
	double res = loc.toDouble(str, &ok);
	if (!ok) {
		// Fall back to C
		QLocale c(QLocale::C);
		res = c.toDouble(str);
	}
	return res;
}

} // namespace anon


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

	return Length(locstringToDouble(length.trimmed()), unit).asString();
}


Length widgetsToLength(QLineEdit const * input, QComboBox const * combo)
{
	QString const length = input->text();
	if (length.isEmpty())
		return Length();

	// don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(fromqstr(length)))
		return Length(fromqstr(length));

	Length::UNIT unit = Length::UNIT_NONE;
	QString const item = combo->currentText();
	for (int i = 0; i < num_units; i++) {
		if (qt_(lyx::unit_name_gui[i]) == item) {
			unit = unitFromString(unit_name[i]);
			break;
		}
	}

	return Length(locstringToDouble(length.trimmed()), unit);
}


void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	Length const & len, Length::UNIT /*defaultUnit*/)
{
	if (len.empty()) {
		// no length (UNIT_NONE)
		combo->setCurrentItem(Length::defaultUnit());
		input->setText("");
	} else {
		combo->setCurrentItem(len.unit());
		QLocale loc;
		loc.setNumberOptions(QLocale::OmitGroupSeparator);
		input->setText(loc.toString(Length(len).value()));
	}
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


void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	docstring const & len, Length::UNIT defaultUnit)
{
	lengthToWidgets(input, combo, to_utf8(len), defaultUnit);
}


double widgetToDouble(QLineEdit const * input)
{
	QString const text = input->text();
	if (text.isEmpty())
		return 0.0;

	return locstringToDouble(text.trimmed());
}


string widgetToDoubleStr(QLineEdit const * input)
{
	return convert<string>(widgetToDouble(input));
}


void doubleToWidget(QLineEdit * input, double const & value, char f, int prec)
{
	QLocale loc;
	loc.setNumberOptions(QLocale::OmitGroupSeparator);
	input->setText(loc.toString(value, f, prec));
}


void doubleToWidget(QLineEdit * input, string const & value, char f, int prec)
{
	doubleToWidget(input, convert<double>(value), f, prec);
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

/// wrapper to hide the change of method name to setSectionResizeMode
void setSectionResizeMode(QHeaderView * view,
    int logicalIndex, QHeaderView::ResizeMode mode) {
#if (QT_VERSION >= 0x050000)
	view->setSectionResizeMode(logicalIndex, mode);
#else
	view->setResizeMode(logicalIndex, mode);
#endif
}

void setSectionResizeMode(QHeaderView * view, QHeaderView::ResizeMode mode) {
#if (QT_VERSION >= 0x050000)
	view->setSectionResizeMode(mode);
#else
	view->setResizeMode(mode);
#endif
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


QString const qt_(QString const & qstr)
{
	return toqstr(_(fromqstr(qstr)));
}


void rescanTexStyles(string const & arg)
{
	// Run rescan in user lyx directory
	PathChanger p(package().user_support());
	FileName const prog = support::libFileSearch("scripts", "TeXFiles.py");
	Systemcall one;
	string const command = os::python() + ' ' +
	    quoteName(prog.toFilesystemEncoding()) + ' ' +
	    arg;
	int const status = one.startscript(Systemcall::Wait, command);
	if (status == 0)
		return;
	// FIXME UNICODE
	frontend::Alert::error(_("Could not update TeX information"),
		bformat(_("The script `%1$s' failed."), from_utf8(prog.absFileName())));
}


QStringList texFileList(QString const & filename)
{
	QStringList list;
	FileName const file = libFileSearch(QString(), filename);
	if (file.empty())
		return list;

	// FIXME Unicode.
	vector<docstring> doclist = 
		getVectorFromString(file.fileContents("UTF-8"), from_ascii("\n"));

	// Normalise paths like /foo//bar ==> /foo/bar
	QSet<QString> set;
	for (size_t i = 0; i != doclist.size(); ++i) {
		QString file = toqstr(doclist[i]);
		file.replace("\r", "");
		while (file.contains("//"))
			file.replace("//", "/");
		if (!file.isEmpty())
			set.insert(file);
	}

	// remove duplicates
	return QList<QString>::fromSet(set);
}

QString const externalLineEnding(docstring const & str)
{
#ifdef Q_WS_MACX
	// The MAC clipboard uses \r for lineendings, and we use \n
	return toqstr(subst(str, '\n', '\r'));
#elif defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
	// Windows clipboard uses \r\n for lineendings, and we use \n
	return toqstr(subst(str, from_ascii("\n"), from_ascii("\r\n")));
#else
	return toqstr(str);
#endif
}


docstring const internalLineEnding(QString const & str)
{
	docstring const s = subst(qstring_to_ucs4(str), 
				  from_ascii("\r\n"), from_ascii("\n"));
	return subst(s, '\r', '\n');
}


QString internalPath(const QString & str)
{
	return toqstr(os::internal_path(fromqstr(str)));
}


QString onlyFileName(const QString & str)
{
	return toqstr(support::onlyFileName(fromqstr(str)));
}


QString onlyPath(const QString & str)
{
	return toqstr(support::onlyPath(fromqstr(str)));
}


QString changeExtension(QString const & oldname, QString const & ext)
{
	return toqstr(support::changeExtension(fromqstr(oldname), fromqstr(ext)));
}

/// Remove the extension from \p name
QString removeExtension(QString const & name)
{
	return toqstr(support::removeExtension(fromqstr(name)));
}

/** Add the extension \p ext to \p name.
 Use this instead of changeExtension if you know that \p name is without
 extension, because changeExtension would wrongly interpret \p name if it
 contains a dot.
 */
QString addExtension(QString const & name, QString const & ext)
{
	return toqstr(support::addExtension(fromqstr(name), fromqstr(ext)));
}

/// Return the extension of the file (not including the .)
QString getExtension(QString const & name)
{
	return toqstr(support::getExtension(fromqstr(name)));
}


/** Convert relative path into absolute path based on a basepath.
  If relpath is absolute, just use that.
  If basepath doesn't exist use CWD.
  */
QString makeAbsPath(QString const & relpath, QString const & base)
{
	return toqstr(support::makeAbsPath(fromqstr(relpath),
		fromqstr(base)).absFileName());
}


/////////////////////////////////////////////////////////////////////////
//
// FileFilterList
//
/////////////////////////////////////////////////////////////////////////

/** Given a string such as
 *      "<glob> <glob> ... *.{abc,def} <glob>",
 *  convert the csh-style brace expresions:
 *      "<glob> <glob> ... *.abc *.def <glob>".
 *  Requires no system support, so should work equally on Unix, Mac, Win32.
 */
static string const convert_brace_glob(string const & glob)
{
	// Matches " *.{abc,def,ghi}", storing "*." as group 1 and
	// "abc,def,ghi" as group 2, while allowing spaces in group 2.
	static lyx::regex const glob_re(" *([^ {]*)\\{([^}]+)\\}");
	// Matches "abc" and "abc,", storing "abc" as group 1,
	// while ignoring surrounding spaces.
	static lyx::regex const block_re(" *([^ ,}]+) *,? *");

	string pattern;

	string::const_iterator it = glob.begin();
	string::const_iterator const end = glob.end();
	while (true) {
		match_results<string::const_iterator> what;
		if (!regex_search(it, end, what, glob_re)) {
			// Ensure that no information is lost.
			pattern += string(it, end);
			break;
		}

		// Everything from the start of the input to
		// the start of the match.
		pattern += string(what[-1].first, what[-1].second);

		// Given " *.{abc,def}", head == "*." and tail == "abc,def".
		string const head = string(what[1].first, what[1].second);
		string const tail = string(what[2].first, what[2].second);

		// Split the ','-separated chunks of tail so that
		// $head{$chunk1,$chunk2} becomes "$head$chunk1 $head$chunk2".
		string const fmt = " " + head + "$1";
		pattern += regex_replace(tail, block_re, fmt);

		// Increment the iterator to the end of the match.
		it += distance(it, what[0].second);
	}

	return pattern;
}


struct Filter
{
	/* \param description text describing the filters.
	 * \param one or more wildcard patterns, separated by
	 * whitespace.
	 */
	Filter(docstring const & description, std::string const & globs);

	docstring const & description() const { return desc_; }

	QString toString() const;

	docstring desc_;
	std::vector<std::string> globs_;
};


Filter::Filter(docstring const & description, string const & globs)
	: desc_(description)
{
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> const separator(" ");

	// Given "<glob> <glob> ... *.{abc,def} <glob>", expand to
	//       "<glob> <glob> ... *.abc *.def <glob>"
	string const expanded_globs = convert_brace_glob(globs);

	// Split into individual globs.
	vector<string> matches;
	Tokenizer const tokens(expanded_globs, separator);
	globs_ = vector<string>(tokens.begin(), tokens.end());
}


QString Filter::toString() const
{
	QString s;

	bool const has_description = !desc_.empty();

	if (has_description) {
		s += toqstr(desc_);
		s += " (";
	}

	for (size_t i = 0; i != globs_.size(); ++i) {
		if (i > 0)
			s += ' ';
		s += toqstr(globs_[i]);
	}

	if (has_description)
		s += ')';
	return s;
}


/** \c FileFilterList parses a Qt-style list of available file filters
 *  to generate the corresponding vector.
 *  For example "TeX documents (*.tex);;LyX Documents (*.lyx)"
 *  will be parsed to fill a vector of size 2, whilst "*.{p[bgp]m} *.pdf"
 *  will result in a vector of size 1 in which the description field is empty.
 */
struct FileFilterList
{
	// FIXME UNICODE: globs_ should be unicode...
	/** \param qt_style_filter a list of available file filters.
	 *  Eg. "TeX documents (*.tex);;LyX Documents (*.lyx)".
	 *  The "All files (*)" filter is always added to the list.
	 */
	explicit FileFilterList(docstring const & qt_style_filter =
				docstring());

	typedef std::vector<Filter>::size_type size_type;

	bool empty() const { return filters_.empty(); }
	size_type size() const { return filters_.size(); }
	Filter & operator[](size_type i) { return filters_[i]; }
	Filter const & operator[](size_type i) const { return filters_[i]; }

	void parse_filter(std::string const & filter);
	std::vector<Filter> filters_;
};


FileFilterList::FileFilterList(docstring const & qt_style_filter)
{
	// FIXME UNICODE
	string const filter = to_utf8(qt_style_filter)
		+ (qt_style_filter.empty() ? string() : ";;")
		+ to_utf8(_("All Files "))
#if defined(_WIN32)		
		+ ("(*.*)");
#else
		+ ("(*)");
#endif

	// Split data such as "TeX documents (*.tex);;LyX Documents (*.lyx)"
	// into individual filters.
	static lyx::regex const separator_re(";;");

	string::const_iterator it = filter.begin();
	string::const_iterator const end = filter.end();
	while (true) {
		match_results<string::const_iterator> what;

		if (!lyx::regex_search(it, end, what, separator_re)) {
			parse_filter(string(it, end));
			break;
		}

		// Everything from the start of the input to
		// the start of the match.
		parse_filter(string(what[-1].first, what[-1].second));

		// Increment the iterator to the end of the match.
		it += distance(it, what[0].second);
	}
}


void FileFilterList::parse_filter(string const & filter)
{
	// Matches "TeX documents (plain) (*.tex)",
	// storing "TeX documents (plain) " as group 1 and "*.tex" as group 2.
	static lyx::regex const filter_re("(.*)\\(([^()]+)\\) *$");

	match_results<string::const_iterator> what;
	if (!lyx::regex_search(filter, what, filter_re)) {
		// Just a glob, no description.
		filters_.push_back(Filter(docstring(), trim(filter)));
	} else {
		// FIXME UNICODE
		docstring const desc = from_utf8(string(what[1].first, what[1].second));
		string const globs = string(what[2].first, what[2].second);
		filters_.push_back(Filter(trim(desc), trim(globs)));
	}
}


/** \returns the equivalent of the string passed in
 *  although any brace expressions are expanded.
 *  (E.g. "*.{png,jpg}" -> "*.png *.jpg")
 */
QStringList fileFilters(QString const & desc)
{
	// we have: "*.{gif,png,jpg,bmp,pbm,ppm,tga,tif,xpm,xbm}"
	// but need:  "*.cpp;*.cc;*.C;*.cxx;*.c++"
	FileFilterList filters(qstring_to_ucs4(desc));
	//LYXERR0("DESC: " << desc);
	QStringList list;
	for (size_t i = 0; i != filters.filters_.size(); ++i) {
		QString f = filters.filters_[i].toString();
		//LYXERR0("FILTER: " << f);
		list.append(f);
	}
	return list;
}


QString guiName(string const & type, BufferParams const & bp)
{
	if (type == "tableofcontents")
		return qt_("Table of Contents");
	if (type == "child")
		return qt_("Child Documents");
	if (type == "graphics")
		return qt_("Graphics");
	if (type == "equation")
		return qt_("Equations");
	if (type == "footnote")
		return qt_("Footnotes");
	if (type == "listing")
		return qt_("Listings");
	if (type == "index")
		return qt_("Index Entries");
	if (type == "marginalnote")
		return qt_("Marginal notes");
	if (type == "nomencl")
		return qt_("Nomenclature Entries");
	if (type == "note")
		return qt_("Notes");
	if (type == "citation")
		return qt_("Citations");
	if (type == "label")
		return qt_("Labels and References");
	if (type == "branch")
		return qt_("Branches");
	if (type == "change")
		return qt_("Changes");

	FloatList const & floats = bp.documentClass().floats();
	if (floats.typeExist(type))
		return qt_(floats.getType(type).listName());

	return qt_(type);
}


} // namespace lyx
