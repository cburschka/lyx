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
		spacingCO->addItem(qt_("Interword Space"), "normal");
		spacingCO->addItem(qt_("Thin Space"), "thinspace");
		spacingCO->addItem(qt_("Medium Space"), "medspace");
		spacingCO->addItem(qt_("Thick Space"), "thickspace");
		spacingCO->addItem(qt_("Negative Thin Space"), "negthinspace");
		spacingCO->addItem(qt_("Negative Medium Space"), "negmedspace");
		spacingCO->addItem(qt_("Negative Thick Space"), "negthickspace");
		spacingCO->addItem(qt_("Half Quad (0.5 em)"), "halfquad");
		spacingCO->addItem(qt_("Quad (1 em)"), "quad");
		spacingCO->addItem(qt_("Double Quad (2 em)"), "qquad");
		spacingCO->addItem(qt_("Horizontal Fill"), "hfill");
		spacingCO->addItem(qt_("Custom"), "custom");
	} else {
		spacingCO->addItem(qt_("Interword Space"), "normal");
		spacingCO->addItem(qt_("Thin Space"), "thinspace");
		spacingCO->addItem(qt_("Negative Thin Space"), "negthinspace");
		spacingCO->addItem(qt_("Negative Medium Space"), "negmedspace");
		spacingCO->addItem(qt_("Negative Thick Space"), "negthickspace");
		spacingCO->addItem(qt_("Half Quad (0.5 em)"), "halfquad");
		spacingCO->addItem(qt_("Quad (1 em)"), "quad");
		spacingCO->addItem(qt_("Double Quad (2 em)"), "qquad");
		spacingCO->addItem(qt_("Horizontal Fill"), "hfill");
		spacingCO->addItem(qt_("Custom"), "custom");
		spacingCO->addItem(qt_("Visible Space"), "visible");
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

	// Set up a signed (glue) length validator
	LengthValidator * v = new LengthValidator(valueLE);
	if (math_mode_)
		v->setBottom(Length());
	else
		v->setBottom(GlueLength());
	valueLE->setValidator(v);

	// initialize the length validator
	addCheckedWidget(valueLE, valueL);
	enableWidgets();
}


void GuiHSpace::changedSlot()
{
	enableWidgets();
	changed();
}


void GuiHSpace::enableWidgets() const
{
	QString const selection = spacingCO->itemData(spacingCO->currentIndex()).toString();
	bool const custom = selection == "custom";
	valueLE->setEnabled(custom);
	if (custom)
		valueLE->setFocus();
	valueL->setEnabled(custom);
	unitCO->setEnabled(custom);
	fillPatternCO->setEnabled(!math_mode_ && selection == "hfill");
	fillPatternL->setEnabled(!math_mode_ && selection == "hfill");
	bool const no_pattern = fillPatternCO->currentIndex() == 0 || math_mode_;
	bool const enable_keep =
		selection == "normal" || selection == "halfquad"
		|| (selection == "hfill" && no_pattern) || custom;
	keepCB->setEnabled(enable_keep);
	keepL->setEnabled(enable_keep);
}


