/**
 * \file FormUrl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormUrl.h"
#include "ControlCommand.h"
#include "forms/form_url.h"

#include "xformsBC.h"

#include "lyx_forms.h"

namespace lyx {
namespace frontend {

typedef  FormController<ControlCommand, FormView<FD_url> > base_class;

FormUrl::FormUrl(Dialog & parent)
	: base_class(parent, _("URL"))
{}


void FormUrl::build()
{
	dialog_.reset(build_url(this));

	fl_set_input_return(dialog_->input_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_url,  FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_name);
	setPrehandler(dialog_->input_url);

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	bcview().addReadOnly(dialog_->input_name);
	bcview().addReadOnly(dialog_->input_url);
	bcview().addReadOnly(dialog_->check_html);
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

} // namespace frontend
} // namespace lyx
