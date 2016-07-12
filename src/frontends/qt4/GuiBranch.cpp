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

#include <QPushButton>

using namespace std;

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
	typedef BranchList::const_iterator const_iterator;
	BranchList const & branchlist = ib->buffer().params().branchlist();
	docstring const cur_branch = ib->branch();

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
	invertedCB->setChecked(ib->params().inverted);
}


docstring GuiBranch::dialogToParams() const
{
	InsetBranchParams params(qstring_to_ucs4(branchCO->currentText()), invertedCB->isChecked());
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
