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

GuiLine::GuiLine(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(OffsetLE, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(OffsetUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(WidthLE, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(WidthUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(HeightLE, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(HeightUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));

	// initialize the length validator
	addCheckedWidget(OffsetLE, OffsetValueL);
	addCheckedWidget(WidthLE, WidthValueL);
	addCheckedWidget(HeightLE, HeightValueL);

	OffsetLE->setValidator(unsignedGlueLengthValidator(OffsetLE));
	WidthLE->setValidator(unsignedGlueLengthValidator(WidthLE));
	HeightLE->setValidator(unsignedGlueLengthValidator(HeightLE));
}


docstring GuiLine::dialogToParams() const
{
	docstring offset = from_utf8(widgetsToLength(OffsetLE, OffsetUnitCO));
	InsetCommandParams params(insetCode());
	params["offset"] = offset;

	// negative widths are senseless
	string width_str = fromqstr(WidthLE->text());
	if (width_str[0] == '-')
		width_str.erase(0,1);
	WidthLE->setText(toqstr(width_str));
	docstring width = from_utf8(widgetsToLength(WidthLE, WidthUnitCO));
	params["width"] = width;

	// negative heights are senseless
	string height_str = fromqstr(HeightLE->text());
	if (height_str[0] == '-')
		height_str.erase(0,1);
	HeightLE->setText(toqstr(height_str));
	docstring height = from_utf8(widgetsToLength(HeightLE, HeightUnitCO));
	params["height"] = height;

	params.setCmdName("rule");
	return from_ascii(InsetLine::params2string("line", params));
}


void GuiLine::paramsToDialog(Inset const * inset)
{
	InsetLine const * line = static_cast<InsetLine const *>(inset);
	InsetCommandParams const & params = line->params();
	lengthToWidgets(OffsetLE,
			OffsetUnitCO,
			params["offset"],
			Length::defaultUnit());
	lengthToWidgets(WidthLE,
			WidthUnitCO,
			params["width"],
			Length::defaultUnit());
	lengthToWidgets(HeightLE,
			HeightUnitCO,
			params["height"],
			Length::defaultUnit());
}


bool GuiLine::checkWidgets() const
{
	if (!InsetParamsWidget::checkWidgets())
		return false;
	// FIXME: Is there something else to check?
	// Transfer some code from dialogToParams()?
	return true;
}

} // namespace frontend
} // namespace lyx


#include "moc_GuiLine.cpp"
