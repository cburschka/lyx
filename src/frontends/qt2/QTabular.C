/**
 * \file QTabular.C
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

#include "debug.h"
#include "ControlTabular.h"
#include "qt_helpers.h"
#include "support/tostr.h"

#include "QTabularDialog.h"
#include "QTabular.h"
#include "Qt2BC.h"
#include "ButtonController.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include "lengthcombo.h"
#include "qsetborder.h"

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlTabular, QView<QTabularDialog> > base_class;

QTabular::QTabular(Dialog & parent)
	: base_class(parent, _("LyX: Table Settings"))
{
}


void QTabular::build_dialog()
{
	dialog_.reset(new QTabularDialog(this));

	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->multicolumnCB);
	bcview().addReadOnly(dialog_->rotateCellCB);
	bcview().addReadOnly(dialog_->rotateTabularCB);
	bcview().addReadOnly(dialog_->specialAlignmentED);
	bcview().addReadOnly(dialog_->widthED);
	bcview().addReadOnly(dialog_->widthUnit);
	bcview().addReadOnly(dialog_->hAlignCB);
	bcview().addReadOnly(dialog_->vAlignCB);
	bcview().addReadOnly(dialog_->borderSetPB);
	bcview().addReadOnly(dialog_->borderUnsetPB);
	bcview().addReadOnly(dialog_->borders);
	bcview().addReadOnly(dialog_->longTabularCB);
	bcview().addReadOnly(dialog_->headerStatusCB);
	bcview().addReadOnly(dialog_->headerBorderAboveCB);
	bcview().addReadOnly(dialog_->headerBorderBelowCB);
	bcview().addReadOnly(dialog_->firstheaderStatusCB);
	bcview().addReadOnly(dialog_->firstheaderBorderAboveCB);
	bcview().addReadOnly(dialog_->firstheaderBorderBelowCB);
	bcview().addReadOnly(dialog_->firstheaderNoContentsCB);
	bcview().addReadOnly(dialog_->footerStatusCB);
	bcview().addReadOnly(dialog_->footerBorderAboveCB);
	bcview().addReadOnly(dialog_->footerBorderBelowCB);
	bcview().addReadOnly(dialog_->lastfooterStatusCB);
	bcview().addReadOnly(dialog_->lastfooterBorderAboveCB);
	bcview().addReadOnly(dialog_->lastfooterBorderBelowCB);
	bcview().addReadOnly(dialog_->lastfooterNoContentsCB);
	bcview().addReadOnly(dialog_->newpageCB);
}


bool QTabular::isValid()
{
	return true;
}


void QTabular::update_borders()
{
	LyXTabular const & tabular = controller().tabular();
	int const cell = controller().getActiveCell();
	bool const isMulticolumnCell = tabular.isMultiColumn(cell);

	if (!isMulticolumnCell) {
		dialog_->borders->setLeftEnabled(true);
		dialog_->borders->setRightEnabled(true);
		dialog_->borders->setTop(tabular.topLine(cell, true));
		dialog_->borders->setBottom(tabular.bottomLine(cell, true));
		dialog_->borders->setLeft(tabular.leftLine(cell, true));
		dialog_->borders->setRight(tabular.rightLine(cell, true));
		// repaint the setborder widget
		dialog_->borders->repaint();
		return;
	}

	dialog_->borders->setTop(tabular.topLine(cell));
	dialog_->borders->setBottom(tabular.bottomLine(cell));
	// pay attention to left/right lines: they are only allowed
	// to set if we are in first/last cell of row or if the left/right
	// cell is also a multicolumn.
	if (tabular.isFirstCellInRow(cell) || tabular.isMultiColumn(cell - 1)) {
		dialog_->borders->setLeftEnabled(true);
		dialog_->borders->setLeft(tabular.leftLine(cell));
	} else {
		dialog_->borders->setLeft(false);
		dialog_->borders->setLeftEnabled(false);
	}
	if (tabular.isLastCellInRow(cell) || tabular.isMultiColumn(cell + 1)) {
		dialog_->borders->setRightEnabled(true);
		dialog_->borders->setRight(tabular.rightLine(cell));
	} else {
		dialog_->borders->setRight(false);
		dialog_->borders->setRightEnabled(false);
	}
	// repaint the setborder widget
	dialog_->borders->repaint();
}


void QTabular::update_contents()
{
	LyXTabular const & tabular(controller().tabular());
	int const cell = controller().getActiveCell();

	int const row(tabular.row_of_cell(cell));
	int const col(tabular.column_of_cell(cell));

	dialog_->tabularRowED->setText(toqstr(tostr(row + 1)));
	dialog_->tabularColumnED->setText(toqstr(tostr(col + 1)));

	bool const multicol(tabular.isMultiColumn(cell));

	dialog_->multicolumnCB->setChecked(multicol);

	dialog_->rotateCellCB->setChecked(tabular.getRotateCell(cell));
	dialog_->rotateTabularCB->setChecked(tabular.getRotateTabular());

	dialog_->longTabularCB->setChecked(tabular.isLongTabular());

	update_borders();

	LyXLength pwidth;
	string special;

	if (multicol) {
		special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
		pwidth = tabular.getMColumnPWidth(cell);
	} else {
		special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
		pwidth = tabular.getColumnPWidth(cell);
	}

	dialog_->specialAlignmentED->setText(toqstr(special));

	bool const isReadonly = bc().bp().isReadOnly();
	dialog_->specialAlignmentED->setEnabled(!isReadonly);

	LyXLength::UNIT default_unit = controller().useMetricUnits() ? LyXLength::CM : LyXLength::IN;

	string colwidth;
	if (!pwidth.zero())
		colwidth = pwidth.asString();
	lengthToWidgets(dialog_->widthED, dialog_->widthUnit,
		colwidth, default_unit);

	dialog_->widthED->setEnabled(!isReadonly);
	dialog_->widthUnit->setEnabled(!isReadonly);

	dialog_->hAlignCB->clear();
	dialog_->hAlignCB->insertItem(qt_("Left"));
	dialog_->hAlignCB->insertItem(qt_("Center"));
	dialog_->hAlignCB->insertItem(qt_("Right"));
	if (!multicol && !pwidth.zero())
		dialog_->hAlignCB->insertItem(qt_("Block"));

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
	dialog_->hAlignCB->setCurrentItem(align);

	int valign = 0;
	switch (tabular.getVAlignment(cell)) {
	case LyXTabular::LYX_VALIGN_TOP:
		valign = 0;
		break;
	case LyXTabular::LYX_VALIGN_MIDDLE:
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

	if (!tabular.isLongTabular()) {
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
	bool row_set = tabular.getRowOfLTHead(row, ltt);
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

	row_set = tabular.getRowOfLTFirstHead(row, ltt);
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

	row_set = tabular.getRowOfLTFoot(row, ltt);
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

	row_set = tabular.getRowOfLTLastFoot(row, ltt);
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
	dialog_->newpageCB->setChecked(tabular.getLTNewPage(row));
}


void QTabular::closeGUI()
{
	// ugly hack to auto-apply the stuff that hasn't been
	// yet. don't let this continue to exist ...

	// Subtle here, we must /not/ apply any changes and
	// then refer to tabular, as it will have been freed
	// since the changes update the actual controller().tabular()
	LyXTabular const & tabular(controller().tabular());

	// apply the fixed width values
	int const cell = controller().getActiveCell();
	bool const multicol = tabular.isMultiColumn(cell);
	string width = widgetsToLength(dialog_->widthED, dialog_->widthUnit);
	string width2;

	LyXLength llen = tabular.getColumnPWidth(cell);
	LyXLength llenMulti = tabular.getMColumnPWidth(cell);

	if (multicol && !llenMulti.zero())
			width2 = llenMulti.asString();
	else if (!multicol && !llen.zero())
			width2 = llen.asString();

	// apply the special alignment
	string const sa1 = fromqstr(dialog_->specialAlignmentED->text());
	string sa2;

	if (multicol)
		sa2 = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
	else
		sa2 = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);

	if (sa1 != sa2) {
		if (multicol)
			controller().set(LyXTabular::SET_SPECIAL_MULTI, sa1);
		else
			controller().set(LyXTabular::SET_SPECIAL_COLUMN, sa1);
	}

	if (width != width2) {
		if (multicol)
			controller().set(LyXTabular::SET_MPWIDTH, width);
		else
			controller().set(LyXTabular::SET_PWIDTH, width);
	}
}

} // namespace frontend
} // namespace lyx
