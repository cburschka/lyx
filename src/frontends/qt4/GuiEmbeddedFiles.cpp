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
#include "Qt2BC.h"

#include "debug.h"

using std::string;

namespace lyx {
namespace frontend {


/////////////////////////////////////////////////////////////////////
//
// GuiEmbeddedFilesDialog
//
/////////////////////////////////////////////////////////////////////
#define INVALID_COLOR "gray"
#define AUTO_COLOR "green"
#define EMBEDDED_COLOR "black"
#define EXTERNAL_COLOR "blue"

GuiEmbeddedFilesDialog::GuiEmbeddedFilesDialog(GuiEmbeddedFiles * form)
	: form_(form)
{
	setupUi(this);
	
	form_->updateEmbeddedFiles();
	
	EmbeddedFiles const * files = form_->embeddedFiles();
	enableCB->setChecked(files->enabled());
	EmbeddedFiles::EmbeddedFileList::const_iterator it = files->begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = files->end();
	for (; it != it_end; ++it) {
		QListWidgetItem * item = new QListWidgetItem(toqstr(it->inzipName()));
		if (!it->valid())
			item->setTextColor(INVALID_COLOR);
		else if(it->status() == EmbeddedFile::AUTO)
			item->setTextColor(AUTO_COLOR);
		else if(it->status() == EmbeddedFile::EMBEDDED)
			item->setTextColor(EMBEDDED_COLOR);
		else
			item->setTextColor(EXTERNAL_COLOR);
		filesLW->addItem(item);
	}
	filesLW->setCurrentRow(0);
	//
	actionCB->addItem("No action");
	actionCB->addItem("Add file");
	actionCB->addItem("Extract file");
	actionCB->addItem("Extract all");
	actionCB->addItem("Embed all");
	actionCB->addItem("Embed layout file");
	actionCB->addItem("View file");
	actionCB->addItem("Edit file");
	update();
}


void GuiEmbeddedFilesDialog::on_filesLW_itemSelectionChanged()
{
	EmbeddedFiles * files = form_->embeddedFiles();

	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	fullpathLE->setEnabled(selection.size() == 1);
		
	EmbeddedFile::STATUS mode = EmbeddedFile::NONE;
	// try to find a common mode, otherwise return NONE.
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		if (selection.size() == 1)
			fullpathLE->setText(toqstr(files->filename(filesLW->row(*it))));
		if (mode == EmbeddedFile::NONE) {
			mode = files->status(filesLW->row(*it));
			continue;
		}
		if (mode != files->status(filesLW->row(*it))) {
			mode = EmbeddedFile::NONE;
			break;
		}
	}
			
	autoRB->setChecked(mode == EmbeddedFile::AUTO);
	embeddedRB->setChecked(mode == EmbeddedFile::EMBEDDED);
	externalRB->setChecked(mode == EmbeddedFile::EXTERNAL);
}


void GuiEmbeddedFilesDialog::on_filesLW_itemDoubleClicked()
{
	// FIXME: view or edit file
}


void GuiEmbeddedFilesDialog::update()
{
	EmbeddedFiles const * files = form_->embeddedFiles();

	bool enabled = files->enabled();
	enableCB->setChecked(enabled);
	statusGB->setEnabled(enabled);
	filesLW->setEnabled(enabled);
	fullpathLE->setEnabled(enabled);
	actionCB->setEnabled(enabled);
	actionPB->setEnabled(enabled);
}


void GuiEmbeddedFilesDialog::on_actionPB_clicked()
{
	// FIXME.
	
	// ACTION
	string action = fromqstr(actionCB->currentText());
	if (action == "Add file") {
	} else if (action == "Extract file") {
	} else if (action == "Extract all") {
	} else if (action == "Embed all") {
	} else if (action == "Embed layout file") {
	} else if (action == "View file") {
	} else if (action == "Edit file") {
	} else {
	}	
}


void GuiEmbeddedFilesDialog::on_actionCB_stateChanged(int idx)
{
	// valid action, enable action button
	actionPB->setEnabled(idx != 0);
}


void GuiEmbeddedFilesDialog::on_enableCB_toggled(bool enable)
{
	// FIXME:
	//
	// When a embedded file is turned to disabled, it should save its
	// embedded files. Otherwise, embedded files will be lost!!!
	//
	form_->embeddedFiles()->enable(enable);
	update();
	// immediately post the change to buffer (and bufferView)
	if (enable)
		form_->setMessage("Enable file embedding");
	else
		form_->setMessage("Disable file embedding");
	// update bufferView
	form_->dispatchParams();
}


void GuiEmbeddedFilesDialog::set_embedding_status(EmbeddedFile::STATUS status)
{
	EmbeddedFiles * files = form_->embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		files->setStatus(filesLW->row(*it), status);	
		if(status == EmbeddedFile::AUTO)
			(*it)->setTextColor(AUTO_COLOR);
		else if(status == EmbeddedFile::EMBEDDED)
			(*it)->setTextColor(EMBEDDED_COLOR);
		else
			(*it)->setTextColor(EXTERNAL_COLOR);
	}
	if (status == EmbeddedFile::AUTO)
		form_->setMessage("Switch to auto embedding");
	else if (status == EmbeddedFile::EMBEDDED)
		form_->setMessage("Switch to always embedding");
	else
		form_->setMessage("Switch to never embedding");
	autoRB->setChecked(status == EmbeddedFile::AUTO);
	embeddedRB->setChecked(status == EmbeddedFile::EMBEDDED);
	externalRB->setChecked(status == EmbeddedFile::EXTERNAL);
	// update bufferView
	form_->dispatchParams();
}


void GuiEmbeddedFilesDialog::on_autoRB_clicked()
{
	set_embedding_status(EmbeddedFile::AUTO);
}


void GuiEmbeddedFilesDialog::on_embeddedRB_clicked()
{
	set_embedding_status(EmbeddedFile::EMBEDDED);
}


void GuiEmbeddedFilesDialog::on_externalRB_clicked()
{
	set_embedding_status(EmbeddedFile::EXTERNAL);
}


} // namespace frontend
} // namespace lyx

#include "GuiEmbeddedFiles_moc.cpp"
