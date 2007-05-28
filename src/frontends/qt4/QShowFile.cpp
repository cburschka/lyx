/**
 * \file QShowFile.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QShowFile.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlShowFile.h"

#include <QTextBrowser>
#include <QPushButton>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QShowFileDialog
//
/////////////////////////////////////////////////////////////////////

QShowFileDialog::QShowFileDialog(QShowFile * form)
	: form_(form)
{
	setupUi(this);
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
}


void QShowFileDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// QShowFile
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlShowFile, QView<QShowFileDialog> >
	ShowFileBase;


QShowFile::QShowFile(Dialog & parent)
	: ShowFileBase(parent, _("Show File"))
{
}


void QShowFile::build_dialog()
{
	dialog_.reset(new QShowFileDialog(this));

	bcview().setCancel(dialog_->closePB);
}


void QShowFile::update_contents()
{
	dialog_->setWindowTitle(toqstr(controller().getFileName()));

	std::string contents = controller().getFileContents();
	if (contents.empty()) {
		contents = "Error -> Cannot load file!";
	}

	dialog_->textTB->setPlainText(toqstr(contents));
}

} // namespace frontend
} // namespace lyx

#include "QShowFile_moc.cpp"
