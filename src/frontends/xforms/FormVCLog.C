/* FormVCLog.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h" 
#include "FormVCLog.h"
#include "form_browser.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "buffer.h"

FormVCLog::FormVCLog(LyXView * lv, Dialogs * d)
	: FormBrowser(lv, d, _("Version Control Log"))
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showVCLogFile.connect(slot(this, &FormVCLog::show));
}


void FormVCLog::update()
{
	if (!dialog_.get() || !lv_->view()->available())
		return;
 
	const string logfile = lv_->view()->buffer()->lyxvc.getLogFile();

	fl_clear_browser(dialog_->browser);

	if (logfile=="" || !fl_load_browser(dialog_->browser, logfile.c_str()))
		fl_add_browser_line(dialog_->browser, _("No version control log file available"));

	lyx::unlink(logfile); 
}
