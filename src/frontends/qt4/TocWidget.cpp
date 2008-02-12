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
#include "support/filetools.h"
#include "support/lstrings.h"

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

using support::FileName;
using support::libFileSearch;

namespace frontend {

TocWidget::TocWidget(QToc * form, QWidget * parent)
	: QWidget(parent), form_(form), depth_(0)
{
	setupUi(this);

	connect(form, SIGNAL(modelReset()),
		SLOT(updateGui()));

	FileName icon_path = libFileSearch("images", "promote.xpm");
	moveOutTB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path = libFileSearch("images", "demote.xpm");
	moveInTB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path = libFileSearch("images", "up.xpm");
	moveUpTB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path = libFileSearch("images", "down.xpm");
	moveDownTB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path = libFileSearch("images", "reload.xpm");
	updateTB->setIcon(QIcon(toqstr(icon_path.absFilename())));

	// avoid flickering
	tocTV->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	tocTV->showColumn(0);

	// hide the pointless QHeader for now
	// in the future, new columns may appear
	// like labels, bookmarks, etc...
	// tocTV->header()->hide();
	tocTV->header()->setVisible(false);

	// Only one item selected at a time.
	tocTV->setSelectionMode(QAbstractItemView::SingleSelection);
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


void TocWidget::on_updateTB_clicked()
{
	// The backend update can take some time so we disable
	// the controls while waiting.
	enableControls(false);
	form_->updateBackend();
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
	return (index.parent() ==
		QModelIndex())? depth : getIndexDepth(index.parent(),depth);
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

void TocWidget::on_typeCO_currentIndexChanged(int value)
{
	setTocModel(value);
}


void TocWidget::on_moveUpTB_clicked()
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


void TocWidget::on_moveDownTB_clicked()
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


void TocWidget::on_moveInTB_clicked()
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


void TocWidget::on_moveOutTB_clicked()
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

	disconnectSelectionModel();
	tocTV->setCurrentIndex(index);
	tocTV->scrollTo(index);
	reconnectSelectionModel();
}


void TocWidget::enableControls(bool enable)
{
	updateTB->setEnabled(enable);

	if (!form_->canOutline(typeCO->currentIndex()))
		enable = false;

	moveUpTB->setEnabled(enable);
	moveDownTB->setEnabled(enable);
	moveInTB->setEnabled(enable);
	moveOutTB->setEnabled(enable);

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
	vector<docstring> const & type_names = form_->typeNames();
	if (type_names.empty()) {
		enableControls(false);
		typeCO->clear();
		tocTV->setModel(new QStandardItemModel);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
		return;
	}

	typeCO->blockSignals(true);
	typeCO->clear();
	for (size_t i = 0; i != type_names.size(); ++i) {
		QString item = toqstr(type_names[i]);
		typeCO->addItem(item);
	}
	if (form_->selectedType() != -1)
		typeCO->setCurrentIndex(form_->selectedType());
	typeCO->blockSignals(false);

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
		this,
		SLOT(selectionChanged(const QModelIndex &,
		     const QModelIndex &)));
}

void TocWidget::disconnectSelectionModel()
{
	disconnect(tocTV->selectionModel(),
		   SIGNAL(currentChanged(const QModelIndex &,
			  const QModelIndex &)),
		   this,
		   SLOT(selectionChanged(const QModelIndex &,
			const QModelIndex &)));
}

} // namespace frontend
} // namespace lyx

#include "TocWidget_moc.cpp"
