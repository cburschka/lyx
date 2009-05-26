/**
 * \file GuiPrintNomencl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPrintNomencl.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "Spacing.h"
#include "FuncRequest.h"

#include "insets/InsetNomencl.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

using namespace std;

namespace lyx {
namespace frontend {

GuiPrintNomencl::GuiPrintNomencl(GuiView & lv)
	: GuiDialog(lv, "nomencl_print", qt_("Nomenclature settings")),
	  params_(insetCode("nomencl_print"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(valueLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(unitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));

	valueLE->setValidator(unsignedLengthValidator(valueLE));

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	// disable for read-only documents
	bc().addReadOnly(valueLE);
	bc().addReadOnly(unitLC);

	// initialize the length validator
	bc().addCheckedLineEdit(valueLE, valueLA);

	setWidthCO->addItem(qt_("Default"),
		QVariant(toqstr("none")));
	setWidthCO->addItem(qt_("Longest label width"),
		QVariant(toqstr("auto")));
	setWidthCO->addItem(qt_("Custom"),
		QVariant(toqstr("custom")));
}


void GuiPrintNomencl::change_adaptor()
{
	changed();
}


void GuiPrintNomencl::on_setWidthCO_activated(int i)
{
	bool const custom =
		(setWidthCO->itemData(i).toString()
		 == "custom");
	valueLE->setEnabled(custom);
	unitLC->setEnabled(custom);
	valueLA->setEnabled(custom);
	changed();
}


void GuiPrintNomencl::paramsToDialog(InsetCommandParams const & /*icp*/)
{
	setWidthCO->setCurrentIndex(
		setWidthCO->findData(toqstr(params_["set_width"])));
	
	lengthToWidgets(valueLE,
			unitLC,
			params_["width"],
			Length::defaultUnit());

	bc().setValid(isValid());
}


void GuiPrintNomencl::updateContents()
{
	bool const custom = (setWidthCO->itemData(
				  setWidthCO->currentIndex()).toString()
			     == "custom");
	valueLE->setEnabled(custom);
	unitLC->setEnabled(custom);
	valueLA->setEnabled(custom);
}


void GuiPrintNomencl::applyView()
{
	docstring const set_width = qstring_to_ucs4(setWidthCO->itemData(
		setWidthCO->currentIndex()).toString());
	params_["set_width"] = set_width;
	docstring width;
	if (set_width == from_ascii("custom"))
		width = from_utf8(widgetsToLength(valueLE, unitLC));
	params_["width"] = width;
}


bool GuiPrintNomencl::isValid() const
{
	return setWidthCO->itemData(
			setWidthCO->currentIndex()).toString() != "custom"
		|| !valueLE->text().isEmpty();
}


bool GuiPrintNomencl::initialiseParams(std::string const & data)
{
	InsetCommand::string2params("nomencl_print", data, params_);
	paramsToDialog(params_);
	return true;
}


void GuiPrintNomencl::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string("nomencl_print", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}



Dialog * createGuiPrintNomencl(GuiView & lv)
{
	return new GuiPrintNomencl(lv);
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiPrintNomencl.cpp"
