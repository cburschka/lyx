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
using std::string;

namespace lyx {
namespace frontend {

string const GuiSelection::get() const
{
	QString const str = qApp->clipboard()->text(QClipboard::Selection);
	lyxerr[Debug::ACTION] << "GuiSelection::get: " << (const char*) str
	                      << endl;
	if (str.isNull())
		return string();

	return internalLineEnding(fromqstr(str));
}


void GuiSelection::put(string const & str)
{
	lyxerr[Debug::ACTION] << "GuiSelection::put: " << str << endl;

	qApp->clipboard()->setText(toqstr(externalLineEnding(str)),
	                           QClipboard::Selection);
}

} // namespace frontend
} // namespace lyx
