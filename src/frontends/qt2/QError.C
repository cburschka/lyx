/**
 * \file QError.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "ControlError.h"
#include "gettext.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "QErrorDialog.h"
#include "QError.h"
#include "Qt2BC.h"
 
typedef Qt2CB<ControlError, Qt2DB<QErrorDialog> > base_class;

QError::QError(ControlError & c, Dialogs &)
	: base_class(c, _("LaTeX Error"))
{
}


void QError::build_dialog()
{
	dialog_.reset(new QErrorDialog(this));

	bc().setCancel(dialog_->closePB);
}


void QError::update_contents()
{
	dialog_->errorTV->setText(controller().params().c_str());
}
