/**
 * \file QTocDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTocDialog.h"
#include "QToc.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "controllers/ControlToc.h"

#include "debug.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QCloseEvent>

#include <vector>
#include <string>
#include <stack>

using std::endl;
using std::pair;
using std::stack;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {

QTocDialog::QTocDialog(QToc * form)
	: form_(form), depth_(2)
{
	setupUi(this);

	// Manage the cancel/close button
	form_->bcview().setCancel(closePB);

	// disable sorting
	tocTW->setSortingEnabled(false);
	tocTW->setColumnCount(1);

	// hide the pointless QHeader
//	QWidget * w = static_cast<QWidget*>(tocTW->child("list view header"));
//	if (w)
//		w->hide();

//	connect(closePB, SIGNAL(clicked()),
//		form, SLOT(slotClose()));
}


QTocDialog::~QTocDialog()
{
	accept();
}

void QTocDialog::on_tocTW_currentItemChanged(QTreeWidgetItem * current,
								 QTreeWidgetItem * previous)
{
	form_->select(fromqstr(current->text(0)));
}

void QTocDialog::on_closePB_clicked()
{
	accept();
}

void QTocDialog::on_updatePB_clicked()
{
	form_->update();
}

void QTocDialog::on_depthSL_valueChanged(int depth)
{
	if (depth == depth_)
		return;

	depth_ = depth;
	updateToc(true);
}

void QTocDialog::on_typeCO_activated(int value)
{
	updateToc();
}

void QTocDialog::on_moveUpPB_clicked()
{
	enableButtons(false);
	form_->moveUp();
	enableButtons();
}

void QTocDialog::on_moveDownPB_clicked()
{
	enableButtons(false);
	form_->moveDown();
	enableButtons();
}

void QTocDialog::on_moveInPB_clicked()
{
	enableButtons(false);
	form_->moveIn();
	enableButtons();
}

void QTocDialog::on_moveOutPB_clicked()
{
	enableButtons(false);
	form_->moveOut();
	enableButtons();
}

void QTocDialog::enableButtons(bool enable)
{
	moveUpPB->setEnabled(enable);
	moveDownPB->setEnabled(enable);
	moveInPB->setEnabled(enable);
	moveOutPB->setEnabled(enable);
	updatePB->setEnabled(enable);
}

void QTocDialog::updateType()
{
	typeCO->clear();

	vector<string> const & choice = form_->controller().getTypes();
	string const & type = toc::getType(form_->controller().params().getCmdName());

	for (vector<string>::const_iterator it = choice.begin();
		it != choice.end(); ++it) {
		string const & guiname = form_->controller().getGuiName(*it);
		typeCO->insertItem(toqstr(guiname));
		if (*it == type) {
			typeCO->setCurrentItem(it - choice.begin());
			form_->setTitle(guiname);
		}
	}
}

void QTocDialog::updateToc(bool newdepth)
{
	vector<string> const & choice = form_->controller().getTypes();
	string type;
	if (!choice.empty())
		type = choice[typeCO->currentItem()];

	toc::Toc const & contents = form_->controller().getContents(type);

	// Check if all elements are the same.
	if (!newdepth && form_->get_toclist() == contents) {
		return;
	}

	tocTW->clear();

	form_->get_toclist() = contents;

	if (form_->get_toclist().empty())
		return;

	tocTW->setUpdatesEnabled(false);

	QTreeWidgetItem * topLevelItem;

	toc::Toc::const_iterator iter = form_->get_toclist().begin();

	while (iter != form_->get_toclist().end()) {

		if (iter->depth == 1) {
			topLevelItem = new QTreeWidgetItem(tocTW);
			topLevelItem->setText(0, toqstr(iter->str));
			if (iter->depth < depth_) tocTW->collapseItem(topLevelItem);

			lyxerr[Debug::GUI]
				<< "Table of contents\n"
				<< "Added Top Level item " << iter->str
				<< " at depth " << iter->depth
				<< endl;

			populateItem(topLevelItem, iter);
		}

		if (iter == form_->get_toclist().end())
			break;

		++iter;
	}

	tocTW->setUpdatesEnabled(true);
	tocTW->update();
	form_->setTitle(fromqstr(typeCO->currentText()));
	tocTW->show();
}

void QTocDialog::populateItem(QTreeWidgetItem * parentItem, toc::Toc::const_iterator& iter)
{
	int curdepth = iter->depth+1;
	QTreeWidgetItem * item;

	while (iter != form_->get_toclist().end()) {

		++iter;

		if (iter == form_->get_toclist().end())
			break;

		if (iter->depth < curdepth) {
			--iter;
			return;
		}
		if (iter->depth > curdepth) {
//			--iter;
			return;
		}

		item = new QTreeWidgetItem(parentItem);
		item->setText(0, toqstr(iter->str));

		if (iter->depth < depth_) tocTW->collapseItem(item);
		else tocTW->expandItem(item);

		lyxerr[Debug::GUI]
			<< "Table of contents: Added item " << iter->str
			<< " at depth " << iter->depth
			<< "  \", parent \""
			<< fromqstr(parentItem->text(0)) << '"'
			<< endl;

		populateItem(item, iter);
	}
}


void QTocDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
