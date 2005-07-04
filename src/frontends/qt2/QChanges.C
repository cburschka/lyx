/**
 * \file QChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
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

#include <qpushbutton.h>
#include <qtextview.h>

using lyx::support::bformat;

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlChanges, QView<QChangesDialog> > base_class;


QChanges::QChanges(Dialog & parent)
	: base_class(parent, _("Merge Changes"))
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
	next();
}


void QChanges::next()
{
	controller().find();

	string text;
	string author(controller().getChangeAuthor());
	string date(controller().getChangeDate());

	if (!author.empty())
		text += bformat(_("Change by %1$s\n\n"), author);
	if (!date.empty())
		text += bformat(_("Change made at %1$s\n"), date);

	dialog_->changeTV->setText(toqstr(text));
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
