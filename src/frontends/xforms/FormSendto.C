/*
 * \file FormSendto.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormSendto.h"
#include "forms/form_sendto.h"
#include "ControlSendto.h"
#include "xformsBC.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "converter.h"
#include "gettext.h"
#include FORMS_H_LOCATION

using std::vector;

typedef FormCB<ControlSendto, FormDB<FD_sendto> > base_class;

FormSendto::FormSendto(ControlSendto & c, Dialogs & d)
	: base_class(c, d, _("Send document to command"))
{}


void FormSendto::build()
{
	dialog_.reset(build_sendto(this));

	fl_set_input_return(dialog_->input_command, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_command);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	// Set up the tooltip mechanism
	string str = _("Export the buffer to this format before running the command below on it.");
	tooltips().init(dialog_->browser_formats, str);

	str = _("Run this command on the buffer exported to the chosen format. $$FName will be replaced by the name of this file.");
	tooltips().init(dialog_->input_command, str);
}


void FormSendto::update()
{
	all_formats_ = controller().allFormats();

	// Check whether the current contents of the browser will be
	// changed by loading the contents of formats
	vector<string> keys;
	keys.resize(all_formats_.size());

	vector<string>::iterator result = keys.begin();
	vector<Format const *>::const_iterator it  = all_formats_.begin();
	vector<Format const *>::const_iterator end = all_formats_.end();
	for (; it != end; ++it, ++result) {
		*result = (*it)->prettyname();
	}

	vector<string> const browser_keys =
		getVector(dialog_->browser_formats);

	if (browser_keys == keys)
		return;

	// Reload the browser
	fl_clear_browser(dialog_->browser_formats);

	for (vector<string>::const_iterator it = keys.begin();
	     it < keys.end(); ++it) {
		fl_add_browser_line(dialog_->browser_formats, it->c_str());
	}

	fl_set_input(dialog_->input_command, controller().getCommand().c_str());
}


ButtonPolicy::SMInput FormSendto::input(FL_OBJECT *, long)
{
	int const line = fl_get_browser(dialog_->browser_formats);
	if (line < 1 || line > fl_get_browser_maxline(dialog_->browser_formats))
		return ButtonPolicy::SMI_INVALID;

	string cmd = getString(dialog_->input_command);
	cmd = trim(cmd);
	if (cmd.empty())
		return ButtonPolicy::SMI_INVALID;

	return ButtonPolicy::SMI_VALID;
}


void FormSendto::apply()
{
	int const line = fl_get_browser(dialog_->browser_formats);
	if (line < 1 || line > fl_get_browser_maxline(dialog_->browser_formats))
		return;

	string const cmd = getString(dialog_->input_command);

	controller().setFormat(all_formats_[line-1]);
	controller().setCommand(cmd);
}
