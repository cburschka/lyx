// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#include <config.h> 

#include <functional>
#include <algorithm>
#include <iterator>

#ifdef __GNUG__
#pragma implementation
#endif 

#include "RadioButtonGroup.h"

#include "debug.h" // for lyxerr

using std::find_if;
using std::bind2nd;
using std::endl;

void RadioButtonGroup::registerRadioButton(FL_OBJECT *button, int value)
{
	map.push_back( ButtonValuePair(button, value) );
}


void RadioButtonGroup::reset()
{
	map.clear();
}

// Functor to help us in our work, we should try to find how to achieve
// this with only STL predicates, but its easier to write this than to
// dig. If you can find the equivalent STL predicate combination, let me
// know.
//
// The idea is to take a pair and a value and return true when the second
// element in the pair equals the value.
template < typename T >
struct equal_to_second_in_pair
{
	typedef bool result_type;
	typedef T	first_argument_type;
	typedef typename T::second_type second_argument_type;

	bool operator() (
	    pair < typename T::first_type, typename T::second_type > const & left,
	    typename T::second_type const & right) const
	{
		return left.second == right;
	}
};

void RadioButtonGroup::setButton(int value)
{
	ButtonValueMap::const_iterator it =
	    find_if(map.begin(), map.end(),
	            bind2nd(equal_to_second_in_pair < ButtonValuePair > (),
	                    value));

	// If we found nothing, report it and return
	if (it == map.end()) {
		lyxerr << "BUG: Requested value in RadioButtonGroup doesn't exists"
		<< endl;
	}
	else {
		fl_set_button((*it).first, 1);
	}

}

template < typename T >
struct is_set_button {
	bool operator() (T const & item) const
	{
		return fl_get_button( (item).first );
	}
};

int RadioButtonGroup::getButton()
{
	// Find the first button that is active
	ButtonValueMap::iterator it =
	    find_if(map.begin(), map.end(),
	            is_set_button < ButtonValuePair > () );

	// If such a button was found, return its value.
	if (it != map.end()) {
		return (*it).second;
	}

	lyxerr << "BUG: No radio button found to be active." << endl;

	// Else return 0.
	return 0;
}

