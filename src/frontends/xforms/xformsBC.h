// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file xformsBC.h
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author Baruch Even, baruch.even@writeme.com
 */

#ifndef XFORMSBC_H
#define XFORMSBC_H

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonController.h"

class xformsBC : public GuiBC<FL_OBJECT, FL_OBJECT>
{
public:
	///
	xformsBC(string const &, string const &);

private:
	/// Updates the button sensitivity (enabled/disabled)
	void setButtonEnabled(FL_OBJECT *, bool enabled);

	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(FL_OBJECT *, bool enabled);

	/// Set the label on the button
	void setButtonLabel(FL_OBJECT *, string const & label);
};

#endif // XFORMSBC_H
