/**
 * \file xforms/FormRef.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlRef.h"
#include "FormRef.h"
#include "form_ref.h"
#include "xforms_helpers.h"
#include "insets/insetref.h"
#include "helper_funcs.h" // getStringFromVector
#include "support/lstrings.h" // frontStrip, strip

using std::find;
using std::max;
using std::sort;
using std::vector;

typedef FormCB<ControlRef, FormDB<FD_form_ref> > base_class;

FormRef::FormRef(ControlRef & c)
	: base_class(c, _("Reference")),
	  at_ref_(false)
{}


void FormRef::build()
{
	dialog_.reset(build_ref());

	for (int i = 0; !InsetRef::types[i].latex_name.empty(); ++i)
		fl_addto_choice(dialog_->choice_type,
				_(InsetRef::types[i].gui_name.c_str()));

	// Force the user to use the browser to change refs.
	fl_deactivate_object(dialog_->input_ref);

	fl_set_input_return(dialog_->input_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ref,  FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_name);
	setPrehandler(dialog_->input_ref);

	// Manage the ok and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->button_update);
	bc().addReadOnly(dialog_->input_name);
	bc().addReadOnly(dialog_->input_ref);
}


void FormRef::update()
{
	fl_set_input(dialog_->input_ref,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->input_name,
		     controller().params().getOptions().c_str());
	fl_set_choice(dialog_->choice_type,
		      InsetRef::getType(controller().params().getCmdName()) + 1);

	at_ref_ = false;
	fl_set_object_label(dialog_->button_go, _("Go to reference"));

	// Name is irrelevant to LaTeX/Literate documents
	if (controller().docType() == ControlRef::LATEX ||
	    controller().docType() == ControlRef::LITERATE) {
		setEnabled(dialog_->input_name, false);
	} else {
		setEnabled(dialog_->input_name, true);
	}

	// type is irrelevant to LinuxDoc/DocBook.
	if (controller().docType() == ControlRef::LINUXDOC ||
	    controller().docType() == ControlRef::DOCBOOK) {
		fl_set_choice(dialog_->choice_type, 1);
		setEnabled(dialog_->choice_type, false);
	} else {
		setEnabled(dialog_->choice_type, true);
	}

	// Get the available buffers
	vector<string> const buffers = controller().getBufferList();
	vector<string> const choice_buffers =
		getVectorFromChoice(dialog_->choice_buffer);

	// If different from the current contents of the choice, then update it
	if (buffers != choice_buffers) {
		// create a string of entries " entry1 | entry2 | entry3 "
		// with which to initialise the xforms choice object.
		string const choice =
			" " + getStringFromVector(buffers, " | ") + " ";

		fl_clear_choice(dialog_->choice_buffer);
		fl_addto_choice(dialog_->choice_buffer, choice.c_str());
	}

	fl_set_choice(dialog_->choice_buffer, controller().getBufferNum() + 1);

	string const name = controller().getBufferName(fl_get_choice(dialog_->choice_buffer) - 1);
	refs_ = controller().getLabelList(name);

	updateBrowser(refs_);
}


void FormRef::updateBrowser(vector<string> const & akeys) const
{
	vector<string> keys(akeys);
	if (fl_get_button(dialog_->check_sort))
		sort(keys.begin(), keys.end());

	vector<string> browser_keys =
		getVectorFromBrowser(dialog_->browser_refs);

	if (browser_keys == keys)
		return;

	fl_clear_browser(dialog_->browser_refs);
	for (vector<string>::const_iterator it = keys.begin();
	     it != keys.end(); ++it)
		fl_add_browser_line(dialog_->browser_refs, it->c_str());

	if (keys.empty()) {
		fl_add_browser_line(dialog_->browser_refs,
				    _("*** No labels found in document ***"));

		setEnabled(dialog_->browser_refs, false);
		setEnabled(dialog_->check_sort,   false);

		fl_set_input(dialog_->input_ref, "");
	} else {
		setEnabled(dialog_->browser_refs, true);
		setEnabled(dialog_->check_sort,   true);

		string ref = fl_get_input(dialog_->input_ref);
		vector<string>::const_iterator cit = (ref.empty())
			? keys.begin()
			: find(keys.begin(), keys.end(), ref);
		if (cit == keys.end()) {
			fl_deselect_browser(dialog_->browser_refs);
		} else {
			if (ref.empty())
				fl_set_input(dialog_->input_ref, cit->c_str());

			int const i = static_cast<int>(cit - keys.begin());
			fl_set_browser_topline(dialog_->browser_refs, max(i-5, 1));
			fl_select_browser_line(dialog_->browser_refs, i+1);
		}
	}
}


void FormRef::apply()
{
	int const type = fl_get_choice(dialog_->choice_type) - 1;
	controller().params().setCmdName(InsetRef::getName(type));

	controller().params().setOptions(fl_get_input(dialog_->input_name));
	controller().params().setContents(fl_get_input(dialog_->input_ref));
}


ButtonPolicy::SMInput FormRef::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput activate(ButtonPolicy::SMI_VALID);

	if (ob == dialog_->button_go) {
		// goto reference / go back

		// No change to data
		activate = ButtonPolicy::SMI_NOOP;

		at_ref_ = !at_ref_;
		if (at_ref_) {
			controller().gotoRef(fl_get_input(dialog_->input_ref));
			fl_set_object_label(dialog_->button_go, _("Go back"));
		} else {
			controller().gotoBookmark();
			fl_set_object_label(dialog_->button_go,
					    _("Go to reference"));
		}

	} else if (ob == dialog_->browser_refs) {

		unsigned int sel = fl_get_browser(dialog_->browser_refs);
		if (sel < 1 || sel > refs_.size())
			return ButtonPolicy::SMI_NOOP;

		if (!controller().isReadonly()) {
			string s = fl_get_browser_line(dialog_->browser_refs, sel);
			fl_set_input(dialog_->input_ref, s.c_str());
		}

		if (at_ref_)
			controller().gotoBookmark();
		at_ref_ = false;
		fl_set_object_label(dialog_->button_go, _("Go to reference"));

		setEnabled(dialog_->choice_type,      true);
		setEnabled(dialog_->button_go, true);
		fl_set_object_lcol(dialog_->input_ref, FL_BLACK);

	} else if (ob == dialog_->button_update ||
		   ob == dialog_->check_sort ||
		   ob == dialog_->choice_buffer) {

		if (ob == dialog_->button_update ||
		    ob == dialog_->choice_buffer) {
			string const name =
				controller().getBufferName(fl_get_choice(dialog_->choice_buffer) - 1);
			refs_ = controller().getLabelList(name);
		}

		fl_freeze_form(form());
		updateBrowser(refs_);
		fl_unfreeze_form(form());

	} else if (ob == dialog_->choice_type) {

		int const type = fl_get_choice(dialog_->choice_type) - 1;
		if (controller().params().getCmdName() ==
		    InsetRef::getName(type)) {
			activate = ButtonPolicy::SMI_NOOP;
		}
	}

	return activate;
}
