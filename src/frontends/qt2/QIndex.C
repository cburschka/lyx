/**
 * \file QIndex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "ControlIndex.h"
#include "gettext.h"

#include "QIndexDialog.h"
#include "QIndex.h"
#include "Qt2BC.h"
#include <qlineedit.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlIndex, Qt2DB<QIndexDialog> > base_class;

QIndex::QIndex(ControlIndex & c, Dialogs &)
	: base_class(c, _("Index"))
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
