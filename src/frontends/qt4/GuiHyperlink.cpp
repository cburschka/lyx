/**
 * \file GuiHyperlink.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiHyperlink.h"

#include "qt_helpers.h"
#include "FuncRequest.h"
#include "insets/InsetCommand.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiHyperlink::GuiHyperlink(LyXView & lv)
	: GuiCommand(lv, "href")
{
	setupUi(this);
	setViewTitle( _("Hyperlink"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(urlED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(WebRB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(EmailRB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(FileRB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));

	setFocusProxy(urlED);

	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(urlED);
	bc().addReadOnly(nameED);
	bc().addReadOnly(WebRB);
	bc().addReadOnly(EmailRB);
	bc().addReadOnly(FileRB);
}


void GuiHyperlink::changed_adaptor()
{
	changed();
}


void GuiHyperlink::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiHyperlink::updateContents()
{
	urlED->setText(toqstr(params_["target"]));
	nameED->setText(toqstr(params_["name"]));
	if (params_["type"] == "")
		WebRB->setChecked(true);
	else if (params_["type"] == "mailto:")
		EmailRB->setChecked(true);
	else if (params_["type"] == "file:")
		FileRB->setChecked(true);
	bc().setValid(isValid());
}


void GuiHyperlink::applyView()
{
	params_["target"] = qstring_to_ucs4(urlED->text());
	params_["name"] = qstring_to_ucs4(nameED->text());
	if (WebRB->isChecked())
		params_["type"] = qstring_to_ucs4("");
	else if (EmailRB->isChecked())
		params_["type"] = qstring_to_ucs4("mailto:");
	else if (FileRB->isChecked())
		params_["type"] = qstring_to_ucs4("file:");
	params_.setCmdName("href");
}


bool GuiHyperlink::isValid()
{
	QString const u = urlED->text();
	QString const n = nameED->text();

	return !u.isEmpty() || !n.isEmpty();
}


Dialog * createGuiHyperlink(LyXView & lv) { return new GuiHyperlink(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiHyperlink_moc.cpp"
