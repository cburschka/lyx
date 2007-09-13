/**
 * \file GuiEmbeddedFiles.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiEmbeddedFiles.h"
#include "debug.h"


namespace lyx {
namespace frontend {

GuiEmbeddedFilesDialog::GuiEmbeddedFilesDialog
		(ControlEmbeddedFiles & controller)
	: controller_(controller)
{
	setupUi(this);
	setWindowTitle("LyX: " + qt_("Embedded Files"));
	updateView();
}


void GuiEmbeddedFilesDialog::on_filesLW_itemChanged(QListWidgetItem* item)
{
	EmbeddedFiles & files = controller_.embeddedFiles();
	if (item->checkState() == Qt::Checked) {
		if (files[filesLW->row(item)].embedded())
			return;
		// this should not be needed after EmbeddedFiles are updated correctly.
		files.update();
		controller_.setEmbed(files[filesLW->row(item)], true, files.enabled());
	} else {
		if (!files[filesLW->row(item)].embedded())
			return;
		// this should not be needed after EmbeddedFiles are updated correctly.
		files.update();
		controller_.setEmbed(files[filesLW->row(item)], false, files.enabled());
	}
}


void GuiEmbeddedFilesDialog::on_filesLW_itemSelectionChanged()
{
	QList<QListWidgetItem *> selection = filesLW->selectedItems();

	if (selection.empty()) {
		fullpathLE->setEnabled(false);
		selectPB->setEnabled(false);
		unselectPB->setEnabled(false);
		extractPB->setEnabled(false);
		updatePB->setEnabled(false);
		return;
	}
	
	fullpathLE->setEnabled(selection.size() == 1);

	// try to find a common embedding status
	bool hasSelected = false;
	bool hasUnselected = false;
	QList<QListWidgetItem*>::iterator it = selection.begin(); 
	QList<QListWidgetItem*>::iterator it_end = selection.end(); 
	for (; it != it_end; ++it) {
		if ((*it)->checkState() == Qt::Checked)
			hasSelected = true;
		else
			hasUnselected = true;
	}
	selectPB->setEnabled(hasUnselected);
	unselectPB->setEnabled(hasSelected);
}


void GuiEmbeddedFilesDialog::on_filesLW_itemClicked(QListWidgetItem* item)
{
	EmbeddedFiles & files = controller_.embeddedFiles();
	int idx = filesLW->row(item);
	fullpathLE->setText(toqstr(files[idx].absFilename()));
	if (files[idx].refCount() > 1) {
		// if multiple insets are referred, click again will move
		// to another inset
		int k = item->data(Qt::UserRole).toInt();
		controller_.goTo(files[idx], k);
		k = (k + 1) % files[idx].refCount();
		item->setData(Qt::UserRole, k);
		// update label
		QString label = toqstr(files[idx].inzipName())
			+ QString(" (%1/%2)").arg(k + 1).arg(files[idx].refCount());
		item->setText(label);
	} else
		controller_.goTo(files[idx], 0);
}


void GuiEmbeddedFilesDialog::on_filesLW_itemDoubleClicked(QListWidgetItem* item)
{
	EmbeddedFiles & files = controller_.embeddedFiles();
	controller_.view(files[filesLW->row(item)]);
}


void GuiEmbeddedFilesDialog::updateView()
{
	filesLW->clear();
	EmbeddedFiles const & files = controller_.embeddedFiles();
	EmbeddedFiles::EmbeddedFileList::const_iterator it = files.begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = files.end();
	for (; it != it_end; ++it) {
		QString label = toqstr(it->inzipName());
		if (it->refCount() > 1)
			label += " (1/" + QString::number(it->refCount()) + ")";
		QListWidgetItem * item = new QListWidgetItem(label);
		Qt::ItemFlags flag = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
		if (it->valid())
			flag |= Qt::ItemIsEnabled;
		item->setFlags(flag);
		if(it->embedded())
			item->setCheckState(Qt::Checked);
		else
			item->setCheckState(Qt::Unchecked);
		// index of the currently used ParConstIterator
		item->setData(Qt::UserRole, 0);
		filesLW->addItem(item);
	}
}


void GuiEmbeddedFilesDialog::on_selectPB_clicked()
{
	EmbeddedFiles & files = controller_.embeddedFiles();
	// this should not be needed after EmbeddedFiles are updated correctly.
	files.update();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		(*it)->setCheckState(Qt::Checked);
		controller_.setEmbed(files[filesLW->row(*it)], true, files.enabled());
	}
	controller_.dispatchMessage("Embedding files");
}


void GuiEmbeddedFilesDialog::on_unselectPB_clicked()
{
	EmbeddedFiles & files = controller_.embeddedFiles();
	// this should not be needed after EmbeddedFiles are updated correctly.
	files.update();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		(*it)->setCheckState(Qt::Checked);
		controller_.setEmbed(files[filesLW->row(*it)], false, files.enabled());
	}
	controller_.dispatchMessage("Stop embedding files");
}


void GuiEmbeddedFilesDialog::on_addPB_clicked()
{
	docstring const file = controller_.browseFile();
	if (!file.empty()) {
		EmbeddedFiles & files = controller_.embeddedFiles();
		files.registerFile(to_utf8(file), true);
	}
	controller_.dispatchMessage("Add an embedded file");
}


void GuiEmbeddedFilesDialog::on_extractPB_clicked()
{
	EmbeddedFiles const & files = controller_.embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		controller_.extract(files[filesLW->row(*it)]);
	// FIXME: collect extraction status and display a dialog
	controller_.dispatchMessage("Extract embedded files");
}


void GuiEmbeddedFilesDialog::on_updatePB_clicked()
{
	EmbeddedFiles const & files = controller_.embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		controller_.update(files[filesLW->row(*it)]);
	// FIXME: collect update status and display a dialog
	controller_.dispatchMessage("Update embedded files from external file");
}



void GuiEmbeddedFilesDialog::on_enableCB_toggled(bool enable)
{
	//
	controller_.embeddedFiles().enable(enable);
	// immediately post the change to buffer (and bufferView)
	if (enable)
		controller_.dispatchMessage("Enable file embedding");
	else
		controller_.dispatchMessage("Disable file embedding");
}


} // namespace frontend
} // namespace lyx

#include "GuiEmbeddedFiles_moc.cpp"
