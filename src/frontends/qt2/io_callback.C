/**
 * \file io_callback.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxserver.h"
#include "io_callback.h"


io_callback::io_callback(int fd, LyXComm * comm)
	: comm_(comm)
{
	sn_.reset(new QSocketNotifier(fd, QSocketNotifier::Read, this));
	connect(sn_.get(), SIGNAL(activated(int)), this, SLOT(data_received()));
}


void io_callback::data_received()
{
	comm_->read_ready();
}
