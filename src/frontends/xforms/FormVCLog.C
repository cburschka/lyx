/**
 * \file FormVCLog.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author John Levon, moz@compsoc.man.ac.uk
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

FormVCLog::FormVCLog(ControlVCLog & c, Dialogs & d)
	: FormCB<ControlVCLog, FormBrowser>(c, d, _("Version Control Log"))
{}


void FormVCLog::update()
{
	fl_clear_browser(dialog_->browser);

	stringstream ss;

	fl_add_browser_line(dialog_->browser,
			    controller().getVCLogFile(ss).str().c_str());
}
