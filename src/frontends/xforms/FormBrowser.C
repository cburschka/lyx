/**
 * \file FormBrowser.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormBrowser.h"
#include "forms/form_browser.h"
#include "xformsBC.h"

FormBrowser::FormBrowser(ControlButtons & c, string const & t, bool allowResize)
	: FormDB<FD_browser>(c, t, allowResize)
{}


void FormBrowser::build()
{
	dialog_.reset(build_browser(this));

	// Manage the close button
	bc().setCancel(dialog_->button_close);
}
