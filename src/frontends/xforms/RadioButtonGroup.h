// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */


#ifndef RADIOBUTTONGROUP_H
#define RADIOBUTTONGROUP_H

#ifdef __GNUG__
#pragma interface
#endif 

#include <vector>
#include <utility>

using std::vector;
using std::pair;

#include FORMS_H_LOCATION

/** This class simplifies the work with a group of radio buttons,
 * the idea is that you register a bunch of radio buttons with the accompanying
 * value for each radio button and then you get to query or set the active
 * button in a single function call.
 * @author Baruch Even
 */
class RadioButtonGroup {
public:
	/// Constructor. Allocate space for 'n' items in the group.
	RadioButtonGroup(unsigned n = 5) : map(n) {};

	/// Register a radio button with it's corresponding value.
	void registerRadioButton(FL_OBJECT *button, int value);
	/// Reset registrations.
	void reset();

	// Set the active button.
	void setButton(int value);

	// Get the active button.
	int getButton();

private:
	///
	typedef pair<FL_OBJECT *, int> ButtonValuePair;
	///
	typedef vector<ButtonValuePair> ButtonValueMap;
	///
	ButtonValueMap map;
};

#endif 
