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
	hide();
	bc().ok();
}


void GuiDialog::CancelButton()
{
	hide();
	bc().cancel();
}


void GuiDialog::RestoreButton()
{
	// Tell the kernel that a request to refresh the dialog's contents
	// has been received. It's up to the kernel to supply the necessary
	// info by calling GuiDialog::update().
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

} // namespace frontend
} // namespace lyx
