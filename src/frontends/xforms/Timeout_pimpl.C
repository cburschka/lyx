/**
 * \file xforms/Timeout_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Timeout_pimpl.h"
#include "debug.h"

using std::endl;

namespace {

extern "C" {

static
void C_intern_timeout_cb(int, void * data)
{
	Timeout * to = static_cast<Timeout *>(data);
	to->emit();
}

}

} // namespace anon


Timeout::Pimpl::Pimpl(Timeout * owner)
	: owner_(owner), timeout_id(-1)
{
}


void Timeout::Pimpl::reset()
{
	timeout_id = -1;
}


bool Timeout::Pimpl::running() const
{
	return timeout_id != -1;
}


void Timeout::Pimpl::start()
{
	if (timeout_id != -1)
		lyxerr << "Timeout::start: already running!" << endl;
	timeout_id = fl_add_timeout(owner_->timeout_ms,
				    C_intern_timeout_cb, owner_);
}


void Timeout::Pimpl::stop()
{
	if (timeout_id != -1) {
		fl_remove_timeout(timeout_id);
		timeout_id = -1;
	}
}
