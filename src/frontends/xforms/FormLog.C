/**
 * \file FormLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormLog.h"
#include "ControlLog.h"
#include "forms/form_browser.h"

#include "support/std_sstream.h"

#include "lyx_forms.h"


FormLog::FormLog(Dialog & parent)
	: FormController<ControlLog, FormBrowser>(parent, "")
{}


void FormLog::update()
{
	setTitle(controller().title());

	std::ostringstream ss;
	controller().getContents(ss);

	fl_clear_browser(dialog_->browser);
	fl_add_browser_line(dialog_->browser, ss.str().c_str());
}
