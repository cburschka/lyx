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

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiShowFile::GuiShowFile(GuiView & lv)
	: GuiDialog(lv, "file", qt_("Show File"))
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


void GuiShowFile::updateContents()
{
	setWindowTitle(onlyFilename(toqstr(filename_.absFilename())));

	QString contents = toqstr(filename_.fileContents("UTF-8"));
	if (contents.isEmpty())
		contents = qt_("Error -> Cannot load file!");

	textTB->setPlainText(contents);
}


bool GuiShowFile::initialiseParams(string const & data)
{
	filename_ = FileName(data);
	return true;
}


void GuiShowFile::clearParams()
{
	filename_.erase();
}


Dialog * createGuiShowFile(GuiView & lv) { return new GuiShowFile(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiShowFile.cpp"
