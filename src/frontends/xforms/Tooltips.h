// -*- C++ -*-
/*
 * \file Tooltips.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 *
 * Tooltips for xforms. xforms 0.89 supports them directly, but 0.88 needs
 * a bit of jiggery pokery. This class wraps it all up in a neat interface.
 * Based on code originally in Toolbar_pimpl.C that appears to have been
 * written by Matthias Ettrich and Jean-Marc Lasgouttes.
 */

#ifndef TOOLTIPS_H
#define TOOLTIPS_H

#include "LString.h"
#include <boost/utility.hpp>
#include <sigc++/signal_system.h>
#include FORMS_H_LOCATION // Can't forward-declare FL_OBJECT

#ifdef __GNUG__
#pragma interface
#endif

class Tooltips : boost::noncopyable {
public:
	/// Activate tooltips for this ob
	void activateTooltip(FL_OBJECT * ob);
	/** Connect this signal to the function returning the tooltip for ob
	    Eg, string FormBase::getTooltip(FL_OBJECT const *)
	    Note that SigC is unable to create a Signal1 returning string const
	    or to connect it to a const method.
	 */
	SigC::Signal1<string, FL_OBJECT const *> getTooltip;

	// We use the old method because we want to change tooltips on the fly
//#if FL_REVISION < 89
	///
	Tooltips() : tooltip_timer_(0) {}

private:

	FL_OBJECT * tooltip_timer_;
//#endif
};

#endif // TOOLTIPS_H
