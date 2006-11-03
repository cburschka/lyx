/**
 * \file QBranch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QBranch.h"
#include "QBranchDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "BranchList.h"

#include "controllers/ControlBranch.h"

#include "insets/insetbranch.h"

#include <QPushButton>

namespace lyx {
namespace frontend {

typedef QController<ControlBranch, QView<QBranchDialog> > base_class;


QBranch::QBranch(Dialog & parent)
	: base_class(parent, _("Branch Settings"))
{}


void QBranch::build_dialog()
{
	dialog_.reset(new QBranchDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QBranch::update_contents()
{
	typedef BranchList::const_iterator const_iterator;

	BranchList const & branchlist = controller().branchlist();
	docstring const cur_branch = controller().params().branch;

	dialog_->branchCO->clear();

	const_iterator const begin = branchlist.begin();
	const_iterator const end = branchlist.end();
	int id = 0;
	int count = 0;
	for (const_iterator it = begin; it != end; ++it, ++count) {
		docstring const & branch = it->getBranch();
		dialog_->branchCO->addItem(toqstr(branch));

		if (cur_branch == branch)
			id = count;
	}
	dialog_->branchCO->setCurrentIndex(id);
}


void QBranch::apply()
{
	docstring const type = qstring_to_ucs4(dialog_->branchCO->currentText());
	controller().params().branch = type;
}

} // namespace frontend
} // namespace lyx
