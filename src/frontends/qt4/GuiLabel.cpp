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

#include "support/debug.h"
#include "qt_helpers.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

using namespace std;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////
//
// Base implementation
//
/////////////////////////////////////////////////////////////////

GuiLabel::GuiLabel(GuiView & lv)
	: GuiCommand(lv, "label", qt_("Label"))
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


Dialog * createGuiLabel(GuiView & lv) { return new GuiLabel(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiLabel_moc.cpp"
