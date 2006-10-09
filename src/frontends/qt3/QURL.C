/**
 * \file QURL.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QURL.h"
#include "QURLDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlCommand.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlCommand, QView<QURLDialog> > base_class;

QURL::QURL(Dialog & parent)
	: base_class(parent, _("URL"))
{
}


void QURL::build_dialog()
{
	dialog_.reset(new QURLDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->urlED);
	bcview().addReadOnly(dialog_->nameED);
	bcview().addReadOnly(dialog_->hyperlinkCB);
}


void QURL::update_contents()
{
	InsetCommandParams const & params = controller().params();

	dialog_->urlED->setText(toqstr(params.getContents()));
	dialog_->nameED->setText(toqstr(params.getOptions()));
	dialog_->hyperlinkCB->setChecked(params.getCmdName() != "url");

	bc().valid(isValid());
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

	return !u.empty() || !n.empty();
}

} // namespace frontend
} // namespace lyx
