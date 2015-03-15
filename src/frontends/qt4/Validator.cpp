/**
 * \file Validator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "Validator.h"
#include "qt_helpers.h"

#include "support/gettext.h"
#include "LyXRC.h"

#include "frontends/alert.h"

#include "support/docstring.h"
#include "support/lstrings.h"

#include <QLineEdit>
#include <QLocale>
#include <QWidget>

using namespace std;

namespace lyx {
namespace frontend {

LengthValidator::LengthValidator(QWidget * parent)
	: QValidator(parent),
	  no_bottom_(true), glue_length_(false)
{}


QValidator::State LengthValidator::validate(QString & qtext, int &) const
{
	QLocale loc;
	bool ok;
	loc.toDouble(qtext.trimmed(), &ok);
	if (!ok) {
		// Fall back to C
		QLocale c(QLocale::C);
		c.toDouble(qtext.trimmed(), &ok);
	}

	if (qtext.isEmpty() || ok)
		return QValidator::Acceptable;

	string const text = fromqstr(qtext);

	if (glue_length_) {
		GlueLength gl;
		return (isValidGlueLength(text, &gl)) ?
			QValidator::Acceptable : QValidator::Intermediate;
	}

	Length l;
	bool const valid_length = isValidLength(text, &l);
	if (!valid_length)
		return QValidator::Intermediate;

	if (no_bottom_)
		return QValidator::Acceptable;

	return b_.inPixels(100) <= l.inPixels(100) ?
		QValidator::Acceptable : QValidator::Intermediate;
}


void LengthValidator::setBottom(Length const & b)
{
	b_ = b;
	no_bottom_ = false;
}


void LengthValidator::setBottom(GlueLength const & g)
{
	g_ = g;
	no_bottom_ = false;
	glue_length_ = true;
}


LengthValidator * unsignedLengthValidator(QLineEdit * ed)
{
	LengthValidator * v = new LengthValidator(ed);
	v->setBottom(Length());
	return v;
}


LengthValidator * unsignedGlueLengthValidator(QLineEdit * ed)
{
	LengthValidator * v = new LengthValidator(ed);
	v->setBottom(GlueLength());
	return v;
}


LengthAutoValidator::LengthAutoValidator(QWidget * parent, QString const & autotext)
	: LengthValidator(parent),
	  autotext_(autotext)
{}


QValidator::State LengthAutoValidator::validate(QString & qtext, int & dummy) const
{
	if (qtext == autotext_)
		return QValidator::Acceptable;
	return LengthValidator::validate(qtext, dummy);
}


LengthAutoValidator * unsignedLengthAutoValidator(QLineEdit * ed, QString const & autotext)
{
	LengthAutoValidator * v = new LengthAutoValidator(ed, autotext);
	v->setBottom(Length());
	return v;
}


DoubleAutoValidator::DoubleAutoValidator(QWidget * parent, QString const & autotext)
	: QDoubleValidator(parent),
	  autotext_(autotext)
{}


DoubleAutoValidator::DoubleAutoValidator(double bottom,
		double top, int decimals, QObject * parent)
	: QDoubleValidator(bottom, top, decimals, parent)
{}


QValidator::State DoubleAutoValidator::validate(QString & input, int & pos) const {
	if (input == autotext_)
		return QValidator::Acceptable;
	return QDoubleValidator::validate(input, pos);
}


NoNewLineValidator::NoNewLineValidator(QWidget * parent)
	: QValidator(parent)
{}


QValidator::State NoNewLineValidator::validate(QString & qtext, int &) const
{
	qtext.remove(QRegExp("[\\n\\r]"));
	return QValidator::Acceptable;
}


PathValidator::PathValidator(bool acceptable_if_empty,
			     QWidget * parent)
	: QValidator(parent),
	  acceptable_if_empty_(acceptable_if_empty),
	  latex_doc_(false),
	  tex_allows_spaces_(false)
{}


static docstring const printable_list(docstring const & invalid_chars)
{
	docstring s;
	docstring::const_iterator const begin = invalid_chars.begin();
	docstring::const_iterator const end = invalid_chars.end();
	docstring::const_iterator it = begin;

	for (; it != end; ++it) {
		if (it != begin)
			s += ", ";
		if (*it == ' ')
			s += _("space");
		else
			s += *it;
	}

	return s;
}


QValidator::State PathValidator::validate(QString & qtext, int &) const
{
	if (!latex_doc_)
		return QValidator::Acceptable;

	docstring const text = support::trim(qstring_to_ucs4(qtext));
	if (text.empty())
		return acceptable_if_empty_ ?
			QValidator::Acceptable : QValidator::Intermediate;

	docstring invalid_chars = from_ascii("#$%{}()[]\"^");
	if (!tex_allows_spaces_)
		invalid_chars += ' ';

	if (text.find_first_of(invalid_chars) != docstring::npos) {

		static int counter = 0;
		if (counter == 0) {
			Alert::error(_("Invalid filename"),
				     _("LyX does not provide LaTeX support for file names containing any of these characters:\n") +
					 printable_list(invalid_chars));
		}
		++counter;
		return QValidator::Intermediate;
	}

	return QValidator::Acceptable;
}


void PathValidator::setChecker(KernelDocType const & type, LyXRC const & lyxrc)
{
	latex_doc_ = type == LATEX;
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

} // namespace frontend
} // namespace lyx

#include "moc_Validator.cpp"
