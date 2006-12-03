// -*- C++ -*-
/**
 * \file qt_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHELPERS_H
#define QTHELPERS_H

#include "lyxlength.h"
#include "support/docstring.h"

#include <QChar>
#include <QString>

#include <vector>
#include <utility>
#include <boost/assert.hpp>

class QComboBox;
class QLineEdit;

class LengthCombo;

namespace lyx {

std::string makeFontName(std::string const & family, std::string const & foundry);

std::pair<std::string,std::string> parseFontName(std::string const & name);

/// method to get a LyXLength from widgets (LengthCombo)
std::string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
/// method to get a LyXLength from widgets (QComboBox)
LyXLength widgetsToLength(QLineEdit const * input, QComboBox const * combo);

/// method to set widgets from a LyXLength
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	std::string const & len, LyXLength::UNIT default_unit);

/// format a string to the given width
docstring const formatted(docstring const & text, int w = 80);

/**
 * toqstr - convert char * into Qt's unicode (UTF16)
 *
 * Use this whenever there's a user-visible string that is encoded
 * for the locale (menus, dialogs etc.)
 */
inline QString const toqstr(char const * str)
{
	return QString::fromUtf8(str);
}


/**
 * toqstr - convert string into unicode
 *
 * Use this whenever there's a user-visible string that is encoded
 * for the locale (menus, dialogs etc.)
 */
inline QString const toqstr(std::string const & str)
{
	return toqstr(str.c_str());
}


/**
 * toqstr - convert ucs4 into QString
 *
 * QString uses utf16 internally.
 */
inline char_type const qchar_to_ucs4(QChar const & qchar) {
	return static_cast<char_type>(qchar.unicode());
}

inline QChar const ucs4_to_qchar(char_type const ucs4) {
	// FIXME: The following cast is not a real conversion but it work
	// for the ucs2 subrange of unicode. Instead of an assertion we should
	// return some special characters that indicates that its display is
	// not supported.
	BOOST_ASSERT(ucs4 < 65536);
	return QChar(static_cast<unsigned short>(ucs4));
}

QString const toqstr(docstring const & ucs4);

void ucs4_to_qstring(docstring const & str, QString & s);

inline void ucs4_to_qstring(char_type const * str, size_t ls, QString & s)
{
	int i = static_cast<int>(ls);
	s.resize(i);
	for (; --i >= 0;)
		s[i] = ucs4_to_qchar(str[i]);
}


docstring const qstring_to_ucs4(QString const & qstr);


/**
 * qt_ - i18nize string and convert to unicode
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(char const * str, const char * comment = 0);


/**
 * qt_ - i18nize string and convert to unicode
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(std::string const & str);


/**
 * fromqstr - convert QString into std::string in locale
 *
 * Return the QString encoded in the locale
 */
std::string const fromqstr(QString const & str);


} // namespace lyx

#endif // QTHELPERS_H
