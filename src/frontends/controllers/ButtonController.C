/**
 * \file ButtonController.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS
 */


#include <config.h>
#include "ButtonController.h"
#include "BCView.h"
#include "support/LAssert.h"
#include "debug.h"


ButtonController::~ButtonController()
{}


BCView & ButtonController::view() const
{
	lyx::Assert(view_.get());
	return *view_.get();
}

void ButtonController::view(BCView * view)
{
	view_.reset(view);
}


ButtonPolicy & ButtonController::bp() const
{
	lyx::Assert(bp_.get());
	return *bp_.get();
}

void ButtonController::bp(ButtonPolicy * bp)
{
	bp_.reset(bp);
}


void ButtonController::refresh()
{
	view().refresh();
}

		
void ButtonController::refreshReadOnly()
{
	view().refreshReadOnly();
}


void ButtonController::ok()
{
	input(ButtonPolicy::SMI_OKAY);
}


void ButtonController::input(ButtonPolicy::SMInput in)
{
	if (ButtonPolicy::SMI_NOOP == in)
		return;
	bp().input(in);
	view().refresh();
}


void ButtonController::apply()
{
	input(ButtonPolicy::SMI_APPLY);
}


void ButtonController::cancel()
{
	input(ButtonPolicy::SMI_CANCEL);
}


void ButtonController::restore()
{
	input(ButtonPolicy::SMI_RESTORE);
}


void ButtonController::hide()
{
	input(ButtonPolicy::SMI_HIDE);
}


void ButtonController::valid(bool v)
{
	if (v) {
		input(ButtonPolicy::SMI_VALID);
	} else {
		input(ButtonPolicy::SMI_INVALID);
	}
}


void ButtonController::invalid()
{
	input(ButtonPolicy::SMI_INVALID);
}


bool ButtonController::readOnly(bool ro)
{
	lyxerr[Debug::GUI] << "Setting controller ro: " << ro << std::endl;

	if (ro) {
		bp().input(ButtonPolicy::SMI_READ_ONLY);
	} else {
		bp().input(ButtonPolicy::SMI_READ_WRITE);
	}
	view().refreshReadOnly();
	view().refresh();
	return ro;
}


void ButtonController::readWrite()
{
	readOnly(false);
}
