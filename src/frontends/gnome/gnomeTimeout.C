/**
 * \file gnomeTimeout.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Michael Koziarski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <glibmm/main.h>
#include "gnomeTimeout.h"
#include "debug.h"


Timeout::Timeout(unsigned int msec, Type t)
	: pimpl_(new gnomeTimeout(*this)), type(t), timeout_ms(msec)
{}


gnomeTimeout::(gnomeTimeoutTimeout * owner)
	: Timeout::Impl(owner), timeout_id(-1)
{}


void gnomeTimeout::reset()
{
	stop();
}


bool gnomeTimeout::running() const
{
	return running_;
}


void gnomeTimeout::start()
{
	if (conn_.connected()) {
		lyxerr << "Timeout::start: already running!" << std::endl;
		stop();
	}

	conn_ = Glib::signal_timeout().connect(
			 SigC::slot(*this, &Timeout::Pimpl::timeoutEvent),
			 timeout_ms()
			);
	running_ = true;
}


void gnomeTimeout::stop()
{
	conn_.disconnect();
	running_ = false;
}


bool gnomeTimeout::timeoutEvent()
{
	emit();
	return false; // discontinue emitting timeouts.
}
