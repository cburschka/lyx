/**
 * \file QTexinfoDialog.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>
#include <vector>

#include "LString.h"

#include "ControlTexinfo.h"
#include "QTexinfoDialog.h"
#include "Dialogs.h"
#include "QTexinfo.h"

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>

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
}

void QTexinfoDialog::viewClicked()
{
	string const sel(fileList->currentText());
	// a valid entry?
	if (!sel.empty()) {
		form_->controller().viewFile(sel.c_str());
	}
}

void QTexinfoDialog::update()
{
	int item = whatStyle->currentItem();

	switch (item) {
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

}
