/* FormVCLog.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlVCLog.h"
#include "FormVCLog.h"
#include "form_browser.h"
#include "gettext.h"

FormVCLog::FormVCLog(ControlVCLog & c)
	: FormCB<ControlVCLog, FormBrowser>(c, _("Version Control Log"))
{}


void FormVCLog::update()
{
	fl_clear_browser(dialog_->browser);

	if (controller().logfile().empty() ||
	    !fl_load_browser(dialog_->browser, controller().logfile().c_str()))
		fl_add_browser_line(dialog_->browser,
				    _("No version control log file available"));
}
