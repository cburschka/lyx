/**
 * \file gnome/Timeout_pimpl.C
 * Copyright 2001 LyX Team
 * Read COPYING
 *
 * \author Baruch Even
 * \author Michael Koziarski
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <gnome--/main.h>
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
	
	conn_ = Gnome::Main::timeout.connect(
			 SigC::slot(this, &Timeout::Pimpl::timeoutEvent),
			 owner_->timeout_ms
			);
	running_ = true;
}


void Timeout::Pimpl::stop()
{
	conn_.disconnect();
	running_ = false;
}


gint Timeout::Pimpl::timeoutEvent()
{
	owner_->emit();
	return 0; // discontinue emitting timeouts.
}
