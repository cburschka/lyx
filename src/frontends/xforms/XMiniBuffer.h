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

#include FORMS_H_LOCATION

#include <boost/scoped_ptr.hpp>
#include <boost/signals/connection.hpp>

#ifdef __GNUG__
#pragma interface
#endif

class DropDown;
class ControlCommandBuffer;
class Timeout;

/// in xforms, the minibuffer is both a status bar and a command buffer
class XMiniBuffer {
public:
	///
	XMiniBuffer(XFormsView * o, ControlCommandBuffer & control,
		    FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w);

	///
	~XMiniBuffer();
	
	/// create drop down
	void dd_init();

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

private:
	/// Are we in editing mode?
	bool isEditingMode() const;

	/// reset buffer to stored input text
	void info_timeout();

	/// go back to "at rest" message
	void idle_timeout();

	/**
	 * Append "c" to the current input contents when the completion
	 * list is displayed and has focus.
	 */
	void append_char(char c);

	/// completion selection callback
	void set_complete_input(string const & str);

	/// set the minibuffer content in editing mode
	void set_input(string const &);

	/// This creates the input widget for the minibuffer
	FL_OBJECT * create_input_box(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord);

	/// go into message mode
	void messageMode(bool on = true);

	/// show a temporary message whilst in edit mode
	void show_info(string const & info, string const & input, bool append = true);

	/// the dropdown menu
	boost::scoped_ptr<DropDown> dropdown_;

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
	FL_OBJECT * the_buffer_;

	/// the input box
	FL_OBJECT * input_obj_;

	/// the controller we use
	ControlCommandBuffer & controller_;

	/// the lyx view
	XFormsView * view_;

	/// stored input when showing info
	string stored_input_;

	/// are we showing an informational temporary message ?
	bool info_shown_;
};

#endif // XMINIBUFFER_H
