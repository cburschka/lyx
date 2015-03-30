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

#include "qt_helpers.h"

#include "insets/InsetCommand.h"

#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {


GuiBibitem::GuiBibitem(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(keyED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(labelED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
}


void GuiBibitem::paramsToDialog(Inset const * inset)
{
	InsetCommand const * ic = static_cast<InsetCommand const *>(inset);
	InsetCommandParams const & params = ic->params();
	keyED->setText(toqstr(params["key"]));
	labelED->setText(toqstr(params["label"]));
}


docstring GuiBibitem::dialogToParams() const
{
	InsetCommandParams params(insetCode());
	params["key"] = qstring_to_ucs4(keyED->text());
	params["label"] = qstring_to_ucs4(labelED->text());
	return from_utf8(InsetCommand::params2string(params));
}


bool GuiBibitem::checkWidgets(bool readonly) const
{
	keyED->setReadOnly(readonly);
	labelED->setReadOnly(readonly);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !keyED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiBibitem.cpp"
