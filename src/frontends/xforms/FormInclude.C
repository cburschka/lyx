/**
 * \file FormInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormInclude.h"
#include "ControlInclude.h"
#include "forms/form_include.h"

#include "Tooltips.h"
#include "xforms_helpers.h" // setEnabled
#include "xformsBC.h"

#include "support/lstrings.h" // strip

#include "lyx_forms.h"

using lyx::support::rtrim;

using std::string;


typedef FormController<ControlInclude, FormView<FD_include> > base_class;

FormInclude::FormInclude(Dialog & parent)
	: base_class(parent, _("Child Document"))
{}


void FormInclude::build()
{
	dialog_.reset(build_include(this));

	// Manage the ok and cancel buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setCancel(dialog_->button_close);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_filename);

	fl_set_input_return(dialog_->input_filename, FL_RETURN_CHANGED);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->button_browse);
	bcview().addReadOnly(dialog_->radio_useinput);
	bcview().addReadOnly(dialog_->radio_useinclude);
	bcview().addReadOnly(dialog_->radio_verbatim);

	type_.init(dialog_->radio_useinput,   ControlInclude::INPUT);
	type_.init(dialog_->radio_useinclude, ControlInclude::INCLUDE);
	type_.init(dialog_->radio_verbatim,   ControlInclude::VERBATIM);

	// set up the tooltips
	string str = _("File name to include.");
	tooltips().init(dialog_->input_filename, str);
	str = _("Browse directories for file name.");
	tooltips().init(dialog_->button_browse, str);
	str = _("Use LaTeX \\input.");
	tooltips().init(dialog_->radio_useinput, str);
	str = _("Use LaTeX \\include.");
	tooltips().init(dialog_->radio_useinclude, str);
	str = _("Use LaTeX \\verbatiminput.");
	tooltips().init(dialog_->radio_verbatim, str);
	str = _("Underline spaces in generated output.");
	tooltips().init(dialog_->check_visiblespace, str);
	str = _("Show LaTeX preview.");
	tooltips().init(dialog_->check_preview, str);
	str = _("Load the file.");
	tooltips().init(dialog_->button_load, str);
}


void FormInclude::update()
{
	string const filename = controller().params().getContents();
	string const cmdname = controller().params().getCmdName();
	bool const preview = static_cast<bool>((controller().params().preview()));

	fl_set_input(dialog_->input_filename, filename.c_str());

	bool const inputCommand = cmdname == "input";
	bool const includeCommand = cmdname == "include";
	bool const verbatimStarCommand = cmdname == "verbatiminput*";
	bool const verbatimCommand = cmdname == "verbatiminput";

	setEnabled(dialog_->check_preview, inputCommand);
	fl_set_button(dialog_->check_preview, inputCommand ? preview : 0);

	if (cmdname.empty())
		type_.set(ControlInclude::INPUT);

	if (includeCommand)
		type_.set(ControlInclude::INCLUDE);

	if (verbatimCommand || verbatimStarCommand) {
		type_.set(ControlInclude::VERBATIM);
		fl_set_button(dialog_->check_visiblespace, verbatimStarCommand);
		setEnabled(dialog_->check_visiblespace, true);
		setEnabled(dialog_->button_load, false);
	} else {
		fl_set_button(dialog_->check_visiblespace, 0);
		setEnabled(dialog_->check_visiblespace, false);
		setEnabled(dialog_->button_load, true);
	}
}


void FormInclude::apply()
{
	InsetCommandParams params = controller().params();

	params.preview(fl_get_button(dialog_->check_preview));
	params.setContents(getString(dialog_->input_filename));

	ControlInclude::Type const type = ControlInclude::Type(type_.get());
	if (type == ControlInclude::INPUT)
		params.setCmdName("input");
	else if (type == ControlInclude::INCLUDE)
		params.setCmdName("include");
	else if (type == ControlInclude::VERBATIM) {
		if (fl_get_button(dialog_->check_visiblespace))
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");
	}

	controller().setParams(params);
}


ButtonPolicy::SMInput FormInclude::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput action = ButtonPolicy::SMI_VALID;

	if (ob == dialog_->button_browse) {
		string const in_name = getString(dialog_->input_filename);
		fl_freeze_form(form());
		ControlInclude::Type const type = ControlInclude::Type(type_.get());
		string const out_name = controller().Browse(in_name, type);
		fl_set_input(dialog_->input_filename, out_name.c_str());
		fl_unfreeze_form(form());

	} else if (ob == dialog_->button_load) {
		string const in_name = getString(dialog_->input_filename);
		if (!rtrim(in_name).empty() && controller().fileExists(in_name)) {
			dialog().OKButton();
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
		string const in_name = getString(dialog_->input_filename);
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
