/**
 * \file QTexinfoDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LString.h"

#include "ControlTexinfo.h"
#include "QTexinfoDialog.h"
#include "QTexinfo.h"

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <vector>


QTexinfoDialog::QTexinfoDialog(QTexinfo * form)
	: QTexinfoDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QTexinfoDialog::change_adaptor()
{
	form_->changed();
}


void QTexinfoDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QTexinfoDialog::rescanClicked()
{
	// build new *Files.lst
	form_->controller().rescanStyles();
	form_->updateStyles();
	enableViewPB();
}


void QTexinfoDialog::viewClicked()
{
	int const fitem = fileList->currentItem();

	string sel;
	switch (whatStyle->currentItem()) {
	case 0:
		sel = form_->cls_[fitem];
		break;
	case 1:
		sel = form_->sty_[fitem];
		break;
	case 2:
		sel = form_->bst_[fitem];
		break;
	default:
		break;
	}

	// a valid entry?
	if (!sel.empty()) {
		form_->controller().viewFile(sel.c_str());
	}
}


void QTexinfoDialog::update()
{
	switch (whatStyle->currentItem()) {
	case 0:
		form_->updateStyles(ControlTexinfo::cls);
		break;
	case 1:
		form_->updateStyles(ControlTexinfo::sty);
		break;
	case 2:
		form_->updateStyles(ControlTexinfo::bst);
		break;
	default:
		break;
	}

	enableViewPB();
}


void QTexinfoDialog::enableViewPB()
{
	viewPB->setEnabled(fileList->currentItem() > -1);
}
