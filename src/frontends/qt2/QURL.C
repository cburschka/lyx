/**
 * \file QURL.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "QURLDialog.h"
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
 
#include "ControlUrl.h"
#include "Qt2BC.h"
#include "QURL.h"
#include "debug.h"
#include "gettext.h"

typedef Qt2CB<ControlUrl, Qt2DB<QURLDialog> > base_class;
 
QURL::QURL(ControlUrl & c)
	: base_class(c, _("URL"))
{
}


void QURL::build()
{
	dialog_.reset(new QURLDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->urlED);
	bc().addReadOnly(dialog_->nameED);
	bc().addReadOnly(dialog_->hyperlinkCB);
}

 
void QURL::update_contents()
{
	lyxerr << "update_contents URL" << std::endl;
	lyxerr << dialog_->okPB << std::endl;
	dialog_->urlED->setText(controller().params().getContents().c_str());
	dialog_->nameED->setText(controller().params().getOptions().c_str());
	dialog_->hyperlinkCB->setChecked(controller().params().getCmdName() != "url");
}

 
void QURL::apply()
{
	controller().params().setContents(dialog_->urlED->text().latin1());
	controller().params().setOptions(dialog_->nameED->text().latin1());

	if (dialog_->hyperlinkCB->isChecked())
		controller().params().setCmdName("htmlurl");
	else
		controller().params().setCmdName("url");
}


bool QURL::isValid()
{
	string const u(dialog_->urlED->text().latin1());
	string const n(dialog_->nameED->text().latin1());

	return !u.empty() && !n.empty(); 
} 
