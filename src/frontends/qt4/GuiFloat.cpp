/**
 * \file GuiFloat.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiFloat.h"

#include "ControlFloat.h"
#include "FloatPlacement.h"

#include "insets/InsetFloat.h"

#include <QCloseEvent>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiFloatDialog::GuiFloatDialog(LyXView & lv)
	: GuiDialog(lv, "float")
{
	setController(new ControlFloat(*this));
	setViewTitle(_("Float Settings"));

	setupUi(this);
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	// enable span columns checkbox
	floatFP->useWide();
	// enable sideways checkbox
	floatFP->useSideways();

	connect(floatFP, SIGNAL(changed()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);

	bc().setCancel(closePB);
	bc().setApply(applyPB);
	bc().setOK(okPB);
	bc().setRestore(restorePB);

	bc().addReadOnly(floatFP);
}


ControlFloat & GuiFloatDialog::controller()
{
	return static_cast<ControlFloat &>(GuiDialog::controller());
}


void GuiFloatDialog::change_adaptor()
{
	changed();
}


void GuiFloatDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiFloatDialog::updateContents()
{
	floatFP->set(controller().params());
}


void GuiFloatDialog::applyView()
{
	InsetFloatParams & params = controller().params();
	params.placement = floatFP->get(params.wide, params.sideways);
}

} // namespace frontend
} // namespace lyx

#include "GuiFloat_moc.cpp"
