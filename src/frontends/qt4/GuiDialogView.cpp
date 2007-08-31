/**
 * \file GuiDialogView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiDialogView.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"


namespace lyx {
namespace frontend {

GuiDialogView::GuiDialogView(Dialog & parent, docstring const & t)
	: Dialog::View(parent,t), updating_(false)
{}


Qt2BC & GuiDialogView::bcview()
{
	return static_cast<Qt2BC &>(dialog().bc().view());
}


bool GuiDialogView::isVisible() const
{
	return form() && form()->isVisible();
}


bool GuiDialogView::readOnly() const
{
	return kernel().isBufferReadonly();
}


void GuiDialogView::show()
{
	if (!form()) {
		build();
	}

	QSize const sizeHint = form()->sizeHint();
	if (sizeHint.height() >= 0 && sizeHint.width() >= 0)
		form()->setMinimumSize(sizeHint);

	update();  // make sure its up-to-date
	if (dialog().controller().exitEarly())
		return;

	form()->setWindowTitle(toqstr("LyX: " + getTitle()));

	if (form()->isVisible()) {
		form()->raise();
		form()->activateWindow();
		form()->setFocus();
	} else {
		form()->show();
		form()->setFocus();
	}
}


void GuiDialogView::hide()
{
	if (form() && form()->isVisible())
		form()->hide();
}


bool GuiDialogView::isValid()
{
	return true;
}


void GuiDialogView::changed()
{
	if (updating_)
		return;
	bc().valid(isValid());
}


void GuiDialogView::slotWMHide()
{
	dialog().CancelButton();
}


void GuiDialogView::slotApply()
{
	dialog().ApplyButton();
}


void GuiDialogView::slotOK()
{
	dialog().OKButton();
}


void GuiDialogView::slotClose()
{
	dialog().CancelButton();
}


void GuiDialogView::slotRestore()
{
	dialog().RestoreButton();
}

} // namespace frontend
} // namespace lyx

#include "GuiDialogView_moc.cpp"
