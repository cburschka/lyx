/**
 * \file FormBrowser.C
 * See the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormBrowser.h"
#include "forms/form_browser.h"
#include "xformsBC.h"

FormBrowser::FormBrowser(string const & t, bool allowResize)
	: FormDB<FD_browser>(t, allowResize)
{}


void FormBrowser::build()
{
	dialog_.reset(build_browser(this));

	// Manage the close button
	bc().setCancel(dialog_->button_close);
}
