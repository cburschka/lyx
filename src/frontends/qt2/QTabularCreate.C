/**
 * \file QTabularCreate.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QTabularCreate.h"
#include "Qt2BC.h"
#include "gettext.h"
#include "debug.h"

#include <qspinbox.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlTabularCreate, Qt2DB<QTabularCreateDialog> > base_class;

QTabularCreate::QTabularCreate()
	: base_class(_("Insert table"))
{
}


void QTabularCreate::build_dialog()
{
	dialog_.reset(new QTabularCreateDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
}


void QTabularCreate::apply()
{
	controller().params().first = dialog_->rowsSB->value();
	controller().params().second = dialog_->columnsSB->value();
}
