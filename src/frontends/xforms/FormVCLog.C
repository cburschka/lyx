/**
 * \file FormVCLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include "Lsstream.h"


#include "xformsBC.h"
#include "ControlVCLog.h"
#include "FormVCLog.h"
#include "forms/form_browser.h"
#include "gettext.h"
#include "lyx_forms.h"


FormVCLog::FormVCLog(Dialog & parent)
	: FormController<ControlVCLog, FormBrowser>(parent, _("Version Control Log"))
{}


void FormVCLog::update()
{
	fl_clear_browser(dialog_->browser);

	ostringstream ss;
	controller().getVCLogFile(ss);

	fl_add_browser_line(dialog_->browser, ss.str().c_str());
}
