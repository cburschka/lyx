/**
 * \file GuiURLDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiURLDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "ButtonController.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiURLDialog::GuiURLDialog(UrlView * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));
	connect(urlED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(hyperlinkCB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));

	setFocusProxy(urlED);
}


void GuiURLDialog::changed_adaptor()
{
	form_->changed();
}


void GuiURLDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}



UrlView::UrlView(Dialog & parent)
	: GuiView<GuiURLDialog>(parent, _("URL"))
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
	QString const u = dialog_->urlED->text();
	QString const n = dialog_->nameED->text();

	return !u.isEmpty() || !n.isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiURLDialog_moc.cpp"
