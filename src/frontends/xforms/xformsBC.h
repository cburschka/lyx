// -*- C++ -*-
/**
 * \file xformsBC.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming <leeming@lyx.org>
 * \author Baruch Even, baruch.even@writeme.com
 */

#ifndef XFORMSBC_H
#define XFORMSBC_H

#include "forms_fwd.h"

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonController.h"

class xformsBC : public GuiBC<FL_OBJECT, FL_OBJECT> {
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
