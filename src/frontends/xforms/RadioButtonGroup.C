/**
 * \file RadioButtonGroup.C
 * Copyright 1995 Matthias Ettrich.
 * Copyright 2000 Baruch Even
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "RadioButtonGroup.h"
#include FORMS_H_LOCATION

#include "support/LAssert.h"
#include "debug.h" // for lyxerr
#include "support/lyxfunctional.h"

#include <algorithm>
#include <iterator>

using std::find_if;
using std::endl;


void RadioButtonGroup::init(FL_OBJECT * ob, size_type value)
{
	// Object must be a ROUND3DBUTTON (let all radio buttons look the same)
	// and of type RADIO_BUTTON (otherwise it ain't work).
	lyx::Assert(ob && ob->objclass == FL_ROUND3DBUTTON
			&& ob->type == FL_RADIO_BUTTON);

	map.push_back(ButtonValuePair(ob, value));
}


void RadioButtonGroup::set(size_type value)
{
	ButtonValueMap::const_iterator it =
		find_if(map.begin(), map.end(),
			lyx::equal_2nd_in_pair<ButtonValuePair>(value));

	if (it != map.end()) {
		set(it->first);
	} else {
		// We found nothing: report it and do nothing.
		lyxerr << "BUG: Requested value in RadioButtonGroup "
			"doesn't exist" << endl;
	}
}


void RadioButtonGroup::set(FL_OBJECT * ob)
{
	fl_set_button(ob, 1);
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
	// Find the active button.
	ButtonValueMap::const_iterator it =
		find_if(map.begin(), map.end(),
			is_set_button<ButtonValuePair> ());

	if (it != map.end())
		return it->second;

	// We found nothing: report it and return 0
	lyxerr << "BUG: No active radio button found." << endl;
	return 0;
}
