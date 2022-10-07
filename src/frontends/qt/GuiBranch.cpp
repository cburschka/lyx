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

#include "Buffer.h"
#include "BufferParams.h"
#include "BranchList.h"

#include "insets/InsetBranch.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <QPushButton>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GuiBranch::GuiBranch(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);
	connect(branchCO, SIGNAL(activated(int)), this, SIGNAL(changed()));
	connect(invertedCB, SIGNAL(clicked()), this, SIGNAL(changed()));
}


void GuiBranch::paramsToDialog(Inset const * inset)
{
	InsetBranch const * ib = static_cast<InsetBranch const *>(inset);
	Buffer const & buf = ib->buffer();
	BranchList const & branchlist = buf.params().branchlist();
	docstring const cur_branch = ib->branch();

	branchCO->clear();
	int id = 0;
	int count = 0;
	for (Branch const & it : branchlist) {
		docstring const & branch = it.branch();
		branchCO->addItem(toqstr(branch), toqstr(branch));
		if (cur_branch == branch)
			id = count;
		++count;
	}
	// Add branches from master
	Buffer const * masterBuf = buf.masterBuffer();
	if (masterBuf != &buf) {
		BranchList const & masterBranchlist = masterBuf->params().branchlist();
		for (Branch const & it : masterBranchlist) {
			docstring const & branch = it.branch();
			if (!branchlist.find(branch)) {
				branchCO->addItem(
					toqstr(bformat(_("%1$s[[branch]] (%2$s)[[master]]"),
						   branch, _("master"))),
					toqstr(branch));
				if (cur_branch == branch)
					id = count;
				++count;
			}
		}
	}
	branchCO->setCurrentIndex(id);
	invertedCB->setChecked(ib->params().inverted);
}


docstring GuiBranch::dialogToParams() const
{
	docstring branch = qstring_to_ucs4(branchCO->itemData(branchCO->currentIndex()).toString());
	InsetBranchParams params(branch, invertedCB->isChecked());
	return from_utf8(InsetBranch::params2string(params));
}


bool GuiBranch::checkWidgets(bool readonly) const
{
	branchCO->setEnabled(!readonly);
	invertedCB->setEnabled(!readonly);
	return InsetParamsWidget::checkWidgets();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiBranch.cpp"
