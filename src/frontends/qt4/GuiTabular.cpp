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

#include "GuiSetBorder.h"
#include "GuiView.h"
#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include "insets/InsetTabular.h"

#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

using namespace std;

namespace lyx {
namespace frontend {

GuiTabular::GuiTabular(GuiView & lv)
	: GuiDialog(lv, "tabular", qt_("Table Settings")),
	// tabular_ is initialised at dialog construction in initialiseParams()
	tabular_(*lv.buffer(), 0, 0)
{
	active_cell_ = Tabular::npos;

	setupUi(this);

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


GuiTabular::~GuiTabular()
{
}


void GuiTabular::change_adaptor()
{
	changed();
}


void GuiTabular::booktabsChanged(bool)
{
	changed();
	booktabs(booktabsRB->isChecked());
	update_borders();
}


void GuiTabular::topspace_changed()
{
	switch (topspaceCO->currentIndex()) {
		case 0: {
			set(Tabular::SET_TOP_SPACE, "");
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			set(Tabular::SET_TOP_SPACE, "default");
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!topspaceED->text().isEmpty())
				set(Tabular::SET_TOP_SPACE,
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


void GuiTabular::bottomspace_changed()
{
	switch (bottomspaceCO->currentIndex()) {
		case 0: {
			set(Tabular::SET_BOTTOM_SPACE, "");
				bottomspaceED->setEnabled(false);
				bottomspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			set(Tabular::SET_BOTTOM_SPACE, "default");
			bottomspaceED->setEnabled(false);
			bottomspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!bottomspaceED->text().isEmpty())
				set(Tabular::SET_BOTTOM_SPACE,
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


void GuiTabular::interlinespace_changed()
{
	switch (interlinespaceCO->currentIndex()) {
		case 0: {
			set(Tabular::SET_INTERLINE_SPACE, "");
				interlinespaceED->setEnabled(false);
				interlinespaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			set(Tabular::SET_INTERLINE_SPACE, "default");
			interlinespaceED->setEnabled(false);
			interlinespaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!interlinespaceED->text().isEmpty())
				set(Tabular::SET_INTERLINE_SPACE,
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


void GuiTabular::close_clicked()
{
	closeGUI();
	slotClose();
}


void GuiTabular::borderSet_clicked()
{
	set(Tabular::SET_ALL_LINES);
	update_borders();
	changed();
}


void GuiTabular::borderUnset_clicked()
{
	set(Tabular::UNSET_ALL_LINES);
	update_borders();
	changed();
}


void GuiTabular::leftBorder_changed()
{
	set(Tabular::TOGGLE_LINE_LEFT);
	changed();
}


void GuiTabular::rightBorder_changed()
{
	set(Tabular::TOGGLE_LINE_RIGHT);
	changed();
}


void GuiTabular::topBorder_changed()
{
	set(Tabular::TOGGLE_LINE_TOP);
	changed();
}


void GuiTabular::bottomBorder_changed()
{
	set(Tabular::TOGGLE_LINE_BOTTOM);
	changed();
}


void GuiTabular::specialAlignment_changed()
{
	string special = fromqstr(specialAlignmentED->text());
	setSpecial(special);
	changed();
}


void GuiTabular::width_changed()
{
	changed();
	string const width = widgetsToLength(widthED, widthUnit);
	setWidth(width);
}


void GuiTabular::multicolumn_clicked()
{
	toggleMultiColumn();
	changed();
}


void GuiTabular::rotateTabular()
{
	rotateTabular(rotateTabularCB->isChecked());
	changed();
}


void GuiTabular::rotateCell()
{
	rotateCell(rotateCellCB->isChecked());
	changed();
}


void GuiTabular::hAlign_changed(int align)
{
	GuiTabular::HALIGN h = GuiTabular::LEFT;

	switch (align) {
		case 0: h = GuiTabular::LEFT; break;
		case 1: h = GuiTabular::CENTER; break;
		case 2: h = GuiTabular::RIGHT; break;
		case 3: h = GuiTabular::BLOCK; break;
	}

	halign(h);
}


void GuiTabular::vAlign_changed(int align)
{
	GuiTabular::VALIGN v = GuiTabular::TOP;

	switch (align) {
		case 0: v = GuiTabular::TOP; break;
		case 1: v = GuiTabular::MIDDLE; break;
		case 2: v = GuiTabular::BOTTOM; break;
	}

	valign(v);
}


void GuiTabular::longTabular()
{
	longTabular(longTabularCB->isChecked());
	changed();
}


void GuiTabular::ltNewpage_clicked()
{
	set(Tabular::SET_LTNEWPAGE);
	changed();
}


void GuiTabular::on_captionStatusCB_toggled()
{
	set(Tabular::TOGGLE_LTCAPTION);
	changed();
}


void GuiTabular::ltHeaderStatus_clicked()
{
	bool enable = headerStatusCB->isChecked();
	if (enable)
		set(Tabular::SET_LTHEAD, "");
	else
		set(Tabular::UNSET_LTHEAD, "");
	headerBorderAboveCB->setEnabled(enable);
	headerBorderBelowCB->setEnabled(enable);
	firstheaderNoContentsCB->setEnabled(enable);
	changed();
}


void GuiTabular::ltHeaderBorderAbove_clicked()
{
	if (headerBorderAboveCB->isChecked())
		set(Tabular::SET_LTHEAD, "dl_above");
	else
		set(Tabular::UNSET_LTHEAD, "dl_above");
	changed();
}


void GuiTabular::ltHeaderBorderBelow_clicked()
{
	if (headerBorderBelowCB->isChecked())
		set(Tabular::SET_LTHEAD, "dl_below");
	else
		set(Tabular::UNSET_LTHEAD, "dl_below");
	changed();
}


void GuiTabular::ltFirstHeaderBorderAbove_clicked()
{
	if (firstheaderBorderAboveCB->isChecked())
		set(Tabular::SET_LTFIRSTHEAD, "dl_above");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "dl_above");
	changed();
}


void GuiTabular::ltFirstHeaderBorderBelow_clicked()
{
	if (firstheaderBorderBelowCB->isChecked())
		set(Tabular::SET_LTFIRSTHEAD, "dl_below");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "dl_below");
	changed();
}


void GuiTabular::ltFirstHeaderStatus_clicked()
{
	bool enable = firstheaderStatusCB->isChecked();
	if (enable)
		set(Tabular::SET_LTFIRSTHEAD, "");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "");
	firstheaderBorderAboveCB->setEnabled(enable);
	firstheaderBorderBelowCB->setEnabled(enable);
	changed();
}


void GuiTabular::ltFirstHeaderEmpty_clicked()
{
	bool enable = firstheaderNoContentsCB->isChecked();
	if (enable)
		set(Tabular::SET_LTFIRSTHEAD, "empty");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "empty");
	firstheaderStatusCB->setEnabled(!enable);
	firstheaderBorderAboveCB->setEnabled(!enable);
	firstheaderBorderBelowCB->setEnabled(!enable);
	changed();
}


void GuiTabular::ltFooterStatus_clicked()
{
	bool enable = footerStatusCB->isChecked();
	if (enable)
		set(Tabular::SET_LTFOOT, "");
	else
		set(Tabular::UNSET_LTFOOT, "");
	footerBorderAboveCB->setEnabled(enable);
	footerBorderBelowCB->setEnabled(enable);
	lastfooterNoContentsCB->setEnabled(enable);
	changed();
}


void GuiTabular::ltFooterBorderAbove_clicked()
{
	if (footerBorderAboveCB->isChecked())
		set(Tabular::SET_LTFOOT, "dl_above");
	else
		set(Tabular::UNSET_LTFOOT, "dl_above");
	changed();
}


void GuiTabular::ltFooterBorderBelow_clicked()
{
	if (footerBorderBelowCB->isChecked())
		set(Tabular::SET_LTFOOT, "dl_below");
	else
		set(Tabular::UNSET_LTFOOT, "dl_below");
	changed();
}


void GuiTabular::ltLastFooterStatus_clicked()
{
	bool enable = lastfooterStatusCB->isChecked();
	if (enable)
		set(Tabular::SET_LTLASTFOOT, "");
	else
		set(Tabular::UNSET_LTLASTFOOT, "");
	lastfooterBorderAboveCB->setEnabled(enable);
	lastfooterBorderBelowCB->setEnabled(enable);
	changed();
}


void GuiTabular::ltLastFooterBorderAbove_clicked()
{
	if (lastfooterBorderAboveCB->isChecked())
		set(Tabular::SET_LTLASTFOOT, "dl_above");
	else
		set(Tabular::UNSET_LTLASTFOOT, "dl_above");
	changed();
}


void GuiTabular::ltLastFooterBorderBelow_clicked()
{
	if (lastfooterBorderBelowCB->isChecked())
		set(Tabular::SET_LTLASTFOOT, "dl_below");
	else
		set(Tabular::UNSET_LTLASTFOOT, "dl_below");
	changed();
}


void GuiTabular::ltLastFooterEmpty_clicked()
{
	bool enable = lastfooterNoContentsCB->isChecked();
	if (enable)
		set(Tabular::SET_LTLASTFOOT, "empty");
	else
		set(Tabular::UNSET_LTLASTFOOT, "empty");
	lastfooterStatusCB->setEnabled(!enable);
	lastfooterBorderAboveCB->setEnabled(!enable);
	lastfooterBorderBelowCB->setEnabled(!enable);
	changed();
}


void GuiTabular::update_borders()
{
	Tabular::idx_type const cell = getActiveCell();
	borders->setTop(tabular_.topLine(cell));
	borders->setBottom(tabular_.bottomLine(cell));
	borders->setLeft(tabular_.leftLine(cell));
	borders->setRight(tabular_.rightLine(cell));
	// repaint the setborder widget
	borders->update();
}


namespace {

Length getColumnPWidth(Tabular const & t, size_t cell)
{
	return t.column_info[t.cellColumn(cell)].p_width;
}


Length getMColumnPWidth(Tabular const & t, size_t cell)
{
	if (t.isMultiColumn(cell))
		return t.cellInfo(cell).p_width;
	return Length();
}


docstring getAlignSpecial(Tabular const & t, size_t cell, int what)
{
	if (what == Tabular::SET_SPECIAL_MULTI)
		return t.cellInfo(cell).align_special;
	return t.column_info[t.cellColumn(cell)].align_special;
}

}



void GuiTabular::updateContents()
{
	initialiseParams(string());

	size_t const cell = getActiveCell();

	Tabular::row_type const row = tabular_.cellRow(cell);
	Tabular::col_type const col = tabular_.cellColumn(cell);

	tabularRowED->setText(QString::number(row + 1));
	tabularColumnED->setText(QString::number(col + 1));

	bool const multicol(tabular_.isMultiColumn(cell));

	multicolumnCB->setChecked(multicol);

	rotateCellCB->setChecked(tabular_.getRotateCell(cell));
	rotateTabularCB->setChecked(tabular_.rotate);

	longTabularCB->setChecked(tabular_.is_long_tabular);

	update_borders();

	Length pwidth;
	docstring special;

	if (multicol) {
		special = getAlignSpecial(tabular_, cell, Tabular::SET_SPECIAL_MULTI);
		pwidth = getMColumnPWidth(tabular_, cell);
	} else {
		special = getAlignSpecial(tabular_, cell, Tabular::SET_SPECIAL_COLUMN);
		pwidth = getColumnPWidth(tabular_, cell);
	}

	specialAlignmentED->setText(toqstr(special));

	bool const isReadonly = bc().policy().isReadOnly();
	specialAlignmentED->setEnabled(!isReadonly);

	Length::UNIT default_unit =
		useMetricUnits() ? Length::CM : Length::IN;

	borderDefaultRB->setChecked(!tabular_.use_booktabs);
	booktabsRB->setChecked(tabular_.use_booktabs);

	if (tabular_.row_info[row].top_space.empty()
	    && !tabular_.row_info[row].top_space_default) {
		topspaceCO->setCurrentIndex(0);
	} else if (tabular_.row_info[row].top_space_default) {
		topspaceCO->setCurrentIndex(1);
	} else {
		topspaceCO->setCurrentIndex(2);
		lengthToWidgets(topspaceED,
				topspaceUnit,
				tabular_.row_info[row].top_space.asString(),
				default_unit);
	}
	topspaceED->setEnabled(!isReadonly
		&& (topspaceCO->currentIndex() == 2));
	topspaceUnit->setEnabled(!isReadonly
		&& (topspaceCO->currentIndex() == 2));
	topspaceCO->setEnabled(!isReadonly);

	if (tabular_.row_info[row].bottom_space.empty()
	    && !tabular_.row_info[row].bottom_space_default) {
		bottomspaceCO->setCurrentIndex(0);
	} else if (tabular_.row_info[row].bottom_space_default) {
		bottomspaceCO->setCurrentIndex(1);
	} else {
		bottomspaceCO->setCurrentIndex(2);
		lengthToWidgets(bottomspaceED,
				bottomspaceUnit,
				tabular_.row_info[row].bottom_space.asString(),
				default_unit);
	}
	bottomspaceED->setEnabled(!isReadonly
		&& (bottomspaceCO->currentIndex() == 2));
	bottomspaceUnit->setEnabled(!isReadonly
		&& (bottomspaceCO->currentIndex() == 2));
	bottomspaceCO->setEnabled(!isReadonly);

	if (tabular_.row_info[row].interline_space.empty()
	    && !tabular_.row_info[row].interline_space_default) {
		interlinespaceCO->setCurrentIndex(0);
	} else if (tabular_.row_info[row].interline_space_default) {
		interlinespaceCO->setCurrentIndex(1);
	} else {
		interlinespaceCO->setCurrentIndex(2);
		lengthToWidgets(interlinespaceED,
				interlinespaceUnit,
				tabular_.row_info[row].interline_space.asString(),
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
	switch (tabular_.getAlignment(cell)) {
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
	switch (tabular_.getVAlignment(cell)) {
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

	if (!tabular_.is_long_tabular) {
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
	}
	captionStatusCB->blockSignals(true);
	captionStatusCB->setChecked(tabular_.ltCaption(row));
	captionStatusCB->blockSignals(false);

	Tabular::ltType ltt;
	bool use_empty;
	bool row_set = tabular_.getRowOfLTHead(row, ltt);
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

	row_set = tabular_.getRowOfLTFirstHead(row, ltt);
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

	row_set = tabular_.getRowOfLTFoot(row, ltt);
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

	row_set = tabular_.getRowOfLTLastFoot(row, ltt);
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
	newpageCB->setChecked(tabular_.getLTNewPage(row));
}


void GuiTabular::closeGUI()
{
	// ugly hack to auto-apply the stuff that hasn't been
	// yet. don't let this continue to exist ...

	// Subtle here, we must /not/ apply any changes and
	// then refer to tabular, as it will have been freed
	// since the changes update the actual tabular_
	//
	// apply the fixed width values
	size_t const cell = getActiveCell();
	bool const multicol = tabular_.isMultiColumn(cell);
	string width = widgetsToLength(widthED, widthUnit);
	string width2;

	Length llen = getColumnPWidth(tabular_, cell);
	Length llenMulti = getMColumnPWidth(tabular_, cell);

	if (multicol && !llenMulti.zero())
		width2 = llenMulti.asString();
	else if (!multicol && !llen.zero())
		width2 = llen.asString();

	// apply the special alignment
	docstring const sa1 = qstring_to_ucs4(specialAlignmentED->text());
	docstring sa2;

	if (multicol)
		sa2 = getAlignSpecial(tabular_, cell, Tabular::SET_SPECIAL_MULTI);
	else
		sa2 = getAlignSpecial(tabular_, cell, Tabular::SET_SPECIAL_COLUMN);

	if (sa1 != sa2) {
		if (multicol)
			set(Tabular::SET_SPECIAL_MULTI, to_utf8(sa1));
		else
			set(Tabular::SET_SPECIAL_COLUMN, to_utf8(sa1));
	}

	if (width != width2) {
		if (multicol)
			set(Tabular::SET_MPWIDTH, width);
		else
			set(Tabular::SET_PWIDTH, width);
	}

	/* DO WE NEED THIS?
	switch (topspaceCO->currentIndex()) {
		case 0:
			set(Tabular::SET_TOP_SPACE, "");
			break;
		case 1:
			set(Tabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			set(Tabular::SET_TOP_SPACE,
				widgetsToLength(topspaceED, topspaceUnit));
			break;
	}

	switch (bottomspaceCO->currentIndex()) {
		case 0:
			set(Tabular::SET_BOTTOM_SPACE, "");
			break;
		case 1:
			set(Tabular::SET_BOTTOM_SPACE, "default");
			break;
		case 2:
			set(Tabular::SET_BOTTOM_SPACE,
				widgetsToLength(bottomspaceED, bottomspaceUnit));
			break;
	}

	switch (interlinespaceCO->currentIndex()) {
		case 0:
			set(Tabular::SET_INTERLINE_SPACE, "");
			break;
		case 1:
			set(Tabular::SET_INTERLINE_SPACE, "default");
			break;
		case 2:
			set(Tabular::SET_INTERLINE_SPACE,
				widgetsToLength(interlinespaceED, interlinespaceUnit));
			break;
	}
*/
}


bool GuiTabular::initialiseParams(string const & data)
{
	// try to get the current cell
	BufferView const * const bv = bufferview();
	InsetTabular const * current_inset = 0;
	if (bv) {
		Cursor const & cur = bv->cursor();
		// get the innermost tabular inset;
		// assume that it is "ours"
		for (int i = cur.depth() - 1; i >= 0; --i)
			if (cur[i].inset().lyxCode() == TABULAR_CODE) {
				current_inset = static_cast<InsetTabular const *>(&cur[i].inset());
				active_cell_ = cur[i].idx();
				break;
			}
	}

	if (current_inset && data.empty()) {
		tabular_ = Tabular(current_inset->tabular);
		return true;
	}

	InsetTabular tmp(const_cast<Buffer &>(buffer()));
	InsetTabular::string2params(data, tmp);
	tabular_ = Tabular(tmp.tabular);
	return true;
}


void GuiTabular::clearParams()
{
	InsetTabular tmp(const_cast<Buffer &>(buffer()));
	tabular_ = tmp.tabular;
	active_cell_ = Tabular::npos;
}


Tabular::idx_type GuiTabular::getActiveCell() const
{
	return active_cell_;
}


void GuiTabular::set(Tabular::Feature f, string const & arg)
{
	string const data = featureAsString(f) + ' ' + arg;
	dispatch(FuncRequest(getLfun(), data));
}


bool GuiTabular::useMetricUnits() const
{
	return lyxrc.default_papersize > PAPER_USEXECUTIVE;
}


void GuiTabular::setSpecial(string const & special)
{
	if (tabular_.isMultiColumn(getActiveCell()))
		set(Tabular::SET_SPECIAL_MULTI, special);
	else
		set(Tabular::SET_SPECIAL_COLUMN, special);
}


void GuiTabular::setWidth(string const & width)
{
	if (tabular_.isMultiColumn(getActiveCell()))
		set(Tabular::SET_MPWIDTH, width);
	else
		set(Tabular::SET_PWIDTH, width);

	updateView();
}


void GuiTabular::toggleMultiColumn()
{
	set(Tabular::MULTICOLUMN);
	updateView();
}


void GuiTabular::rotateTabular(bool yes)
{
	if (yes)
		set(Tabular::SET_ROTATE_TABULAR);
	else
		set(Tabular::UNSET_ROTATE_TABULAR);
}


void GuiTabular::rotateCell(bool yes)
{
	if (yes)
		set(Tabular::SET_ROTATE_CELL);
	else
		set(Tabular::UNSET_ROTATE_CELL);
}


void GuiTabular::halign(GuiTabular::HALIGN h)
{
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

	if (tabular_.isMultiColumn(getActiveCell()))
		set(multi_num);
	else
		set(num);
}


void GuiTabular::valign(GuiTabular::VALIGN v)
{
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

	if (tabular_.isMultiColumn(getActiveCell()))
		set(multi_num);
	else
		set(num);
}


void GuiTabular::booktabs(bool yes)
{
	if (yes)
		set(Tabular::SET_BOOKTABS);
	else
		set(Tabular::UNSET_BOOKTABS);
}


void GuiTabular::longTabular(bool yes)
{
	if (yes)
		set(Tabular::SET_LONGTABULAR);
	else
		set(Tabular::UNSET_LONGTABULAR);
}


Dialog * createGuiTabular(GuiView & lv) { return new GuiTabular(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabular.cpp"
