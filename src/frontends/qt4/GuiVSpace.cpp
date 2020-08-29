/**
 * \file GuiVSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jürgen Vigna
 * \author Rob Lahaye
 * \author Angus Leeming
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiVSpace.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "Spacing.h"
#include "VSpace.h"

#include "insets/InsetVSpace.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

using namespace std;

namespace lyx {
namespace frontend {

GuiVSpace::GuiVSpace(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(valueLE, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(keepCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(unitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));

	connect(spacingCO, SIGNAL(activated(int)),
		this, SLOT(enableCustom(int)));

	// Set up a signed glue length validator
	LengthValidator * v = new LengthValidator(valueLE);
	v->setBottom(GlueLength());
	valueLE->setValidator(v);

	// initialize the length validator
	addCheckedWidget(valueLE, valueL);
	enableCustom(spacingCO->currentIndex());
}


void GuiVSpace::enableCustom(int selection)
{
	bool const enable = selection == 5;
	valueLE->setEnabled(enable);
	if (enable)
		valueLE->setFocus();
	valueL->setEnabled(enable);
	unitCO->setEnabled(enable);
	changed();
}


static void setWidgetsFromVSpace(VSpace const & space,
			  QComboBox * spacing,
			  QLineEdit * value,
			  LengthCombo * unit,
			  QCheckBox * keep)
{
	int item = 0;
	switch (space.kind()) {
		case VSpace::DEFSKIP:   item = 0; break;
		case VSpace::SMALLSKIP: item = 1; break;
		case VSpace::MEDSKIP:   item = 2; break;
		case VSpace::BIGSKIP:   item = 3; break;
		case VSpace::VFILL:     item = 4; break;
		case VSpace::LENGTH:    item = 5; break;
	}
	spacing->setCurrentIndex(item);
	keep->setChecked(space.keep());

	Length::UNIT const default_unit = Length::defaultUnit();
	bool const custom_vspace = space.kind() == VSpace::LENGTH;
	if (custom_vspace) {
		value->setEnabled(true);
		unit->setEnabled(true);
		string length = space.length().asString();
		lengthToWidgets(value, unit, length, default_unit);
	} else {
		lengthToWidgets(value, unit, "", default_unit);
		value->setEnabled(false);
		unit->setEnabled(false);
	}
}


static VSpace setVSpaceFromWidgets(int spacing,
	QLineEdit * value, LengthCombo * unit, bool keep)
{
	VSpace space;

	switch (spacing) {
		case 0: space = VSpace(VSpace::DEFSKIP); break;
		case 1: space = VSpace(VSpace::SMALLSKIP); break;
		case 2: space = VSpace(VSpace::MEDSKIP); break;
		case 3: space = VSpace(VSpace::BIGSKIP); break;
		case 4: space = VSpace(VSpace::VFILL); break;
		case 5: space = VSpace(GlueLength(widgetsToLength(value, unit))); break;
	}

	space.setKeep(keep);
	return space;
}


docstring GuiVSpace::dialogToParams() const
{
	// If a vspace choice is "Length" but there's no text in
	// the input field, do not insert a vspace at all.
	if (spacingCO->currentIndex() == 5 && valueLE->text().isEmpty())
		return docstring();

	VSpace const params = setVSpaceFromWidgets(spacingCO->currentIndex(),
			valueLE, unitCO, keepCB->isChecked());
	return from_ascii(InsetVSpace::params2string(params));
}


void GuiVSpace::paramsToDialog(Inset const * inset)
{
	InsetVSpace const * vs = static_cast<InsetVSpace const *>(inset);
	VSpace const & params = vs->space();
	setWidgetsFromVSpace(params, spacingCO, valueLE, unitCO, keepCB);
	enableCustom(spacingCO->currentIndex());
}


bool GuiVSpace::checkWidgets(bool readonly) const
{
	keepCB->setEnabled(!readonly);
	keepL->setEnabled(!readonly);
	spacingCO->setEnabled(!readonly);
	spacingL->setEnabled(!readonly);

	valueLE->setReadOnly(readonly);

	bool const enable_custom =
		!readonly && (spacingCO->currentIndex() == 5);

	valueL->setEnabled(enable_custom);
	valueLE->setEnabled(enable_custom);
	unitCO->setEnabled(enable_custom);

	return InsetParamsWidget::checkWidgets();
}

} // namespace frontend
} // namespace lyx


#include "moc_GuiVSpace.cpp"
