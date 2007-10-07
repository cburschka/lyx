/**
 * \file GuiBibitem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBibitem.h"
#include "ControlCommand.h"
#include "qt_helpers.h"
#include "FuncRequest.h"

#include "insets/InsetCommand.h"

#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {


GuiBibitem::GuiBibitem(LyXView & lv)
	: GuiDialog(lv, "bibitem"), Controller(this), params_("bibitem")
{
	setupUi(this);
	setViewTitle(_("Bibliography Entry Settings"));
	setController(this, false);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(keyED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(labelED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::OkCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(keyED);
	bc().addReadOnly(labelED);
}


void GuiBibitem::change_adaptor()
{
	changed();
}


void GuiBibitem::closeEvent(QCloseEvent *e)
{
	slotClose();
	e->accept();
}


void GuiBibitem::updateContents()
{
	keyED->setText(toqstr(params_["key"]));
	labelED->setText(toqstr(params_["label"]));
}


void GuiBibitem::applyView()
{
	params_["key"] = qstring_to_ucs4(keyED->text());
	params_["label"] = qstring_to_ucs4(labelED->text());
}


bool GuiBibitem::isValid()
{
	return !keyED->text().isEmpty();
}


bool GuiBibitem::initialiseParams(std::string const & data)
{
	// The name passed with LFUN_INSET_APPLY is also the name
	// used to identify the mailer.
	InsetCommandMailer::string2params("bibitem", data, params_);
	return true;
}


void GuiBibitem::dispatchParams()
{
	std::string const lfun = InsetCommandMailer::params2string("bibitem", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


Dialog * createGuiBibitem(LyXView & lv) { return new GuiBibitem(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiBibitem_moc.cpp"
