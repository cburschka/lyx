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

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlVCLog.h"
#include "FormVCLog.h"
#include "forms/form_browser.h"
#include "gettext.h"
#include FORMS_H_LOCATION


FormVCLog::FormVCLog()
	: FormCB<ControlVCLog, FormBrowser>(_("Version Control Log"))
{}


void FormVCLog::update()
{
	fl_clear_browser(dialog_->browser);

	ostringstream ss;
	controller().getVCLogFile(ss);

	fl_add_browser_line(dialog_->browser,
			    ss.str().c_str());
}
