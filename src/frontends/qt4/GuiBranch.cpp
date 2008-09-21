/**
 * \file GuiBranch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBranch.h"

#include "qt_helpers.h"
#include "BranchList.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BranchList.h"
#include "FuncRequest.h"

#include "insets/InsetBranch.h"

#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiBranch::GuiBranch(GuiView & lv)
	: GuiDialog(lv, "branch", qt_("Branch Settings"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(branchCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
}


void GuiBranch::change_adaptor()
{
	changed();
}


void GuiBranch::updateContents()
{
	typedef BranchList::const_iterator const_iterator;

	BranchList const & branchlist = buffer().params().branchlist();
	docstring const cur_branch = params_.branch;

	branchCO->clear();

	const_iterator const begin = branchlist.begin();
	const_iterator const end = branchlist.end();
	int id = 0;
	int count = 0;
	for (const_iterator it = begin; it != end; ++it, ++count) {
		docstring const & branch = it->branch();
		branchCO->addItem(toqstr(branch));

		if (cur_branch == branch)
			id = count;
	}
	branchCO->setCurrentIndex(id);
}


void GuiBranch::applyView()
{
	params_.branch = qstring_to_ucs4(branchCO->currentText());
}


bool GuiBranch::initialiseParams(string const & data)
{
	InsetBranch::string2params(data, params_);
	return true;
}


void GuiBranch::clearParams()
{
	params_ = InsetBranchParams();
}


void GuiBranch::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetBranch::params2string(params_)));
}


Dialog * createGuiBranch(GuiView & lv) { return new GuiBranch(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiBranch_moc.cpp"
