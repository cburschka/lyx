// -*- C++ -*-
/**
 * \file SignalSlot.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SIGNALSLOT_H
#define SIGNALSLOT_H

namespace lyx {

class Slot;
class SlotImpl;
class SignalImpl;

class Signal
{
public:
	Signal();
	~Signal();
	void fire();
private:
	Signal(Signal const &);
	void operator=(Signal const &);
	SignalImpl * impl;
	friend void connect(Signal & sig, Slot & slot);
};

class Slot
{
public:
	Slot();
	virtual ~Slot();
	virtual void called() {}
private:
	Slot(Slot const &);
	void operator=(Slot const &);
	SlotImpl * impl;
	friend void connect(Signal & sig, Slot & slot);
};

void connect(Signal & sig, Slot & slot);

} // namespace lyx

#endif
