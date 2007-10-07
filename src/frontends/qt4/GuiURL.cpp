/**
 * \file GuiURL.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiURL.h"

#include "GuiURL.h"
#include "qt_helpers.h"
#include "FuncRequest.h"
#include "insets/InsetCommand.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiURL::GuiURL(LyXView & lv)
	: GuiDialog(lv, "url"), Controller(this), params_("url")
{
	setupUi(this);
	setViewTitle( _("URL"));
	setController(this, false);

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


void GuiURL::changed_adaptor()
{
	changed();
}


void GuiURL::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}



void GuiURL::updateContents()
{
	urlED->setText(toqstr(params_["target"]));
	nameED->setText(toqstr(params_["name"]));
	hyperlinkCB->setChecked(params_.getCmdName() != "url");

	bc().setValid(isValid());
}


void GuiURL::applyView()
{
	params_["target"] = qstring_to_ucs4(urlED->text());
	params_["name"] = qstring_to_ucs4(nameED->text());

	if (hyperlinkCB->isChecked())
		params_.setCmdName("htmlurl");
	else
		params_.setCmdName("url");
}


bool GuiURL::isValid()
{
	QString const u = urlED->text();
	QString const n = nameED->text();

	return !u.isEmpty() || !n.isEmpty();
}


bool GuiURL::initialiseParams(std::string const & data)
{
	// The name passed with LFUN_INSET_APPLY is also the name
	// used to identify the mailer.
	InsetCommandMailer::string2params("name", data, params_);
	return true;
}


void GuiURL::dispatchParams()
{
	std::string const lfun = 
		InsetCommandMailer::params2string("url", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


Dialog * createGuiURL(LyXView & lv) { return new GuiURL(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiURL_moc.cpp"
