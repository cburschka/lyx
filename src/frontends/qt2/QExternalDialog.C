/**
 * \file QExternalDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "qt_helpers.h"
#include "ControlExternal.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qtextview.h>
#include <qlineedit.h>
#include <qvalidator.h>

#include "QExternalDialog.h"
#include "QExternal.h"


QExternalDialog::QExternalDialog(QExternal * form)
	: QExternalDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	QIntValidator * validator = new QIntValidator(displayscale);
	validator->setBottom(1);
	displayscale->setValidator(validator);
}


void QExternalDialog::show()
{
	QExternalDialogBase::show();
	fileED->setFocus();
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


void QExternalDialog::browseClicked()
{
	QString file =
		QFileDialog::getOpenFileName(QString::null,
					     qt_("External material (*)"),
					     this, 0,
					     qt_("Select external material"));
	if (!file.isNull()) {
		fileED->setText(file);
		form_->changed();
	}
}


void QExternalDialog::templateChanged()
{
	externalTV->setText(toqstr(form_->helpText()));
	form_->changed();
}
