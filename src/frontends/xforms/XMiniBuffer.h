// -*- C++ -*-
/**
 * \file XMiniBuffer.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 */

#ifndef XMINIBUFFER_H
#define XMINIBUFFER_H

#include "frontends/MiniBuffer.h"

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma interface
#endif

class DropDown;

///
class XMiniBuffer : public MiniBuffer {
public:
	///
	XMiniBuffer(LyXView * o,
		    FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w);

	/// destructor
	~XMiniBuffer();

	/// create drop down
	void dd_init();

	///
	void redraw();

	///
	int peek_event(FL_OBJECT *, int, int, XEvent * xev);

protected:
	/// Are we in editing mode?
	virtual bool isEditingMode() const;
	/// enter editing mode
	virtual void editingMode();
	/// enter message display mode
	virtual void messageMode();

	/**
	 * Append "c" to the current input contents when the completion
	 * list is displayed and has focus.
	 */
	void append_char(char c);

	/// set the minibuffer content in editing mode
	virtual void set_input(string const &);

	/// This creates the input widget for the minibuffer
	FL_OBJECT * create_input_box(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord);

	/// the dropdown menu
	DropDown * dropdown_;

	/// This is the input widget object
	FL_OBJECT * the_buffer;
};
#endif
