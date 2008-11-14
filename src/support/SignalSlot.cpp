// -*- C++ -*-
/**
 * \file SignalSlot.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/SignalSlot.h"
#include "support/SignalSlotPrivate.h"

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// Signal
//
/////////////////////////////////////////////////////////////////////

Signal::Signal()
	: impl(new SignalImpl)
{}


Signal::~Signal()
{
	delete impl;
}


void Signal::fire()
{
	impl->fire();
}


/////////////////////////////////////////////////////////////////////
//
// Slot
//
/////////////////////////////////////////////////////////////////////

void SlotImpl::called()
{
	owner_.called();
}

Slot::Slot()
	: impl(new SlotImpl(*this))
{}

Slot::~Slot()
{
	delete impl;
}


/////////////////////////////////////////////////////////////////////
//
// Connect
//
/////////////////////////////////////////////////////////////////////

void connect(Signal & sig, Slot & slot)
{
	QObject::connect(sig.impl, SIGNAL(fire()), slot.impl, SLOT(called()));
}

} // namespace lyx
