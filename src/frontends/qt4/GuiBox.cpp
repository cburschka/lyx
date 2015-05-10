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


GuiBox::GuiBox(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	// fill the box type choice
	ids_ = boxGuiIds();
	gui_names_ = boxGuiNames();
	for (int i = 0; i != ids_.size(); ++i)
		typeCO->addItem(gui_names_[i], ids_[i]);

	// add the special units to the height choice
	// width needs different handling
	ids_spec_ = boxGuiSpecialLengthIds();
	gui_names_spec_ = boxGuiSpecialLengthNames();
	for (int i = 0; i != ids_spec_.size(); ++i)
		heightUnitsLC->addItem(gui_names_spec_[i], ids_spec_[i]);

	connect(widthED, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(widthUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(valignCO, SIGNAL(highlighted(QString)), this, SIGNAL(changed()));
	connect(heightED, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(heightUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(halignCO, SIGNAL(activated(int)), this, SIGNAL(changed()));
	connect(ialignCO, SIGNAL(activated(int)), this, SIGNAL(changed()));

	heightED->setValidator(unsignedLengthValidator(heightED));
	widthED->setValidator(unsignedLengthValidator(widthED));

	// initialize the length validator
	addCheckedWidget(widthED, widthCB);
	addCheckedWidget(heightED, heightCB);

	initDialog();
}


void GuiBox::on_innerBoxCO_activated(int /* index */)
{
	QString itype =
		innerBoxCO->itemData(innerBoxCO->currentIndex()).toString();
	// handle parbox and minipage the same way
	bool const ibox =
		(itype != "none"
		 && itype != "makebox");
	QString const outer =
		typeCO->itemData(typeCO->currentIndex()).toString();
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	if (heightCB->isChecked() && !ibox)
		heightCB->setChecked(false);
	heightCB->setEnabled(ibox);
	// the width can only be selected for makebox or framebox
	widthCB->setEnabled(itype == "makebox"
	                    || (outer == "Boxed" && itype == "none"));
	widthCB->setChecked(!widthED->text().isEmpty());
	// except for frameless and boxed, the width cannot be specified if
	// there is no inner box
	bool const width_enabled =
		ibox || outer == "Frameless" || outer == "Boxed";
	// enable if width_enabled, except if checkbaox is active but unset
	widthED->setEnabled(width_enabled || (widthCB->isEnabled() && widthCB->isChecked()));
	widthUnitsLC->setEnabled(width_enabled || (widthCB->isEnabled() && widthCB->isChecked()));
	if (!widthCB->isChecked() && widthCB->isEnabled()) {
		widthED->setEnabled(false);
		widthUnitsLC->setEnabled(false);
	}
	// halign is only allowed without inner box and if a width is used and if
	// pagebreak is not used
	halignCO->setEnabled(!pagebreakCB->isChecked() && widthCB->isChecked()
	                     && ((!ibox && outer == "Boxed") || itype == "makebox"));
	// pagebreak is only allowed for Boxed without inner box
	pagebreakCB->setEnabled(!ibox && outer == "Boxed");
	setSpecial(ibox);
	changed();
}


void GuiBox::on_typeCO_activated(int index)
{
 	QString const type =
		typeCO->itemData(index).toString();
	bool const frameless = (type == "Frameless");
	QString itype =
		innerBoxCO->itemData(innerBoxCO->currentIndex()).toString();
	setInnerType(frameless, itype);
	// refresh itype because it might have been changed in setInnerType
	itype =
		innerBoxCO->itemData(innerBoxCO->currentIndex()).toString();
	// handle parbox and minipage the same way
	bool const ibox =
		(itype != "none"
		 && itype != "makebox");
	if (frameless && itype != "makebox") {
		valignCO->setEnabled(ibox);
		ialignCO->setEnabled(ibox);
		if (heightCB->isChecked() && !ibox)
			heightCB->setChecked(false);
		heightCB->setEnabled(ibox);
		setSpecial(ibox);
	}
	// the width can only be selected for makebox or framebox
	widthCB->setEnabled(itype == "makebox"
	                    || (type == "Boxed" && itype == "none"));
	// except for frameless and boxed, the width cannot be specified if
	// there is no inner box
	bool const width_enabled =
		itype != "none" || frameless || type == "Boxed";
	// enable if width_enabled, except if checkbaox is active but unset
	widthED->setEnabled(width_enabled || (widthCB->isEnabled() && widthCB->isChecked()));
	widthUnitsLC->setEnabled(width_enabled || (widthCB->isEnabled() && widthCB->isChecked()));
	if (!widthCB->isChecked() && widthCB->isEnabled()) {
		widthED->setEnabled(false);
		widthUnitsLC->setEnabled(false);
	}
	// halign is only allowed without inner box and if a width is used and if
	// pagebreak is not used
	halignCO->setEnabled(!pagebreakCB->isChecked() && widthCB->isChecked()
	                     && ((itype == "none" && type == "Boxed") || itype == "makebox"));
	// pagebreak is only allowed for Boxed without inner box
	pagebreakCB->setEnabled(type == "Boxed" && itype == "none");
	if (type != "Boxed") {
		if (type != "Frameless")
			widthCB->setChecked(itype != "none");
		pagebreakCB->setChecked(false);
	}
	changed();
}


void GuiBox::initDialog()
{
	setInnerType(true, toqstr("minipage"));
	widthED->setText("100");
	widthCB->setChecked(true);
	widthCB->setEnabled(false);
	widthUnitsLC->setCurrentItem(Length::PCW);
	heightED->setText("1");
	heightUnitsLC->setCurrentItem("totalheight");
}


void GuiBox::on_widthCB_stateChanged(int)
{
	if (widthCB->isEnabled()) {
		widthED->setEnabled(widthCB->isChecked());
		widthUnitsLC->setEnabled(widthCB->isChecked());
		halignCO->setEnabled(widthCB->isChecked());
	}
	changed();
}


void GuiBox::on_heightCB_stateChanged(int state)
{
	bool const enable = (innerBoxCO->currentText() != qt_("None"))
		&& (state == Qt::Checked);
	heightED->setEnabled(enable);
	heightUnitsLC->setEnabled(enable);
	changed();
}


void GuiBox::on_pagebreakCB_stateChanged()
{
	bool pbreak = (pagebreakCB->checkState() == Qt::Checked);
	innerBoxCO->setEnabled(!pbreak);
	widthCB->setEnabled(!pbreak);
	if (pbreak)
		widthCB->setChecked(!pbreak);
	widthED->setEnabled(!pbreak);
	widthUnitsLC->setEnabled(!pbreak);
	if (!pbreak) {
		on_typeCO_activated(typeCO->currentIndex());
		return;
	}
	valignCO->setEnabled(false);
	ialignCO->setEnabled(false);
	halignCO->setEnabled(false);
	heightCB->setEnabled(false);
	heightED->setEnabled(false);
	heightUnitsLC->setEnabled(false);
	setSpecial(false);
	changed();
}


void GuiBox::paramsToDialog(Inset const * inset)
{
	InsetBox const * box = static_cast<InsetBox const *>(inset);
	InsetBoxParams const & params = box->params();
	QString type = toqstr(params.type);
	if (type == "Framed") {
		pagebreakCB->setChecked(true);
		type = "Boxed";
	} else {
		pagebreakCB->setChecked(false);
	}

	typeCO->setCurrentIndex(typeCO->findData(type));

	// default: minipage
	QString inner_type = "minipage";
	if (!params.inner_box)
		inner_type = "none";
	if (params.use_parbox)
		inner_type = "parbox";
	if (params.use_makebox)
		inner_type = "makebox";
	bool const frameless = (params.type == "Frameless");
	setInnerType(frameless, inner_type);

	char c = params.pos;
	valignCO->setCurrentIndex(string("tcb").find(c, 0));
	c = params.inner_pos;
	ialignCO->setCurrentIndex(string("tcbs").find(c, 0));
	c = params.hor_pos;
	halignCO->setCurrentIndex(string("lcrs").find(c, 0));

	bool ibox = (params.inner_box && !params.use_makebox);
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	setSpecial(ibox);

	// halign is only allowed without inner box and if a width is used and if
	// pagebreak is not used
	halignCO->setEnabled(!pagebreakCB->isChecked() && widthCB->isChecked()
	                     && ((!ibox && type == "Boxed") || params.use_makebox));
	// pagebreak is only allowed for Boxed without inner box
	pagebreakCB->setEnabled(!ibox && type == "Boxed");

	Length::UNIT const default_unit = Length::defaultUnit();

	// the width can only be selected for makebox or framebox
	widthCB->setEnabled(inner_type == "makebox"
	                    || (type == "Boxed"
				&& !ibox && !pagebreakCB->isChecked()));
	if (params.width.empty()) {
		widthCB->setChecked(false);
		lengthToWidgets(widthED, widthUnitsLC,
			params.width, default_unit);
	} else {
		widthCB->setChecked(true);
		lengthToWidgets(widthED, widthUnitsLC,
			params.width, default_unit);
		QString const special = toqstr(params.special);
		if (!special.isEmpty() && special != "none")
			widthUnitsLC->setCurrentItem(special);
	}

	widthED->setEnabled(widthCB->isChecked());
	widthUnitsLC->setEnabled(widthCB->isChecked());

	lengthToWidgets(heightED, heightUnitsLC,
		(params.height).asString(), default_unit);

	QString const height_special = toqstr(params.height_special);
	if (!height_special.isEmpty() && height_special != "none")
		heightUnitsLC->setCurrentItem(height_special);
	// set no optional height if the value is the default "1\height"
	// (special units like \height are handled as "in",
	// FIXME: this is a very bad UI, this check box should be disabled in
	// this case, not forced to 'unchecked' state.
	if (height_special == "totalheight" && params.height == Length("1in"))
		heightCB->setCheckState(Qt::Unchecked);
	else
		heightCB->setCheckState(Qt::Checked);

	heightCB->setEnabled(ibox);
}


docstring GuiBox::dialogToParams() const
{
	bool const pagebreak =
		pagebreakCB->isEnabled() && pagebreakCB->isChecked();
	string box_type;
	if (pagebreak)
		box_type = "Framed";
	else
		box_type = fromqstr(typeCO->itemData(
				typeCO->currentIndex()).toString());

	InsetBoxParams params(box_type);
	params.inner_box =
		(!pagebreak && innerBoxCO->currentText() != qt_("None"));
	params.use_parbox =
		(!pagebreak && innerBoxCO->currentText() == qt_("Parbox"));
	params.use_makebox =
		(!pagebreak && innerBoxCO->currentText() == qt_("Makebox"));

	params.pos = "tcb"[valignCO->currentIndex()];
	params.inner_pos = "tcbs"[ialignCO->currentIndex()];
	params.hor_pos = "lcrs"[halignCO->currentIndex()];

	QString unit =
		widthUnitsLC->itemData(widthUnitsLC->currentIndex()).toString();
	QString value = widthED->text();

	if (widthED->isEnabled()) {
		if (ids_spec_.contains(unit) && !isValidLength(fromqstr(value))) {
			params.special = fromqstr(unit);
			// Note: the unit is simply ignored in this case
			params.width = Length(value.toDouble(), Length::IN);
		} else {
			params.special = "none";
			// we must specify a valid length in this case
			if (value.isEmpty())
				widthED->setText("0");
			params.width = Length(widgetsToLength(widthED, widthUnitsLC));
		}
	} else {
		params.special = "none";
		params.width = Length();
	}

	// the height parameter is omitted if the value
	// is "1in" and "Total Height" is used as unit.
	// 1in + "Total Height" means "1\height" which is the LaTeX default
	// if no height is given
	if (heightCB->checkState() == Qt::Unchecked) {
		params.height = Length("1in");
		params.height_special = "totalheight";
	} else {
		unit = heightUnitsLC->itemData(heightUnitsLC->currentIndex()).toString();
		value = heightED->text();
		if (ids_spec_.contains(unit) && !isValidLength(fromqstr(value))) {
			params.height_special = fromqstr(unit);
			// Note: the unit is simply ignored in this case
			params.height = Length(value.toDouble(), Length::IN);
		} else {
			params.height_special = "none";
			params.height =
				Length(widgetsToLength(heightED, heightUnitsLC));
		}
	}
	return from_ascii(InsetBox::params2string(params));
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


void GuiBox::setInnerType(bool frameless, QString const & type)
{
	// with "frameless" boxes, inner box is mandatory
	// (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo
	innerBoxCO->clear();
	if (!frameless)
		innerBoxCO->addItem(qt_("None"), toqstr("none"));
	else
		innerBoxCO->addItem(qt_("Makebox"), toqstr("makebox"));
	innerBoxCO->addItem(qt_("Parbox"), toqstr("parbox"));
	innerBoxCO->addItem(qt_("Minipage"), toqstr("minipage"));
	int i = (innerBoxCO->findData(type) != -1)
		? innerBoxCO->findData(type) : 0;
	innerBoxCO->setCurrentIndex(i);
}

} // namespace frontend
} // namespace lyx


#include "moc_GuiBox.cpp"
