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
#include "debug.h"
#include "qt_helpers.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QSettings>
#include <QShowEvent>

using std::string;

namespace lyx {
namespace frontend {

GuiDialog::GuiDialog(GuiView & lv, std::string const & name)
	: QDialog(&lv), Dialog(lv, name), is_closing_(false)
{}


GuiDialog::~GuiDialog()
{
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
	updateDialog();
	bc().restore();
}

void GuiDialog::checkStatus()
{
	// buffer independant dialogs are always active.
	// This check allows us leave canApply unimplemented for some dialogs.
	if (!isBufferDependent())
		return;

	// deactivate the dialog if we have no buffer
	if (!isBufferAvailable()) {
		bc().setReadOnly(true);
		return;
	}

	// check whether this dialog may be active
	if (canApply()) {
		bool const readonly = isBufferReadonly();
		bc().setReadOnly(readonly);
		// refreshReadOnly() is too generous in _enabling_ widgets
		// update dialog to disable disabled widgets again

		if (!readonly || canApplyToReadOnly())
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
	if (exitEarly())
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
	if (isBufferDependent() && !isBufferAvailable())
		return;

	if (!initialiseParams(data)) {
		LYXERR(Debug::ANY, "Dialog \"" << name()
			<< "\" failed to translate the data string passed to show()");
		return;
	}

	bc().setReadOnly(isBufferReadonly());
	showView();
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void GuiDialog::updateData(string const & data)
{
	if (isBufferDependent() && !isBufferAvailable())
		return;

	if (!initialiseParams(data)) {
		LYXERR(Debug::ANY, "Dialog \"" << name()
		       << "\" could not be initialized");
		return;
	}

	bc().setReadOnly(isBufferReadonly());
	updateView();
	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void GuiDialog::hide()
{
	if (!isVisibleView())
		return;

	clearParams();
	hideView();
	Dialog::disconnect();
}


void GuiDialog::apply()
{
	if (isBufferDependent()) {
		if (!isBufferAvailable() ||
		    (isBufferReadonly() && !canApplyToReadOnly()))
			return;
	}

	applyView();
	dispatchParams();

	if (disconnectOnApply() && !is_closing_) {
		Dialog::disconnect();
		initialiseParams(string());
		updateView();
	}
}


void GuiDialog::showEvent(QShowEvent * e)
{
	QSettings settings;
	string key = name() + "/geometry";
	restoreGeometry(settings.value(key.c_str()).toByteArray());
	QDialog::showEvent(e);
}


void GuiDialog::closeEvent(QCloseEvent * e)
{
	QSettings settings;
	string key = name() + "/geometry";
	settings.setValue(key.c_str(), saveGeometry());
	QDialog::closeEvent(e);
}

} // namespace frontend
} // namespace lyx


/////////////////////////////////////////////////////////////////////
//
// Command based dialogs
//
/////////////////////////////////////////////////////////////////////

#include "FuncRequest.h"
#include "insets/InsetCommand.h"


using std::string;

namespace lyx {
namespace frontend {

GuiCommand::GuiCommand(GuiView & lv, string const & name)
	: GuiDialog(lv, name), params_(insetCode(name)), lfun_name_(name)
{
}


bool GuiCommand::initialiseParams(string const & data)
{
	// The name passed with LFUN_INSET_APPLY is also the name
	// used to identify the mailer.
	InsetCommandMailer::string2params(lfun_name_, data, params_);
	return true;
}


void GuiCommand::dispatchParams()
{
	if (lfun_name_.empty())
		return;

	string const lfun = 
		InsetCommandMailer::params2string(lfun_name_, params_);
	dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx

#include "GuiDialog_moc.cpp"
