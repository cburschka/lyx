/**
 * \file QGraphicsDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlGraphics.h"
#include "debug.h"
#include "gettext.h"
#include "LString.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

#include "lengthcombo.h"

#include "QGraphicsDialog.h"
#include "QGraphics.h"

#include <vector>


QGraphicsDialog::QGraphicsDialog(QGraphics * form)
	: QGraphicsDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
}


void QGraphicsDialog::show()
{
	QGraphicsDialogBase::show();
	filename->setFocus();
}


void QGraphicsDialog::change_adaptor()
{
	form_->changed();
}


void QGraphicsDialog::change_bb()
{
	form_->controller().bbChanged = true;
	lyxerr[Debug::GRAPHICS]
		<< "[controller().bb_Changed set to true]\n";
	form_->changed();
}


void QGraphicsDialog::change_WUnit()
{
	bool useHeight = (widthUnit->currentItem() > 0);
	height->setEnabled(useHeight);
	heightUnit->setEnabled(useHeight);
	form_->changed();
}


void QGraphicsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QGraphicsDialog::browse_clicked()
{
	QString file = QFileDialog::getOpenFileName(
		QString::null, _("Files (*)"),
		this, 0, _("Select a graphic file"));
	if (!file.isNull()) {
		string const name = file.latin1();
		filename->setText(name.c_str());
		form_->changed();
	}
}


void QGraphicsDialog::getBB_clicked()
{
	form_->getBB();
}
