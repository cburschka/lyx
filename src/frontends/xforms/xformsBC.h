// -*- C++ -*-
/**
 * \file xformsBC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming 
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
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
