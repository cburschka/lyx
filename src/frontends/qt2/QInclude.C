/**
 * \file QInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
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
	InsetInclude::Params const & params = controller().params();

	dialog_->filenameED->setText(params.cparams.getContents().c_str());

	dialog_->visiblespaceCB->setChecked(false);
	dialog_->visiblespaceCB->setEnabled(false);
	dialog_->previewCB->setChecked(false);
	dialog_->previewCB->setEnabled(false);

	switch (params.flag) {
		case InsetInclude::INPUT:
			dialog_->typeCO->setCurrentItem(0);
			dialog_->previewCB->setEnabled(true);
			dialog_->previewCB->setChecked(params.cparams.preview());
			break;

		case InsetInclude::INCLUDE:
			dialog_->typeCO->setCurrentItem(1);
			break;

		case InsetInclude::VERBAST:
			dialog_->visiblespaceCB->setChecked(true);
			/* fall through */
		case InsetInclude::VERB:
			dialog_->typeCO->setCurrentItem(2);
			dialog_->visiblespaceCB->setEnabled(true);
			break;
	}
}


void QInclude::apply()
{
	InsetInclude::Params & params = controller().params();

	params.cparams.setContents(dialog_->filenameED->text().latin1());
	params.cparams.preview(dialog_->previewCB->isChecked());

	int const item = dialog_->typeCO->currentItem();
	if (item == 0)
		params.flag = InsetInclude::INPUT;
	else if (item == 1)
		params.flag = InsetInclude::INCLUDE;
	else {
		if (dialog_->visiblespaceCB->isChecked())
			params.flag = InsetInclude::VERBAST;
		else
			params.flag = InsetInclude::VERB;
	}
}


void QInclude::browse()
{
	ControlInclude::Type type;

	int const item = dialog_->typeCO->currentItem();
	if (item == 0)
		type = ControlInclude::INPUT;
	else if (item == 1)
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
