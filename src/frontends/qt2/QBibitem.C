/**
 * \file QBibitem.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/lstrings.h"

#include "QBibitemDialog.h"
#include "ControlBibitem.h"
#include "QBibitem.h"
#include "Qt2BC.h"
#include "gettext.h"
#include "debug.h" 

#include <qlineedit.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlBibitem, Qt2DB<QBibitemDialog> > base_class;

QBibitem::QBibitem(ControlBibitem & c)
	: base_class(c, _("Bibliography Item"))
{
}


void QBibitem::build_dialog()
{
	dialog_.reset(new QBibitemDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->keyED);
	bc().addReadOnly(dialog_->labelED);
}


void QBibitem::update_contents()
{
	dialog_->keyED->setText(controller().params().getContents().c_str());
	dialog_->labelED->setText(controller().params().getOptions().c_str());
}


void QBibitem::apply()
{
	controller().params().setContents(dialog_->keyED->text().latin1()); 
	controller().params().setOptions(dialog_->labelED->text().latin1()); 
}


bool QBibitem::isValid()
{
	return !string(dialog_->keyED->text().latin1()).empty();
}
