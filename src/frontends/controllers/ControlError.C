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

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlError.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlInset.tmpl"
#include "Dialogs.h"
#include "buffer.h"
#include "insets/inseterror.h"

#include "frontends/LyXView.h"

#include <boost/bind.hpp>

ControlError::ControlError(LyXView & lv, Dialogs & d)
	: ControlInset<InsetError, string>(lv, d)
{
	d_.showError = boost::bind(&ControlError::showInset, this, _1);
}


string const ControlError::getParams(InsetError const & inset)
{
	return inset.getContents();
}
