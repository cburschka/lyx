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

#include "GuiView.h"
#include "qt_helpers.h"

#include "support/debug.h"

#include <QCloseEvent>

using namespace std;

namespace lyx {
namespace frontend {

GuiDialog::GuiDialog(GuiView & lv, QString const & name, QString const & title)
	: QDialog(&lv), Dialog(lv, name, "LyX: " + title), updating_(false), 
	  is_closing_(false)
{
	// remove question marks from Windows dialogs
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}


void GuiDialog::closeEvent(QCloseEvent * ev)
{
	slotClose();
	ev->accept();
}


void GuiDialog::setButtonsValid(bool valid)
{
	bc().setValid(valid);
}


void GuiDialog::slotApply()
{
	apply();
	bc().apply();
}


void GuiDialog::slotAutoApply()
{
	apply();
	bc().autoApply();
}


void GuiDialog::slotOK()
{
	is_closing_ = true;
	apply();
	is_closing_ = false;
	hideView();
	bc().ok();
}


void GuiDialog::slotClose()
{
	hideView();
	bc().cancel();
}


void GuiDialog::slotRestore()
{
	// Tell the controller that a request to refresh the dialog's contents
	// has been received. It's up to the controller to supply the necessary
	// info by calling GuiDialog::updateView().
	updateDialog();
	bc().restore();
}


void GuiDialog::changed()
{
	if (updating_)
		return;
	bc().setValid(isValid());
}


void GuiDialog::enableView(bool enable)
{
	if (!enable) {
		bc().setReadOnly(true);
		bc().setValid(false);
	}
	Dialog::enableView(enable);
}


void GuiDialog::updateView()
{
	setUpdatesEnabled(false);

	bc().setReadOnly(isBufferReadonly());
	// protect the BC from unwarranted state transitions
	updating_ = true;
	updateContents();
	updating_ = false;
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();

	setUpdatesEnabled(true);
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiDialog.cpp"
