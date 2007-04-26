/**
 * \file qstring_helpers.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qstring_helpers.h"

#include <QVector>


namespace lyx {

using std::string;

#if QT_VERSION < 0x040200
// We use QString::fromUcs4 in Qt 4.2 and higher
QString const toqstr(docstring const & str)
{
	// This does not properly convert surrogate pairs
	QString s;
	int i = static_cast<int>(str.size()); 
	s.resize(i);
	for (; --i >= 0;)
		s[i] = ucs4_to_qchar(str[i]);
	return s;
}
#endif


docstring const qstring_to_ucs4(QString const & qstr)
{
#if QT_VERSION >= 0x040200
	QVector<uint> const ucs4 = qstr.toUcs4();
	return docstring(ucs4.begin(), ucs4.end());
#else
	// This does not properly convert surrogate pairs
	int const ls = qstr.size();
	docstring ucs4;
	for (int i = 0; i < ls; ++i)
		ucs4 += qchar_to_ucs4(qstr[i].unicode());
	return ucs4;
#endif
}


string const fromqstr(QString const & str)
{
	return str.isEmpty() ? string() : string(str.toUtf8());
}

} // namespace lyx
