/**
 * \file QWrapDialog.C
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

#include "ControlWrap.h"

#include "QWrap.h"
#include "QWrapDialog.h"

#include <qpushbutton.h>
#include <qtextview.h>
#include "lengthcombo.h"

QWrapDialog::QWrapDialog(QWrap * form)
	: QWrapDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QWrapDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QWrapDialog::change_adaptor()
{
	form_->changed();
}
