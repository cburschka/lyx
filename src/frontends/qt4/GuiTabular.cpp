/**
 * \file GuiTabular.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiTabular.h"

#include "ControlTabular.h"
#include "GuiSetBorder.h"
#include "LengthCombo.h"
#include "Validator.h"
#include "qt_helpers.h"

#include "support/convert.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

using std::string;


namespace lyx {
namespace frontend {

GuiTabularDialog::GuiTabularDialog(LyXView & lv)
	: GuiDialog(lv, "tabular")
{
	setupUi(this);
	setViewTitle(_("Table Settings"));
	setController(new ControlTabular(*this));

	widthED->setValidator(unsignedLengthValidator(widthED));
	topspaceED->setValidator(new LengthValidator(topspaceED));
	bottomspaceED->setValidator(new LengthValidator(bottomspaceED));
	interlinespaceED->setValidator(new LengthValidator(interlinespaceED));

	connect(topspaceED, SIGNAL(returnPressed()),
		this, SLOT(topspace_changed()));
	connect(topspaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(topspace_changed()));
	connect(topspaceCO, SIGNAL(activated(int)), this, SLOT(topspace_changed()));
	connect(bottomspaceED, SIGNAL(returnPressed()),
		this, SLOT(bottomspace_changed()));
	connect(bottomspaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(bottomspace_changed()));
	connect(bottomspaceCO, SIGNAL(activated(int)), this, SLOT(bottomspace_changed()));
	connect(interlinespaceED, SIGNAL(returnPressed()),
		this, SLOT(interlinespace_changed()));
	connect(interlinespaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(interlinespace_changed()));
	connect(interlinespaceCO, SIGNAL(activated(int)), this, SLOT(interlinespace_changed()));
	connect(booktabsRB, SIGNAL(clicked(bool)), this, SLOT(booktabsChanged(bool)));
	connect(borderDefaultRB, SIGNAL(clicked(bool)), this, SLOT(booktabsChanged(bool)));
	connect(borderSetPB, SIGNAL(clicked()), this, SLOT(borderSet_clicked()));
	connect(borderUnsetPB, SIGNAL(clicked()), this, SLOT(borderUnset_clicked()));
	connect(longTabularCB, SIGNAL(toggled(bool)), longtableGB, SLOT(setEnabled(bool)));
	connect(longTabularCB, SIGNAL(toggled(bool)), newpageCB, SLOT(setEnabled(bool)));
	connect(hAlignCB, SIGNAL(activated(int)), this, SLOT(hAlign_changed(int)));
	connect(vAlignCB, SIGNAL(activated(int)), this, SLOT(vAlign_changed(int)));
	connect(multicolumnCB, SIGNAL(clicked()), this, SLOT(multicolumn_clicked()));
	connect(newpageCB, SIGNAL(clicked()), this, SLOT(ltNewpage_clicked()));
	connect(headerStatusCB, SIGNAL(clicked()), this, SLOT(ltHeaderStatus_clicked()));
	connect(headerBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltHeaderBorderAbove_clicked()));
	connect(headerBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltHeaderBorderBelow_clicked()));
	connect(firstheaderStatusCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderStatus_clicked()));
	connect(firstheaderBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderBorderAbove_clicked()));
	connect(firstheaderBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderBorderBelow_clicked()));
	connect(firstheaderNoContentsCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderEmpty_clicked()));
	connect(footerStatusCB, SIGNAL(clicked()), this, SLOT(ltFooterStatus_clicked()));
	connect(footerBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltFooterBorderAbove_clicked()));
	connect(footerBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltFooterBorderBelow_clicked()));
	connect(lastfooterStatusCB, SIGNAL(clicked()), this, SLOT(ltLastFooterStatus_clicked()));
	connect(lastfooterBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltLastFooterBorderAbove_clicked()));
	connect(lastfooterBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltLastFooterBorderBelow_clicked()));
	connect(lastfooterNoContentsCB, SIGNAL(clicked()), this, SLOT(ltLastFooterEmpty_clicked()));
	connect(specialAlignmentED, SIGNAL(returnPressed()), this, SLOT(specialAlignment_changed()));
	connect(widthED, SIGNAL(returnPressed()), this, SLOT(width_changed()));
	connect(widthUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)), this, SLOT(width_changed()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(close_clicked()));
	connect(borders, SIGNAL(topSet(bool)), this, SLOT(topBorder_changed()));
	connect(borders, SIGNAL(bottomSet(bool)), this, SLOT(bottomBorder_changed()));
	connect(borders, SIGNAL(rightSet(bool)), this, SLOT(rightBorder_changed()));
	connect(borders, SIGNAL(leftSet(bool)), this, SLOT(leftBorder_changed()));
	connect(rotateTabularCB, SIGNAL(clicked()), this, SLOT(rotateTabular()));
	connect(rotateCellCB, SIGNAL(clicked()), this, SLOT(rotateCell()));
	connect(longTabularCB, SIGNAL(clicked()), this, SLOT(longTabular()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);

	bc().addReadOnly(topspaceED);
	bc().addReadOnly(topspaceUnit);
	bc().addReadOnly(topspaceCO);
	bc().addReadOnly(bottomspaceED);
	bc().addReadOnly(bottomspaceUnit);
	bc().addReadOnly(bottomspaceCO);
	bc().addReadOnly(interlinespaceED);
	bc().addReadOnly(interlinespaceUnit);
	bc().addReadOnly(interlinespaceCO);
	bc().addReadOnly(borderDefaultRB);
	bc().addReadOnly(booktabsRB);

	bc().addReadOnly(multicolumnCB);
	bc().addReadOnly(rotateCellCB);
	bc().addReadOnly(rotateTabularCB);
	bc().addReadOnly(specialAlignmentED);
	bc().addReadOnly(widthED);
	bc().addReadOnly(widthUnit);
	bc().addReadOnly(hAlignCB);
	bc().addReadOnly(vAlignCB);
	bc().addReadOnly(borderSetPB);
	bc().addReadOnly(borderUnsetPB);
	bc().addReadOnly(borders);
	bc().addReadOnly(longTabularCB);
	bc().addReadOnly(headerStatusCB);
	bc().addReadOnly(headerBorderAboveCB);
	bc().addReadOnly(headerBorderBelowCB);
	bc().addReadOnly(firstheaderStatusCB);
	bc().addReadOnly(firstheaderBorderAboveCB);
	bc().addReadOnly(firstheaderBorderBelowCB);
	bc().addReadOnly(firstheaderNoContentsCB);
	bc().addReadOnly(footerStatusCB);
	bc().addReadOnly(footerBorderAboveCB);
	bc().addReadOnly(footerBorderBelowCB);
	bc().addReadOnly(lastfooterStatusCB);
	bc().addReadOnly(lastfooterBorderAboveCB);
	bc().addReadOnly(lastfooterBorderBelowCB);
	bc().addReadOnly(lastfooterNoContentsCB);
	bc().addReadOnly(newpageCB);

	// initialize the length validator
	bc().addCheckedLineEdit(widthED, fixedWidthColLA);
	bc().addCheckedLineEdit(topspaceED, topspaceLA);
	bc().addCheckedLineEdit(bottomspaceED, bottomspaceLA);
	bc().addCheckedLineEdit(interlinespaceED, interlinespaceLA);
}


ControlTabular & GuiTabularDialog::controller() const
{
	return static_cast<ControlTabular &>(GuiDialog::controller());
}


void GuiTabularDialog::change_adaptor()
{
	changed();
}


void GuiTabularDialog::closeEvent(QCloseEvent * e)
{
	slotWMHide();
	e->accept();
}


void GuiTabularDialog::booktabsChanged(bool)
{
	changed();
	controller().booktabs(booktabsRB->isChecked());
	update_borders();
}


void GuiTabularDialog::topspace_changed()
{
	switch(topspaceCO->currentIndex()) {
		case 0: {
			controller().set(Tabular::SET_TOP_SPACE, "");
				topspaceED->setEnabled(false);
				topspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			controller().set(Tabular::SET_TOP_SPACE, "default");
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!topspaceED->text().isEmpty())
				controller().set(Tabular::SET_TOP_SPACE,
					widgetsToLength(topspaceED, topspaceUnit));
			if (!bc().policy().isReadOnly()) {
				topspaceED->setEnabled(true);
				topspaceUnit->setEnabled(true);
			}
			break;
		}
	}
	changed();
}


void GuiTabularDialog::bottomspace_changed()
{
	switch(bottomspaceCO->currentIndex()) {
		case 0: {
			controller().set(Tabular::SET_BOTTOM_SPACE, "");
				bottomspaceED->setEnabled(false);
				bottomspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			controller().set(Tabular::SET_BOTTOM_SPACE, "default");
			bottomspaceED->setEnabled(false);
			bottomspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!bottomspaceED->text().isEmpty())
				controller().set(Tabular::SET_BOTTOM_SPACE,
					widgetsToLength(bottomspaceED, bottomspaceUnit));
			if (!bc().policy().isReadOnly()) {
				bottomspaceED->setEnabled(true);
				bottomspaceUnit->setEnabled(true);
			}
			break;
		}
	}
	changed();
}


void GuiTabularDialog::interlinespace_changed()
{
	switch(interlinespaceCO->currentIndex()) {
		case 0: {
			controller().set(Tabular::SET_INTERLINE_SPACE, "");
				interlinespaceED->setEnabled(false);
				interlinespaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			controller().set(Tabular::SET_INTERLINE_SPACE, "default");
			interlinespaceED->setEnabled(false);
			interlinespaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!interlinespaceED->text().isEmpty())
				controller().set(Tabular::SET_INTERLINE_SPACE,
					widgetsToLength(interlinespaceED, interlinespaceUnit));
			if (!bc().policy().isReadOnly()) {
				interlinespaceED->setEnabled(true);
				interlinespaceUnit->setEnabled(true);
			}
			break;
		}
	}
	changed();
}


void GuiTabularDialog::close_clicked()
{
	closeGUI();
	slotClose();
}


void GuiTabularDialog::borderSet_clicked()
{
	controller().set(Tabular::SET_ALL_LINES);
	update_borders();
	changed();
}


void GuiTabularDialog::borderUnset_clicked()
{
	controller().set(Tabular::UNSET_ALL_LINES);
	update_borders();
	changed();
}


void GuiTabularDialog::leftBorder_changed()
{
	controller().toggleLeftLine();
	changed();
}


void GuiTabularDialog::rightBorder_changed()
{
	controller().toggleRightLine();
	changed();
}


void GuiTabularDialog::topBorder_changed()
{
	controller().toggleTopLine();
	changed();
}


void GuiTabularDialog::bottomBorder_changed()
{
	controller().toggleBottomLine();
	changed();
}


void GuiTabularDialog::specialAlignment_changed()
{
	string special = fromqstr(specialAlignmentED->text());
	controller().setSpecial(special);
	changed();
}


void GuiTabularDialog::width_changed()
{
	changed();
	string const width = widgetsToLength(widthED, widthUnit);
	controller().setWidth(width);
}


void GuiTabularDialog::multicolumn_clicked()
{
	controller().toggleMultiColumn();
	changed();
}


void GuiTabularDialog::rotateTabular()
{
	controller().rotateTabular(rotateTabularCB->isChecked());
	changed();
}


void GuiTabularDialog::rotateCell()
{
	controller().rotateCell(rotateCellCB->isChecked());
	changed();
}


void GuiTabularDialog::hAlign_changed(int align)
{
	ControlTabular::HALIGN h = ControlTabular::LEFT;

	switch (align) {
		case 0: h = ControlTabular::LEFT; break;
		case 1: h = ControlTabular::CENTER; break;
		case 2: h = ControlTabular::RIGHT; break;
		case 3: h = ControlTabular::BLOCK; break;
	}

	controller().halign(h);
}


void GuiTabularDialog::vAlign_changed(int align)
{
	ControlTabular::VALIGN v = ControlTabular::TOP;

	switch (align) {
		case 0: v = ControlTabular::TOP; break;
		case 1: v = ControlTabular::MIDDLE; break;
		case 2: v = ControlTabular::BOTTOM; break;
	}

	controller().valign(v);
}


void GuiTabularDialog::longTabular()
{
	controller().longTabular(longTabularCB->isChecked());
	changed();
}


void GuiTabularDialog::ltNewpage_clicked()
{
	controller().set(Tabular::SET_LTNEWPAGE);
	changed();
}


void GuiTabularDialog::ltHeaderStatus_clicked()
{
	bool enable(headerStatusCB->isChecked());
	if (enable)
		controller().set(Tabular::SET_LTHEAD, "");
	else
		controller().set(Tabular::UNSET_LTHEAD, "");
	headerBorderAboveCB->setEnabled(enable);
	headerBorderBelowCB->setEnabled(enable);
	firstheaderNoContentsCB->setEnabled(enable);
	changed();
}


void GuiTabularDialog::ltHeaderBorderAbove_clicked()
{
	if (headerBorderAboveCB->isChecked())
		controller().set(Tabular::SET_LTHEAD, "dl_above");
	else
		controller().set(Tabular::UNSET_LTHEAD, "dl_above");
	changed();
}


void GuiTabularDialog::ltHeaderBorderBelow_clicked()
{
	if (headerBorderBelowCB->isChecked())
		controller().set(Tabular::SET_LTHEAD, "dl_below");
	else
		controller().set(Tabular::UNSET_LTHEAD, "dl_below");
	changed();
}


void GuiTabularDialog::ltFirstHeaderBorderAbove_clicked()
{
	if (firstheaderBorderAboveCB->isChecked())
		controller().set(Tabular::SET_LTFIRSTHEAD, "dl_above");
	else
		controller().set(Tabular::UNSET_LTFIRSTHEAD, "dl_above");
	changed();
}


void GuiTabularDialog::ltFirstHeaderBorderBelow_clicked()
{
	if (firstheaderBorderBelowCB->isChecked())
		controller().set(Tabular::SET_LTFIRSTHEAD, "dl_below");
	else
		controller().set(Tabular::UNSET_LTFIRSTHEAD, "dl_below");
	changed();
}


void GuiTabularDialog::ltFirstHeaderStatus_clicked()
{
	bool enable(firstheaderStatusCB->isChecked());
	if (enable)
		controller().set(Tabular::SET_LTFIRSTHEAD, "");
	else
		controller().set(Tabular::UNSET_LTFIRSTHEAD, "");
	firstheaderBorderAboveCB->setEnabled(enable);
	firstheaderBorderBelowCB->setEnabled(enable);
	changed();
}


void GuiTabularDialog::ltFirstHeaderEmpty_clicked()
{
	bool enable(firstheaderNoContentsCB->isChecked());
	if (enable)
		controller().set(Tabular::SET_LTFIRSTHEAD, "empty");
	else
		controller().set(Tabular::UNSET_LTFIRSTHEAD, "empty");
	firstheaderStatusCB->setEnabled(!enable);
	firstheaderBorderAboveCB->setEnabled(!enable);
	firstheaderBorderBelowCB->setEnabled(!enable);
	changed();
}


void GuiTabularDialog::ltFooterStatus_clicked()
{
	bool enable(footerStatusCB->isChecked());
	if (enable)
		controller().set(Tabular::SET_LTFOOT, "");
	else
		controller().set(Tabular::UNSET_LTFOOT, "");
	footerBorderAboveCB->setEnabled(enable);
	footerBorderBelowCB->setEnabled(enable);
	lastfooterNoContentsCB->setEnabled(enable);
	changed();
}


void GuiTabularDialog::ltFooterBorderAbove_clicked()
{
	if (footerBorderAboveCB->isChecked())
		controller().set(Tabular::SET_LTFOOT, "dl_above");
	else
		controller().set(Tabular::UNSET_LTFOOT, "dl_above");
	changed();
}


void GuiTabularDialog::ltFooterBorderBelow_clicked()
{
	if (footerBorderBelowCB->isChecked())
		controller().set(Tabular::SET_LTFOOT, "dl_below");
	else
		controller().set(Tabular::UNSET_LTFOOT, "dl_below");
	changed();
}


void GuiTabularDialog::ltLastFooterStatus_clicked()
{
	bool enable(lastfooterStatusCB->isChecked());
	if (enable)
		controller().set(Tabular::SET_LTLASTFOOT, "");
	else
		controller().set(Tabular::UNSET_LTLASTFOOT, "");
	lastfooterBorderAboveCB->setEnabled(enable);
	lastfooterBorderBelowCB->setEnabled(enable);
	changed();
}


void GuiTabularDialog::ltLastFooterBorderAbove_clicked()
{
	if (lastfooterBorderAboveCB->isChecked())
		controller().set(Tabular::SET_LTLASTFOOT, "dl_above");
	else
		controller().set(Tabular::UNSET_LTLASTFOOT, "dl_above");
	changed();
}


void GuiTabularDialog::ltLastFooterBorderBelow_clicked()
{
	if (lastfooterBorderBelowCB->isChecked())
		controller().set(Tabular::SET_LTLASTFOOT, "dl_below");
	else
		controller().set(Tabular::UNSET_LTLASTFOOT, "dl_below");
	changed();
}


void GuiTabularDialog::ltLastFooterEmpty_clicked()
{
	bool enable(lastfooterNoContentsCB->isChecked());
	if (enable)
		controller().set(Tabular::SET_LTLASTFOOT, "empty");
	else
		controller().set(Tabular::UNSET_LTLASTFOOT, "empty");
	lastfooterStatusCB->setEnabled(!enable);
	lastfooterBorderAboveCB->setEnabled(!enable);
	lastfooterBorderBelowCB->setEnabled(!enable);
	changed();
}


void GuiTabularDialog::update_borders()
{
	Tabular const & tabular = controller().tabular();
	Tabular::idx_type const cell = controller().getActiveCell();
	bool const isMulticolumnCell = tabular.isMultiColumn(cell);

	if (!isMulticolumnCell) {
		borders->setLeftEnabled(true);
		borders->setRightEnabled(true);
		borders->setTop(tabular.topLine(cell, true));
		borders->setBottom(tabular.bottomLine(cell, true));
		borders->setLeft(tabular.leftLine(cell, true));
		borders->setRight(tabular.rightLine(cell, true));
		// repaint the setborder widget
		borders->update();
		return;
	}

	borders->setTop(tabular.topLine(cell));
	borders->setBottom(tabular.bottomLine(cell));
	// pay attention to left/right lines: they are only allowed
	// to set if we are in first/last cell of row or if the left/right
	// cell is also a multicolumn.
	if (tabular.isFirstCellInRow(cell) || tabular.isMultiColumn(cell - 1)) {
		borders->setLeftEnabled(true);
		borders->setLeft(tabular.leftLine(cell));
	} else {
		borders->setLeft(false);
		borders->setLeftEnabled(false);
	}
	if (tabular.isLastCellInRow(cell) || tabular.isMultiColumn(cell + 1)) {
		borders->setRightEnabled(true);
		borders->setRight(tabular.rightLine(cell));
	} else {
		borders->setRight(false);
		borders->setRightEnabled(false);
	}
	// repaint the setborder widget
	borders->update();
}


void GuiTabularDialog::update_contents()
{
	controller().initialiseParams(string());

	Tabular const & tabular(controller().tabular());
	Tabular::idx_type const cell = controller().getActiveCell();

	Tabular::row_type const row = tabular.cellRow(cell);
	Tabular::col_type const col = tabular.cellColumn(cell);

	tabularRowED->setText(toqstr(convert<string>(row + 1)));
	tabularColumnED->setText(toqstr(convert<string>(col + 1)));

	bool const multicol(tabular.isMultiColumn(cell));

	multicolumnCB->setChecked(multicol);

	rotateCellCB->setChecked(tabular.getRotateCell(cell));
	rotateTabularCB->setChecked(tabular.getRotateTabular());

	longTabularCB->setChecked(tabular.isLongTabular());

	update_borders();

	Length pwidth;
	docstring special;

	if (multicol) {
		special = tabular.getAlignSpecial(cell, Tabular::SET_SPECIAL_MULTI);
		pwidth = tabular.getMColumnPWidth(cell);
	} else {
		special = tabular.getAlignSpecial(cell, Tabular::SET_SPECIAL_COLUMN);
		pwidth = tabular.getColumnPWidth(cell);
	}

	specialAlignmentED->setText(toqstr(special));

	bool const isReadonly = bc().policy().isReadOnly();
	specialAlignmentED->setEnabled(!isReadonly);

	Length::UNIT default_unit =
		controller().useMetricUnits() ? Length::CM : Length::IN;

	borderDefaultRB->setChecked(!tabular.useBookTabs());
	booktabsRB->setChecked(tabular.useBookTabs());

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
	topspaceED->setEnabled(!isReadonly
		&& (topspaceCO->currentIndex() == 2));
	topspaceUnit->setEnabled(!isReadonly
		&& (topspaceCO->currentIndex() == 2));
	topspaceCO->setEnabled(!isReadonly);

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
	bottomspaceED->setEnabled(!isReadonly
		&& (bottomspaceCO->currentIndex() == 2));
	bottomspaceUnit->setEnabled(!isReadonly
		&& (bottomspaceCO->currentIndex() == 2));
	bottomspaceCO->setEnabled(!isReadonly);

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
	interlinespaceED->setEnabled(!isReadonly
		&& (interlinespaceCO->currentIndex() == 2));
	interlinespaceUnit->setEnabled(!isReadonly
		&& (interlinespaceCO->currentIndex() == 2));
	interlinespaceCO->setEnabled(!isReadonly);

	string colwidth;
	if (!pwidth.zero())
		colwidth = pwidth.asString();
	lengthToWidgets(widthED, widthUnit,
		colwidth, default_unit);

	widthED->setEnabled(!isReadonly);
	widthUnit->setEnabled(!isReadonly);

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
		valign = 1;
		break;
	}
	if (pwidth.zero())
		valign = 1;
	vAlignCB->setCurrentIndex(valign);

	hAlignCB->setEnabled(true);
	vAlignCB->setEnabled(!pwidth.zero());

	if (!tabular.isLongTabular()) {
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
		return;
	}

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
			firstheaderNoContentsCB->setChecked(ltt.empty);
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
			lastfooterNoContentsCB->setChecked(ltt.empty);
			if (ltt.empty)
				lastfooterStatusCB->setEnabled(false);
		}
	}
	newpageCB->setChecked(tabular.getLTNewPage(row));
}


void GuiTabularDialog::closeGUI()
{
	// ugly hack to auto-apply the stuff that hasn't been
	// yet. don't let this continue to exist ...

	// Subtle here, we must /not/ apply any changes and
	// then refer to tabular, as it will have been freed
	// since the changes update the actual controller().tabular()
	Tabular const & tabular(controller().tabular());

	// apply the fixed width values
	Tabular::idx_type const cell = controller().getActiveCell();
	bool const multicol = tabular.isMultiColumn(cell);
	string width = widgetsToLength(widthED, widthUnit);
	string width2;

	Length llen = tabular.getColumnPWidth(cell);
	Length llenMulti = tabular.getMColumnPWidth(cell);

	if (multicol && !llenMulti.zero())
			width2 = llenMulti.asString();
	else if (!multicol && !llen.zero())
			width2 = llen.asString();

	// apply the special alignment
	docstring const sa1 = qstring_to_ucs4(specialAlignmentED->text());
	docstring sa2;

	if (multicol)
		sa2 = tabular.getAlignSpecial(cell, Tabular::SET_SPECIAL_MULTI);
	else
		sa2 = tabular.getAlignSpecial(cell, Tabular::SET_SPECIAL_COLUMN);

	if (sa1 != sa2) {
		if (multicol)
			controller().set(Tabular::SET_SPECIAL_MULTI, to_utf8(sa1));
		else
			controller().set(Tabular::SET_SPECIAL_COLUMN, to_utf8(sa1));
	}

	if (width != width2) {
		if (multicol)
			controller().set(Tabular::SET_MPWIDTH, width);
		else
			controller().set(Tabular::SET_PWIDTH, width);
	}

	/* DO WE NEED THIS?
	switch (topspaceCO->currentIndex()) {
		case 0:
			controller().set(Tabular::SET_TOP_SPACE, "");
			break;
		case 1:
			controller().set(Tabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			controller().set(Tabular::SET_TOP_SPACE,
				widgetsToLength(topspaceED,
					topspaceUnit));
			break;
	}

	switch (bottomspaceCO->currentIndex()) {
		case 0:
			controller().set(Tabular::SET_BOTTOM_SPACE, "");
			break;
		case 1:
			controller().set(Tabular::SET_BOTTOM_SPACE, "default");
			break;
		case 2:
			controller().set(Tabular::SET_BOTTOM_SPACE,
				widgetsToLength(bottomspaceED,
					bottomspaceUnit));
			break;
	}

	switch (interlinespaceCO->currentIndex()) {
		case 0:
			controller().set(Tabular::SET_INTERLINE_SPACE, "");
			break;
		case 1:
			controller().set(Tabular::SET_INTERLINE_SPACE, "default");
			break;
		case 2:
			controller().set(Tabular::SET_INTERLINE_SPACE,
				widgetsToLength(interlinespaceED,
					interlinespaceUnit));
			break;
	}
*/
}

} // namespace frontend
} // namespace lyx

#include "GuiTabular_moc.cpp"
