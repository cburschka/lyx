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

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtextview.h>

#include "QExternalDialog.h"
#include "QExternal.h"
#include "Qt2BC.h"

#include <vector>

typedef Qt2CB<ControlExternal, Qt2DB<QExternalDialog> > base_class;


QExternal::QExternal()
	: base_class(qt_("LyX: Insert External Material"))
{
}


void QExternal::build_dialog()
{
	dialog_.reset(new QExternalDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->externalCO);
	bc().addReadOnly(dialog_->fileED);
	bc().addReadOnly(dialog_->browsePB);

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
	isValid();
}


string const & QExternal::helpText()
{
	InsetExternal::Params & params = controller().params();

	params.templ = controller().getTemplate(dialog_->externalCO->currentItem());
	return params.templ.helpText;
}


void QExternal::apply()
{
	InsetExternal::Params & params = controller().params();

	params.filename = fromqstr(dialog_->fileED->text());
	params.parameters = fromqstr(dialog_->paramsED->text());

	params.templ = controller().getTemplate(dialog_->externalCO->currentItem());
}
