/**
 * \file RadioButtonGroup.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "RadioButtonGroup.h"

#include "xforms_helpers.h"

#include "debug.h"

#include "support/lyxfunctional.h"

#include "lyx_forms.h"

#include <boost/assert.hpp>

using std::endl;


void RadioButtonGroup::init(FL_OBJECT * ob, size_type value)
{
	// Object must be a ROUND3DBUTTON (let all radio buttons look the same)
	// and of type RADIO_BUTTON (otherwise it ain't work).
	BOOST_ASSERT(ob && ob->objclass == FL_ROUND3DBUTTON
			&& ob->type == FL_RADIO_BUTTON);

	map.push_back(ButtonValuePair(ob, value));
}


void RadioButtonGroup::set(size_type value) const
{
	ButtonValueMap::const_iterator it =
		find_if(map.begin(), map.end(),
			lyx::equal_2nd_in_pair<ButtonValuePair>(value));

	if (it != map.end()) {
		fl_set_button(it->first, 1);
	} else {
		// We found nothing: report it and do nothing.
		lyxerr << "BUG: Requested value in RadioButtonGroup "
			"doesn't exist" << endl;
	}
}


void RadioButtonGroup::set(FL_OBJECT * ob) const
{
	// Object must be member of the radiobutton group.
	bool isMember = false;
	ButtonValueMap::const_iterator it = map.begin();
	for (; it != map.end() && !isMember; ++it) {
		isMember = it->first == ob;
	}

	if (isMember) {
		fl_set_button(ob, 1);
	} else {
		// Object is not a member; report it and do nothing.
		lyxerr << "BUG: Requested object is not a member of "
			<< "the RadioButtonGroup." << endl;
	}
}


template <typename T>
struct is_set_button : public std::unary_function<T, bool> {
	bool operator() (T const & item) const
	{
		return fl_get_button((item).first);
	}
};


void RadioButtonGroup::unset() const
{
	// Find the active button.
	ButtonValueMap::const_iterator it =
		find_if(map.begin(), map.end(),
			is_set_button<ButtonValuePair> ());

	if (it == map.end())
		// Nothing to do. No button is set.
		return;

	fl_set_button(it->first, 0);
}


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


void RadioButtonGroup::setEnabled(bool enable)
{
	ButtonValueMap::iterator it  = map.begin();
	ButtonValueMap::iterator end = map.end();
	for (; it != end; ++it) {
		::setEnabled(it->first, enable);
	}
}
