/**
 * \file QSendtoDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <vector>
#include "ControlSendto.h"
#include "gettext.h"
#include "debug.h"

#include "converter.h"
#include "LString.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include "QSendtoDialog.h"
#include "QSendto.h"


QSendtoDialog::QSendtoDialog(QSendto * form)
	: QSendtoDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QSendtoDialog::changed_adaptor()
{
	form_->changed();
}


void QSendtoDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
