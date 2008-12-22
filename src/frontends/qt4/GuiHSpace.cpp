/**
 * \file GuiHSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiHSpace.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "LyXRC.h"
#include "Spacing.h"
#include "FuncRequest.h"

#include "insets/InsetSpace.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

using namespace std;

namespace lyx {
namespace frontend {

GuiHSpace::GuiHSpace(GuiView & lv, bool math)
	: GuiDialog(lv, math ? "mathspace" : "space", qt_("Horizontal Space Settings"))
{
	params_.math = math;
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(spacingCO, SIGNAL(highlighted(QString)),
		this, SLOT(change_adaptor()));
	connect(valueLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(spacingCO, SIGNAL(activated(int)),
		this, SLOT(enableWidgets(int)));
	connect(keepCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(unitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(fillPatternCO, SIGNAL(activated(int)),
		this, SLOT(patternChanged()));

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
	bc().addReadOnly(fillPatternCO);

	// initialize the length validator
	bc().addCheckedLineEdit(valueLE, valueL);

	// remove the %-items from the unit choice
	unitCO->noPercents();
}


void GuiHSpace::change_adaptor()
{
	changed();
}


void GuiHSpace::setMath(bool custom)
{
	valueLE->setEnabled(custom);
	unitCO->setEnabled(custom);
	fillPatternCO->setEnabled(false);
	keepCB->setToolTip(qt_("Insert the spacing even after a line break"));
	keepCB->setEnabled(false);
}


void GuiHSpace::enableWidgets(int selection)
{
	if (params_.math) {
		setMath(selection == 9);
		changed();
		return;
	}
	valueLE->setEnabled(selection == 7);
	unitCO->setEnabled(selection == 7);
	fillPatternCO->setEnabled(selection == 6);
	int pattern = fillPatternCO->currentIndex();
	bool const enable_keep =
		selection == 0 || selection == 3  ||
		(selection == 6 && pattern == 0) || selection == 7;
	keepCB->setEnabled(enable_keep);
	if (selection == 3)
		keepCB->setToolTip(qt_("Insert the spacing even after a line break.\n"
				       "Note that a protected Half Quad will be turned into\n"
				       "a vertical space if used at the beginning of a paragraph!"));
	else
		keepCB->setToolTip(qt_("Insert the spacing even after a line break"));
	changed();
}


void GuiHSpace::patternChanged()
{
	enableWidgets(spacingCO->currentIndex());
	changed();
}


static void setWidgetsFromHSpace(InsetSpaceParams const & params,
			  QComboBox * spacing,
			  QLineEdit * value,
			  LengthCombo * unit,
			  QCheckBox * keep,
			  QComboBox * fillPattern)
{
	spacing->clear();
	if (params.math) {
		spacing->addItem(qt_("Thin space"));
		spacing->addItem(qt_("Medium space"));
		spacing->addItem(qt_("Thick space"));
		spacing->addItem(qt_("Negative thin space"));
		spacing->addItem(qt_("Negative medium space"));
		spacing->addItem(qt_("Negative thick space"));
		spacing->addItem(qt_("Half Quad (0.5 em)"));
		spacing->addItem(qt_("Quad (1 em)"));
		spacing->addItem(qt_("Double Quad (2 em)"));
		spacing->addItem(qt_("Custom"));
		keep->setEnabled(false);
		fillPattern->setEnabled(false);
	} else {
		spacing->addItem(qt_("Inter-word space"));
		spacing->addItem(qt_("Thin space"));
		spacing->addItem(qt_("Negative thin space"));
		spacing->addItem(qt_("Half Quad (0.5 em)"));
		spacing->addItem(qt_("Quad (1 em)"));
		spacing->addItem(qt_("Double Quad (2 em)"));
		spacing->addItem(qt_("Horizontal Fill"));
		spacing->addItem(qt_("Custom"));
		keep->setEnabled(true);
		fillPattern->setEnabled(true);
	}

	int item = 0;
	int pattern = 0;
	bool protect = false;
	switch (params.kind) {
		case InsetSpaceParams::NORMAL:
			item = 0;
			break;
		case InsetSpaceParams::PROTECTED:
			item = 0;
			protect = !params.math;
			break;
		case InsetSpaceParams::THIN:
			item = params.math ? 0 : 1;
			break;
		case InsetSpaceParams::MEDIUM:
			item = 1;
			break;
		case InsetSpaceParams::THICK:
			item = params.math ? 2 : 1;
			break;
		case InsetSpaceParams::NEGTHIN:
			item = params.math ? 3 : 2;
			break;
		case InsetSpaceParams::NEGMEDIUM:
			item = params.math ? 4 : 2;
			break;
		case InsetSpaceParams::NEGTHICK:
			item = params.math ? 5 : 2;
			break;
		case InsetSpaceParams::ENSKIP:
			item = params.math ? 6 : 3;
			break;
		case InsetSpaceParams::ENSPACE:
			item = params.math ? 6 : 3;
			protect = !params.math;
			break;
		case InsetSpaceParams::QUAD:
			item = params.math ? 7 : 4;
			break;
		case InsetSpaceParams::QQUAD:
			item = params.math ? 8 : 5;
			break;
		case InsetSpaceParams::HFILL:
			item = params.math ? 3 : 6;
			break;
		case InsetSpaceParams::HFILL_PROTECTED:
			item = params.math ? 3 : 6;
			protect = !params.math;
			break;
		case InsetSpaceParams::DOTFILL:
			item = params.math ? 3 : 6;
			pattern = 1;
			break;
		case InsetSpaceParams::HRULEFILL:
			item = params.math ? 3 : 6;
			pattern = 2;
			break;
		case InsetSpaceParams::LEFTARROWFILL:
			item = params.math ? 3 : 6;
			pattern = 3;
			break;
		case InsetSpaceParams::RIGHTARROWFILL:
			item = params.math ? 3 : 6;
			pattern = 4;
			break;
		case InsetSpaceParams::UPBRACEFILL:
			item = params.math ? 3 : 6;
			pattern = 5;
			break;
		case InsetSpaceParams::DOWNBRACEFILL:
			item = params.math ? 3 : 6;
			pattern = 6;
			break;
		case InsetSpaceParams::CUSTOM:
			item = params.math ? 9 : 7;
			break;
		case InsetSpaceParams::CUSTOM_PROTECTED:
			item = params.math ? 9 : 7;
			protect = !params.math;
			break;
	}
	spacing->setCurrentIndex(item);
	fillPattern->setCurrentIndex(pattern);
	keep->setChecked(protect);

	Length::UNIT default_unit =
			(lyxrc.default_papersize > 3) ? Length::CM : Length::IN;
	if (item == (params.math ? 9 : 7))
		lengthToWidgets(value, unit, params.length, default_unit);
	else
		lengthToWidgets(value, unit, "", default_unit);
}


static InsetSpaceParams setHSpaceFromWidgets(int spacing,
	QLineEdit * value, LengthCombo * unit, bool keep, int fill, bool math)
{
	InsetSpaceParams params(math);
	if (math) {
		switch (spacing) {
		case 0: params.kind = InsetSpaceParams::THIN;      break;
		case 1: params.kind = InsetSpaceParams::MEDIUM;    break;
		case 2: params.kind = InsetSpaceParams::THICK;     break;
		case 3: params.kind = InsetSpaceParams::NEGTHIN;   break;
		case 4: params.kind = InsetSpaceParams::NEGMEDIUM; break;
		case 5: params.kind = InsetSpaceParams::NEGTHICK;  break;
		case 6: params.kind = InsetSpaceParams::ENSKIP;    break;
		case 7: params.kind = InsetSpaceParams::QUAD;      break;
		case 8: params.kind = InsetSpaceParams::QQUAD;     break;
		case 9:
			params.kind = InsetSpaceParams::CUSTOM;
			params.length = Length(widgetsToLength(value, unit));
			break;
		}
		return params;
	}

	switch (spacing) {
		case 0:
			if (keep)
				params.kind = InsetSpaceParams::PROTECTED;
			else
				params.kind = InsetSpaceParams::NORMAL;
			break;
		case 1:
			params.kind = InsetSpaceParams::THIN;
			break;
		case 2:
			params.kind = InsetSpaceParams::NEGTHIN;
			break;
		case 3:
			if (keep)
				params.kind = InsetSpaceParams::ENSPACE;
			else
				params.kind = InsetSpaceParams::ENSKIP;
			break;
		case 4:
			params.kind = InsetSpaceParams::QUAD;
			break;
		case 5:
			params.kind = InsetSpaceParams::QQUAD;
			break;
		case 6:
			if (fill == 1)
				params.kind = InsetSpaceParams::DOTFILL;
			else if (fill == 2)
				params.kind = InsetSpaceParams::HRULEFILL;
			else if (fill == 3)
				params.kind = InsetSpaceParams::LEFTARROWFILL;
			else if (fill == 4)
				params.kind = InsetSpaceParams::RIGHTARROWFILL;
			else if (fill == 5)
				params.kind = InsetSpaceParams::UPBRACEFILL;
			else if (fill == 6)
				params.kind = InsetSpaceParams::DOWNBRACEFILL;
			else if (keep)
				params.kind = InsetSpaceParams::HFILL_PROTECTED;
			else
				params.kind = InsetSpaceParams::HFILL;
			break;
		case 7:
			if (keep)
				params.kind = InsetSpaceParams::CUSTOM_PROTECTED;
			else
				params.kind = InsetSpaceParams::CUSTOM;
			params.length = Length(widgetsToLength(value, unit));
			break;
	}
	return params;
}


void GuiHSpace::applyView()
{
	params_ = setHSpaceFromWidgets(spacingCO->currentIndex(),
			valueLE, unitCO, keepCB->isChecked(),
			fillPatternCO->currentIndex(), params_.math);
}


void GuiHSpace::updateContents()
{
	setWidgetsFromHSpace(params_, spacingCO, valueLE, unitCO, keepCB,
		fillPatternCO);
	enableWidgets(spacingCO->currentIndex());
}


bool GuiHSpace::initialiseParams(string const & data)
{
	bool const math = params_.math;
	InsetSpace::string2params(data, params_);
	params_.math = math;
	if (params_.math)
		setMath(params_.kind == InsetSpaceParams::CUSTOM);
	setButtonsValid(true);
	return true;
}


void GuiHSpace::clearParams()
{
	params_ = InsetSpaceParams(params_.math);
}


void GuiHSpace::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetSpace::params2string(params_)));
}


bool GuiHSpace::isValid()
{
	return spacingCO->currentIndex() != (params_.math ? 9 : 7)
		|| !valueLE->text().isEmpty();
}


Dialog * createGuiMathHSpace(GuiView & lv) { return new GuiHSpace(lv, true); }


Dialog * createGuiTextHSpace(GuiView & lv) { return new GuiHSpace(lv, false); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiHSpace.cpp"
