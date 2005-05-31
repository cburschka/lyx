/**
 * \file validators.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "validators.h"
#include "qt_helpers.h"

#include "gettext.h"
#include "lyxrc.h"

#include "frontends/Alert.h"

#include "frontends/controllers/Dialog.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"

#include <qlineedit.h>
#include <qwidget.h>

#include <sstream>

using lyx::support::isStrDbl;
using std::string;


LengthValidator::LengthValidator(QWidget * parent, const char * name)
	: QValidator(parent, name),
	  no_bottom_(true), glue_length_(false)
{}


QValidator::State LengthValidator::validate(QString & qtext, int &) const
{
	string const text = fromqstr(qtext);
	if (text.empty() || isStrDbl(text))
		return QValidator::Acceptable;

	if (glue_length_) {
		LyXGlueLength gl;
		return (isValidGlueLength(text, &gl)) ?
			QValidator::Acceptable : QValidator::Intermediate;
		}

	LyXLength l;
	bool const valid_length = isValidLength(text, &l);
	if (!valid_length)
		return QValidator::Intermediate;

	if (no_bottom_)
		return QValidator::Acceptable;

	return b_.inPixels(100) <= l.inPixels(100) ?
		QValidator::Acceptable : QValidator::Intermediate;
}


void LengthValidator::setBottom(LyXLength const & b)
{
	b_ = b;
	no_bottom_ = false;
}


void LengthValidator::setBottom(LyXGlueLength const & g)
{
	g_ = g;
	no_bottom_ = false;
	glue_length_ = true;
}


LengthValidator * unsignedLengthValidator(QLineEdit * ed)
{
	LengthValidator * v = new LengthValidator(ed);
	v->setBottom(LyXLength());
	return v;
}


PathValidator::PathValidator(bool acceptable_if_empty,
			     QWidget * parent, const char * name)
	: QValidator(parent, name),
	  acceptable_if_empty_(acceptable_if_empty),
	  latex_doc_(false),
	  tex_allows_spaces_(false)
{}


namespace {

string const printable_list(string const & invalid_chars)
{
	std::ostringstream ss;
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

	return ss.str();
}

} // namespace anon


QValidator::State PathValidator::validate(QString & qtext, int &) const
{
	if (!latex_doc_)
		return QValidator::Acceptable;

	string const text = lyx::support::trim(fromqstr(qtext));
	if (text.empty())
		return 	acceptable_if_empty_ ?
			QValidator::Acceptable : QValidator::Intermediate;

	string invalid_chars("#$%{}()[]\"^");
	if (!tex_allows_spaces_)
		invalid_chars += ' ';

	if (text.find_first_of(invalid_chars) != string::npos) {

		static int counter = 0;
		if (counter == 0) {
			Alert::error(_("Invalid filename"),
				     _("LyX does not provide LateX support for file names containing any of these characters:\n") +
				     printable_list(invalid_chars));
		}
		++counter;
		return QValidator::Intermediate;
	}

	return QValidator::Acceptable;
}


void PathValidator::setChecker(lyx::frontend::KernelDocType const & type,
			       LyXRC const & lyxrc)
{
	latex_doc_ = type == lyx::frontend::Kernel::LATEX;
	tex_allows_spaces_ = lyxrc.tex_allows_spaces;
}


PathValidator * getPathValidator(QLineEdit * ed)
{
	if (!ed)
		return 0;
	QValidator * validator = const_cast<QValidator *>(ed->validator());
	if (!validator)
		return 0;
	return dynamic_cast<PathValidator *>(validator);
}
