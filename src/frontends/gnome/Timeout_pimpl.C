/**
 * \file Timeout_pimpl.C
 * Copyright 2001 LyX Team
 * Read COPYING
 *
 * \author Baruch Even
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
}


void Timeout::Pimpl::stop()
{
	conn_.disconnect();
}


gint Timeout::Pimpl::timeoutEvent()
{
	owner_->emit();
	return 0; // discontinue emitting timeouts.
}
