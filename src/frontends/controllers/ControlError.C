/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlError.C
 * \author Angus Leeming <leeming@lyx.org>
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
