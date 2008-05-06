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

#include "LengthCombo.h"
#include "qt_helpers.h"

#include "lengthcommon.h"
#include "gettext.h"

#include "support/os.h"
#include "support/lstrings.h"
#include "support/convert.h"

#include "debug.h"

#include <QComboBox>
#include <QCheckBox>
#include <qlineedit.h>
#include <qtextcodec.h>

#include <algorithm>


namespace lyx {

using support::isStrDbl;

using std::vector;
using std::make_pair;
using std::string;
using std::pair;
using std::endl;


string makeFontName(string const & family, string const & foundry)
{
	if (foundry.empty())
		return family;
	return family + " [" + foundry + ']';
}


pair<string, string> parseFontName(string const & name)
{
	string::size_type const idx = name.find('[');
	if (idx == string::npos || idx == 0)
		return make_pair(name, string());
	return make_pair(name.substr(0, idx - 1),
			 name.substr(idx + 1, name.size() - idx - 2));
}


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


namespace {

void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
		     Length const & len)
{
	combo->setCurrentItem(Length(len).unit());
	input->setText(toqstr(convert<string>(Length(len).value())));
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
		lengthToWidgets(input, combo, Length(len));
	}
}


void lengthAutoToWidgets(QLineEdit * input, LengthCombo * combo,
	Length const & len, Length::UNIT defaultUnit)
{
	if (len.value() == 0)
		lengthToWidgets(input, combo, "auto", defaultUnit);
	else
		lengthToWidgets(input, combo, len);
}


//NOTE "CB" here because we probably will want one of these
//for labeled sets, as well.
void setAutoTextCB(QCheckBox * checkBox, QLineEdit * lineEdit,
	LengthCombo * lengthCombo)
{
	if (!checkBox->isChecked())
		lengthToWidgets(lineEdit, lengthCombo,
				"auto", lengthCombo->currentLengthItem());
	else if (lineEdit->text() == "auto")
		lengthToWidgets(lineEdit, lengthCombo, string(),
				lengthCombo->currentLengthItem());
}


QString const qt_(char const * str, const char *)
{
	return toqstr(_(str));
}


QString const qt_(string const & str)
{
	return toqstr(_(str));
}


docstring const formatted(docstring const & text, int w)
{
	docstring sout;

	if (text.empty())
		return sout;

	docstring::size_type curpos = 0;
	docstring line;

	for (;;) {
		docstring::size_type const nxtpos1 = text.find(' ',  curpos);
		docstring::size_type const nxtpos2 = text.find('\n', curpos);
		docstring::size_type const nxtpos = std::min(nxtpos1, nxtpos2);

		docstring const word =
			nxtpos == docstring::npos ?
			text.substr(curpos) :
			text.substr(curpos, nxtpos - curpos);

		bool const newline = (nxtpos2 != docstring::npos &&
				      nxtpos2 < nxtpos1);

		docstring const line_plus_word =
			line.empty() ? word : line + char_type(' ') + word;

		// FIXME: make w be size_t
		if (int(line_plus_word.length()) >= w) {
			sout += line + char_type('\n');
			if (newline) {
				sout += word + char_type('\n');
				line.erase();
			} else {
				line = word;
			}

		} else if (newline) {
			sout += line_plus_word + char_type('\n');
			line.erase();

		} else {
			if (!line.empty())
				line += char_type(' ');
			line += word;
		}

		if (nxtpos == docstring::npos) {
			if (!line.empty())
				sout += line;
			break;
		}

		curpos = nxtpos + 1;
	}

	return sout;
}

} // namespace lyx
