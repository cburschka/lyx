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
#include <QString>

#include <string>

#include "support/lstrings.h"
using lyx::support::internalLineEnding;
using lyx::support::externalLineEnding;

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

#ifdef Q_WS_X11
QClipboard::Mode const CLIPBOARD_MODE = QClipboard::Selection;
#else
// FIXME external clipboard support is mostly broken for windows
// because the following fixe would involves too much side effects WRT mouse selection.
//QClipboard::Mode const CLIPBOARD_MODE = QClipboard::Clipboard;
QClipboard::Mode const CLIPBOARD_MODE = QClipboard::Selection;
#endif

void GuiClipboard::haveSelection(bool own)
{
	if (!qApp->clipboard()->supportsSelection())
		return;

	if (own) {
		qApp->clipboard()->setText(QString(), CLIPBOARD_MODE);
	}
	// We don't need to do anything if own = false, as this case is
	// handled by QT.
}


string const GuiClipboard::get() const
{
	QString str = qApp->clipboard()->text(CLIPBOARD_MODE);
	lyxerr[Debug::ACTION] << "getClipboard: " << (const char*) str << endl;
	if (str.isNull())
		return string();

	return internalLineEnding(fromqstr(str));
}


void GuiClipboard::put(string const & str)
{
	lyxerr[Debug::ACTION] << "putClipboard: " << str << endl;

	qApp->clipboard()->setText(toqstr(externalLineEnding(str)), CLIPBOARD_MODE);
}

} // namespace frontend
} // namespace lyx
