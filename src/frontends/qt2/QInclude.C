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

#include "ControlInclude.h"
#include "gettext.h"
#include "debug.h"
 
#include "QIncludeDialog.h"
#include "QInclude.h"
#include "Qt2BC.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>

typedef Qt2CB<ControlInclude, Qt2DB<QIncludeDialog> > base_class;

QInclude::QInclude()
	: base_class(_("Include"))
{
}


void QInclude::build_dialog()
{
	dialog_.reset(new QIncludeDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->filenameED);
	bc().addReadOnly(dialog_->browsePB);
	bc().addReadOnly(dialog_->visiblespaceCB);
	bc().addReadOnly(dialog_->typeCO);
}


void QInclude::update_contents()
{
	dialog_->filenameED->setText(controller().params().cparams.getContents().c_str());

	string const cmdname = controller().params().cparams.getCmdName();

	dialog_->visiblespaceCB->setChecked(false);
	dialog_->visiblespaceCB->setEnabled(false);

	lyxerr << cmdname << endl;
 
	if (cmdname == "input")
		dialog_->typeCO->setCurrentItem(0);
	else if (!cmdname.empty()) {
		dialog_->typeCO->setCurrentItem(2);
		dialog_->visiblespaceCB->setChecked(cmdname == "verbatiminput*");
		dialog_->visiblespaceCB->setEnabled(true);
	} else {
		dialog_->typeCO->setCurrentItem(1);
	}
}


void QInclude::apply()
{
	controller().params().cparams.
		setContents(dialog_->filenameED->text().latin1());

	int const item = dialog_->typeCO->currentItem();
	if (item == 0)
		controller().params().flag = InsetInclude::INPUT;
	else if (item == 1)
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

	int const item = dialog_->typeCO->currentItem();
	if (item==0)
		type = ControlInclude::INPUT;
	else if (item==1)
		type = ControlInclude::INCLUDE;
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
