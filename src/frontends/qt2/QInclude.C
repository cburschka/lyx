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


#include "ControlInclude.h"
#include "qt_helpers.h"
#include "debug.h"

#include "QIncludeDialog.h"
#include "QInclude.h"
#include "Qt2BC.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>


typedef QController<ControlInclude, QView<QIncludeDialog> > base_class;


QInclude::QInclude(Dialog & parent)
	: base_class(parent, _("LyX: Child Document"))
{}


void QInclude::build_dialog()
{
	dialog_.reset(new QIncludeDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->filenameED);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->visiblespaceCB);
	bcview().addReadOnly(dialog_->typeCO);
}


void QInclude::update_contents()
{
	InsetInclude::Params const & params = controller().params();

	dialog_->filenameED->setText(toqstr(params.cparams.getContents()));

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
	InsetInclude::Params params = controller().params();

	params.cparams.setContents(fromqstr(dialog_->filenameED->text()));
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
	controller().setParams(params);
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

	string const & name = controller().Browse(fromqstr(dialog_->filenameED->text()), type);
	if (!name.empty())
		dialog_->filenameED->setText(toqstr(name));
}


void QInclude::load()
{
	if (isValid()) {
		string const file(fromqstr(dialog_->filenameED->text()));
		slotOK();
		controller().load(file);
	}
}


bool QInclude::isValid()
{
	return !dialog_->filenameED->text().isEmpty();
}
