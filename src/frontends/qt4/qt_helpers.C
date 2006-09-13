/**
 * \file qt_helpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lengthcombo.h"
#include "qt_helpers.h"

#include "lengthcommon.h"
#include "gettext.h"

#include "support/lstrings.h"
#include "support/convert.h"

#include "debug.h"

#include <QComboBox>
#include <qlineedit.h>
#include <qtextcodec.h>

#include <algorithm>


using lyx::support::isStrDbl;
using lyx::char_type;
using lyx::docstring;

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

	LyXLength::UNIT const unit = combo->currentLengthItem();

	return LyXLength(length.toDouble(), unit).asString();
}


LyXLength widgetsToLength(QLineEdit const * input, QComboBox const * combo)
{
	QString const length = input->text();
	if (length.isEmpty())
		return LyXLength();

	// don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(fromqstr(length)))
		return LyXLength(fromqstr(length));

	LyXLength::UNIT const unit = unitFromString(fromqstr(combo->currentText()));

	return LyXLength(length.toDouble(), unit);
}


void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	string const & len, LyXLength::UNIT defaultUnit)
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
		combo->setCurrentItem(LyXLength(len).unit());
		input->setText(toqstr(convert<string>(LyXLength(len).value())));
	}
}


QString const toqstr(char const * str)
{
	return QString::fromUtf8(str);
}


QString const toqstr(string const & str)
{
	return toqstr(str.c_str());
}


QString const ucs4_to_qstring(char_type const * str, size_t ls)
{
	QString s;

	for (size_t i = 0; i < ls; ++i)
		s.append(ucs4_to_qchar(str[i]));

	return s;
}


QString const toqstr(docstring const & ucs4)
{
	QString s;
	size_t const ls = ucs4.size();

	for (size_t i = 0; i < ls; ++i)
		s.append(ucs4_to_qchar(ucs4[i]));

	return s;
}


docstring const qstring_to_ucs4(QString const & qstr)
{
	int const ls = qstr.size();
	docstring ucs4;
	for (int i = 0; i < ls; ++i)
		ucs4 += static_cast<char_type>(qstr[i].unicode());

	return ucs4;
}


void qstring_to_ucs4(QString const & qstr, vector<char_type> & ucs4)
{
	int const ls = qstr.size();
	ucs4.clear();
	for (int i = 0; i < ls; ++i)
		ucs4.push_back(static_cast<lyx::char_type>(qstr[i].unicode()));
}


char_type const qchar_to_ucs4(QChar const & qchar)
{
	return static_cast<lyx::char_type>(qchar.unicode());
}


QChar const ucs4_to_qchar(char_type const & ucs4)
{
	return QChar(static_cast<unsigned short>(ucs4));
}


QString const qt_(char const * str)
{
	return toqstr(_(str));
}


QString const qt_(string const & str)
{
	return toqstr(_(str));
}


string const fromqstr(QString const & str)
{
	return str.isEmpty()? string(): string(str.toAscii());
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
			line.empty() ? word : line + lyx::char_type(' ') + word;

		// FIXME: make w be size_t
		if (int(line_plus_word.length()) >= w) {
			sout += line + lyx::char_type('\n');
			if (newline) {
				sout += word + lyx::char_type('\n');
				line.erase();
			} else {
				line = word;
			}

		} else if (newline) {
			sout += line_plus_word + lyx::char_type('\n');
			line.erase();

		} else {
			if (!line.empty())
				line += lyx::char_type(' ');
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
