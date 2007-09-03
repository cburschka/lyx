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

/////////////////////////////////////////////////////////////////////
//
// GuiTabularDialog
//
/////////////////////////////////////////////////////////////////////

GuiTabularDialog::GuiTabularDialog(GuiTabular * form)
	: form_(form)
{
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
}


void GuiTabularDialog::change_adaptor()
{
	form_->changed();
}


void GuiTabularDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiTabularDialog::booktabsChanged(bool)
{
	form_->changed();
	form_->controller().booktabs(booktabsRB->isChecked());
	form_->update_borders();
}


void GuiTabularDialog::topspace_changed()
{
	switch(topspaceCO->currentIndex()) {
		case 0: {
			form_->controller().set(Tabular::SET_TOP_SPACE, "");
				topspaceED->setEnabled(false);
				topspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			form_->controller().set(Tabular::SET_TOP_SPACE, "default");
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!topspaceED->text().isEmpty())
				form_->controller().set(Tabular::SET_TOP_SPACE,
					widgetsToLength(topspaceED, topspaceUnit));
			if (!form_->bc().policy().isReadOnly()) {
				topspaceED->setEnabled(true);
				topspaceUnit->setEnabled(true);
			}
			break;
		}
	}
	form_->changed();
}


void GuiTabularDialog::bottomspace_changed()
{
	switch(bottomspaceCO->currentIndex()) {
		case 0: {
			form_->controller().set(Tabular::SET_BOTTOM_SPACE, "");
				bottomspaceED->setEnabled(false);
				bottomspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			form_->controller().set(Tabular::SET_BOTTOM_SPACE, "default");
			bottomspaceED->setEnabled(false);
			bottomspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!bottomspaceED->text().isEmpty())
				form_->controller().set(Tabular::SET_BOTTOM_SPACE,
					widgetsToLength(bottomspaceED, bottomspaceUnit));
			if (!form_->bc().policy().isReadOnly()) {
				bottomspaceED->setEnabled(true);
				bottomspaceUnit->setEnabled(true);
			}
			break;
		}
	}
	form_->changed();
}


void GuiTabularDialog::interlinespace_changed()
{
	switch(interlinespaceCO->currentIndex()) {
		case 0: {
			form_->controller().set(Tabular::SET_INTERLINE_SPACE, "");
				interlinespaceED->setEnabled(false);
				interlinespaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			form_->controller().set(Tabular::SET_INTERLINE_SPACE, "default");
			interlinespaceED->setEnabled(false);
			interlinespaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!interlinespaceED->text().isEmpty())
				form_->controller().set(Tabular::SET_INTERLINE_SPACE,
					widgetsToLength(interlinespaceED, interlinespaceUnit));
			if (!form_->bc().policy().isReadOnly()) {
				interlinespaceED->setEnabled(true);
				interlinespaceUnit->setEnabled(true);
			}
			break;
		}
	}
	form_->changed();
}


void GuiTabularDialog::close_clicked()
{
	form_->closeGUI();
	form_->slotClose();
}


void GuiTabularDialog::borderSet_clicked()
{
	form_->controller().set(Tabular::SET_ALL_LINES);
	form_->update_borders();
	form_->changed();
}


void GuiTabularDialog::borderUnset_clicked()
{
	form_->controller().set(Tabular::UNSET_ALL_LINES);
	form_->update_borders();
	form_->changed();
}


void GuiTabularDialog::leftBorder_changed()
{
	form_->controller().toggleLeftLine();
	form_->changed();
}


void GuiTabularDialog::rightBorder_changed()
{
	form_->controller().toggleRightLine();
	form_->changed();
}


void GuiTabularDialog::topBorder_changed()
{
	form_->controller().toggleTopLine();
	form_->changed();
}


void GuiTabularDialog::bottomBorder_changed()
{
	form_->controller().toggleBottomLine();
	form_->changed();
}


void GuiTabularDialog::specialAlignment_changed()
{
	string special = fromqstr(specialAlignmentED->text());
	form_->controller().setSpecial(special);
	form_->changed();
}


void GuiTabularDialog::width_changed()
{
	form_->changed();
	string const width = widgetsToLength(widthED, widthUnit);
	form_->controller().setWidth(width);
}


