/**
 * \file xformsBC.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "xformsBC.h"
#include "xforms_helpers.h"
#include "lyx_forms.h"


xformsBC::xformsBC(ButtonController const & parent,
		   string const & cancel, string const & close)
	: GuiBC<FL_OBJECT, FL_OBJECT>(parent, cancel, close)
{}


void xformsBC::setButtonEnabled(FL_OBJECT * obj, bool enabled) const
{
	setEnabled(obj, enabled);
}


void xformsBC::setWidgetEnabled(FL_OBJECT * obj, bool enabled) const
{
	setEnabled(obj, enabled);
}


void xformsBC::setButtonLabel(FL_OBJECT * obj, string const & label) const
{
	fl_set_object_label(obj, label.c_str());
}
