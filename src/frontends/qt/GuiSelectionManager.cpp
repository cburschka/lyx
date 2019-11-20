/**
 * \file GuiSelectionManager.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 * \author Et Alia
 *
 * Some of the material in this file previously appeared in
 * GuiCitationDialog.cpp.
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSelectionManager.h"
#include "qt_helpers.h"

#include "support/debug.h"

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QItemSelection>
#include <QListView>
#include <QKeyEvent>
#include <QPushButton>

#ifdef KeyPress
#undef KeyPress
#endif

#ifdef ControlModifier
#undef ControlModifier
#endif

#ifdef FocusIn
#undef FocusIn
#endif


namespace lyx {
namespace frontend {

GuiSelectionManager::GuiSelectionManager(QObject * parent,
                                         QAbstractItemView * avail,
                                         QAbstractItemView * sel,
                                         QPushButton * add,
                                         QPushButton * del,
                                         QPushButton * up,
                                         QPushButton * down,
                                         QAbstractItemModel * amod,
                                         QAbstractItemModel * smod,
                                         int const main_sel_col)
: QObject(parent), availableLV(avail), selectedLV(sel),
  addPB(add), deletePB(del), upPB(up), downPB(down),
  availableModel(amod), selectedModel(smod),
  selectedHasFocus_(false), main_sel_col_(main_sel_col),
  allow_multi_selection_(false)
{
	selectedLV->setModel(smod);
	availableLV->setModel(amod);
	selectedLV->setSelectionBehavior(QAbstractItemView::SelectRows);
	selectedLV->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(availableLV->selectionModel(),
	        SIGNAL(currentChanged(QModelIndex, QModelIndex)),
	        this, SLOT(availableChanged(QModelIndex, QModelIndex)));
	connect(selectedLV->selectionModel(),
	        SIGNAL(currentChanged(QModelIndex, QModelIndex)),
	        this, SLOT(selectedChanged(QModelIndex, QModelIndex)));
	connect(availableLV->selectionModel(),
	        SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
	        this, SLOT(availableChanged(QItemSelection, QItemSelection)));
	connect(availableLV->selectionModel(),
		SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
		this, SLOT(updateButtons()));
	connect(selectedLV->selectionModel(),
	        SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
	        this, SLOT(selectedChanged(QItemSelection, QItemSelection)));
	connect(selectedLV->selectionModel(),
		SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
		this, SLOT(updateButtons()));
	connect(selectedLV->itemDelegate(), SIGNAL(commitData(QWidget*)),
		this, SLOT(selectedEdited()));
	connect(addPB, SIGNAL(clicked()),
	        this, SLOT(addPB_clicked()));
	connect(deletePB, SIGNAL(clicked()),
	        this, SLOT(deletePB_clicked()));
	connect(upPB, SIGNAL(clicked()),
	        this, SLOT(upPB_clicked()));
	connect(downPB, SIGNAL(clicked()),
	        this, SLOT(downPB_clicked()));
	connect(availableLV, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(availableLV_doubleClicked(QModelIndex)));

	availableLV->installEventFilter(this);
	selectedLV->installEventFilter(this);
}


void GuiSelectionManager::update()
{
	updateAddPB();
	updateDelPB();
	updateDownPB();
	updateUpPB();
}


void GuiSelectionManager::updateButtons()
{
	update();
	updateHook();
}


QModelIndex GuiSelectionManager::getSelectedIndex(int const c) const
{
	QModelIndexList avail = availableLV->selectionModel()->selectedIndexes();
	QModelIndexList sel   = selectedLV->selectionModel()->selectedRows(c);
	bool const have_avl = !avail.isEmpty();
	bool const have_sel = !sel.isEmpty();

	if (selectedFocused()) {
		if (have_sel)
			return sel.front();
		if (have_avl)
			return avail.first();
	}
	else { // available has focus
		if (have_avl)
			return avail.first();
		if (have_sel)
			return sel.front();
	}
	return QModelIndex();
}


void GuiSelectionManager::updateAddPB()
{
	int const arows = availableModel->rowCount();
	QModelIndexList const availSels =
		availableLV->selectionModel()->selectedIndexes();
	addPB->setEnabled(arows > 0 &&
		!availSels.isEmpty() &&
		(allow_multi_selection_ || !isSelected(availSels.first())));
}


void GuiSelectionManager::updateDelPB()
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		deletePB->setEnabled(false);
		return;
	}
	QModelIndexList const selSels =
		selectedLV->selectionModel()->selectedIndexes();
	int const sel_nr = selSels.empty() ? -1 : selSels.first().row();
	deletePB->setEnabled(sel_nr >= 0);
}


void GuiSelectionManager::updateUpPB()
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		upPB->setEnabled(false);
		return;
	}
	QModelIndexList const selSels =
			selectedLV->selectionModel()->selectedIndexes();
	int const sel_nr = selSels.empty() ? -1 : selSels.first().row();
	upPB->setEnabled(sel_nr > 0);
}


void GuiSelectionManager::updateDownPB()
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		downPB->setEnabled(false);
		return;
	}
	QModelIndexList const selSels =
			selectedLV->selectionModel()->selectedIndexes();
	int const sel_nr = selSels.empty() ? -1 : selSels.first().row();
	downPB->setEnabled(sel_nr >= 0 && sel_nr < srows - 1);
}


bool GuiSelectionManager::isSelected(const QModelIndex & idx)
{
	if (selectedModel->rowCount() == 0)
		return false;
	QVariant const & str = availableModel->data(idx, Qt::DisplayRole);
	QModelIndexList qmil =
			selectedModel->match(selectedModel->index(0, main_sel_col_),
			                     Qt::DisplayRole, str, 1,
			                     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap));
	return !qmil.empty();
}


void GuiSelectionManager::availableChanged(QItemSelection const & qis, QItemSelection const &)
{
	QModelIndexList il = qis.indexes();
	if (il.empty())
		return;
	availableChanged(il.front(), QModelIndex());
}


void GuiSelectionManager::availableChanged(const QModelIndex & idx, const QModelIndex &)
{
	if (!idx.isValid())
		return;

	selectedHasFocus_ = false;
	updateHook();
}


void GuiSelectionManager::selectedChanged(QItemSelection const & qis, QItemSelection const &)
{
	QModelIndexList il = qis.indexes();
	if (il.empty())
		return;
	selectedChanged(il.front(), QModelIndex());
}


void GuiSelectionManager::selectedChanged(const QModelIndex & idx, const QModelIndex &)
{
	if (!idx.isValid())
		return;

	selectedHasFocus_ = true;
	updateHook();
}


void GuiSelectionManager::selectedEdited()
{
	selectionChanged();
}


bool GuiSelectionManager::insertRowToSelected(int i,
		QMap<int, QVariant> const & itemData)
{
	if (i <= -1)
		i = 0;
	if (i > selectedModel->rowCount())
		i = selectedModel->rowCount();
	if (!selectedModel->insertRow(i))
		return false;
	return selectedModel->setItemData(selectedModel->index(i, main_sel_col_), itemData);
}


bool GuiSelectionManager::insertRowToSelected(int i, QMap<int, QMap<int, QVariant>> & qms)
{
	if (i <= -1)
		i = 0;
	if (i > selectedModel->rowCount())
		i = selectedModel->rowCount();
	if (!selectedModel->insertRow(i))
		return false;
	bool res = true;
	QMap<int, QMap<int, QVariant>>::const_iterator it = qms.constBegin();
	for (; it != qms.constEnd(); ++it)
		res &= selectedModel->setItemData(selectedModel->index(i, it.key()), it.value());
	return res;
}


void GuiSelectionManager::addPB_clicked()
{
	QModelIndexList selIdx =
		availableLV->selectionModel()->selectedIndexes();
	if (selIdx.isEmpty())
		return;

	QModelIndex const idxToAdd = selIdx.first();
	int const srows = selectedModel->rowCount();

	QMap<int, QVariant> qm = availableModel->itemData(idxToAdd);
	insertRowToSelected(srows, qm);

	selectionChanged(); //signal
	
	QModelIndex const idx = selectedLV->currentIndex();
	if (idx.isValid())
		selectedLV->setCurrentIndex(idx);

	updateHook();
}


void GuiSelectionManager::deletePB_clicked()
{
	QModelIndexList selIdx =
		selectedLV->selectionModel()->selectedIndexes();
	if (selIdx.isEmpty())
		return;
	QModelIndex idx = selIdx.first();

	int const row = idx.row();
	int nrows = selectedLV->model()->rowCount();

	selectedModel->removeRow(row);
	selectionChanged(); //signal

	// select previous item
	if (nrows > 0)
		selectedLV->setCurrentIndex(selectedLV->model()->index(row - 1, 0));
	else if (nrows == 0)
		selectedLV->setCurrentIndex(selectedLV->model()->index(0, 0));
	selectedHasFocus_ = (nrows > 1);
	updateHook();
}


void GuiSelectionManager::upPB_clicked()
{
	QModelIndexList selIdx =
		selectedLV->selectionModel()->selectedIndexes();
	if (selIdx.isEmpty())
		return;
	QModelIndex idx = selIdx.first();

	int const pos = idx.row();
	if (pos <= 0)
		return;

	QMap<int, QMap<int, QVariant>> qms;
	QList<QModelIndex>::const_iterator it = selIdx.constBegin();
	for (; it != selIdx.constEnd(); ++it)
		qms[it->column()] = selectedModel->itemData(*it);

	selectedModel->removeRow(pos);
	insertRowToSelected(pos - 1, qms);

	idx = selIdx.first();
	selectedLV->setCurrentIndex(idx.sibling(idx.row() - 1, idx.column()));
	selectedHasFocus_ = true;
	updateHook();
}


void GuiSelectionManager::downPB_clicked()
{
	QModelIndexList selIdx =
		selectedLV->selectionModel()->selectedIndexes();
	if (selIdx.isEmpty())
		return;
	QModelIndex idx = selIdx.first();

	int const pos = idx.row();
	if (pos >= selectedModel->rowCount() - 1)
		return;

	QMap<int, QMap<int, QVariant>> qms;
	QList<QModelIndex>::const_iterator it = selIdx.constBegin();
	for (; it != selIdx.constEnd(); ++it)
		qms[it->column()] = selectedModel->itemData(*it);

	selectedModel->removeRow(pos);
	insertRowToSelected(pos + 1, qms);

	idx = selIdx.first();
	selectedLV->setCurrentIndex(idx.sibling(idx.row() + 1, idx.column()));
	selectedHasFocus_ = true;
	updateHook();
}


void GuiSelectionManager::availableLV_doubleClicked(const QModelIndex & idx)
{
	if (isSelected(idx) || !addPB->isEnabled())
		return;

	if (idx.isValid())
		selectedHasFocus_ = false;
	addPB_clicked();
	//updateHook() will be emitted there
}


bool GuiSelectionManager::eventFilter(QObject * obj, QEvent * event)
{
	QEvent::Type etype = event->type();
	if (obj == availableLV) {
		if (etype == QEvent::KeyPress) {
			QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
			int const keyPressed = keyEvent->key();
			Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();
			// Enter key without modifier will add current item.
			// Ctrl-Enter will add it and close the dialog.
			// This is designed to work both with the main enter key
			// and the one on the numeric keypad.
			if (keyPressed == Qt::Key_Enter || keyPressed == Qt::Key_Return) {
				if (!keyModifiers ||
				    keyModifiers == Qt::ControlModifier ||
				    keyModifiers == Qt::KeypadModifier  ||
				    keyModifiers == (Qt::ControlModifier
				                     | Qt::KeypadModifier)) {
					if (addPB->isEnabled()) {
						addPB_clicked();
					}
					if (keyModifiers)
						okHook(); //signal
				}
				event->accept();
				return true;
			}
			else if (keyPressed == Qt::Key_Right) {
				focusAndHighlight(selectedLV);
				event->accept();
				return true;
			}
		} else if (etype == QEvent::FocusIn) {
			if (selectedHasFocus_) {
				selectedHasFocus_ = false;
				updateHook();
			}
			return false;
		}
	} else if (obj == selectedLV) {
		if (etype == QEvent::KeyPress) {
			QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
			int const keyPressed = keyEvent->key();
			Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();
			// Delete or backspace key will delete current item
			// ...with control modifier will clear the list
			if (keyPressed == Qt::Key_Delete || keyPressed == Qt::Key_Backspace) {
				if (keyModifiers == Qt::NoModifier && deletePB->isEnabled()) {
					deletePB_clicked();
					updateHook();
				} else if (keyModifiers == Qt::ControlModifier) {
					selectedModel->removeRows(0, selectedModel->rowCount());
					updateHook();
				} else
					return QObject::eventFilter(obj, event);
			}
			// Ctrl-Up activates upPB
			else if (keyPressed == Qt::Key_Up) {
				if (keyModifiers == Qt::ControlModifier) {
					if (upPB->isEnabled())
						upPB_clicked();
					event->accept();
					return true;
				}
			}
			// Ctrl-Down activates downPB
			else if (keyPressed == Qt::Key_Down) {
				if (keyModifiers == Qt::ControlModifier) {
					if (downPB->isEnabled())
						downPB_clicked();
					event->accept();
					return true;
				}
			}
			else if (keyPressed == Qt::Key_Left) {
				focusAndHighlight(availableLV);
				event->accept();
				return true;
			}
		} else if (etype == QEvent::FocusIn) {
			if (!selectedHasFocus_) {
				selectedHasFocus_ = true;
				updateHook();
			}
			return false;
		}
	}
	return QObject::eventFilter(obj, event);
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiSelectionManager.cpp"
