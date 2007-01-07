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

#include <QTreeWidgetItem>
#include <QPushButton>
#include <QCloseEvent>
#include <QHeaderView>

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

QTocDialog::QTocDialog(Dialog & dialog, QToc * form)
	: Dialog::View(dialog, _("Toc")), form_(form), depth_(2)
{
	setupUi(this);

	updateGui();

	connect(tocTV->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &,
			const QModelIndex &)),
		this, SLOT(selectionChanged(const QModelIndex &,
			const QModelIndex &)));
}


QTocDialog::~QTocDialog()
{
	accept();
}


void QTocDialog::selectionChanged(const QModelIndex & current,
				  const QModelIndex & /*previous*/)
{
	lyxerr[Debug::GUI]
		<< "selectionChanged index " << current.row()
		<< ", " << current.column()
		<< endl;

	form_->goTo(current);
}


void QTocDialog::on_closePB_clicked()
{
	accept();
}


void QTocDialog::on_updatePB_clicked()
{
	update();
}

/* FIXME (Ugras 17/11/06):
I have implemented a getIndexDepth function to get the model indices. In my
opinion, somebody should derive a new qvariant class for tocModelItem
which saves the string data and depth information. that will save the
depth calculation.
*/
int QTocDialog::getIndexDepth(QModelIndex const & index, int depth)
{
	++depth;
	return (index.parent() == QModelIndex())? depth : getIndexDepth(index.parent(),depth);
}


void QTocDialog::on_depthSL_valueChanged(int depth)
{
	if (depth == depth_)
		return;
	setTreeDepth(depth);
}


void QTocDialog::setTreeDepth(int depth)
{
	if(depth!=-1)
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


void QTocDialog::on_typeCO_activated(int value)
{
	form_->setTocModel(value);
	tocTV->setModel(form_->tocModel());
	reconnectSelectionModel();
	enableButtons();
	update();
}


void QTocDialog::on_moveUpPB_clicked()
{
	enableButtons(false);
	QModelIndex index = tocTV->selectionModel()->selectedIndexes()[0];
	form_->goTo(index);
	form_->outlineUp();
	update();
}


void QTocDialog::on_moveDownPB_clicked()
{
	enableButtons(false);
	QModelIndex index = tocTV->selectionModel()->selectedIndexes()[0];
	form_->goTo(index);
	form_->outlineDown();
	update();
}


void QTocDialog::on_moveInPB_clicked()
{
	enableButtons(false);
	QModelIndex index = tocTV->selectionModel()->selectedIndexes()[0];
	form_->goTo(index);
	form_->outlineIn();
	update();
}


void QTocDialog::on_moveOutPB_clicked()
{
	enableButtons(false);
	QModelIndex index = tocTV->selectionModel()->selectedIndexes()[0];
	form_->goTo(index);
	form_->outlineOut();
	update();
}


void QTocDialog::select(QModelIndex const & index)
{
//	tocTV->setModel(form_->tocModel());

	if (!index.isValid()) {
		lyxerr[Debug::GUI]
			<< "QTocDialog::select(): QModelIndex is invalid!" << endl;
		return;
	}

	tocTV->scrollTo(index);
	tocTV->selectionModel()->select(index, QItemSelectionModel::Select);
}


void QTocDialog::enableButtons(bool enable)
{
	updatePB->setEnabled(enable);

	if (!form_->canOutline())
		enable = false;

	moveUpPB->setEnabled(enable);
	moveDownPB->setEnabled(enable);
	moveInPB->setEnabled(enable);
	moveOutPB->setEnabled(enable);
}


void QTocDialog::update()
{
	form_->updateToc();
	updateGui();
}


void QTocDialog::updateGui()
{
	QStringListModel * type_model = form_->typeModel();
	if (type_model->stringList().isEmpty()) {
		enableButtons();
		typeCO->setModel(type_model);
		tocTV->setModel(new QStandardItemModel);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
		depthSL->setEnabled(false);
		return;
	}

	typeCO->setModel(type_model);
	typeCO->setCurrentIndex(form_->getType());

	if (form_->tocModel()) {
		tocTV->setModel(form_->tocModel());
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
	// avoid flickering
	tocTV-> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	tocTV->showColumn(0);
	// hide the pointless QHeader for now
	// in the future, new columns may appear
	// like labels, bookmarks, etc...
	// tocTV->header()->hide();
	tocTV->header()->setVisible(false);
	enableButtons();

	reconnectSelectionModel();
	depthSL->setEnabled(true);
	depthSL->setMaximum(form_->getTocDepth());
	setTreeDepth();
	select(form_->getCurrentIndex());

	lyxerr[Debug::GUI]
		<< "form_->tocModel()->rowCount " << form_->tocModel()->rowCount()
		<< "\nform_->tocModel()->columnCount " << form_->tocModel()->columnCount()
		<< endl;
//	setTitle(form_->guiname())
}


void QTocDialog::reconnectSelectionModel()
{
	connect(tocTV->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &,
			const QModelIndex &)),
		this, SLOT(selectionChanged(const QModelIndex &,
			const QModelIndex &)));
}


void QTocDialog::apply()
{
	// Nothing to do here... for now.
	// Ideas welcome... (Abdel, 17042006)
}


void QTocDialog::hide()
{
	accept();
}


void QTocDialog::show()
{
	form_->update();
	QDialog::show();
}


bool QTocDialog::isVisible() const
{
	return QDialog::isVisible();
}


} // namespace frontend
} // namespace lyx

#include "QTocDialog_moc.cpp"
