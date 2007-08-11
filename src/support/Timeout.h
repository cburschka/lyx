// -*- C++ -*-
/**
 * \file qtTimeout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <boost/signal.hpp>


namespace lyx {

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
	/// Note that the c-tor is implemented in the GUI-specific frontends
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
	boost::signal<void()> timeout;
	/// emit the signal
	void emit();
	/// set the timer type
	Timeout & setType(Type t);
	/// set the timeout value
	Timeout & setTimeout(unsigned int msec);

	/** Base class for the GUI implementation.
	    It must be public so that C callback functions can access its
	    daughter classes.
	 */
	class Impl
	{
	public:
		///
		Impl(Timeout & owner) : owner_(owner) {}
		///
		virtual ~Impl() {}
		/// Is the timer running?
		virtual bool running() const = 0;
		/// start the timer
		virtual void start() = 0;
		/// stop the timer
		virtual void stop() = 0;
		/// reset
		virtual void reset() = 0;

	protected:
		///
		void emit() { owner_.emit(); }
		///
		unsigned int timeout_ms() const { return owner_.timeout_ms; }

	private:
		///
		Timeout & owner_;
	};

private:
	///
	friend class Impl;
	///
	boost::scoped_ptr<Impl> const pimpl_;
	/// one-shot or repeating
	Type type;
	/// timeout value in milliseconds
	unsigned int timeout_ms;
};


} // namespace lyx

#endif
