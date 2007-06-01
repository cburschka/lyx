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
#include "unicode.h"

#include <QVector>


namespace lyx {

using std::string;

#if QT_VERSION < 0x040200
// We use QString::fromUcs4 in Qt 4.2 and higher
QString const toqstr(docstring const & str)
{
	QString s;
	int i = static_cast<int>(str.size());
	s.resize(i);
	for (; --i >= 0;) {
		char_type const c = str[i];
		if (is_utf16(c))
			// Use a simple cast in the common case for speed
			// reasons
			s[i] = static_cast<unsigned short>(c);
		else {
			// A simple cast is not possible, so we need to use
			// the full blown conversion.
			std::vector<unsigned short> const utf16 =
				ucs4_to_utf16(str.data(), str.size());
			// Enable the compiler to do NRVO
			s = QString::fromUtf16(&utf16[0], utf16.size());
			break;
		}
	}
	return s;
}
#endif


docstring const qstring_to_ucs4(QString const & qstr)
{
#if QT_VERSION >= 0x040200
	QVector<uint> const ucs4 = qstr.toUcs4();
	return docstring(ucs4.begin(), ucs4.end());
#else
	int const ls = qstr.size();
	docstring ucs4;
	for (int i = 0; i < ls; ++i) {
		char_type const c = static_cast<char_type>(qstr[i].unicode());
		if (is_utf16(c))
			// Use a simple cast in the common case for speed
			// reasons
			ucs4 += c;
		else {
			// A simple cast is not possible, so we need to use
			// the full blown conversion.
			std::vector<char_type> const v = utf16_to_ucs4(
				reinterpret_cast<unsigned short const *>(qstr.utf16()),
				qstr.size());
			// Enable the compiler to do NRVO
			ucs4 = docstring(v.begin(), v.end());
			break;
		}
	}
	return ucs4;
#endif
}


string const fromqstr(QString const & str)
{
	return str.isEmpty() ? string() : string(str.toUtf8());
}

} // namespace lyx