void GuiHSpace::paramsToDialog(Inset const * inset)
{
	InsetSpaceParams const params = math_mode_
		? static_cast<InsetMathSpace const *>(inset)->params()
		: static_cast<InsetSpace const *>(inset)->params();

	QString item;
	int pattern = 0;
	bool protect = false;
	switch (params.kind) {
		case InsetSpaceParams::NORMAL:
			item = "normal";
			break;
		case InsetSpaceParams::PROTECTED:
			item = "normal";
			protect = true;
			break;
		case InsetSpaceParams::VISIBLE:
			item = "visible";
			protect = true;
			break;
		case InsetSpaceParams::THIN:
			item = "thinspace";
			break;
		case InsetSpaceParams::MEDIUM:
			item = "medspace";
			break;
		case InsetSpaceParams::THICK:
			item = "thickspace";
			break;
		case InsetSpaceParams::NEGTHIN:
			item = "negthinspace";
			break;
		case InsetSpaceParams::NEGMEDIUM:
			item = "negmedspace";
			break;
		case InsetSpaceParams::NEGTHICK:
			item = "negthickspace";
			break;
		case InsetSpaceParams::ENSKIP:
			item = "halfquad";
			break;
		case InsetSpaceParams::ENSPACE:
			item = "halfquad";
			protect = true;
			break;
		case InsetSpaceParams::QUAD:
			item = "quad";
			break;
		case InsetSpaceParams::QQUAD:
			item = "qquad";
			break;
		case InsetSpaceParams::HFILL:
			item = "hfill";
			break;
		case InsetSpaceParams::HFILL_PROTECTED:
			item = "hfill";
			protect = true;
			break;
		case InsetSpaceParams::DOTFILL:
			item = "hfill";
			pattern = 1;
			break;
		case InsetSpaceParams::HRULEFILL:
			item = "hfill";
			pattern = 2;
			break;
		case InsetSpaceParams::LEFTARROWFILL:
			item = "hfill";
			pattern = 3;
			break;
		case InsetSpaceParams::RIGHTARROWFILL:
			item = "hfill";
			pattern = 4;
			break;
		case InsetSpaceParams::UPBRACEFILL:
			item = "hfill";
			pattern = 5;
			break;
		case InsetSpaceParams::DOWNBRACEFILL:
			item = "hfill";
			pattern = 6;
			break;
		case InsetSpaceParams::CUSTOM:
			item = "custom";
			break;
		case InsetSpaceParams::CUSTOM_PROTECTED:
			item = "custom";
			protect = true;
			break;
	}
	spacingCO->setCurrentIndex(spacingCO->findData(item));
	fillPatternCO->setCurrentIndex(pattern);
	keepCB->setChecked(protect);
	if (item == "halfquad") {
		keepCB->setToolTip(qt_("Insert the spacing even after a line break.\n"
				       "Note that a protected Half Quad will be turned into\n"
				       "a vertical space if used at the beginning of a paragraph!"));
	} else {
		keepCB->setToolTip(qt_("Insert the spacing even after a line break"));
	}
	Length::UNIT const default_unit = Length::defaultUnit();
	if (item == "custom") {
		string length = params.length.asString();
		lengthToWidgets(valueLE, unitCO, length, default_unit);
	} else
		lengthToWidgets(valueLE, unitCO, "", default_unit);

	enableWidgets();
}


docstring GuiHSpace::dialogToParams() const
{
	InsetSpaceParams params = math_mode_ ?
		InsetSpaceParams(true) : InsetSpaceParams(false);

	QString const item =
		spacingCO->itemData(spacingCO->currentIndex()).toString();

	if (item == "normal")
		params.kind = keepCB->isChecked() ?
			InsetSpaceParams::PROTECTED : InsetSpaceParams::NORMAL;
	else if (item == "thinspace")
		params.kind = InsetSpaceParams::THIN;
	else if (item == "medspace")
		params.kind = InsetSpaceParams::MEDIUM;
	else if (item == "thickspace")
		params.kind = InsetSpaceParams::THICK;
	else if (item == "negthinspace")
		params.kind = InsetSpaceParams::NEGTHIN;
	else if (item == "negmedspace")
		params.kind = InsetSpaceParams::NEGMEDIUM;
	else if (item == "negthickspace")
		params.kind = InsetSpaceParams::NEGTHICK;
	else if (item == "halfquad")
		params.kind = keepCB->isChecked() ?
			InsetSpaceParams::ENSPACE : InsetSpaceParams::ENSKIP;
	else if (item == "quad")
			params.kind = InsetSpaceParams::QUAD;
	else if (item == "qquad")
			params.kind = InsetSpaceParams::QQUAD;
	else if (item == "hfill") {
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
	} else if (item == "custom") {
			params.kind = keepCB->isChecked() ?
				InsetSpaceParams::CUSTOM_PROTECTED : InsetSpaceParams::CUSTOM;
			params.length = GlueLength(widgetsToLength(valueLE, unitCO));
	} else if (item == "visible") 
		params.kind = InsetSpaceParams::VISIBLE;

	return from_ascii(InsetSpace::params2string(params));
}


bool GuiHSpace::checkWidgets(bool readonly) const
{
	valueLE->setReadOnly(readonly);

	if (readonly) {
		spacingCO->setEnabled(false);
		unitCO->setEnabled(false);
		fillPatternCO->setEnabled(false);
		keepCB->setEnabled(false);
		valueLE->setEnabled(false);
	} else
		enableWidgets();

	if (!InsetParamsWidget::checkWidgets())
		return false;
	return spacingCO->itemData(spacingCO->currentIndex()).toString() != "custom"
		|| !valueLE->text().isEmpty();
}

} // namespace frontend
} // namespace lyx


#include "moc_GuiHSpace.cpp"
