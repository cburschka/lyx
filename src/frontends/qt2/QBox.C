/**
 * \file QBox.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna (Minipage stuff)
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QBox.h"

#include "checkedwidgets.h"
#include "lengthcombo.h"
#include "QBoxDialog.h"
#include "qt_helpers.h"
#include "Qt2BC.h"

#include "lengthcommon.h"
#include "lyxrc.h" // to set the default length values

#include "controllers/ControlBox.h"
#include "controllers/helper_funcs.h"

#include "insets/insetbox.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbuttongroup.h>

#include <vector>

using lyx::support::getStringFromVector;
using lyx::support::isStrDbl;
using lyx::support::subst;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlBox, QView<QBoxDialog> > base_class;

QBox::QBox(Dialog & parent)
	: base_class(parent, _("LyX: Box Settings"))
{}


void QBox::build_dialog()
{
	dialog_.reset(new QBoxDialog(this));

	// fill the box type choice
	box_gui_tokens(ids_, gui_names_);
	for (unsigned int i = 0; i < gui_names_.size(); ++i)
		dialog_->typeCO->insertItem(toqstr(gui_names_[i]));

	// add the special units to the height choice
	// width needs different handling
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	for (unsigned int i = 1; i < gui_names_spec_.size(); ++i) {
		dialog_->heightUnitsLC->insertItem(toqstr(gui_names_spec_[i]));
	}

	bcview().addReadOnly(dialog_->typeCO);
	bcview().addReadOnly(dialog_->innerBoxCO);
	bcview().addReadOnly(dialog_->valignCO);
	bcview().addReadOnly(dialog_->ialignCO);
	bcview().addReadOnly(dialog_->halignCO);
	bcview().addReadOnly(dialog_->widthED);
	bcview().addReadOnly(dialog_->heightED);
	bcview().addReadOnly(dialog_->widthUnitsLC);
	bcview().addReadOnly(dialog_->heightUnitsLC);

	bcview().setRestore(dialog_->restorePB);
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	
	// initialize the length validator
	addCheckedLineEdit(bcview(), dialog_->widthED, dialog_->widthLA);
	addCheckedLineEdit(bcview(), dialog_->heightED, dialog_->heightLA);
}


void QBox::update_contents()
{
	string type(controller().params().type);
	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			dialog_->typeCO->setCurrentItem(i);
	}

	// default: minipage
	unsigned int inner_type = 2;
	if (!controller().params().inner_box)
		// none
		inner_type = 0;
	if (controller().params().use_parbox)
		// parbox
		inner_type = 1;
	bool frameless = (controller().params().type == "Frameless");
	setInnerType(frameless, inner_type);

	char c = controller().params().pos;
	dialog_->valignCO->setCurrentItem(string("tcb").find(c, 0));
	c = controller().params().inner_pos;
	dialog_->ialignCO->setCurrentItem(string("tcbs").find(c, 0));
	c = controller().params().hor_pos;
	dialog_->halignCO->setCurrentItem(string("lcrs").find(c, 0));

	bool ibox = controller().params().inner_box;
	dialog_->valignCO->setEnabled(ibox);
	dialog_->ialignCO->setEnabled(ibox);
	dialog_->halignCO->setEnabled(!ibox);
	setSpecial(ibox);

	LyXLength::UNIT default_unit =
		(lyxrc.default_papersize > 3) ? LyXLength::CM : LyXLength::IN;

	lengthToWidgets(dialog_->widthED, dialog_->widthUnitsLC,
		(controller().params().width).asString(), default_unit);

	string const special(controller().params().special);
	if (!special.empty() && special != "none") {
		QString spc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); i++) {
			if (special == ids_spec_[i])
				spc = toqstr(gui_names_spec_[i].c_str());
		}
		for (int j = 0; j < dialog_->widthUnitsLC->count(); j++) {
			if (dialog_->widthUnitsLC->text(j) == spc)
				dialog_->widthUnitsLC->setCurrentItem(j);
		}
	}

	lengthToWidgets(dialog_->heightED, dialog_->heightUnitsLC,
		(controller().params().height).asString(), default_unit);

	string const height_special(controller().params().height_special);
	if (!height_special.empty() && height_special != "none") {
		QString hspc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); i++) {
			if (height_special == ids_spec_[i]) {
				hspc = toqstr(gui_names_spec_[i].c_str());
			}
		}
		for (int j = 0; j < dialog_->heightUnitsLC->count(); j++) {
			if (dialog_->heightUnitsLC->text(j) == hspc) {
				dialog_->heightUnitsLC->setCurrentItem(j);
			}
		}
	}

	dialog_->heightED->setEnabled(ibox);
	dialog_->heightUnitsLC->setEnabled(ibox);
}


void QBox::apply()
{
	controller().params().type =
		ids_[dialog_->typeCO->currentItem()];

	controller().params().inner_box =
		dialog_->innerBoxCO->currentText() != qt_("None");
	controller().params().use_parbox =
		dialog_->innerBoxCO->currentText() ==  qt_("Parbox");

	controller().params().pos =
		"tcb"[dialog_->valignCO->currentItem()];
	controller().params().inner_pos =
		"tcbs"[dialog_->ialignCO->currentItem()];
	controller().params().hor_pos =
		"lcrs"[dialog_->halignCO->currentItem()];

	int i = 0;
	bool spec = false;
	QString special = dialog_->widthUnitsLC->currentText();
	QString value = dialog_->widthED->text();
	if (special == qt_("Height")) {
		i = 1;
		spec = true;
	} else if (special == qt_("Depth")) {
		i = 2;
		spec = true;
	} else if (special == qt_("Total Height")) {
		i = 3;
		spec = true;
	} else if (special == qt_("Width")) {
		i = 4;
		spec = true;
	} 
	// the user might insert a non-special value in the line edit
	if (isValidLength(fromqstr(value))) {
		i = 0;
		spec = false;
	}
	controller().params().special = ids_spec_[i];

	string width;
	if (spec) {
		width = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		width += "in";
	} else
		width = widgetsToLength(dialog_->widthED, dialog_->widthUnitsLC);

	controller().params().width = LyXLength(width);

	i = 0;
	spec = false;
	special = dialog_->heightUnitsLC->currentText();
	value = dialog_->heightED->text();
	if (special == qt_("Height")) {
		i = 1;
		spec = true;
	} else if (special == qt_("Depth")) {
		i = 2;
		spec = true;
	} else if (special == qt_("Total Height")) {
		i = 3;
		spec = true;
	} else if (special == qt_("Width")) {
		i = 4;
		spec = true;
	}
	// the user might insert a non-special value in the line edit
	if (isValidLength(fromqstr(value))) {
		i = 0;
		spec = false;
	}
	controller().params().height_special = ids_spec_[i];

	string height;
	if (spec  && !isValidLength(fromqstr(dialog_->heightED->text()))) {
		height = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		height += "in";
	} else
		height = widgetsToLength(dialog_->heightED, dialog_->heightUnitsLC);

	controller().params().height = LyXLength(height);
}


void QBox::setSpecial(bool ibox)
{
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	// check if the widget contains the special units
	int count = dialog_->widthUnitsLC->count();
	bool has_special = false;
	for (int i = 0; i < count; i++)
		if (dialog_->widthUnitsLC->text(i).contains(qt_("Total Height")) > 0)
			has_special = true;
	// insert 'em if needed...
	if (!ibox && !has_special) {
		for (unsigned int i = 1; i < gui_names_spec_.size(); i++)
			dialog_->widthUnitsLC->insertItem(toqstr(gui_names_spec_[i]));
	// ... or remove 'em if needed
	} else if (ibox && has_special) {
		dialog_->widthUnitsLC->clear();
		for (int i = 0; i < num_units; i++)
			dialog_->widthUnitsLC->insertItem(unit_name_gui[i]);
	}
}


void QBox::setInnerType(bool frameless, int i)
{
	// with "frameless" boxes, inner box is mandatory (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo
	if (frameless) {
		dialog_->innerBoxCO->clear();
		dialog_->innerBoxCO->insertItem(qt_("Parbox"));
		dialog_->innerBoxCO->insertItem(qt_("Minipage"));
		dialog_->innerBoxCO->setCurrentItem(i - 1);
	} else {
		if (dialog_->innerBoxCO->count() == 2)
			i += 1;
		dialog_->innerBoxCO->clear();
		dialog_->innerBoxCO->insertItem(qt_("None"));
		dialog_->innerBoxCO->insertItem(qt_("Parbox"));
		dialog_->innerBoxCO->insertItem(qt_("Minipage"));
		dialog_->innerBoxCO->setCurrentItem(i);
	}
}

} // namespace frontend
} // namespace lyx
