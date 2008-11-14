// -*- C++ -*-
/**
 * \file SignalSlotPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SIGNALSLOTPRIVATE_H
#define SIGNALSLOTPRIVATE_H

#include <QObject>

namespace lyx {

class SignalImpl : public QObject
{
	Q_OBJECT
public:
	SignalImpl() {}
	virtual ~SignalImpl() {}
	
Q_SIGNALS:
	void fire();
	friend class Signal;
};


class Slot;
class SlotImpl : public QObject
{
	Q_OBJECT
public:
	SlotImpl(Slot & owner) : owner_(owner) {}
	virtual ~SlotImpl() {}
public Q_SLOTS:
	void called();
private:
	Slot & owner_;
};

} // namespace lyx

#endif // SIGNALSLOTPRIVATE_H
