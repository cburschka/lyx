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

#include "socket_callback.h"
#include "qt_helpers.h"

namespace lyx {

socket_callback::socket_callback(int fd, boost::function<void()> func)
	: func_(func)
{
	sn_.reset(new QSocketNotifier(fd, QSocketNotifier::Read, this));
	Q_CONNECT_1(QSocketNotifier, sn_.get(), activated, int,
				socket_callback, this, data_received, void);
}


void socket_callback::data_received()
{
	func_();
}

} // namespace lyx

#include "socket_callback_moc.cpp"
