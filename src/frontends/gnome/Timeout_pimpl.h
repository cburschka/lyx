// -*- C++ -*-
/**
 * \file gnome/Timeout_pimpl.h
 * Copyright 2001 LyX Team
 * Read COPYING
 *
 * \author Baruch Even
 */
#ifndef TIMEOUTPIMPL_H
#define TIMEOUTPIMPL_H

#include "frontends/Timeout.h"
#include "glib.h" // for gint

#include <sigc++/signal_system.h>

#ifdef __GNUG__
#pragma interface
#endif

/**
 * This class executes the callback when the timeout expires
 * using Gnome mechanisms
 */
struct Timeout::Pimpl : public SigC::Object {
public:
	///
	Pimpl(Timeout * owner_);
	/// start the timer
	void start();
	/// stop the timer
	void stop();
	/// reset
        void reset();
        /// Is the timer running?
        bool running() const;
  

public:
	/// The timeout signal, this gets called when the timeout passed.
	gint timeoutEvent();
private:
	/// the owning timer
	Timeout * owner_;
	/// Timer connection
	SigC::Connection conn_;
        /// Used for running as SigC::Connection::connected() isn't const
        bool running_;
};

#endif
