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
#include "qt_helpers.h"

#include "QURL.h"
#include "QURLDialog.h"
#include "Qt2BC.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

typedef Qt2CB<ControlUrl, Qt2DB<QURLDialog> > base_class;


QURL::QURL()
	: base_class(qt_("URL"))
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

	dialog_->urlED->setText(toqstr(params.getContents()));
	dialog_->nameED->setText(toqstr(params.getOptions()));
	dialog_->hyperlinkCB->setChecked(params.getCmdName() != "url");
}


void QURL::apply()
{
	InsetCommandParams & params = controller().params();

	params.setContents(fromqstr(dialog_->urlED->text()));
	params.setOptions(fromqstr(dialog_->nameED->text()));

	if (dialog_->hyperlinkCB->isChecked())
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");
}


bool QURL::isValid()
{
	string const u(fromqstr(dialog_->urlED->text()));
	string const n(fromqstr(dialog_->nameED->text()));

	return !u.empty() && !n.empty();
}
