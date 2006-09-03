// -*- C++ -*-
/**
 * \file qt4/GuiSelection.C
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

#include <QApplication>
#include <QClipboard>
#include <QString>

#include "support/lstrings.h"
using lyx::support::internalLineEnding;
using lyx::support::externalLineEnding;

using std::endl;

namespace lyx {
namespace frontend {

void GuiSelection::haveSelection(bool own)
{
	if (!qApp->clipboard()->supportsSelection())
		return;

	if (own)
		qApp->clipboard()->setText(QString(), QClipboard::Selection);
	// We don't need to do anything if own = false, as this case is
	// handled by QT.
}


docstring const GuiSelection::get() const
{
	QString const str = qApp->clipboard()->text(QClipboard::Selection);
	lyxerr[Debug::ACTION] << "GuiSelection::get: " << fromqstr(str)
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

} // namespace frontend
} // namespace lyx
