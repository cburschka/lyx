/**
 * \file Tooltips.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

/* Tooltips for xforms. xforms 0.89 supports them directly, but 0.88 needs
 * a bit of jiggery pokery. This class wraps it all up in a neat interface.
 * Based on code originally in Toolbar_pimpl.C that appears to have been
 * written by Matthias Ettrich and Jean-Marc Lasgouttes.
 */

#include <config.h>

#include "Tooltips.h"

#include "xforms_helpers.h" // formatted

#include "support/lstrings.h"

#include "lyx_forms.h"

#include <boost/bind.hpp>

using lyx::support::trim;

using std::string;


bool Tooltips::enabled_ = true;

boost::signal0<void> Tooltips::toggled;


Tooltips::Tooltips()
{
	toggled.connect(boost::bind(&Tooltips::set, this));
}


void Tooltips::toggleEnabled()
{
	enabled_ = !enabled_;
	toggled();
}


void Tooltips::set()
{
	if (tooltipsMap.empty())
		return;

	TooltipsMap::const_iterator it  = tooltipsMap.begin();
	TooltipsMap::const_iterator end = tooltipsMap.end();
	for (; it != end; ++it) {
		FL_OBJECT * const ob = it->first;
		char const * const c_str = enabled_ ? it->second.c_str() : 0;
		fl_set_object_helper(ob, c_str);
	}
}


void Tooltips::init(FL_OBJECT * ob, string const & tip)
{
	BOOST_ASSERT(ob && ob->form);

	// Store the tooltip string
	string const str = formatted(trim(tip), 400);
	tooltipsMap[ob] = str;

	// Set the tooltip
	char const * const c_str = enabled_ ? str.c_str() : 0;
	fl_set_object_helper(ob, c_str);
}
