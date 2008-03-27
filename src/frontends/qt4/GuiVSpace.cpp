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

#include "LyXRC.h" // to set the default length values
#include "Spacing.h"
#include "FuncRequest.h"

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

GuiVSpace::GuiVSpace(GuiView & lv)
	: GuiDialog(lv, "vspace", qt_("Vertical Space Settings"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(spacingCO, SIGNAL(highlighted(QString)),
		this, SLOT(change_adaptor()));
	connect(valueLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(spacingCO, SIGNAL(activated(int)),
		this, SLOT(enableCustom(int)));
	connect(keepCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(unitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));

	valueLE->setValidator(unsignedLengthValidator(valueLE));

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	// disable for read-only documents
	bc().addReadOnly(spacingCO);
	bc().addReadOnly(valueLE);
	bc().addReadOnly(unitCO);
	bc().addReadOnly(keepCB);

	// initialize the length validator
	bc().addCheckedLineEdit(valueLE, valueL);

	// remove the %-items from the unit choice
	unitCO->noPercents();
}


void GuiVSpace::change_adaptor()
{
	changed();
}


void GuiVSpace::enableCustom(int selection)
{
	bool const enable = selection == 5;
	valueLE->setEnabled(enable);
	unitCO->setEnabled(enable);
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

	Length::UNIT default_unit =
			(lyxrc.default_papersize > 3) ? Length::CM : Length::IN;
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


void GuiVSpace::applyView()
{
	// If a vspace choice is "Length" but there's no text in
	// the input field, do not insert a vspace at all.
	if (spacingCO->currentIndex() == 5 && valueLE->text().isEmpty())
		return;

	params_ = setVSpaceFromWidgets(spacingCO->currentIndex(),
			valueLE, unitCO, keepCB->isChecked()); 
}


void GuiVSpace::updateContents()
{
	setWidgetsFromVSpace(params_, spacingCO, valueLE, unitCO, keepCB);
}


bool GuiVSpace::initialiseParams(string const & data)
{
	InsetVSpace::string2params(data, params_);
	setButtonsValid(true);
	return true;
}


void GuiVSpace::clearParams()
{
	params_ = VSpace();
}


void GuiVSpace::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetVSpace::params2string(params_)));
}


Dialog * createGuiVSpace(GuiView & lv) { return new GuiVSpace(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiVSpace_moc.cpp"
