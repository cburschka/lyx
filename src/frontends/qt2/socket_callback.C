/**
 * \file io_callback.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "lyxsocket.h"
#include "socket_callback.h"


socket_callback::socket_callback(LyXServerSocket * server)
	: server_(server)
{
	sn_.reset(new QSocketNotifier(server->fd(), QSocketNotifier::Read, this));
	connect(sn_.get(), SIGNAL(activated(int)), this, SLOT(server_received()));
}

socket_callback::socket_callback(LyXDataSocket * data)
	: data_(data)
{
	sn_.reset(new QSocketNotifier(data->fd(), QSocketNotifier::Read, this));
	connect(sn_.get(), SIGNAL(activated(int)), this, SLOT(data_received()));
}


void socket_callback::server_received()
{
	server_->serverCallback();
}

void socket_callback::data_received()
{
	data_->server()->dataCallback(data_);
}
