/**
 * \file QMinipageDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector>

#include "ControlMinipage.h"

#include "QMinipage.h"
#include "QMinipageDialog.h"

#include <qpushbutton.h>
#include <qtextview.h>
#include "lengthcombo.h"

QMinipageDialog::QMinipageDialog(QMinipage * form)
	: QMinipageDialogBase(0, 0, false, 0),
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


void QMinipageDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QMinipageDialog::change_adaptor()
{
	form_->changed();
}
