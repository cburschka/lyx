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

#include "GuiNomencl.h"

#include "qt_helpers.h"
#include "FuncRequest.h"

#include "insets/InsetCommand.h"

#include "support/debug.h"

#include <QLabel>
#include <QLineEdit>

using namespace std;

namespace lyx {
namespace frontend {

GuiNomenclature::GuiNomenclature(GuiView & lv)
	: GuiDialog(lv, "nomenclature", qt_("Nomenclature")),
	  params_(insetCode("nomenclature"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(symbolED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(descriptionTE, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));

	setFocusProxy(descriptionTE);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(symbolED);
	bc().addReadOnly(descriptionTE);
	bc().addReadOnly(prefixED);
}


void GuiNomenclature::change_adaptor()
{
	changed();
}


void GuiNomenclature::reject()
{
	slotClose();
}


void GuiNomenclature::updateContents()
{
	prefixED->setText(toqstr(params_["prefix"]));
	symbolED->setText(toqstr(params_["symbol"]));
	QString description = toqstr(params_["description"]);
	description.replace("\\\\","\n");
	descriptionTE->setPlainText(description);

	bc().setValid(isValid());
}


void GuiNomenclature::applyView()
{
	params_["prefix"] = qstring_to_ucs4(prefixED->text());
	params_["symbol"] = qstring_to_ucs4(symbolED->text());
	QString description = descriptionTE->toPlainText();
	description.replace('\n',"\\\\");
	params_["description"] = qstring_to_ucs4(description);
}


bool GuiNomenclature::isValid()
{
	QString const description = descriptionTE->toPlainText();
	return !symbolED->text().isEmpty() && !description.isEmpty();
}


bool GuiNomenclature::initialiseParams(std::string const & data)
{
	InsetCommand::string2params("nomenclature", data, params_);
	return true;
}


void GuiNomenclature::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string("nomenclature", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}



Dialog * createGuiNomenclature(GuiView & lv)
{
	return new GuiNomenclature(lv);
}


} // namespace frontend
} // namespace lyx

#include "GuiNomencl_moc.cpp"
