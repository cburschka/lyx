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
#include "helper_funcs.h"

#include "support/lstrings.h"

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

typedef Qt2CB<ControlFloat, Qt2DB<QFloatDialog> > base_class;
 
QFloat::QFloat(ControlFloat & c)
	: base_class(c, _("LaTeX Information"))
{
}


void QFloat::build_dialog()
{
	dialog_.reset(new QFloatDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().setApply(dialog_->applyPB);
	bc().setOK(dialog_->okPB);
	bc().setRestore(dialog_->restorePB);
}

 
void QFloat::update_contents()
{
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool forcehere = false;

	string placement(controller().params().placement);

	if (contains(placement, "H")) {
		forcehere = true;
	} else {
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

	dialog_->top->setChecked(top);
	dialog_->bottom->setChecked(bottom);
	dialog_->page->setChecked(page);
	dialog_->here->setChecked(here);
	dialog_->forcehere->setChecked(forcehere);
}

void QFloat::apply()
{
	string placement;
	
	if (dialog_->forcehere->isChecked()) {
		placement += "H";
	} else {
		if (dialog_->top->isChecked()) {
			placement += "t";
		}
		if (dialog_->bottom->isChecked()) {
			placement += "b";
		}
		if (dialog_->page->isChecked()) {
			placement += "p";
		}
		if (dialog_->here->isChecked()) {
			placement += "h";
		}
	}
	controller().params().placement = placement;
}
