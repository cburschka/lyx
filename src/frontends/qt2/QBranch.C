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
#include "qt_helpers.h"
#include "ControlBranch.h"
#include "insets/insetbranch.h"
#include "support/lstrings.h"

#include <qcombobox.h>
#include <qpushbutton.h>

#include "QBranchDialog.h"
#include "QBranch.h"
#include "Qt2BC.h"

using lyx::support::getVectorFromString;

using std::string;
using std::vector;


typedef QController<ControlBranch, QView<QBranchDialog> > base_class;


QBranch::QBranch(Dialog & parent)
	: base_class(parent, _("LyX: Branch Settings"))
{}


void QBranch::build_dialog()
{
	string all_branches(controller().params().branchlist.allBranches());

	dialog_.reset(new QBranchDialog(this));

	std::vector<string> all = getVectorFromString(all_branches, "|");
	for (unsigned i = 0; i < all.size(); ++i) {
		QString const bname = toqstr(all[i].c_str());
		dialog_->branchCO->insertItem(bname);
		}

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QBranch::update_contents()
{
	// re-read branch list
	dialog_->branchCO->clear();
	string all_branches(controller().params().branchlist.allBranches());
	string cur_branch(controller().params().branch);
	unsigned int cur_item = 0;
	std::vector<string> all = getVectorFromString(all_branches, "|");
	for (unsigned i = 0; i < all.size(); ++i) {
		QString const bname = toqstr(all[i].c_str());
		dialog_->branchCO->insertItem(bname);
		if (bname == toqstr(cur_branch))
			cur_item = i;
	}
	// set to current item. A better idea anyone?
	if (all_branches.find(cur_branch) != string::npos && cur_branch != "none")
		dialog_->branchCO->setCurrentItem(cur_item);
}


void QBranch::apply()
{
	string const type = fromqstr(dialog_->branchCO->currentText());
	controller().params().branch = type;
}
