// -*- C++ -*-
/**
 * \file XMiniBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars
 * \author Asger and Juergen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef XMINIBUFFER_H
#define XMINIBUFFER_H

#include "lyx_forms.h"

#include "LString.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals/connection.hpp>

struct fl_freebrowser_;
typedef fl_freebrowser_ FL_FREEBROWSER;

class ControlCommandBuffer;
class Timeout;

/// in xforms, the minibuffer is both a status bar and a command buffer
class XMiniBuffer {
public:
	///
	XMiniBuffer(ControlCommandBuffer & control,
		    FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w);

	///
	~XMiniBuffer();

	/// repaint the minibuffer
	void redraw();

	/// command completion, history, etc. management
	int peek_event(FL_OBJECT *, int, int, XEvent * xev);

	/// show a message
	void message(string const & str);

	/// focus the buffer for editing mode
	void focus() { messageMode(false); }

	/// disable event management
	void freeze();

	/// xforms callback routine
	void freebrowserCB(int action);

private:
	/// Are we in editing mode?
	bool isEditingMode() const;

	/// reset buffer to stored input text
	void info_timeout();

	/// go back to "at rest" message
	void idle_timeout();

	/// set the minibuffer content in editing mode
	void set_input(string const &);

	/// go into message mode
	void messageMode(bool on = true);

	/// show a temporary message whilst in edit mode
	void show_info(string const & info, string const & input, bool append = true);

	/// the dropdown menu
	boost::shared_ptr<FL_FREEBROWSER> freebrowser_;

	/// info timer
	boost::scoped_ptr<Timeout> info_timer_;

	/// idle timer
	boost::scoped_ptr<Timeout> idle_timer_;

	///
	boost::signals::connection info_con;
	///
	boost::signals::connection idle_con;
	///
	boost::signals::connection result_con;
	///
	boost::signals::connection keypress_con;

	/// This is the input widget object
	FL_OBJECT * input_;

	/// the controller we use
	ControlCommandBuffer & controller_;

	/// stored input when showing info
	string stored_input_;

	/// are we showing an informational temporary message ?
	bool info_shown_;
};

#endif // XMINIBUFFER_H
