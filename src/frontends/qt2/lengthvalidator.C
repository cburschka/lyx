/**
 * \file lengthvalidator.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "lengthvalidator.h"
#include "qt_helpers.h"

#include "support/lstrings.h"

#include <qwidget.h>


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
