/**
 * \file FormExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "xformsBC.h"
#include "ControlExternal.h"
#include "FormExternal.h"
#include "forms/form_external.h"

#include "helper_funcs.h"
#include "input_validators.h"
#include "Tooltips.h"
#include "xforms_helpers.h"

#include "gettext.h"

#include "support/tostr.h"
#include "support/lstrings.h"
#include "lyx_forms.h"

typedef FormController<ControlExternal, FormView<FD_external> > base_class;

FormExternal::FormExternal(Dialog & parent)
	: base_class(parent, _("External Material"))
{}


void FormExternal::apply()
{
	InsetExternal::Params params = controller().params();

	params.filename = fl_get_input(dialog_->input_filename);

	int const choice = fl_get_choice(dialog_->choice_template) - 1;
	params.templ = controller().getTemplate(choice);

	params.lyxscale = strToInt(getString(dialog_->input_lyxscale));
	if (params.lyxscale == 0)
		params.lyxscale = 100;

	switch (fl_get_choice(dialog_->choice_display)) {
	case 5:
		params.display = grfx::NoDisplay;
		break;
	case 4:
		params.display = grfx::ColorDisplay;
		break;
	case 3:
		params.display = grfx::GrayscaleDisplay;
		break;
	case 2:
		params.display = grfx::MonochromeDisplay;
		break;
	case 1:
		params.display = grfx::DefaultDisplay;
	}

	controller().setParams(params);
}


void FormExternal::build()
{
	dialog_.reset(build_external(this));

	string const choice =
		' ' + getStringFromVector(controller().getTemplates(), " | ") + ' ';
	fl_addto_choice(dialog_->choice_template, choice.c_str());

	fl_set_input_return (dialog_->input_filename,  FL_RETURN_CHANGED);

	// Disable for read-only documents.
	bcview().addReadOnly(dialog_->input_filename);
	bcview().addReadOnly(dialog_->button_browse);

	// Trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_filename);

	// Activate ok/apply immediately upon input.
	fl_set_input_return(dialog_->input_filename, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_lyxscale, FL_RETURN_CHANGED);

	fl_set_input_filter(dialog_->input_lyxscale, fl_unsigned_int_filter);

	string const display_List =
		_("Default|Monochrome|Grayscale|Color|Do not display");
	fl_addto_choice(dialog_->choice_display, display_List.c_str());

	// Set up the tooltips.
	string str = _("The file you want to insert.");
	tooltips().init(dialog_->input_filename, str);
	str = _("Browse the directories.");
	tooltips().init(dialog_->button_browse, str);

	str = _("Scale the image to inserted percentage value.");
	tooltips().init(dialog_->input_lyxscale, str);
	str = _("Select display mode for this image.");
	tooltips().init(dialog_->choice_display, str);

	// Manage the ok, apply and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
}


void FormExternal::update()
{
	InsetExternal::Params const & params = controller().params();

	fl_set_input(dialog_->input_filename, params.filename.c_str());

	int ID = controller().getTemplateNumber(params.templ.lyxName);
	if (ID < 0) ID = 0;
	fl_set_choice(dialog_->choice_template, ID+1);

	updateComboChange();

	fl_set_input(dialog_->input_lyxscale, tostr(params.lyxscale).c_str());

	switch (params.display) {
	case grfx::NoDisplay:
		fl_set_choice(dialog_->choice_display, 5);
		break;
	case grfx::ColorDisplay:
		fl_set_choice(dialog_->choice_display, 4);
		break;
	case grfx::GrayscaleDisplay:
		fl_set_choice(dialog_->choice_display, 3);
		break;
	case grfx::MonochromeDisplay:
		fl_set_choice(dialog_->choice_display, 2);
		break;
	case grfx::DefaultDisplay:
		fl_set_choice(dialog_->choice_display, 1);
	}
}


ButtonPolicy::SMInput FormExternal::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->choice_template) {

		// set to the chosen template
		int const choice = fl_get_choice(dialog_->choice_template) - 1;
		InsetExternal::Params params = controller().params();
		params.templ = controller().getTemplate(choice);
		controller().setParams(params);

		updateComboChange();

	} else if (ob == dialog_->button_browse) {

		string const in_name  = fl_get_input(dialog_->input_filename);
		string const out_name = controller().Browse(in_name);
		fl_set_input(dialog_->input_filename, out_name.c_str());

	} else if (ob == dialog_->button_edit) {
		controller().editExternal();
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
}
