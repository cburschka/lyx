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
	connect(longTabularCB, SIGNAL(toggled(bool)),
		longtableGB, SLOT(setEnabled(bool)));
	connect(longTabularCB, SIGNAL(toggled(bool)),
		newpageCB, SLOT(setEnabled(bool)));
	connect(longTabularCB, SIGNAL(toggled(bool)),
		alignmentGB, SLOT(setEnabled(bool)));
	// longtables cannot have a vertical alignment
	connect(longTabularCB, SIGNAL(toggled(bool)),
		TableAlignCB, SLOT(setDisabled(bool)));
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
	topspaceED->setEnabled(topspaceCO->currentIndex() == 2);
	topspaceUnit->setEnabled(topspaceCO->currentIndex() == 2);
	bottomspaceED->setEnabled(bottomspaceCO->currentIndex() == 2);
	bottomspaceUnit->setEnabled(bottomspaceCO->currentIndex() == 2);
	interlinespaceED->setEnabled(interlinespaceCO->currentIndex() == 2);
	interlinespaceUnit->setEnabled(interlinespaceCO->currentIndex() == 2);
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


// to get the status of the longtable row settings
static bool funcEnabled(Tabular::Feature f)
{
	return getStatus(
		FuncRequest(LFUN_INSET_MODIFY, featureAsString(f))).enabled();
}


