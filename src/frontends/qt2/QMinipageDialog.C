/**
 * \file QMinipageDialog.C
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

#include "ControlMinipage.h"

#include "QMinipage.h"
#include "QMinipageDialog.h"

#include <qpushbutton.h>
#include <qtextview.h>
#include "lengthcombo.h"

#include <vector>


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
