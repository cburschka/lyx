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

#include "qt_helpers.h"

#include "insets/InsetLabel.h"

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

GuiLabel::GuiLabel(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(keywordED, SIGNAL(textChanged(const QString &)),
		this, SIGNAL(changed()));

	setFocusProxy(keywordED);
}


void GuiLabel::paramsToDialog(Inset const * inset)
{
	InsetLabel const * label = static_cast<InsetLabel const *>(inset);
	InsetCommandParams const & params = label->params();
	keywordED->setText(toqstr(params["name"]));
}


docstring GuiLabel::dialogToParams() const
{
	InsetCommandParams params(insetCode());
	params["name"] = qstring_to_ucs4(keywordED->text());
	return from_utf8(InsetLabel::params2string(params));
}


bool GuiLabel::initialiseParams(std::string const & data)
{
	InsetCommandParams p(insetCode());
	if (!InsetCommand::string2params(data, p))
		return false;
	keywordED->setText(toqstr(p["name"]));
	return true;
}


bool GuiLabel::checkWidgets(bool readonly) const
{
	keywordED->setReadOnly(readonly);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !keywordED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiLabel.cpp"
