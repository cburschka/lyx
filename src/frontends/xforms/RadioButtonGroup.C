/**
 * \file RadioButtonGroup.C
 * Copyright 1995 Matthias Ettrich.
 * Copyright 2000 Baruch Even
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "RadioButtonGroup.h"
#include FORMS_H_LOCATION

#include "debug.h" // for lyxerr
#include "support/lyxfunctional.h"

//#include <functional>
#include <algorithm>
#include <iterator>

using std::find_if;
//using std::bind2nd;
using std::endl;


void RadioButtonGroup::init(FL_OBJECT *button, size_type value)
{
	map.push_back(ButtonValuePair(button, value));
}


void RadioButtonGroup::reset()
{
	map.clear();
}


void RadioButtonGroup::set(size_type value)
{
	ButtonValueMap::const_iterator it =
		find_if(map.begin(), map.end(),
			lyx::equal_2nd_in_pair<ButtonValuePair>(value));

	// If we found nothing, report it and return
	if (it == map.end()) {
		lyxerr << "BUG: Requested value in RadioButtonGroup doesn't exists"
		       << endl;
	}
	else {
		fl_set_button(it->first, 1);
	}

}


template < typename T >
struct is_set_button {
	bool operator() (T const & item) const
	{
		return fl_get_button((item).first);
	}
};


RadioButtonGroup::size_type RadioButtonGroup::get() const
{
	// Find the first button that is active
	ButtonValueMap::const_iterator it =
		find_if(map.begin(), map.end(),
			is_set_button<ButtonValuePair> ());

	// If such a button was found, return its value.
	if (it != map.end()) {
		return it->second;
	}

	lyxerr << "BUG: No radio button found to be active." << endl;

	// Else return 0.
	return 0;
}
