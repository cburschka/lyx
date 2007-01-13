// -*- C++ -*-
/**
 * \file qt4/GuiClipboard.C
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


namespace {

char const * const mime_type = "application/x-lyx";

}


namespace lyx {
namespace frontend {

string const GuiClipboard::getAsLyX() const
{
	lyxerr[Debug::ACTION] << "GuiClipboard::getAsLyX(): `";
	// We don't convert encodings here since the encoding of the
	// clipboard contents is specified in the data itself
	QMimeData const * source =
		qApp->clipboard()->mimeData(QClipboard::Clipboard);
	if (!source) {
		lyxerr[Debug::ACTION] << "' (no QMimeData)" << endl;
		return string();
	}
	if (source->hasFormat(mime_type)) {
		// data from ourself or some other LyX instance
		QByteArray const ar = source->data(mime_type);
		string const s(ar.data(), ar.count());
		if (lyxerr.debugging(Debug::ACTION))
			lyxerr[Debug::ACTION] << s << "'" << endl;
		return s;
	}
	lyxerr[Debug::ACTION] << "'" << endl;
	return string();
}


docstring const GuiClipboard::getAsText() const
{
	// text data from other applications
	QString const str = qApp->clipboard()->text(QClipboard::Clipboard);
	if (lyxerr.debugging(Debug::ACTION))
		lyxerr[Debug::ACTION] << "GuiClipboard::getAsText(): `"
		                      << fromqstr(str) << "'" << endl;
	if (str.isNull())
		return docstring();

	return internalLineEnding(qstring_to_ucs4(str));
}


void GuiClipboard::put(string const & lyx, docstring const & text)
{
	if (lyxerr.debugging(Debug::ACTION))
		lyxerr[Debug::ACTION] << "GuiClipboard::put(`" << lyx << "' `"
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
	return qApp->clipboard()->ownsClipboard();
}


bool GuiClipboard::empty() const
{
	return qApp->clipboard()->text(QClipboard::Clipboard).isEmpty();
}

} // namespace frontend
} // namespace lyx
