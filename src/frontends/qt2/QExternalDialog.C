/**
 * \file QExternalDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include "insets/ExternalTemplate.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlExternal.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"

#include "QExternalDialog.h"

#include "lengthcombo.h"
#include "lengthvalidator.h"
#include "qt_helpers.h"
#include "QExternal.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtextview.h>
#include <qlineedit.h>
#include <qvalidator.h>

using lyx::support::float_equal;
using lyx::support::isStrDbl;
using lyx::support::strToDbl;
using std::string;

namespace lyx {
namespace frontend {

namespace {

LengthValidator * unsignedLengthValidator(QLineEdit * ed)
{
	LengthValidator * v = new LengthValidator(ed);
	v->setBottom(LyXLength());
	return v;
}

} // namespace anon


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

	QIntValidator * validator = new QIntValidator(displayscaleED);
	validator->setBottom(1);
	displayscaleED->setValidator(validator);

	angleED->setValidator(new QDoubleValidator(-360, 360, 2, angleED));

	xlED->setValidator(new QIntValidator(xlED));
	ybED->setValidator(new QIntValidator(ybED));
	xrED->setValidator(new QIntValidator(xrED));
	ytED->setValidator(new QIntValidator(ytED));

	widthED->setValidator(unsignedLengthValidator(widthED));
	heightED->setValidator(unsignedLengthValidator(heightED));
}


void QExternalDialog::show()
{
	QExternalDialogBase::show();
	fileED->setFocus();
}



bool QExternalDialog::activateAspectratio() const
{
	if (widthUnitCO->currentItem() == 0)
		return false;

	string const wstr = fromqstr(widthED->text());
	if (wstr.empty())
		return false;
	bool const wIsDbl = isStrDbl(wstr);
	if (wIsDbl && float_equal(strToDbl(wstr), 0.0, 0.05))
		return false;
	LyXLength l;
	if (!wIsDbl && (!isValidLength(wstr, &l) || l.zero()))
		return false;

	string const hstr = fromqstr(heightED->text());
	if (hstr.empty())
		return false;
	bool const hIsDbl = isStrDbl(hstr);
	if (hIsDbl && float_equal(strToDbl(hstr), 0.0, 0.05))
		return false;
	if (!hIsDbl && (!isValidLength(hstr, &l) || l.zero()))
		return false;

	return true;
}


void QExternalDialog::bbChanged()
{
	form_->controller().bbChanged(true);
	form_->changed();
}


void QExternalDialog::browseClicked()
{
	int const choice =  externalCO->currentItem();
	string const template_name =
		form_->controller().getTemplate(choice).lyxName;
	string const str =
		form_->controller().browse(fromqstr(fileED->text()),
					   template_name);
	fileED->setText(toqstr(str));
	form_->changed();
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
	form_->controller().editExternal();
}



void QExternalDialog::extraChanged(const QString& text)
{
	std::string const format = fromqstr(extraFormatCO->currentText());
	form_->extra_[format] = text;
	form_->changed();
}


void QExternalDialog::formatChanged(const QString& format)
{
	extraED->setText(form_->extra_[fromqstr(format)]);
}


void QExternalDialog::getbbClicked()
{
	form_->getBB();
}


void QExternalDialog::sizeChanged()
{
	aspectratioCB->setEnabled(activateAspectratio());
	form_->changed();
}


void QExternalDialog::templateChanged()
{
	form_->updateTemplate();
	form_->changed();
}


void QExternalDialog::widthUnitChanged()
{
	bool useHeight = (widthUnitCO->currentItem() > 0);

	if (useHeight)
		widthED->setValidator(unsignedLengthValidator(widthED));
	else
		widthED->setValidator(new QDoubleValidator(0, 1000, 2, widthED));

	heightED->setEnabled(useHeight);
	heightUnitCO->setEnabled(useHeight);
	form_->changed();
}

} // namespace frontend
} // namespace lyx
