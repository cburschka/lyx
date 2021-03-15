/**
 * \file GuiTabular.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Herbert Voß
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiTabular.h"

#include "GuiApplication.h"
#include "GuiSetBorder.h"
#include "GuiView.h"
#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Language.h"
#include "LyX.h"
#include "LyXRC.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lstrings.h"

#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

using namespace std;

namespace lyx {
namespace frontend {

GuiTabular::GuiTabular(QWidget * parent)
	: InsetParamsWidget(parent), firstheader_suppressable_(false),
	  lastfooter_suppressable_(false), orig_leftborder_(GuiSetBorder::LINE_UNDEF),
	  orig_rightborder_(GuiSetBorder::LINE_UNDEF), lastrow_(0)

{
	setupUi(this);

	tabularWidthED->setValidator(unsignedLengthValidator(tabularWidthED));
	columnWidthED->setValidator(unsignedLengthValidator(columnWidthED));
	multirowOffsetED->setValidator(new LengthValidator(multirowOffsetED));
	topspaceED->setValidator(new LengthValidator(topspaceED));
	bottomspaceED->setValidator(new LengthValidator(bottomspaceED));
	interlinespaceED->setValidator(new LengthValidator(interlinespaceED));

	tabularWidthUnitLC->setCurrentItem(Length::defaultUnit());
	columnWidthUnitLC->setCurrentItem(Length::defaultUnit());
	multirowOffsetUnitLC->setCurrentItem(Length::defaultUnit());
	topspaceUnitLC->setCurrentItem(Length::defaultUnit());
	bottomspaceUnitLC->setCurrentItem(Length::defaultUnit());
	interlinespaceUnitLC->setCurrentItem(Length::defaultUnit());

	connect(topspaceED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));
	connect(topspaceUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(bottomspaceED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));
	connect(bottomspaceUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(interlinespaceED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));
	connect(interlinespaceUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(booktabsRB, SIGNAL(clicked(bool)),
		this, SLOT(booktabs_toggled(bool)));
	connect(borderDefaultRB, SIGNAL(clicked(bool)),
		this, SLOT(nonbooktabs_toggled(bool)));
	connect(borderSetPB, SIGNAL(clicked()),
		this, SLOT(borderSet_clicked()));
	connect(borderUnsetPB, SIGNAL(clicked()),
		this, SLOT(borderUnset_clicked()));
	connect(resetFormalCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(hAlignCO, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(vAlignCO, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(multicolumnCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(multirowCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(multirowOffsetED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));
	connect(multirowOffsetUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(newpageCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(headerStatusCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(headerBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(headerBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(firstheaderStatusCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(firstheaderBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(firstheaderBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(firstheaderNoContentsCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(footerStatusCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(footerBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(footerBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(lastfooterStatusCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(lastfooterBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(lastfooterBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(lastfooterNoContentsCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(captionStatusCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(specialAlignmentED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));
	connect(columnWidthED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));
	connect(columnWidthUnitLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(topSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(bottomSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(rightSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(leftSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(topLTSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(topRTSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(bottomLTSet()),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(bottomRTSet()),
		this, SLOT(checkEnabled()));
	connect(rotateTabularCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(rotateTabularAngleSB, SIGNAL(valueChanged(int)),
		this, SLOT(checkEnabled()));
	connect(rotateCellCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(rotateCellAngleSB, SIGNAL(valueChanged(int)),
		this, SLOT(checkEnabled()));
	connect(TableAlignCO, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(longTabularCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(leftRB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(centerRB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(rightRB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(tabularWidthED, SIGNAL(textEdited(const QString &)),
		this, SLOT(checkEnabled()));

#if QT_VERSION < 0x060000
	decimalPointED->setValidator(new QRegExpValidator(QRegExp("\\S"), this));
#else
	decimalPointED->setValidator(new QRegularExpressionValidator(QRegularExpression("\\S"), this));
#endif
	decimalPointED->setMaxLength(1);

	// initialize the length validator
	addCheckedWidget(columnWidthED, columnWidthLA);
	addCheckedWidget(multirowOffsetED, multirowOffsetLA);
	addCheckedWidget(topspaceED, topspaceLA);
	addCheckedWidget(bottomspaceED, bottomspaceLA);
	addCheckedWidget(interlinespaceED, interlinespaceLA);
	addCheckedWidget(tabularWidthED, tabularWidthLA);
}


void GuiTabular::on_topspaceCO_activated(int index)
{
	bool const enable = (index == 2);
	topspaceED->setEnabled(enable);
	topspaceUnitLC->setEnabled(enable);
}


void GuiTabular::on_bottomspaceCO_activated(int index)
{
	bool const enable = (index == 2);
	bottomspaceED->setEnabled(enable);
	bottomspaceUnitLC->setEnabled(enable);
}


void GuiTabular::on_interlinespaceCO_activated(int index)
{
	bool const enable = (index == 2);
	interlinespaceED->setEnabled(enable);
	interlinespaceUnitLC->setEnabled(enable);
}


void GuiTabular::on_columnTypeCO_activated(int index)
{
	checkEnabled();
	if (index == 2)
		columnWidthED->setFocus();
}


void GuiTabular::enableWidgets() const
{
	// if there is a LaTeX argument, the width and alignment will be overwritten
	// therefore disable them in this case
	bool const fixed = specialAlignmentED->text().isEmpty()
			&& columnTypeCO->currentIndex() == 2;
	columnWidthED->setEnabled(fixed);
	columnWidthUnitLC->setEnabled(fixed);
	// if the column has a width, multirows are always left-aligned
	// therefore disable hAlignCB in this case
	hAlignCO->setEnabled(!(multirowCB->isChecked()
		&& !widgetsToLength(columnWidthED, columnWidthUnitLC).empty())
		&& specialAlignmentED->text().isEmpty());
	// decimal alignment is only possible for non-multicol and non-multirow cells
	if ((multicolumnCB->isChecked() || multirowCB->isChecked() || columnTypeCO->currentIndex() == 1)
		&& hAlignCO->findData(toqstr("decimal")))
		hAlignCO->removeItem(hAlignCO->findData(toqstr("decimal")));
	else if (!multicolumnCB->isChecked() && !multirowCB->isChecked()
		&& hAlignCO->findData(toqstr("decimal")) == -1)
		hAlignCO->addItem(qt_("At Decimal Separator"), toqstr("decimal"));
	bool const dalign =
		hAlignCO->itemData(hAlignCO->currentIndex()).toString() == QString("decimal");
	decimalPointED->setEnabled(dalign);
	decimalLA->setEnabled(dalign);
	resetFormalCB->setEnabled(booktabsRB->isChecked());

	bool const setwidth = TableAlignCO->currentText() == qt_("Middle");
	tabularWidthLA->setEnabled(setwidth);
	tabularWidthED->setEnabled(setwidth);
	tabularWidthUnitLC->setEnabled(setwidth);

	rotateTabularAngleSB->setEnabled(rotateTabularCB->isChecked()
					 && !longTabularCB->isChecked());
	rotateCellAngleSB->setEnabled(rotateCellCB->isChecked());

	bool const enable_valign =
		!multirowCB->isChecked()
		&& !widgetsToLength(columnWidthED, columnWidthUnitLC).empty()
		&& specialAlignmentED->text().isEmpty();
	vAlignCO->setEnabled(enable_valign);
	vAlignLA->setEnabled(enable_valign);

	topspaceED->setEnabled(topspaceCO->currentIndex() == 2);
	topspaceED->setEnabled(topspaceCO->currentIndex() == 2);
	topspaceUnitLC->setEnabled(topspaceCO->currentIndex() == 2);
	bottomspaceED->setEnabled(bottomspaceCO->currentIndex() == 2);
	bottomspaceUnitLC->setEnabled(bottomspaceCO->currentIndex() == 2);
	interlinespaceED->setEnabled(interlinespaceCO->currentIndex() == 2);
	interlinespaceUnitLC->setEnabled(interlinespaceCO->currentIndex() == 2);

	// setting as longtable is not allowed when table is inside a float
	bool const is_tabular_star = !tabularWidthED->text().isEmpty();
	longTabularCB->setEnabled(funcEnabled(Tabular::SET_LONGTABULAR));
	bool const longtabular = longTabularCB->isChecked();
	longtableGB->setEnabled(true);
	newpageCB->setEnabled(longtabular);
	alignmentGB->setEnabled(longtabular);
	// longtables and tabular* cannot have a vertical alignment
	TableAlignLA->setDisabled(is_tabular_star || longtabular);
	TableAlignCO->setDisabled(is_tabular_star || longtabular);
	// longtable cannot be rotated with rotating package, only
	// with [pdf]lscape, which only supports 90 deg.
	rotateTabularAngleSB->setDisabled(longtabular);

	// FIXME: This Dialog is really horrible, disabling/enabling a checkbox
	// depending on the cursor position is very very unintuitive...
	// We need some edit boxes to show which rows are header/footer/etc
	// without having to move the cursor first.
	headerStatusCB->setEnabled(longtabular
		&& (headerStatusCB->isChecked() ?
		    funcEnabled(Tabular::UNSET_LTHEAD) :
		    funcEnabled(Tabular::SET_LTHEAD)));
	headerBorderAboveCB->setEnabled(longtabular
		&& headerStatusCB->isChecked());
	headerBorderBelowCB->setEnabled(longtabular
		&& headerStatusCB->isChecked());

	// first header can only be suppressed when there is a header
	// firstheader_suppressable_ is set in paramsToDialog
	firstheaderNoContentsCB->setEnabled(longtabular && firstheader_suppressable_);
	// check if setting a first header is allowed
	// additionally check firstheaderNoContentsCB because when this is
	// the case a first header makes no sense
	firstheaderStatusCB->setEnabled((firstheaderStatusCB->isChecked() ?
		   funcEnabled(Tabular::UNSET_LTFIRSTHEAD) :
		   funcEnabled(Tabular::SET_LTFIRSTHEAD))
		&& longtabular && !firstheaderNoContentsCB->isChecked());
	firstheaderBorderAboveCB->setEnabled(longtabular
		&& firstheaderStatusCB->isChecked());
	firstheaderBorderBelowCB->setEnabled(longtabular
		&& firstheaderStatusCB->isChecked());

	footerStatusCB->setEnabled(longtabular
		&& (footerStatusCB->isChecked() ?
		    funcEnabled(Tabular::UNSET_LTFOOT) :
		    funcEnabled(Tabular::SET_LTFOOT)));
	footerBorderAboveCB->setEnabled(longtabular
		&& footerBorderAboveCB->isChecked());
	footerBorderBelowCB->setEnabled(longtabular
		&& footerBorderAboveCB->isChecked());

	// last footer can only be suppressed when there is a footer
	// lastfooter_suppressable_ is set in paramsToDialog
	lastfooterNoContentsCB->setEnabled(longtabular && lastfooter_suppressable_);
	// check if setting a last footer is allowed
	// additionally check lastfooterNoContentsCB because when this is
	// the case a last footer makes no sense
	lastfooterStatusCB->setEnabled((lastfooterStatusCB->isChecked() ?
		   funcEnabled(Tabular::UNSET_LTLASTFOOT) :
		   funcEnabled(Tabular::SET_LTLASTFOOT))
		&& longtabular && !lastfooterNoContentsCB->isChecked());
	lastfooterBorderAboveCB->setEnabled(longtabular
		&& lastfooterBorderAboveCB->isChecked());
	lastfooterBorderBelowCB->setEnabled(longtabular
		&& lastfooterBorderAboveCB->isChecked());

	captionStatusCB->setEnabled(funcEnabled(Tabular::TOGGLE_LTCAPTION)
		&& longtabular);

	multicolumnCB->setEnabled(funcEnabled(Tabular::MULTICOLUMN)
		&& !dalign && !multirowCB->isChecked());
	multirowCB->setEnabled(funcEnabled(Tabular::MULTIROW)
		&& !dalign && !multicolumnCB->isChecked());
	bool const enable_mr = multirowCB->isChecked();
	multirowOffsetLA->setEnabled(enable_mr);
	multirowOffsetED->setEnabled(enable_mr);
	multirowOffsetUnitLC->setEnabled(enable_mr);

	// Vertical lines cannot be set in formal tables
	borders->setLeftEnabled(!booktabsRB->isChecked());
	borders->setRightEnabled(!booktabsRB->isChecked());
	// Trimming is only allowed in booktabs and if the line is set
	int const row = tabularRowED->text().toInt();
	borders->setTopLeftTrimEnabled(booktabsRB->isChecked()
				       && borders->topLineSet()
				       && row > 1);
	borders->setTopRightTrimEnabled(booktabsRB->isChecked()
					&& borders->topLineSet()
					&& row > 1);
	borders->setBottomLeftTrimEnabled(booktabsRB->isChecked()
					  && borders->bottomLineSet()
					  && row < lastrow_);
	borders->setBottomRightTrimEnabled(booktabsRB->isChecked()
					   && borders->bottomLineSet()
					   && row < lastrow_);
}


void GuiTabular::checkEnabled()
{
	enableWidgets();
	changed();
}


void GuiTabular::borderSet_clicked()
{
	borders->setTop(GuiSetBorder::LINE_SET);
	borders->setBottom(GuiSetBorder::LINE_SET);
	borders->setLeft(GuiSetBorder::LINE_SET);
	borders->setRight(GuiSetBorder::LINE_SET);
	borders->setTopLeftTrim(GuiSetBorder::LINE_SET);
	borders->setBottomLeftTrim(GuiSetBorder::LINE_SET);
	borders->setTopRightTrim(GuiSetBorder::LINE_SET);
	borders->setBottomRightTrim(GuiSetBorder::LINE_SET);
	// repaint the setborder widget
	borders->update();
	checkEnabled();
}


void GuiTabular::borderUnset_clicked()
{
	borders->setTop(GuiSetBorder::LINE_UNSET);
	borders->setBottom(GuiSetBorder::LINE_UNSET);
	borders->setLeft(GuiSetBorder::LINE_UNSET);
	borders->setRight(GuiSetBorder::LINE_UNSET);
	borders->setTopLeftTrim(GuiSetBorder::LINE_UNSET);
	borders->setBottomLeftTrim(GuiSetBorder::LINE_UNSET);
	borders->setTopRightTrim(GuiSetBorder::LINE_UNSET);
	borders->setBottomRightTrim(GuiSetBorder::LINE_UNSET);
	// repaint the setborder widget
	borders->update();
	checkEnabled();
}


void GuiTabular::booktabs_toggled(bool const check)
{
	// when switching from formal, restore the left/right lines
	if (!check) {
		borders->setLeft(orig_leftborder_);
		borders->setRight(orig_rightborder_);
	}
	// repaint the setborder widget
	borders->update();
	checkEnabled();
}


void GuiTabular::nonbooktabs_toggled(bool const check)
{
	booktabs_toggled(!check);
}


static void addParam(set<string> & params, Tabular::Feature f,
		     string const & arg = string())
{
	if (arg.empty())
		params.insert(featureAsString(f));
	else
		params.insert(featureAsString(f) + ' ' + arg);
}


void GuiTabular::setHAlign(set<string> & params) const
{
	Tabular::Feature num = Tabular::ALIGN_LEFT;
	Tabular::Feature multi_num = Tabular::M_ALIGN_LEFT;
	string const align =
		fromqstr(hAlignCO->itemData(hAlignCO->currentIndex()).toString());
	if (align == "left") {
		num = Tabular::ALIGN_LEFT;
		multi_num = Tabular::M_ALIGN_LEFT;
	} else if (align == "center") {
		num = Tabular::ALIGN_CENTER;
		multi_num = Tabular::M_ALIGN_CENTER;
	} else if (align == "right") {
		num = Tabular::ALIGN_RIGHT;
		multi_num = Tabular::M_ALIGN_RIGHT;
	} else if (align == "justified") {
		num = Tabular::ALIGN_BLOCK;
		//multi_num: no equivalent
	} else if (align == "decimal") {
		num = Tabular::ALIGN_DECIMAL;
		//multi_num: no equivalent
	}

	if (multicolumnCB->isChecked())
		addParam(params, multi_num);
	else
		addParam(params, num);
}


void GuiTabular::setVAlign(set<string> & params) const
{
	int const align = vAlignCO->currentIndex();
	enum VALIGN { TOP, MIDDLE, BOTTOM };
	VALIGN v = TOP;

	switch (align) {
		case 0: v = TOP; break;
		case 1: v = MIDDLE; break;
		case 2: v = BOTTOM; break;
	}

	Tabular::Feature num = Tabular::VALIGN_MIDDLE;
	Tabular::Feature multi_num = Tabular::M_VALIGN_MIDDLE;

	switch (v) {
		case TOP:
			num = Tabular::VALIGN_TOP;
			multi_num = Tabular::M_VALIGN_TOP;
			break;
		case MIDDLE:
			num = Tabular::VALIGN_MIDDLE;
			multi_num = Tabular::M_VALIGN_MIDDLE;
			break;
		case BOTTOM:
			num = Tabular::VALIGN_BOTTOM;
			multi_num = Tabular::M_VALIGN_BOTTOM;
			break;
	}
	if (multicolumnCB->isChecked() || multirowCB->isChecked())
		addParam(params, multi_num);
	else
		addParam(params, num);
}


void GuiTabular::setTableAlignment(set<string> & params) const
{
	int const align = TableAlignCO->currentIndex();
	switch (align) {
		case 0: addParam(params, Tabular::TABULAR_VALIGN_TOP);
			break;
		case 1: addParam(params, Tabular::TABULAR_VALIGN_MIDDLE);
			break;
		case 2: addParam(params, Tabular::TABULAR_VALIGN_BOTTOM);
			break;
	}
}


set<string> const GuiTabular::getTabFeatures() const
{
	set<string> res;
	// table width
	string tabwidth = widgetsToLength(tabularWidthED, tabularWidthUnitLC);
	if (tabwidth.empty())
		tabwidth = "0pt";
	addParam(res, Tabular::SET_TABULAR_WIDTH, tabwidth);

	// apply the fixed width values
	// this must be done before applying the column alignment
	// because its value influences the alignment of multirow cells
	string width = widgetsToLength(columnWidthED, columnWidthUnitLC);
	if (width.empty() || columnTypeCO->currentIndex() != 2)
		width = "0pt";
	if (multicolumnCB->isChecked())
		addParam(res, Tabular::SET_MPWIDTH, width);
	else
		addParam(res, Tabular::SET_PWIDTH, width);

	bool const varwidth = specialAlignmentED->text().isEmpty()
			&& columnTypeCO->currentIndex() == 1;
	if (varwidth)
		addParam(res, Tabular::TOGGLE_VARWIDTH_COLUMN, "on");
	else
		addParam(res, Tabular::TOGGLE_VARWIDTH_COLUMN, "off");

	// apply the column alignment
	// multirows inherit the alignment from the column; if a column width
	// is set, multirows are always left-aligned so that in this case
	// its alignment must not be applied (see bug #8084)
	if (!(multirowCB->isChecked() && width != "0pt"))
		setHAlign(res);

	// SET_DECIMAL_POINT must come after setHAlign() (ALIGN_DECIMAL)
	string decimal_sep = fromqstr(decimalPointED->text());
	if (decimal_sep.empty())
		decimal_sep = to_utf8(decimal_sep_);
	addParam(res, Tabular::SET_DECIMAL_POINT, decimal_sep);

	setVAlign(res);
	setTableAlignment(res);
	//
	if (booktabsRB->isChecked())
		addParam(res, Tabular::SET_BOOKTABS);
	else
		addParam(res, Tabular::UNSET_BOOKTABS);

	//
	switch (topspaceCO->currentIndex()) {
		case 0:
			addParam(res, Tabular::SET_TOP_SPACE, "none");
			break;
		case 1:
			addParam(res, Tabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			if (!topspaceED->text().isEmpty())
				addParam(res, Tabular::SET_TOP_SPACE,
					 widgetsToLength(topspaceED, topspaceUnitLC));
			break;
	}

	//
	switch (bottomspaceCO->currentIndex()) {
		case 0:
			addParam(res, Tabular::SET_BOTTOM_SPACE, "none");
			break;
		case 1:
			addParam(res, Tabular::SET_BOTTOM_SPACE, "default");
			break;
		case 2:
			if (!bottomspaceED->text().isEmpty())
				addParam(res, Tabular::SET_BOTTOM_SPACE,
					widgetsToLength(bottomspaceED,
							bottomspaceUnitLC));
			break;
	}

	//
	switch (interlinespaceCO->currentIndex()) {
		case 0:
			addParam(res, Tabular::SET_INTERLINE_SPACE, "none");
			break;
		case 1:
			addParam(res, Tabular::SET_INTERLINE_SPACE, "default");
			break;
		case 2:
			if (!interlinespaceED->text().isEmpty())
				addParam(res, Tabular::SET_INTERLINE_SPACE,
					widgetsToLength(interlinespaceED,
							interlinespaceUnitLC));
			break;
	}

	//
	if (resetFormalCB->isChecked())
		addParam(res, Tabular::RESET_FORMAL_DEFAULT);
	else if (borders->topLineSet() && borders->bottomLineSet() && borders->leftLineSet()
		&& borders->rightLineSet())
		addParam(res, Tabular::SET_ALL_LINES);
	else if (borders->topLineUnset() && borders->bottomLineUnset() && borders->leftLineUnset()
		&& borders->rightLineUnset())
		addParam(res, Tabular::UNSET_ALL_LINES);
	else {
		if (borders->getLeft() != GuiSetBorder::LINE_UNDECIDED)
			addParam(res, Tabular::SET_LINE_LEFT,
				 borders->leftLineSet() ? "true" : "false");
		if (borders->getRight() != GuiSetBorder::LINE_UNDECIDED)
			addParam(res, Tabular::SET_LINE_RIGHT,
				 borders->rightLineSet() ? "true" : "false");
		if (borders->getTop() != GuiSetBorder::LINE_UNDECIDED)
			addParam(res, Tabular::SET_LINE_TOP,
				 borders->topLineSet() ? "true" : "false");
		if (borders->getBottom() != GuiSetBorder::LINE_UNDECIDED)
			addParam(res, Tabular::SET_LINE_BOTTOM,
				 borders->bottomLineSet() ? "true" : "false");
	}
	if (borders->topLineLTSet())
		addParam(res, Tabular::SET_LTRIM_TOP, "false");
	else if (borders->topLineLTUnset())
		addParam(res, Tabular::SET_LTRIM_TOP, "true");
	if (borders->topLineRTSet())
		addParam(res, Tabular::SET_RTRIM_TOP, "false");
	else if (borders->topLineRTUnset())
		addParam(res, Tabular::SET_RTRIM_TOP, "true");
	if (borders->bottomLineLTSet())
		addParam(res, Tabular::SET_LTRIM_BOTTOM, "false");
	else if (borders->bottomLineLTUnset())
		addParam(res, Tabular::SET_LTRIM_BOTTOM, "true");
	if (borders->bottomLineRTSet())
		addParam(res, Tabular::SET_RTRIM_BOTTOM, "false");
	else if (borders->bottomLineRTUnset())
		addParam(res, Tabular::SET_RTRIM_BOTTOM, "true");

	// apply the special alignment
	string special = fromqstr(specialAlignmentED->text());
	if (support::trim(special).empty())
		special = "none";
	if (multicolumnCB->isChecked())
		addParam(res, Tabular::SET_SPECIAL_MULTICOLUMN, special);
	else
		addParam(res, Tabular::SET_SPECIAL_COLUMN, special);

	//
	if (multicolumnCB->isChecked())
		addParam(res, Tabular::SET_MULTICOLUMN);
	else
		addParam(res, Tabular::UNSET_MULTICOLUMN);

	// apply the multirow offset
	string mroffset = widgetsToLength(multirowOffsetED, multirowOffsetUnitLC);
	if (mroffset.empty())
		mroffset = "0pt";
	if (multirowCB->isChecked())
		addParam(res, Tabular::SET_MROFFSET, mroffset);
	//
	if (multirowCB->isChecked())
		addParam(res, Tabular::SET_MULTIROW);
	else
		addParam(res, Tabular::UNSET_MULTIROW);
	// store the table rotation angle
	string const tabular_angle = convert<string>(rotateTabularAngleSB->value());
	if (rotateTabularCB->isChecked())
		addParam(res, Tabular::SET_ROTATE_TABULAR, tabular_angle);
	else
		addParam(res, Tabular::UNSET_ROTATE_TABULAR, tabular_angle);
	// store the cell rotation angle
	string const cell_angle = convert<string>(rotateCellAngleSB->value());
	if (rotateCellCB->isChecked())
		addParam(res, Tabular::SET_ROTATE_CELL, cell_angle);
	else
		addParam(res, Tabular::UNSET_ROTATE_CELL, cell_angle);
	//
	if (longTabularCB->isChecked())
		addParam(res, Tabular::SET_LONGTABULAR);
	else
		addParam(res, Tabular::UNSET_LONGTABULAR);
	//
	if (newpageCB->isChecked())
		addParam(res, Tabular::SET_LTNEWPAGE);
	else
		addParam(res, Tabular::UNSET_LTNEWPAGE);
	//
	if (captionStatusCB->isChecked())
		addParam(res, Tabular::SET_LTCAPTION);
	else
		addParam(res, Tabular::UNSET_LTCAPTION);
	//
	if (headerStatusCB->isChecked())
		addParam(res, Tabular::SET_LTHEAD, "none");
	else
		addParam(res, Tabular::UNSET_LTHEAD, "none");
	//
	if (headerBorderAboveCB->isChecked())
		addParam(res, Tabular::SET_LTHEAD, "dl_above");
	else
		addParam(res, Tabular::UNSET_LTHEAD, "dl_above");
	//
	if (headerBorderBelowCB->isChecked())
		addParam(res, Tabular::SET_LTHEAD, "dl_below");
	else
		addParam(res, Tabular::UNSET_LTHEAD, "dl_below");
	if (firstheaderBorderAboveCB->isChecked())
		addParam(res, Tabular::SET_LTFIRSTHEAD, "dl_above");
	else
		addParam(res, Tabular::UNSET_LTFIRSTHEAD, "dl_above");
	if (firstheaderBorderBelowCB->isChecked())
		addParam(res, Tabular::SET_LTFIRSTHEAD, "dl_below");
	else
		addParam(res, Tabular::UNSET_LTFIRSTHEAD, "dl_below");
	if (firstheaderStatusCB->isChecked())
		addParam(res, Tabular::SET_LTFIRSTHEAD, "none");
	else
		addParam(res, Tabular::UNSET_LTFIRSTHEAD, "none");
	if (firstheaderNoContentsCB->isChecked())
		addParam(res, Tabular::SET_LTFIRSTHEAD, "empty");
	else
		addParam(res, Tabular::UNSET_LTFIRSTHEAD, "empty");
	if (footerStatusCB->isChecked())
		addParam(res, Tabular::SET_LTFOOT, "none");
	else
		addParam(res, Tabular::UNSET_LTFOOT, "none");
	if (footerBorderAboveCB->isChecked())
		addParam(res, Tabular::SET_LTFOOT, "dl_above");
	else
		addParam(res, Tabular::UNSET_LTFOOT, "dl_above");
	if (footerBorderBelowCB->isChecked())
		addParam(res, Tabular::SET_LTFOOT, "dl_below");
	else
		addParam(res, Tabular::UNSET_LTFOOT, "dl_below");
	if (lastfooterStatusCB->isChecked())
		addParam(res, Tabular::SET_LTLASTFOOT, "none");
	else
		addParam(res, Tabular::UNSET_LTLASTFOOT, "none");
	if (lastfooterBorderAboveCB->isChecked())
		addParam(res, Tabular::SET_LTLASTFOOT, "dl_above");
	else
		addParam(res, Tabular::UNSET_LTLASTFOOT, "dl_above");
	if (lastfooterBorderBelowCB->isChecked())
		addParam(res, Tabular::SET_LTLASTFOOT, "dl_below");
	else
		addParam(res, Tabular::UNSET_LTLASTFOOT, "dl_below");
	if (lastfooterNoContentsCB->isChecked())
		addParam(res, Tabular::SET_LTLASTFOOT, "empty");
	else
		addParam(res, Tabular::UNSET_LTLASTFOOT, "empty");

	if (leftRB->isChecked())
		addParam(res, Tabular::LONGTABULAR_ALIGN_LEFT);
	else if (centerRB->isChecked())
		addParam(res, Tabular::LONGTABULAR_ALIGN_CENTER);
	else if (rightRB->isChecked())
		addParam(res, Tabular::LONGTABULAR_ALIGN_RIGHT);

	return res;
}


docstring GuiTabular::dialogToParams() const
{
	set<string> features = getTabFeatures();
	// Only modify features that have changed
	vector<string> changed_features;
	for (auto const & f : features) {
		if (features_.find(f) == features_.end())
			changed_features.push_back(f);
	}
	if (changed_features.empty())
		return docstring();

	return from_utf8("tabular " + support::getStringFromVector(changed_features, " "));
}


static Length getColumnPWidth(Tabular const & t, size_t cell)
{
	return t.column_info[t.cellColumn(cell)].p_width;
}


static Length getMColumnPWidth(Tabular const & t, size_t cell)
{
	if (t.isMultiColumn(cell) || t.isMultiRow(cell))
		return t.cellInfo(cell).p_width;
	return Length();
}


static Length getMROffset(Tabular const & t, size_t cell)
{
	if (t.isMultiRow(cell))
		return t.cellInfo(cell).mroffset;
	return Length();
}


static docstring getAlignSpecial(Tabular const & t, size_t cell, int what)
{
	if (what == Tabular::SET_SPECIAL_MULTICOLUMN)
		return t.cellInfo(cell).align_special;
	return t.column_info[t.cellColumn(cell)].align_special;
}


GuiSetBorder::BorderState GuiTabular::borderState(GuiSetBorder::BorderState bs,
						  bool const line)
{
	if (bs == GuiSetBorder::LINE_UNDEF)
		bs = line ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
	else if ((bs == GuiSetBorder::LINE_SET && !line)
		 || (bs == GuiSetBorder::LINE_UNSET && line))
		bs = GuiSetBorder::LINE_UNDECIDED;
	return bs;
}


void GuiTabular::paramsToDialog(Inset const * inset)
{
	InsetTabular const * itab = static_cast<InsetTabular const *>(inset);
	// Copy Tabular of current inset.
	Tabular const & tabular = itab->tabular;

	BufferView const * bv = guiApp->currentView()->currentBufferView();
	size_t const cell = bv->cursor().idx();

	row_type const row = tabular.cellRow(cell);
	col_type const col = tabular.cellColumn(cell);

	tabularRowED->setText(QString::number(row + 1));
	tabularColumnED->setText(QString::number(col + 1));
	lastrow_ = int(tabular.nrows());

	bool const multicol = tabular.isMultiColumn(cell);
	multicolumnCB->setChecked(multicol);

	bool const multirow = tabular.isMultiRow(cell);
	multirowCB->setChecked(multirow);

	rotateCellCB->setChecked(tabular.getRotateCell(cell) != 0);
	if (rotateCellCB->isChecked()) {
		if (tabular.getRotateCell(cell) != 0)
			rotateCellAngleSB->setValue(tabular.getRotateCell(cell));
		else
			rotateCellAngleSB->setValue(90);
	}

	longTabularCB->setChecked(tabular.is_long_tabular);

	rotateTabularCB->setChecked(tabular.rotate != 0);
	if (rotateTabularCB->isChecked()) {
		if (longTabularCB->isChecked())
			rotateTabularAngleSB->setValue(90);
		else
			rotateTabularAngleSB->setValue(tabular.rotate != 0 ? tabular.rotate : 90);
	}

	// In what follows, we check the borders of all selected cells,
	// and if there are diverging settings, we use the LINE_UNDECIDED
	// border status.
	GuiSetBorder::BorderState ltop = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState lbottom = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState lleft = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState lright = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState ltop_ltrim = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState ltop_rtrim = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState lbottom_ltrim = GuiSetBorder::LINE_UNDEF;
	GuiSetBorder::BorderState lbottom_rtrim = GuiSetBorder::LINE_UNDEF;
	CursorSlice const & beg = bv->cursor().selBegin();
	CursorSlice const & end = bv->cursor().selEnd();
	if (beg != end) {
		col_type cs = tabular.cellColumn(beg.idx());
		col_type ce = tabular.cellColumn(end.idx());
		if (cs > ce)
			swap(cs, ce);
		row_type rs = tabular.cellRow(beg.idx());
		row_type re = tabular.cellRow(end.idx());
		if (rs > re)
			swap(rs, re);
		for (row_type r = rs; r <= re; ++r)
			for (col_type c = cs; c <= ce; ++c) {
				idx_type const cc = tabular.cellIndex(r, c);
				ltop = borderState(ltop, tabular.topLine(cc));
				lbottom = borderState(lbottom, tabular.bottomLine(cc));
				lleft = borderState(lleft, tabular.leftLine(cc));
				lright = borderState(lright, tabular.rightLine(cc));
				ltop_ltrim = borderState(ltop_ltrim, !tabular.topLineTrim(cc).first);
				ltop_rtrim = borderState(ltop_rtrim, !tabular.topLineTrim(cc).second);
				lbottom_ltrim = borderState(lbottom_ltrim, !tabular.bottomLineTrim(cc).first);
				lbottom_rtrim = borderState(lbottom_rtrim, !tabular.bottomLineTrim(cc).second);
				// store left/right borders for the case of formal/nonformal switch
				orig_leftborder_ = borderState(lleft, tabular.leftLine(cc, true));
				orig_rightborder_ = borderState(lright, tabular.rightLine(cc, true));
			}
	} else {
		ltop = tabular.topLine(cell) ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
		lbottom = tabular.bottomLine(cell) ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
		lleft = tabular.leftLine(cell) ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
		lright = tabular.rightLine(cell) ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
		ltop_ltrim = tabular.topLineTrim(cell).first ? GuiSetBorder::LINE_UNSET : GuiSetBorder::LINE_SET;
		ltop_rtrim = tabular.topLineTrim(cell).second ? GuiSetBorder::LINE_UNSET : GuiSetBorder::LINE_SET;
		lbottom_ltrim = tabular.bottomLineTrim(cell).first ? GuiSetBorder::LINE_UNSET : GuiSetBorder::LINE_SET;
		lbottom_rtrim = tabular.bottomLineTrim(cell).second ? GuiSetBorder::LINE_UNSET : GuiSetBorder::LINE_SET;
		// store left/right borders for the case of formal/nonformal switch
		orig_leftborder_ = tabular.leftLine(cell, true) ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
		orig_rightborder_ = tabular.rightLine(cell, true) ? GuiSetBorder::LINE_SET : GuiSetBorder::LINE_UNSET;
	}
	borders->setTop(ltop);
	borders->setBottom(lbottom);
	borders->setLeft(lleft);
	borders->setRight(lright);
	borders->setTopLeftTrim(ltop_ltrim);
	borders->setTopRightTrim(ltop_rtrim);
	borders->setBottomLeftTrim(lbottom_ltrim);
	borders->setBottomRightTrim(lbottom_rtrim);
	// repaint the setborder widget
	borders->update();

	Length::UNIT const default_unit = Length::defaultUnit();

	///////////////////////////////////
	// Set width and alignment

	Length const tabwidth = tabular.tabularWidth();
	if (tabwidth.zero()
	    && !(tabularWidthED->hasFocus() && tabularWidthED->text() == "0"))
		tabularWidthED->clear();
	else
		lengthToWidgets(tabularWidthED, tabularWidthUnitLC,
			tabwidth.asString(), default_unit);

	Length pwidth;
	docstring special;
	if (multicol) {
		special = getAlignSpecial(tabular, cell,
			Tabular::SET_SPECIAL_MULTICOLUMN);
		pwidth = getMColumnPWidth(tabular, cell);
	} else {
		special = getAlignSpecial(tabular, cell,
			Tabular::SET_SPECIAL_COLUMN);
		pwidth = getColumnPWidth(tabular, cell);
	}
	bool const varwidth = tabular.column_info[tabular.cellColumn(cell)].varwidth;
	if (varwidth)
		columnTypeCO->setCurrentIndex(1);
	string colwidth;
	if (pwidth.zero()
	    && !(columnWidthED->hasFocus() && columnWidthED->text() == "0")) {
		columnWidthED->clear();
		if (!varwidth)
			columnTypeCO->setCurrentIndex(0);
	} else {
		colwidth = pwidth.asString();
		lengthToWidgets(columnWidthED, columnWidthUnitLC,
			colwidth, default_unit);
		columnTypeCO->setCurrentIndex(2);
	}
	Length mroffset;
	if (multirow)
		mroffset = getMROffset(tabular, cell);
	string offset;
	if (mroffset.zero()
	    && !(multirowOffsetED->hasFocus() && multirowOffsetED->text() == "0"))
		multirowOffsetED->clear();
	else {
		offset = mroffset.asString();
		lengthToWidgets(multirowOffsetED, multirowOffsetUnitLC,
			offset, default_unit);
	}
	specialAlignmentED->setText(toqstr(special));
	///////////////////////////////////


	borderDefaultRB->setChecked(!tabular.use_booktabs);
	booktabsRB->setChecked(tabular.use_booktabs);

	if (tabular.row_info[row].top_space.empty()
	    && !tabular.row_info[row].top_space_default) {
		topspaceCO->setCurrentIndex(0);
	} else if (tabular.row_info[row].top_space_default) {
		topspaceCO->setCurrentIndex(1);
	} else {
		topspaceCO->setCurrentIndex(2);
		lengthToWidgets(topspaceED,
				topspaceUnitLC,
				tabular.row_info[row].top_space.asString(),
				default_unit);
	}

	if (tabular.row_info[row].bottom_space.empty()
	    && !tabular.row_info[row].bottom_space_default) {
		bottomspaceCO->setCurrentIndex(0);
	} else if (tabular.row_info[row].bottom_space_default) {
		bottomspaceCO->setCurrentIndex(1);
	} else {
		bottomspaceCO->setCurrentIndex(2);
		lengthToWidgets(bottomspaceED,
				bottomspaceUnitLC,
				tabular.row_info[row].bottom_space.asString(),
				default_unit);
	}

	if (tabular.row_info[row].interline_space.empty()
	    && !tabular.row_info[row].interline_space_default) {
		interlinespaceCO->setCurrentIndex(0);
	} else if (tabular.row_info[row].interline_space_default) {
		interlinespaceCO->setCurrentIndex(1);
	} else {
		interlinespaceCO->setCurrentIndex(2);
		lengthToWidgets(interlinespaceED,
				interlinespaceUnitLC,
				tabular.row_info[row].interline_space.asString(),
				default_unit);
	}

	hAlignCO->clear();
	hAlignCO->addItem(qt_("Left"), toqstr("left"));
	hAlignCO->addItem(qt_("Center"), toqstr("center"));
	hAlignCO->addItem(qt_("Right"), toqstr("right"));
	if (!multicol && !pwidth.zero())
		hAlignCO->addItem(qt_("Justified"), toqstr("justified"));
	if (!multicol && !multirow)
		hAlignCO->addItem(qt_("At Decimal Separator"), toqstr("decimal"));

	string align;
	switch (tabular.getAlignment(cell)) {
		case LYX_ALIGN_LEFT:
			align = "left";
			break;
		case LYX_ALIGN_CENTER:
			align = "center";
			break;
		case LYX_ALIGN_RIGHT:
			align = "right";
			break;
		case LYX_ALIGN_BLOCK:
		{
			if (!multicol && !pwidth.zero())
				align = "justified";
			break;
		}
		case LYX_ALIGN_DECIMAL:
		{
			if (!multicol && !multirow)
				align = "decimal";
			break;
		}
		default:
			// we should never end up here
			break;
	}
	hAlignCO->setCurrentIndex(hAlignCO->findData(toqstr(align)));

	//
	decimal_sep_ = tabular.column_info[col].decimal_point;
	if (decimal_sep_.empty()) {
		Language const * lang = itab->buffer().paragraphs().front().getParLanguage(itab->buffer().params());
		decimal_sep_ = lang->decimalSeparator();
	}
	decimalPointED->setText(toqstr(decimal_sep_));

	int valign = 0;
	switch (tabular.getVAlignment(cell)) {
	case Tabular::LYX_VALIGN_TOP:
		valign = 0;
		break;
	case Tabular::LYX_VALIGN_MIDDLE:
		valign = 1;
		break;
	case Tabular::LYX_VALIGN_BOTTOM:
		valign = 2;
		break;
	default:
		valign = 0;
		break;
	}
	if (pwidth.zero())
		valign = 0;
	vAlignCO->setCurrentIndex(valign);

	int tableValign = 1;
	switch (tabular.tabular_valignment) {
	case Tabular::LYX_VALIGN_TOP:
		tableValign = 0;
		break;
	case Tabular::LYX_VALIGN_MIDDLE:
		tableValign = 1;
		break;
	case Tabular::LYX_VALIGN_BOTTOM:
		tableValign = 2;
		break;
	default:
		tableValign = 0;
		break;
	}
	TableAlignCO->setCurrentIndex(tableValign);

	if (!tabular.is_long_tabular) {
		headerStatusCB->setChecked(false);
		headerBorderAboveCB->setChecked(false);
		headerBorderBelowCB->setChecked(false);
		firstheaderStatusCB->setChecked(false);
		firstheaderBorderAboveCB->setChecked(false);
		firstheaderBorderBelowCB->setChecked(false);
		firstheaderNoContentsCB->setChecked(false);
		footerStatusCB->setChecked(false);
		footerBorderAboveCB->setChecked(false);
		footerBorderBelowCB->setChecked(false);
		lastfooterStatusCB->setChecked(false);
		lastfooterBorderAboveCB->setChecked(false);
		lastfooterBorderBelowCB->setChecked(false);
		lastfooterNoContentsCB->setChecked(false);
		newpageCB->setChecked(false);
		captionStatusCB->blockSignals(true);
		captionStatusCB->setChecked(false);
		captionStatusCB->blockSignals(false);
		checkEnabled();
		// Store feature status
		features_ = getTabFeatures();
		return;
	} else {
		// longtables cannot have a vertical alignment
		TableAlignCO->setCurrentIndex(Tabular::LYX_VALIGN_MIDDLE);
	}
	switch (tabular.longtabular_alignment) {
	case Tabular::LYX_LONGTABULAR_ALIGN_LEFT:
		leftRB->setChecked(true);
		break;
	case Tabular::LYX_LONGTABULAR_ALIGN_CENTER:
		centerRB->setChecked(true);
		break;
	case Tabular::LYX_LONGTABULAR_ALIGN_RIGHT:
		rightRB->setChecked(true);
		break;
	default:
		centerRB->setChecked(true);
		break;
	}
	captionStatusCB->blockSignals(true);
	captionStatusCB->setChecked(tabular.ltCaption(row));
	captionStatusCB->blockSignals(false);

	Tabular::ltType ltt;
	bool use_empty;
	bool row_set = tabular.getRowOfLTHead(row, ltt);
	headerStatusCB->setChecked(row_set);
	if (ltt.set) {
		headerBorderAboveCB->setChecked(ltt.topDL);
		headerBorderBelowCB->setChecked(ltt.bottomDL);
		use_empty = true;
	} else {
		headerBorderAboveCB->setChecked(false);
		headerBorderBelowCB->setChecked(false);
		firstheaderNoContentsCB->setChecked(false);
		use_empty = false;
	}

	row_set = tabular.getRowOfLTFirstHead(row, ltt);
	firstheaderStatusCB->setChecked(row_set);
	if (ltt.set && (!ltt.empty || !use_empty)) {
		firstheaderBorderAboveCB->setChecked(ltt.topDL);
		firstheaderBorderBelowCB->setChecked(ltt.bottomDL);
	} else {
		firstheaderBorderAboveCB->setChecked(false);
		firstheaderBorderBelowCB->setChecked(false);
	}

	row_set = tabular.getRowOfLTFoot(row, ltt);
	footerStatusCB->setChecked(row_set);
	if (ltt.set) {
		footerBorderAboveCB->setChecked(ltt.topDL);
		footerBorderBelowCB->setChecked(ltt.bottomDL);
		use_empty = true;
	} else {
		footerBorderAboveCB->setChecked(false);
		footerBorderBelowCB->setChecked(false);
		lastfooterNoContentsCB->setChecked(false);
		use_empty = false;
	}

	row_set = tabular.getRowOfLTLastFoot(row, ltt);
	lastfooterStatusCB->setChecked(row_set);
	if (ltt.set && (!ltt.empty || !use_empty)) {
		lastfooterBorderAboveCB->setChecked(ltt.topDL);
		lastfooterBorderBelowCB->setChecked(ltt.bottomDL);
	} else {
		lastfooterBorderAboveCB->setChecked(false);
		lastfooterBorderBelowCB->setChecked(false);
	}
	newpageCB->setChecked(tabular.getLTNewPage(row));

	// first header can only be suppressed when there is a header
	firstheader_suppressable_ = tabular.haveLTHead()
			&& !tabular.haveLTFirstHead();
	// last footer can only be suppressed when there is a footer
	lastfooter_suppressable_ = tabular.haveLTFoot()
			&& !tabular.haveLTLastFoot();

	// after setting the features, check if they are enabled
	checkEnabled();

	// Finally, store feature status
	features_ = getTabFeatures();
}


bool GuiTabular::checkWidgets(bool readonly) const
{
	tabularRowED->setReadOnly(readonly);
	tabularColumnED->setReadOnly(readonly);
	tabularWidthED->setReadOnly(readonly);
	specialAlignmentED->setReadOnly(readonly);
	columnWidthED->setReadOnly(readonly);
	multirowOffsetED->setReadOnly(readonly);
	decimalPointED->setReadOnly(readonly);

	if (readonly) {
		multicolumnCB->setEnabled(false);
		multirowCB->setEnabled(false);
		rotateCellCB->setEnabled(false);
		rotateCellAngleSB->setEnabled(false);
		rotateTabularCB->setEnabled(false);
		rotateTabularAngleSB->setEnabled(false);
		longTabularCB->setEnabled(false);
		borders->setEnabled(false);
		tabularWidthUnitLC->setEnabled(false);
		columnWidthUnitLC->setEnabled(false);
		columnTypeCO->setEnabled(false);
		multirowOffsetUnitLC->setEnabled(false);
		setBordersGB->setEnabled(false);
		allBordersGB->setEnabled(false);
		borderStyleGB->setEnabled(false);
		booktabsRB->setEnabled(false);
		topspaceCO->setEnabled(false);
		topspaceUnitLC->setEnabled(false);
		bottomspaceCO->setEnabled(false);
		bottomspaceUnitLC->setEnabled(false);
		interlinespaceCO->setEnabled(false);
		interlinespaceUnitLC->setEnabled(false);
		hAlignCO->setEnabled(false);
		vAlignCO->setEnabled(false);
		TableAlignCO->setEnabled(false);
		longtableGB->setEnabled(false);
		alignmentGB->setEnabled(false);
	} else
		enableWidgets();

	return InsetParamsWidget::checkWidgets();
}


bool GuiTabular::funcEnabled(Tabular::Feature f) const
{
	FuncRequest r(LFUN_INSET_MODIFY, "tabular for-dialog" + featureAsString(f));
	return getStatus(r).enabled();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabular.cpp"
