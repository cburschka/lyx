/**
 * \file FormShowFile.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlShowFile.h"
#include "FormShowFile.h"
#include "forms/form_browser.h"
#include "gettext.h"
#include FORMS_H_LOCATION


FormShowFile::FormShowFile()
	: FormCB<ControlShowFile, FormBrowser>(string())
{}


void FormShowFile::update()
{
	fl_hide_object(dialog_->button_update);
	fl_clear_browser(dialog_->browser);
	// courier medium
	fl_set_browser_fontstyle(dialog_->browser,FL_FIXED_STYLE);
	fl_set_form_title(dialog_->form, controller().getFileName().c_str());

	string const contents = controller().getFileContents();
	if (contents.empty())
		fl_add_browser_line(dialog_->browser,
				    "Error -> Cannot load file!");
	else
		fl_add_browser_line(dialog_->browser, contents.c_str());
}
