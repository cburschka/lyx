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

#include "Spacing.h"

#include "insets/InsetSpace.h"

#include "mathed/InsetMathSpace.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

using namespace std;

namespace lyx {
namespace frontend {

GuiHSpace::GuiHSpace(bool math_mode, QWidget * parent)
	: InsetParamsWidget(parent), math_mode_(math_mode)
{
	setupUi(this);

	spacingCO->clear();
	if (math_mode_) {
		spacingCO->addItem(qt_("Thin Space"));
		spacingCO->addItem(qt_("Medium Space"));
		spacingCO->addItem(qt_("Thick Space"));
		spacingCO->addItem(qt_("Negative Thin Space"));
		spacingCO->addItem(qt_("Negative Medium Space"));
		spacingCO->addItem(qt_("Negative Thick Space"));
		spacingCO->addItem(qt_("Half Quad (0.5 em)"));
		spacingCO->addItem(qt_("Quad (1 em)"));
		spacingCO->addItem(qt_("Double Quad (2 em)"));
		spacingCO->addItem(qt_("Custom"));
	} else {
		spacingCO->addItem(qt_("Interword Space"));
		spacingCO->addItem(qt_("Thin Space"));
		spacingCO->addItem(qt_("Negative Thin Space"));
		spacingCO->addItem(qt_("Half Quad (0.5 em)"));
		spacingCO->addItem(qt_("Quad (1 em)"));
		spacingCO->addItem(qt_("Double Quad (2 em)"));
		spacingCO->addItem(qt_("Horizontal Fill"));
		spacingCO->addItem(qt_("Custom"));
	}

	connect(spacingCO, SIGNAL(highlighted(QString)),
		this, SLOT(changedSlot()));
	connect(valueLE, SIGNAL(textChanged(QString)),
		this, SLOT(changedSlot()));
	connect(spacingCO, SIGNAL(activated(int)),
		this, SLOT(changedSlot()));
	connect(keepCB, SIGNAL(clicked()),
		this, SLOT(changedSlot()));
	connect(unitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(changedSlot()));
	connect(fillPatternCO, SIGNAL(activated(int)),
		this, SLOT(changedSlot()));

	if (math_mode_)
		valueLE->setValidator(unsignedLengthValidator(valueLE));
	else
		valueLE->setValidator(unsignedGlueLengthValidator(valueLE));

	// initialize the length validator
	addCheckedWidget(valueLE, valueL);
	enableWidgets();
}


void GuiHSpace::changedSlot()
{
	enableWidgets();
	changed();
}


void GuiHSpace::enableWidgets()
{
	int const selection = spacingCO->currentIndex();
	bool const custom = (selection == spacingCO->count() - 1);
	valueLE->setEnabled(custom);
	unitCO->setEnabled(custom);
	if (math_mode_) {
		fillPatternCO->setEnabled(false);
		keepCB->setEnabled(false);
		return;
	}
	fillPatternCO->setEnabled(selection == 6);
	bool const no_pattern = fillPatternCO->currentIndex() == 0;
	bool const enable_keep =
		selection == 0 || selection == 3  ||
		(selection == 6 && no_pattern) || custom;
	keepCB->setEnabled(enable_keep);
}


void GuiHSpace::paramsToDialog(Inset const * inset)
{
	InsetSpaceParams const params = math_mode_
		? static_cast<InsetMathSpace const *>(inset)->params()
		: static_cast<InsetSpace const *>(inset)->params();

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
	spacingCO->setCurrentIndex(item);
	fillPatternCO->setCurrentIndex(pattern);
	keepCB->setChecked(protect);
	if (math_mode_) {
		keepCB->setToolTip(qt_("Insert the spacing even after a line break"));
	} else if (item == 3) {
		keepCB->setToolTip(qt_("Insert the spacing even after a line break.\n"
				       "Note that a protected Half Quad will be turned into\n"
				       "a vertical space if used at the beginning of a paragraph!"));
	} else {
		keepCB->setToolTip(qt_("Insert the spacing even after a line break"));
	}
	Length::UNIT const default_unit = Length::defaultUnit();
	if (item == (params.math ? 9 : 7)) {
		string length = params.length.asString();
		lengthToWidgets(valueLE, unitCO, length, default_unit);
	} else
		lengthToWidgets(valueLE, unitCO, "", default_unit);

	enableWidgets();
}


docstring GuiHSpace::dialogToMathParams() const
{
	InsetSpaceParams params(true);
	switch (spacingCO->currentIndex()) {
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
		params.length = GlueLength(widgetsToLength(valueLE, unitCO));
		break;
	}
	return from_ascii(InsetSpace::params2string(params));
}


docstring GuiHSpace::dialogToParams() const
{
	if (math_mode_)
		return dialogToMathParams();

	InsetSpaceParams params(false);

	switch (spacingCO->currentIndex()) {
		case 0:
			if (keepCB->isChecked())
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
			if (keepCB->isChecked())
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
			switch (fillPatternCO->currentIndex()) {
			case 1:
				params.kind = InsetSpaceParams::DOTFILL;
				break;
			case 2:
				params.kind = InsetSpaceParams::HRULEFILL;
				break;
			case 3:
				params.kind = InsetSpaceParams::LEFTARROWFILL;
				break;
			case 4:
				params.kind = InsetSpaceParams::RIGHTARROWFILL;
				break;
			case 5:
				params.kind = InsetSpaceParams::UPBRACEFILL;
				break;
			case 6:
				params.kind = InsetSpaceParams::DOWNBRACEFILL;
				break;
			default:
				if (keepCB->isChecked())
					params.kind = InsetSpaceParams::HFILL_PROTECTED;
				else
					params.kind = InsetSpaceParams::HFILL;
				break;
			}
			break;
		case 7:
			if (keepCB->isChecked())
				params.kind = InsetSpaceParams::CUSTOM_PROTECTED;
			else
				params.kind = InsetSpaceParams::CUSTOM;
			params.length = GlueLength(widgetsToLength(valueLE, unitCO));
			break;
	}
	return from_ascii(InsetSpace::params2string(params));
}


bool GuiHSpace::checkWidgets() const
{
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return spacingCO->currentIndex() != (math_mode_ ? 9 : 7)
		|| !valueLE->text().isEmpty();
}

} // namespace frontend
} // namespace lyx


#include "moc_GuiHSpace.cpp"
