/**
 * \file gtk/GTimeout.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Michael Koziarski
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include <gtkmm.h>

#include "GTimeout.h"
#include "debug.h"


Timeout::Timeout(unsigned int msec, Type t)
	: pimpl_(new GTimeout(*this)), type(t), timeout_ms(msec)
{}


GTimeout::GTimeout(Timeout & owner)
	: Timeout::Impl(owner)
{
}


void GTimeout::reset()
{
	stop();
}


bool GTimeout::running() const
{
	return running_;
}


void GTimeout::start()
{
	if (conn_.connected()) {
		lyxerr << "Timeout::start: already running!" << std::endl;
		stop();
	}

	conn_ = Glib::signal_timeout().connect(
			 SigC::slot(*this, &GTimeout::timeoutEvent),
			 timeout_ms()
			);
	running_ = true;
}


void GTimeout::stop()
{
	conn_.disconnect();
	running_ = false;
}


bool GTimeout::timeoutEvent()
{
	emit();
	return false; // discontinue emitting timeouts.
}
