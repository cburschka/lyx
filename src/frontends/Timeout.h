// -*- C++ -*-
/**
 * \file Timeout.h
 * Copyright 2001 LyX Team
 * Read COPYING
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 */
#ifndef TIMEOUT_H
#define TIMEOUT_H

#ifdef __GNUG__
#pragma interface
#endif

#include <sigc++/signal_system.h>

/**
 * This class executes the callback when the timeout expires.
 */
class Timeout {
public:
	///
	enum Type {
		/// one-shot timer
		ONETIME,
		/// repeating
		CONTINUOUS
	};
	///
	Timeout(unsigned int msec, Type = ONETIME);
	///
	~Timeout();
	/// Is the timer running?
	bool running() const;
	/// start the timer
	void start();
	/// stop the timer
	void stop();
	/// restart the timer
	void restart();
	/// signal emitted on timer expiry
	SigC::Signal0<void> timeout;
	/// emit the signal
	void emit();
	/// set the timer type
	Timeout & setType(Type t);
	/// set the timeout value
	Timeout & setTimeout(unsigned int msec);

private:
	struct Pimpl;
	friend struct Pimpl;
	/// implementation
	Pimpl * pimpl_;

	/// one-shot or repeating
	Type type;
	/// timeout value in milliseconds
	unsigned int timeout_ms;
};

#endif
