/**
 * \file xforms/FormUrl.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */


#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlUrl.h"
#include "FormUrl.h"
#include "forms/form_url.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlUrl, FormDB<FD_url> > base_class;

FormUrl::FormUrl()
	: base_class(_("Url"))
{}


void FormUrl::build()
{
	dialog_.reset(build_url(this));

	fl_set_input_return(dialog_->input_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_url,  FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_name);
	setPrehandler(dialog_->input_url);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_name);
	bc().addReadOnly(dialog_->input_url);
	bc().addReadOnly(dialog_->check_html);
}


void FormUrl::update()
{
	fl_set_input(dialog_->input_url,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->input_name,
		     controller().params().getOptions().c_str());

	if (controller().params().getCmdName() == "url")
		fl_set_button(dialog_->check_html, 0);
	else
		fl_set_button(dialog_->check_html, 1);
}


void FormUrl::apply()
{
	controller().params().setContents(fl_get_input(dialog_->input_url));
	controller().params().setOptions(fl_get_input(dialog_->input_name));

	if (fl_get_button(dialog_->check_html))
		controller().params().setCmdName("htmlurl");
	else
		controller().params().setCmdName("url");
}