void GuiTabular::setHAlign(string & param_str) const
{
	int const align = hAlignCB->currentIndex();

	enum HALIGN { LEFT, RIGHT, CENTER, BLOCK };
	HALIGN h = LEFT;

	switch (align) {
		case 0: h = LEFT; break;
		case 1: h = CENTER; break;
		case 2: h = RIGHT; break;
		case 3: h = BLOCK; break;
	}

	Tabular::Feature num = Tabular::ALIGN_LEFT;
	Tabular::Feature multi_num = Tabular::M_ALIGN_LEFT;

	switch (h) {
		case LEFT:
			num = Tabular::ALIGN_LEFT;
			multi_num = Tabular::M_ALIGN_LEFT;
			break;
		case CENTER:
			num = Tabular::ALIGN_CENTER;
			multi_num = Tabular::M_ALIGN_CENTER;
			break;
		case RIGHT:
			num = Tabular::ALIGN_RIGHT;
			multi_num = Tabular::M_ALIGN_RIGHT;
			break;
		case BLOCK:
			num = Tabular::ALIGN_BLOCK;
			//multi_num: no equivalent
			break;
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
		if (borders->getLeft())
			setParam(param_str, Tabular::TOGGLE_LINE_LEFT);
		if (borders->getRight())
			setParam(param_str, Tabular::TOGGLE_LINE_RIGHT);
		if (borders->getTop())
			setParam(param_str, Tabular::TOGGLE_LINE_TOP);
		if (borders->getBottom())
			setParam(param_str, Tabular::TOGGLE_LINE_BOTTOM);
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

	//
	borders->setTop(tabular.topLine(cell));
	borders->setBottom(tabular.bottomLine(cell));
	borders->setLeft(tabular.leftLine(cell));
	borders->setRight(tabular.rightLine(cell));
	// repaint the setborder widget
	borders->update();

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

	specialAlignmentED->setText(toqstr(special));

	Length::UNIT const default_unit = Length::defaultUnit();

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
	string colwidth;
	if (!pwidth.zero()) {
		colwidth = pwidth.asString();

		lengthToWidgets(widthED, widthUnitCB,
			colwidth, default_unit);
	}

	hAlignCB->clear();
	hAlignCB->addItem(qt_("Left"));
	hAlignCB->addItem(qt_("Center"));
	hAlignCB->addItem(qt_("Right"));
	if (!multicol && !pwidth.zero())
		hAlignCB->addItem(qt_("Justified"));

	int align = 0;
	switch (tabular.getAlignment(cell)) {
	case LYX_ALIGN_LEFT:
		align = 0;
		break;
	case LYX_ALIGN_CENTER:
		align = 1;
		break;
	case LYX_ALIGN_RIGHT:
		align = 2;
		break;
	case LYX_ALIGN_BLOCK:
	{
		if (!multicol && !pwidth.zero())
			align = 3;
		break;
	}
	default:
		align = 0;
		break;
	}
	hAlignCB->setCurrentIndex(align);

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

	hAlignCB->setEnabled(true);
	if (!multirow && !pwidth.zero())
	vAlignCB->setEnabled(true);

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
		newpageCB->setEnabled(false);
		captionStatusCB->blockSignals(true);
		captionStatusCB->setChecked(false);
		captionStatusCB->blockSignals(false);
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

	// FIXME: shouldn't this be handled by GuiDialog?
	// FIXME: Some of them should be handled directly in TabularUI.ui
	firstheaderBorderAboveCB->setEnabled(
		funcEnabled(Tabular::SET_LTFIRSTHEAD));
	firstheaderBorderBelowCB->setEnabled(
		funcEnabled(Tabular::SET_LTFIRSTHEAD));
	// first header can only be suppressed when there is a header
	firstheaderNoContentsCB->setEnabled(tabular.haveLTHead()
		&& !tabular.haveLTFirstHead());

	//firstheaderStatusCB->setEnabled(
	//	!firstheaderNoContentsCB->isChecked());
	headerBorderAboveCB->setEnabled(funcEnabled(Tabular::SET_LTHEAD));
	headerBorderBelowCB->setEnabled(funcEnabled(Tabular::SET_LTHEAD));
	headerStatusCB->setEnabled(funcEnabled(Tabular::SET_LTHEAD));

	footerBorderAboveCB->setEnabled(funcEnabled(Tabular::SET_LTFOOT));
	footerBorderBelowCB->setEnabled(funcEnabled(Tabular::SET_LTFOOT));
	footerStatusCB->setEnabled(funcEnabled(Tabular::SET_LTFOOT));

	lastfooterBorderAboveCB->setEnabled(
		funcEnabled(Tabular::SET_LTLASTFOOT));
	lastfooterBorderBelowCB->setEnabled(
		funcEnabled(Tabular::SET_LTLASTFOOT));
	// last footer can only be suppressed when there is a footer
	lastfooterNoContentsCB->setEnabled(tabular.haveLTFoot()
		&& !tabular.haveLTLastFoot());

	captionStatusCB->setEnabled(
		funcEnabled(Tabular::TOGGLE_LTCAPTION));
	// When a row is set as longtable caption, it must not be allowed
	// to unset that this row is a multicolumn.
	multicolumnCB->setEnabled(funcEnabled(Tabular::MULTICOLUMN));
	multirowCB->setEnabled(funcEnabled(Tabular::MULTIROW));

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
		headerBorderAboveCB->setEnabled(false);
		headerBorderBelowCB->setEnabled(false);
		firstheaderNoContentsCB->setChecked(false);
		firstheaderNoContentsCB->setEnabled(false);
		use_empty = false;
	}

	row_set = tabular.getRowOfLTFirstHead(row, ltt);
	// check if setting a first header is allowed
	// additionally check firstheaderNoContentsCB because when this is
	// the case a first header makes no sense
	firstheaderStatusCB->setEnabled(
		funcEnabled(Tabular::SET_LTFIRSTHEAD)
		&& !firstheaderNoContentsCB->isChecked());
	firstheaderStatusCB->setChecked(row_set);
	if (ltt.set && (!ltt.empty || !use_empty)) {
		firstheaderBorderAboveCB->setChecked(ltt.topDL);
		firstheaderBorderBelowCB->setChecked(ltt.bottomDL);
	} else {
		firstheaderBorderAboveCB->setEnabled(false);
		firstheaderBorderBelowCB->setEnabled(false);
		firstheaderBorderAboveCB->setChecked(false);
		firstheaderBorderBelowCB->setChecked(false);
		if (use_empty) {
			if (ltt.empty)
				firstheaderStatusCB->setEnabled(false);
		}
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
		footerBorderAboveCB->setEnabled(false);
		footerBorderBelowCB->setEnabled(false);
		lastfooterNoContentsCB->setChecked(false);
		lastfooterNoContentsCB->setEnabled(false);
		use_empty = false;
	}

	row_set = tabular.getRowOfLTLastFoot(row, ltt);
	// check if setting a last footer is allowed
	// additionally check lastfooterNoContentsCB because when this is
	// the case a last footer makes no sense
	lastfooterStatusCB->setEnabled(
		funcEnabled(Tabular::SET_LTLASTFOOT)
		&& !lastfooterNoContentsCB->isChecked());
	lastfooterStatusCB->setChecked(row_set);
	if (ltt.set && (!ltt.empty || !use_empty)) {
		lastfooterBorderAboveCB->setChecked(ltt.topDL);
		lastfooterBorderBelowCB->setChecked(ltt.bottomDL);
	} else {
		lastfooterBorderAboveCB->setEnabled(false);
		lastfooterBorderBelowCB->setEnabled(false);
		lastfooterBorderAboveCB->setChecked(false);
		lastfooterBorderBelowCB->setChecked(false);
		if (use_empty) {
			if (ltt.empty)
				lastfooterStatusCB->setEnabled(false);
		}
	}
	newpageCB->setChecked(tabular.getLTNewPage(row));
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabular.cpp"
