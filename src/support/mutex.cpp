/**
 * \file mutex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/mutex.h"

#include <QMutex>


namespace lyx {


struct Mutex::Private
{
#if QT_VERSION < 0x060000
	// QMutex::Recursive: less risks for dead-locks
	Private() : qmutex_(QMutex::Recursive)
	{
	}

	QMutex qmutex_;
#else
	Private() : qmutex_()
	{
	}

	QRecursiveMutex qmutex_;
#endif
};


Mutex::Mutex() : d(new Private)
{
}


Mutex::~Mutex()
{
	delete d;
}


Mutex::Locker::Locker(Mutex* mtx) : mutex_(mtx)
{
	mutex_->d->qmutex_.lock();
}


Mutex::Locker::~Locker()
{
	mutex_->d->qmutex_.unlock();
}



} // namespace lyx
