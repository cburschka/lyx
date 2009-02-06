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
	return QStringList() << "height" << "depth"
		<< "totalheight" << "width";
}


static QStringList boxGuiSpecialLengthNames()
{
	return QStringList() << qt_("Height") << qt_("Depth")
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
	for (int i = 0; i != ids_spec_.size(); ++i)
		heightUnitsLC->addItem(gui_names_spec_[i], ids_spec_[i]);

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
	heightUnitsLC->setCurrentItem("totalheight");
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

	Length::UNIT const default_unit = Length::defaultUnit();

	lengthToWidgets(widthED, widthUnitsLC,
		(params_.width).asString(), default_unit);

	QString const special = toqstr(params_.special);
	if (!special.isEmpty() && special != "none")
		widthUnitsLC->setCurrentItem(special);

	lengthToWidgets(heightED, heightUnitsLC,
		(params_.height).asString(), default_unit);
	
	QString const height_special = toqstr(params_.height_special);
	if (!height_special.isEmpty() && height_special != "none")
		heightUnitsLC->setCurrentItem(height_special);
	// set no optional height if the value is the default "1\height"
	// (special units like \height are handled as "in",
	if (height_special == "totalheight" && params_.height == Length("1in"))
		heightCB->setCheckState(Qt::Unchecked);
	else
		heightCB->setCheckState(Qt::Checked);

	heightCB->setEnabled(ibox);
}


void GuiBox::applyView()
{
	bool pagebreak =
		pagebreakCB->isEnabled() && pagebreakCB->isChecked();
	if (pagebreak)
		params_.type = "Framed";
	else
		params_.type = fromqstr(ids_[typeCO->currentIndex()]);

	params_.inner_box =
		(!pagebreak && innerBoxCO->currentText() != qt_("None"));
	params_.use_parbox =
		(!pagebreak && innerBoxCO->currentText() == qt_("Parbox"));

	params_.pos = "tcb"[valignCO->currentIndex()];
	params_.inner_pos = "tcbs"[ialignCO->currentIndex()];
	params_.hor_pos = "lcrs"[halignCO->currentIndex()];

	QString unit =
		widthUnitsLC->itemData(widthUnitsLC->currentIndex()).toString();
	QString value = widthED->text();
	if (ids_spec_.contains(unit) && !isValidLength(fromqstr(value))) {
		params_.special = fromqstr(unit);
		// Note: the unit is simply ignored in this case
		params_.width = Length(value.toDouble(), Length::IN);
	} else {
		params_.special = "none";
		params_.width = Length(widgetsToLength(widthED, widthUnitsLC));
	}

	// the height parameter is omitted in if the value
	// is "1in" and "Total Height" is used as unit.
	// 1in + "Total Height" means "1\height" which is the LaTeX default
	// if no height is given
	if (heightCB->checkState() == Qt::Unchecked) {
		params_.height = Length("1in");
		params_.height_special = "totalheight";
	} else {
		unit = heightUnitsLC->itemData(heightUnitsLC->currentIndex()).toString();
		value = heightED->text();
		if (ids_spec_.contains(unit) && !isValidLength(fromqstr(value))) {
			params_.height_special = fromqstr(unit);
			// Note: the unit is simply ignored in this case
			params_.height = Length(value.toDouble(), Length::IN);
		} else {
			params_.height_special = "none";
			params_.height =
				Length(widgetsToLength(heightED, heightUnitsLC));
		}
	}
}


void GuiBox::setSpecial(bool ibox)
{
	QString const last_item =
		widthUnitsLC->itemData(heightUnitsLC->currentIndex()).toString();

	// check if the widget contains the special units
	bool const has_special = (widthUnitsLC->findData("totalheight") != -1);
	// insert 'em if needed...
	if (!ibox && !has_special) {
		for (int i = 1; i < ids_spec_.size(); ++i)
			widthUnitsLC->addItem(gui_names_spec_[i], ids_spec_[i]);
	// ... or remove 'em if needed
	} else if (ibox && has_special) {
		for (int i = 1; i < ids_spec_.size(); ++i) {
			int n = widthUnitsLC->findData(ids_spec_[i]);
			if (n != -1)
				widthUnitsLC->removeItem(n);
		}
	}
	// restore selected text, if possible
	widthUnitsLC->setCurrentItem(last_item);
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


#include "moc_GuiBox.cpp"
