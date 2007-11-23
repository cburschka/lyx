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

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"
#include "FuncRequest.h"

#include "insets/InsetWrap.h"

#include "support/lstrings.h"

#include <QLineEdit>
#include <QCloseEvent>
#include <QPushButton>

using std::string;


namespace lyx {
namespace frontend {

GuiWrap::GuiWrap(GuiView & lv)
	: GuiDialog(lv, "wrap")
{
	setupUi(this);
	setViewTitle(_("Wrap Float Settings"));

	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(widthUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(QString)),
		this, SLOT(change_adaptor()));
	connect(overhangCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(overhangED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(overhangUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(linesCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(linesSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));

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


void GuiWrap::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiWrap::change_adaptor()
{
	changed();
}


void GuiWrap::applyView()
{
	double const width_value = widthED->text().toDouble();
	Length::UNIT widthUnit = widthUnitLC->currentLengthItem();
	if (widthED->text().isEmpty())
		widthUnit = Length::UNIT_NONE;
	double const overhang_value = overhangED->text().toDouble();
	Length::UNIT overhangUnit = overhangUnitLC->currentLengthItem();
	if (overhangED->text().isEmpty())
		overhangUnit = Length::UNIT_NONE;
	
	params_.width = Length(width_value, widthUnit);

	if (overhangCB->checkState() == Qt::Checked)
		params_.overhang = Length(overhang_value, overhangUnit);
	else
		// when value is "0" the option is not set in the LaTeX-output
		// in InsetWrap.cpp
		params_.overhang = Length("0in");

	if (linesCB->checkState() == Qt::Checked)
		params_.lines = linesSB->value();
	else
		// when value is "0" the option is not set in the LaTeX-output
		// in InsetWrap.cpp
		params_.lines = 0;

	switch (valignCO->currentIndex()) {
	case 0:
		params_.placement = "o";
		break;
	case 1:
		params_.placement = "i";
		break;
	case 2:
		params_.placement = "l";
		break;
	case 3:
		params_.placement = "r";
		break;
	}
}


void GuiWrap::updateContents()
{
	// 0pt is a legal width now, it yields a
	// wrapfloat just wide enough for the contents.
	Length len_w = params_.width;
	widthED->setText(QString::number(len_w.value()));
	widthUnitLC->setCurrentItem(len_w.unit());

	Length len_o(params_.overhang);
	overhangED->setText(QString::number(len_o.value()));
	overhangUnitLC->setCurrentItem(len_o.unit());
	if (len_o.value() == 0)
		overhangCB->setCheckState(Qt::Unchecked);
	else
		overhangCB->setCheckState(Qt::Checked);

	linesSB->setValue(params_.lines);
	if (params_.lines == 0)
		linesCB->setCheckState(Qt::Unchecked);
	else
		linesCB->setCheckState(Qt::Checked);

	int item = 0;
	if (params_.placement == "i")
		item = 1;
	else if (params_.placement == "l")
		item = 2;
	else if (params_.placement == "r")
		item = 3;

	valignCO->setCurrentIndex(item);
}


bool GuiWrap::initialiseParams(string const & data)
{
	InsetWrapMailer::string2params(data, params_);
	return true;
}


void GuiWrap::clearParams()
{
	params_ = InsetWrapParams();
}


void GuiWrap::dispatchParams()
{
	string const lfun = InsetWrapMailer::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


Dialog * createGuiWrap(GuiView & lv) { return new GuiWrap(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiWrap_moc.cpp"
