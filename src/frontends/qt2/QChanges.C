/**
 * \file QChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlChanges.h"
#include "qt_helpers.h"

#include <qpushbutton.h>
#include <qtextview.h>

#include "QChangesDialog.h"
#include "QChanges.h"
#include "Qt2BC.h"

typedef Qt2CB<ControlChanges, Qt2DB<QChangesDialog> > base_class;


QChanges::QChanges()
	: base_class(qt_("Changes"))
{
}


void QChanges::build_dialog()
{
	dialog_.reset(new QChangesDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->acceptPB);
	bc().addReadOnly(dialog_->rejectPB);
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
