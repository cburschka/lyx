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

#include "support/lstrings.h"
using lyx::support::internalLineEnding;
using lyx::support::externalLineEnding;

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

string const GuiClipboard::get() const
{
	QString const str = qApp->clipboard()->text(QClipboard::Clipboard);
	lyxerr[Debug::ACTION] << "GuiClipboard::get: " << fromqstr(str)
	                      << endl;
	if (str.isNull())
		return string();

	return internalLineEnding(fromqstr(str));
}


void GuiClipboard::put(string const & str)
{
	lyxerr[Debug::ACTION] << "GuiClipboard::put: " << str << endl;

	qApp->clipboard()->setText(toqstr(externalLineEnding(str)),
	                           QClipboard::Clipboard);
}

} // namespace frontend
} // namespace lyx
