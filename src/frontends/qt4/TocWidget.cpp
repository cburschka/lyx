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

#include "GuiView.h"
#include "qt_helpers.h"
#include "TocModel.h"

#include "FuncRequest.h"
#include "LyXFunc.h"

#include "support/debug.h"

#include <QHeaderView>
#include <QTimer>

#include <vector>

using namespace std;

namespace lyx {
namespace frontend {

TocWidget::TocWidget(GuiView & gui_view, QWidget * parent)
	: QWidget(parent), depth_(0), gui_view_(gui_view)
{
	setupUi(this);

	moveOutTB->setIcon(QIcon(":/images/promote.png"));
	moveInTB->setIcon(QIcon(":/images/demote.png"));
	moveUpTB->setIcon(QIcon(":/images/up.png"));
	moveDownTB->setIcon(QIcon(":/images/down.png"));
	updateTB->setIcon(QIcon(":/images/reload.png"));

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


void TocWidget::on_tocTV_activated(QModelIndex const & index)
{
	goTo(index);
}


void TocWidget::on_tocTV_clicked(QModelIndex const & index)
{
	goTo(index);
	gui_view_.setFocus();
}


void TocWidget::goTo(QModelIndex const & index)
{
	LYXERR(Debug::GUI, "goto " << index.row()
		<< ", " << index.column());

	gui_view_.tocModels().goTo(typeCO->currentIndex(), index);
}


void TocWidget::on_updateTB_clicked()
{
	// The backend update can take some time so we disable
	// the controls while waiting.
	enableControls(false);
	gui_view_.tocModels().updateBackend();
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
	return (index.parent() == QModelIndex())
		? depth : getIndexDepth(index.parent(),depth);
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
		Qt::MatchFlags(Qt::MatchWildcard|Qt::MatchRecursive));

	int size = indices.size();
	for (int i = 0; i < size; i++) {
		QModelIndex index = indices[i];
		tocTV->setExpanded(index, getIndexDepth(index) < depth_);
	}
}


void TocWidget::on_typeCO_currentIndexChanged(int value)
{
	setTocModel(value);
}


void TocWidget::outline(int func_code)
{
	enableControls(false);
	QModelIndexList const & list = tocTV->selectionModel()->selectedIndexes();
	if (list.isEmpty())
		return;
	enableControls(false);
	goTo(list[0]);
	dispatch(FuncRequest(static_cast<FuncCode>(func_code)));
	enableControls(true);
}


void TocWidget::on_moveUpTB_clicked()
{
	outline(LFUN_OUTLINE_UP);
}


void TocWidget::on_moveDownTB_clicked()
{
	outline(LFUN_OUTLINE_DOWN);
}


void TocWidget::on_moveInTB_clicked()
{
	outline(LFUN_OUTLINE_IN);
}


void TocWidget::on_moveOutTB_clicked()
{
	outline(LFUN_OUTLINE_OUT);
}


void TocWidget::select(QModelIndex const & index)
{
	if (!index.isValid()) {
		LYXERR(Debug::GUI, "TocWidget::select(): QModelIndex is invalid!");
		return;
	}

	tocTV->scrollTo(index);
	tocTV->clearSelection();
	tocTV->setCurrentIndex(index);
}


void TocWidget::enableControls(bool enable)
{
	updateTB->setEnabled(enable);

	if (!gui_view_.tocModels().canOutline(typeCO->currentIndex()))
		enable = false;

	moveUpTB->setEnabled(enable);
	moveDownTB->setEnabled(enable);
	moveInTB->setEnabled(enable);
	moveOutTB->setEnabled(enable);

	depthSL->setEnabled(enable);
}


void TocWidget::updateView()
{
	LYXERR(Debug::GUI, "In TocWidget::updateView()");
	setTreeDepth(depth_);
	select(gui_view_.tocModels().currentIndex(typeCO->currentIndex()));
}


void TocWidget::init(QString const & str)
{
	QStringList const & type_names = gui_view_.tocModels().typeNames();
	if (type_names.isEmpty()) {
		enableControls(false);
		typeCO->clear();
		tocTV->setModel(new QStandardItemModel);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
		return;
	}

	int selected_type = gui_view_.tocModels().decodeType(str);

	QString const current_text = typeCO->currentText();
	typeCO->blockSignals(true);
	typeCO->clear();
	for (int i = 0; i != type_names.size(); ++i)
		typeCO->addItem(type_names[i]);
	if (!str.isEmpty())
		typeCO->setCurrentIndex(selected_type);
	else {
		int const new_index = typeCO->findText(current_text);
		if (new_index != -1)
			typeCO->setCurrentIndex(new_index);
		else
			typeCO->setCurrentIndex(selected_type);
	}

	typeCO->blockSignals(false);

	setTocModel(typeCO->currentIndex());
}


void TocWidget::setTocModel(size_t type)
{
	bool controls_enabled = false;
	QStandardItemModel * toc_model = gui_view_.tocModels().model(type);
	if (toc_model) {
		controls_enabled = toc_model->rowCount() > 0;
		tocTV->setModel(toc_model);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
		LYXERR(Debug::GUI, "tocModel()->rowCount "
			<< toc_model->rowCount()
			<< "\nform_->tocModel()->columnCount "
			<< toc_model->columnCount());
	}

	enableControls(controls_enabled);

	if (controls_enabled) {
		depthSL->setMaximum(gui_view_.tocModels().depth(type));
		depthSL->setValue(depth_);
	}

	// setTocModel produce QTreeView reset and setting depth again
	// is needed. That must be done after all Qt updates are processed.
	QTimer::singleShot(0, this, SLOT(updateView()));
}

} // namespace frontend
} // namespace lyx

#include "TocWidget_moc.cpp"
