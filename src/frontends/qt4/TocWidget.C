/**
 * \file TocWidget.cpp
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
	LYXERR(Debug::GUI)
		<< "selectionChanged index " << current.row()
		<< ", " << current.column()
		<< endl;

	form_->goTo(typeCO->currentIndex(), current);
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
	QModelIndexList indices = tocTV->model()->match(
		tocTV->model()->index(0,0),
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
	setTocModel(value);
}


void TocWidget::on_moveUpPB_clicked()
{
	enableControls(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableControls(false);
		form_->goTo(typeCO->currentIndex(), list[0]);
		form_->outlineUp();
		enableControls(true);
	}
}


void TocWidget::on_moveDownPB_clicked()
{
	enableControls(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableControls(false);
		form_->goTo(typeCO->currentIndex(), list[0]);
		form_->outlineDown();
		enableControls(true);
	}
}


void TocWidget::on_moveInPB_clicked()
{
	enableControls(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableControls(false);
		form_->goTo(typeCO->currentIndex(), list[0]);
		form_->outlineIn();
		enableControls(true);
	}
}


void TocWidget::on_moveOutPB_clicked()
{
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		enableControls(false);
		form_->goTo(typeCO->currentIndex(), list[0]);
		form_->outlineOut();
		enableControls(true);
	}
}


void TocWidget::select(QModelIndex const & index)
{
	if (!index.isValid()) {
		LYXERR(Debug::GUI)
			<< "TocWidget::select(): QModelIndex is invalid!" << endl;
		return;
	}

	tocTV->selectionModel()->blockSignals(true);
	tocTV->scrollTo(index);
	tocTV->selectionModel()->setCurrentIndex(index,
		QItemSelectionModel::ClearAndSelect);
	tocTV->selectionModel()->blockSignals(false);
}


void TocWidget::enableControls(bool enable)
{
	updatePB->setEnabled(enable);

	if (!form_->canOutline(typeCO->currentIndex()))
		enable = false;

	moveUpPB->setEnabled(enable);
	moveDownPB->setEnabled(enable);
	moveInPB->setEnabled(enable);
	moveOutPB->setEnabled(enable);

	depthSL->setEnabled(enable);
}


void TocWidget::update()
{
	LYXERR(Debug::GUI) << "In TocWidget::update()" << endl;
	select(form_->getCurrentIndex(typeCO->currentIndex()));
	QWidget::update();
}


void TocWidget::updateGui()
{
	QStringListModel * type_model = form_->typeModel();
	if (type_model->stringList().isEmpty()) {
		enableControls(false);
		typeCO->setModel(type_model);
		tocTV->setModel(new QStandardItemModel);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
		return;
	}

	QString current_text = typeCO->currentText();
	typeCO->setModel(type_model);
	int const current_type = typeCO->findText(current_text);
	if (current_type != -1)
		typeCO->setCurrentIndex(current_type);
	else
		typeCO->setCurrentIndex(form_->selectedType());

	setTocModel(typeCO->currentIndex());
}


void TocWidget::setTocModel(size_t type)
{
	bool controls_enabled = false;
	QStandardItemModel * toc_model = form_->tocModel(type);
	if (toc_model) {
		controls_enabled = toc_model->rowCount() > 0;
		tocTV->setModel(toc_model);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	enableControls(controls_enabled);

	reconnectSelectionModel();

	if (controls_enabled) {
		depthSL->setMaximum(form_->getTocDepth(type));
		depthSL->setValue(depth_);
	}

	LYXERR(Debug::GUI) << "In TocWidget::updateGui()" << endl;

	select(form_->getCurrentIndex(typeCO->currentIndex()));

	if (toc_model) {
		LYXERR(Debug::GUI)
		<< "form_->tocModel()->rowCount " 
			<< toc_model->rowCount()
			<< "\nform_->tocModel()->columnCount "
			<< toc_model->columnCount()
			<< endl;
	}
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
