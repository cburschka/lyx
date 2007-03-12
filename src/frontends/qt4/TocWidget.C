/**
 * \file TocWidget.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TocWidget.h"

#include "QToc.h"
#include "qt_helpers.h"

#include "debug.h"

#include <QHeaderView>
#include <QPushButton>
#include <QTreeWidgetItem>

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


TocWidget::TocWidget(QToc * form, QWidget * parent)
	: QWidget(parent), form_(form), depth_(0)
{
	setupUi(this);

	connect(form, SIGNAL(modelReset()),
		SLOT(updateGui()));

	// avoid flickering
	tocTV->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	tocTV->showColumn(0);

	// hide the pointless QHeader for now
	// in the future, new columns may appear
	// like labels, bookmarks, etc...
	// tocTV->header()->hide();
	tocTV->header()->setVisible(false);
}


void TocWidget::selectionChanged(const QModelIndex & current,
				  const QModelIndex & /*previous*/)
{
	lyxerr[Debug::GUI]
		<< "selectionChanged index " << current.row()
		<< ", " << current.column()
		<< endl;

	form_->goTo(current);
}


void TocWidget::on_updatePB_clicked()
{
	form_->updateBackend();
	form_->update();
	update();
}

/* FIXME (Ugras 17/11/06):
I have implemented a getIndexDepth function to get the model indices. In my
opinion, somebody should derive a new qvariant class for tocModelItem
which saves the string data and depth information. that will save the
depth calculation.
*/
int TocWidget::getIndexDepth(QModelIndex const & index, int depth)
{
	++depth;
	return (index.parent() == QModelIndex())? depth : getIndexDepth(index.parent(),depth);
}


void TocWidget::on_depthSL_valueChanged(int depth)
{
	if (depth == depth_)
		return;
	setTreeDepth(depth);
}


void TocWidget::setTreeDepth(int depth)
{
	depth_ = depth;

	// expanding and then collapsing is probably better, 
	// but my qt 4.1.2 doesn't have expandAll()..
	//tocTV->expandAll(); 
	QModelIndexList indices = 
		form_->tocModel()->match(form_->tocModel()->index(0,0),
		 			Qt::DisplayRole, "*", -1, 
					Qt::MatchWildcard|Qt::MatchRecursive);
	
	int size = indices.size();
	for (int i = 0; i < size; i++) {
		QModelIndex index = indices[i];
		if (getIndexDepth(index) < depth_) 
			tocTV->expand(index); 
		else
			tocTV->collapse(index); 
	}
}


void TocWidget::on_typeCO_activated(int value)
{
	form_->setTocModel(value);
	updateGui();
}


void TocWidget::on_moveUpPB_clicked()
{
	enableButtons(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableButtons(false);
		form_->goTo(list[0]);
		form_->outlineUp();
		enableButtons(true);
	}
}


void TocWidget::on_moveDownPB_clicked()
{
	enableButtons(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableButtons(false);
		form_->goTo(list[0]);
		form_->outlineDown();
		enableButtons(true);
	}
}


void TocWidget::on_moveInPB_clicked()
{
	enableButtons(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableButtons(false);
		form_->goTo(list[0]);
		form_->outlineIn();
		enableButtons(true);
	}
}


void TocWidget::on_moveOutPB_clicked()
{
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableButtons(false);
		form_->goTo(list[0]);
		form_->outlineOut();
		enableButtons(true);
	}
}


void TocWidget::select(QModelIndex const & index)
{
	if (!index.isValid()) {
		lyxerr[Debug::GUI]
			<< "TocWidget::select(): QModelIndex is invalid!" << endl;
		return;
	}

	tocTV->selectionModel()->blockSignals(true);
	tocTV->scrollTo(index);
	tocTV->selectionModel()->setCurrentIndex(index,
		QItemSelectionModel::ClearAndSelect);
	tocTV->selectionModel()->blockSignals(false);
}


void TocWidget::enableButtons(bool enable)
{
	updatePB->setEnabled(enable);

	if (!form_->canOutline())
		enable = false;

	moveUpPB->setEnabled(enable);
	moveDownPB->setEnabled(enable);
	moveInPB->setEnabled(enable);
	moveOutPB->setEnabled(enable);
}


void TocWidget::update()
{
	select(form_->getCurrentIndex());
	QWidget::update();
}


void TocWidget::updateGui()
{
	QStringListModel * type_model = form_->typeModel();
	if (type_model->stringList().isEmpty()) {
		enableButtons(false);
		typeCO->setModel(type_model);
		tocTV->setModel(new QStandardItemModel);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
		depthSL->setEnabled(false);
		return;
	}

	typeCO->setModel(type_model);
	typeCO->setCurrentIndex(form_->getType());

	bool buttons_enabled = false;
	if (form_->tocModel()) {
		buttons_enabled = form_->tocModel()->rowCount() > 0;
		tocTV->setModel(form_->tocModel());
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	enableButtons(buttons_enabled);

	reconnectSelectionModel();
	depthSL->setEnabled(true);
	depthSL->setMaximum(form_->getTocDepth());
	depthSL->setValue(depth_);
	select(form_->getCurrentIndex());

	lyxerr[Debug::GUI]
		<< "form_->tocModel()->rowCount " << form_->tocModel()->rowCount()
		<< "\nform_->tocModel()->columnCount " << form_->tocModel()->columnCount()
		<< endl;
}


void TocWidget::reconnectSelectionModel()
{
	connect(tocTV->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &,
			const QModelIndex &)),
		this, SLOT(selectionChanged(const QModelIndex &,
			const QModelIndex &)));
}

} // namespace frontend
} // namespace lyx

#include "TocWidget_moc.cpp"
