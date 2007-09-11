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
#include "lengthcommon.h"
#include "LyXRC.h" // to set the default length values
#include "Validator.h"

#include "insets/InsetBox.h"

#include "support/lstrings.h"

#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>


using std::string;

namespace lyx {
namespace frontend {

GuiBoxDialog::GuiBoxDialog(LyXView & lv)
	: GuiDialog(lv, "box")
{
	setupUi(this);
	setViewTitle(_("Box Settings"));
	setController(new ControlBox(*this));

	// fill the box type choice
	box_gui_tokens(ids_, gui_names_);
	for (unsigned int i = 0; i < gui_names_.size(); ++i)
		typeCO->addItem(toqstr(gui_names_[i]));

	// add the special units to the height choice
	// width needs different handling
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	for (unsigned int i = 1; i < gui_names_spec_.size(); ++i)
		heightUnitsLC->addItem(toqstr(gui_names_spec_[i]));

	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

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

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);

	bc().addReadOnly(typeCO);
	bc().addReadOnly(innerBoxCO);
	bc().addReadOnly(valignCO);
	bc().addReadOnly(ialignCO);
	bc().addReadOnly(halignCO);
	bc().addReadOnly(widthED);
	bc().addReadOnly(heightED);
	bc().addReadOnly(widthUnitsLC);
	bc().addReadOnly(heightUnitsLC);

	bc().setRestore(restorePB);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	// initialize the length validator
	bc().addCheckedLineEdit(widthED, widthLA);
	bc().addCheckedLineEdit(heightED, heightLA);
}


ControlBox & GuiBoxDialog::controller()
{
	return static_cast<ControlBox &>(GuiDialog::controller());
}


void GuiBoxDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiBoxDialog::change_adaptor()
{
	changed();
}


void GuiBoxDialog::innerBoxChanged(const QString & str)
{
	bool const ibox = (str != qt_("None"));
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	heightED->setEnabled(ibox);
	heightUnitsLC->setEnabled(ibox);
	setSpecial(ibox);
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
		setSpecial(true);
	}
	int itype = innerBoxCO->currentIndex();
	setInnerType(frameless, itype);
}


void GuiBoxDialog::restoreClicked()
{
	setInnerType(true, 2);
	widthED->setText("100");
	widthUnitsLC->setCurrentItem(Length::PCW);
	heightED->setText("1");
	for (int j = 0; j < heightUnitsLC->count(); j++) {
		if (heightUnitsLC->itemText(j) == qt_("Total Height"))
			heightUnitsLC->setCurrentItem(j);
	}
}


void GuiBoxDialog::updateContents()
{
	string type(controller().params().type);
	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			typeCO->setCurrentIndex(i);
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
	valignCO->setCurrentIndex(string("tcb").find(c, 0));
	c = controller().params().inner_pos;
	ialignCO->setCurrentIndex(string("tcbs").find(c, 0));
	c = controller().params().hor_pos;
	halignCO->setCurrentIndex(string("lcrs").find(c, 0));

	bool ibox = controller().params().inner_box;
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	setSpecial(ibox);

	Length::UNIT default_unit =
		(lyxrc.default_papersize > 3) ? Length::CM : Length::IN;

	lengthToWidgets(widthED, widthUnitsLC,
		(controller().params().width).asString(), default_unit);

	string const special = controller().params().special;
	if (!special.empty() && special != "none") {
		QString spc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); i++) {
			if (special == ids_spec_[i])
				spc = toqstr(gui_names_spec_[i].c_str());
		}
		for (int j = 0; j < widthUnitsLC->count(); j++) {
			if (widthUnitsLC->itemText(j) == spc)
				widthUnitsLC->setCurrentIndex(j);
		}
	}

	lengthToWidgets(heightED, heightUnitsLC,
		(controller().params().height).asString(), default_unit);

	string const height_special = controller().params().height_special;
	if (!height_special.empty() && height_special != "none") {
		QString hspc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); i++) {
			if (height_special == ids_spec_[i]) {
				hspc = toqstr(gui_names_spec_[i].c_str());
			}
		}
		for (int j = 0; j < heightUnitsLC->count(); j++) {
			if (heightUnitsLC->itemText(j) == hspc) {
				heightUnitsLC->setCurrentIndex(j);
			}
		}
	}

	heightED->setEnabled(ibox);
	heightUnitsLC->setEnabled(ibox);
}


void GuiBoxDialog::applyView()
{
	controller().params().type =
		ids_[typeCO->currentIndex()];

	controller().params().inner_box =
		innerBoxCO->currentText() != qt_("None");
	controller().params().use_parbox =
		innerBoxCO->currentText() ==  qt_("Parbox");

	controller().params().pos =
		"tcb"[valignCO->currentIndex()];
	controller().params().inner_pos =
		"tcbs"[ialignCO->currentIndex()];
	controller().params().hor_pos =
		"lcrs"[halignCO->currentIndex()];

	int i = 0;
	bool spec = false;
	QString special = widthUnitsLC->currentText();
	QString value = widthED->text();
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
		width = widgetsToLength(widthED, widthUnitsLC);

	controller().params().width = Length(width);

	i = 0;
	spec = false;
	special = heightUnitsLC->currentText();
	value = heightED->text();
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
	if (spec  && !isValidLength(fromqstr(heightED->text()))) {
		height = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		height += "in";
	} else
		height = widgetsToLength(heightED, heightUnitsLC);

	controller().params().height = Length(height);
}


void GuiBoxDialog::setSpecial(bool ibox)
{
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	// check if the widget contains the special units
	int count = widthUnitsLC->count();
	bool has_special = false;
	for (int i = 0; i < count; i++)
		if (widthUnitsLC->itemText(i).contains(qt_("Total Height")) > 0)
			has_special = true;
	// insert 'em if needed...
	if (!ibox && !has_special) {
		for (unsigned int i = 1; i < gui_names_spec_.size(); i++)
			widthUnitsLC->addItem(toqstr(gui_names_spec_[i]));
	// ... or remove 'em if needed
	} else if (ibox && has_special) {
		widthUnitsLC->clear();
		for (int i = 0; i < num_units; i++)
			widthUnitsLC->addItem(qt_(unit_name_gui[i]));
	}
}


void GuiBoxDialog::setInnerType(bool frameless, int i)
{
	// with "frameless" boxes, inner box is mandatory (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo
	if (frameless) {
		innerBoxCO->clear();
		innerBoxCO->addItem(qt_("Parbox"));
		innerBoxCO->addItem(qt_("Minipage"));
		innerBoxCO->setCurrentIndex(i - 1);
	} else {
		if (innerBoxCO->count() == 2)
			++i;
		innerBoxCO->clear();
		innerBoxCO->addItem(qt_("None"));
		innerBoxCO->addItem(qt_("Parbox"));
		innerBoxCO->addItem(qt_("Minipage"));
		innerBoxCO->setCurrentIndex(i);
	}
}

} // namespace frontend
} // namespace lyx


#include "GuiBox_moc.cpp"
