/**
 * \file qt_helpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/tostr.h"
#include "gettext.h"
#include "qt_helpers.h"

#include "lengthcombo.h"

#include <qlineedit.h>
#include <qtextcodec.h>


using std::pair;
using std::make_pair;
using std::min;

string makeFontName(string const & family, string const & foundry)
{
	if (foundry.empty())
		return family;
#if QT_VERSION  >= 300
	return family + " [" + foundry + ']';
#else
	return foundry + '-' + family;
#endif
}


pair<string,string> parseFontName(string const & name)
{
#if QT_VERSION  >= 300
	string::size_type const idx = name.find('[');
	if (idx == string::npos || idx == 0)
		return make_pair(name, string());
	return make_pair(name.substr(0, idx - 1),
			 name.substr(idx + 1, name.size() - idx - 2));
#else
	string::size_type const idx = name.find('-');
	if (idx == string::npos || idx == 0)
		return make_pair(name, string());
	return make_pair(name.substr(idx + 1),
			 name.substr(0, idx));
#endif
}


string widgetsToLength(QLineEdit const * input, LengthCombo const * combo)
{
	QString length = input->text();
	if (length.isEmpty())
		return string();

	// don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(fromqstr(length)))
		return fromqstr(length);

	LyXLength::UNIT unit = combo->currentLengthItem();

	return LyXLength(length.toDouble(), unit).asString();
}


void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	string const & len, LyXLength::UNIT defaultUnit)
{
	if (len.empty()) {
		// no length (UNIT_NONE)
		combo->setCurrentItem(defaultUnit);
		input->setText("");
	} else {
		combo->setCurrentItem(LyXLength(len).unit());
		input->setText(toqstr(tostr(LyXLength(len).value())));
	}
}


QString const toqstr(char const * str)
{
	QTextCodec * codec = QTextCodec::codecForLocale();

	return codec->toUnicode(str);
}


QString const toqstr(string const & str)
{
	return toqstr(str.c_str());
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
	QTextCodec * codec = QTextCodec::codecForLocale();
	QCString tmpstr = codec->fromUnicode(str);
	char const * tmpcstr = tmpstr;
	return tmpcstr;
}


string const formatted(string const & text, int w)
{
	string sout;

	if (text.empty())
		return sout;

	string::size_type curpos = 0;
	string line;

	for (;;) {
		string::size_type const nxtpos1 = text.find(' ',  curpos);
		string::size_type const nxtpos2 = text.find('\n', curpos);
		string::size_type const nxtpos = std::min(nxtpos1, nxtpos2);

		string const word = nxtpos == string::npos ?
			text.substr(curpos) : text.substr(curpos, nxtpos-curpos);

		bool const newline = (nxtpos2 != string::npos &&
				      nxtpos2 < nxtpos1);

		string const line_plus_word =
			line.empty() ? word : line + ' ' + word;

		// FIXME: make w be size_t
		if (line_plus_word.length() >= w) {
			sout += line + '\n';
			if (newline) {
				sout += word + '\n';
				line.erase();
			} else {
				line = word;
			}

		} else if (newline) {
			sout += line_plus_word + '\n';
			line.erase();

		} else {
			if (!line.empty())
				line += ' ';
			line += word;
		}

		if (nxtpos == string::npos) {
			if (!line.empty())
				sout += line;
			break;
		}

		curpos = nxtpos + 1;
	}

	return sout;
}
