/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ButtonControllerBase.C
 * \author Allan Rae
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ButtonControllerBase.h"
#include "support/LAssert.h"


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
	if (ro) {
		bp().input(ButtonPolicy::SMI_READ_ONLY);
	} else {
		bp().input(ButtonPolicy::SMI_READ_WRITE);
	}
	refreshReadOnly();
	return ro;
}


void ButtonControllerBase::readWrite()
{
	readOnly(false);
}
