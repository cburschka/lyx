/**
 * \file QBranches.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QBranches.h"

#include "lcolorcache.h"
#include "validators.h"
#include "qt_helpers.h"

#include "bufferparams.h"
#include "controllers/ControlDocument.h"

#include "support/lstrings.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QIcon>
#include <QColor>
#include <QColorDialog>

using std::string;

namespace lyx {
namespace frontend {


QBranches::QBranches(QWidget * parent, Qt::WFlags f)
	: QWidget(parent, f)
{
	setupUi(this);
	branchesTW->setColumnCount(2);
	branchesTW->headerItem()->setText(0, qt_("Branch"));
	branchesTW->headerItem()->setText(1, qt_("Activated"));
}

QBranches::~QBranches()
{
}

void QBranches::update(BufferParams const & params)
{
	branchlist_ = params.branchlist();
	update();
}

void QBranches::update()
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
		QTreeWidgetItem * newItem =
			new QTreeWidgetItem(branchesTW);

		QString const bname = toqstr(it->getBranch());
		newItem->setText(0, bname);

		QString const sel = it->getSelected() ? qt_("Yes") : qt_("No");
		newItem->setText(1, sel);

		QColor const itemcolor = rgb2qcolor(it->getColor());
		if (itemcolor.isValid()) {
			QPixmap coloritem(32, 32);
			coloritem.fill(itemcolor);
			newItem->setIcon(0, QIcon(coloritem));
		}
		// restore selected branch
		if (bname == sel_branch)
			branchesTW->setItemSelected(newItem, true);
	}
	emit changed();
}

void QBranches::apply(BufferParams & params) const
{
	params.branchlist() = branchlist_;
}

void QBranches::on_addBranchPB_pressed()
{
	QString const new_branch = newBranchLE->text();
	if (!new_branch.isEmpty()) {
		branchlist_.add(fromqstr(new_branch));
		newBranchLE->clear();
		update();
	}
}


void QBranches::on_removePB_pressed()
{
	QTreeWidgetItem * selItem =
		branchesTW->currentItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (!sel_branch.isEmpty()) {
		branchlist_.remove(fromqstr(sel_branch));
		newBranchLE->clear();
		update();
	}
}


void QBranches::on_activatePB_pressed()
{
	toggleBranch(branchesTW->currentItem());
}


void QBranches::on_branchesTW_itemDoubleClicked(QTreeWidgetItem * item, int col)
{
	toggleBranch(item);
}


void QBranches::toggleBranch(QTreeWidgetItem * item)
{
	if (item == 0)
		return;

	QString sel_branch = item->text(0);
	if (!sel_branch.isEmpty()) {
		bool const selected = item->text(1) == qt_("Yes");
		Branch * branch = branchlist_.find(fromqstr(sel_branch));
		if (branch && branch->setSelected(!selected)) {
			newBranchLE->clear();
			update();
		}
	}
}


void QBranches::on_colorPB_clicked()
{
	QTreeWidgetItem * selItem =
		branchesTW->currentItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (!sel_branch.isEmpty()) {
		string current_branch = fromqstr(sel_branch);
		Branch * branch =
			branchlist_.find(current_branch);
		if (!branch)
			return;

		QColor const initial = rgb2qcolor(branch->getColor());
		QColor ncol(QColorDialog::getColor(initial, qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget()));
		if (ncol.isValid()){
			// add the color to the branchlist
			branch->setColor(fromqstr(ncol.name()));
			newBranchLE->clear();
			update();
		}
	}
}

} // namespace frontend
} // namespace lyx
