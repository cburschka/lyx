/**
 * \file GuiBranch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBranch.h"

#include "ControlBranch.h"

#include "qt_helpers.h"
#include "BranchList.h"

#include "insets/InsetBranch.h"

#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

GuiBranchDialog::GuiBranchDialog(LyXView & lv)
	: GuiDialog(lv, "branch")
{
	setupUi(this);
	setController(new ControlBranch(*this));
	setViewTitle(_("Branch Settings"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(branchCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
}


ControlBranch & GuiBranchDialog::controller() const
{
	return static_cast<ControlBranch &>(GuiDialog::controller());
}


void GuiBranchDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiBranchDialog::change_adaptor()
{
	changed();
}


void GuiBranchDialog::update_contents()
{
	typedef BranchList::const_iterator const_iterator;

	BranchList const & branchlist = controller().branchlist();
	docstring const cur_branch = controller().params().branch;

	branchCO->clear();

	const_iterator const begin = branchlist.begin();
	const_iterator const end = branchlist.end();
	int id = 0;
	int count = 0;
	for (const_iterator it = begin; it != end; ++it, ++count) {
		docstring const & branch = it->getBranch();
		branchCO->addItem(toqstr(branch));

		if (cur_branch == branch)
			id = count;
	}
	branchCO->setCurrentIndex(id);
}


void GuiBranchDialog::applyView()
{
	docstring const type = qstring_to_ucs4(branchCO->currentText());
	controller().params().branch = type;
}

} // namespace frontend
} // namespace lyx

#include "GuiBranch_moc.cpp"
