/**
 * \file FormUrl.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "ControlUrl.h"
#include "FormUrl.h"
#include "gettext.h"
#include "urldlg.h"

FormUrl::FormUrl(ControlUrl & c)
	: KFormBase<ControlUrl, UrlDialog>(c)
{
}


void FormUrl::update()
{
	dialog_->url->setText(controller().params().getContents().c_str());
	dialog_->urlname->setText(controller().params().getOptions().c_str());

	dialog_->htmlurl->setChecked(controller().params().getCmdName() != "url");
}

 
void FormUrl::apply()
{
	controller().params().setContents(dialog_->url->text());
	controller().params().setOptions(dialog_->urlname->text());

	if (dialog_->htmlurl->isChecked())
		controller().params().setCmdName("htmlurl");
	else
		controller().params().setCmdName("url");
}

 
void FormUrl::build()
{
	dialog_.reset(new UrlDialog(this, 0, _("LyX: Url"), false));
 
	bc().setOK(dialog_->buttonOk);
	bc().setCancel(dialog_->buttonCancel);

	bc().addReadOnly(dialog_->urlname);
	bc().addReadOnly(dialog_->url);
	bc().addReadOnly(dialog_->htmlurl); 
 
}
