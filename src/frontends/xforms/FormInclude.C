/**
 * \file FormInclude.C
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlInclude.h"
#include "FormInclude.h"
#include "forms/form_include.h"
#include "insets/insetinclude.h"
#include "xforms_helpers.h" // setEnabled
#include "support/lstrings.h" // strip
#include FORMS_H_LOCATION

typedef FormCB<ControlInclude, FormDB<FD_include> > base_class;

FormInclude::FormInclude()
	: base_class(_("Include file"))
{}


void FormInclude::build()
{
	dialog_.reset(build_include(this));

	fl_set_input_return(dialog_->input_filename, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_filename);

	// Manage the ok and cancel buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->button_browse);
	bc().addReadOnly(dialog_->radio_verbatim);
	bc().addReadOnly(dialog_->check_typeset);
	bc().addReadOnly(dialog_->radio_useinput);
	bc().addReadOnly(dialog_->radio_useinclude);
}


void FormInclude::update()
{
	#if 0
	// I believe this is not needed.
	// Anyway, it is plain wrong (JSpitzm 3/7/02)
	if (controller().params().noload) {
		fl_set_input(dialog_->input_filename, "");
		fl_set_button(dialog_->check_typeset, 0);
		fl_set_button(dialog_->radio_useinput, 0);
		fl_set_button(dialog_->radio_useinclude, 1);
		fl_set_button(dialog_->radio_verbatim, 0);
		fl_set_button(dialog_->check_visiblespace, 0);
		fl_deactivate_object(dialog_->check_visiblespace);
		fl_set_object_lcol(dialog_->check_visiblespace, FL_INACTIVE);
		return;
	}
	#endif

	fl_set_input(dialog_->input_filename,
		     controller().params().cparams.getContents().c_str());

	string const cmdname = controller().params().cparams.getCmdName();

	fl_set_button(dialog_->check_typeset,
		      int(controller().params().noload));

	if (cmdname == "input")
		fl_set_button(dialog_->check_preview,
			      int(controller().params().cparams.preview()));
	else
		fl_set_button(dialog_->check_preview, 0);

	setEnabled(dialog_->check_preview, (cmdname == "input"));

	fl_set_button(dialog_->radio_useinput, cmdname == "input");
	fl_set_button(dialog_->radio_useinclude, cmdname == "include");
	if (cmdname == "verbatiminput" || cmdname == "verbatiminput*") {
		fl_set_button(dialog_->radio_verbatim, 1);
		fl_set_button(dialog_->check_visiblespace, cmdname == "verbatiminput*");
		setEnabled(dialog_->check_visiblespace, true);
		setEnabled(dialog_->button_load, false);
	} else {
		fl_set_button(dialog_->check_visiblespace, 0);
		setEnabled(dialog_->check_visiblespace, false);
		setEnabled(dialog_->button_load, true);
	}

	if (cmdname.empty())
		fl_set_button(dialog_->radio_useinclude, 1);
}


void FormInclude::apply()
{
	controller().params().noload = fl_get_button(dialog_->check_typeset);
	controller().params().cparams
		.preview(fl_get_button(dialog_->check_preview));

	string const file = fl_get_input(dialog_->input_filename);
	if (controller().fileExists(file))
		controller().params().cparams.setContents(file);
	else
		controller().params().cparams.setContents("");

	if (fl_get_button(dialog_->radio_useinput))
		controller().params().flag = InsetInclude::INPUT;
	else if (fl_get_button(dialog_->radio_useinclude))
		controller().params().flag = InsetInclude::INCLUDE;
	else if (fl_get_button(dialog_->radio_verbatim)) {
		if (fl_get_button(dialog_->check_visiblespace))
			controller().params().flag = InsetInclude::VERBAST;
		else
			controller().params().flag = InsetInclude::VERB;
	}
}


ButtonPolicy::SMInput FormInclude::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput action = ButtonPolicy::SMI_VALID;

	if (ob == dialog_->button_browse) {
		ControlInclude::Type type;
		if (fl_get_button(dialog_->radio_useinput))
			type = ControlInclude::INPUT;
		else if (fl_get_button(dialog_->radio_verbatim))
			type = ControlInclude::VERBATIM;
		else
			type = ControlInclude::INCLUDE;

		string const in_name  = fl_get_input(dialog_->input_filename);
		fl_freeze_form(form());
		string const out_name = controller().Browse(in_name, type);
		fl_set_input(dialog_->input_filename, out_name.c_str());
		fl_unfreeze_form(form());

	} else if (ob == dialog_->button_load) {
		string const in_name = fl_get_input(dialog_->input_filename);
		if (!rtrim(in_name).empty() && controller().fileExists(in_name)) {
//			ApplyButton();
			controller().OKButton();
			controller().load(rtrim(in_name));
			action = ButtonPolicy::SMI_NOOP;
		}

	} else if (ob == dialog_->radio_verbatim) {
		setEnabled(dialog_->check_visiblespace, true);
		setEnabled(dialog_->button_load, false);

	} else if (ob == dialog_->radio_useinclude ||
		   ob == dialog_->radio_useinput) {
		fl_set_button(dialog_->check_visiblespace, 0);
		setEnabled(dialog_->check_visiblespace, false);
		setEnabled(dialog_->button_load, true);

	} else if (ob == dialog_->input_filename) {
		string const in_name = fl_get_input(dialog_->input_filename);
		if (rtrim(in_name).empty())
			action = ButtonPolicy::SMI_INVALID;
	}

	if (ob == dialog_->radio_useinput) {
		setEnabled(dialog_->check_preview, true);
	} else if (ob == dialog_->radio_verbatim ||
		   ob == dialog_->radio_useinclude) {
		fl_set_button(dialog_->check_preview, 0);
		setEnabled(dialog_->check_preview, false);
	}

	return action;
}
