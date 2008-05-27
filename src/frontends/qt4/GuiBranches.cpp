/**
 * \file GuiBranches.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBranches.h"

#include "ColorCache.h"
#include "GuiApplication.h"
#include "Validator.h"
#include "qt_helpers.h"

#include "BufferParams.h"

#include "support/lstrings.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QIcon>
#include <QColor>
#include <QColorDialog>


namespace lyx {
namespace frontend {


GuiBranches::GuiBranches(QWidget * parent, Qt::WFlags f)
	: QWidget(parent, f)
{
	setupUi(this);
	branchesTW->setColumnCount(3);
	branchesTW->headerItem()->setText(0, qt_("Branch"));
	branchesTW->headerItem()->setText(1, qt_("Activated"));
	branchesTW->headerItem()->setText(2, qt_("Color"));
	branchesTW->setSortingEnabled(true);
}

void GuiBranches::update(BufferParams const & params)
{
	branchlist_ = params.branchlist();
	updateView();
}


void GuiBranches::updateView()
{
	// store the selected branch
	QTreeWidgetItem * item = branchesTW->currentItem();
	QString sel_branch;
	if (item != 0)
		sel_branch = item->text(0);

	branchesTW->clear();

	BranchList::const_iterator it = branchlist_.begin();
	BranchList::const_iterator const end = branchlist_.end();
	for (; it != end; ++it) {
		QTreeWidgetItem * newItem = new QTreeWidgetItem(branchesTW);

		QString const bname = toqstr(it->getBranch());
		newItem->setText(0, bname);
		newItem->setText(1, it->getSelected() ? qt_("Yes") : qt_("No"));

		QColor const itemcolor = rgb2qcolor(it->getColor());
		if (itemcolor.isValid()) {
			QPixmap coloritem(30, 10);
			coloritem.fill(itemcolor);
			newItem->setIcon(2, QIcon(coloritem));
		}
		// restore selected branch
		if (bname == sel_branch) {
			branchesTW->setCurrentItem(newItem);
			branchesTW->setItemSelected(newItem, true);
		}
	}
	// emit signal
	changed();
}


void GuiBranches::apply(BufferParams & params) const
{
	params.branchlist() = branchlist_;
}


void GuiBranches::on_addBranchPB_pressed()
{
	QString const new_branch = newBranchLE->text();
	if (!new_branch.isEmpty()) {
		branchlist_.add(qstring_to_ucs4(new_branch));
		newBranchLE->clear();
		updateView();
	}
}


void GuiBranches::on_removePB_pressed()
{
	QTreeWidgetItem * selItem = branchesTW->currentItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (!sel_branch.isEmpty()) {
		branchlist_.remove(qstring_to_ucs4(sel_branch));
		newBranchLE->clear();
		updateView();
	}
}


void GuiBranches::on_activatePB_pressed()
{
	toggleBranch(branchesTW->currentItem());
}


void GuiBranches::on_branchesTW_itemDoubleClicked(QTreeWidgetItem * item, int col)
{
	if (col < 2)
		toggleBranch(item);
	else
		toggleColor(item);
}


void GuiBranches::toggleBranch(QTreeWidgetItem * item)
{
	if (item == 0)
		return;

	QString sel_branch = item->text(0);
	if (sel_branch.isEmpty())
		return;

	bool const selected = (item->text(1) == qt_("Yes"));
	Branch * branch = branchlist_.find(qstring_to_ucs4(sel_branch));
	if (branch && branch->setSelected(!selected)) {
		newBranchLE->clear();
		updateView();
	}
}


void GuiBranches::on_colorPB_clicked()
{
	toggleColor(branchesTW->currentItem());
}


void GuiBranches::toggleColor(QTreeWidgetItem * item)
{
	if (item == 0)
		return;

	QString sel_branch = item->text(0);
	if (sel_branch.isEmpty())
		return;

	docstring current_branch = qstring_to_ucs4(sel_branch);
	Branch * branch = branchlist_.find(current_branch);
	if (!branch)
		return;

	QColor const initial = rgb2qcolor(branch->getColor());
	QColor ncol = QColorDialog::getColor(initial, qApp->focusWidget());
	if (!ncol.isValid())
		return;

	// add the color to the branchlist
	branch->setColor(fromqstr(ncol.name()));
	newBranchLE->clear();
	updateView();
}

} // namespace frontend
} // namespace lyx

#include "GuiBranches_moc.cpp"
