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

#ifdef __GNUG__
#pragma interface
#endif

class Tooltips : boost::noncopyable, public boost::signals::trackable {
public:
	///
	Tooltips();

	/// Initialise a tooltip for this ob.
	void init(FL_OBJECT * ob, string const & tip);

	/// Are the tooltips on or off?
	static bool enabled() { return enabled_; }

	/// This method is connected to the tooltipsToggled signal.
	void set();

#if FL_VERSION < 1 && FL_REVISION < 89

	/** Return the tooltip associated with this object.
	 *  Required by an xforms callback routine.
	 */
	string const get(FL_OBJECT *) const;

#endif

	/** This method is connected to Dialogs::toggleTooltips and toggles
	 *  the state of enabled_.
	 */
	static void toggleEnabled();

private:

	/// Are the tooltips on or off?
	static bool enabled_;

	/** Once enabled_ is changed, then this signal is emitted to update
	 *  all the tooltips.
	 */
	static boost::signal0<void> toggled;

	/// The tooltips are stored so that they can be turned on and off.
	typedef std::map<FL_OBJECT *, string> TooltipsMap;

	TooltipsMap tooltipsMap;

#if FL_VERSION < 1 && FL_REVISION < 89

	/** A timer is started once the mouse enters an object, so that the
	 *  tip appears a short delay afterwards.
	 */
	FL_OBJECT * tooltip_timer_;

#endif
};

#endif // TOOLTIPS_H
