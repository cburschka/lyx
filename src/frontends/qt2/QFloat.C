/**
 * \file QFloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlFloat.h"
#include "QFloatDialog.h"
#include "QFloat.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "insets/insetfloat.h"
#include "support/lstrings.h"

#include <qpushbutton.h>
#include <qcheckbox.h>

typedef QController<ControlFloat, QView<QFloatDialog> > base_class;


QFloat::QFloat(Dialog & parent)
	: base_class(parent, qt_("LyX: Float Settings"))
{
}


void QFloat::build_dialog()
{
	dialog_.reset(new QFloatDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().setApply(dialog_->applyPB);
	bcview().setOK(dialog_->okPB);
	bcview().setRestore(dialog_->restorePB);

	bcview().addReadOnly(dialog_->topCB);
	bcview().addReadOnly(dialog_->bottomCB);
	bcview().addReadOnly(dialog_->herepossiblyCB);
	bcview().addReadOnly(dialog_->heredefinitelyCB);
	bcview().addReadOnly(dialog_->pageCB);
	bcview().addReadOnly(dialog_->ignoreCB);
	bcview().addReadOnly(dialog_->defaultsCB);
	bcview().addReadOnly(dialog_->spanCB);
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

	InsetFloatParams const & params = controller().params();

	string const & placement = params.placement;

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

	if (params.wide) {
		dialog_->herepossiblyCB->setChecked(false);
		dialog_->bottomCB->setChecked(false);
	}

	dialog_->spanCB->setChecked(params.wide);
}


void QFloat::apply()
{
	InsetFloatParams & params = controller().params();

	params.wide = dialog_->spanCB->isChecked();

	if (dialog_->defaultsCB->isChecked()) {
		params.placement.erase();
		return;
	}

	string placement;

	if (dialog_->heredefinitelyCB->isChecked()) {
		placement += 'H';
	} else {
		if (dialog_->ignoreCB->isChecked()) {
			placement += '!';
		}
		if (dialog_->topCB->isChecked()) {
			placement += 't';
		}
		if (dialog_->bottomCB->isChecked()) {
			placement += 'b';
		}
		if (dialog_->pageCB->isChecked()) {
			placement += 'p';
		}
		if (dialog_->herepossiblyCB->isChecked()) {
			placement += 'h';
		}
	}
	params.placement = placement;
}
