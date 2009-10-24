/**
 * \file GuiCompare.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCompare.h"

#include "Buffer.h"
#include "BufferView.h"
#include "BufferList.h"
#include "buffer_funcs.h"
#include "FuncRequest.h"
#include "GuiView.h"
#include "LyXRC.h"
#include "qt_helpers.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/gettext.h"

#include <QThread>


using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiCompare::GuiCompare(GuiView & lv)
	: GuiDialog(lv, "compare", qt_("Compare LyX files")),
	dest_buffer_(0)
{
	setupUi(this);
	setModal(Qt::WindowModal);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotCancel()));

	connect(newFilePB, SIGNAL(clicked()), this, SLOT(select_newfile()));
	connect(oldFilePB, SIGNAL(clicked()), this, SLOT(select_oldfile()));

	connect(newFileCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(newFileCB, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(oldFileCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(oldFileCB, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(change_adaptor()));

	newSettingsRB->setChecked(true);

	progressBar->setValue(0);
	progressBar->setEnabled(false);

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(okPB);
}

GuiCompare::~GuiCompare()
{
}

void GuiCompare::closeEvent(QCloseEvent *)
{
	slotCancel();	
}


void GuiCompare::change_adaptor()
{
	changed();
}


bool GuiCompare::isValid()
{
	bool const valid = !newFileCB->currentText().isEmpty()
		&& !oldFileCB->currentText().isEmpty();
	return valid;
}


void GuiCompare::updateContents()
{
	QString restore_filename1 = newFileCB->currentText();
	QString restore_filename2 = oldFileCB->currentText();
	newFileCB->clear();
	oldFileCB->clear();
	progressBar->setValue(0);
	BufferList::iterator it = theBufferList().begin();
	BufferList::iterator const end = theBufferList().end();
	for (; it != end; ++it) {
		QString filename = toqstr((*it)->absFileName());
		newFileCB->addItem(filename);
		oldFileCB->addItem(filename);
	}
	if (lyxview().documentBufferView())
		newFileCB->setEditText(toqstr(buffer().absFileName()));
	else
		newFileCB->setEditText(restore_filename1);

	if (!restore_filename2.isEmpty())
		oldFileCB->setEditText(restore_filename2);
	else
		oldFileCB->clearEditText();

	if (isValid()) {
		bc().setValid(isValid());
		bc().apply();
	}
}


void GuiCompare::select_newfile()
{
	QString name = browse(newFileCB->currentText());
	if (!name.isEmpty())
		newFileCB->setEditText(name);
	changed();
}


void GuiCompare::select_oldfile()
{
	QString name = browse(oldFileCB->currentText());
	if (!name.isEmpty())
		oldFileCB->setEditText(name);
	changed();
}


QString GuiCompare::browse(QString const & in_name) const
{
	QString const title = qt_("Select document");

	QStringList const & filters = fileFilters(qt_("LyX Documents (*.lyx)"));
	
	QString filename;
	if (lyxview().documentBufferView()) {
		QString path = bufferFilepath();
		filename = browseRelFile(in_name, path, title, filters, false, 
			qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
	} else {
		QString path = toqstr(lyxrc.document_path);
		QString rel_filename = browseRelFile(in_name, path, title, filters, false, 
			qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		filename = makeAbsPath(rel_filename, path);
	}
	return filename;	
}


void GuiCompare::enableControls(bool enable) const
{
	newFileLA->setEnabled(enable);
	newFilePB->setEnabled(enable);
	newFileCB->setEnabled(enable);
	oldFileLA->setEnabled(enable);
	oldFilePB->setEnabled(enable);
	oldFileCB->setEnabled(enable);
	okPB->setEnabled(enable);
	groupBox->setEnabled(enable);
	progressBar->setEnabled(!enable);

	if (enable)
		closePB->setText(qt_("Close"));
	else
		closePB->setText(qt_("Cancel"));
}


void GuiCompare::finished(bool aborted)
{
	enableControls(true);
	
	if (aborted) {
		dest_buffer_->markClean();
		theBufferList().release(dest_buffer_);
		setWindowTitle(window_title_);
		progressBar->setValue(0);
	} else {
		hideView();
		bc().ok();
		dispatch(FuncRequest(LFUN_BUFFER_SWITCH, dest_buffer_->absFileName()));
	}
}


void GuiCompare::nextIt(int val)
{
	progressBar->setValue(progressBar->value() + val);
}


void GuiCompare::progress_max(int max) const
{
	progressBar->setMaximum(max);
}
	

void GuiCompare::slotOK()
{
	enableControls(false);
	if (!run()) {
		Alert::error(_("Error"),
			_("Unable to compare files."));
		finished(true);
	}
}


void GuiCompare::slotCancel()
{
	GuiDialog::slotClose();
	progressBar->setValue(0);
}


Buffer const * GuiCompare::bufferFromFileName(string const & file) const
{
	FileName fname;
	if (FileName::isAbsolute(file))
		fname.set(file);
	else if (lyxview().documentBufferView())
		fname = support::makeAbsPath(file, fromqstr(bufferFilepath()));

	if (fname.empty()
			|| (!fname.exists() && !theBufferList().getBuffer(fname))) {
		LYXERR0( "Unable to read: " << file);
		return 0;
	}
	return loadIfNeeded(fname);
}


int GuiCompare::run()
{
	progressBar->setValue(0);

	new_buffer_ = bufferFromFileName(fromqstr(newFileCB->currentText()));
	old_buffer_ = bufferFromFileName(fromqstr(oldFileCB->currentText()));

	// new buffer that will carry the output
	FileName initpath(lyxrc.document_path);
	dest_buffer_ = newUnnamedFile(initpath, to_utf8(_("differences")));
	dest_buffer_->changed();
	dest_buffer_->markDirty();

	return 0;
}


Dialog * createGuiCompare(GuiView & lv) { return new GuiCompare(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiCompare.cpp"
