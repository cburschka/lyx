/**
 * \file xformsTimeout.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "xformsTimeout.h"
#include "debug.h"

#include "lyx_forms.h"

Timeout::Timeout(unsigned int msec, Type t)
	: pimpl_(new lyx::frontend::xformsTimeout(*this)),
	  type(t), timeout_ms(msec)
{}


namespace lyx {
namespace frontend {

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

} // namespace frontend
} // namespace lyx
