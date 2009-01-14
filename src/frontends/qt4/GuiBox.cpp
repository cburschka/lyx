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

#include "FuncRequest.h"
#include "LengthCombo.h"
#include "Length.h"
#include "LyXRC.h" // to set the default length values
#include "qt_helpers.h"
#include "Validator.h"

#include "insets/InsetBox.h"

#include "support/gettext.h"
#include "support/foreach.h"
#include "support/lstrings.h"

#include <QPushButton>
#include <QLineEdit>

#ifdef IN
#undef IN
#endif

using namespace std;


namespace lyx {
namespace frontend {

static QStringList boxGuiIds()
{
	return QStringList()
		<< "Frameless" << "Boxed"
		<< "ovalbox" << "Ovalbox"
		<< "Shadowbox" << "Shaded"
		<< "Doublebox";
}


static QStringList boxGuiNames()
{
	return QStringList()
		<< qt_("No frame") << qt_("Simple rectangular frame")
		<< qt_("Oval frame, thin") << qt_("Oval frame, thick")
		<< qt_("Drop shadow") << qt_("Shaded background")
		<< qt_("Double rectangular frame");
}


static QStringList boxGuiSpecialLengthIds()
{
	return QStringList() << "none" << "height" << "depth"
		<< "totalheight" << "width";
}


static QStringList boxGuiSpecialLengthNames()
{
	return QStringList() << qt_("None") << qt_("Height") << qt_("Depth")
		<< qt_("Total Height") << qt_("Width");
}


GuiBox::GuiBox(GuiView & lv)
	: GuiDialog(lv, "box", qt_("Box Settings")), params_("")
{
	setupUi(this);

	// fill the box type choice
	ids_ = boxGuiIds();
	gui_names_ = boxGuiNames();
	foreach (QString const & str, gui_names_)
		typeCO->addItem(str);

	// add the special units to the height choice
	// width needs different handling
	ids_spec_ = boxGuiSpecialLengthIds();
	gui_names_spec_ = boxGuiSpecialLengthNames();
	foreach (QString const & str, gui_names_spec_)
		heightUnitsLC->addItem(str);

	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(widthUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(QString)),
		this, SLOT(change_adaptor()));
	connect(heightCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(heightED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(heightUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(restoreClicked()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
	connect(halignCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(ialignCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(innerBoxCO, SIGNAL(activated(QString)),
		this, SLOT(innerBoxChanged(QString)));
	connect(innerBoxCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pagebreakCB, SIGNAL(stateChanged(int)),
		this, SLOT(pagebreakClicked()));

	heightED->setValidator(unsignedLengthValidator(heightED));
	widthED->setValidator(unsignedLengthValidator(widthED));

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);

	bc().addReadOnly(typeCO);
	bc().addReadOnly(innerBoxCO);
	bc().addReadOnly(valignCO);
	bc().addReadOnly(ialignCO);
	bc().addReadOnly(halignCO);
	bc().addReadOnly(widthED);
	bc().addReadOnly(widthUnitsLC);
	bc().addReadOnly(heightCB);
	bc().addReadOnly(heightED);
	bc().addReadOnly(heightUnitsLC);
	bc().addReadOnly(pagebreakCB);

	bc().setRestore(restorePB);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	// initialize the length validator
	bc().addCheckedLineEdit(widthED, widthLA);
	bc().addCheckedLineEdit(heightED, heightCB);
}


void GuiBox::change_adaptor()
{
	changed();
}


void GuiBox::innerBoxChanged(QString const & str)
{
	bool const ibox = (str != qt_("None"));
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	heightCB->setEnabled(ibox);
	pagebreakCB->setEnabled(!ibox && typeCO->currentIndex() == 1);
	heightED->setEnabled(heightCB->checkState() == Qt::Checked && ibox);
	heightUnitsLC->setEnabled(heightCB->checkState() == Qt::Checked && ibox);
	setSpecial(ibox);
}


void GuiBox::typeChanged(int index)
{
	bool const frameless = (index == 0);
	if (frameless) {
		valignCO->setEnabled(true);
		ialignCO->setEnabled(true);
		halignCO->setEnabled(false);
		heightCB->setEnabled(true);
		heightED->setEnabled(heightCB->checkState() == Qt::Checked);
		heightUnitsLC->setEnabled(heightCB->checkState() == Qt::Checked);
		setSpecial(true);
	}
	if (index != 1)
		pagebreakCB->setChecked(false);
	int itype = innerBoxCO->currentIndex();
	if (innerBoxCO->count() == 2)
		++itype;
	pagebreakCB->setEnabled(index == 1 && itype == 0);
	widthED->setEnabled(index != 5);
	widthUnitsLC->setEnabled(index != 5);
	setInnerType(frameless, itype);
}


void GuiBox::restoreClicked()
{
	setInnerType(true, 2);
	widthED->setText("100");
	widthUnitsLC->setCurrentItem(Length::PCW);
	heightCB->setCheckState(Qt::Checked);
	heightED->setText("1");
	for (int i = 0; i != heightUnitsLC->count(); ++i) {
		if (heightUnitsLC->itemText(i) == qt_("Total Height"))
			heightUnitsLC->setCurrentItem(i);
	}
}


void GuiBox::pagebreakClicked()
{
	bool pbreak = (pagebreakCB->checkState() == Qt::Checked);
	innerBoxCO->setEnabled(!pbreak);
	widthED->setEnabled(!pbreak);
	widthUnitsLC->setEnabled(!pbreak);
	if (pbreak) {
		valignCO->setEnabled(false);
		ialignCO->setEnabled(false);
		halignCO->setEnabled(false);
		heightCB->setEnabled(false);
		heightED->setEnabled(false);
		heightUnitsLC->setEnabled(false);
		setSpecial(false);
	} else {
		typeChanged(typeCO->currentIndex());
	}
	change_adaptor();
}


void GuiBox::updateContents()
{
	QString type = toqstr(params_.type);
	if (type == "Framed") {
		pagebreakCB->setChecked(true);
		type = "Boxed";
	} else {
		pagebreakCB->setChecked(false);
	}

	pagebreakCB->setEnabled(type == "Boxed" && !params_.inner_box);

	for (int i = 0; i != gui_names_.size(); ++i) {
		if (type == ids_[i])
			typeCO->setCurrentIndex(i);
	}

	// default: minipage
	int inner_type = 2;
	if (!params_.inner_box)
		// none
		inner_type = 0;
	if (params_.use_parbox)
		// parbox
		inner_type = 1;
	bool frameless = (params_.type == "Frameless");
	setInnerType(frameless, inner_type);

	char c = params_.pos;
	valignCO->setCurrentIndex(string("tcb").find(c, 0));
	c = params_.inner_pos;
	ialignCO->setCurrentIndex(string("tcbs").find(c, 0));
	c = params_.hor_pos;
	halignCO->setCurrentIndex(string("lcrs").find(c, 0));

	bool ibox = params_.inner_box;
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	setSpecial(ibox);

	Length::UNIT default_unit =
		(lyxrc.default_papersize > 3) ? Length::CM : Length::IN;

	lengthToWidgets(widthED, widthUnitsLC,
		(params_.width).asString(), default_unit);

	QString const special = toqstr(params_.special);
	if (!special.isEmpty() && special != "none") {
		QString spc;
		for (int i = 0; i != gui_names_spec_.size(); ++i) {
			if (special == ids_spec_[i])
				spc = gui_names_spec_[i];
		}
		for (int i = 0; i != widthUnitsLC->count(); ++i) {
			if (widthUnitsLC->itemText(i) == spc)
				widthUnitsLC->setCurrentIndex(i);
		}
	}

	lengthToWidgets(heightED, heightUnitsLC,
		(params_.height).asString(), default_unit);
	
	QString const height_special = toqstr(params_.height_special);
	if (!height_special.isEmpty() && height_special != "none") {
		QString hspc;
		for (int i = 0; i != gui_names_spec_.size(); ++i) {
			if (height_special == ids_spec_[i])
				hspc = gui_names_spec_[i];
		}
		for (int i = 0; i != heightUnitsLC->count(); ++i) {
			if (heightUnitsLC->itemText(i) == hspc)
				heightUnitsLC->setCurrentIndex(i);
		}
	}
	// set no optional height when the value is the default "1\height"
	// (special units like \height are handled as "in",
	if (height_special == "totalheight" &&  params_.height == Length("1in"))
		heightCB->setCheckState(Qt::Unchecked);
	else
		heightCB->setCheckState(Qt::Checked);

	heightCB->setEnabled(ibox);
}


void GuiBox::applyView()
{
	bool pagebreak = pagebreakCB->isEnabled() && pagebreakCB->isChecked();
	if (pagebreak)
		params_.type = "Framed";
	else
		params_.type = fromqstr(ids_[typeCO->currentIndex()]);

	params_.inner_box = (!pagebreak && innerBoxCO->currentText() != qt_("None"));
	params_.use_parbox = (!pagebreak && innerBoxCO->currentText() == qt_("Parbox"));

	params_.pos = "tcb"[valignCO->currentIndex()];
	params_.inner_pos = "tcbs"[ialignCO->currentIndex()];
	params_.hor_pos = "lcrs"[halignCO->currentIndex()];

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
	params_.special = fromqstr(ids_spec_[i]);

	string width;
	if (spec) {
		width = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		width += "in";
	} else {
		width = widgetsToLength(widthED, widthUnitsLC);
	}

	params_.width = Length(width);

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
	params_.height_special = fromqstr(ids_spec_[i]);

	string height;
	if (spec  && !isValidLength(fromqstr(heightED->text()))) {
		height = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		height += "in";
	} else
		height = widgetsToLength(heightED, heightUnitsLC);

	// the height parameter is omitted in InsetBox.cpp when the value
	// is "1in" and "Total Height" is used as unit.
	// 1in + "Total Height" means "1\height" which is the LaTeX default when
	// no height is given
	if (heightCB->checkState() == Qt::Checked)
		params_.height = Length(height);
	else {
		params_.height = Length("1in");
		params_.height_special = fromqstr(ids_spec_[3]);
	}
}


void GuiBox::setSpecial(bool ibox)
{
	// FIXME: Needed? Already done in the constructor
	ids_spec_ = boxGuiSpecialLengthIds();
	gui_names_spec_ = boxGuiSpecialLengthNames();

	QString const current_text = widthUnitsLC->currentText();

	// check if the widget contains the special units
	int const count = widthUnitsLC->count();
	bool has_special = false;
	for (int i = 0; i != count; ++i)
		if (widthUnitsLC->itemText(i).contains(qt_("Total Height")) > 0)
			has_special = true;
	// insert 'em if needed...
	if (!ibox && !has_special) {
		for (int i = 1; i < gui_names_spec_.size(); ++i)
			widthUnitsLC->addItem(gui_names_spec_[i]);
	// ... or remove 'em if needed
	} else if (ibox && has_special) {
		widthUnitsLC->clear();
		for (int i = 0; i != num_units; ++i)
			widthUnitsLC->addItem(qt_(unit_name_gui[i]));
	}
	// restore selected text, if possible
	int const idx = widthUnitsLC->findText(current_text);
	if (idx != -1)
		widthUnitsLC->setCurrentIndex(idx);
}


void GuiBox::setInnerType(bool frameless, int i)
{
	// with "frameless" boxes, inner box is mandatory (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo
	if (frameless) {
		innerBoxCO->clear();
		innerBoxCO->addItem(qt_("Parbox"));
		innerBoxCO->addItem(qt_("Minipage"));
		if (i != 0)
			innerBoxCO->setCurrentIndex(i - 1);
		else
			innerBoxCO->setCurrentIndex(i);
	} else {
		innerBoxCO->clear();
		innerBoxCO->addItem(qt_("None"));
		innerBoxCO->addItem(qt_("Parbox"));
		innerBoxCO->addItem(qt_("Minipage"));
		innerBoxCO->setCurrentIndex(i);
	}
}

bool GuiBox::initialiseParams(string const & data)
{
	InsetBox::string2params(data, params_);
	return true;

}


void GuiBox::clearParams()
{
	params_ = InsetBoxParams("");
}


void GuiBox::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetBox::params2string(params_)));
}


Dialog * createGuiBox(GuiView & lv) { return new GuiBox(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiBox_moc.cpp"
