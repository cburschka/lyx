/**
 * \file qt2/Timeout_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Timeout_pimpl.h"
#include "debug.h"

using std::endl;

Timeout::Pimpl::Pimpl(Timeout * owner)
	: owner_(owner), timeout_id(-1)
{
}


void Timeout::Pimpl::timerEvent(QTimerEvent *)
{
	owner_->emit();
}


void Timeout::Pimpl::reset()
{
	killTimers();
	timeout_id = -1;
}


bool Timeout::Pimpl::running() const
{
	return timeout_id != -1;
}


void Timeout::Pimpl::start()
{
	if (running())
		lyxerr << "Timeout::start: already running!" << endl;
	timeout_id = startTimer(owner_->timeout_ms);
}


void Timeout::Pimpl::stop()
{
	if (running())
		reset();
}