void GuiTabularDialog::multicolumn_clicked()
{
	form_->controller().toggleMultiColumn();
	form_->changed();
}


void GuiTabularDialog::rotateTabular()
{
	form_->controller().rotateTabular(rotateTabularCB->isChecked());
	form_->changed();
}


void GuiTabularDialog::rotateCell()
{
	form_->controller().rotateCell(rotateCellCB->isChecked());
	form_->changed();
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

	form_->controller().halign(h);
}


void GuiTabularDialog::vAlign_changed(int align)
{
	ControlTabular::VALIGN v = ControlTabular::TOP;

	switch (align) {
		case 0: v = ControlTabular::TOP; break;
		case 1: v = ControlTabular::MIDDLE; break;
		case 2: v = ControlTabular::BOTTOM; break;
	}

	form_->controller().valign(v);
}


void GuiTabularDialog::longTabular()
{
	form_->controller().longTabular(longTabularCB->isChecked());
	form_->changed();
}


void GuiTabularDialog::ltNewpage_clicked()
{
	form_->controller().set(Tabular::SET_LTNEWPAGE);
	form_->changed();
}


void GuiTabularDialog::ltHeaderStatus_clicked()
{
	bool enable(headerStatusCB->isChecked());
	if (enable)
		form_->controller().set(Tabular::SET_LTHEAD, "");
	else
		form_->controller().set(Tabular::UNSET_LTHEAD, "");
	headerBorderAboveCB->setEnabled(enable);
	headerBorderBelowCB->setEnabled(enable);
	firstheaderNoContentsCB->setEnabled(enable);
	form_->changed();
}


void GuiTabularDialog::ltHeaderBorderAbove_clicked()
{
	if (headerBorderAboveCB->isChecked())
		form_->controller().set(Tabular::SET_LTHEAD, "dl_above");
	else
		form_->controller().set(Tabular::UNSET_LTHEAD, "dl_above");
	form_->changed();
}


void GuiTabularDialog::ltHeaderBorderBelow_clicked()
{
	if (headerBorderBelowCB->isChecked())
		form_->controller().set(Tabular::SET_LTHEAD, "dl_below");
	else
		form_->controller().set(Tabular::UNSET_LTHEAD, "dl_below");
	form_->changed();
}


void GuiTabularDialog::ltFirstHeaderBorderAbove_clicked()
{
	if (firstheaderBorderAboveCB->isChecked())
		form_->controller().set(Tabular::SET_LTFIRSTHEAD, "dl_above");
	else
		form_->controller().set(Tabular::UNSET_LTFIRSTHEAD, "dl_above");
	form_->changed();
}


void GuiTabularDialog::ltFirstHeaderBorderBelow_clicked()
{
	if (firstheaderBorderBelowCB->isChecked())
		form_->controller().set(Tabular::SET_LTFIRSTHEAD, "dl_below");
	else
		form_->controller().set(Tabular::UNSET_LTFIRSTHEAD, "dl_below");
	form_->changed();
}


void GuiTabularDialog::ltFirstHeaderStatus_clicked()
{
	bool enable(firstheaderStatusCB->isChecked());
	if (enable)
		form_->controller().set(Tabular::SET_LTFIRSTHEAD, "");
	else
		form_->controller().set(Tabular::UNSET_LTFIRSTHEAD, "");
	firstheaderBorderAboveCB->setEnabled(enable);
	firstheaderBorderBelowCB->setEnabled(enable);
	form_->changed();
}


void GuiTabularDialog::ltFirstHeaderEmpty_clicked()
{
	bool enable(firstheaderNoContentsCB->isChecked());
	if (enable)
		form_->controller().set(Tabular::SET_LTFIRSTHEAD, "empty");
	else
		form_->controller().set(Tabular::UNSET_LTFIRSTHEAD, "empty");
	firstheaderStatusCB->setEnabled(!enable);
	firstheaderBorderAboveCB->setEnabled(!enable);
	firstheaderBorderBelowCB->setEnabled(!enable);
	form_->changed();
}


