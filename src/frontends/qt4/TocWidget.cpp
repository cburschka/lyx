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

#include "Buffer.h"
#include "FuncRequest.h"
#include "LyXFunc.h"

#include "support/debug.h"
#include "support/lassert.h"

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

	// The toc types combo won't change its model.
	typeCO->setModel(gui_view_.tocModels().nameModel());
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

	gui_view_.tocModels().goTo(current_type_, index);
}


void TocWidget::on_updateTB_clicked()
{
	// The backend update can take some time so we disable
	// the controls while waiting.
	enableControls(false);
	gui_view_.tocModels().updateBackend();
}


/* FIXME (Ugras 17/11/06):
I have implemented a indexDepth function to get the model indices. In my
opinion, somebody should derive a new qvariant class for tocModelItem
which saves the string data and depth information. That will save the
depth calculation.  */

static int indexDepth(QModelIndex const & index, int depth = -1)
{
	++depth;
	return index.parent() == QModelIndex()
		? depth : indexDepth(index.parent(), depth);
}


void TocWidget::on_depthSL_valueChanged(int depth)
{
	if (depth == depth_)
		return;
	setTreeDepth(depth);
	gui_view_.setFocus();
}


void TocWidget::setTreeDepth(int depth)
{
	depth_ = depth;
	if (!tocTV->model())
		return;

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
		tocTV->setExpanded(index, indexDepth(index) < depth_);
	}
}


void TocWidget::on_typeCO_currentIndexChanged(int index)
{
	current_type_ = typeCO->itemData(index).toString();
	updateView();
	gui_view_.setFocus();
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
	gui_view_.setFocus();
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


/// Test if outlining operation is possible
static bool canOutline(QString const & type)
{
	return type == "tableofcontents";
}


void TocWidget::enableControls(bool enable)
{
	updateTB->setEnabled(enable);

	if (!canOutline(current_type_))
		enable = false;

	moveUpTB->setEnabled(enable);
	moveDownTB->setEnabled(enable);
	moveInTB->setEnabled(enable);
	moveOutTB->setEnabled(enable);
}


/// Test if synchronized navigation is possible
static bool canNavigate(QString const & type)
{
	// It is not possible to have synchronous navigation in a correctl
	// and efficient way with the label type because Toc::item() do a linear
	// seatch. Even if fixed, it might even not be desirable to do so if we 
	// want to support drag&drop of labels and references.
	return type != "label";
}


void TocWidget::updateView()
{
	if (!gui_view_.view()) {
		enableControls(false);
		typeCO->setEnabled(false);
		tocTV->setModel(0);
		tocTV->setEnabled(false);
		return;
	}
	typeCO->setEnabled(true);
	tocTV->setEnabled(true);

	QStandardItemModel * toc_model = gui_view_.tocModels().model(current_type_);	
	if (tocTV->model() != toc_model) {
		tocTV->setModel(toc_model);
		tocTV->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
	bool controls_enabled = toc_model && toc_model->rowCount() > 0
		&& !gui_view_.buffer()->isReadonly();
	enableControls(controls_enabled);

	depthSL->setMaximum(gui_view_.tocModels().depth(current_type_));
	depthSL->setValue(depth_);
	setTreeDepth(depth_);
	if (canNavigate(current_type_))
		select(gui_view_.tocModels().currentIndex(current_type_));
}


static QString decodeType(QString const & str)
{
	QString type = str;
	if (type.contains("tableofcontents")) {
		type = "tableofcontents";
	} else if (type.contains("floatlist")) {
		if (type.contains("\"figure"))
			type = "figure";
		else if (type.contains("\"table"))
			type = "table";
		else if (type.contains("\"algorithm"))
			type = "algorithm";
	}
	return type;
}


void TocWidget::init(QString const & str)
{
	int new_index;
	if (str.isEmpty())
		new_index = typeCO->findData(current_type_);
	else
		new_index = typeCO->findData(decodeType(str));

	// If everything else fails, settle on the table of contents which is
	// guaranted to exist.
	if (new_index == -1) {
		current_type_ = "tableofcontents";
		new_index = typeCO->findData(current_type_);
	}

	typeCO->blockSignals(true);
	typeCO->setCurrentIndex(new_index);
	typeCO->blockSignals(false);
}

} // namespace frontend
} // namespace lyx

#include "TocWidget_moc.cpp"
