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
#include "Compare.h"
#include "FuncRequest.h"
#include "GuiView.h"
#include "LyXRC.h"
#include "qt_helpers.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/gettext.h"

#include <QDialogButtonBox>
#include <QThread>


using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiCompare::GuiCompare(GuiView & lv)
	: GuiDialog(lv, "compare", qt_("Compare LyX files")),
	compare_(0), dest_buffer_(0), old_buffer_(0), new_buffer_(0)
{
	setupUi(this);
	setModal(Qt::WindowModal);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

	connect(newFilePB, SIGNAL(clicked()), this, SLOT(selectNewFile()));
	connect(oldFilePB, SIGNAL(clicked()), this, SLOT(selectOldFile()));

	connect(newFileCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeAdaptor()));
	connect(newFileCB, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(changeAdaptor()));
	connect(oldFileCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeAdaptor()));
	connect(oldFileCB, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(changeAdaptor()));

	newSettingsRB->setChecked(true);
	trackingCB->setChecked(true);

	buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::ArrowCursor);

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
}

GuiCompare::~GuiCompare()
{
	if (compare_)
		delete compare_;
}

void GuiCompare::closeEvent(QCloseEvent *)
{
	slotCancel();
}


void GuiCompare::changeAdaptor()
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
	if (compare_ && compare_->isRunning())
		return;

	QString restore_filename1 = newFileCB->currentText();
	QString restore_filename2 = oldFileCB->currentText();
	newFileCB->clear();
	oldFileCB->clear();
	progressBar->setValue(0);
	statusBar->clearMessage();
	BufferList::iterator it = theBufferList().begin();
	BufferList::iterator const end = theBufferList().end();
	for (; it != end; ++it) {
		QString filename = toqstr((*it)->absFileName());
		newFileCB->addItem(filename);
		oldFileCB->addItem(filename);
	}
	if (!restore_filename1.isEmpty())
		newFileCB->setEditText(restore_filename1);
	else if (lyxview().documentBufferView())
		newFileCB->setEditText(toqstr(buffer().absFileName()));

	if (!restore_filename2.isEmpty())
		oldFileCB->setEditText(restore_filename2);
	else
		oldFileCB->clearEditText();

	if (isValid()) {
		bc().setValid(isValid());
		bc().apply();
	}
}


void GuiCompare::selectNewFile()
{
	QString name = browse(newFileCB->currentText());
	if (!name.isEmpty())
		newFileCB->setEditText(name);
	changed();
}


void GuiCompare::selectOldFile()
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
		QString path = bufferFilePath();
		filename = browseRelToParent(in_name, path, title, filters, false,
			qt_("D&ocuments"), toqstr(lyxrc.document_path));
	} else {
		QString path = toqstr(lyxrc.document_path);
		QString rel_filename = browseRelToParent(in_name, path, title, filters, false,
			qt_("D&ocuments"), toqstr(lyxrc.document_path));
		filename = makeAbsPath(rel_filename, path);
	}
	return filename;
}


void GuiCompare::enableControls(bool enable)
{
	// Set the hourglass cursor for the dialog, but
	// never for the cancel button.
	setCursor(enable ? Qt::ArrowCursor : Qt::WaitCursor);

	newFileLA->setEnabled(enable);
	newFilePB->setEnabled(enable);
	newFileCB->setEnabled(enable);
	oldFileLA->setEnabled(enable);
	oldFilePB->setEnabled(enable);
	oldFileCB->setEnabled(enable);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
	groupBox->setEnabled(enable);
	progressBar->setEnabled(!enable);

	if (enable)
		buttonBox->button(QDialogButtonBox::Cancel)->setText(qt_("Close"));
	else
		buttonBox->button(QDialogButtonBox::Cancel)->setText(qt_("Cancel"));
}


void GuiCompare::error()
{
	Alert::error(_("Error"), _("Error while comparing documents."));
	finished(true);
}

void GuiCompare::finished(bool aborted)
{
	enableControls(true);

	if (compare_) {
		delete compare_;
		compare_ = 0;
	}

	if (aborted) {
		if (dest_buffer_) {
			dest_buffer_->markClean();
			theBufferList().release(dest_buffer_);
		}
		progressBar->setValue(0);
		statusBar->showMessage(qt_("Aborted"), 5000);
	} else {
		hideView();
		bc().ok();
		if (dest_buffer_) {
			dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
				dest_buffer_->absFileName()));
			if (trackingCB->isChecked()) {
				dispatch(FuncRequest(LFUN_CHANGES_OUTPUT));
				dispatch(FuncRequest(LFUN_CHANGES_TRACK));
			}
		}
		statusBar->showMessage(qt_("Finished"), 5000);
	}
}


