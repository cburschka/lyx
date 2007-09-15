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

#include "SocketCallback.h"


namespace lyx {

SocketCallback::SocketCallback(QObject * parent,
		int fd, boost::function<void()> func)
	: QObject(parent), func_(func)
{
	sn_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(sn_, SIGNAL(activated(int)), this, SLOT(dataReceived()));
}


void SocketCallback::dataReceived()
{
	func_();
}

} // namespace lyx

#include "SocketCallback_moc.cpp"
