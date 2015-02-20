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

#include <ctime>
#include <iomanip>
#include <sstream>

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
	LASSERT(msec, msec = 1000);

	timeout_ms = msec;
	return *this;
}


struct Timer::Private
{
	time_t start_time;
};


Timer::Timer() : d(new Private)
{
	restart();
}


Timer::~Timer()
{
	delete d;
}


void Timer::restart()
{
	time(&d->start_time);
}


int Timer::elapsed() const
{
	time_t end_time;
	time(&end_time);
	double diff = difftime(end_time, d->start_time);
	return int(diff);
}


string Timer::timeStr(char separator) const
{
	tm * timeinfo = localtime(&d->start_time);
	// With less flexibility we could also use:
	//strftime(buffer, 10, "%X", timeinfo);
	ostringstream out;
	out << setw(2) << setfill('0');
	if (separator) {
		out << separator << setw(2) << setfill('0') << timeinfo->tm_hour
		    << separator << setw(2) << setfill('0') << timeinfo->tm_min
		    << separator << setw(2) << setfill('0') << timeinfo->tm_sec;
	} else {
		out << setw(2) << setfill('0') << timeinfo->tm_hour
		    << setw(2) << setfill('0') << timeinfo->tm_min
		    << setw(2) << setfill('0') << timeinfo->tm_sec;
	}
	return out.str();
}


string Timer::dateStr(char separator) const
{
	tm * timeinfo = localtime(&d->start_time);
	// With less flexibility we could also use:
	//res = strftime(buffer, 10, "%d%m%y", timeinfo);
	ostringstream out;
	out << setw(2) << setfill('0') << timeinfo->tm_mday;
	if (separator)
		out << separator;
	out << setw(2) << setfill('0') << timeinfo->tm_mon;
	if (separator)
		out << separator;
	out << setw(2) << setfill('0') << timeinfo->tm_year - 100;
	return out.str();
}


string Timer::toStr() const
{
	tm * timeinfo = localtime(&d->start_time);
	return asctime(timeinfo);
}


string Timer::currentToStr()
{
	time_t current_time;
	time(&current_time);
	tm * timeinfo = localtime(&current_time);
	return asctime(timeinfo);
}

} // namespace lyx