void GuiCompare::progress(int val)
{
	progressBar->setValue(progressBar->value() + val);
}


void GuiCompare::progressMax(int max) const
{
	progressBar->setMaximum(max);
}


void GuiCompare::setStatusMessage(QString const & msg)
{
	statusBar->showMessage(msg);
}


void GuiCompare::slotOK()
{
	enableControls(false);
	if (!run())
		error();
}


void GuiCompare::slotCancel()
{
	if (compare_ && compare_->isRunning()) {
		statusBar->showMessage(qt_("Aborting process..."));
		compare_->abort();
	} else {
		GuiDialog::slotClose();
		progressBar->setValue(0);
		statusBar->clearMessage();
	}
}


void GuiCompare::slotButtonBox(QAbstractButton * button)
{
	switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Ok:
		slotOK();
		break;
	case QDialogButtonBox::Cancel:
		slotCancel();
		break;
	default:
		break;
	}
}


Buffer const * GuiCompare::bufferFromFileName(string const & file) const
{
	FileName fname;
	if (FileName::isAbsolute(file))
		fname.set(file);
	else if (lyxview().documentBufferView())
		fname = support::makeAbsPath(file, fromqstr(bufferFilePath()));

	if (fname.empty()
			|| (!fname.exists() && !theBufferList().getBuffer(fname))) {
		LYXERR0( "Unable to read: " << file);
		return 0;
	}
	return loadIfNeeded(fname);
}


int GuiCompare::run(bool blocking_mode)
{
	progressBar->setValue(0);

	new_buffer_ = bufferFromFileName(fromqstr(newFileCB->currentText()));
	old_buffer_ = bufferFromFileName(fromqstr(oldFileCB->currentText()));

	// new buffer that will carry the output
	FileName initpath(lyxrc.document_path);
	dest_buffer_ = newUnnamedFile(initpath, to_utf8(_("differences")));

	if (!new_buffer_ || !old_buffer_ || !dest_buffer_)
		return 0;

	dest_buffer_->changed(true);
	if (blocking_mode)
		//blocking mode is infinitive and we don't want diff autosave
		//if user decides to kill ther running lyx instance
		dest_buffer_->markClean();
	else
		dest_buffer_->markDirty();


	// get the options from the dialog
	CompareOptions options;
	options.settings_from_new = newSettingsRB->isChecked();

	// init the compare object and start it

	compare_ = new Compare(new_buffer_, old_buffer_, dest_buffer_, options);

	connect(compare_, SIGNAL(error()), this, SLOT(error()));
	// Only connect the finished() method to the signal if we're *not* in blocking_mode
	//  (i.e. we want to make it possible for caller to block for the results)
	if (! blocking_mode) {
		connect(compare_, SIGNAL(finished(bool)), this, SLOT(finished(bool)));
	}
	connect(compare_, SIGNAL(progress(int)), this, SLOT(progress(int)));
	connect(compare_, SIGNAL(progressMax(int)), this, SLOT(progressMax(int)));
	connect(compare_, SIGNAL(statusMessage(QString)),
		this, SLOT(setStatusMessage(QString)));
	compare_->start(QThread::LowPriority);

	return 1;
}

bool GuiCompare::initialiseParams(std::string const &par)
{
	//just for the sake of parsing arguments
	FuncRequest cmd(LFUN_UNKNOWN_ACTION, par);
	if (cmd.getArg(0) == "run" || cmd.getArg(0) == "run-blocking") {
		oldFileCB->setEditText(toqstr(cmd.getArg(1)));
		newFileCB->setEditText(toqstr(cmd.getArg(2)));

		if (cmd.getArg(0) == "run" ) {
			// Run asynchronously
			slotOK();
		}
		else {
			// Run synchronously
			enableControls(false);

			if (! run(true)) {
				error();
				return false;
			}

			// Wait for the Compare function to process in a thread
			compare_->wait();

			finished(false);
			//Hiding dialog does not work as intended through finished routine, because showView
			//is triggered after initialiseParams returns true. So we return false, warning will
			//show on the terminal though.
			return false;
		}
	}

	progressBar->setValue(0);
	progressBar->setEnabled(false);
	progressBar->setMaximum(1);

	return true;
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiCompare.cpp"
