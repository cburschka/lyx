// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 * 
 * \file FormRef.C
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
		fl_addto_choice(dialog_->type,
				_(InsetRef::types[i].gui_name.c_str()));

	// Force the user to use the browser to change refs.
	fl_deactivate_object(dialog_->ref);

        // Manage the ok and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);
}


void FormRef::update()
{
	fl_set_input(dialog_->ref,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->name,
		     controller().params().getOptions().c_str());
	fl_set_choice(dialog_->type, 
		      InsetRef::getType(controller().params().getCmdName()) + 1);

	at_ref_ = false;
	fl_set_object_label(dialog_->button_go, _("Goto reference"));

	// Name is irrelevant to LaTeX/Literate documents
	if (controller().docType() == ControlRef::LATEX ||
	    controller().docType() == ControlRef::LITERATE) {
		setEnabled(dialog_->name, false);
	} else {
		setEnabled(dialog_->name, true);
	}

	// type is irrelevant to LinuxDoc/DocBook.
	if (controller().docType() == ControlRef::LINUXDOC ||
	    controller().docType() == ControlRef::DOCBOOK) {
		fl_set_choice(dialog_->type, 1);
		setEnabled(dialog_->type, false);
	} else {
		setEnabled(dialog_->type, true);
	}

	refs_ = controller().getLabelList();
	updateBrowser(refs_);
}


void FormRef::updateBrowser(vector<string> const & akeys) const
{
	vector<string> keys(akeys);
	if (fl_get_button(dialog_->sort))
		sort(keys.begin(), keys.end());

	fl_clear_browser(dialog_->browser);
	for (vector<string>::const_iterator it = keys.begin();
	     it != keys.end(); ++it)
		fl_add_browser_line(dialog_->browser, it->c_str());

	if (keys.empty()) {
		fl_add_browser_line(dialog_->browser,
				    _("*** No labels found in document ***"));
	
		setEnabled(dialog_->browser, false);
		setEnabled(dialog_->sort,    false);

		fl_set_input(dialog_->ref, "");
	} else {
		setEnabled(dialog_->browser, true);
		setEnabled(dialog_->sort,    true);

		string ref = fl_get_input(dialog_->ref);
		vector<string>::const_iterator cit =
			find(keys.begin(), keys.end(), ref);
		if (cit == keys.end()) {
			cit = keys.begin();
			fl_set_input(dialog_->ref, cit->c_str());
		} else if (ref.empty())
			fl_set_input(dialog_->ref, cit->c_str());

		int const i = static_cast<int>(cit - keys.begin());
		fl_set_browser_topline(dialog_->browser, max(i-5, 1));
		fl_select_browser_line(dialog_->browser, i+1);
	}
}


void FormRef::apply()
{
	int const type = fl_get_choice(dialog_->type) - 1;
	controller().params().setCmdName(InsetRef::getName(type));

	controller().params().setOptions(fl_get_input(dialog_->name));
	controller().params().setContents(fl_get_input(dialog_->ref));
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
			controller().gotoRef(fl_get_input(dialog_->ref));
	  		fl_set_object_label(dialog_->button_go, _("Go back"));
		} else {
			controller().gotoBookmark();
			fl_set_object_label(dialog_->button_go,
					    _("Goto reference"));
		}

	} else if (ob == dialog_->browser) {

		unsigned int sel = fl_get_browser(dialog_->browser);
		if (sel < 1 || sel > refs_.size())
			return ButtonPolicy::SMI_NOOP;

		if (!controller().isReadonly()) {
			string s = fl_get_browser_line(dialog_->browser, sel);
			fl_set_input(dialog_->ref, s.c_str());
		}

		if (at_ref_)
			controller().gotoBookmark();
		at_ref_ = false;
		fl_set_object_label(dialog_->button_go, _("Goto reference"));

		setEnabled(dialog_->type,      true);
		setEnabled(dialog_->button_go, true);
		fl_set_object_lcol(dialog_->ref, FL_BLACK);

	} else if (ob == dialog_->button_update || 
		   ob == dialog_->sort) {

		if (ob == dialog_->button_update)
			refs_ = controller().getLabelList();

		fl_freeze_form(form());
		updateBrowser(refs_);
		fl_unfreeze_form(form());

	} else if (ob == dialog_->type) {

		int const type = fl_get_choice(dialog_->type) - 1;
		if (controller().params().getCmdName() ==
		    InsetRef::getName(type)) {
			activate = ButtonPolicy::SMI_NOOP;
		}
	}

	return activate;
}
