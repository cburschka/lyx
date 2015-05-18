/**
 * \file GuiBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna (Minipage stuff)
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBox.h"

#include "GuiApplication.h"
#include "ColorCache.h"
#include "ColorSet.h"
#include "LengthCombo.h"
#include "Length.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "insets/InsetBox.h"

#include "support/gettext.h"
#include "support/foreach.h"
#include "support/lstrings.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

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


static QList<ColorCode> colors()
{
	QList<ColorCode> colors;
	colors << Color_none;
	colors << Color_black;
	colors << Color_white;
	colors << Color_blue;
	colors << Color_brown;
	colors << Color_cyan;
	colors << Color_darkgray;
	colors << Color_gray;
	colors << Color_green;
	colors << Color_lightgray;
	colors << Color_lime;
	colors << Color_magenta;
	colors << Color_olive;
	colors << Color_orange;
	colors << Color_pink;
	colors << Color_purple;
	colors << Color_red;
	colors << Color_teal;
	colors << Color_violet;
	colors << Color_yellow;
	return colors;
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
	connect(thicknessED, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(thicknessUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(separationED, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(separationUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(shadowsizeED, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(shadowsizeUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SIGNAL(changed()));
	connect(frameColorCO, SIGNAL(highlighted(QString)), this, SIGNAL(changed()));
	connect(backgroundColorCO, SIGNAL(highlighted(QString)), this, SIGNAL(changed()));

	heightED->setValidator(unsignedLengthValidator(heightED));
	widthED->setValidator(unsignedLengthValidator(widthED));
	thicknessED->setValidator(unsignedLengthValidator(thicknessED));
	separationED->setValidator(unsignedLengthValidator(separationED));
	shadowsizeED->setValidator(unsignedLengthValidator(shadowsizeED));

	// initialize the length validator
	addCheckedWidget(widthED, widthCB);
	addCheckedWidget(heightED, heightCB);
	addCheckedWidget(thicknessED, thicknessLA);
	addCheckedWidget(separationED, separationLA);
	addCheckedWidget(shadowsizeED, shadowsizeLA);

	// the background can be uncolored while the frame cannot
	color_codes_ = colors();
	fillComboColor(backgroundColorCO, true);
	fillComboColor(frameColorCO, false);

	initDialog();
}


void GuiBox::fillComboColor(QComboBox * combo, bool const is_none)
{
	combo->clear();
	QPixmap coloritem(32, 32);
	QColor color;
	// frameColorCO cannot be uncolored
	if (is_none)
		combo->addItem(toqstr(translateIfPossible(lcolor.getGUIName(Color_none))),
			       toqstr(lcolor.getLaTeXName(Color_none)));
	QList<ColorCode>::const_iterator cit = color_codes_.begin() + 1;
	for (; cit != color_codes_.end(); ++cit) {
		QString const latexname = toqstr(lcolor.getLaTeXName(*cit));
		QString const guiname = toqstr(translateIfPossible(lcolor.getGUIName(*cit)));
		color = QColor(guiApp->colorCache().get(*cit, false));
		coloritem.fill(color);
		combo->addItem(QIcon(coloritem), guiname, latexname);
	}
}


void GuiBox::on_innerBoxCO_activated(int index)
{
	QString itype =	innerBoxCO->itemData(index).toString();
	// handle parbox and minipage the same way
	bool const ibox = (itype != "none" && itype != "makebox");
	if (heightCB->isChecked() && !ibox)
		heightCB->setChecked(false);
	widthCB->setChecked(!widthED->text().isEmpty());
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
	bool const ibox = (itype != "none" && itype != "makebox");
	if (frameless && itype != "makebox") {
		if (heightCB->isChecked() && !ibox)
			heightCB->setChecked(false);
		setSpecial(ibox);
	}
	if (type != "Boxed") {
		if (type != "Frameless")
			widthCB->setChecked(itype != "none");
		pagebreakCB->setChecked(false);
	}
	// assure that the frame color is black for frameless boxes to
	// provide the color "none"
	int const b = frameColorCO->findData("black");
	if (frameless && frameColorCO->currentIndex() != b)
		frameColorCO->setCurrentIndex(b);
	changed();
}


void GuiBox::on_frameColorCO_currentIndexChanged(int index)
{
	// if there is a non-black frame color the background cannot be uncolored
	// therefore remove the entry "none" in this case
	if (index != frameColorCO->findData("black")) {
		int const n = backgroundColorCO->findData("none");
		if (n != -1) {
			if (backgroundColorCO->currentIndex() == n)
				backgroundColorCO->setCurrentIndex(
					    backgroundColorCO->findData("white"));
			backgroundColorCO->removeItem(n);
		}
	} else {
		if (backgroundColorCO->count() == color_codes_.count() - 1)
			backgroundColorCO->insertItem(0, toqstr(translateIfPossible((lcolor.getGUIName(Color_none)))),
						      toqstr(lcolor.getLaTeXName(Color_none)));
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
	// LaTeX's default for \fboxrule is 0.4 pt
	thicknessED->setText("0.4");
	thicknessUnitsLC->setCurrentItem(Length::PT);
	// LaTeX's default for \fboxsep is 3 pt
	separationED->setText("3");
	separationUnitsLC->setCurrentItem(Length::PT);
	// LaTeX's default for \shadowsize is 4 pt
	shadowsizeED->setText("4");
	shadowsizeUnitsLC->setCurrentItem(Length::PT);
}


void GuiBox::on_widthCB_stateChanged(int)
{
	changed();
}


void GuiBox::on_heightCB_stateChanged(int /*state*/)
{
	changed();
}


