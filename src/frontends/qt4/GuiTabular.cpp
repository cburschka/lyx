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

#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"
#include "LyXRC.h"

#include "insets/InsetTabular.h"

#include "support/debug.h"

#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

using namespace std;

namespace lyx {
namespace frontend {

GuiTabular::GuiTabular(QWidget * parent)
	: InsetParamsWidget(parent)
{
	setupUi(this);

	widthED->setValidator(unsignedLengthValidator(widthED));
	topspaceED->setValidator(new LengthValidator(topspaceED));
	bottomspaceED->setValidator(new LengthValidator(bottomspaceED));
	interlinespaceED->setValidator(new LengthValidator(interlinespaceED));

	widthUnitCB->setCurrentItem(Length::defaultUnit());

	connect(topspaceED, SIGNAL(returnPressed()),
		this, SLOT(checkEnabled()));
	connect(topspaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(topspaceCO, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(bottomspaceED, SIGNAL(returnPressed()),
		this, SLOT(checkEnabled()));
	connect(bottomspaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(bottomspaceCO, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(interlinespaceED, SIGNAL(returnPressed()),
		this, SLOT(checkEnabled()));
	connect(interlinespaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(interlinespaceCO, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(booktabsRB, SIGNAL(clicked(bool)),
		this, SLOT(checkEnabled()));
	connect(borderDefaultRB, SIGNAL(clicked(bool)),
		this, SLOT(checkEnabled()));
	connect(borderSetPB, SIGNAL(clicked()),
		this, SLOT(borderSet_clicked()));
	connect(borderUnsetPB, SIGNAL(clicked()), 
		this, SLOT(borderUnset_clicked()));
	connect(hAlignCB, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(vAlignCB, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(multicolumnCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(multirowCB, SIGNAL(clicked()),
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
	connect(specialAlignmentED, SIGNAL(returnPressed()),
		this, SLOT(checkEnabled()));
	connect(widthED, SIGNAL(textEdited(QString)),
		this, SLOT(checkEnabled()));
	connect(widthUnitCB, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(topSet(bool)),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(bottomSet(bool)),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(rightSet(bool)),
		this, SLOT(checkEnabled()));
	connect(borders, SIGNAL(leftSet(bool)),
		this, SLOT(checkEnabled()));
	connect(rotateTabularCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(rotateCellCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(TableAlignCB, SIGNAL(activated(int)),
		this, SLOT(checkEnabled()));
	connect(longTabularCB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(leftRB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(centerRB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
	connect(rightRB, SIGNAL(clicked()),
		this, SLOT(checkEnabled()));
		
	
	// initialize the length validator
	addCheckedWidget(widthED, fixedWidthColLA);
	addCheckedWidget(topspaceED, topspaceLA);
	addCheckedWidget(bottomspaceED, bottomspaceLA);
	addCheckedWidget(interlinespaceED, interlinespaceLA);
}


void GuiTabular::checkEnabled()
{
	hAlignCB->setEnabled(true);
	bool dalign =
		hAlignCB->itemData(hAlignCB->currentIndex()).toString() == QString("decimal");
	decimalPointLE->setEnabled(dalign);
	decimalL->setEnabled(dalign);

	vAlignCB->setEnabled(!multirowCB->isChecked() 
		&& !widgetsToLength(widthED, widthUnitCB).empty());

	topspaceED->setEnabled(topspaceCO->currentIndex() == 2);
	topspaceUnit->setEnabled(topspaceCO->currentIndex() == 2);
	bottomspaceED->setEnabled(bottomspaceCO->currentIndex() == 2);
	bottomspaceUnit->setEnabled(bottomspaceCO->currentIndex() == 2);
	interlinespaceED->setEnabled(interlinespaceCO->currentIndex() == 2);
	interlinespaceUnit->setEnabled(interlinespaceCO->currentIndex() == 2);

	bool const longtabular = longTabularCB->isChecked();
	longtableGB->setEnabled(true);
	newpageCB->setEnabled(longtabular);
	alignmentGB->setEnabled(longtabular);
	// longtables cannot have a vertical alignment
	TableAlignCB->setDisabled(longtabular);

	// FIXME: This Dialog is really horrible, disabling/enabling a checkbox
	// depending on the cursor position is very very unintuitive...
	// We need some edit boxes to show which rows are header/footer/etc
	// without having to move the cursor first.
	headerStatusCB->setEnabled(longtabular);
	headerBorderAboveCB->setEnabled(longtabular
		&& headerStatusCB->isChecked());
	headerBorderBelowCB->setEnabled(longtabular
		&& headerStatusCB->isChecked());

	// first header can only be suppressed when there is a header
	// FIXME: old code also checked for
	//  tabular.haveLTHead() && !tabular.haveLTFirstHead());
	firstheaderNoContentsCB->setEnabled(longtabular);
	// check if setting a first header is allowed
	// additionally check firstheaderNoContentsCB because when this is
	// the case a first header makes no sense
	firstheaderStatusCB->setEnabled(funcEnabled(Tabular::SET_LTFIRSTHEAD)
		&& longtabular && !firstheaderNoContentsCB->isChecked());
	firstheaderBorderAboveCB->setEnabled(longtabular
		&& firstheaderStatusCB->isChecked());
	firstheaderBorderBelowCB->setEnabled(longtabular
		&& firstheaderStatusCB->isChecked());

	footerStatusCB->setEnabled(longtabular);
	footerBorderAboveCB->setEnabled(longtabular
		&& footerBorderAboveCB->isChecked());
	footerBorderBelowCB->setEnabled(longtabular
		&& footerBorderAboveCB->isChecked());

	// last footer can only be suppressed when there is a footer
	// FIXME: old code also checked for
	//   tabular.haveLTFoot() && !tabular.haveLTLastFoot());
	lastfooterNoContentsCB->setEnabled(longtabular);
	// check if setting a last footer is allowed
	// additionally check lastfooterNoContentsCB because when this is
	// the case a last footer makes no sense
	lastfooterStatusCB->setEnabled(funcEnabled(Tabular::SET_LTLASTFOOT)
		&& longtabular && !lastfooterNoContentsCB->isChecked());
	lastfooterBorderAboveCB->setEnabled(longtabular
		&& lastfooterBorderAboveCB->isChecked());
	lastfooterBorderBelowCB->setEnabled(longtabular
		&& lastfooterBorderAboveCB->isChecked());

	captionStatusCB->setEnabled(funcEnabled(Tabular::TOGGLE_LTCAPTION));
	
	multicolumnCB->setEnabled(funcEnabled(Tabular::MULTICOLUMN));
	multirowCB->setEnabled(funcEnabled(Tabular::MULTIROW));

	changed();
}


void GuiTabular::borderSet_clicked()
{
	borders->setTop(true);
	borders->setBottom(true);
	borders->setLeft(true);
	borders->setRight(true);
	// repaint the setborder widget
	borders->update();
	checkEnabled();
}


void GuiTabular::borderUnset_clicked()
{
	borders->setTop(false);
	borders->setBottom(false);
	borders->setLeft(false);
	borders->setRight(false);
	// repaint the setborder widget
	borders->update();
	checkEnabled();
}


static void setParam(string & param_str, Tabular::Feature f, string const & arg = string())
{
	param_str += ' ';
	param_str += featureAsString(f) + ' ' + arg;
}


void GuiTabular::setHAlign(string & param_str) const
{
	Tabular::Feature num = Tabular::ALIGN_LEFT;
	Tabular::Feature multi_num = Tabular::M_ALIGN_LEFT;
	string const align = 
		fromqstr(hAlignCB->itemData(hAlignCB->currentIndex()).toString());
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
		setParam(param_str, multi_num);
	else
		setParam(param_str, num);
}


void GuiTabular::setVAlign(string & param_str) const
{
	int const align = vAlignCB->currentIndex();
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
		setParam(param_str, multi_num);
	else
		setParam(param_str, num);
}


void GuiTabular::setTableAlignment(string & param_str) const
{
	int const align = TableAlignCB->currentIndex();
	switch (align) {
		case 0: setParam(param_str, Tabular::TABULAR_VALIGN_TOP);
			break;
		case 1: setParam(param_str, Tabular::TABULAR_VALIGN_MIDDLE);
			break;
		case 2: setParam(param_str, Tabular::TABULAR_VALIGN_BOTTOM);
			break;
	}
}


docstring GuiTabular::dialogToParams() const
{
	// FIXME: We should use Tabular directly.
	string param_str = "tabular";
	setHAlign(param_str);

	// SET_DECIMAL_POINT must come after setHAlign() (ALIGN_DECIMAL)
	string decimal_point = fromqstr(decimalPointLE->text());
	if (decimal_point.empty())
		decimal_point = lyxrc.default_decimal_point;
	setParam(param_str, Tabular::SET_DECIMAL_POINT, decimal_point);

	setVAlign(param_str);
	setTableAlignment(param_str);
	//
	if (booktabsRB->isChecked())
		setParam(param_str, Tabular::SET_BOOKTABS);
	else
		setParam(param_str, Tabular::UNSET_BOOKTABS);

	//
	switch (topspaceCO->currentIndex()) {
		case 0:
			setParam(param_str, Tabular::SET_TOP_SPACE, "none");
			break;
		case 1:
			setParam(param_str, Tabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			if (!topspaceED->text().isEmpty())
				setParam(param_str, Tabular::SET_TOP_SPACE,
					 widgetsToLength(topspaceED, topspaceUnit));
			break;
	}

	//
	switch (bottomspaceCO->currentIndex()) {
		case 0:
			setParam(param_str, Tabular::SET_BOTTOM_SPACE, "none");
			break;
		case 1:
			setParam(param_str, Tabular::SET_BOTTOM_SPACE, "default");
			break;
		case 2:
			if (!bottomspaceED->text().isEmpty())
				setParam(param_str, Tabular::SET_BOTTOM_SPACE,
					widgetsToLength(bottomspaceED,
							bottomspaceUnit));
			break;
	}

	//
	switch (interlinespaceCO->currentIndex()) {
		case 0:
			setParam(param_str, Tabular::SET_INTERLINE_SPACE, "none");
			break;
		case 1:
			setParam(param_str, Tabular::SET_INTERLINE_SPACE, "default");
			break;
		case 2:
			if (!interlinespaceED->text().isEmpty())
				setParam(param_str, Tabular::SET_INTERLINE_SPACE,
					widgetsToLength(interlinespaceED,
							interlinespaceUnit));
			break;
	}

	//
	if (borders->getTop() && borders->getBottom() && borders->getLeft()
		&& borders->getRight())
		setParam(param_str, Tabular::SET_ALL_LINES);
	else if (!borders->getTop() && !borders->getBottom() && !borders->getLeft()
		&& !borders->getRight())
		setParam(param_str, Tabular::UNSET_ALL_LINES);
	else {
		setParam(param_str, Tabular::SET_LINE_LEFT,
			 borders->getLeft() ? "true" : "false");
		setParam(param_str, Tabular::SET_LINE_RIGHT,
			 borders->getRight() ? "true" : "false");
		setParam(param_str, Tabular::SET_LINE_TOP,
			 borders->getTop() ? "true" : "false");
		setParam(param_str, Tabular::SET_LINE_BOTTOM,
			 borders->getBottom() ? "true" : "false");
	}

	// apply the special alignment
	string special = fromqstr(specialAlignmentED->text());
	if (special.empty())
		special = "none";
	if (multicolumnCB->isChecked())
		setParam(param_str, Tabular::SET_SPECIAL_MULTICOLUMN, special);
	else if (multirowCB->isChecked())
		setParam(param_str, Tabular::SET_SPECIAL_MULTIROW, special);
	else
		setParam(param_str, Tabular::SET_SPECIAL_COLUMN, special);

	// apply the fixed width values
	string width = widgetsToLength(widthED, widthUnitCB);
	if (width.empty())
		width = "0pt";
	if (multicolumnCB->isChecked())
		setParam(param_str, Tabular::SET_MPWIDTH, width);
	else
		setParam(param_str, Tabular::SET_PWIDTH, width);

	//
	if (multicolumnCB->isChecked())
		setParam(param_str, Tabular::MULTICOLUMN);
	//
	if (multirowCB->isChecked())
		setParam(param_str, Tabular::MULTIROW);
	//
	if (rotateTabularCB->isChecked())
		setParam(param_str, Tabular::SET_ROTATE_TABULAR);
	else
		setParam(param_str, Tabular::UNSET_ROTATE_TABULAR);
	//
	if (rotateCellCB->isChecked())
		setParam(param_str, Tabular::SET_ROTATE_CELL);
	else
		setParam(param_str, Tabular::UNSET_ROTATE_CELL);
	//
	if (longTabularCB->isChecked())
		setParam(param_str, Tabular::SET_LONGTABULAR);
	else
		setParam(param_str, Tabular::UNSET_LONGTABULAR);
	//
	if (newpageCB->isChecked())
		setParam(param_str, Tabular::SET_LTNEWPAGE);
	//
	if (captionStatusCB->isChecked())
		setParam(param_str, Tabular::TOGGLE_LTCAPTION);
	//
	if (headerStatusCB->isChecked())
		setParam(param_str, Tabular::SET_LTHEAD, "none");
	else
		setParam(param_str, Tabular::UNSET_LTHEAD, "none");
	//
	if (headerBorderAboveCB->isChecked())
		setParam(param_str, Tabular::SET_LTHEAD, "dl_above");
	else
		setParam(param_str, Tabular::UNSET_LTHEAD, "dl_above");
	//
	if (headerBorderBelowCB->isChecked())
		setParam(param_str, Tabular::SET_LTHEAD, "dl_below");
	else
		setParam(param_str, Tabular::UNSET_LTHEAD, "dl_below");
	if (firstheaderBorderAboveCB->isChecked())
		setParam(param_str, Tabular::SET_LTFIRSTHEAD, "dl_above");
	else
		setParam(param_str, Tabular::UNSET_LTFIRSTHEAD, "dl_above");
	if (firstheaderBorderBelowCB->isChecked())
		setParam(param_str, Tabular::SET_LTFIRSTHEAD, "dl_below");
	else
		setParam(param_str, Tabular::UNSET_LTFIRSTHEAD, "dl_below");
	if (firstheaderStatusCB->isChecked())
		setParam(param_str, Tabular::SET_LTFIRSTHEAD, "none");
	else
		setParam(param_str, Tabular::UNSET_LTFIRSTHEAD, "none");
	if (firstheaderNoContentsCB->isChecked())
		setParam(param_str, Tabular::SET_LTFIRSTHEAD, "empty");
	else
		setParam(param_str, Tabular::UNSET_LTFIRSTHEAD, "empty");
	if (footerStatusCB->isChecked())
		setParam(param_str, Tabular::SET_LTFOOT, "none");
	else
		setParam(param_str, Tabular::UNSET_LTFOOT, "none");
	if (footerBorderAboveCB->isChecked())
		setParam(param_str, Tabular::SET_LTFOOT, "dl_above");
	else
		setParam(param_str, Tabular::UNSET_LTFOOT, "dl_above");
	if (footerBorderBelowCB->isChecked())
		setParam(param_str, Tabular::SET_LTFOOT, "dl_below");
	else
		setParam(param_str, Tabular::UNSET_LTFOOT, "dl_below");
	if (lastfooterStatusCB->isChecked())
		setParam(param_str, Tabular::SET_LTLASTFOOT, "none");
	else
		setParam(param_str, Tabular::UNSET_LTLASTFOOT, "none");
	if (lastfooterBorderAboveCB->isChecked())
		setParam(param_str, Tabular::SET_LTLASTFOOT, "dl_above");
	else
		setParam(param_str, Tabular::UNSET_LTLASTFOOT, "dl_above");
	if (lastfooterBorderBelowCB->isChecked())
		setParam(param_str, Tabular::SET_LTLASTFOOT, "dl_below");
	else
		setParam(param_str, Tabular::UNSET_LTLASTFOOT, "dl_below");
	if (lastfooterNoContentsCB->isChecked())
		setParam(param_str, Tabular::SET_LTLASTFOOT, "empty");
	else
		setParam(param_str, Tabular::UNSET_LTLASTFOOT, "empty");

	if (leftRB->isChecked())
		setParam(param_str, Tabular::LONGTABULAR_ALIGN_LEFT);
	else if (centerRB->isChecked())
		setParam(param_str, Tabular::LONGTABULAR_ALIGN_CENTER);
	else if (rightRB->isChecked())
		setParam(param_str, Tabular::LONGTABULAR_ALIGN_RIGHT);

	return from_utf8(param_str);
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


static docstring getAlignSpecial(Tabular const & t, size_t cell, int what)
{
	if (what == Tabular::SET_SPECIAL_MULTICOLUMN
		|| what == Tabular::SET_SPECIAL_MULTIROW)
		return t.cellInfo(cell).align_special;
	return t.column_info[t.cellColumn(cell)].align_special;
}


void GuiTabular::paramsToDialog(Inset const * inset)
{
	InsetTabular const * itab = static_cast<InsetTabular const *>(inset);
	// Copy Tabular of current inset.
	Tabular const & tabular = itab->tabular;

	BufferView const * bv = guiApp->currentView()->currentBufferView();
	size_t const cell = bv->cursor().idx();

	Tabular::row_type const row = tabular.cellRow(cell);
	Tabular::col_type const col = tabular.cellColumn(cell);

	tabularRowED->setText(QString::number(row + 1));
	tabularColumnED->setText(QString::number(col + 1));

	bool const multicol = tabular.isMultiColumn(cell);
	multicolumnCB->setChecked(multicol);

	bool const multirow = tabular.isMultiRow(cell);
	multirowCB->setChecked(multirow);

	rotateCellCB->setChecked(tabular.getRotateCell(cell));
	rotateTabularCB->setChecked(tabular.rotate);

	longTabularCB->setChecked(tabular.is_long_tabular);

	borders->setTop(tabular.topLine(cell));
	borders->setBottom(tabular.bottomLine(cell));
	borders->setLeft(tabular.leftLine(cell));
	borders->setRight(tabular.rightLine(cell));
	// repaint the setborder widget
	borders->update();

	Length::UNIT const default_unit = Length::defaultUnit();

	///////////////////////////////////
	// Set width and alignment
	Length pwidth;
	docstring special;
	if (multicol) {
		special = getAlignSpecial(tabular, cell,
			Tabular::SET_SPECIAL_MULTICOLUMN);
		pwidth = getMColumnPWidth(tabular, cell);
	} else if (multirow) {
		special = getAlignSpecial(tabular, cell,
			Tabular::SET_SPECIAL_MULTIROW);
		pwidth = getMColumnPWidth(tabular, cell);
	} else {
		special = getAlignSpecial(tabular, cell,
			Tabular::SET_SPECIAL_COLUMN);
		pwidth = getColumnPWidth(tabular, cell);
	}
	string colwidth;
	if (pwidth.zero())
		widthED->clear();
	else {
		colwidth = pwidth.asString();
		lengthToWidgets(widthED, widthUnitCB,
			colwidth, default_unit);
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
				topspaceUnit,
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
				bottomspaceUnit,
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
				interlinespaceUnit,
				tabular.row_info[row].interline_space.asString(),
				default_unit);
	}

	hAlignCB->clear();
	hAlignCB->addItem(qt_("Left"), toqstr("left"));
	hAlignCB->addItem(qt_("Center"), toqstr("center"));
	hAlignCB->addItem(qt_("Right"), toqstr("right"));
	if (!multicol && !pwidth.zero())
		hAlignCB->addItem(qt_("Justified"), toqstr("justified"));
	if (!multicol)
		hAlignCB->addItem(qt_("At Decimal Separator"), toqstr("decimal"));

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
			if (!multicol)
				align = "decimal";
			break;
		}
		default:
			// we should never end up here
			break;
	}
	hAlignCB->setCurrentIndex(hAlignCB->findData(toqstr(align)));

	//
	QString decimal_point = toqstr(tabular.column_info[col].decimal_point);
	if (decimal_point.isEmpty())
		decimal_point = toqstr(from_utf8(lyxrc.default_decimal_point));
	decimalPointLE->setText(decimal_point);

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
	vAlignCB->setCurrentIndex(valign);

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
	TableAlignCB->setCurrentIndex(tableValign);

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
		return;
	} else {
		// longtables cannot have a vertical alignment
		TableAlignCB->setCurrentIndex(Tabular::LYX_VALIGN_MIDDLE);
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
}


bool GuiTabular::funcEnabled(Tabular::Feature f) const
{
	string cmd = "tabular " + featureAsString(f);
	return getStatus(FuncRequest(LFUN_INSET_MODIFY, cmd)).enabled();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabular.cpp"
