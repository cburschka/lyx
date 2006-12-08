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
 * toqstr - convert a UTF8 encoded char * into a QString
 *
 * This should not be used, since all possibly non-ASCII stuff should be
 * stored in a docstring.
 */
inline QString const toqstr(char const * str)
{
	return QString::fromUtf8(str);
}


/**
 * toqstr - convert a UTF8 encoded std::string into a QString
 *
 * This should not be used, since all possibly non-ASCII stuff should be
 * stored in a docstring.
 */
inline QString const toqstr(std::string const & str)
{
	return toqstr(str.c_str());
}


/**
 * Convert a QChar into a UCS4 character.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
inline char_type const qchar_to_ucs4(QChar const & qchar)
{
	return static_cast<char_type>(qchar.unicode());
}


/**
 * Convert a UCS4 character into a QChar.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
inline QChar const ucs4_to_qchar(char_type const ucs4)
{
	// FIXME: The following cast is not a real conversion but it work
	// for the ucs2 subrange of unicode. Instead of an assertion we should
	// return some special characters that indicates that its display is
	// not supported.
	BOOST_ASSERT(ucs4 < 65536);
	return QChar(static_cast<unsigned short>(ucs4));
}


/**
 * toqstr - convert a UCS4 encoded docstring into a QString
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to QString.
 */
#if QT_VERSION >= 0x040200
inline QString const toqstr(docstring const & ucs4)
{
	// If possible we let qt do the work, since this version does not
	// need to be superfast.
	return QString::fromUcs4(reinterpret_cast<uint const *>(ucs4.data()), ucs4.length());
}
#else
QString const toqstr(docstring const & ucs4);
#endif


/**
 * ucs4_to_qstring - convert a UCS4 encoded char_type * into a QString
 *
 * This is a hack for the painter and font metrics and should not be used
 * elsewhere. Since it uses ucs4_to_qchar it has the same limitations.
 */
inline void ucs4_to_qstring(char_type const * str, size_t ls, QString & s)
{
	int i = static_cast<int>(ls);
	s.resize(i);
	for (; --i >= 0;)
		s[i] = ucs4_to_qchar(str[i]);
}


/**
 * qstring_to_ucs4 - convert a QString into a UCS4 encoded docstring
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to docstring.
 */
docstring const qstring_to_ucs4(QString const & qstr);


/**
 * qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(char const * str, const char * comment = 0);


/**
 * qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(std::string const & str);


/**
 * fromqstr - convert a QString into a UTF8 encoded std::string
 *
 * This should not be used except for output to lyxerr, since all possibly
 * non-ASCII stuff should be stored in a docstring.
 */
std::string const fromqstr(QString const & str);

} // namespace lyx

#endif // QTHELPERS_H
