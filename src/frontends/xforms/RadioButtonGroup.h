// -*- C++ -*-
/**
 * \file RadioButtonGroup.h
 * Copyright 2002 the LyX Team
 * Copyright 2000 Baruch Even
 * Read the file COPYING
 *
 * \author Baruch Even, baruch.even@writeme.com
 */


#ifndef RADIOBUTTONGROUP_H
#define RADIOBUTTONGROUP_H

#ifdef __GNUG__
#pragma interface
#endif

#include "support/types.h"
#include <vector>
#include <utility>
#include FORMS_H_LOCATION

/** This class simplifies the work with a group of radio buttons,
 * the idea is that you register a bunch of radio buttons with the accompanying
 * value for each radio button and then you get to query or set the active
 * button in a single function call.
 */
class RadioButtonGroup {
public:
	///
	typedef lyx::size_type size_type;

	/// Constructor. Allocate space for 'n' items in the group.
	RadioButtonGroup(unsigned n = 5) : map(n) {};

	/// Register a radio button with it's corresponding value.
	void init(FL_OBJECT * button, size_type value);
	/// Reset registrations.
	void reset();

	// Set the active button.
	void set(size_type value);

	// Get the active button.
	size_type get() const;

private:
	///
	typedef std::pair<FL_OBJECT *, size_type> ButtonValuePair;
	///
	typedef std::vector<ButtonValuePair> ButtonValueMap;
	///
	ButtonValueMap map;
};

#endif
