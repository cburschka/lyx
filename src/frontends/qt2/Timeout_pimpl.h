// -*- C++ -*-
/**
 * \file qt2/Timeout_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef TIMEOUTPIMPL_H
#define TIMEOUTPIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/Timeout.h"
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
