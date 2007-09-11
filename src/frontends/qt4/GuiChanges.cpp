/**
 * \file GuiChanges.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiChanges.h"
#include "ControlChanges.h"

#include "qt_helpers.h"

#include "support/lstrings.h"

#include <QCloseEvent>
#include <QTextBrowser>

using lyx::support::bformat;


namespace lyx {
namespace frontend {

GuiChangesDialog::GuiChangesDialog(LyXView & lv)
	: GuiDialog(lv, "changes")
{
	setupUi(this);
	setController(new ControlChanges(*this));
	setViewTitle(_("Merge Changes"));

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(nextPB, SIGNAL(clicked()), this, SLOT(nextPressed()));
	connect(rejectPB, SIGNAL(clicked()), this, SLOT(rejectPressed()));
	connect(acceptPB, SIGNAL(clicked()), this, SLOT(acceptPressed()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
	bc().addReadOnly(acceptPB);
	bc().addReadOnly(rejectPB);
}


ControlChanges & GuiChangesDialog::controller()
{
	return static_cast<ControlChanges &>(GuiDialog::controller());
}


void GuiChangesDialog::closeEvent(QCloseEvent *e)
{
	slotClose();
	e->accept();
}


void GuiChangesDialog::updateContents()
{
	docstring text;
	docstring author = controller().getChangeAuthor();
	docstring date = controller().getChangeDate();

	if (!author.empty())
		text += bformat(_("Change by %1$s\n\n"), author);
	if (!date.empty())
		text += bformat(_("Change made at %1$s\n"), date);

	changeTB->setPlainText(toqstr(text));
}


void GuiChangesDialog::nextPressed()
{
	controller().next();
}


void GuiChangesDialog::acceptPressed()
{
	controller().accept();
}


void GuiChangesDialog::rejectPressed()
{
	controller().reject();
}

} // namespace frontend
} // namespace lyx

#include "GuiChanges_moc.cpp"
