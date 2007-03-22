/**
 * \file QChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QChanges.h"
#include "QChangesDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "support/lstrings.h"

#include "controllers/ControlChanges.h"

#include <QPushButton>
#include <QTextBrowser>

using lyx::support::bformat;

namespace lyx {
namespace frontend {

typedef QController<ControlChanges, QView<QChangesDialog> > changes_base_class;


QChanges::QChanges(Dialog & parent)
	: changes_base_class(parent, _("Merge Changes"))
{
}


void QChanges::build_dialog()
{
	dialog_.reset(new QChangesDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->acceptPB);
	bcview().addReadOnly(dialog_->rejectPB);
}


void QChanges::update_contents()
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


void QChanges::next()
{
	controller().next();
}


void QChanges::accept()
{
	controller().accept();
}


void QChanges::reject()
{
	controller().reject();
}

} // namespace frontend
} // namespace lyx
