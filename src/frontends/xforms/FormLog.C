/* FormLog.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h" 
#include "FormLog.h"
#include "form_browser.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "buffer.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

FormLog::FormLog(LyXView * lv, Dialogs * d)
	: FormBrowser(lv, d, _("LaTeX Log"))
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showLogFile.connect(slot(this, &FormLog::show));
}

FormLog::~FormLog()
{
}

void FormLog::update()
{
	if (!dialog_ || !lv_->view()->available())
		return;
 
	string const logfile = lv_->view()->buffer()->getLatexLogName();

	fl_clear_browser(dialog_->browser);

	if (!fl_load_browser(dialog_->browser, logfile.c_str()))
		fl_add_browser_line(dialog_->browser, _("No LaTeX log file found"));
}
