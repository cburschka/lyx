/**
 * \file QExternalDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include <qwidget.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qtextview.h>
#include <qlineedit.h>

#include "gettext.h"
#include "QExternalDialog.h"
#include "ControlExternal.h"
#include "Dialogs.h"
#include "QExternal.h"

QExternalDialog::QExternalDialog(QExternal * form)
	: QExternalDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QExternalDialog::change_adaptor()
{
	form_->changed();
}


void QExternalDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QExternalDialog::editClicked()
{
	form_->changed();
	form_->controller().editExternal();
}


void QExternalDialog::viewClicked()
{
	form_->changed();
	form_->controller().viewExternal();
}


void QExternalDialog::updateClicked()
{
	form_->changed();
	form_->controller().updateExternal();
}


void QExternalDialog::browseClicked()
{
	QString file = QFileDialog::getOpenFileName(QString::null,
		_("External material (*)"), this, 0, _("Select external material"));
	if (!file.isNull()) {
		fileED->setText(file.latin1());
		form_->changed();
	}
}


void QExternalDialog::templateChanged()
{
	externalTV->setText(form_->helpText().c_str());

	updatePB->setEnabled(!form_->controller().params().templ.automaticProduction);
	form_->changed();
}
