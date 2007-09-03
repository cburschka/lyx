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


/////////////////////////////////////////////////////////////////////
//
// GuiEmbeddedFilesDialog
//
/////////////////////////////////////////////////////////////////////

static QString const INVALID_COLOR = "gray";
static QString const AUTO_COLOR = "green";
static QString const EMBEDDED_COLOR = "black";
static QString const EXTERNAL_COLOR = "blue";

GuiEmbeddedFilesDialog::GuiEmbeddedFilesDialog(GuiEmbeddedFiles * form)
	: form_(form)
{
	setupUi(this);
	//
	update();
}


void GuiEmbeddedFilesDialog::on_filesLW_itemSelectionChanged()
{
	EmbeddedFiles & files = form_->embeddedFiles();

	QList<QListWidgetItem *> selection = filesLW->selectedItems();

	fullpathLE->setEnabled(selection.size() == 1);

	// try to find a common mode, otherwise return NONE.
	QList<QListWidgetItem*>::iterator it = selection.begin(); 
	QList<QListWidgetItem*>::iterator it_end = selection.end(); 
	// if the selection is not empty
	if (it != it_end) {
		int idx = filesLW->row(*it);
		fullpathLE->setText(toqstr(files[idx].absFilename()));
		// go to the first selected item
		form_->goTo(files[idx]);
	}

	EmbeddedFile::STATUS mode = EmbeddedFile::NONE;
	for (; it != it_end; ++it) {
		int idx = filesLW->row(*it);
		if (mode == EmbeddedFile::NONE) {
			mode = files[idx].status();
			continue;
		}
		if (mode != files[idx].status()) {
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
	EmbeddedFiles & files = form_->embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	form_->view(files[filesLW->row(*selection.begin())]);
}


void GuiEmbeddedFilesDialog::update()
{
	filesLW->clear();
	
	//
	EmbeddedFiles const & files = form_->embeddedFiles();
	EmbeddedFiles::EmbeddedFileList::const_iterator it = files.begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = files.end();
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
	//
	bool enabled = files.enabled();
	enableCB->setChecked(enabled);
	statusGB->setEnabled(enabled);
	fullpathLE->setEnabled(enabled);
}


void GuiEmbeddedFilesDialog::on_addPB_clicked()
{
	docstring const file = form_->browseFile();
	if (!file.empty()) {
		EmbeddedFiles & files = form_->embeddedFiles();
		files.registerFile(to_utf8(file), EmbeddedFile::EMBEDDED);
	}		
}


void GuiEmbeddedFilesDialog::on_extractPB_clicked()
{
	EmbeddedFiles const & files = form_->embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		form_->extract(files[filesLW->row(*it)]);
}


void GuiEmbeddedFilesDialog::on_enableCB_toggled(bool enable)
{
	// FIXME:
	//
	// When a embedded file is turned to disabled, it should save its
	// embedded files. Otherwise, embedded files will be lost!!!
	//
	form_->embeddedFiles().enable(enable);
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
	EmbeddedFiles & files = form_->embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it) {
		int row = filesLW->row(*it);
		// FIXME: mark buffer dirty
		if (status != files[row].status())
			files[row].setStatus(status);			
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
