// -*- C++ -*-
/**
 * \file gtk/GTimeout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTIMEOUT_H
#define GTIMEOUT_H

#include "frontends/Timeout.h"

#include <sigc++/sigc++.h>

/**
 * This class executes the callback when the timeout expires
 * using Gtk mechanisms
 */
class GTimeout : public Timeout::Impl, public SigC::Object {
public:
	///
	GTimeout(Timeout & owner_);
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
	bool timeoutEvent();
private:
	/// Timer connection
	SigC::Connection conn_;
	/// Used for running as SigC::Connection::connected() isn't const
	bool running_;
};

#endif
