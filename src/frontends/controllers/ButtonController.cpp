/**
 * \file ButtonController.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ButtonController.h"
#include "BCView.h"
#include "debug.h"

namespace lyx {
namespace frontend {

BCView & ButtonController::view() const
{
	BOOST_ASSERT(view_.get());
	return *view_.get();
}

void ButtonController::view(BCView * view)
{
	view_.reset(view);
}


ButtonPolicy & ButtonController::bp() const
{
	BOOST_ASSERT(bp_.get());
	return *bp_.get();
}

void ButtonController::bp(ButtonPolicy * bp)
{
	bp_.reset(bp);
}


void ButtonController::refresh() const
{
	view().refresh();
}


void ButtonController::refreshReadOnly() const
{
	view().refreshReadOnly();
}


void ButtonController::ok() const
{
	input(ButtonPolicy::SMI_OKAY);
}


void ButtonController::input(ButtonPolicy::SMInput in) const
{
	if (ButtonPolicy::SMI_NOOP == in)
		return;
	bp().input(in);
	view().refresh();
}


void ButtonController::apply() const
{
	input(ButtonPolicy::SMI_APPLY);
}


void ButtonController::cancel() const
{
	input(ButtonPolicy::SMI_CANCEL);
}


void ButtonController::restore() const
{
	input(ButtonPolicy::SMI_RESTORE);
}


void ButtonController::hide() const
{
	input(ButtonPolicy::SMI_HIDE);
}


void ButtonController::valid(bool v) const
{
	if (v) {
		input(ButtonPolicy::SMI_VALID);
	} else {
		input(ButtonPolicy::SMI_INVALID);
	}
}


bool ButtonController::readOnly(bool ro) const
{
	LYXERR(Debug::GUI) << "Setting controller ro: " << ro << std::endl;

	if (ro) {
		bp().input(ButtonPolicy::SMI_READ_ONLY);
	} else {
		bp().input(ButtonPolicy::SMI_READ_WRITE);
	}
	view().refreshReadOnly();
	view().refresh();
	return ro;
}

} // namespace frontend
} // namespace lyx
