/**
 * \file xformsBC.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author Baruch Even, baruch.even@writeme.com
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ButtonController.tmpl"
#include "xforms_helpers.h"

xformsBC::xformsBC(string const & cancel, string const & close)
	: GuiBC<FL_OBJECT, FL_OBJECT>(cancel, close)
{}


void xformsBC::setButtonEnabled(FL_OBJECT * obj, bool enabled)
{
	setEnabled(obj, enabled);
}


void xformsBC::setWidgetEnabled(FL_OBJECT * obj, bool enabled)
{
	setEnabled(obj, enabled);
}


void xformsBC::setButtonLabel(FL_OBJECT * obj, string const & label)
{
	fl_set_object_label(obj, label.c_str());
}
