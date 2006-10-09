/**
 * \file QVSpace.C
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

#include "QVSpace.h"
#include "QVSpaceDialog.h"
#include "Qt2BC.h"

#include "checkedwidgets.h"
#include "lengthcombo.h"
#include "qt_helpers.h"

#include "lyxrc.h" // to set the default length values
#include "Spacing.h"
#include "vspace.h"

#include "controllers/ControlVSpace.h"
#include "controllers/helper_funcs.h"

#include "support/lstrings.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>


using std::string;

namespace lyx {
namespace frontend {

namespace {

void setWidgetsFromVSpace(VSpace const & space,
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
	spacing->setCurrentItem(item);
	keep->setChecked(space.keep());

	LyXLength::UNIT default_unit =
			(lyxrc.default_papersize > 3) ? LyXLength::CM : LyXLength::IN;
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


VSpace setVSpaceFromWidgets(int spacing,
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
		space = VSpace(LyXGlueLength(
				      widgetsToLength(value, unit)));
		break;
	}

	space.setKeep(keep);
	return space;
}

} // namespace anon


typedef QController<ControlVSpace, QView<QVSpaceDialog> > base_class;

QVSpace::QVSpace(Dialog & parent)
	: base_class(parent, _("Vertical Space Settings"))
{}


void QVSpace::build_dialog()
{
	// the tabbed folder
	dialog_.reset(new QVSpaceDialog(this));

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


void QVSpace::apply()
{
	// spacing
	// If a vspace choice is "Length" but there's no text in
	// the input field, do not insert a vspace at all.
	if (dialog_->spacingCO->currentItem() == 5
	    && dialog_->valueLE->text().isEmpty())
		return;

	VSpace const space =
		setVSpaceFromWidgets(dialog_->spacingCO->currentItem(),
				     dialog_->valueLE,
				     dialog_->unitCO,
				     dialog_->keepCB->isChecked());

	controller().params() = space;
}


void QVSpace::update_contents()
{
	setWidgetsFromVSpace(controller().params(),
			     dialog_->spacingCO,
			     dialog_->valueLE,
			     dialog_->unitCO,
			     dialog_->keepCB);
}

} // namespace frontend
} // namespace lyx
