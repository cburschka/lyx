// -*- C++ -*-
/**
 * \file Timeout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */
 
#ifndef TIMEOUT_H
#define TIMEOUT_H

#ifdef __GNUG__
#pragma interface
#endif

#include <boost/signals/signal0.hpp>

/**
 * This class executes the callback when the timeout expires.
 */
class Timeout {
public:
	/// the timeout type
	enum Type {
		ONETIME, //< one-shot timer
		CONTINUOUS //< repeating
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
	boost::signal0<void> timeout;
	/// emit the signal
	void emit();
	/// set the timer type
	Timeout & setType(Type t);
	/// set the timeout value
	Timeout & setTimeout(unsigned int msec);

private:
	///
	struct Pimpl;
	///
	friend struct Pimpl;
	/// implementation
	Pimpl * pimpl_;
	/// one-shot or repeating
	Type type;
	/// timeout value in milliseconds
	unsigned int timeout_ms;
};

#endif
