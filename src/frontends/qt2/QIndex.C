/**
 * \file QIndex.C
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

#include "ControlIndex.h"
#include "qt_helpers.h"

#include "QIndexDialog.h"
#include "QIndex.h"
#include "Qt2BC.h"
#include <qlineedit.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlIndex, Qt2DB<QIndexDialog> > base_class;


QIndex::QIndex()
	: base_class(qt_("Index"))
{
}


void QIndex::build_dialog()
{
	dialog_.reset(new QIndexDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->keywordED);
}


void QIndex::update_contents()
{
	dialog_->keywordED->setText(toqstr(controller().params().getContents()));
}


void QIndex::apply()
{
	controller().params().setContents(fromqstr(dialog_->keywordED->text()));
}


bool QIndex::isValid()
{
	return !dialog_->keywordED->text().isEmpty();
}
