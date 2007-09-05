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
	showView();
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
	updateView();
	postUpdate();
}


void Dialog::checkStatus()
{
}


void Dialog::hide()
{
	if (!isVisibleView())
		return;

	controller().clearParams();
	hideView();
	kernel().disconnect(name());
}


void Dialog::apply()
{
	if (controller().isBufferDependent()) {
		if (!kernel().isBufferAvailable() ||
		    (kernel().isBufferReadonly() && !controller().canApplyToReadOnly()))
			return;
	}

	applyView();
	controller().dispatchParams();

	if (controller().disconnectOnApply() && !is_closing_) {
		kernel().disconnect(name());
		controller().initialiseParams(string());
		updateView();
	}
}


bool Dialog::isVisible() const
{
	return isVisibleView();
}


void Dialog::redraw()
{
	redrawView();
}


void Dialog::setController(Controller * i)
{
	BOOST_ASSERT(i && !controller_ptr_.get());
	controller_ptr_.reset(i);
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


void Dialog::setViewTitle(docstring const & newtitle)
{
	title_ = newtitle;
}


docstring const & Dialog::getViewTitle() const
{
	return title_;
}


} // namespace frontend
} // namespace lyx
