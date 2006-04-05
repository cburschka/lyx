/**
 * \file QPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QPrefs.h"
#include "QPrefsDialog.h"

#include "lcolorcache.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"
#include "LColor.h"
#include "lyxfont.h"

#include "support/lstrings.h"
#include "support/os.h"

#include "controllers/ControlPrefs.h"
#include "controllers/frnt_lang.h"
#include "controllers/helper_funcs.h"

#include "frontends/lyx_gui.h"

using namespace Ui;

namespace lyx {
namespace frontend {

typedef QController<ControlPrefs, QView<QPrefsDialog> > base_class;

QPrefs::QPrefs(Dialog & parent)
	: base_class(parent, _("Preferences"))
{

}

Converters & QPrefs::converters()
{
	return controller().converters();
}

Formats & QPrefs::formats()
{
	return controller().formats();
}

Movers & QPrefs::movers()
{
	return controller().movers();
}

void QPrefs::build_dialog()
{
	dialog_.reset(new QPrefsDialog(this));
}

void QPrefs::apply()
{
	dialog_->apply(controller().rc());
}

void QPrefs::update_contents()
{
	dialog_->update(controller().rc());
}

} // namespace frontend
} // namespace lyx
