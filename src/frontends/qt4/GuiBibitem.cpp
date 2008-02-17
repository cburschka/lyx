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
#include "FuncRequest.h"

#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {


GuiBibitem::GuiBibitem(GuiView & lv)
	: GuiCommand(lv, "bibitem", qt_("Bibliography Entry Settings"))
{
	setupUi(this);

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


Dialog * createGuiBibitem(GuiView & lv) { return new GuiBibitem(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiBibitem_moc.cpp"
