// -*- C++ -*-
/**
 * \file qt4/GuiClipboard.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiClipboard.h"
#include "qt_helpers.h"

#include "debug.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QString>

#include "support/lstrings.h"

using lyx::support::internalLineEnding;
using lyx::support::externalLineEnding;

using std::endl;
using std::string;

static char const * const mime_type = "application/x-lyx";


namespace lyx {
namespace frontend {

string const GuiClipboard::getAsLyX() const
{
	LYXERR(Debug::ACTION) << "GuiClipboard::getAsLyX(): `";
	// We don't convert encodings here since the encoding of the
	// clipboard contents is specified in the data itself
	QMimeData const * source =
		qApp->clipboard()->mimeData(QClipboard::Clipboard);
	if (!source) {
		LYXERR(Debug::ACTION) << "' (no QMimeData)" << endl;
		return string();
	}
	if (source->hasFormat(mime_type)) {
		// data from ourself or some other LyX instance
		QByteArray const ar = source->data(mime_type);
		string const s(ar.data(), ar.count());
		LYXERR(Debug::ACTION) << s << "'" << endl;
		return s;
	}
	LYXERR(Debug::ACTION) << "'" << endl;
	return string();
}


docstring const GuiClipboard::getAsText() const
{
	// text data from other applications
	QString const str = qApp->clipboard()->text(QClipboard::Clipboard)
				.normalized(QString::NormalizationForm_C);
	LYXERR(Debug::ACTION) << "GuiClipboard::getAsText(): `"
			      << fromqstr(str) << "'" << endl;
	if (str.isNull())
		return docstring();

	return internalLineEnding(qstring_to_ucs4(str));
}


void GuiClipboard::put(string const & lyx, docstring const & text)
{
	LYXERR(Debug::ACTION) << "GuiClipboard::put(`" << lyx << "' `"
			      << to_utf8(text) << "')" << endl;
	// We don't convert the encoding of lyx since the encoding of the
	// clipboard contents is specified in the data itself
	QMimeData * data = new QMimeData;
	if (!lyx.empty()) {
		QByteArray const qlyx(lyx.c_str(), lyx.size());
		data->setData(mime_type, qlyx);
	}
	// Don't test for text.empty() since we want to be able to clear the
	// clipboard.
	QString const qtext = toqstr(text);
	data->setText(qtext);
	qApp->clipboard()->setMimeData(data, QClipboard::Clipboard);
}


bool GuiClipboard::hasLyXContents() const
{
	QMimeData const * const source =
		qApp->clipboard()->mimeData(QClipboard::Clipboard);
	return source && source->hasFormat(mime_type);
}


bool GuiClipboard::isInternal() const
{
	// ownsClipboard() is also true for stuff coming from dialogs, e.g.
	// the preamble dialog
	// FIXME: This does only work on X11, since ownsClipboard() is
	// hardwired to return false on Windows and OS X.
	return qApp->clipboard()->ownsClipboard() && hasLyXContents();
}


bool GuiClipboard::empty() const
{
	// We need to check both the plaintext and the LyX version of the
	// clipboard. The plaintext version is empty if the LyX version
	// contains only one inset, and the LyX version is empry if the
	// clipboard does not come from LyX.
	if (!qApp->clipboard()->text(QClipboard::Clipboard).isEmpty())
		return false;
	return !hasLyXContents();
}

} // namespace frontend
} // namespace lyx
