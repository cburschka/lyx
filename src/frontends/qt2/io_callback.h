/**
 * \file io_callback.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef IO_CALLBACK_H
#define IO_CALLBACK_H
 
#include <config.h>

#include "lyxserver.h"
 
#include <qsocketnotifier.h>

class io_callback : public QObject {
	Q_OBJECT 
public:
	io_callback(int fd, LyXComm * comm)
		: comm_(comm) {
		QSocketNotifier * sn = new QSocketNotifier(fd,
			QSocketNotifier::Read, this);
		connect(sn, SIGNAL(activated(int)), this, SLOT(data_received()));
	}

public slots:
	void data_received() {
		comm_->read_ready();
	}

private:
	LyXComm * comm_;
};

#endif // IO_CALLBACK_H 
