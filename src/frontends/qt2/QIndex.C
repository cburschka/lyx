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
#include "gettext.h"

#include "QIndexDialog.h"
#include "QIndex.h"
#include "Qt2BC.h"
#include <qlineedit.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlIndex, Qt2DB<QIndexDialog> > base_class;


QIndex::QIndex()
	: base_class(_("Index"))
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
	dialog_->keywordED->setText(controller().params().getContents().c_str());
}


void QIndex::apply()
{
	controller().params().setContents(dialog_->keywordED->text().latin1());
}


bool QIndex::isValid()
{
	return !string(dialog_->keywordED->text()).empty();
}
