/**
 * \file GuiShowFile.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiShowFile.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlShowFile.h"

#include <QTextBrowser>
#include <QPushButton>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiShowFileDialog
//
/////////////////////////////////////////////////////////////////////

GuiShowFileDialog::GuiShowFileDialog(GuiShowFile * form)
	: form_(form)
{
	setupUi(this);
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
}


void GuiShowFileDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// GuiShowFile
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlShowFile, GuiView<GuiShowFileDialog> >
	ShowFileBase;


GuiShowFile::GuiShowFile(Dialog & parent)
	: ShowFileBase(parent, _("Show File"))
{
}


void GuiShowFile::build_dialog()
{
	dialog_.reset(new GuiShowFileDialog(this));

	bcview().setCancel(dialog_->closePB);
}


void GuiShowFile::update_contents()
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

#include "GuiShowFile_moc.cpp"