void GuiTabularDialog::ltFooterStatus_clicked()
{
	bool enable(footerStatusCB->isChecked());
	if (enable)
		form_->controller().set(Tabular::SET_LTFOOT, "");
	else
		form_->controller().set(Tabular::UNSET_LTFOOT, "");
	footerBorderAboveCB->setEnabled(enable);
	footerBorderBelowCB->setEnabled(enable);
	lastfooterNoContentsCB->setEnabled(enable);
	form_->changed();
}


void GuiTabularDialog::ltFooterBorderAbove_clicked()
{
	if (footerBorderAboveCB->isChecked())
		form_->controller().set(Tabular::SET_LTFOOT, "dl_above");
	else
		form_->controller().set(Tabular::UNSET_LTFOOT, "dl_above");
	form_->changed();
}


void GuiTabularDialog::ltFooterBorderBelow_clicked()
{
	if (footerBorderBelowCB->isChecked())
		form_->controller().set(Tabular::SET_LTFOOT, "dl_below");
	else
		form_->controller().set(Tabular::UNSET_LTFOOT, "dl_below");
	form_->changed();
}


void GuiTabularDialog::ltLastFooterStatus_clicked()
{
	bool enable(lastfooterStatusCB->isChecked());
	if (enable)
		form_->controller().set(Tabular::SET_LTLASTFOOT, "");
	else
		form_->controller().set(Tabular::UNSET_LTLASTFOOT, "");
	lastfooterBorderAboveCB->setEnabled(enable);
	lastfooterBorderBelowCB->setEnabled(enable);
	form_->changed();
}


void GuiTabularDialog::ltLastFooterBorderAbove_clicked()
{
	if (lastfooterBorderAboveCB->isChecked())
		form_->controller().set(Tabular::SET_LTLASTFOOT, "dl_above");
	else
		form_->controller().set(Tabular::UNSET_LTLASTFOOT, "dl_above");
	form_->changed();
}


void GuiTabularDialog::ltLastFooterBorderBelow_clicked()
{
	if (lastfooterBorderBelowCB->isChecked())
		form_->controller().set(Tabular::SET_LTLASTFOOT, "dl_below");
	else
		form_->controller().set(Tabular::UNSET_LTLASTFOOT, "dl_below");
	form_->changed();
}


