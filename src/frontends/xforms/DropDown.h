/**
 * \file DropDown.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef DROPDOWN_H
#define DROPDOWN_H
 
#include "LyXView.h"
#include <sigc++/signal_system.h> 
#include "LString.h"
#include FORMS_H_LOCATION
#include <vector>
 
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
	SigC::Signal1<void, string const &> result;
	 
	/// signal that a key was pressed
	SigC::Signal1<void, char> keypress;
	 
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
