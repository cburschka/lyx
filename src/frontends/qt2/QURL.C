/**
 * \file QURL.C
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

#include "ControlUrl.h"
#include "debug.h"
#include "gettext.h"

#include "QURL.h"
#include "QURLDialog.h"
#include "Qt2BC.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

typedef Qt2CB<ControlUrl, Qt2DB<QURLDialog> > base_class;


QURL::QURL()
	: base_class(_("URL"))
{
}


void QURL::build_dialog()
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
	InsetCommandParams const & params = controller().params();

	dialog_->urlED->setText(params.getContents().c_str());
	dialog_->nameED->setText(params.getOptions().c_str());
	dialog_->hyperlinkCB->setChecked(params.getCmdName() != "url");
}


void QURL::apply()
{
	InsetCommandParams & params = controller().params();

	params.setContents(dialog_->urlED->text().latin1());
	params.setOptions(dialog_->nameED->text().latin1());

	if (dialog_->hyperlinkCB->isChecked())
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");
}


bool QURL::isValid()
{
	string const u(dialog_->urlED->text().latin1());
	string const n(dialog_->nameED->text().latin1());

	return !u.empty() && !n.empty();
}
