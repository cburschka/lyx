/**
 * \file FormExternal.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlExternal.h"
#include "FormExternal.h"
#include "forms/form_external.h"
#include "gettext.h"
#include "xforms_helpers.h"
#include "helper_funcs.h"

#include "support/lstrings.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlExternal, FormDB<FD_external> > base_class;

FormExternal::FormExternal()
	: base_class(_("Edit external file"))
{}


void FormExternal::apply()
{
	controller().params().filename =
		fl_get_input(dialog_->input_filename);
	controller().params().parameters =
		fl_get_input(dialog_->input_parameters);

	int const choice = fl_get_choice(dialog_->choice_template);
	controller().params().templ = controller().getTemplate(choice);
}


void FormExternal::build()
{
	dialog_.reset(build_external(this));

	string const choice =
		" " + getStringFromVector(controller().getTemplates(), " | ") + " ";
	fl_addto_choice(dialog_->choice_template, choice.c_str());

	fl_set_input_return (dialog_->input_filename,  FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_parameters, FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_filename);
	setPrehandler(dialog_->input_parameters);

	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->input_filename);
	bc().addReadOnly(dialog_->button_filenamebrowse);
	bc().addReadOnly(dialog_->input_parameters);
}


void FormExternal::update()
{
	InsetExternal::Params const & params = controller().params();

	fl_set_input(dialog_->input_filename, params.filename.c_str());
	fl_set_input(dialog_->input_parameters, params.parameters.c_str());

	int const ID = controller().getTemplateNumber(params.templ.lyxName);
	if (ID >= 0) {
		setEnabled(dialog_->choice_template, true);
		fl_set_choice(dialog_->choice_template, ID+1);
	} else
		setEnabled(dialog_->choice_template, false);

	updateComboChange();
}


ButtonPolicy::SMInput FormExternal::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->choice_template) {

		// set to the chosen template
		int const choice = fl_get_choice(dialog_->choice_template);
		controller().params().templ = controller().getTemplate(choice);

		updateComboChange();

	} else if (ob == dialog_->button_filenamebrowse) {

		string const in_name  = fl_get_input(dialog_->input_filename);
		string const out_name = controller().Browse(in_name);
		fl_set_input(dialog_->input_filename, out_name.c_str());

	} else if (ob == dialog_->button_edit) {
		controller().editExternal();

	} else if (ob == dialog_->button_view) {
		controller().viewExternal();

	} else if (ob == dialog_->button_update) {
		controller().updateExternal();
	}

	return ButtonPolicy::SMI_VALID;
}


void FormExternal::updateComboChange()
{
	// Update the help text
	fl_clear_browser(dialog_->browser_helptext);
	fl_addto_browser(dialog_->browser_helptext,
			 controller().params().templ.helpText.c_str());
	fl_set_browser_topline(dialog_->browser_helptext, 0);

	bool const enabled = (!controller().params().templ.automaticProduction);
	setEnabled(dialog_->button_update, enabled);
}
