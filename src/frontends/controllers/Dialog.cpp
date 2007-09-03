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

#include "frontends/LyXView.h"

#include "debug.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyXFunc.h"


using std::string;

namespace lyx {
namespace frontend {

Dialog::Dialog(LyXView & lv, string const & name)
	: is_closing_(false), kernel_(lv), name_(name)
{}


Dialog::~Dialog()
{}


void Dialog::setButtonsValid(bool /*valid*/)
{}


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

	preShow();
	view().showView();
	postShow();
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

	preUpdate();
	view().updateView();
	postUpdate();
}


void Dialog::checkStatus()
{
}


void Dialog::hide()
{
	if (!view().isVisibleView())
		return;

	controller().clearParams();
	view().hideView();
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

	view().applyView();
	controller().dispatchParams();

	if (controller().disconnectOnApply() && !is_closing_) {
		kernel().disconnect(name());
		controller().initialiseParams(string());
		view().updateView();
	}
}


bool Dialog::isVisible() const
{
	return view().isVisibleView();
}


void Dialog::redraw()
{
	view().redrawView();
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


void Dialog::View::setViewTitle(docstring const & newtitle)
{
	title_ = newtitle;
}


docstring const & Dialog::View::getViewTitle() const
{
	return title_;
}


void Dialog::View::partialUpdateView(int)
{}

} // namespace frontend
} // namespace lyx
