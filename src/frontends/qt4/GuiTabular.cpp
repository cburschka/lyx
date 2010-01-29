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

#include "GuiSetBorder.h"
#include "GuiView.h"
#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyXFunc.h"

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
	tabular_(&(lv.currentBufferView()->buffer()), 0, 0),
	applying_(false)
{
	active_cell_ = Tabular::npos;

	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().setRestore(restorePB);

	widthED->setValidator(unsignedLengthValidator(widthED));
	topspaceED->setValidator(new LengthValidator(topspaceED));
	bottomspaceED->setValidator(new LengthValidator(bottomspaceED));
	interlinespaceED->setValidator(new LengthValidator(interlinespaceED));

	widthUnitCB->setCurrentItem(Length::defaultUnit());

	connect(topspaceED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(topspaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(topspaceCO, SIGNAL(activated(int)),
		this, SLOT(topspace_changed()));
	connect(bottomspaceED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(bottomspaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(bottomspaceCO, SIGNAL(activated(int)),
		this, SLOT(bottomspace_changed()));
	connect(interlinespaceED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(interlinespaceUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(interlinespaceCO, SIGNAL(activated(int)),
		this, SLOT(interlinespace_changed()));
	connect(booktabsRB, SIGNAL(clicked(bool)),
		this, SLOT(booktabsChanged(bool)));
	connect(borderDefaultRB, SIGNAL(clicked(bool)),
		this, SLOT(booktabsChanged(bool)));
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
		this, SLOT(change_adaptor()));
	connect(vAlignCB, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(multicolumnCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(newpageCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(captionStatusCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(headerStatusCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(headerBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(headerBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(firstheaderStatusCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(firstheaderBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(firstheaderBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(firstheaderNoContentsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(footerStatusCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(footerBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(footerBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(lastfooterStatusCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(lastfooterBorderAboveCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(lastfooterBorderBelowCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(lastfooterNoContentsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(specialAlignmentED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(widthED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(widthUnitCB, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(borders, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(rotateTabularCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(rotateCellCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(TableAlignCB, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(longTabularCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(leftRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(centerRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(rightRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

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
	bc().addReadOnly(widthUnitCB);
	bc().addReadOnly(hAlignCB);
	bc().addReadOnly(vAlignCB);
	bc().addReadOnly(TableAlignCB);
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
	bc().addReadOnly(leftRB);
	bc().addReadOnly(centerRB);
	bc().addReadOnly(rightRB);
	
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


void GuiTabular::topspace_changed()
{
	switch (topspaceCO->currentIndex()) {
		case 0: {
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
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
			bottomspaceED->setEnabled(false);
			bottomspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			bottomspaceED->setEnabled(false);
			bottomspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
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
			interlinespaceED->setEnabled(false);
			interlinespaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			interlinespaceED->setEnabled(false);
			interlinespaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!bc().policy().isReadOnly()) {
				interlinespaceED->setEnabled(true);
				interlinespaceUnit->setEnabled(true);
			}
			break;
		}
	}
	changed();
}


void GuiTabular::booktabsChanged(bool)
{
	if (booktabsRB->isChecked()) {
		borders->setLeft(false);
		borders->setRight(false);
		borders->setLeftEnabled(false);
		borders->setRightEnabled(false);
	} else {
		borders->setLeftEnabled(true);
		borders->setRightEnabled(true); 
	}
	// repaint the setborder widget
	borders->update();
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
	changed();
}


void GuiTabular::borderUnset_clicked()
{
	borders->setTop(false);
	borders->setBottom(false);
	borders->setLeft(false);
	borders->setRight(false);
	// repaint the setborder widget
	borders->update();
	changed();
}


void GuiTabular::update_borders()
{
	Tabular::idx_type const cell = getActiveCell();
	borders->setTop(tabular_.topLine(cell));
	borders->setBottom(tabular_.bottomLine(cell));
	borders->setLeft(tabular_.leftLine(cell));
	borders->setRight(tabular_.rightLine(cell));
	borders->setLeftEnabled(!booktabsRB->isChecked());
	borders->setRightEnabled(!booktabsRB->isChecked());
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


void GuiTabular::applyView()
{
	applying_ = true;

	Tabular::idx_type const cell = getActiveCell();
	Tabular::row_type const row = tabular_.cellRow(cell);
	bool const multicol = tabular_.isMultiColumn(cell);

	Tabular::Feature num = Tabular::ALIGN_LEFT;
	Tabular::Feature multi_num = Tabular::M_ALIGN_LEFT;

	switch (hAlignCB->currentIndex()) {
		case 0:
			num = Tabular::ALIGN_LEFT;
			multi_num = Tabular::M_ALIGN_LEFT;
			break;
		case 1:
			num = Tabular::ALIGN_CENTER;
			multi_num = Tabular::M_ALIGN_CENTER;
			break;
		case 2:
			num = Tabular::ALIGN_RIGHT;
			multi_num = Tabular::M_ALIGN_RIGHT;
			break;
		case 3:
			num = Tabular::ALIGN_BLOCK;
			//multi_num: no equivalent
			break;
	}
	if (multicol)
		set(multi_num);
	else
		set(num);

	num = Tabular::VALIGN_MIDDLE;
	multi_num = Tabular::M_VALIGN_MIDDLE;

	switch (vAlignCB->currentIndex()) {
		case 0:
			num = Tabular::VALIGN_TOP;
			multi_num = Tabular::M_VALIGN_TOP;
			break;
		case 1:
			num = Tabular::VALIGN_MIDDLE;
			multi_num = Tabular::M_VALIGN_MIDDLE;
			break;
		case 2:
			num = Tabular::VALIGN_BOTTOM;
			multi_num = Tabular::M_VALIGN_BOTTOM;
			break;
	}
	if (multicol)
		set(multi_num);
	else
		set(num);

	switch (TableAlignCB->currentIndex()) {
		case 0:
			set(Tabular::TABULAR_VALIGN_TOP);
			break;
		case 1:
			set(Tabular::TABULAR_VALIGN_MIDDLE);
			break;
		case 2:
			set(Tabular::TABULAR_VALIGN_BOTTOM);
			break;
	}

	string value;

	value = fromqstr(specialAlignmentED->text());
	if (multicol)
		set(Tabular::SET_SPECIAL_MULTI, value);
	else
		set(Tabular::SET_SPECIAL_COLUMN, value);

	value = widgetsToLength(widthED, widthUnitCB);
	if (multicol)
		set(Tabular::SET_MPWIDTH, value);
	else
		set(Tabular::SET_PWIDTH, value);

	if (multicolumnCB->isChecked() != multicol)
		set(Tabular::MULTICOLUMN);;

	if (rotateTabularCB->isChecked())
		set(Tabular::SET_ROTATE_TABULAR);
	else
		set(Tabular::UNSET_ROTATE_TABULAR);

	if (rotateCellCB->isChecked())
		set(Tabular::SET_ROTATE_CELL);
	else
		set(Tabular::UNSET_ROTATE_CELL);

	// Borders
	if (borders->getLeft() != tabular_.leftLine(cell))
		set(Tabular::TOGGLE_LINE_LEFT);
	if (borders->getRight() != tabular_.rightLine(cell))
		set(Tabular::TOGGLE_LINE_RIGHT);
	if (borders->getTop() != tabular_.topLine(cell))
		set(Tabular::TOGGLE_LINE_TOP);
	if (borders->getBottom() != tabular_.bottomLine(cell))
		set(Tabular::TOGGLE_LINE_BOTTOM);

	if (booktabsRB->isChecked())
		set(Tabular::SET_BOOKTABS);
	else
		set(Tabular::UNSET_BOOKTABS);
	
	switch (topspaceCO->currentIndex()) {
		case 0:
			set(Tabular::SET_TOP_SPACE, "");
			break;
		case 1:
			set(Tabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			set(Tabular::SET_TOP_SPACE,
				widgetsToLength(topspaceED,
					        topspaceUnit));
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
				widgetsToLength(bottomspaceED,
					        bottomspaceUnit));
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
				widgetsToLength(interlinespaceED,
					        interlinespaceUnit));
			break;
	}

	// Longtabular
	if (longTabularCB->isChecked())
		set(Tabular::SET_LONGTABULAR);
	else
		set(Tabular::UNSET_LONGTABULAR);

	if (headerStatusCB->isChecked())
		set(Tabular::SET_LTHEAD, "");
	else
		set(Tabular::UNSET_LTHEAD, "");
	
	if (headerBorderAboveCB->isChecked())
		set(Tabular::SET_LTHEAD, "dl_above");
	else
		set(Tabular::UNSET_LTHEAD, "dl_above");

	if (headerBorderAboveCB->isChecked())
		set(Tabular::SET_LTHEAD, "dl_above");
	else
		set(Tabular::UNSET_LTHEAD, "dl_above");

	if (headerBorderBelowCB->isChecked())
		set(Tabular::SET_LTHEAD, "dl_below");
	else
		set(Tabular::UNSET_LTHEAD, "dl_below");

	if (firstheaderBorderAboveCB->isChecked())
		set(Tabular::SET_LTFIRSTHEAD, "dl_above");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "dl_above");

	if (firstheaderBorderBelowCB->isChecked())
		set(Tabular::SET_LTFIRSTHEAD, "dl_below");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "dl_below");

	if (firstheaderStatusCB->isChecked())
		set(Tabular::SET_LTFIRSTHEAD, "");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "");

	if (firstheaderNoContentsCB->isChecked())
		set(Tabular::SET_LTFIRSTHEAD, "empty");
	else
		set(Tabular::UNSET_LTFIRSTHEAD, "empty");

	if (footerStatusCB->isChecked())
		set(Tabular::SET_LTFOOT, "");
	else
		set(Tabular::UNSET_LTFOOT, "");

	if (footerBorderAboveCB->isChecked())
		set(Tabular::SET_LTFOOT, "dl_above");
	else
		set(Tabular::UNSET_LTFOOT, "dl_above");

	if (footerBorderBelowCB->isChecked())
		set(Tabular::SET_LTFOOT, "dl_below");
	else
		set(Tabular::UNSET_LTFOOT, "dl_below");

	if (lastfooterStatusCB->isChecked())
		set(Tabular::SET_LTLASTFOOT, "");
	else
		set(Tabular::UNSET_LTLASTFOOT, "");

	if (lastfooterBorderAboveCB->isChecked())
		set(Tabular::SET_LTLASTFOOT, "dl_above");
	else
		set(Tabular::UNSET_LTLASTFOOT, "dl_above");

	if (lastfooterBorderBelowCB->isChecked())
		set(Tabular::SET_LTLASTFOOT, "dl_below");
	else
		set(Tabular::UNSET_LTLASTFOOT, "dl_below");

	if (lastfooterNoContentsCB->isChecked())
		set(Tabular::SET_LTLASTFOOT, "empty");
	else
		set(Tabular::UNSET_LTLASTFOOT, "empty");

	if (newpageCB->isChecked() != tabular_.getLTNewPage(row))
		set(Tabular::SET_LTNEWPAGE);

	if (captionStatusCB->isChecked() != tabular_.ltCaption(row))
		set(Tabular::TOGGLE_LTCAPTION);

	if (leftRB->isChecked())
		set(Tabular::LONGTABULAR_ALIGN_LEFT);
	else if (centerRB->isChecked())
		set(Tabular::LONGTABULAR_ALIGN_CENTER);
	else if (rightRB->isChecked())
		set(Tabular::LONGTABULAR_ALIGN_RIGHT);

	applying_ = false;
}


void GuiTabular::updateContents()
{
	if (applying_)
		return;

	initialiseParams(string());

	Tabular::idx_type const cell = getActiveCell();

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
		special = getAlignSpecial(tabular_, cell,
			Tabular::SET_SPECIAL_MULTI);
		pwidth = getMColumnPWidth(tabular_, cell);
	} else {
		special = getAlignSpecial(tabular_, cell,
			Tabular::SET_SPECIAL_COLUMN);
		pwidth = getColumnPWidth(tabular_, cell);
	}

	specialAlignmentED->setText(toqstr(special));

	bool const isReadonly = bc().policy().isReadOnly();
	specialAlignmentED->setEnabled(!isReadonly);

	Length::UNIT const default_unit = Length::defaultUnit();

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
	if (!pwidth.zero()) {
		colwidth = pwidth.asString();

		lengthToWidgets(widthED, widthUnitCB,
			colwidth, default_unit);
	}

	widthED->setEnabled(!isReadonly);
	widthUnitCB->setEnabled(!isReadonly);

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
		valign = 0;
		break;
	}
	if (pwidth.zero())
		valign = 0;
	vAlignCB->setCurrentIndex(valign);

	hAlignCB->setEnabled(true);
	vAlignCB->setEnabled(!pwidth.zero());

	int tableValign = 1;
	switch (tabular_.tabular_valignment) {
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
	} else
		// longtables cannot have a vertical alignment
		TableAlignCB->setCurrentIndex(Tabular::LYX_VALIGN_MIDDLE);

	switch (tabular_.longtabular_alignment) {
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
	captionStatusCB->setChecked(tabular_.ltCaption(row));
	captionStatusCB->blockSignals(false);

	// FIXME: shouldn't this be handled by GuiDialog?
	// FIXME: Some of them should be handled directly in TabularUI.ui
	firstheaderBorderAboveCB->setEnabled(
		funcEnabled(Tabular::SET_LTFIRSTHEAD));
	firstheaderBorderBelowCB->setEnabled(
		funcEnabled(Tabular::SET_LTFIRSTHEAD));
	// first header can only be suppressed when there is a header
	firstheaderNoContentsCB->setEnabled(tabular_.haveLTHead()
		&& !tabular_.haveLTFirstHead());

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
	lastfooterNoContentsCB->setEnabled(tabular_.haveLTFoot()
		&& !tabular_.haveLTLastFoot());

	captionStatusCB->setEnabled(
		funcEnabled(Tabular::TOGGLE_LTCAPTION));
	// When a row is set as longtable caption, it must not be allowed
	// to unset that this row is a multicolumn.
	multicolumnCB->setEnabled(funcEnabled(Tabular::MULTICOLUMN));

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
	newpageCB->setChecked(tabular_.getLTNewPage(row));
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
				current_inset =
					static_cast<InsetTabular const *>(&cur[i].inset());
				active_cell_ = cur[i].idx();
				break;
			}
	}

	if (current_inset && data.empty()) {
		tabular_ = Tabular(current_inset->tabular);
		return true;
	}

	InsetTabular tmp(const_cast<Buffer *>(&buffer()));
	InsetTabular::string2params(data, tmp);
	tabular_ = Tabular(tmp.tabular);
	return true;
}


void GuiTabular::clearParams()
{
	// This function is also called when LyX is closing and the dialog
	// is still open. At that time, the buffer might not be available
	// anymore.
	if (isBufferAvailable()) {
		InsetTabular tmp(const_cast<Buffer *>(&buffer()));
		tabular_ = tmp.tabular;
	}
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


// to get the status of the longtable row settings
bool GuiTabular::funcEnabled(Tabular::Feature f) const
{
	return getStatus(
		FuncRequest(getLfun(), featureAsString(f))).enabled();
}


Dialog * createGuiTabular(GuiView & lv) { return new GuiTabular(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabular.cpp"
