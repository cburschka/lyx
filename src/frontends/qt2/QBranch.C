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

#include "debug.h"
#include "BranchList.h"

#include "controllers/ControlBranch.h"

#include "insets/insetbranch.h"

#include "QBranch.h"
#include "QBranchDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include <qcombobox.h>
#include <qpushbutton.h>

using std::string;


typedef QController<ControlBranch, QView<QBranchDialog> > base_class;


QBranch::QBranch(Dialog & parent)
	: base_class(parent, _("LyX: Branch Settings"))
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
	string const cur_branch = controller().params().branch;

	dialog_->branchCO->clear();

	const_iterator const begin = branchlist.begin();
	const_iterator const end = branchlist.end();
	int id = 1;
	int count = 1;
	for (const_iterator it = begin; it != end; ++it, ++count) {
		string const & branch = it->getBranch();
		dialog_->branchCO->insertItem(toqstr(branch));

		if (cur_branch == branch)
			id = count;
	}
	dialog_->branchCO->setCurrentItem(id);
}


void QBranch::apply()
{
	string const type = fromqstr(dialog_->branchCO->currentText());
	controller().params().branch = type;
}
