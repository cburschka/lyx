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
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlError.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "insets/inseterror.h"

using SigC::slot;

ControlError::ControlError(LyXView & lv, Dialogs & d)
	: ControlInset<InsetError, string>(lv, d)
{
	d_.showError.connect(slot(this, &ControlError::showInset));
}

string const ControlError::getParams(InsetError const & inset)
{
	return inset.getContents();
}

