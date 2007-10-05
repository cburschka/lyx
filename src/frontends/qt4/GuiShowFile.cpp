/**
 * \file GuiShowFile.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiShowFile.h"

#include "qt_helpers.h"
#include "support/filetools.h"

#include <QTextBrowser>
#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

using support::FileName;
using support::onlyFilename;

GuiShowFile::GuiShowFile(LyXView & lv)
	: GuiDialog(lv, "file"), Controller(this)
{
	setupUi(this);
	setViewTitle(_("Show File"));
	setController(this, false);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


void GuiShowFile::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiShowFile::updateContents()
{
	setWindowTitle(toqstr(onlyFilename(filename_.absFilename())));

	std::string contents = support::getFileContents(filename_);
	if (contents.empty())
		contents = "Error -> Cannot load file!";

	textTB->setPlainText(toqstr(contents));
}


bool GuiShowFile::initialiseParams(std::string const & data)
{
	filename_ = FileName(data);
	return true;
}


void GuiShowFile::clearParams()
{
	filename_.erase();
}


Dialog * createGuiShowFile(LyXView & lv) { return new GuiShowFile(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiShowFile_moc.cpp"
