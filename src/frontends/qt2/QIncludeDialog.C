/**
 * \file QIncludeDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector>

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include "LString.h"

#include "QIncludeDialog.h"
#include "ControlInclude.h"
#include "Dialogs.h"
#include "QInclude.h"
#include "debug.h"

QIncludeDialog::QIncludeDialog(QInclude * form)
	: QIncludeDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
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
