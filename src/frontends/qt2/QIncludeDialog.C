/**
 * \file QIncludeDialog.C
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

#include <vector>

#include "ControlInclude.h"
#include "debug.h"
#include "LString.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include "QIncludeDialog.h"
#include "QInclude.h"

QIncludeDialog::QIncludeDialog(QInclude * form)
	: QIncludeDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QIncludeDialog::show()
{
	QIncludeDialogBase::show();
	filenameED->setFocus();
} 
 

void QIncludeDialog::change_adaptor()
{
	form_->changed();
}


void QIncludeDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QIncludeDialog::typeChanged(int v)
{
	switch (v) {
		case 0:
		case 1:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			break;
		default:
			visiblespaceCB->setEnabled(true);
			break;
	}
}


void QIncludeDialog::loadClicked()
{
	form_->load();
}


void QIncludeDialog::browseClicked()
{
	form_->browse();
}
