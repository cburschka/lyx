// -*- C++ -*-
/**
 * \file io_callback.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef IO_CALLBACK_H
#define IO_CALLBACK_H


#include <gtkmm.h>

#include <boost/function.hpp>


/**
 * io_callback - a simple wrapper for asynchronous socket notification
 *
 * This is used by the lyxsocket to notice the socket is ready to be
 * connected/read.
 *
 */
class io_callback : public sigc::trackable {
public:
	/// connect a connection notification from the LyXServerSocket
	io_callback(int fd, boost::function<void()> func);
private:
        bool data_received(Glib::IOCondition);
	/// our notifier
	sigc::connection conn_;
	/// The callback function
	boost::function<void()> func_;
};

#endif // IO_CALLBACK_H
