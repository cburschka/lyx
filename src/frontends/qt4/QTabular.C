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

#include "checkedwidgets.h"
#include "QTabular.h"
#include "QTabularDialog.h"
#include "Qt2BC.h"

#include "lengthcombo.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlTabular.h"

#include "support/convert.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include "qsetborder.h"

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlTabular, QView<QTabularDialog> > base_class;

QTabular::QTabular(Dialog & parent)
	: base_class(parent, lyx::to_utf8(_("Table Settings")))
{
}


void QTabular::build_dialog()
{
	dialog_.reset(new QTabularDialog(this));

	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->topspaceED);
	bcview().addReadOnly(dialog_->topspaceUnit);
	bcview().addReadOnly(dialog_->topspaceCO);
	bcview().addReadOnly(dialog_->bottomspaceED);
	bcview().addReadOnly(dialog_->bottomspaceUnit);
	bcview().addReadOnly(dialog_->bottomspaceCO);
	bcview().addReadOnly(dialog_->interlinespaceED);
	bcview().addReadOnly(dialog_->interlinespaceUnit);
	bcview().addReadOnly(dialog_->interlinespaceCO);
	bcview().addReadOnly(dialog_->borderDefaultRB);
	bcview().addReadOnly(dialog_->booktabsRB);

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

	// initialize the length validator
	addCheckedLineEdit(bcview(), dialog_->widthED,
		dialog_->fixedWidthColLA);
	addCheckedLineEdit(bcview(), dialog_->topspaceED,
		dialog_->topspaceLA);
	addCheckedLineEdit(bcview(), dialog_->bottomspaceED,
		dialog_->bottomspaceLA);
	addCheckedLineEdit(bcview(), dialog_->interlinespaceED,
		dialog_->interlinespaceLA);
}


bool QTabular::isValid()
{
	return true;
}


