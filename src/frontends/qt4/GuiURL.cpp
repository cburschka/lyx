/**
 * \file GuiURL.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiURL.h"

#include "ControlCommand.h"
#include "qt_helpers.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiURLDialog::GuiURLDialog(LyXView & lv)
	: GuiDialog(lv, "url")
{
	setupUi(this);
	setViewTitle( _("URL"));
	setController(new ControlCommand(*this, "url", "url"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(urlED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(hyperlinkCB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));

	setFocusProxy(urlED);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(urlED);
	bc().addReadOnly(nameED);
	bc().addReadOnly(hyperlinkCB);
}


ControlCommand & GuiURLDialog::controller() const
{
	return static_cast<ControlCommand &>(GuiDialog::controller());
}


void GuiURLDialog::changed_adaptor()
{
	changed();
}


void GuiURLDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}



void GuiURLDialog::update_contents()
{
	InsetCommandParams const & params = controller().params();

	urlED->setText(toqstr(params["target"]));
	nameED->setText(toqstr(params["name"]));
	hyperlinkCB->setChecked(params.getCmdName() != "url");

	bc().setValid(isValid());
}


void GuiURLDialog::applyView()
{
	InsetCommandParams & params = controller().params();

	params["target"] = qstring_to_ucs4(urlED->text());
	params["name"] = qstring_to_ucs4(nameED->text());

	if (hyperlinkCB->isChecked())
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");
}


bool GuiURLDialog::isValid()
{
	QString const u = urlED->text();
	QString const n = nameED->text();

	return !u.isEmpty() || !n.isEmpty();
}

} // namespace frontend
} // namespace lyx


#include "GuiURL_moc.cpp"
