/**
 * \file QFloat.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlFloat.h"
#include "QFloatDialog.h"
#include "QFloat.h"
#include "Qt2BC.h"
#include "gettext.h"

#include "support/lstrings.h"

#include <qpushbutton.h>
#include <qcheckbox.h>

typedef Qt2CB<ControlFloat, Qt2DB<QFloatDialog> > base_class;

QFloat::QFloat()
	: base_class(_("Float Settings"))
{
}


void QFloat::build_dialog()
{
	dialog_.reset(new QFloatDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().setApply(dialog_->applyPB);
	bc().setOK(dialog_->okPB);
	bc().setRestore(dialog_->restorePB);

	bc().addReadOnly(dialog_->topCB);
	bc().addReadOnly(dialog_->bottomCB);
	bc().addReadOnly(dialog_->herepossiblyCB);
	bc().addReadOnly(dialog_->heredefinitelyCB);
	bc().addReadOnly(dialog_->pageCB);
	bc().addReadOnly(dialog_->ignoreCB);
	bc().addReadOnly(dialog_->defaultsCB);
	bc().addReadOnly(dialog_->spanCB);
}


void QFloat::update_contents()
{
	bool def_placement = false;
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool force = false;
	bool here_definitely = false;

	string const placement(controller().params().placement);

	if (placement.empty()) {
		def_placement = true;
	} else if (contains(placement, "H")) {
		here_definitely = true;
	} else {
		if (contains(placement, "!")) {
			force = true;
		}
		if (contains(placement, "t")) {
			top = true;
		}
		if (contains(placement, "b")) {
			bottom = true;
		}
		if (contains(placement, "p")) {
			page = true;
		}
		if (contains(placement, "h")) {
			here = true;
		}
	}
 
	dialog_->defaultsCB->setChecked(def_placement);
	dialog_->topCB->setChecked(top);
	dialog_->bottomCB->setChecked(bottom);
	dialog_->pageCB->setChecked(page);
	dialog_->herepossiblyCB->setChecked(here);
	dialog_->ignoreCB->setChecked(force);
	dialog_->ignoreCB->setEnabled(top || bottom || page || here);
	dialog_->heredefinitelyCB->setChecked(here_definitely);
 
	if (controller().params().wide) {
		dialog_->herepossiblyCB->setChecked(false);
		dialog_->bottomCB->setChecked(false);
	}
 
	dialog_->spanCB->setChecked(controller().params().wide);
}

 
void QFloat::apply()
{
	controller().params().wide = dialog_->spanCB->isChecked(); 

	if (dialog_->defaultsCB->isChecked()) {
		controller().params().placement = "";
		return;
	}
 
	string placement;

	if (dialog_->heredefinitelyCB->isChecked()) {
		placement += "H";
	} else {
		if (dialog_->ignoreCB->isChecked()) {
			placement += "!";
		}
		if (dialog_->topCB->isChecked()) {
			placement += "t";
		}
		if (dialog_->bottomCB->isChecked()) {
			placement += "b";
		}
		if (dialog_->pageCB->isChecked()) {
			placement += "p";
		}
		if (dialog_->herepossiblyCB->isChecked()) {
			placement += "h";
		}
	}
	controller().params().placement = placement;
}
