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
#include "FloatPlacement.h"

#include "insets/InsetFloat.h"

#include <QCloseEvent>
#include <QPushButton>

namespace lyx {
namespace frontend {

GuiFloatDialog::GuiFloatDialog(GuiFloat * form)
	: form_(form)
{
	setupUi(this);
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	// enable span columns checkbox
	floatFP->useWide();

	// enable sideways checkbox
	floatFP->useSideways();

	connect(floatFP, SIGNAL(changed()),
		this, SLOT(change_adaptor()));
}


void GuiFloatDialog::change_adaptor()
{
	form_->changed();
}


void GuiFloatDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


GuiFloat::GuiFloat(GuiDialog & parent)
	:  GuiView<GuiFloatDialog>(parent, _("Float Settings"))
{
}


void GuiFloat::build_dialog()
{
	dialog_.reset(new GuiFloatDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().setApply(dialog_->applyPB);
	bc().setOK(dialog_->okPB);
	bc().setRestore(dialog_->restorePB);

	bc().addReadOnly(dialog_->floatFP);
}


void GuiFloat::update_contents()
{
	dialog_->floatFP->set(controller().params());
}


void GuiFloat::applyView()
{
	InsetFloatParams & params = controller().params();

	params.placement = dialog_->floatFP->get(params.wide, params.sideways);
}

} // namespace frontend
} // namespace lyx

#include "GuiFloat_moc.cpp"
