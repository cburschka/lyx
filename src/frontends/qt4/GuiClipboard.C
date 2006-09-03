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

namespace lyx {
namespace frontend {

docstring const GuiClipboard::get() const
{
	QString const str = qApp->clipboard()->text(QClipboard::Clipboard);
	lyxerr[Debug::ACTION] << "GuiClipboard::get: " << fromqstr(str)
	                      << endl;
	if (str.isNull())
		return docstring();

	return internalLineEnding(qstring_to_ucs4(str));
}


void GuiClipboard::put(docstring const & str)
{
	lyxerr[Debug::ACTION] << "GuiClipboard::put: " << lyx::to_utf8(str) << endl;

	qApp->clipboard()->setText(toqstr(externalLineEnding(str)),
	                           QClipboard::Clipboard);
}

} // namespace frontend
} // namespace lyx
