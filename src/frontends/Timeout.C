/**
 * \file Timeout.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */
#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "Timeout.h"
#include "debug.h"

#include "Timeout_pimpl.h"


Timeout::Timeout(unsigned int msec, Type t)
	: type(t), timeout_ms(msec)
{
	pimpl_ = new Pimpl(this);
}


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
	return * this;
}


Timeout & Timeout::setTimeout(unsigned int msec)
{
	timeout_ms = msec;
	return * this;
}
