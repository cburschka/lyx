/**
 * \file FormBranch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
 */


#include <config.h>

#include "xformsBC.h"
#include "ControlBranch.h"
#include "FormBranch.h"
#include "forms/form_branch.h"
#include "xforms_helpers.h" // formatted
#include "lyx_forms.h"
#include "insets/insetbranch.h"
#include "debug.h"

typedef FormController<ControlBranch, FormView<FD_branch> > base_class;

FormBranch::FormBranch(Dialog & parent)
	: base_class(parent, _("Branch"))
{}


void FormBranch::build()
{
	string all_branches(controller().params().branchlist.allBranches());

	dialog_.reset(build_branch(this));

	fl_addto_choice(dialog_->choice_branch, all_branches.c_str());
	
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_cancel);
}


void FormBranch::update()
{
	// Make changes in all_branches propagate within session:
	string all_branches(controller().params().branchlist.allBranches());
	fl_clear_choice(dialog_->choice_branch);
	fl_addto_choice(dialog_->choice_branch, all_branches.c_str());

	string branch(controller().params().branch);
	if (all_branches.find(branch) != string::npos && branch != "none")
		fl_set_choice_text(dialog_->choice_branch, branch.c_str());
}


void FormBranch::apply()
{
	string const type = fl_get_choice_text(dialog_->choice_branch);

	controller().params().branch = type;
}

