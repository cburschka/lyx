/**
 * \file QInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlInclude.h"
#include "qt_helpers.h"

#include "QIncludeDialog.h"
#include "QInclude.h"
#include "Qt2BC.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>

using std::string;

namespace lyx {
namespace frontend {

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
	InsetCommandParams const & params = controller().params();

	dialog_->filenameED->setText(toqstr(params.getContents()));

	dialog_->visiblespaceCB->setChecked(false);
	dialog_->visiblespaceCB->setEnabled(false);
	dialog_->previewCB->setChecked(false);
	dialog_->previewCB->setEnabled(false);

	string cmdname = controller().params().getCmdName();
	if (cmdname != "include" &&
	    cmdname != "verbatiminput" &&
	    cmdname != "verbatiminput*")
		cmdname = "input";

	if (cmdname == "input") {
		dialog_->typeCO->setCurrentItem(0);
		dialog_->previewCB->setEnabled(true);
		dialog_->previewCB->setChecked(params.preview());

	} else if (cmdname == "include") {
		dialog_->typeCO->setCurrentItem(1);

	} else if (cmdname == "verbatiminput*") {
		dialog_->typeCO->setCurrentItem(2);
		dialog_->visiblespaceCB->setEnabled(true);
		dialog_->visiblespaceCB->setChecked(true);

	} else if (cmdname == "verbatiminput") {
		dialog_->typeCO->setCurrentItem(2);
		dialog_->visiblespaceCB->setEnabled(true);
	}
}


void QInclude::apply()
{
	InsetCommandParams params = controller().params();

	params.setContents(fromqstr(dialog_->filenameED->text()));
	params.preview(dialog_->previewCB->isChecked());

	int const item = dialog_->typeCO->currentItem();
	if (item == 0)
		params.setCmdName("input");
	else if (item == 1)
		params.setCmdName("include");
	else {
		if (dialog_->visiblespaceCB->isChecked())
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");
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

	string const & name = controller().browse(fromqstr(dialog_->filenameED->text()), type);
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

} // namespace frontend
} // namespace lyx
