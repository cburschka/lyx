/**
 * \file QExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlExternal.h"
#include "qt_helpers.h"

#include "insets/ExternalTemplate.h"
#include "insets/insetexternal.h"

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

using lyx::support::strToInt;

using std::string;


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
	InsetExternalParams const & params = controller().params();

	string const name =
		params.filename.outputFilename(kernel().bufferFilepath());
	dialog_->fileED->setText(toqstr(name));

	dialog_->externalCO->setCurrentItem(controller()
					    .getTemplateNumber(params.templatename()));
	dialog_->externalTV->setText(toqstr(helpText()));

	int item = 0;
	switch (params.display) {
		case lyx::graphics::DefaultDisplay: item = 0; break;
		case lyx::graphics::MonochromeDisplay: item = 1; break;
		case lyx::graphics::GrayscaleDisplay: item = 2; break;
		case lyx::graphics::ColorDisplay: item = 3; break;
		case lyx::graphics::NoDisplay: item = 0; break;
	}
	dialog_->showCB->setCurrentItem(item);
	dialog_->showCB->setEnabled(params.display != lyx::graphics::NoDisplay &&
				    !readOnly());
	dialog_->displayCB->setChecked(params.display != lyx::graphics::NoDisplay);
	dialog_->displayscale->setEnabled(params.display != lyx::graphics::NoDisplay &&
					  !readOnly());
	dialog_->displayscale->setText(toqstr(tostr(params.lyxscale)));

	isValid();
}


string const QExternal::helpText() const
{
	lyx::external::Template templ =
		controller().getTemplate(dialog_->externalCO->currentItem());
	return templ.helpText;
}


void QExternal::apply()
{
	InsetExternalParams params = controller().params();

	params.filename.set(fromqstr(dialog_->fileED->text()),
			    kernel().bufferFilepath());

	params.settemplate(
		controller().getTemplate(dialog_->externalCO->currentItem()).lyxName);

	switch (dialog_->showCB->currentItem()) {
		case 0: params.display = lyx::graphics::DefaultDisplay; break;
		case 1: params.display = lyx::graphics::MonochromeDisplay; break;
		case 2: params.display = lyx::graphics::GrayscaleDisplay; break;
		case 3: params.display = lyx::graphics::ColorDisplay; break;
		default:;
	}

	if (!dialog_->displayCB->isChecked())
		params.display = lyx::graphics::NoDisplay;

	params.lyxscale = strToInt(fromqstr(dialog_->displayscale->text()));

	controller().setParams(params);
}
