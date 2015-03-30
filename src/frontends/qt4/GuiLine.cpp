/**
 * \file GuiLine.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
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

	OffsetLE->setText("0.5");
	OffsetUnitCO->setCurrentItem(Length::EX);
	WidthLE->setText("100");
	WidthUnitCO->setCurrentItem(Length::PCW);
	HeightLE->setText("1");
	HeightUnitCO->setCurrentItem(Length::PT);
	setFocusProxy(WidthLE);
}


docstring GuiLine::dialogToParams() const
{
	InsetCommandParams params(insetCode());
	params["offset"] = from_utf8(widgetsToLength(OffsetLE, OffsetUnitCO));
	params["width"] = from_utf8(widgetsToLength(WidthLE, WidthUnitCO));
	params["height"] = from_utf8(widgetsToLength(HeightLE, HeightUnitCO));
	params.setCmdName("rule");
	return from_ascii(InsetLine::params2string(params));
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


bool GuiLine::checkWidgets(bool readonly) const
{
	WidthLE->setReadOnly(readonly);
	HeightLE->setReadOnly(readonly);
	OffsetLE->setReadOnly(readonly);
	OffsetUnitCO->setEnabled(!readonly);
	HeightUnitCO->setEnabled(!readonly);
	WidthUnitCO->setEnabled(!readonly);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	// FIXME: this should be handled in unsignedGlueLengthValidator!
	if (WidthLE->text().startsWith('-'))
		return false;
	// FIXME: this should be handled in unsignedGlueLengthValidator!
	if (HeightLE->text().startsWith('-'))
		return false;
	return true;
}

} // namespace frontend
} // namespace lyx


#include "moc_GuiLine.cpp"
