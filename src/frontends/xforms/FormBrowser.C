/* FormBrowser.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "FormBrowser.h"
#include "form_browser.h"
#include "xformsBC.h"

FormBrowser::FormBrowser(ControlButtons & c, string const & t, bool allowResize)
	: FormDB<FD_form_browser>(c, t, allowResize)
{}
	

void FormBrowser::build()
{
	dialog_.reset(build_browser());

	// Manage the close button
	bc().setCancel(dialog_->button_close);
}
