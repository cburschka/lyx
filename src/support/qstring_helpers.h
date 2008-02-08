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

#include <QString>
#include <QVector>

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
 * toqstr - convert a UCS4 encoded docstring into a QString
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to QString.
 */
inline QString const toqstr(docstring const & ucs4)
{
	// If possible we let qt do the work, since this version does not
	// need to be superfast.
	return QString::fromUcs4(reinterpret_cast<uint const *>(ucs4.data()), ucs4.length());
}

/**
 * toqstr - convert a UCS4 encoded character into a QString
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to QString.
 */
inline QString const toqstr(char_type ucs4)
{
	return QString::fromUcs4(reinterpret_cast<uint const *>(&ucs4), 1);
}

/**
 * qstring_to_ucs4 - convert a QString into a UCS4 encoded docstring
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to docstring.
 */
inline docstring const qstring_to_ucs4(QString const & qstr)
{
	if (qstr.isEmpty())
		return docstring();
	QVector<uint> const ucs4 = qstr.toUcs4();
	return docstring((char_type const *)(ucs4.constData()), ucs4.size());
}

/**
 * fromqstr - convert a QString into a UTF8 encoded std::string
 *
 * This should not be used except for output to lyxerr, since all possibly
 * non-ASCII stuff should be stored in a docstring.
 */
inline std::string const fromqstr(QString const & str)
{
	return str.isEmpty() ? std::string() : std::string(str.toUtf8());
}

} // namespace lyx

#endif // QSTRING_HELPERS_H
