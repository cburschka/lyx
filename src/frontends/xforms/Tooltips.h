// -*- C++ -*-
/**
 * \file Tooltips.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

/* Tooltips for xforms. xforms 0.89 supports them directly, but 0.88 needs
 * a bit of jiggery pokery. This class wraps it all up in a neat interface.
 * Based on code originally in Toolbar_pimpl.C that appears to have been
 * written by Matthias Ettrich and Jean-Marc Lasgouttes.
 */

#ifndef TOOLTIPS_H
#define TOOLTIPS_H

#include "LString.h"

#include <boost/utility.hpp>
#include <boost/signals/signal0.hpp>
#include <boost/signals/trackable.hpp>

#include "forms_fwd.h" // Can't forward-declare FL_OBJECT

#include <map>

class Tooltips : boost::noncopyable, public boost::signals::trackable {
public:
	///
	Tooltips();

	/// Initialise a tooltip for this ob.
	void init(FL_OBJECT * ob, string const & tip);

	/// Are the tooltips on or off?
	static bool enabled() { return enabled_; }

	/** This method is connected to Dialogs::toggleTooltips and toggles
	 *  the state of enabled_.
	 */
	static void toggleEnabled();

private:

	/// This method is connected to the Tooltips::toggled signal.
	void set();

	/// Are the tooltips on or off?
	static bool enabled_;

	/** Once enabled_ is changed, then this signal is emitted to update
	 *  all the tooltips.
	 */
	static boost::signal0<void> toggled;

	/// The tooltips are stored so that they can be turned on and off.
	typedef std::map<FL_OBJECT *, string> TooltipsMap;

	TooltipsMap tooltipsMap;
};

#endif // TOOLTIPS_H
