/**
 * \file QExternal.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector> 

#include "LString.h" 
#include "QExternalDialog.h"
#include "ControlExternal.h"
#include "QExternal.h"
#include "Qt2BC.h"
#include "gettext.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h> 
#include <qtextview.h>

typedef Qt2CB<ControlExternal, Qt2DB<QExternalDialog> > base_class;
 
QExternal::QExternal(ControlExternal & c)
	: base_class(c, _("External"))
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
		dialog_->externalCO->insertItem(cit->c_str(), -1);
	}
}

 
void QExternal::update_contents()
{
	InsetExternal::Params const & params = controller().params();

	dialog_->fileED->setText(params.filename.c_str());
	dialog_->paramsED->setText(params.parameters.c_str());
 
	dialog_->externalCO->setCurrentItem(controller().getTemplateNumber(params.templ.lyxName));
	dialog_->externalTV->setText(controller().params().templ.helpText.c_str()); 
	isValid();
}


string const & QExternal::helpText()
{
	controller().params().templ = controller().getTemplate(dialog_->externalCO->currentItem() + 1);
	return controller().params().templ.helpText; 
}

 
void QExternal::apply()
{
	controller().params().filename =
		string(dialog_->fileED->text().latin1());
	controller().params().parameters =
		string(dialog_->paramsED->text().latin1());

	controller().params().templ = controller().getTemplate(dialog_->externalCO->currentItem() + 1);
}


bool QExternal::isValid()
{
	bool const valid = !string(dialog_->fileED->text()).empty();
	dialog_->viewPB->setEnabled(valid); 
	dialog_->editPB->setEnabled(valid); 
	dialog_->updatePB->setEnabled(valid); 
	return valid;
}
