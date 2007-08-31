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
#include "Qt2BC.h"

#include "CheckedLineEdit.h"
#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "LyXRC.h" // to set the default length values
#include "Spacing.h"
#include "VSpace.h"

#include "controllers/ControlVSpace.h"
#include "controllers/frontend_helpers.h"

#include "support/lstrings.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>


using std::string;

namespace lyx {
namespace frontend {


/////////////////////////////////////////////////////////////////////
//
// GuiVSpaceDialog
//
/////////////////////////////////////////////////////////////////////


GuiVSpaceDialog::GuiVSpaceDialog(GuiVSpace * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));

	connect(spacingCO, SIGNAL(highlighted(const QString &)),
		this, SLOT(change_adaptor()));
	connect(valueLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(spacingCO, SIGNAL(activated(int)),
		this, SLOT(enableCustom(int)));
	connect(keepCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(unitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));

	valueLE->setValidator(unsignedLengthValidator(valueLE));
}


void GuiVSpaceDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiVSpaceDialog::change_adaptor()
{
	form_->changed();
}


void GuiVSpaceDialog::enableCustom(int selection)
{
	bool const enable = selection == 5;
	valueLE->setEnabled(enable);
	unitCO->setEnabled(enable);
}


/////////////////////////////////////////////////////////////////////
//
// GuiVSpace
//
/////////////////////////////////////////////////////////////////////

static void setWidgetsFromVSpace(VSpace const & space,
			  QComboBox * spacing,
			  QLineEdit * value,
			  LengthCombo * unit,
			  QCheckBox * keep)
{
	int item = 0;
	switch (space.kind()) {
	case VSpace::DEFSKIP:
		item = 0;
		break;
	case VSpace::SMALLSKIP:
		item = 1;
		break;
	case VSpace::MEDSKIP:
		item = 2;
		break;
	case VSpace::BIGSKIP:
		item = 3;
		break;
	case VSpace::VFILL:
		item = 4;
		break;
	case VSpace::LENGTH:
		item = 5;
		break;
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
			    QLineEdit * value,
			    LengthCombo * unit,
			    bool keep)
{
	VSpace space;

	switch (spacing) {
	case 0:
		space = VSpace(VSpace::DEFSKIP);
		break;
	case 1:
		space = VSpace(VSpace::SMALLSKIP);
		break;
	case 2:
		space = VSpace(VSpace::MEDSKIP);
		break;
	case 3:
		space = VSpace(VSpace::BIGSKIP);
		break;
	case 4:
		space = VSpace(VSpace::VFILL);
		break;
	case 5:
		space = VSpace(GlueLength(widgetsToLength(value, unit)));
		break;
	}

	space.setKeep(keep);
	return space;
}


typedef QController<ControlVSpace, GuiView<GuiVSpaceDialog> > VSpaceBase;

GuiVSpace::GuiVSpace(Dialog & parent)
	: VSpaceBase(parent, _("Vertical Space Settings"))
{}


void GuiVSpace::build_dialog()
{
	// the tabbed folder
	dialog_.reset(new GuiVSpaceDialog(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->spacingCO);
	bcview().addReadOnly(dialog_->valueLE);
	bcview().addReadOnly(dialog_->unitCO);
	bcview().addReadOnly(dialog_->keepCB);

	// initialize the length validator
	addCheckedLineEdit(bcview(), dialog_->valueLE, dialog_->valueL);

	// remove the %-items from the unit choice
	dialog_->unitCO->noPercents();
}


void GuiVSpace::apply()
{
	// spacing
	// If a vspace choice is "Length" but there's no text in
	// the input field, do not insert a vspace at all.
	if (dialog_->spacingCO->currentIndex() == 5
	    && dialog_->valueLE->text().isEmpty())
		return;

	VSpace const space =
		setVSpaceFromWidgets(dialog_->spacingCO->currentIndex(),
				     dialog_->valueLE,
				     dialog_->unitCO,
				     dialog_->keepCB->isChecked());

	controller().params() = space;
}


void GuiVSpace::update_contents()
{
	setWidgetsFromVSpace(controller().params(),
			     dialog_->spacingCO,
			     dialog_->valueLE,
			     dialog_->unitCO,
			     dialog_->keepCB);
}

} // namespace frontend
} // namespace lyx


#include "GuiVSpace_moc.cpp"
