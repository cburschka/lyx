/**
 * \file FormExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormExternal.h"
#include "ControlExternal.h"
#include "forms/form_external.h"

#include "input_validators.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "insets/ExternalTemplate.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include "lyx_forms.h"

using namespace lyx::support;


typedef FormController<ControlExternal, FormView<FD_external> > base_class;

FormExternal::FormExternal(Dialog & parent)
	: base_class(parent, _("External Material"))
{}


void FormExternal::apply()
{
	InsetExternal::Params params = controller().params();

	string const buffer_path = kernel().bufferFilepath();
	params.filename.set(getString(dialog_->input_filename), buffer_path);

	int const choice = fl_get_choice(dialog_->choice_template) - 1;
	params.templatename = controller().getTemplate(choice).lyxName;

	params.lyxscale = strToInt(getString(dialog_->input_lyxscale));
	if (params.lyxscale == 0)
		params.lyxscale = 100;

	switch (fl_get_choice(dialog_->choice_display)) {
	case 5:
		params.display = lyx::graphics::NoDisplay;
		break;
	case 4:
		params.display = lyx::graphics::ColorDisplay;
		break;
	case 3:
		params.display = lyx::graphics::GrayscaleDisplay;
		break;
	case 2:
		params.display = lyx::graphics::MonochromeDisplay;
		break;
	case 1:
		params.display = lyx::graphics::DefaultDisplay;
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

	string const buffer_path = kernel().bufferFilepath();
	string const name = params.filename.outputFilename(buffer_path);
	fl_set_input(dialog_->input_filename, name.c_str());

	int ID = controller().getTemplateNumber(params.templatename);
	if (ID < 0) ID = 0;
	fl_set_choice(dialog_->choice_template, ID+1);

	updateComboChange();

	fl_set_input(dialog_->input_lyxscale, tostr(params.lyxscale).c_str());

	switch (params.display) {
	case lyx::graphics::NoDisplay:
		fl_set_choice(dialog_->choice_display, 5);
		break;
	case lyx::graphics::ColorDisplay:
		fl_set_choice(dialog_->choice_display, 4);
		break;
	case lyx::graphics::GrayscaleDisplay:
		fl_set_choice(dialog_->choice_display, 3);
		break;
	case lyx::graphics::MonochromeDisplay:
		fl_set_choice(dialog_->choice_display, 2);
		break;
	case lyx::graphics::DefaultDisplay:
		fl_set_choice(dialog_->choice_display, 1);
	}
}


ButtonPolicy::SMInput FormExternal::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput result = ButtonPolicy::SMI_VALID;
	if (ob == dialog_->choice_template) {

		// set to the chosen template
		updateComboChange();

	} else if (ob == dialog_->button_browse) {

		string const in_name  = fl_get_input(dialog_->input_filename);
		string const out_name = controller().Browse(in_name);
		fl_set_input(dialog_->input_filename, out_name.c_str());

	} else if (ob == dialog_->button_edit) {
		controller().editExternal();
		result = ButtonPolicy::SMI_NOOP;
	}

	return result;
}


void FormExternal::updateComboChange()
{
	int const choice = fl_get_choice(dialog_->choice_template) - 1;
	ExternalTemplate templ = controller().getTemplate(choice);

	// Update the help text
	fl_clear_browser(dialog_->browser_helptext);
	fl_addto_browser(dialog_->browser_helptext, templ.helpText.c_str());
	fl_set_browser_topline(dialog_->browser_helptext, 0);
}
