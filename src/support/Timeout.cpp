/**
 * \file Timeout.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lassert.h"
#include "support/Timeout.h"
#include "support/debug.h"

#include <QObject>
#include <QTimerEvent>

using namespace std;

namespace lyx {

/**
 * This class executes the callback when the timeout expires
 * using Qt mechanisms
 */
class Timeout::Impl : QObject {
public:
	///
	Impl(Timeout & owner) : owner_(owner), timeout_id(-1) {}
	///
	bool running() const { return timeout_id != -1; }
	/// start the timer
	void start();
	/// stop the timer
	void stop();
	/// reset
	void reset();
	///
	unsigned int timeout_ms() const { return owner_.timeout_ms; }

protected:
	///
	void timerEvent(QTimerEvent *) { owner_.emit(); }

private:
	///
	Timeout & owner_;
	/// timout id
	int timeout_id;
};


void Timeout::Impl::reset()
{
	if (timeout_id != -1)
		killTimer(timeout_id);
	timeout_id = -1;
}


void Timeout::Impl::start()
{
	if (running())
		lyxerr << "Timeout::start: already running!" << endl;
	timeout_id = startTimer(timeout_ms());
}


void Timeout::Impl::stop()
{
	if (running())
		reset();
}


//
// Timeout
//

Timeout::Timeout(unsigned int msec, Type t)
	: pimpl_(new Impl(*this)), type(t), timeout_ms(msec)
{}


Timeout::~Timeout()
{
	pimpl_->stop();
	delete pimpl_;
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
	return *this;
}


Timeout & Timeout::setTimeout(unsigned int msec)
{
	// Can't have a timeout of zero!
	LASSERT(msec, /**/);

	timeout_ms = msec;
	return *this;
}


} // namespace lyx
