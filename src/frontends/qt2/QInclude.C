/**
 * \file QInclude.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QIncludeDialog.h"
#include "ControlInclude.h"
#include "QInclude.h"
#include "Qt2BC.h"
#include "gettext.h"
#include "debug.h" 

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

typedef Qt2CB<ControlInclude, Qt2DB<QIncludeDialog> > base_class;

QInclude::QInclude(ControlInclude & c)
	: base_class(c, _("Include"))
{
}


void QInclude::build_dialog()
{
	dialog_.reset(new QIncludeDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->filenameED);
	bc().addReadOnly(dialog_->browsePB);
	bc().addReadOnly(dialog_->typesetCB);
	bc().addReadOnly(dialog_->visiblespaceCB);
	bc().addReadOnly(dialog_->includeRB);
	bc().addReadOnly(dialog_->inputRB);
	bc().addReadOnly(dialog_->verbatimRB);
}


void QInclude::update_contents()
{
	if (controller().params().noload) {
		dialog_->filenameED->setText("");
		dialog_->typesetCB->setChecked(false);
		dialog_->visiblespaceCB->setChecked(false);
		dialog_->visiblespaceCB->setEnabled(false);
		dialog_->includeRB->setChecked(true);
		return;
	}

	dialog_->filenameED->setText(controller().params().cparams.getContents().c_str());

	string const cmdname = controller().params().cparams.getCmdName();

	// FIXME: ??
	dialog_->typesetCB->setChecked(controller().params().noload);

	dialog_->visiblespaceCB->setChecked(false);
	dialog_->visiblespaceCB->setEnabled(false);

	if (cmdname == "input")
		dialog_->inputRB->setChecked(true);
	else if (cmdname == "include")
		dialog_->includeRB->setChecked(true);
	else if (!cmdname.empty()) {
		dialog_->verbatimRB->setChecked(true);
		dialog_->visiblespaceCB->setChecked(cmdname == "verbatiminput*");
		dialog_->visiblespaceCB->setEnabled(true);
	}
	
	if (cmdname.empty())
		dialog_->includeRB->setChecked(true);
}


void QInclude::apply()
{
	controller().params().noload = dialog_->typesetCB->isChecked();

	controller().params().cparams.
		setContents(dialog_->filenameED->text().latin1());

	if (dialog_->inputRB->isChecked())
		controller().params().flag = InsetInclude::INPUT;
	else if (dialog_->includeRB->isChecked())
		controller().params().flag = InsetInclude::INCLUDE;
	else {
		if (dialog_->visiblespaceCB->isChecked())
			controller().params().flag = InsetInclude::VERBAST;
		else
			controller().params().flag = InsetInclude::VERB;
	}
}


void QInclude::browse()
{
	ControlInclude::Type type;

	if (dialog_->includeRB->isChecked())
		type = ControlInclude::INCLUDE;
	else if (dialog_->inputRB->isChecked())
		type = ControlInclude::INPUT;
	else
		type = ControlInclude::VERBATIM;

	string const & name = controller().Browse(dialog_->filenameED->text().latin1(), type);
	if (!name.empty())
		dialog_->filenameED->setText(name.c_str()); 
}


void QInclude::load()
{
	if (isValid()) {
		string const file(dialog_->filenameED->text().latin1());
		slotOK();
		controller().load(file);
	}
}


bool QInclude::isValid()
{
	return !string(dialog_->filenameED->text().latin1()).empty();
}
