/**
 * \file ControlError.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlError.h"


ControlError::ControlError(Dialog & parent)
	: Dialog::Controller(parent)
{}


void ControlError::initialiseParams(string const & data)
{
	params_ = data;
}


void ControlError::clearParams()
{
	// g++ 2.95.3 doesn't like this although it should be ok
	//params_.clear();
	params_ = string();
}

