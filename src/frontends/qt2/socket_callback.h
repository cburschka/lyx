// -*- C++ -*-
/**
 * \file io_callback.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SOCKET_CALLBACK_H
#define SOCKET_CALLBACK_H


#include <qobject.h>
#include <qsocketnotifier.h>
#include <boost/scoped_ptr.hpp>

class LyXServerSocket;
class LyXDataSocket;

/**
 * socket_callback - a simple wrapper for asynchronous socket notification
 *
 * This is used by the lyxsocket to notice the socket is ready to be
 * connected/read.
 *
 * FIXME: this code apparently will not work on Windows.
 */
class socket_callback : public QObject {
	Q_OBJECT
public:
	/// connect a connection notification from the LyXServerSocket
	socket_callback(LyXServerSocket * server);
	socket_callback(LyXDataSocket * data);
public slots:
	void server_received();
	void data_received();
private:
	/// our notifier
	boost::scoped_ptr<QSocketNotifier> sn_;

	LyXServerSocket * server_;
	LyXDataSocket * data_;
};

#endif // SOCKET_CALLBACK_H
