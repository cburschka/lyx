/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 * 
 * \file FormUrl.C
 * \author Angus Leeming, a.leeming@ic.ac.uk 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlUrl.h"
#include "FormUrl.h"
#include "form_url.h"

typedef FormCB<ControlUrl, FormDB<FD_form_url> > base_class;

FormUrl::FormUrl(ControlUrl & c)
	: base_class(c, _("Url"))
{}


void FormUrl::build()
{
	dialog_.reset(build_url());

	fl_set_input_return(dialog_->name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->url,  FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setUndoAll(dialog_->button_restore);
	bc().refresh();

	bc().addReadOnly(dialog_->name);
	bc().addReadOnly(dialog_->url);
	bc().addReadOnly(dialog_->radio_html);
}


void FormUrl::update()
{
	fl_set_input(dialog_->url,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->name,
		     controller().params().getOptions().c_str());

	if (controller().params().getCmdName() == "url")
		fl_set_button(dialog_->radio_html, 0);
	else
		fl_set_button(dialog_->radio_html, 1);
}


void FormUrl::apply()
{
	controller().params().setContents(fl_get_input(dialog_->url));
	controller().params().setOptions(fl_get_input(dialog_->name));

	if (fl_get_button(dialog_->radio_html))
		controller().params().setCmdName("htmlurl");
	else
		controller().params().setCmdName("url");
}
