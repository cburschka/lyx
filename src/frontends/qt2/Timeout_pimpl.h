// -*- C++ -*-
/**
 * \file qt2/Timeout_pimpl.h
 * Copyright 2001 LyX Team
 * Read COPYING
 *
 * \author John Levon
 */

#ifndef TIMEOUTPIMPL_H
#define TIMEOUTPIMPL_H

#include <config.h>

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/Timeout.h"

#include <sigc++/signal_system.h>

#include <qobject.h>
// stupid Qt
#undef emit

/**
 * This class executes the callback when the timeout expires
 * using Qt mechanisms
 */
struct Timeout::Pimpl : QObject {
public:
	///
	Pimpl(Timeout * owner_);
	/// start the timer
	void start();
	/// stop the timer
	void stop();
	/// reset
	void reset();
	/// is the timer running ?
	bool running() const;

protected:
	/// slot
	virtual void timerEvent(QTimerEvent *);
private:
	/// the owning timer
	Timeout * owner_;
	/// xforms id
	int timeout_id;
};

#endif
