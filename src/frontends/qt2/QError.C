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


#include "ControlError.h"
#include "qt_helpers.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "QErrorDialog.h"
#include "QError.h"
#include "Qt2BC.h"

typedef QController<ControlError, QView<QErrorDialog> > base_class;


QError::QError(Dialog & parent)
	: base_class(parent, qt_("LyX: LaTeX Error"))
{
}


void QError::build_dialog()
{
	dialog_.reset(new QErrorDialog(this));

	bcview().setCancel(dialog_->closePB);
}


void QError::update_contents()
{
	dialog_->errorTV->setText(toqstr(controller().params()));
}
