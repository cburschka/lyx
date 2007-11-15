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

docstring const qstring_to_ucs4(QString const & qstr)
{
	QVector<uint> const ucs4 = qstr.toUcs4();
	return docstring(ucs4.begin(), ucs4.end());
}


string const fromqstr(QString const & str)
{
	return str.isEmpty() ? string() : string(str.toUtf8());
}

} // namespace lyx
