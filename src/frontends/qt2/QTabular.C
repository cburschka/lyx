/**
 * \file QTabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Juergen Spitzmueller
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlTabular.h"
#include "insets/insettabular.h"
#include "qt_helpers.h"
#include "support/lstrings.h"
#include "lyxrc.h"

#include "QTabularDialog.h"
#include "QTabular.h"
#include "Qt2BC.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include "lengthcombo.h"
#include "qsetborder.h"

typedef Qt2CB<ControlTabular, Qt2DB<QTabularDialog> > base_class;

QTabular::QTabular()
	: base_class(qt_("LyX: Edit Table"))
{
}


void QTabular::build_dialog()
{
	dialog_.reset(new QTabularDialog(this));

	bc().setCancel(dialog_->closePB);

	bc().addReadOnly(dialog_->multicolumnCB);
	bc().addReadOnly(dialog_->rotateCellCB);
	bc().addReadOnly(dialog_->rotateTabularCB);
	bc().addReadOnly(dialog_->specialAlignmentED);
	bc().addReadOnly(dialog_->widthED);
	bc().addReadOnly(dialog_->widthUnit);
	bc().addReadOnly(dialog_->hAlignCB);
	bc().addReadOnly(dialog_->vAlignCB);
	bc().addReadOnly(dialog_->columnAddPB);
	bc().addReadOnly(dialog_->columnDeletePB);
	bc().addReadOnly(dialog_->rowAddPB);
	bc().addReadOnly(dialog_->rowDeletePB);
	bc().addReadOnly(dialog_->borderSetPB);
	bc().addReadOnly(dialog_->borderUnsetPB);
	bc().addReadOnly(dialog_->borders);
	bc().addReadOnly(dialog_->longTabularCB);
	bc().addReadOnly(dialog_->headerStatusCB);
	bc().addReadOnly(dialog_->headerBorderAboveCB);
	bc().addReadOnly(dialog_->headerBorderBelowCB);
	bc().addReadOnly(dialog_->firstheaderStatusCB);
	bc().addReadOnly(dialog_->firstheaderBorderAboveCB);
	bc().addReadOnly(dialog_->firstheaderBorderBelowCB);
	bc().addReadOnly(dialog_->firstheaderNoContentsCB);
	bc().addReadOnly(dialog_->footerStatusCB);
	bc().addReadOnly(dialog_->footerBorderAboveCB);
	bc().addReadOnly(dialog_->footerBorderBelowCB);
	bc().addReadOnly(dialog_->lastfooterStatusCB);
	bc().addReadOnly(dialog_->lastfooterBorderAboveCB);
	bc().addReadOnly(dialog_->lastfooterBorderBelowCB);
	bc().addReadOnly(dialog_->lastfooterNoContentsCB);
	bc().addReadOnly(dialog_->newpageCB);
}


bool QTabular::isValid()
{
	return true;
}


void QTabular::update_borders()
{
	LyXTabular * tabular(controller().tabular());
	int cell(controller().inset()->getActCell());

	if (!controller().isMulticolumnCell()) {
		dialog_->borders->setLeftEnabled(true);
		dialog_->borders->setRightEnabled(true);
		dialog_->borders->setTop(tabular->TopLine(cell, true));
		dialog_->borders->setBottom(tabular->BottomLine(cell, true));
		dialog_->borders->setLeft(tabular->LeftLine(cell, true));
		dialog_->borders->setRight(tabular->RightLine(cell, true));
		// repaint the setborder widget
		dialog_->borders->repaint();
		return;
	}

	dialog_->borders->setTop(tabular->TopLine(cell));
	dialog_->borders->setBottom(tabular->BottomLine(cell));
	// pay attention to left/right lines: they are only allowed
	// to set if we are in first/last cell of row or if the left/right
	// cell is also a multicolumn.
	if (tabular->IsFirstCellInRow(cell) || tabular->IsMultiColumn(cell - 1)) {
		dialog_->borders->setLeftEnabled(true);
		dialog_->borders->setLeft(tabular->LeftLine(cell));
	} else {
		dialog_->borders->setLeft(false);
		dialog_->borders->setLeftEnabled(false);
	}
	if (tabular->IsLastCellInRow(cell) || tabular->IsMultiColumn(cell + 1)) {
		dialog_->borders->setRightEnabled(true);
		dialog_->borders->setRight(tabular->RightLine(cell));
	} else {
		dialog_->borders->setRight(false);
		dialog_->borders->setRightEnabled(false);
	}
	// repaint the setborder widget
	dialog_->borders->repaint();
}


void QTabular::update_contents()
{
	LyXTabular * tabular(controller().tabular());
	int cell(controller().inset()->getActCell());

	int const row(tabular->row_of_cell(cell));
	int const col(tabular->column_of_cell(cell));

	dialog_->tabularRowED->setText(toqstr(tostr(row + 1)));
	dialog_->tabularColumnED->setText(toqstr(tostr(col + 1)));

	bool const multicol(controller().isMulticolumnCell());

	dialog_->multicolumnCB->setChecked(multicol);

	dialog_->rotateCellCB->setChecked(tabular->GetRotateCell(cell));
	dialog_->rotateTabularCB->setChecked(tabular->GetRotateTabular());

	dialog_->longTabularCB->setChecked(tabular->IsLongTabular());

	update_borders();

	LyXLength pwidth;
	string special;

	if (multicol) {
		special = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
		pwidth = tabular->GetMColumnPWidth(cell);
	} else {
		special = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
		pwidth = tabular->GetColumnPWidth(cell);
	}

	dialog_->specialAlignmentED->setText(toqstr(special));

	bool const isReadonly = bc().bp().isReadOnly();
	dialog_->specialAlignmentED->setEnabled(!isReadonly);

	LyXLength::UNIT default_unit = controller().metric() ? LyXLength::CM : LyXLength::IN;
	if (!pwidth.zero()) {
		dialog_->widthED->setText(toqstr(tostr(pwidth.value())));
		dialog_->widthUnit->setCurrentItem(pwidth.unit());
	} else {
		dialog_->widthED->setText("");
		dialog_->widthUnit->setCurrentItem(default_unit);
	}
	dialog_->widthED->setEnabled(!isReadonly);
	dialog_->widthUnit->setEnabled(!isReadonly);

	dialog_->hAlignCB->clear();
	dialog_->hAlignCB->insertItem(qt_("Left"));
	dialog_->hAlignCB->insertItem(qt_("Center"));
	dialog_->hAlignCB->insertItem(qt_("Right"));
	if (!multicol && !pwidth.zero())
		dialog_->hAlignCB->insertItem(qt_("Block"));

	int align = 0;
	switch (tabular->GetAlignment(cell)) {
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
	dialog_->hAlignCB->setCurrentItem(align);

	int valign = 0;
	switch (tabular->GetVAlignment(cell)) {
	case LyXTabular::LYX_VALIGN_TOP:
		valign = 0;
		break;
	case LyXTabular::LYX_VALIGN_CENTER:
		valign = 1;
		break;
	case LyXTabular::LYX_VALIGN_BOTTOM:
		valign = 2;
		break;
	default:
		valign = 1;
		break;
	}
	if (pwidth.zero())
		valign = 1;
	dialog_->vAlignCB->setCurrentItem(valign);

	dialog_->hAlignCB->setEnabled(true);
	dialog_->vAlignCB->setEnabled(!pwidth.zero());

	if (!tabular->IsLongTabular()) {
		dialog_->headerStatusCB->setChecked(false);
		dialog_->headerBorderAboveCB->setChecked(false);
		dialog_->headerBorderBelowCB->setChecked(false);
		dialog_->firstheaderStatusCB->setChecked(false);
		dialog_->firstheaderBorderAboveCB->setChecked(false);
		dialog_->firstheaderBorderBelowCB->setChecked(false);
		dialog_->firstheaderNoContentsCB->setChecked(false);
		dialog_->footerStatusCB->setChecked(false);
		dialog_->footerBorderAboveCB->setChecked(false);
		dialog_->footerBorderBelowCB->setChecked(false);
		dialog_->lastfooterStatusCB->setChecked(false);
		dialog_->lastfooterBorderAboveCB->setChecked(false);
		dialog_->lastfooterBorderBelowCB->setChecked(false);
		dialog_->lastfooterNoContentsCB->setChecked(false);
		dialog_->newpageCB->setChecked(false);
		return;
	}

	LyXTabular::ltType ltt;
	bool use_empty;
	bool row_set = tabular->GetRowOfLTHead(row, ltt);
	dialog_->headerStatusCB->setChecked(row_set);
	if (ltt.set) {
		dialog_->headerBorderAboveCB->setChecked(ltt.topDL);
		dialog_->headerBorderBelowCB->setChecked(ltt.bottomDL);
		use_empty = true;
	} else {
		dialog_->headerBorderAboveCB->setChecked(false);
		dialog_->headerBorderBelowCB->setChecked(false);
		dialog_->headerBorderAboveCB->setEnabled(false);
		dialog_->headerBorderBelowCB->setEnabled(false);
		dialog_->firstheaderNoContentsCB->setChecked(false);
		dialog_->firstheaderNoContentsCB->setEnabled(false);
		use_empty = false;
	}

	row_set = tabular->GetRowOfLTFirstHead(row, ltt);
	dialog_->firstheaderStatusCB->setChecked(row_set);
	if (ltt.set && (!ltt.empty || !use_empty)) {
		dialog_->firstheaderBorderAboveCB->setChecked(ltt.topDL);
		dialog_->firstheaderBorderBelowCB->setChecked(ltt.bottomDL);
	} else {
		dialog_->firstheaderBorderAboveCB->setEnabled(false);
		dialog_->firstheaderBorderBelowCB->setEnabled(false);
		dialog_->firstheaderBorderAboveCB->setChecked(false);
		dialog_->firstheaderBorderBelowCB->setChecked(false);
		if (use_empty) {
			dialog_->firstheaderNoContentsCB->setChecked(ltt.empty);
			if (ltt.empty)
				dialog_->firstheaderStatusCB->setEnabled(false);
		}
	}

	row_set = tabular->GetRowOfLTFoot(row, ltt);
	dialog_->footerStatusCB->setChecked(row_set);
	if (ltt.set) {
		dialog_->footerBorderAboveCB->setChecked(ltt.topDL);
		dialog_->footerBorderBelowCB->setChecked(ltt.bottomDL);
		use_empty = true;
	} else {
		dialog_->footerBorderAboveCB->setChecked(false);
		dialog_->footerBorderBelowCB->setChecked(false);
		dialog_->footerBorderAboveCB->setEnabled(false);
		dialog_->footerBorderBelowCB->setEnabled(false);
		dialog_->lastfooterNoContentsCB->setChecked(false);
		dialog_->lastfooterNoContentsCB->setEnabled(false);
		use_empty = false;
	}

	row_set = tabular->GetRowOfLTLastFoot(row, ltt);
		dialog_->lastfooterStatusCB->setChecked(row_set);
	if (ltt.set && (!ltt.empty || !use_empty)) {
		dialog_->lastfooterBorderAboveCB->setChecked(ltt.topDL);
		dialog_->lastfooterBorderBelowCB->setChecked(ltt.bottomDL);
	} else {
		dialog_->lastfooterBorderAboveCB->setEnabled(false);
		dialog_->lastfooterBorderBelowCB->setEnabled(false);
		dialog_->lastfooterBorderAboveCB->setChecked(false);
		dialog_->lastfooterBorderBelowCB->setChecked(false);
		if (use_empty) {
			dialog_->lastfooterNoContentsCB->setChecked(ltt.empty);
			if (ltt.empty)
				dialog_->lastfooterStatusCB->setEnabled(false);
		}
	}
	dialog_->newpageCB->setChecked(tabular->GetLTNewPage(row));
}


void QTabular::closeGUI()
{
	// ugly hack to auto-apply the stuff that hasn't been
	// yet. don't let this continue to exist ...

	InsetTabular * inset(controller().inset());
	LyXTabular * tabular(controller().tabular());

	// apply the fixed width values
	int cell = inset->getActCell();
	string str1 = LyXLength(dialog_->widthED->text().toDouble(),
			dialog_->widthUnit->currentLengthItem()).asString();
	string str2;
	LyXLength llen(tabular->GetColumnPWidth(cell));
	if (llen.zero())
		str2 = "";
	else
		str2 = llen.asString();

	if (str1 != str2) {
		if (controller().isMulticolumnCell())
			controller().set(LyXTabular::SET_MPWIDTH, str1);
		else
			controller().set(LyXTabular::SET_PWIDTH, str1);
	}

	// apply the special alignment
	str1 = fromqstr(dialog_->specialAlignmentED->text());
	if (controller().isMulticolumnCell())
		str2 = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
	else
		str2 = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);

	if (str1 != str2) {
		if (controller().isMulticolumnCell())
			controller().set(LyXTabular::SET_SPECIAL_MULTI, str1);
		else
			controller().set(LyXTabular::SET_SPECIAL_COLUMN, str1);
	}
}
