// -*- C++ -*-
/**
 * \file DropDown.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifndef DROPDOWN_H
#define DROPDOWN_H

#ifdef __GNUG__
#pragma interface
#endif

#include "forms_fwd.h"
#include <X11/Xlib.h>

#include <boost/signals/signal1.hpp>

#include "LString.h"
#include <vector>

class LyXView;

class DropDown {
public:
	/// constructor
	DropDown(LyXView * lv, FL_OBJECT * ob);
	/// destructor
	~DropDown();

	/// choose from the list of choices.
	void select(std::vector<string> const & choices, int x, int y, int w);

	/// user completed action
	void completed();

	/// a key was pressed. Act on it.
	void key_pressed(char c);

	/// signal for completion
	boost::signal1<void, string const &> result;

	/// signal that a key was pressed
	boost::signal1<void, char> keypress;

	/// X event
	int peek(XEvent *);

private:
	/// move up a browser line
	void line_up();

	/// move down a browser line
	void line_down();

	/// owning lyxview
	LyXView * lv_;

	/// our form
	FL_FORM * form_;

	/// the browser
	FL_OBJECT * browser_;
};

#endif // DROPDOWN_H
