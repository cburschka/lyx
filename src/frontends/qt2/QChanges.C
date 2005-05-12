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

#include "controllers/ControlChanges.h"

#include <qpushbutton.h>
#include <qtextview.h>


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


void QChanges::next()
{
	controller().find();

	string text;
	string author(controller().getChangeAuthor());
	string date(controller().getChangeDate());

	if (!author.empty())
		text += "Change by " + author + "\n\n";
	if (!date.empty())
		text += "Change made at " + date + "\n";

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
