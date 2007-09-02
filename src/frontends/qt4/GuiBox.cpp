/**
 * \file GuiBox.cpp
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

#include "GuiBox.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Qt2BC.h"
#include "lengthcommon.h"
#include "LyXRC.h" // to set the default length values
#include "Validator.h"

#include "controllers/ControlBox.h"
#include "controllers/frontend_helpers.h"

#include "insets/InsetBox.h"

#include "support/lstrings.h"

#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>


using lyx::support::getStringFromVector;
using lyx::support::isStrDbl;
using lyx::support::subst;
using std::string;


namespace lyx {
namespace frontend {

//////////////////////////////////////////////////////////////////
//
// GuiBoxDialog
//
//////////////////////////////////////////////////////////////////

GuiBoxDialog::GuiBoxDialog(GuiBox * form)
	: form_(form)
{
	setupUi(this);
	connect(restorePB, SIGNAL(clicked()), form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(widthUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(const QString &)),
		this, SLOT(change_adaptor()));
	connect(heightED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(heightUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT) ),
		this, SLOT(change_adaptor()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(restoreClicked()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
	connect(halignCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(ialignCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(innerBoxCO, SIGNAL(activated(const QString&)),
		this, SLOT(innerBoxChanged(const QString &)));
	connect(innerBoxCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	heightED->setValidator(unsignedLengthValidator(heightED));
	widthED->setValidator(unsignedLengthValidator(widthED));
}


void GuiBoxDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiBoxDialog::change_adaptor()
{
	form_->changed();
}


void GuiBoxDialog::innerBoxChanged(const QString & str)
{
	bool const ibox = (str != qt_("None"));
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	heightED->setEnabled(ibox);
	heightUnitsLC->setEnabled(ibox);
	form_->setSpecial(ibox);
}


void GuiBoxDialog::typeChanged(int index)
{
	bool const frameless = (index == 0);
	if (frameless) {
		valignCO->setEnabled(true);
		ialignCO->setEnabled(true);
		halignCO->setEnabled(false);
		heightED->setEnabled(true);
		heightUnitsLC->setEnabled(true);
		form_->setSpecial(true);
	}
	int itype = innerBoxCO->currentIndex();
	form_->setInnerType(frameless, itype);
}


void GuiBoxDialog::restoreClicked()
{
	form_->setInnerType(true, 2);
	widthED->setText("100");
	widthUnitsLC->setCurrentItem(Length::PCW);
	heightED->setText("1");
	for (int j = 0; j < heightUnitsLC->count(); j++) {
		if (heightUnitsLC->itemText(j) == qt_("Total Height"))
			heightUnitsLC->setCurrentItem(j);
	}
}


//////////////////////////////////////////////////////////////////
//
// GuiBox
//
//////////////////////////////////////////////////////////////////


GuiBox::GuiBox(Dialog & parent)
	: GuiView<GuiBoxDialog>(parent, _("Box Settings"))
{}


void GuiBox::build_dialog()
{
	dialog_.reset(new GuiBoxDialog(this));

	// fill the box type choice
	box_gui_tokens(ids_, gui_names_);
	for (unsigned int i = 0; i < gui_names_.size(); ++i)
		dialog_->typeCO->addItem(toqstr(gui_names_[i]));

	// add the special units to the height choice
	// width needs different handling
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	for (unsigned int i = 1; i < gui_names_spec_.size(); ++i)
		dialog_->heightUnitsLC->addItem(toqstr(gui_names_spec_[i]));

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


void GuiBox::update_contents()
{
	string type(controller().params().type);
	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			dialog_->typeCO->setCurrentIndex(i);
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
	dialog_->valignCO->setCurrentIndex(string("tcb").find(c, 0));
	c = controller().params().inner_pos;
	dialog_->ialignCO->setCurrentIndex(string("tcbs").find(c, 0));
	c = controller().params().hor_pos;
	dialog_->halignCO->setCurrentIndex(string("lcrs").find(c, 0));

	bool ibox = controller().params().inner_box;
	dialog_->valignCO->setEnabled(ibox);
	dialog_->ialignCO->setEnabled(ibox);
	dialog_->halignCO->setEnabled(!ibox);
	setSpecial(ibox);

	Length::UNIT default_unit =
		(lyxrc.default_papersize > 3) ? Length::CM : Length::IN;

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
			if (dialog_->widthUnitsLC->itemText(j) == spc)
				dialog_->widthUnitsLC->setCurrentIndex(j);
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
			if (dialog_->heightUnitsLC->itemText(j) == hspc) {
				dialog_->heightUnitsLC->setCurrentIndex(j);
			}
		}
	}

	dialog_->heightED->setEnabled(ibox);
	dialog_->heightUnitsLC->setEnabled(ibox);
}


void GuiBox::apply()
{
	controller().params().type =
		ids_[dialog_->typeCO->currentIndex()];

	controller().params().inner_box =
		dialog_->innerBoxCO->currentText() != qt_("None");
	controller().params().use_parbox =
		dialog_->innerBoxCO->currentText() ==  qt_("Parbox");

	controller().params().pos =
		"tcb"[dialog_->valignCO->currentIndex()];
	controller().params().inner_pos =
		"tcbs"[dialog_->ialignCO->currentIndex()];
	controller().params().hor_pos =
		"lcrs"[dialog_->halignCO->currentIndex()];

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

	controller().params().width = Length(width);

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

	controller().params().height = Length(height);
}


void GuiBox::setSpecial(bool ibox)
{
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	// check if the widget contains the special units
	int count = dialog_->widthUnitsLC->count();
	bool has_special = false;
	for (int i = 0; i < count; i++)
		if (dialog_->widthUnitsLC->itemText(i).contains(qt_("Total Height")) > 0)
			has_special = true;
	// insert 'em if needed...
	if (!ibox && !has_special) {
		for (unsigned int i = 1; i < gui_names_spec_.size(); i++)
			dialog_->widthUnitsLC->addItem(toqstr(gui_names_spec_[i]));
	// ... or remove 'em if needed
	} else if (ibox && has_special) {
		dialog_->widthUnitsLC->clear();
		for (int i = 0; i < num_units; i++)
			dialog_->widthUnitsLC->addItem(qt_(unit_name_gui[i]));
	}
}


void GuiBox::setInnerType(bool frameless, int i)
{
	// with "frameless" boxes, inner box is mandatory (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo
	if (frameless) {
		dialog_->innerBoxCO->clear();
		dialog_->innerBoxCO->addItem(qt_("Parbox"));
		dialog_->innerBoxCO->addItem(qt_("Minipage"));
		dialog_->innerBoxCO->setCurrentIndex(i - 1);
	} else {
		if (dialog_->innerBoxCO->count() == 2)
			i += 1;
		dialog_->innerBoxCO->clear();
		dialog_->innerBoxCO->addItem(qt_("None"));
		dialog_->innerBoxCO->addItem(qt_("Parbox"));
		dialog_->innerBoxCO->addItem(qt_("Minipage"));
		dialog_->innerBoxCO->setCurrentIndex(i);
	}
}

} // namespace frontend
} // namespace lyx


#include "GuiBox_moc.cpp"
