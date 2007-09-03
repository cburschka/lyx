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


namespace lyx {
namespace frontend {

GuiSelectionManager::GuiSelectionManager(
		QListView * avail, 
		QListView * sel,
		QPushButton * add, 
		QPushButton * del, 
		QPushButton * up, 
		QPushButton * down,
		QStringListModel * amod,
		QStringListModel * smod)
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
					SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
								 this, SLOT(availableChanged(const QModelIndex &, const QModelIndex &)));
	connect(selectedLV->selectionModel(),
					SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
								 this, SLOT(selectedChanged(const QModelIndex &, const QModelIndex &)));
	connect(addPB, SIGNAL(clicked()), 
					this, SLOT(addPB_clicked()));
	connect(deletePB, SIGNAL(clicked()), 
					this, SLOT(deletePB_clicked()));
	connect(upPB, SIGNAL(clicked()), 
					this, SLOT(upPB_clicked()));
	connect(downPB, SIGNAL(clicked()), 
					this, SLOT(downPB_clicked()));
	connect(availableLV, SIGNAL(clicked(const QModelIndex &)), 
					this, SLOT(availableLV_clicked(const QModelIndex &)));
	connect(availableLV, SIGNAL(doubleClicked(const QModelIndex &)), 
					this, SLOT(availableLV_doubleClicked(const QModelIndex &)));
	connect(selectedLV, SIGNAL(clicked(const QModelIndex &)), 
					this, SLOT(selectedLV_clicked(const QModelIndex &)));

	availableLV->installEventFilter(this);
	selectedLV->installEventFilter(this);
}


void GuiSelectionManager::updateView()
{
	int const arows = availableLV->model()->rowCount();
	QModelIndexList const availSels = 
			availableLV->selectionModel()->selectedIndexes();
	addPB->setEnabled(arows > 0 &&
			!availSels.isEmpty() &&
			!isSelected(availSels.first()));

	int const srows = selectedLV->model()->rowCount();
	QModelIndexList const selSels = 
			selectedLV->selectionModel()->selectedIndexes();
	int const sel_nr = 	selSels.empty() ? -1 : selSels.first().row();
	deletePB->setEnabled(sel_nr >= 0);
	upPB->setEnabled(sel_nr > 0);
	downPB->setEnabled(sel_nr >= 0 && sel_nr < srows - 1);
}


bool GuiSelectionManager::isSelected(const QModelIndex & idx)
{
	QString const str = idx.data().toString();
	return selectedModel->stringList().contains(str);
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


static QModelIndex getSelectedIndex(QListView * lv)
{
	QModelIndex retval = QModelIndex();
	QModelIndexList selIdx = 
			lv->selectionModel()->selectedIndexes();
	if (!selIdx.empty())
		retval = selIdx.first();
	return retval;
}


void GuiSelectionManager::addPB_clicked()
{
	QModelIndex const idxToAdd = getSelectedIndex(availableLV);
	if (!idxToAdd.isValid())
		return;
	QModelIndex idx = selectedLV->currentIndex();
	
	QStringList keys = selectedModel->stringList();
	keys.append(idxToAdd.data().toString());
	selectedModel->setStringList(keys);
	selectionChanged(); //signal
	
	if (idx.isValid())
		selectedLV->setCurrentIndex(idx);
	updateHook();
}


void GuiSelectionManager::deletePB_clicked()
{
	QModelIndex idx = getSelectedIndex(selectedLV);
	if (!idx.isValid())
		return;

	QStringList keys = selectedModel->stringList();
	keys.removeAt(idx.row());
	selectedModel->setStringList(keys);
	selectionChanged(); //signal

	int nrows = selectedLV->model()->rowCount();
	if (idx.row() == nrows) //was last item on list
		idx = idx.sibling(idx.row() - 1, idx.column());

	if (nrows > 1)
		selectedLV->setCurrentIndex(idx);
	else if (nrows == 1)
		selectedLV->setCurrentIndex(selectedLV->model()->index(0,0));
	selectedHasFocus_ = (nrows > 0);
	updateHook();
}


void GuiSelectionManager::upPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	
	int const pos = idx.row();
	QStringList keys = selectedModel->stringList();
	keys.swap(pos, pos - 1);
	selectedModel->setStringList(keys);
	selectionChanged(); //signal
	
	selectedLV->setCurrentIndex(idx.sibling(idx.row() - 1, idx.column()));
	selectedHasFocus_ = true;
	updateHook();
}


void GuiSelectionManager::downPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	
	int const pos = idx.row();
	QStringList keys = selectedModel->stringList();
	keys.swap(pos, pos + 1);
	selectedModel->setStringList(keys);
	selectionChanged(); //signal
	
	selectedLV->setCurrentIndex(idx.sibling(idx.row() + 1, idx.column()));
	selectedHasFocus_ = true;
	updateHook();
}


//FIXME These slots do not really do what they need to do, since focus
//can enter the QListView in other ways. But there are no signals sent
//in that case. We need to reimplement focusInEvent() to capture those,
//which means subclassing QListView. (rgh)
void GuiSelectionManager::availableLV_clicked(const QModelIndex &)
{
	selectedHasFocus_ = false;
	updateHook();
}


void GuiSelectionManager::availableLV_doubleClicked(const QModelIndex & idx)
{
	if (isSelected(idx))
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
		//Enter key without modifier will add current item.
		//Ctrl-Enter will add it and close the dialog.
		//This is designed to work both with the main enter key
		//and the one on the numeric keypad.
		if ((keyPressed == Qt::Key_Enter || keyPressed == Qt::Key_Return) &&
				//We want one or both of Control and Keypad, and nothing else
				//(KeypadModifier is what you get if you use the Enter key on the
				//numeric keypad.)
					 (!keyModifiers || 
					 (keyModifiers == Qt::ControlModifier) ||
					 (keyModifiers == Qt::KeypadModifier)  ||
					 (keyModifiers == (Qt::ControlModifier | Qt::KeypadModifier))
					 )
			 ) {
			if (addPB->isEnabled()) {
				addPB_clicked();
				okHook(); //signal
			}
			event->accept();
			return true;
			 } 
	} else if (obj == selectedLV) {
		//Delete or backspace key will delete current item
		//...with control modifier will clear the list
		if (event->type() != QEvent::KeyPress)
			return QObject::eventFilter(obj, event);
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		int const keyPressed = keyEvent->key();
		Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();
		if (keyPressed == Qt::Key_Delete || keyPressed == Qt::Key_Backspace) {
			if (keyModifiers == Qt::NoModifier && deletePB->isEnabled())
				deletePB_clicked();
			else if (keyModifiers == Qt::ControlModifier) {
				QStringList list = selectedModel->stringList();
				list.clear();
				selectedModel->setStringList(list);
				updateHook();
			} else
				//ignore it otherwise
				return QObject::eventFilter(obj, event);
				event->accept();
				return true;
		}
	}
	return QObject::eventFilter(obj, event);
}

}//namespace frontend
}//namespace lyx

#include "GuiSelectionManager_moc.cpp"
