// -*- C++ -*-
/**
 * \file Timeout.h
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

private:
	/// noncopyable
	Timeout(Timeout const &);
	void operator=(Timeout const &);
	///
	class Impl;
	///
	friend class Impl;
	///
	Impl * const pimpl_;
	/// one-shot or repeating
	Type type;
	/// timeout value in milliseconds
	unsigned int timeout_ms;
};

// Small Timer class useful for debugging and performance investigation.
class Timer
{
public:
	///
	Timer();
	///
	~Timer();
	///
	void restart();
	///
	int elapsed() const;
	///
	std::string dateStr(char separator = 0) const;
	///
	std::string timeStr(char separator = 0) const;
	///
	std::string toStr() const;
	///
	static std::string currentToStr();
private:
	struct Private;
	Private * d;
};

} // namespace lyx

#endif
