/**
 * \file QError.C
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

#include "ControlError.h"
#include "gettext.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "QErrorDialog.h"
#include "QError.h"
#include "Qt2BC.h"

typedef Qt2CB<ControlError, Qt2DB<QErrorDialog> > base_class;


QError::QError()
	: base_class(_("LaTeX Error"))
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
