/**
 * \file UrlView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "UrlView.h"
#include "GuiURLDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlCommand.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

using std::string;

namespace lyx {
namespace frontend {

typedef QController< ControlCommand, GuiView<GuiURLDialog> > urlview_base_class;

UrlView::UrlView(Dialog & parent)
	: urlview_base_class(parent, _("URL"))
{
}


void UrlView::build_dialog()
{
	dialog_.reset(new GuiURLDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->urlED);
	bcview().addReadOnly(dialog_->nameED);
	bcview().addReadOnly(dialog_->hyperlinkCB);
}


void UrlView::update_contents()
{
	InsetCommandParams const & params = controller().params();

	dialog_->urlED->setText(toqstr(params["target"]));
	dialog_->nameED->setText(toqstr(params["name"]));
	dialog_->hyperlinkCB->setChecked(params.getCmdName() != "url");

	bc().valid(isValid());
}


void UrlView::apply()
{
	InsetCommandParams & params = controller().params();

	params["target"] = qstring_to_ucs4(dialog_->urlED->text());
	params["name"] = qstring_to_ucs4(dialog_->nameED->text());

	if (dialog_->hyperlinkCB->isChecked())
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");
}


bool UrlView::isValid()
{
	string const u(fromqstr(dialog_->urlED->text()));
	string const n(fromqstr(dialog_->nameED->text()));

	return !u.empty() || !n.empty();
}

} // namespace frontend
} // namespace lyx
