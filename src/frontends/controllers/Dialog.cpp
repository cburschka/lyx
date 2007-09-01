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

#include "Dialog.h"

#include "ButtonController.h"
#include "BCView.h"

#include "frontends/LyXView.h"

#include "debug.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyXFunc.h"


using std::string;

namespace lyx {
namespace frontend {

Dialog::Dialog(LyXView & lv, string const & name)
	: is_closing_(false), kernel_(lv), name_(name),
	  bc_ptr_(new ButtonController)
{}


void Dialog::ApplyButton()
{
	apply();
	bc().apply();
}


void Dialog::OKButton()
{
	is_closing_ = true;
	apply();
	is_closing_ = false;
	hide();
	bc().ok();
}


void Dialog::CancelButton()
{
	hide();
	bc().cancel();
}


void Dialog::RestoreButton()
{
	// Tell the kernel that a request to refresh the dialog's contents
	// has been received. It's up to the kernel to supply the necessary
	// info by calling Dialog::update().
	kernel().updateDialog(name_);
	bc().restore();
}


void Dialog::show(string const & data)
{
	if (controller().isBufferDependent() && !kernel().isBufferAvailable())
		return;

	if (!controller().initialiseParams(data)) {
		lyxerr << "Dialog \"" << name_
		       << "\" failed to translate the data "
			"string passed to show()" << std::endl;
		return;
	}

	bc().readOnly(kernel().isBufferReadonly());
	view().show();

	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void Dialog::update(string const & data)
{
	if (controller().isBufferDependent() && !kernel().isBufferAvailable())
		return;

	if (!controller().initialiseParams(data)) {
		lyxerr << "Dialog \"" << name_
		       << "\" could not be initialized" << std::endl;
		return;
	}

	bc().readOnly(kernel().isBufferReadonly());
	view().update();

	// The widgets may not be valid, so refresh the button controller
	bc().refresh();
}


void Dialog::hide()
{
	if (!view().isVisible())
		return;

	controller().clearParams();
	view().hide();
	kernel().disconnect(name());
}


void Dialog::apply()
{
	if (controller().isBufferDependent()) {
		if (!kernel().isBufferAvailable() ||
		    (kernel().isBufferReadonly() &&
		     !controller().canApplyToReadOnly()))
			return;
	}

	view().apply();
	controller().dispatchParams();

	if (controller().disconnectOnApply() && !is_closing_) {
		kernel().disconnect(name());
		controller().initialiseParams(string());
		view().update();
	}
}


bool Dialog::isVisible() const
{
	return view().isVisible();
}


void Dialog::redraw()
{
	view().redraw();
}


ButtonController & Dialog::bc() const
{
	BOOST_ASSERT(bc_ptr_.get());
	return *bc_ptr_.get();
}


void Dialog::setController(Controller * i)
{
	BOOST_ASSERT(i && !controller_ptr_.get());
	controller_ptr_.reset(i);
}


void Dialog::setView(View * v)
{
	BOOST_ASSERT(v && !view_ptr_.get());
	view_ptr_.reset(v);
}


void Dialog::checkStatus()
{
	// buffer independant dialogs are always active.
	// This check allows us leave canApply unimplemented for some dialogs.
	if (!controller().isBufferDependent())
		return;

	// deactivate the dialog if we have no buffer
	if (!kernel().isBufferAvailable()) {
		bc().readOnly(true);
		return;
	}

	// check whether this dialog may be active
	if (controller().canApply()) {
		bool const readonly = kernel().isBufferReadonly();
		bc().readOnly(readonly);
		// refreshReadOnly() is too generous in _enabling_ widgets
		// update dialog to disable disabled widgets again
		if (!readonly || controller().canApplyToReadOnly())
			view().update();
	} else
		bc().readOnly(true);
}


Dialog::Controller::Controller(Dialog & parent)
	: parent_(parent)
{}


bool Dialog::Controller::canApply() const
{
	FuncRequest const fr(getLfun(), dialog().name());
	FuncStatus const fs(getStatus(fr));
	return fs.enabled();
}


Dialog::Controller & Dialog::controller() const
{
	BOOST_ASSERT(controller_ptr_.get());
	return *controller_ptr_.get();
}


Dialog::View::View(Dialog & parent, docstring title) :
	p_(parent), title_(title)
{}


Dialog::View & Dialog::view() const
{
	BOOST_ASSERT(view_ptr_.get());
	return *view_ptr_.get();
}


void Dialog::View::setTitle(docstring const & newtitle)
{
	title_ = newtitle;
}


docstring const & Dialog::View::getTitle() const
{
	return title_;
}


void Dialog::View::partialUpdate(int)
{}

} // namespace frontend
} // namespace lyx
