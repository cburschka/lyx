/**
 * \file ControlError.C
 * Read the file COPYING
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlError.h"
#include "insets/inseterror.h"


ControlError::ControlError(LyXView & lv, Dialogs & d)
	: ControlInset<InsetError, string>(lv, d)
{}


string const ControlError::getParams(InsetError const & inset)
{
	return inset.getContents();
}
