/**
 * \file GuiLine.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiLine.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "FuncRequest.h"

#include "insets/InsetLine.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

using namespace std;

namespace lyx {
namespace frontend {

GuiLine::GuiLine(GuiView & lv)
	: GuiDialog(lv, "line", qt_("Horizontal line")),
	  params_(insetCode("line"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(OffsetLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(OffsetUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(WidthLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(WidthUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(HeightLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(HeightUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	// disable for read-only documents
	bc().addReadOnly(OffsetLE);
	bc().addReadOnly(OffsetUnitCO);
	bc().addReadOnly(WidthLE);
	bc().addReadOnly(WidthUnitCO);
	bc().addReadOnly(HeightLE);
	bc().addReadOnly(HeightUnitCO);

	// initialize the length validator
	bc().addCheckedLineEdit(OffsetLE, OffsetValueL);
	bc().addCheckedLineEdit(WidthLE, WidthValueL);
	bc().addCheckedLineEdit(HeightLE, HeightValueL);

	OffsetLE->setValidator(unsignedGlueLengthValidator(OffsetLE));
	WidthLE->setValidator(unsignedGlueLengthValidator(WidthLE));
	HeightLE->setValidator(unsignedGlueLengthValidator(HeightLE));
}


void GuiLine::change_adaptor()
{
	changed();
}


void GuiLine::paramsToDialog(InsetCommandParams const & /*icp*/)
{
	lengthToWidgets(OffsetLE,
			OffsetUnitCO,
			params_["offset"],
			Length::defaultUnit());
	lengthToWidgets(WidthLE,
			WidthUnitCO,
			params_["width"],
			Length::defaultUnit());
	lengthToWidgets(HeightLE,
			HeightUnitCO,
			params_["height"],
			Length::defaultUnit());

	bc().setValid(isValid());
}


void GuiLine::applyView()
{
	docstring offset = from_utf8(widgetsToLength(OffsetLE, OffsetUnitCO));
	params_["offset"] = offset;
	
	// negative widths are senseless
	string width_str = fromqstr(WidthLE->text());
	if (width_str[0] == '-')
		width_str.erase(0,1);
	WidthLE->setText(toqstr(width_str));
	docstring width = from_utf8(widgetsToLength(WidthLE, WidthUnitCO));
	params_["width"] = width;

	// negative heights are senseless
	string height_str = fromqstr(HeightLE->text());
	if (height_str[0] == '-')
		height_str.erase(0,1);
	HeightLE->setText(toqstr(height_str));
	docstring height = from_utf8(widgetsToLength(HeightLE, HeightUnitCO));
	params_["height"] = height;

	params_.setCmdName("rule");
}


bool GuiLine::isValid() const
{
	return true;
}


bool GuiLine::initialiseParams(std::string const & data)
{
	InsetCommand::string2params("line", data, params_);
	paramsToDialog(params_);
	return true;
}


void GuiLine::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string("line", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}



Dialog * createGuiLine(GuiView & lv) { return new GuiLine(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiLine.cpp"
