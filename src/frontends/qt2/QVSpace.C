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

#include "debug.h"
#include "ControlVSpace.h"
#include "QVSpace.h"
#include "QVSpaceDialog.h"
#include "Qt2BC.h"
#include "lyxrc.h" // to set the deafult length values
#include "qt_helpers.h"
#include "helper_funcs.h"

#include "Spacing.h"
#include "vspace.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

using lyx::support::contains_functor;
using lyx::support::isStrDbl;
using lyx::support::subst;
using lyx::support::trim;

using std::bind2nd;
using std::remove_if;

using std::vector;
using std::string;


namespace {

void setWidgetsFromVSpace(VSpace const & space,
			  QComboBox * spacing,
			  QLineEdit * value,
			  QComboBox * unit,
			  QCheckBox * keep, vector<string> units_)
{
	value->setText("");
	value->setEnabled(false);
	unit->setEnabled(false);

	int item = 0;
	switch (space.kind()) {
	case VSpace::NONE:
		item = 0;
		break;
	case VSpace::DEFSKIP:
		item = 1;
		break;
	case VSpace::SMALLSKIP:
		item = 2;
		break;
	case VSpace::MEDSKIP:
		item = 3;
		break;
	case VSpace::BIGSKIP:
		item = 4;
		break;
	case VSpace::VFILL:
		item = 5;
		break;
	case VSpace::LENGTH:
		item = 6;
		value->setEnabled(true);
		unit->setEnabled(true);
		string length = space.length().asString();
		string const default_unit =
			(lyxrc.default_papersize > 3) ? "cm" : "in";
		string supplied_unit = default_unit;
		LyXLength len(length);
		if ((isValidLength(length)
		     || isStrDbl(length)) && !len.zero()) {
			length = tostr(len.value());
			supplied_unit = subst(stringFromUnit(len.unit()),
					      "%", "%%");
		}

		int unit_item = 0;
		int i = 0;
		for (vector<string>::const_iterator it = units_.begin();
		     it != units_.end(); ++it) {
			if (*it == default_unit) {
				unit_item = i;
			}
			if (*it == supplied_unit) {
				unit_item = i;
				break;
			}
			i += 1;
		}
		value->setText(toqstr(length));
		unit->setCurrentItem(unit_item);
		break;
	}
	spacing->setCurrentItem(item);
	keep->setChecked(space.keep());
}


VSpace setVSpaceFromWidgets(int spacing,
			    string value,
			    string unit,
			    bool keep)
{
	VSpace space;

	switch (spacing) {
	case 0:
		space = VSpace(VSpace::NONE);
		break;
	case 1:
		space = VSpace(VSpace::DEFSKIP);
		break;
	case 2:
		space = VSpace(VSpace::SMALLSKIP);
		break;
	case 3:
		space = VSpace(VSpace::MEDSKIP);
		break;
	case 4:
		space = VSpace(VSpace::BIGSKIP);
		break;
	case 5:
		space = VSpace(VSpace::VFILL);
		break;
	case 6:
		string s;
		string const length = trim(value);
		if (isValidGlueLength(length)) {
			s = length;
		} else if (!length.empty()){
			string u = trim(unit);
			u = subst(u, "%%", "%");
			s = length + u;
		}
		space = VSpace(LyXGlueLength(s));
		break;
	}

	space.setKeep(keep);

	return space;
}

} // namespace anon


typedef QController<ControlVSpace, QView<QVSpaceDialog> > base_class;

QVSpace::QVSpace(Dialog & parent)
	: base_class(parent, _("LyX: VSpace Settings"))
{}


void QVSpace::build_dialog()
{
	// the tabbed folder
	dialog_.reset(new QVSpaceDialog(this));

	// Create the contents of the unit choices
	// Don't include the "%" terms...
	units_ = getLatexUnits();
	vector<string>::iterator del =
		remove_if(units_.begin(), units_.end(),
			  bind2nd(contains_functor(), "%"));
	units_.erase(del, units_.end());

	for (vector<string>::const_iterator it = units_.begin();
		it != units_.end(); ++it) {
		dialog_->unitCO->insertItem(toqstr(*it));
	}

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().setRestore(dialog_->restorePB);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->spacingCO);
	bcview().addReadOnly(dialog_->valueLE);
	bcview().addReadOnly(dialog_->unitCO);
	bcview().addReadOnly(dialog_->keepCB);
}


void QVSpace::apply()
{
	// spacing
	// If a vspace choice is "Length" but there's no text in
	// the input field, reset the choice to "None".
	if (dialog_->spacingCO->currentItem() == 6
	    && dialog_->valueLE->text().isEmpty())
		dialog_->spacingCO->setCurrentItem(0);

	VSpace const space =
		setVSpaceFromWidgets(dialog_->spacingCO->currentItem(),
				     fromqstr(dialog_->valueLE->text()),
				     fromqstr(dialog_->unitCO->currentText()),
				     dialog_->keepCB->isChecked());

	controller().params() = space;
}


void QVSpace::update_contents()
{
	setWidgetsFromVSpace(controller().params(),
			     dialog_->spacingCO,
			     dialog_->valueLE,
			     dialog_->unitCO,
			     dialog_->keepCB, units_);
}
