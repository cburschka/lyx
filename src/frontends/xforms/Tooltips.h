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
#include <map>

#ifdef __GNUG__
#pragma interface
#endif

class Tooltips : boost::noncopyable, public SigC::Object {
public:
	Tooltips();

	/// Initialise a tooltip for this ob
	void initTooltip(FL_OBJECT * ob, string const & tip);

	/// Are the tooltips on or off?
	static bool enabled() { return enabled_; }

#if FL_REVISION < 89

	/// Return the tooltip associated with this object
	string const getTooltip(FL_OBJECT *) const;

#endif

private:

	/// Are the tooltips on or off?
	static bool enabled_;

	/** This method is connected to Dialogs::toggleTooltips and toggles
	 *  the state of enabled_
	 */
	static void toggleEnabled();

	/** Once enabled_ is changed, then this signal is emitted to update
	 *  all the tooltips.
	 */
	static SigC::Signal0<void> tooltipsToggled;

	/// This method is connected to the tooltipsToggled signal.
	void toggleTooltips();

	/// The tooltips are stored so that they can be turned on and off
	typedef std::map<FL_OBJECT *, string> TooltipsMap;

	TooltipsMap tooltipsMap;

#if FL_REVISION < 89

	FL_OBJECT * tooltip_timer_;

#endif // FL_REVISION >= 89
};

#endif // TOOLTIPS_H
