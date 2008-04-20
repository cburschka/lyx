/**
 * \file GuiLabel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiLabel.h"

#include "FuncRequest.h"
#include "qt_helpers.h"

#include "support/debug.h"
#include "insets/InsetCommand.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

using namespace std;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////
//
// GuiLabel
//
/////////////////////////////////////////////////////////////////

GuiLabel::GuiLabel(GuiView & lv)
	: GuiDialog(lv, "label", qt_("Label")),
	  params_(insetCode("label"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(keywordED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

	setFocusProxy(keywordED);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(keywordED);
}


void GuiLabel::change_adaptor()
{
	changed();
}


void GuiLabel::reject()
{
	slotClose();
}


void GuiLabel::updateContents()
{
	docstring const contents = params_["name"];
	keywordED->setText(toqstr(contents));
	bc().setValid(!contents.empty());
}


void GuiLabel::applyView()
{
	params_["name"] = qstring_to_ucs4(keywordED->text());
}


bool GuiLabel::isValid()
{
	return !keywordED->text().isEmpty();
}


bool GuiLabel::initialiseParams(std::string const & data)
{
	InsetCommand::string2params("label", data, params_);
	return true;
}


void GuiLabel::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string("label", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


Dialog * createGuiLabel(GuiView & lv) { return new GuiLabel(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiLabel_moc.cpp"