void GuiBox::on_pagebreakCB_stateChanged()
{
	bool pbreak = (pagebreakCB->checkState() == Qt::Checked);
	if (pbreak)
		widthCB->setChecked(!pbreak);
	if (!pbreak) {
		on_typeCO_activated(typeCO->currentIndex());
		return;
	}
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

	// halign is only allowed if a width is used
	halignCO->setEnabled(widthCB->isChecked());
	// add the entry "Stretch" if the box is \makebox or \framebox and if not already there
	if ((inner_type == "makebox" || (type == "Boxed" && inner_type == "none"))
		&& halignCO->count() < 4)
		halignCO->addItem(toqstr("Stretch"));
	else if (inner_type != "makebox" && (type != "Boxed" && inner_type != "none"))
		halignCO->removeItem(3); 
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

	// enable line thickness only for the rectangular frame types and drop shadow
	thicknessED->setEnabled(type == "Boxed" || type == "Doublebox" || type == "Shadowbox");
	thicknessUnitsLC->setEnabled(type == "Boxed" || type == "Doublebox" || type == "Shadowbox");
	lengthToWidgets(thicknessED, thicknessUnitsLC,
		(params.thickness).asString(), default_unit);
	// enable line separation for the allowed frame types
	separationED->setEnabled(type == "Boxed" || type == "ovalbox" || type == "Ovalbox"
		|| type == "Doublebox" || type == "Shadowbox");
	separationUnitsLC->setEnabled(type == "Boxed" || type == "ovalbox" || type == "Ovalbox"
		|| type == "Doublebox" || type == "Shadowbox");
	lengthToWidgets(separationED, separationUnitsLC,
		(params.separation).asString(), default_unit);
	// enable shadow size for drop shadow
	shadowsizeED->setEnabled(type == "Shadowbox");
	shadowsizeUnitsLC->setEnabled(type == "Shadowbox");
	lengthToWidgets(shadowsizeED, shadowsizeUnitsLC,
		(params.shadowsize).asString(), default_unit);
	// set color
	frameColorCO->setCurrentIndex(frameColorCO->findData(toqstr(params.framecolor)));
	backgroundColorCO->setCurrentIndex(backgroundColorCO->findData(toqstr(params.backgroundcolor)));
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

	// handle the line thickness, line separation and shadow size
	if (thicknessED->isEnabled())
		params.thickness = Length(widgetsToLength(thicknessED, thicknessUnitsLC));
	else
		params.thickness = Length("0.4pt");
	if (separationED->isEnabled())
		params.separation = Length(widgetsToLength(separationED, separationUnitsLC));
	else
		params.separation = Length("3pt");
	if (separationED->isEnabled())
		params.shadowsize = Length(widgetsToLength(shadowsizeED, shadowsizeUnitsLC));
	else
		params.shadowsize = Length("4pt");
	if (frameColorCO->isEnabled())
		params.framecolor =
			fromqstr(frameColorCO->itemData(frameColorCO->currentIndex()).toString());
	else
		params.framecolor = "black";
	if (backgroundColorCO->isEnabled())
		params.backgroundcolor =
			fromqstr(backgroundColorCO->itemData(backgroundColorCO->currentIndex()).toString());
	else
		params.backgroundcolor = "none";

	return from_ascii(InsetBox::params2string(params));
}


