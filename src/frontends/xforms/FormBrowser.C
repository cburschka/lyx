/**
 * \file FormBrowser.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


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
	bcview().setCancel(dialog_->button_close);
}
