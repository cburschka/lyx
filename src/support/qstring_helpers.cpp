/**
 * \file qstring_helper.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of unicode conversion functions, using iconv.
 */

#include <config.h>

#include "support/docstring.h"

#include <QString>
#include <QVector>

namespace lyx {

QString toqstr(char const * str)
{
	return QString::fromUtf8(str);
}

QString toqstr(std::string const & str)
{
	return toqstr(str.c_str());
}


QString toqstr(docstring const & ucs4)
{
	// If possible we let qt do the work, since this version does not
	// need to be superfast.
	if (ucs4.empty())
		return QString();
	return QString::fromUcs4((uint const *)ucs4.data(), ucs4.length());
}

QString toqstr(char_type ucs4)
{
	union { char_type c; uint i; } u = { ucs4 };
	return QString::fromUcs4(&u.i, 1);
}

docstring qstring_to_ucs4(QString const & qstr)
{
	if (qstr.isEmpty())
		return docstring();
	QVector<uint> const ucs4 = qstr.toUcs4();
	return docstring((char_type const *)(ucs4.constData()), ucs4.size());
}

std::string fromqstr(QString const & str)
{
	return str.isEmpty() ? std::string() : std::string(str.toUtf8());
}

} // namespace lyx
