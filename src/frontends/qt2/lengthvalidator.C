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

#include "support/lstrings.h"

#include "qt_helpers.h"


using lyx::support::isStrDbl;
using std::string;


LengthValidator::LengthValidator(QWidget * parent, const char * name)
	: QValidator(parent, name),
	  no_bottom_(true)
{}


QValidator::State LengthValidator::validate(QString & qtext, int &) const
{
	string const text = fromqstr(qtext);
	if (text.empty() || isStrDbl(text))
		return QValidator::Acceptable;

	LyXLength l;
	bool const valid_length = isValidLength(text, &l);
	if (!valid_length)
		return QValidator::Intermediate;

	if (no_bottom_)
		return QValidator::Acceptable;

	return b_.inPixels(100) <= l.inPixels(100) ?
		QValidator::Acceptable : QValidator::Intermediate;
}


void LengthValidator::setBottom(LyXLength b)
{
	b_ = b;
	no_bottom_ = false;
}
