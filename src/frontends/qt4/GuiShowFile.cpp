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

#include "ControlShowFile.h"
#include "qt_helpers.h"

#include <QTextBrowser>
#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

GuiShowFileDialog::GuiShowFileDialog(LyXView & lv)
	: GuiDialog(lv, "file")
{
	setupUi(this);
	setViewTitle(_("Show File"));
	setController(new ControlShowFile(*this));

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


ControlShowFile & GuiShowFileDialog::controller() const
{
	return static_cast<ControlShowFile &>(GuiDialog::controller());
}


void GuiShowFileDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiShowFileDialog::update_contents()
{
	setWindowTitle(toqstr(controller().getFileName()));

	std::string contents = controller().getFileContents();
	if (contents.empty()) {
		contents = "Error -> Cannot load file!";
	}

	textTB->setPlainText(toqstr(contents));
}

} // namespace frontend
} // namespace lyx

#include "GuiShowFile_moc.cpp"
