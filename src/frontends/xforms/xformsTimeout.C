/**
 * \file xformsTimeout.C
 * Copyright 2001 LyX Team
 * Read COPYING
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Angus Leeming
 */

#include <config.h>

#include "xformsTimeout.h"
#include "debug.h"

#include FORMS_H_LOCATION


Timeout::Timeout(unsigned int msec, Type t)
	: pimpl_(new xformsTimeout(*this)), type(t), timeout_ms(msec)
{}


namespace {

extern "C"
void C_TimeoutCB(int, void * data)
{
	xformsTimeout * to = static_cast<xformsTimeout *>(data);
	to->emitCB();
}

} // namespace anon


xformsTimeout::xformsTimeout(Timeout & owner)
	: Timeout::Impl(owner), timeout_id(-1)
{}


void xformsTimeout::emitCB()
{
	emit();
}


bool xformsTimeout::running() const
{
	return timeout_id != -1;
}


void xformsTimeout::start()
{
	if (running()) {
		lyxerr << "Timeout::start: already running!" << std::endl;

	} else {
		timeout_id = fl_add_timeout(timeout_ms(),
					    C_TimeoutCB, this);
	}
}


void xformsTimeout::stop()
{
	if (running()) {
		fl_remove_timeout(timeout_id);
		timeout_id = -1;
	}
}


void xformsTimeout::reset()
{
	timeout_id = -1;
}
