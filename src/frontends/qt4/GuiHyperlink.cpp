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

	setFocusProxy(urlED);

	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(urlED);
	bc().addReadOnly(nameED);
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
	bc().setValid(isValid());
}


void GuiHyperlink::applyView()
{
	params_["target"] = qstring_to_ucs4(urlED->text());
	params_["name"] = qstring_to_ucs4(nameED->text());
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
