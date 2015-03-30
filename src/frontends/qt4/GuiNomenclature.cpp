/**
 * \file GuiNomencl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiNomenclature.h"

#include "qt_helpers.h"

#include "insets/InsetNomencl.h"

using namespace std;

namespace lyx {
namespace frontend {

GuiNomenclature::GuiNomenclature(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);
	connect(symbolED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(descriptionTE, SIGNAL(textChanged()),
		this, SIGNAL(changed()));

	setFocusProxy(descriptionTE);
}


void GuiNomenclature::paramsToDialog(Inset const * inset)
{
	InsetNomencl const * nomencl = static_cast<InsetNomencl const *>(inset);
	InsetCommandParams const & params = nomencl->params();

	prefixED->setText(toqstr(params["prefix"]));
	symbolED->setText(toqstr(params["symbol"]));
	QString description = toqstr(params["description"]);
	description.replace("\\\\","\n");
	descriptionTE->setPlainText(description);
	descriptionTE->setFocus();
}


docstring GuiNomenclature::dialogToParams() const
{
	InsetCommandParams params(insetCode());
	params["prefix"] = qstring_to_ucs4(prefixED->text());
	params["symbol"] = qstring_to_ucs4(symbolED->text());
	QString description = descriptionTE->toPlainText();
	description.replace('\n',"\\\\");
	params["description"] = qstring_to_ucs4(description);
	return from_utf8(InsetNomencl::params2string(params));
}


bool GuiNomenclature::initialiseParams(std::string const & data)
{
	InsetCommandParams p(insetCode());
	if (!InsetCommand::string2params(data, p))
		return false;
	symbolED->setText(toqstr(p["symbol"]));
	return true;
}


bool GuiNomenclature::checkWidgets(bool readonly) const
{
	symbolED->setReadOnly(readonly);
	descriptionTE->setReadOnly(readonly);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	QString const description = descriptionTE->toPlainText();
	return !symbolED->text().isEmpty() && !description.isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiNomenclature.cpp"
