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
#include "frontends/LyXView.h"

#include <QCloseEvent>
#include <QSettings>
#include <QShowEvent>

using std::string;

namespace lyx {
namespace frontend {

GuiDialog::GuiDialog(LyXView & lv, std::string const & name)
	: is_closing_(false), name_(name), controller_(0)
{
	lyxview_ = &lv;
}


GuiDialog::~GuiDialog()
{
	delete controller_;
}


void GuiDialog::setViewTitle(docstring const & title)
{
	setWindowTitle("LyX: " + toqstr(title));
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


void GuiDialog::slotOK()
{
	is_closing_ = true;
	apply();
	is_closing_ = false;
	QDialog::hide();
	bc().ok();
}


void GuiDialog::slotClose()
{
	QDialog::hide();
	bc().cancel();
}


void GuiDialog::slotRestore()
{
	// Tell the controller that a request to refresh the dialog's contents
	// has been received. It's up to the controller to supply the necessary
	// info by calling GuiDialog::updateView().
	controller().updateDialog(name_);
	bc().restore();
}

void GuiDialog::checkStatus()
{
	// buffer independant dialogs are always active.
	// This check allows us leave canApply unimplemented for some dialogs.
	if (!controller().isBufferDependent())
		return;

	// deactivate the dialog if we have no buffer
	if (!controller().isBufferAvailable()) {
		bc().setReadOnly(true);
		return;
	}

	// check whether this dialog may be active
	if (controller().canApply()) {
		bool const readonly = controller().isBufferReadonly();
		bc().setReadOnly(readonly);
		// refreshReadOnly() is too generous in _enabling_ widgets
		// update dialog to disable disabled widgets again

		if (!readonly || controller().canApplyToReadOnly())
			updateView();

	} else {
		bc().setReadOnly(true);
	}	
}


bool GuiDialog::isVisibleView() const
{
	return QDialog::isVisible();
}


void GuiDialog::showView()
{
	QSize const hint = sizeHint();
	if (hint.height() >= 0 && hint.width() >= 0)
		setMinimumSize(hint);

	updateView();  // make sure its up-to-date
	if (controller().exitEarly())
		return;

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


void GuiDialog::changed()
{
	if (updating_)
		return;
	bc().setValid(isValid());
}


void GuiDialog::updateView()
{
	setUpdatesEnabled(false);

	// protect the BC from unwarranted state transitions
	updating_ = true;
	updateContents();
	updating_ = false;

	setUpdatesEnabled(true);
	QDialog::update();
}


void GuiDialog::showData(string const & data)
{
	if (controller().isBufferDependent() && !controller().isBufferAvailable())
		return;

	if (!controller().initialiseParams(data)) {
		lyxerr << "Dialog \"" << name_
		       << "\" failed to translate the data "
			"string passed to show()" << std::endl;
		return;
	}

	bc().setReadOnly(controller().isBufferReadonly());
	showView();
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void GuiDialog::updateData(string const & data)
{
	if (controller().isBufferDependent() && !controller().isBufferAvailable())
		return;

	if (!controller().initialiseParams(data)) {
		lyxerr << "Dialog \"" << name_
		       << "\" could not be initialized" << std::endl;
		return;
	}

	bc().setReadOnly(controller().isBufferReadonly());
	updateView();
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void GuiDialog::hide()
{
	if (!isVisibleView())
		return;

	controller().clearParams();
	hideView();
	controller().disconnect(name_);
}


void GuiDialog::apply()
{
	if (controller().isBufferDependent()) {
		if (!controller().isBufferAvailable() ||
		    (controller().isBufferReadonly() && !controller().canApplyToReadOnly()))
			return;
	}

	applyView();
	controller().dispatchParams();

	if (controller().disconnectOnApply() && !is_closing_) {
		controller().disconnect(name_);
		controller().initialiseParams(string());
		updateView();
	}
}


void GuiDialog::setController(Controller * controller)
{
	BOOST_ASSERT(controller);
	BOOST_ASSERT(!controller_);
	controller_ = controller;
	controller_->setLyXView(*lyxview_);
}


void GuiDialog::showEvent(QShowEvent * e)
{
	QSettings settings;
	string key = name_ + "/geometry";
	restoreGeometry(settings.value(key.c_str()).toByteArray());
	QDialog::showEvent(e);
}


void GuiDialog::closeEvent(QCloseEvent * e)
{
	QSettings settings;
	string key = name_ + "/geometry";
	settings.setValue(key.c_str(), saveGeometry());
	QDialog::closeEvent(e);
}

} // namespace frontend
} // namespace lyx

#include "GuiDialog_moc.cpp"
