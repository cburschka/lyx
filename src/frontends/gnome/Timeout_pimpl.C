/**
 * \file gnome/Timeout_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Michael Koziarski
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <gtkmm/main.h>
#include "Timeout_pimpl.h"
#include "debug.h"


Timeout::Pimpl::Pimpl(Timeout * owner)
	: owner_(owner)
{
}


void Timeout::Pimpl::reset()
{
	stop();
}

bool Timeout::Pimpl::running() const
{
	return running_;
}

void Timeout::Pimpl::start()
{
	if (conn_.connected()) {
		lyxerr << "Timeout::start: already running!" << std::endl;
		stop();
	}

	conn_ = Gtk::Main::signal_timeout().connect(
			 SigC::slot(*this, &Timeout::Pimpl::timeoutEvent),
			 owner_->timeout_ms
			);
	running_ = true;
}


void Timeout::Pimpl::stop()
{
	conn_.disconnect();
	running_ = false;
}


bool Timeout::Pimpl::timeoutEvent()
{
	owner_->emit();
	return false; // discontinue emitting timeouts.
}
