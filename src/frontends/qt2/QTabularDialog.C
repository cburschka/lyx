/**
 * \file QTabularDialog.C
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


#include "ControlTabular.h"

#include "QTabular.h"
#include "QTabularDialog.h"
#include "tabular.h"
#include "lengthcombo.h"
#include "qsetborder.h"
#include "qt_helpers.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>


QTabularDialog::QTabularDialog(QTabular * form)
	: QTabularDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QTabularDialog::change_adaptor()
{
	form_->changed();
}


void QTabularDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

void QTabularDialog::close_clicked()
{
	form_->closeGUI();
}

void QTabularDialog::columnAppend_clicked()
{
	form_->controller().set(LyXTabular::APPEND_COLUMN);
}


void QTabularDialog::rowAppend_clicked()
{
	form_->controller().set(LyXTabular::APPEND_ROW);
}


void QTabularDialog::columnDelete_clicked()
{
	form_->controller().set(LyXTabular::DELETE_COLUMN);
}


void QTabularDialog::rowDelete_clicked()
{
	form_->controller().set(LyXTabular::DELETE_ROW);
}


void QTabularDialog::borderSet_clicked()
{
	form_->controller().set(LyXTabular::SET_ALL_LINES);
	form_->update_borders();
	form_->changed();
}

void QTabularDialog::borderUnset_clicked()
{
	form_->controller().set(LyXTabular::UNSET_ALL_LINES);
	form_->update_borders();
	form_->changed();
}


void QTabularDialog::leftBorder_changed()
{
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(LyXTabular::M_TOGGLE_LINE_LEFT);
	else
		form_->controller().set(LyXTabular::TOGGLE_LINE_LEFT);
	form_->changed();
}


void QTabularDialog::rightBorder_changed()
{
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(LyXTabular::M_TOGGLE_LINE_RIGHT);
	else
		form_->controller().set(LyXTabular::TOGGLE_LINE_RIGHT);
	form_->changed();
}


void QTabularDialog::topBorder_changed()
{
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(LyXTabular::M_TOGGLE_LINE_TOP);
	else
		form_->controller().set(LyXTabular::TOGGLE_LINE_TOP);
	form_->changed();
}


void QTabularDialog::bottomBorder_changed()
{
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(LyXTabular::M_TOGGLE_LINE_BOTTOM);
	else
		form_->controller().set(LyXTabular::TOGGLE_LINE_BOTTOM);
	form_->changed();
}


void QTabularDialog::specialAlignment_changed()
{
	string special = fromqstr(specialAlignmentED->text());
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(LyXTabular::SET_SPECIAL_MULTI, special);
	else
		form_->controller().set(LyXTabular::SET_SPECIAL_COLUMN, special);
}


void QTabularDialog::width_changed()
{
	string const width =
		LyXLength(widthED->text().toDouble(),
			widthUnit->currentLengthItem()).asString();
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(LyXTabular::SET_MPWIDTH, width);
	else
		form_->controller().set(LyXTabular::SET_PWIDTH, width);
	form_->changed();
	form_->update_contents();
}


void QTabularDialog::multicolumn_clicked()
{
	form_->controller().set(LyXTabular::MULTICOLUMN);
	form_->changed();
	form_->update_contents();
}


void QTabularDialog::rotateTabular_checked(int state)
{
	switch (state) {
	case 0:
		form_->controller().set(LyXTabular::UNSET_ROTATE_TABULAR);
		break;
	case 1:
		// "no change state", should not happen
		break;
	case 2:
		form_->controller().set(LyXTabular::SET_ROTATE_TABULAR);
		break;
	}
}


void QTabularDialog::rotateCell_checked(int state)
{
	switch (state) {
	case 0:
		form_->controller().set(LyXTabular::UNSET_ROTATE_CELL);
		break;
	case 1:
		// "no change state", should not happen
		break;
	case 2:
		form_->controller().set(LyXTabular::SET_ROTATE_CELL);
		break;
	}
}


void QTabularDialog::hAlign_changed(int align)
{
	LyXTabular::Feature num = LyXTabular::ALIGN_LEFT;
	LyXTabular::Feature multi_num = LyXTabular::M_ALIGN_LEFT;

	switch (align) {
		case 0:
		{
			num = LyXTabular::ALIGN_LEFT;
			multi_num = LyXTabular::M_ALIGN_LEFT;
			break;
		}
		case 1:
		{
			num = LyXTabular::ALIGN_CENTER;
			multi_num = LyXTabular::M_ALIGN_CENTER;
			break;
		}
		case 2:
		{
			num = LyXTabular::ALIGN_RIGHT;
			multi_num = LyXTabular::M_ALIGN_RIGHT;
			break;
		case 3:
		{
			num = LyXTabular::ALIGN_BLOCK;
			//multi_num: no equivalent
			break;
		}
		}
	}
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(multi_num);
	else
		form_->controller().set(num);
}


void QTabularDialog::vAlign_changed(int align)
{
	LyXTabular::Feature num = LyXTabular::VALIGN_CENTER;
	LyXTabular::Feature multi_num = LyXTabular::M_VALIGN_CENTER;

	switch (align) {
		case 0:
		{
			num = LyXTabular::VALIGN_TOP;
			multi_num = LyXTabular::M_VALIGN_TOP;
			break;
		}
		case 1:
		{
			num = LyXTabular::VALIGN_CENTER;
			multi_num = LyXTabular::M_VALIGN_CENTER;
			break;
		}
		case 2:
		{
			num = LyXTabular::VALIGN_BOTTOM;
			multi_num = LyXTabular::M_VALIGN_BOTTOM;
			break;
		}
	}
	if (form_->controller().isMulticolumnCell())
		form_->controller().set(multi_num);
	else
		form_->controller().set(num);
}


void QTabularDialog::longTabular_changed(int state)
{
	switch (state) {
	case 0:
		form_->controller().set(LyXTabular::UNSET_LONGTABULAR);
		break;
	case 1:
		// "no change state", should not happen
		break;
	case 2:
		form_->controller().set(LyXTabular::SET_LONGTABULAR);
		break;
	}
	form_->changed();
}


void QTabularDialog::ltNewpage_clicked()
{
	form_->controller().set(LyXTabular::SET_LTNEWPAGE);
	form_->changed();
}


void QTabularDialog::ltHeaderStatus_clicked()
{
	bool enable(headerStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTHEAD, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTHEAD, "");
	headerBorderAboveCB->setEnabled(enable);
	headerBorderBelowCB->setEnabled(enable);
	firstheaderNoContentsCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltHeaderBorderAbove_clicked()
{
	if (headerBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTHEAD, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTHEAD, "");
	form_->changed();
}


void QTabularDialog::ltHeaderBorderBelow_clicked()
{
	if (headerBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTHEAD, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTHEAD, "");
	form_->changed();
}


void QTabularDialog::ltFirstHeaderBorderAbove_clicked()
{
	if (firstheaderBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "");
	form_->changed();
}


void QTabularDialog::ltFirstHeaderBorderBelow_clicked()
{
	if (firstheaderBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "");
	form_->changed();
}


void QTabularDialog::ltFirstHeaderStatus_clicked()
{
	bool enable(firstheaderStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "");
	firstheaderBorderAboveCB->setEnabled(enable);
	firstheaderBorderBelowCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltFirstHeaderEmpty_clicked()
{
	bool enable(firstheaderNoContentsCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "empty");
	else {
		ltFirstHeaderBorderBelow_clicked();
		ltFirstHeaderBorderAbove_clicked();
	}
	firstheaderStatusCB->setEnabled(!enable);
	firstheaderBorderAboveCB->setEnabled(!enable);
	firstheaderBorderBelowCB->setEnabled(!enable);
	form_->changed();
}


void QTabularDialog::ltFooterStatus_clicked()
{
	bool enable(footerStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTFOOT, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTFOOT, "");
	footerBorderAboveCB->setEnabled(enable);
	footerBorderBelowCB->setEnabled(enable);
	lastfooterNoContentsCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltFooterBorderAbove_clicked()
{
	if (footerBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFOOT, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTFOOT, "");
	form_->changed();
}


void QTabularDialog::ltFooterBorderBelow_clicked()
{
	if (footerBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFOOT, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTFOOT, "");
	form_->changed();
}


void QTabularDialog::ltLastFooterStatus_clicked()
{
	bool enable(lastfooterStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "");
	lastfooterBorderAboveCB->setEnabled(enable);
	lastfooterBorderBelowCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltLastFooterBorderAbove_clicked()
{
	if (lastfooterBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "");
	form_->changed();
}


void QTabularDialog::ltLastFooterBorderBelow_clicked()
{
	if (lastfooterBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "");
	form_->changed();
}


void QTabularDialog::ltLastFooterEmpty_clicked()
{
	bool enable(lastfooterNoContentsCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "empty");
	else {
		ltLastFooterBorderBelow_clicked();
		ltLastFooterBorderAbove_clicked();
	}
	lastfooterStatusCB->setEnabled(!enable);
	lastfooterBorderAboveCB->setEnabled(!enable);
	lastfooterBorderBelowCB->setEnabled(!enable);
	form_->changed();
}
