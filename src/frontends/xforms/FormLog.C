/**
 * \file FormLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlLog.h"
#include "FormLog.h"
#include "forms/form_browser.h"
#include "gettext.h"
#include FORMS_H_LOCATION

FormLog::FormLog()
	: FormCB<ControlLog, FormBrowser>( _("LaTeX Log"))
{}


void FormLog::update()
{
	fl_clear_browser(dialog_->browser);

	if (controller().logfile().first == Buffer::buildlog) {
		fl_set_form_title(dialog_->form, _("Build log"));
		if (!fl_load_browser(dialog_->browser,
				     controller().logfile().second.c_str()))
			fl_add_browser_line(dialog_->browser,
					    _("No build log file found"));
		return;
	}

	fl_set_form_title(dialog_->form, _("LaTeX Log"));
	if (!fl_load_browser(dialog_->browser,
			     controller().logfile().second.c_str()))
		fl_add_browser_line(dialog_->browser,
				    _("No LaTeX log file found"));
}
