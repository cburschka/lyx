// -*- C++ -*-
/**
 * \file io_callback.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef IO_CALLBACK_H
#define IO_CALLBACK_H

#ifdef __GNUG__
#pragma interface
#endif

#include <qobject.h>
#include <qsocketnotifier.h>
#include <boost/scoped_ptr.hpp>

class LyXComm;

/**
 * io_callback - a simple wrapper for asynchronous pipe notification
 *
 * This is used by the lyxserver to notice the pipe is ready to be
 * read.
 *
 * FIXME: this code apparently will not work on Windows.
 */
class io_callback : public QObject {
	Q_OBJECT
public:
	/// connect a read ready notification for fd to the LyXComm
	io_callback(int fd, LyXComm * comm);
public slots:
	void data_received();
private:
	/// our notifier
	boost::scoped_ptr<QSocketNotifier> sn_;

	LyXComm * comm_;
};

#endif // IO_CALLBACK_H
