/**
 * \file QExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlExternal.h"
#include "qt_helpers.h"
#include "support/lstrings.h"
#include "support/tostr.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtextview.h>

#include "QExternalDialog.h"
#include "QExternal.h"
#include "Qt2BC.h"

#include <vector>

typedef QController<ControlExternal, QView<QExternalDialog> > base_class;


QExternal::QExternal(Dialog & parent)
	: base_class(parent, _("LyX: External Material"))
{
}


void QExternal::build_dialog()
{
	dialog_.reset(new QExternalDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->externalCO);
	bcview().addReadOnly(dialog_->fileED);
	bcview().addReadOnly(dialog_->browsePB);

	std::vector<string> templates(controller().getTemplates());

	for (std::vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit) {
		dialog_->externalCO->insertItem(toqstr(*cit), -1);
	}
}


void QExternal::update_contents()
{
	InsetExternal::Params const & params = controller().params();

	dialog_->fileED->setText(toqstr(params.filename));
	dialog_->paramsED->setText(toqstr(params.parameters));

	dialog_->externalCO->setCurrentItem(controller().getTemplateNumber(params.templ.lyxName));
	dialog_->externalTV->setText(toqstr(params.templ.helpText));

	int item = 0;
	switch (params.display) {
		case grfx::DefaultDisplay: item = 0; break;
		case grfx::MonochromeDisplay: item = 1; break;
		case grfx::GrayscaleDisplay: item = 2; break;
		case grfx::ColorDisplay: item = 3; break;
		case grfx::NoDisplay: item = 0; break;
	}
	dialog_->showCB->setCurrentItem(item);
	dialog_->showCB->setEnabled(params.display != grfx::NoDisplay &&
				    !readOnly());
	dialog_->displayCB->setChecked(params.display != grfx::NoDisplay);
	dialog_->displayscale->setEnabled(params.display != grfx::NoDisplay &&
					  !readOnly());
	dialog_->displayscale->setText(toqstr(tostr(params.lyxscale)));

	isValid();
}


string const QExternal::helpText() const
{
	ExternalTemplate templ =
		controller().getTemplate(dialog_->externalCO->currentItem());
	return templ.helpText;
}


void QExternal::apply()
{
	InsetExternal::Params params = controller().params();

	params.filename = fromqstr(dialog_->fileED->text());
	params.parameters = fromqstr(dialog_->paramsED->text());

	params.templ = controller().getTemplate(dialog_->externalCO->currentItem());

	switch (dialog_->showCB->currentItem()) {
		case 0: params.display = grfx::DefaultDisplay; break;
		case 1: params.display = grfx::MonochromeDisplay; break;
		case 2: params.display = grfx::GrayscaleDisplay; break;
		case 3: params.display = grfx::ColorDisplay; break;
		default:;
	}

	if (!dialog_->displayCB->isChecked())
		params.display = grfx::NoDisplay;

	params.lyxscale = strToInt(fromqstr(dialog_->displayscale->text()));

	controller().setParams(params);
}