void QTabular::update_borders()
{
	LyXTabular const & tabular = controller().tabular();
	LyXTabular::idx_type const cell = controller().getActiveCell();
	bool const isMulticolumnCell = tabular.isMultiColumn(cell);

	if (!isMulticolumnCell) {
		dialog_->borders->setLeftEnabled(true);
		dialog_->borders->setRightEnabled(true);
		dialog_->borders->setTop(tabular.topLine(cell, true));
		dialog_->borders->setBottom(tabular.bottomLine(cell, true));
		dialog_->borders->setLeft(tabular.leftLine(cell, true));
		dialog_->borders->setRight(tabular.rightLine(cell, true));
		// repaint the setborder widget
		dialog_->borders->update();
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
	dialog_->borders->update();
}


void QTabular::update_contents()
{
	LyXTabular const & tabular(controller().tabular());
	LyXTabular::idx_type const cell = controller().getActiveCell();

	LyXTabular::row_type const row(tabular.row_of_cell(cell));
	LyXTabular::col_type const col(tabular.column_of_cell(cell));

	dialog_->tabularRowED->setText(toqstr(convert<string>(row + 1)));
	dialog_->tabularColumnED->setText(toqstr(convert<string>(col + 1)));

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

	dialog_->borderDefaultRB->setChecked(!tabular.useBookTabs());
	dialog_->booktabsRB->setChecked(tabular.useBookTabs());

	if (tabular.row_info[row].top_space.empty()
	    && !tabular.row_info[row].top_space_default) {
		dialog_->topspaceCO->setCurrentIndex(0);
	} else if (tabular.row_info[row].top_space_default) {
		dialog_->topspaceCO->setCurrentIndex(1);
	} else {
		dialog_->topspaceCO->setCurrentIndex(2);
		lengthToWidgets(dialog_->topspaceED, 
				dialog_->topspaceUnit,
				tabular.row_info[row].top_space.asString(),
				default_unit);
	}
	dialog_->topspaceED->setEnabled(!isReadonly 
		&& (dialog_->topspaceCO->currentIndex() == 2));
	dialog_->topspaceUnit->setEnabled(!isReadonly 
		&& (dialog_->topspaceCO->currentIndex() == 2));
	dialog_->topspaceCO->setEnabled(!isReadonly);

	if (tabular.row_info[row].bottom_space.empty()
	    && !tabular.row_info[row].bottom_space_default) {
		dialog_->bottomspaceCO->setCurrentIndex(0);
	} else if (tabular.row_info[row].bottom_space_default) {
		dialog_->bottomspaceCO->setCurrentIndex(1);
	} else {
		dialog_->bottomspaceCO->setCurrentIndex(2);
		lengthToWidgets(dialog_->bottomspaceED, 
				dialog_->bottomspaceUnit,
				tabular.row_info[row].bottom_space.asString(),
				default_unit);
	}
	dialog_->bottomspaceED->setEnabled(!isReadonly 
		&& (dialog_->bottomspaceCO->currentIndex() == 2));
	dialog_->bottomspaceUnit->setEnabled(!isReadonly 
		&& (dialog_->bottomspaceCO->currentIndex() == 2));
	dialog_->bottomspaceCO->setEnabled(!isReadonly);

	if (tabular.row_info[row].interline_space.empty()
	    && !tabular.row_info[row].interline_space_default) {
		dialog_->interlinespaceCO->setCurrentIndex(0);
	} else if (tabular.row_info[row].interline_space_default) {
		dialog_->interlinespaceCO->setCurrentIndex(1);
	} else {
		dialog_->interlinespaceCO->setCurrentIndex(2);
		lengthToWidgets(dialog_->interlinespaceED, 
				dialog_->interlinespaceUnit,
				tabular.row_info[row].interline_space.asString(),
				default_unit);
	}
	dialog_->interlinespaceED->setEnabled(!isReadonly 
		&& (dialog_->interlinespaceCO->currentIndex() == 2));
	dialog_->interlinespaceUnit->setEnabled(!isReadonly 
		&& (dialog_->interlinespaceCO->currentIndex() == 2));
	dialog_->interlinespaceCO->setEnabled(!isReadonly);

	string colwidth;
	if (!pwidth.zero())
		colwidth = pwidth.asString();
	lengthToWidgets(dialog_->widthED, dialog_->widthUnit,
		colwidth, default_unit);

	dialog_->widthED->setEnabled(!isReadonly);
	dialog_->widthUnit->setEnabled(!isReadonly);

	dialog_->hAlignCB->clear();
	dialog_->hAlignCB->addItem(qt_("Left"));
	dialog_->hAlignCB->addItem(qt_("Center"));
	dialog_->hAlignCB->addItem(qt_("Right"));
	if (!multicol && !pwidth.zero())
		dialog_->hAlignCB->addItem(qt_("Block"));

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
	dialog_->hAlignCB->setCurrentIndex(align);

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
	dialog_->vAlignCB->setCurrentIndex(valign);

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
		dialog_->newpageCB->setEnabled(false);
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
	LyXTabular::idx_type const cell = controller().getActiveCell();
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

	/* DO WE NEED THIS?
	switch (dialog_->topspaceCO->currentIndex()) {
		case 0:
			controller().set(LyXTabular::SET_TOP_SPACE, "");
			break;
		case 1:
			controller().set(LyXTabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			controller().set(LyXTabular::SET_TOP_SPACE,
				widgetsToLength(dialog_->topspaceED, 
					dialog_->topspaceUnit));
			break;
	}

	switch (dialog_->bottomspaceCO->currentIndex()) {
		case 0:
			controller().set(LyXTabular::SET_BOTTOM_SPACE, "");
			break;
		case 1:
			controller().set(LyXTabular::SET_BOTTOM_SPACE, "default");
			break;
		case 2:
			controller().set(LyXTabular::SET_BOTTOM_SPACE,
				widgetsToLength(dialog_->bottomspaceED, 
					dialog_->bottomspaceUnit));
			break;
	}

	switch (dialog_->interlinespaceCO->currentIndex()) {
		case 0:
			controller().set(LyXTabular::SET_INTERLINE_SPACE, "");
			break;
		case 1:
			controller().set(LyXTabular::SET_INTERLINE_SPACE, "default");
			break;
		case 2:
			controller().set(LyXTabular::SET_INTERLINE_SPACE,
				widgetsToLength(dialog_->interlinespaceED, 
					dialog_->interlinespaceUnit));
			break;
	}
*/
}

} // namespace frontend
} // namespace lyx
