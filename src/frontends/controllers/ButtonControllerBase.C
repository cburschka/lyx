/**
 * \file ButtonControllerBase.C
 * Read the file COPYING
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ButtonControllerBase.h"
#include "support/LAssert.h"
#include "debug.h"


ButtonControllerBase::ButtonControllerBase(string const & cancel,
					   string const & close)
	: cancel_label_(cancel), close_label_(close)
{}


void ButtonControllerBase::ok()
{
	input(ButtonPolicy::SMI_OKAY);
}


void ButtonControllerBase::input(ButtonPolicy::SMInput in)
{
	if (ButtonPolicy::SMI_NOOP == in) return;
	bp().input(in);
	refresh();
}


void ButtonControllerBase::apply()
{
	input(ButtonPolicy::SMI_APPLY);
}


void ButtonControllerBase::cancel()
{
	input(ButtonPolicy::SMI_CANCEL);
}


void ButtonControllerBase::restore()
{
	input(ButtonPolicy::SMI_RESTORE);
}


void ButtonControllerBase::hide()
{
	input(ButtonPolicy::SMI_HIDE);
}


void ButtonControllerBase::valid(bool v)
{
	if (v) {
		input(ButtonPolicy::SMI_VALID);
	} else {
		input(ButtonPolicy::SMI_INVALID);
	}
}


void ButtonControllerBase::invalid()
{
	input(ButtonPolicy::SMI_INVALID);
}


bool ButtonControllerBase::readOnly(bool ro)
{
	lyxerr[Debug::GUI] << "Setting controller ro: " << ro << std::endl;

	if (ro) {
		bp().input(ButtonPolicy::SMI_READ_ONLY);
	} else {
		bp().input(ButtonPolicy::SMI_READ_WRITE);
	}
	refreshReadOnly();
	refresh();
	return ro;
}


void ButtonControllerBase::readWrite()
{
	readOnly(false);
}
