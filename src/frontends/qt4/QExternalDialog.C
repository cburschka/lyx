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

#include "support/convert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

#include "QExternalDialog.h"

#include "lengthcombo.h"
#include "validators.h"
#include "qt_helpers.h"
#include "QExternal.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QLineEdit>


using lyx::support::float_equal;
using lyx::support::isStrDbl;
using std::string;

namespace lyx {
namespace frontend {

QExternalDialog::QExternalDialog(QExternal * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	connect( displayCB, SIGNAL( toggled(bool) ), showCO, SLOT( setEnabled(bool) ) );
	connect( displayCB, SIGNAL( toggled(bool) ), displayscaleED, SLOT( setEnabled(bool) ) );
	connect( showCO, SIGNAL( activated(const QString&) ), this, SLOT( change_adaptor() ) );
	connect( originCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
	connect( aspectratioCB, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
	connect( browsePB, SIGNAL( clicked() ), this, SLOT( browseClicked() ) );
	connect( editPB, SIGNAL( clicked() ), this, SLOT( editClicked() ) );
	connect( externalCO, SIGNAL( activated(const QString&) ), this, SLOT( templateChanged() ) );
	connect( extraED, SIGNAL( textChanged(const QString&) ), this, SLOT( extraChanged(const QString&) ) );
	connect( extraFormatCO, SIGNAL( activated(const QString&) ), this, SLOT( formatChanged(const QString&) ) );
	connect( widthUnitCO, SIGNAL( activated(int) ), this, SLOT( widthUnitChanged() ) );
	connect( heightUnitCO, SIGNAL( selectionChanged(LyXLength::UNIT) ), this, SLOT( change_adaptor() ) );
	connect( displayCB, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
	connect( displayscaleED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
	connect( angleED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
	connect( widthED, SIGNAL( textChanged(const QString&) ), this, SLOT( sizeChanged() ) );
	connect( heightED, SIGNAL( textChanged(const QString&) ), this, SLOT( sizeChanged() ) );
	connect( fileED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
	connect( clipCB, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
	connect( getbbPB, SIGNAL( clicked() ), this, SLOT( getbbClicked() ) );
	connect( xrED, SIGNAL( textChanged(const QString&) ), this, SLOT( bbChanged() ) );
	connect( ytED, SIGNAL( textChanged(const QString&) ), this, SLOT( bbChanged() ) );
	connect( xlED, SIGNAL( textChanged(const QString&) ), this, SLOT( bbChanged() ) );
	connect( ybED, SIGNAL( textChanged(const QString&) ), this, SLOT( bbChanged() ) );
	connect( draftCB, SIGNAL( clicked() ), this, SLOT( change_adaptor() ) );

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

	fileED->setValidator(new PathValidator(true, fileED));
}


void QExternalDialog::show()
{
	QDialog::show();
	fileED->setFocus();
}



bool QExternalDialog::activateAspectratio() const
{
	if (widthUnitCO->currentIndex() == 0)
		return false;

	string const wstr = fromqstr(widthED->text());
	if (wstr.empty())
		return false;
	bool const wIsDbl = isStrDbl(wstr);
	if (wIsDbl && float_equal(convert<double>(wstr), 0.0, 0.05))
		return false;
	LyXLength l;
	if (!wIsDbl && (!isValidLength(wstr, &l) || l.zero()))
		return false;

	string const hstr = fromqstr(heightED->text());
	if (hstr.empty())
		return false;
	bool const hIsDbl = isStrDbl(hstr);
	if (hIsDbl && float_equal(convert<double>(hstr), 0.0, 0.05))
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
	int const choice =  externalCO->currentIndex();
	docstring const template_name = 
		lyx::from_utf8(form_->controller().getTemplate(choice).lyxName);
	docstring const str =
		form_->controller().browse(qstring_to_ucs4(fileED->text()),
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
	bool useHeight = (widthUnitCO->currentIndex() > 0);

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

#include "QExternalDialog_moc.cpp"
