// -*- C++ -*-
/**
 * \file qt3/GuiSelection.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSelection.h"
#include "qt_helpers.h"

#include "debug.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qstring.h>

#include "support/lstrings.h"
using lyx::support::internalLineEnding;
using lyx::support::externalLineEnding;

using std::endl;

namespace lyx {
namespace frontend {

docstring const GuiSelection::get() const
{
	QString const str = qApp->clipboard()->text(QClipboard::Selection);
	lyxerr[Debug::ACTION] << "GuiSelection::get: " << (const char*) str
	                      << endl;
	if (str.isNull())
		return docstring();

	return internalLineEnding(qstring_to_ucs4(str));
}


void GuiSelection::put(docstring const & str)
{
	lyxerr[Debug::ACTION] << "GuiSelection::put: " << lyx::to_utf8(str) << endl;

	qApp->clipboard()->setText(toqstr(externalLineEnding(str)),
	                           QClipboard::Selection);
}


void GuiSelection::haveSelection(bool own)
{
	if (!QApplication::clipboard()->supportsSelection())
		return;

	if (own)
		QApplication::clipboard()->setText(QString(), QClipboard::Selection);
	// We don't need to do anything if own = false, as this case is
	// handled by QT.
}

} // namespace frontend
} // namespace lyx
