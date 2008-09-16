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
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiHyperlink::GuiHyperlink(GuiView & lv)
	: GuiDialog(lv, "href", qt_("Hyperlink")),
    params_(insetCode("href"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(targetED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(webRB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(emailRB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(fileRB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));

	setFocusProxy(targetED);

	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(targetED);
	bc().addReadOnly(nameED);
	bc().addReadOnly(webRB);
	bc().addReadOnly(emailRB);
	bc().addReadOnly(fileRB);
}


void GuiHyperlink::changed_adaptor()
{
	changed();
}


void GuiHyperlink::paramsToDialog(InsetCommandParams const & /*icp*/)
{
	targetED->setText(toqstr(params_["target"]));
	nameED->setText(toqstr(params_["name"]));
	if (params_["type"] == "")
		webRB->setChecked(true);
	else if (params_["type"] == "mailto:")
		emailRB->setChecked(true);
	else if (params_["type"] == "file:")
		fileRB->setChecked(true);
	bc().setValid(isValid());
}


void GuiHyperlink::applyView()
{
	params_["target"] = qstring_to_ucs4(targetED->text());
	params_["name"] = qstring_to_ucs4(nameED->text());
	if (webRB->isChecked())
		params_["type"] = qstring_to_ucs4("");
	else if (emailRB->isChecked())
		params_["type"] = qstring_to_ucs4("mailto:");
	else if (fileRB->isChecked())
		params_["type"] = qstring_to_ucs4("file:");
	params_.setCmdName("href");
}


bool GuiHyperlink::isValid()
{
	return !targetED->text().isEmpty() || !nameED->text().isEmpty();
}


bool GuiHyperlink::initialiseParams(std::string const & data)
{
	// The name passed with LFUN_INSET_APPLY is also the name
	// used to identify the mailer.
	InsetCommand::string2params("href", data, params_);
	paramsToDialog(params_);
	return true;
}


void GuiHyperlink::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string("href", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


Dialog * createGuiHyperlink(GuiView & lv) { return new GuiHyperlink(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiHyperlink_moc.cpp"
