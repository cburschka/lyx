// -*- C++ -*-
/**
 * \file qtTimeout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTTIMEOUT_H
#define QTTIMEOUT_H

#include "frontends/Timeout.h"
#include <qobject.h> 

// stupid Qt
#undef emit

/**
 * This class executes the callback when the timeout expires
 * using Qt mechanisms
 */
struct qtTimeout : QObject, public Timeout::Impl {
public:
	///
	qtTimeout(Timeout & owner_);
	///
	virtual bool running() const;
	/// start the timer
	virtual void start();
	/// stop the timer
	virtual void stop();
	/// reset
	virtual void reset();

protected:
	/// slot
	virtual void timerEvent(QTimerEvent *);

private:
	/// timout id
	int timeout_id;
};

#endif // QTTIMEOUT_H
