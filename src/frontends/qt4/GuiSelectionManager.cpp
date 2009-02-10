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

#include "support/debug.h"

#include <QKeyEvent>
#include <QListView>
#include <QPushButton>
#include <QAbstractListModel>


namespace lyx {
namespace frontend {

GuiSelectionManager::GuiSelectionManager(
	QListView * avail, 
	QListView * sel,
	QPushButton * add, 
	QPushButton * del, 
	QPushButton * up, 
	QPushButton * down,
	QAbstractListModel * amod,
	QAbstractListModel * smod)
{
	availableLV = avail;
	selectedLV = sel;
	addPB = add;
	deletePB = del;
	upPB = up;
	downPB = down;
	availableModel = amod;
	selectedModel = smod;
	
	selectedLV->setModel(smod);
	availableLV->setModel(amod);
	
	connect(availableLV->selectionModel(),
	        SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	        this, SLOT(availableChanged(QModelIndex, QModelIndex)));
	connect(selectedLV->selectionModel(),
	        SIGNAL(currentChanged(QModelIndex, QModelIndex)),
	        this, SLOT(selectedChanged(QModelIndex, QModelIndex)));
	connect(addPB, SIGNAL(clicked()), 
	        this, SLOT(addPB_clicked()));
	connect(deletePB, SIGNAL(clicked()), 
	        this, SLOT(deletePB_clicked()));
	connect(upPB, SIGNAL(clicked()), 
	        this, SLOT(upPB_clicked()));
	connect(downPB, SIGNAL(clicked()), 
	        this, SLOT(downPB_clicked()));
	connect(availableLV, SIGNAL(clicked(QModelIndex)), 
	        this, SLOT(availableLV_clicked(QModelIndex)));
	connect(availableLV, SIGNAL(doubleClicked(QModelIndex)), 
	        this, SLOT(availableLV_doubleClicked(QModelIndex)));
	connect(selectedLV, SIGNAL(clicked(QModelIndex)), 
	        this, SLOT(selectedLV_clicked(QModelIndex)));
	
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


void GuiSelectionManager::updateAddPB()
{
	int const arows = availableModel->rowCount();
	QModelIndexList const availSels = 
		availableLV->selectionModel()->selectedIndexes();
	addPB->setEnabled(arows > 0 &&
		!availSels.isEmpty() &&
		!isSelected(availSels.first()));
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
	int const sel_nr = 	selSels.empty() ? -1 : selSels.first().row();
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
	int const sel_nr = 	selSels.empty() ? -1 : selSels.first().row();
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
	int const sel_nr = 	selSels.empty() ? -1 : selSels.first().row();
	downPB->setEnabled(sel_nr >= 0 && sel_nr < srows - 1);
}


bool GuiSelectionManager::isSelected(const QModelIndex & idx)
{
	if (selectedModel->rowCount() == 0)
		return false;
	QVariant const & str = availableModel->data(idx, Qt::DisplayRole);
	QModelIndexList qmil = 
			selectedModel->match(selectedModel->index(0), 
			                     Qt::DisplayRole, str, 1,
			                     Qt::MatchExactly | Qt::MatchWrap);
	return !qmil.empty();
}


void GuiSelectionManager::availableChanged(const QModelIndex & idx, const QModelIndex &)
{
	if (!idx.isValid())
		return;
	
	selectedHasFocus_ = false;
	updateHook();
}


void GuiSelectionManager::selectedChanged(const QModelIndex & idx, const QModelIndex &)
{
	if (!idx.isValid())
		return;
	
	selectedHasFocus_ = true;
	updateHook();
}


bool GuiSelectionManager::insertRowToSelected(int i, 
		QMap<int, QVariant> const & itemData)
{
	if (i <= -1 || i > selectedModel->rowCount())
		return false;
	if (!selectedModel->insertRow(i))
		return false;
	return selectedModel->setItemData(selectedModel->index(i), itemData);
}


void GuiSelectionManager::addPB_clicked()
{
	QModelIndexList selIdx =
		availableLV->selectionModel()->selectedIndexes();
	if (selIdx.isEmpty())
		return;

	QModelIndex const idxToAdd = selIdx.first();
	QModelIndex const idx = selectedLV->currentIndex();
	int const srows = selectedModel->rowCount();
	
	QMap<int, QVariant> qm = availableModel->itemData(idxToAdd);
	insertRowToSelected(srows, qm);
	
	selectionChanged(); //signal

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
	selectedModel->removeRow(idx.row());
	selectionChanged(); //signal
	
	int nrows = selectedLV->model()->rowCount();
	if (idx.row() == nrows) //was last item on list
		idx = idx.sibling(idx.row() - 1, idx.column());
	
	if (nrows > 1)
		selectedLV->setCurrentIndex(idx);
	else if (nrows == 1)
		selectedLV->setCurrentIndex(selectedLV->model()->index(0, 0));
	selectedHasFocus_ = (nrows > 0);
	updateHook();
}


void GuiSelectionManager::upPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();

	int const pos = idx.row();
	if (pos <= 0)
		return;
	
	QMap<int, QVariant> qm = selectedModel->itemData(idx);

	selectedModel->removeRow(pos);
	insertRowToSelected(pos - 1, qm);

	selectionChanged(); //signal

	selectedLV->setCurrentIndex(idx.sibling(idx.row() - 1, idx.column()));
	selectedHasFocus_ = true;
	updateHook();
}


void GuiSelectionManager::downPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();

	int const pos = idx.row();
	if (pos >= selectedModel->rowCount() - 1)
		return;

	QMap<int, QVariant> qm = selectedModel->itemData(idx);

	selectedModel->removeRow(pos);
	insertRowToSelected(pos + 1, qm);

	selectionChanged(); //signal
	
	selectedLV->setCurrentIndex(idx.sibling(idx.row() + 1, idx.column()));
	selectedHasFocus_ = true;
	updateHook();
}


// FIXME These slots do not really do what they need to do, since focus
// can enter the QListView in other ways. But there are no signals sent
// in that case. We need to reimplement focusInEvent() to capture those,
// which means subclassing QListView. (rgh)
// Or by installing an event listener.. (andre)
void GuiSelectionManager::availableLV_clicked(const QModelIndex &)
{
	selectedHasFocus_ = false;
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


void GuiSelectionManager::selectedLV_clicked(const QModelIndex &)
{
	selectedHasFocus_ = true;
	updateHook();
}


bool GuiSelectionManager::eventFilter(QObject * obj, QEvent * event) 
{
	if (obj == availableLV) {
		if (event->type() != QEvent::KeyPress)
			return QObject::eventFilter(obj, event);
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		int const keyPressed = keyEvent->key();
		Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();
		// Enter key without modifier will add current item.
		// Ctrl-Enter will add it and close the dialog.
		// This is designed to work both with the main enter key
		// and the one on the numeric keypad.
		if (keyPressed == Qt::Key_Enter || keyPressed == Qt::Key_Return) {
			if (!keyModifiers)
				addPB_clicked();
			else if (keyModifiers == Qt::ControlModifier ||
					keyModifiers == Qt::KeypadModifier  ||
					keyModifiers == (Qt::ControlModifier | Qt::KeypadModifier)) {
				if (addPB->isEnabled()) {
					addPB_clicked();
					okHook(); //signal
				}
			}
			event->accept();
			return true;
		}
	} else if (obj == selectedLV) {
		if (event->type() != QEvent::KeyPress)
			return QObject::eventFilter(obj, event);
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
	}
	return QObject::eventFilter(obj, event);
}

} // namespace frontend
} // namespace lyx

#include "GuiSelectionManager_moc.cpp"
