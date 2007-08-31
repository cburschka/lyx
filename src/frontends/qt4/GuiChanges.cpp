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
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "support/lstrings.h"

#include "controllers/ControlChanges.h"

#include <QPushButton>
#include <QCloseEvent>
#include <QTextBrowser>

using lyx::support::bformat;


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiChangesDialog
//
/////////////////////////////////////////////////////////////////////

GuiChangesDialog::GuiChangesDialog(GuiChanges * form)
	: form_(form)
{
	setupUi(this);
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(nextPB, SIGNAL(clicked()), this, SLOT(nextPressed()));
	connect(rejectPB, SIGNAL(clicked()), this, SLOT(rejectPressed()));
	connect(acceptPB, SIGNAL(clicked()), this, SLOT(acceptPressed()));
}


void GuiChangesDialog::nextPressed()
{
	form_->next();
}


void GuiChangesDialog::acceptPressed()
{
	form_->accept();
}


void GuiChangesDialog::rejectPressed()
{
	form_->reject();
}


void GuiChangesDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}



/////////////////////////////////////////////////////////////////////
//
// GuiChanges
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlChanges, GuiView<GuiChangesDialog> > ChangesBase;


GuiChanges::GuiChanges(Dialog & parent)
	: ChangesBase(parent, _("Merge Changes"))
{
}


void GuiChanges::build_dialog()
{
	dialog_.reset(new GuiChangesDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->acceptPB);
	bcview().addReadOnly(dialog_->rejectPB);
}


void GuiChanges::update_contents()
{
	docstring text;
	docstring author = controller().getChangeAuthor();
	docstring date = controller().getChangeDate();

	if (!author.empty())
		text += bformat(_("Change by %1$s\n\n"), author);
	if (!date.empty())
		text += bformat(_("Change made at %1$s\n"), date);

	dialog_->changeTB->setPlainText(toqstr(text));
}


void GuiChanges::next()
{
	controller().next();
}


void GuiChanges::accept()
{
	controller().accept();
}


void GuiChanges::reject()
{
	controller().reject();
}

} // namespace frontend
} // namespace lyx

#include "GuiChanges_moc.cpp"
