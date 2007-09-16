// -*- C++ -*-
/**
 * \file qstring_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSTRING_HELPERS_H
#define QSTRING_HELPERS_H

#include "support/docstring.h"

#include <QChar>
#include <QString>

#include <boost/assert.hpp>

namespace lyx {

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


/// Is \p c a valid utf16 char?
inline bool is_utf16(char_type c)
{
	// 0xd800 ... 0xdfff is the range of surrogate pairs.
	return c < 0xd800 || (c > 0xdfff && c < 0x10000);
}


/**
 * Convert a QChar into a UCS4 character.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
inline char_type qchar_to_ucs4(QChar const & qchar)
{
	BOOST_ASSERT(is_utf16(static_cast<char_type>(qchar.unicode())));
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
	BOOST_ASSERT(is_utf16(ucs4));
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
 * qstring_to_ucs4 - convert a QString into a UCS4 encoded docstring
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to docstring.
 */
docstring const qstring_to_ucs4(QString const & qstr);


/**
 * fromqstr - convert a QString into a UTF8 encoded std::string
 *
 * This should not be used except for output to lyxerr, since all possibly
 * non-ASCII stuff should be stored in a docstring.
 */
std::string const fromqstr(QString const & str);

} // namespace lyx

#endif // QSTRING_HELPERS_H
