/**
 * \file QTabularDialog.C
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

#include "gettext.h"

#include "QTabular.h"
#include "QTabularDialog.h"

#include <qpushbutton.h>


QTabularDialog::QTabularDialog(QTabular * form)
	: QTabularDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QTabularDialog::change_adaptor()
{
	form_->changed();
}


void QTabularDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
