/**
 * \file GuiWrap.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiWrap.h"

#include "ControlWrap.h"
#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "insets/InsetWrap.h"

#include "support/lstrings.h"

#include <QLineEdit>
#include <QCloseEvent>
#include <QPushButton>

using std::string;


namespace lyx {
namespace frontend {

GuiWrapDialog::GuiWrapDialog(LyXView & lv)
	: GuiDialog(lv, "wrap")
{
	setupUi(this);
	setViewTitle(_("Wrap Float Settings"));
	setController(new ControlWrap(*this));

	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(widthUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(const QString &)),
		this, SLOT(change_adaptor()));
	connect(overhangCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(overhangED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(overhangUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(linesCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(linesSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));

	connect(overhangCB, SIGNAL(stateChanged(int)), this, SLOT(overhangChecked(int)));
	connect(linesCB, SIGNAL(stateChanged(int)), this, SLOT(linesChecked(int)));

	widthED->setValidator(unsignedLengthValidator(widthED));
	// FIXME:
	// overhang can be negative, but the unsignedLengthValidator allows this
	overhangED->setValidator(unsignedLengthValidator(overhangED));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setRestore(restorePB);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	bc().addReadOnly(widthED);
	bc().addReadOnly(widthUnitLC);
	bc().addReadOnly(valignCO);
	bc().addReadOnly(overhangCB);
	bc().addReadOnly(overhangED);
	bc().addReadOnly(overhangUnitLC);
	bc().addReadOnly(linesCB);
	bc().addReadOnly(linesSB);

	// initialize the length validator
	bc().addCheckedLineEdit(widthED, widthLA);
	bc().addCheckedLineEdit(overhangED, overhangCB);
}


ControlWrap & GuiWrapDialog::controller()
{
	return static_cast<ControlWrap &>(GuiDialog::controller());
}


void GuiWrapDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiWrapDialog::change_adaptor()
{
	changed();
}


void GuiWrapDialog::overhangChecked(int checkState)
{
	if (checkState == Qt::Checked) {
		overhangED->setEnabled(true);
		overhangUnitLC->setEnabled(true);
	} else { 
		overhangED->setEnabled(false);
		overhangUnitLC->setEnabled(false);
	}
}

void GuiWrapDialog::linesChecked(int checkState)
{
	if (checkState == Qt::Checked)
		linesSB->setEnabled(true);
	else 
		linesSB->setEnabled(false);
}


void GuiWrapDialog::applyView()
{
	double const width_value = widthED->text().toDouble();
	Length::UNIT widthUnit = widthUnitLC->currentLengthItem();
	if (widthED->text().isEmpty())
		widthUnit = Length::UNIT_NONE;
	double const overhang_value = overhangED->text().toDouble();
	Length::UNIT overhangUnit = overhangUnitLC->currentLengthItem();
	if (overhangED->text().isEmpty())
		overhangUnit = Length::UNIT_NONE;
	
	InsetWrapParams & params = controller().params();

	params.width = Length(width_value, widthUnit);

	if (overhangCB->checkState() == Qt::Checked)
		params.overhang = Length(overhang_value, overhangUnit);
	else
		// when value is "0" the option is not set in the LaTeX-output
		// in InsetWrap.cpp
		params.overhang = Length("0in");

	if (linesCB->checkState() == Qt::Checked)
		params.lines = linesSB->value();
	else
		// when value is "0" the option is not set in the LaTeX-output
		// in InsetWrap.cpp
		params.lines = 0;

	switch (valignCO->currentIndex()) {
	case 0:
		params.placement = "o";
		break;
	case 1:
		params.placement = "i";
		break;
	case 2:
		params.placement = "l";
		break;
	case 3:
		params.placement = "r";
		break;
	}
}


void GuiWrapDialog::updateContents()
{
	InsetWrapParams & params = controller().params();

	//0pt is a legal width now, it yields a
	//wrapfloat just wide enough for the contents.
	Length len_w(params.width);
	widthED->setText(QString::number(len_w.value()));
	widthUnitLC->setCurrentItem(len_w.unit());
	Length len_o(params.overhang);
	overhangED->setText(QString::number(len_o.value()));
	overhangUnitLC->setCurrentItem(len_o.unit());
	linesSB->setValue(params.lines);

	int item = 0;
	if (params.placement == "i")
		item = 1;
	else if (params.placement == "l")
		item = 2;
	else if (params.placement == "r")
		item = 3;

	valignCO->setCurrentIndex(item);
}

} // namespace frontend
} // namespace lyx


#include "GuiWrap_moc.cpp"
