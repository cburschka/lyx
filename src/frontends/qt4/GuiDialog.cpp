/**
 * \file Dialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiDialog.h"
#include "debug.h"
#include "qt_helpers.h"

namespace lyx {
namespace frontend {

GuiDialog::GuiDialog(LyXView & lv, std::string const & name)
	: Dialog(lv, name)
{}


void GuiDialog::setButtonsValid(bool valid)
{
	bc().setValid(valid);
}


void GuiDialog::ApplyButton()
{
	apply();
	bc().apply();
}


void GuiDialog::OKButton()
{
	is_closing_ = true;
	apply();
	is_closing_ = false;
	QDialog::hide();
	bc().ok();
}


void GuiDialog::CancelButton()
{
	QDialog::hide();
	bc().cancel();
}


void GuiDialog::RestoreButton()
{
	// Tell the kernel that a request to refresh the dialog's contents
	// has been received. It's up to the kernel to supply the necessary
	// info by calling GuiDialog::updateView().
	kernel().updateDialog(name_);
	bc().restore();
}


void GuiDialog::preShow()
{
	bc().setReadOnly(kernel().isBufferReadonly());
}


void GuiDialog::postShow()
{
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void GuiDialog::preUpdate()
{
	bc().setReadOnly(kernel().isBufferReadonly());
}


void GuiDialog::postUpdate()
{
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void GuiDialog::checkStatus()
{
	// buffer independant dialogs are always active.
	// This check allows us leave canApply unimplemented for some dialogs.
	if (!controller().isBufferDependent())
		return;

	// deactivate the dialog if we have no buffer
	if (!kernel().isBufferAvailable()) {
		bc().setReadOnly(true);
		return;
	}

	// check whether this dialog may be active
	if (controller().canApply()) {
		bool const readonly = kernel().isBufferReadonly();
		bc().setReadOnly(readonly);
		// refreshReadOnly() is too generous in _enabling_ widgets
		// update dialog to disable disabled widgets again
/*
 * 	FIXME:
		if (!readonly || controller().canApplyToReadOnly())
			update();
*/
	} else {
		bc().setReadOnly(true);
	}	
}


bool GuiDialog::isVisibleView() const
{
	return QDialog::isVisible();
}


bool GuiDialog::readOnly() const
{
	return kernel().isBufferReadonly();
}


void GuiDialog::showView()
{
	QSize const hint = sizeHint();
	if (hint.height() >= 0 && hint.width() >= 0)
		setMinimumSize(hint);

	updateView();  // make sure its up-to-date
	if (controller().exitEarly())
		return;

	setWindowTitle(toqstr("LyX: " + getViewTitle()));

	if (QWidget::isVisible()) {
		raise();
		activateWindow();
	} else {
		QWidget::show();
	}
	setFocus();
}


void GuiDialog::hideView()
{
	QDialog::hide();
}


bool GuiDialog::isValid()
{
	return true;
}


void GuiDialog::changed()
{
	if (updating_)
		return;
	bc().setValid(isValid());
}


void GuiDialog::slotWMHide()
{
	CancelButton();
}


void GuiDialog::slotApply()
{
	ApplyButton();
}


void GuiDialog::slotOK()
{
	OKButton();
}


void GuiDialog::slotClose()
{
	CancelButton();
}


void GuiDialog::slotRestore()
{
	RestoreButton();
}

void GuiDialog::updateView()
{
	setUpdatesEnabled(false);

	// protect the BC from unwarranted state transitions
	updating_ = true;
	update_contents();
	updating_ = false;

	setUpdatesEnabled(true);
	QDialog::update();
}

} // namespace frontend
} // namespace lyx

#include "GuiDialog_moc.cpp"
