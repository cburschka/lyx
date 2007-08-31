/**
 * \file GuiFloatDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiFloatDialog.h"
#include "GuiFloat.h"

#include <QCloseEvent>
#include <QPushButton>

#include "FloatPlacement.h"



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

} // namespace frontend
} // namespace lyx

#include "GuiFloatDialog_moc.cpp"
