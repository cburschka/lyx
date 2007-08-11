/**
 * \file qtTimeout.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <QObject>
#include <QTimerEvent>

#include "debug.h"
#include "Timeout.h"


namespace lyx {

/**
 * This class executes the callback when the timeout expires
 * using Qt mechanisms
 */
class qtTimeout : QObject, public Timeout::Impl {
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


Timeout::Timeout(unsigned int msec, Type t)
	: pimpl_(new qtTimeout(*this)), type(t), timeout_ms(msec)
{}


qtTimeout::qtTimeout(Timeout & owner)
	: Timeout::Impl(owner), timeout_id(-1)
{}


void qtTimeout::timerEvent(QTimerEvent *)
{
	emit();
}


void qtTimeout::reset()
{
	if (timeout_id != -1)
		killTimer(timeout_id);
	timeout_id = -1;
}


bool qtTimeout::running() const
{
	return timeout_id != -1;
}


void qtTimeout::start()
{
	if (running())
		lyxerr << "Timeout::start: already running!" << std::endl;
	timeout_id = startTimer(timeout_ms());
}


void qtTimeout::stop()
{
	if (running())
		reset();
}


//
// Timeoute
//

Timeout::~Timeout()
{
	pimpl_->stop();
}


bool Timeout::running() const
{
	return pimpl_->running();
}


void Timeout::start()
{
	pimpl_->start();
}


void Timeout::stop()
{
	pimpl_->stop();
}


void Timeout::restart()
{
	pimpl_->stop();
	pimpl_->start();
}


void Timeout::emit()
{
	pimpl_->reset();
	timeout();
	if (type == CONTINUOUS)
		pimpl_->start();
}


Timeout & Timeout::setType(Type t)
{
	type = t;
	return * this;
}


Timeout & Timeout::setTimeout(unsigned int msec)
{
	// Can't have a timeout of zero!
	BOOST_ASSERT(msec);

	timeout_ms = msec;
	return * this;
}


} // namespace lyx
