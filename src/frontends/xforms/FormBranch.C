/**
 * \file FormBranch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "FormBranch.h"
#include "forms/form_branch.h"
#include "xformsBC.h"

#include "BranchList.h"

#include "controllers/ControlBranch.h"

#include "insets/insetbranch.h"

#include "lyx_forms.h"

using std::string;

namespace lyx {
namespace frontend {


typedef FormController<ControlBranch, FormView<FD_branch> > base_class;

FormBranch::FormBranch(Dialog & parent)
	: base_class(parent, _("Branch"))
{}


void FormBranch::build()
{
	dialog_.reset(build_branch(this));

	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_cancel);
}


void FormBranch::update()
{
	typedef BranchList::const_iterator const_iterator;

	BranchList const & branchlist = controller().branchlist();
	string const cur_branch = controller().params().branch;

	fl_clear_choice(dialog_->choice_branch);

	const_iterator const begin = branchlist.begin();
	const_iterator const end = branchlist.end();
	int id = 1;
	int count = 1;
	for (const_iterator it = begin; it != end; ++it, ++count) {
		string const & branch = it->getBranch();
		fl_addto_choice(dialog_->choice_branch, branch.c_str());

		if (cur_branch == branch)
			id = count;
	}
	fl_set_choice(dialog_->choice_branch, id);
}


void FormBranch::apply()
{
	string const type = fl_get_choice_text(dialog_->choice_branch);
	controller().params().branch = type;
}

} // namespace frontend
} // namespace lyx
