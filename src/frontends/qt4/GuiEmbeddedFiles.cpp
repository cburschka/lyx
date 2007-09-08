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
#include "support/convert.h"

using std::string;

namespace lyx {

namespace frontend {


GuiEmbeddedFilesDialog::GuiEmbeddedFilesDialog(LyXView & lv)
	: GuiDialog(lv, "embedding")
{
	setupUi(this);
	setController(new ControlEmbeddedFiles(*this));
	setViewTitle(_("Embedded Files"));

	//setView(new DockView<GuiEmbeddedFiles, GuiEmbeddedFilesDialog>(
	//		*dialog, qef, &gui_view, _("Embedded Files"), Qt::RightDockWidgetArea));
	
	bc().setPolicy(ButtonPolicy::OkCancelPolicy);

	updateView();
}


ControlEmbeddedFiles & GuiEmbeddedFilesDialog::controller() const
{
	return static_cast<ControlEmbeddedFiles &>(Dialog::controller());
}


void GuiEmbeddedFilesDialog::on_filesLW_itemChanged(QListWidgetItem* item)
{
	EmbeddedFiles & files = controller().embeddedFiles();
	if (item->checkState() == Qt::Checked) {
		if (files[filesLW->row(item)].embedded())
			return;
		controller().setEmbed(files[filesLW->row(item)], true);
		controller().dispatchMessage("Embed file " + fromqstr(item->text()));
	} else {
		if (!files[filesLW->row(item)].embedded())
			return;
		controller().setEmbed(files[filesLW->row(item)], false);
		controller().dispatchMessage("Stop embedding file " + fromqstr(item->text()));
	}
}


void GuiEmbeddedFilesDialog::on_filesLW_itemSelectionChanged()
{
	EmbeddedFiles & files = controller().embeddedFiles();

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
	EmbeddedFiles & files = controller().embeddedFiles();
	int idx = filesLW->row(item);
	fullpathLE->setText(toqstr(files[idx].absFilename()));
	if (files[idx].refCount() > 1) {
		// if multiple insets are referred, click again will move
		// to another inset
		int k = item->data(Qt::UserRole).toInt();
		controller().goTo(files[idx], k);
		k = (k + 1) % files[idx].refCount();
		item->setData(Qt::UserRole, k);
		// update label
		string label = files[idx].inzipName() + " ("
			+ convert<string>(k + 1)  + "/"
			+ convert<string>(files[idx].refCount()) + ")";
		item->setText(toqstr(label));
	} else
		controller().goTo(files[idx], 0);
}


void GuiEmbeddedFilesDialog::on_filesLW_itemDoubleClicked(QListWidgetItem* item)
{
	EmbeddedFiles & files = controller().embeddedFiles();
	controller().view(files[filesLW->row(item)]);
}


void GuiEmbeddedFilesDialog::updateView()
{
	filesLW->clear();
	//
	EmbeddedFiles const & files = controller().embeddedFiles();
	EmbeddedFiles::EmbeddedFileList::const_iterator it = files.begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = files.end();
	for (; it != it_end; ++it) {
		string label = it->inzipName();
		if (it->refCount() > 1)
			label += " (1/" + convert<string>(it->refCount()) + ")";
		QListWidgetItem * item = new QListWidgetItem(toqstr(label));
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
	EmbeddedFiles & files = controller().embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		(*it)->setCheckState(Qt::Checked);
		controller().setEmbed(files[filesLW->row(*it)], true);
	}
	controller().dispatchMessage("Embedding files");
}


void GuiEmbeddedFilesDialog::on_unselectPB_clicked()
{
	EmbeddedFiles & files = controller().embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		(*it)->setCheckState(Qt::Checked);
		controller().setEmbed(files[filesLW->row(*it)], false);
	}
	controller().dispatchMessage("Stop embedding files");
}


void GuiEmbeddedFilesDialog::on_addPB_clicked()
{
	docstring const file = controller().browseFile();
	if (!file.empty()) {
		EmbeddedFiles & files = controller().embeddedFiles();
		files.registerFile(to_utf8(file), true);
	}
	controller().dispatchMessage("Add an embedded file");
}


void GuiEmbeddedFilesDialog::on_extractPB_clicked()
{
	EmbeddedFiles const & files = controller().embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		controller().extract(files[filesLW->row(*it)]);
	// FIXME: collect extraction status and display a dialog
	controller().dispatchMessage("Extract embedded files");
}


void GuiEmbeddedFilesDialog::on_updatePB_clicked()
{
	EmbeddedFiles const & files = controller().embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		controller().update(files[filesLW->row(*it)]);
	// FIXME: collect update status and display a dialog
	controller().dispatchMessage("Update embedded files from external file");
}



void GuiEmbeddedFilesDialog::on_enableCB_toggled(bool enable)
{
	//
	controller().embeddedFiles().enable(enable);
	// immediately post the change to buffer (and bufferView)
	if (enable)
		controller().dispatchMessage("Enable file embedding");
	else
		controller().dispatchMessage("Disable file embedding");
}


} // namespace frontend
} // namespace lyx

#include "GuiEmbeddedFiles_moc.cpp"
