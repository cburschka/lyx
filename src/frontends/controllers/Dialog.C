/**
 * \file Dialog.C
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


using std::string;


Dialog::Dialog(LyXView & lv, string const & name)
	: is_closing_(false), kernel_(lv), name_(name),
	  bc_ptr_(new ButtonController)
{}


Dialog::~Dialog()
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
		    kernel().isBufferReadonly())
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


Dialog::Controller::Controller(Dialog & parent)
	: parent_(parent)
{}


Dialog::Controller & Dialog::controller() const
{
	BOOST_ASSERT(controller_ptr_.get());
	return *controller_ptr_.get();
}


Dialog::View::View(Dialog & parent, string title) :
	p_(parent), title_(title)
{}


Dialog::View & Dialog::view() const
{
	BOOST_ASSERT(view_ptr_.get());
	return *view_ptr_.get();
}


void Dialog::View::setTitle(string const & newtitle)
{
	title_ = newtitle;
}


string const & Dialog::View::getTitle() const
{
	return title_;
}


void Dialog::View::partialUpdate(int)
{}
