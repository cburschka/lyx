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

#include "Buffer.h"

#include "FuncRequest.h"
#include "gettext.h"
#include "debug.h"
#include "Format.h"
#include "LyXRC.h"

#include "frontend_helpers.h"
#include "frontends/LyXView.h"

#include "support/FileFilterList.h"
#include "support/convert.h"
#include "support/FileName.h"
#include "support/filetools.h"

using std::string;


namespace lyx {
namespace frontend {

using support::FileFilterList;
using support::FileName;
using support::libFileSearch;

EmbeddedFilesWidget::EmbeddedFilesWidget
		(GuiEmbeddedFiles & controller)
	: controller_(controller)
{
	setupUi(this);
	setWindowTitle("LyX: " + qt_("Embedded Files"));
	// Temporary icons.
	FileName icon_path = libFileSearch("images", "tabular-feature_set-all-lines.png");
	selectPB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path =  libFileSearch("images", "tabular-feature_unset-all-lines.png");
	unselectPB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path =  libFileSearch("images", "file-open.png");
	addPB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path =  libFileSearch("images", "depth-decrement.png");
	extractPB->setIcon(QIcon(toqstr(icon_path.absFilename())));
	icon_path =  libFileSearch("images", "depth-increment.png");
	updatePB->setIcon(QIcon(toqstr(icon_path.absFilename())));

	updateView();
}


void EmbeddedFilesWidget::on_filesLW_itemChanged(QListWidgetItem* item)
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


void EmbeddedFilesWidget::on_filesLW_itemSelectionChanged()
{
	if (controller_.isReadonly())
		return;

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


void EmbeddedFilesWidget::on_filesLW_itemClicked(QListWidgetItem* item)
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
	} else if (files[idx].refCount() == 1)
		controller_.goTo(files[idx], 0);
}


void EmbeddedFilesWidget::on_filesLW_itemDoubleClicked(QListWidgetItem* item)
{
	EmbeddedFiles & files = controller_.embeddedFiles();
	controller_.view(files[filesLW->row(item)]);
}


void EmbeddedFilesWidget::updateView()
{
	bool readOnly = controller_.isReadonly();
	fullpathLE->setEnabled(!readOnly);
	selectPB->setEnabled(!readOnly);
	unselectPB->setEnabled(!readOnly);
	addPB->setEnabled(!readOnly);
	extractPB->setEnabled(!readOnly);
	updatePB->setEnabled(!readOnly);
	enableCB->setEnabled(!readOnly);
	
	filesLW->clear();
	EmbeddedFiles const & files = controller_.embeddedFiles();
	enableCB->setCheckState(files.enabled() ? Qt::Checked : Qt::Unchecked);
	EmbeddedFiles::EmbeddedFileList::const_iterator it = files.begin();
	EmbeddedFiles::EmbeddedFileList::const_iterator it_end = files.end();
	for (; it != it_end; ++it) {
		QString label = toqstr(it->inzipName());
		if (it->refCount() > 1)
			label += " (1/" + QString::number(it->refCount()) + ")";
		QListWidgetItem * item = new QListWidgetItem(label);
		Qt::ItemFlags flag = Qt::ItemIsSelectable;
		if (!readOnly)
			flag |= Qt::ItemIsUserCheckable;
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


void EmbeddedFilesWidget::on_selectPB_clicked()
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


void EmbeddedFilesWidget::on_unselectPB_clicked()
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


void EmbeddedFilesWidget::on_addPB_clicked()
{
	if (controller_.browseAndAddFile())
		updateView();
}


void EmbeddedFilesWidget::on_extractPB_clicked()
{
	EmbeddedFiles const & files = controller_.embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		controller_.extract(files[filesLW->row(*it)]);
	// FIXME: collect extraction status and display a dialog
	controller_.dispatchMessage("Extract embedded files");
}


void EmbeddedFilesWidget::on_updatePB_clicked()
{
	EmbeddedFiles const & files = controller_.embeddedFiles();
	QList<QListWidgetItem *> selection = filesLW->selectedItems();
	for (QList<QListWidgetItem*>::iterator it = selection.begin(); 
		it != selection.end(); ++it)
		controller_.update(files[filesLW->row(*it)]);
	// FIXME: collect update status and display a dialog
	controller_.dispatchMessage("Update embedded files from external file");
}



void EmbeddedFilesWidget::on_enableCB_toggled(bool enable)
{
	controller_.setEmbedding(enable);
}



GuiEmbeddedFiles::GuiEmbeddedFiles(GuiViewBase & parent, Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "embedded", area, flags)
{
	widget_ = new EmbeddedFilesWidget(*this);
	setWidget(widget_);
	setWindowTitle(widget_->windowTitle());
}


void GuiEmbeddedFiles::updateView()
{
	widget_->updateView();
}


EmbeddedFiles & GuiEmbeddedFiles::embeddedFiles()
{
	return buffer().embeddedFiles();
}


bool GuiEmbeddedFiles::initialiseParams(string const &)
{
	return true;
}


void GuiEmbeddedFiles::updateEmbeddedFiles()
{
	// copy buffer embeddedFiles to a local copy
	buffer().embeddedFiles().update();
	buffer().embeddingChanged();
}


void GuiEmbeddedFiles::dispatchMessage(string const & msg)
{
	// FIXME: the right thing to do? QT guys?
	// lyx view will only be updated if we do something to the main window. :-)
	dispatch(FuncRequest(LFUN_MESSAGE, msg));
}


bool GuiEmbeddedFiles::isReadonly()
{
	return buffer().isReadonly();
}


void GuiEmbeddedFiles::setEmbedding(bool enable)
{
	if (embeddedFiles().enabled() == enable)
		return;
	embeddedFiles().enable(enable);
	buffer().markDirty();
	if (enable)
		dispatchMessage("Stop saving in bundled format.");
	else
		dispatchMessage("Save in bundled format.");
}


void GuiEmbeddedFiles::goTo(EmbeddedFile const & item, int idx)
{
	BOOST_ASSERT(idx < item.refCount());
	item.saveBookmark(&buffer(), idx);
	lyxview().dispatch(FuncRequest(LFUN_BOOKMARK_GOTO, "0"));
}


void GuiEmbeddedFiles::view(EmbeddedFile const & item)
{
	formats.view(buffer(), item, formats.getFormatFromFile(item));
}


void GuiEmbeddedFiles::setEmbed(EmbeddedFile & item, bool embed, bool update)
{
	if (item.embedded() == embed)
		return;
	item.setEmbed(embed);
	if (update) {
		if (embed)
			item.updateFromExternalFile(&buffer());
		else
			item.extract(&buffer());
		item.updateInsets(&buffer());
		// FIXME: unless we record the type of file item, we will
		// need to update all possible dialogs (bibtex etc).
		updateDialog("graphics");
	}
	if (embed)
		dispatchMessage("Embed file " + item.outputFilename(buffer().filePath()));
	else
		dispatchMessage("Stop embedding file " + item.outputFilename(buffer().filePath()));
	buffer().markDirty();
}


bool GuiEmbeddedFiles::browseAndAddFile()
{
	std::pair<docstring, docstring> dir1(_("Documents|#o#O"),
				  from_utf8(lyxrc.document_path));
	FileFilterList const filter(_("All file (*.*)"));
	docstring const file = browseRelFile(docstring(), from_utf8(bufferFilepath()),
			     _("Select a file to embed"),
			     filter, false, dir1);
	if (!file.empty()) {
		EmbeddedFile & ef = embeddedFiles().registerFile(to_utf8(file), true);
		if (embeddedFiles().enabled())
			ef.updateFromExternalFile(&buffer());
		buffer().markDirty();
		dispatchMessage("Add an embedded file" + to_utf8(file));
		return true;
	}
	return false;
}


bool GuiEmbeddedFiles::extract(EmbeddedFile const & item)
{
	if (item.embedded())
		return item.extract(&buffer());
	else
		return false;
}


bool GuiEmbeddedFiles::update(EmbeddedFile const & item)
{
	if (item.embedded())
		return item.updateFromExternalFile(&buffer());
	else
		return false;
}


Dialog * createGuiEmbeddedFiles(LyXView & lv)
{
	GuiViewBase & guiview = static_cast<GuiViewBase &>(lv);
#ifdef Q_WS_MACX
	// On Mac show as a drawer at the right
	return new GuiEmbeddedFiles(guiview, Qt::RightDockWidgetArea, Qt::Drawer);
#else
	return new GuiEmbeddedFiles(guiview, Qt::RightDockWidgetArea);
#endif
}


} // namespace frontend
} // namespace lyx

#include "GuiEmbeddedFiles_moc.cpp"
