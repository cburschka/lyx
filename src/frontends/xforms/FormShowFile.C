/**
 * \file FormShowFile.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormShowFile.h"
#include "ControlShowFile.h"
#include "forms/form_browser.h"

#include "xformsBC.h"

#include "lyx_forms.h"


FormShowFile::FormShowFile(Dialog & parent)
	: FormController<ControlShowFile, FormBrowser>(parent, _("Show File"))
{}


void FormShowFile::update()
{
	fl_hide_object(dialog_->button_update);
	fl_clear_browser(dialog_->browser);

	// courier medium
	fl_set_browser_fontstyle(dialog_->browser,FL_FIXED_STYLE);

	string const title = controller().getFileName();
	setTitle(title);

	string const contents = controller().getFileContents();
	if (contents.empty())
		fl_add_browser_line(dialog_->browser,
				    "Error -> Cannot load file!");
	else
		fl_add_browser_line(dialog_->browser, contents.c_str());
}