void GuiTabularDialog::ltLastFooterEmpty_clicked()
{
	bool enable(lastfooterNoContentsCB->isChecked());
	if (enable)
		form_->controller().set(Tabular::SET_LTLASTFOOT, "empty");
	else
		form_->controller().set(Tabular::UNSET_LTLASTFOOT, "empty");
	lastfooterStatusCB->setEnabled(!enable);
	lastfooterBorderAboveCB->setEnabled(!enable);
	lastfooterBorderBelowCB->setEnabled(!enable);
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// GuiTabular
//
/////////////////////////////////////////////////////////////////////


GuiTabular::GuiTabular(GuiDialog & parent)
	: GuiView<GuiTabularDialog>(parent, _("Table Settings"))
{
}


void GuiTabular::build_dialog()
{
	dialog_.reset(new GuiTabularDialog(this));

	bc().setCancel(dialog_->closePB);

	bc().addReadOnly(dialog_->topspaceED);
	bc().addReadOnly(dialog_->topspaceUnit);
	bc().addReadOnly(dialog_->topspaceCO);
	bc().addReadOnly(dialog_->bottomspaceED);
	bc().addReadOnly(dialog_->bottomspaceUnit);
	bc().addReadOnly(dialog_->bottomspaceCO);
	bc().addReadOnly(dialog_->interlinespaceED);
	bc().addReadOnly(dialog_->interlinespaceUnit);
	bc().addReadOnly(dialog_->interlinespaceCO);
	bc().addReadOnly(dialog_->borderDefaultRB);
	bc().addReadOnly(dialog_->booktabsRB);

	bc().addReadOnly(dialog_->multicolumnCB);
	bc().addReadOnly(dialog_->rotateCellCB);
	bc().addReadOnly(dialog_->rotateTabularCB);
	bc().addReadOnly(dialog_->specialAlignmentED);
	bc().addReadOnly(dialog_->widthED);
	bc().addReadOnly(dialog_->widthUnit);
	bc().addReadOnly(dialog_->hAlignCB);
	bc().addReadOnly(dialog_->vAlignCB);
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

	// initialize the length validator
	bc().addCheckedLineEdit(dialog_->widthED, dialog_->fixedWidthColLA);
	bc().addCheckedLineEdit(dialog_->topspaceED, dialog_->topspaceLA);
	bc().addCheckedLineEdit(dialog_->bottomspaceED, dialog_->bottomspaceLA);
	bc().addCheckedLineEdit(dialog_->interlinespaceED, dialog_->interlinespaceLA);
}


bool GuiTabular::isValid()
{
	return true;
}


void GuiTabular::update_borders()
{
	Tabular const & tabular = controller().tabular();
	Tabular::idx_type const cell = controller().getActiveCell();
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


void GuiTabular::update_contents()
{
	controller().initialiseParams(string());

	Tabular const & tabular(controller().tabular());
	Tabular::idx_type const cell = controller().getActiveCell();

	Tabular::row_type const row(tabular.row_of_cell(cell));
	Tabular::col_type const col(tabular.column_of_cell(cell));

	dialog_->tabularRowED->setText(toqstr(convert<string>(row + 1)));
	dialog_->tabularColumnED->setText(toqstr(convert<string>(col + 1)));

	bool const multicol(tabular.isMultiColumn(cell));

	dialog_->multicolumnCB->setChecked(multicol);

	dialog_->rotateCellCB->setChecked(tabular.getRotateCell(cell));
	dialog_->rotateTabularCB->setChecked(tabular.getRotateTabular());

	dialog_->longTabularCB->setChecked(tabular.isLongTabular());

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

	dialog_->specialAlignmentED->setText(toqstr(special));

	bool const isReadonly = bc().policy().isReadOnly();
	dialog_->specialAlignmentED->setEnabled(!isReadonly);

	Length::UNIT default_unit =
		controller().useMetricUnits() ? Length::CM : Length::IN;

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
		dialog_->hAlignCB->addItem(qt_("Justified"));

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

	Tabular::ltType ltt;
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


void GuiTabular::closeGUI()
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
	string width = widgetsToLength(dialog_->widthED, dialog_->widthUnit);
	string width2;

	Length llen = tabular.getColumnPWidth(cell);
	Length llenMulti = tabular.getMColumnPWidth(cell);

	if (multicol && !llenMulti.zero())
			width2 = llenMulti.asString();
	else if (!multicol && !llen.zero())
			width2 = llen.asString();

	// apply the special alignment
	docstring const sa1 = qstring_to_ucs4(dialog_->specialAlignmentED->text());
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
	switch (dialog_->topspaceCO->currentIndex()) {
		case 0:
			controller().set(Tabular::SET_TOP_SPACE, "");
			break;
		case 1:
			controller().set(Tabular::SET_TOP_SPACE, "default");
			break;
		case 2:
			controller().set(Tabular::SET_TOP_SPACE,
				widgetsToLength(dialog_->topspaceED,
					dialog_->topspaceUnit));
			break;
	}

	switch (dialog_->bottomspaceCO->currentIndex()) {
		case 0:
			controller().set(Tabular::SET_BOTTOM_SPACE, "");
			break;
		case 1:
			controller().set(Tabular::SET_BOTTOM_SPACE, "default");
			break;
		case 2:
			controller().set(Tabular::SET_BOTTOM_SPACE,
				widgetsToLength(dialog_->bottomspaceED,
					dialog_->bottomspaceUnit));
			break;
	}

	switch (dialog_->interlinespaceCO->currentIndex()) {
		case 0:
			controller().set(Tabular::SET_INTERLINE_SPACE, "");
			break;
		case 1:
			controller().set(Tabular::SET_INTERLINE_SPACE, "default");
			break;
		case 2:
			controller().set(Tabular::SET_INTERLINE_SPACE,
				widgetsToLength(dialog_->interlinespaceED,
					dialog_->interlinespaceUnit));
			break;
	}
*/
}

} // namespace frontend
} // namespace lyx

#include "GuiTabular_moc.cpp"
