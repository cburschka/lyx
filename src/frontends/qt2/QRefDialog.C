/**
 * \file QRefDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "ControlRef.h"
#include "debug.h"

#include "QRef.h"
#include "QRefDialog.h"
 
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlistbox.h>

QRefDialog::QRefDialog(QRef * form)
	: QRefDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


void QRefDialog::changed_adaptor()
{
	form_->changed();
}


void QRefDialog::gotoClicked()
{
	form_->gotoRef();
}


void QRefDialog::refHighlighted(const QString & sel)
{
	if (form_->readOnly())
		return;

	referenceED->setText(sel);
	if (form_->at_ref_)
		form_->gotoRef();
	gotoPB->setEnabled(true);
	if (form_->typeAllowed())
		typeCO->setEnabled(true);
	if (form_->nameAllowed())
		nameED->setEnabled(true);
}


void QRefDialog::refSelected(const QString &)
{
	form_->gotoRef();
}


void QRefDialog::sortToggled(bool on)
{
	form_->sort_ = on;
	refsLB->clear();
	form_->redoRefs();
}


void QRefDialog::updateClicked()
{
	form_->updateRefs();
}


void QRefDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