bool GuiBox::checkWidgets(bool readonly) const
{
	typeCO->setEnabled(!readonly);

	if (readonly) {
		pagebreakCB->setEnabled(false);
		innerBoxCO->setEnabled(false);
		valignCO->setEnabled(false);
		ialignCO->setEnabled(false);
		halignCO->setEnabled(false);
		widthCB->setEnabled(false);
		widthED->setEnabled(false);
		widthUnitsLC->setEnabled(false);
		heightED->setEnabled(false);
		heightUnitsLC->setEnabled(false);
		heightCB->setEnabled(false);
		thicknessED->setEnabled(false);
		thicknessUnitsLC->setEnabled(false);
		separationED->setEnabled(false);
		separationUnitsLC->setEnabled(false);
		shadowsizeED->setEnabled(false);
		shadowsizeUnitsLC->setEnabled(false);
	} else {
		QString const outer =
			typeCO->itemData(typeCO->currentIndex()).toString();
		QString const itype =
			innerBoxCO->itemData(innerBoxCO->currentIndex()).toString();
		bool const ibox = (itype != "none" && itype != "makebox");
		valignCO->setEnabled(ibox);
		ialignCO->setEnabled(ibox);
		if (heightCB->isChecked() && !ibox)
			heightCB->setChecked(false);
		heightCB->setEnabled(ibox);
		// the width can only be selected for makebox or framebox
		widthCB->setEnabled(itype == "makebox"
			|| (outer == "Boxed" && itype == "none"	&& !pagebreakCB->isChecked()));
		// except for Frameless and Boxed, the width cannot be specified if
		// there is no inner box
		bool const width_enabled =
			ibox || outer == "Frameless" || (outer == "Boxed" && !pagebreakCB->isChecked());
		// enable if width_enabled
		widthED->setEnabled(width_enabled);
		widthUnitsLC->setEnabled(width_enabled);
		if (!widthCB->isChecked() && widthCB->isEnabled()) {
			widthED->setEnabled(false);
			widthUnitsLC->setEnabled(false);
		}
		// halign is only allowed if a width is used
		halignCO->setEnabled(widthCB->isChecked());
		// add the entry "Stretch" if the box is \makebox or \framebox and if not already there
		if ((itype == "makebox" || (outer == "Boxed" && itype == "none"))
			&& halignCO->count() < 4)
			halignCO->addItem(toqstr("Stretch"));
		else if (itype != "makebox" && (outer != "Boxed" && itype != "none"))
			halignCO->removeItem(3);
		// pagebreak is only allowed for Boxed without inner box
		pagebreakCB->setEnabled(!ibox && outer == "Boxed");

		heightED->setEnabled(itype != "none" && heightCB->isChecked());
		heightUnitsLC->setEnabled(itype != "none" && heightCB->isChecked());
		heightCB->setEnabled(ibox);

		// enable line thickness for the rectangular frame types and drop shadow
		thicknessED->setEnabled(outer == "Boxed" || outer == "Doublebox" || outer == "Shadowbox");
		thicknessUnitsLC->setEnabled(outer == "Boxed" || outer == "Doublebox" || outer == "Shadowbox");
		// set default values if empty
		if (thicknessED->text().isEmpty() && thicknessED->isEnabled()) {
			thicknessED->setText("0.4");
			thicknessUnitsLC->setCurrentItem(Length::PT);
		}
		// enable line separation for the allowed frame types
		separationED->setEnabled(outer == "Boxed" || outer == "ovalbox" || outer == "Ovalbox"
			|| outer == "Doublebox" || outer == "Shadowbox");
		separationUnitsLC->setEnabled(outer == "Boxed" || outer == "ovalbox" || outer == "Ovalbox"
			|| outer == "Doublebox" || outer == "Shadowbox");
		// set default values if empty
		if (separationED->text().isEmpty() && separationED->isEnabled()) {
			separationED->setText("3");
			separationUnitsLC->setCurrentItem(Length::PT);
		}
		// enable shadow size for drop shadow
		shadowsizeED->setEnabled(outer == "Shadowbox");
		shadowsizeUnitsLC->setEnabled(outer == "Shadowbox");
		// set default values if empty
		if (shadowsizeED->text().isEmpty() && shadowsizeED->isEnabled()) {
			shadowsizeED->setText("4");
			shadowsizeUnitsLC->setCurrentItem(Length::PT);
		}
		// \fboxcolor and \colorbox cannot be used for fancybox boxes
		frameColorCO->setEnabled(!pagebreakCB->isChecked() && outer == "Boxed");
		backgroundColorCO->setEnabled(!pagebreakCB->isChecked() && (frameColorCO->isEnabled() || outer == "Frameless"));
	}

	return InsetParamsWidget::checkWidgets();
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
