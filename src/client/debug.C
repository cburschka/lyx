/**
 * \file debug.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "gettext.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <iostream>
#include <iomanip>

using lyx::support::ascii_lowercase;
using lyx::support::bformat;
using lyx::support::isStrInt;

using std::setw;
using std::string;
using std::ostream;

namespace {

struct error_item {
	Debug::type level;
	char const * name;
	char const * desc;
};


error_item errorTags[] = {
	{ Debug::NONE,      "none",      N_("No debugging message")},
	{ Debug::INFO,      "info",      N_("General information")},
	{ Debug::DEBUG,     "debug",     N_("Developers' general debug messages")},
	{ Debug::ANY,       "any",       N_("All debugging messages")}
};


int const numErrorTags = sizeof(errorTags)/sizeof(error_item);

} // namespace anon


lyx_debug_trait::type lyx_debug_trait::value(string const & val)
{
	type l = Debug::NONE;
	string v(val);
	while (!v.empty()) {
		string::size_type st = v.find(',');
		string tmp(ascii_lowercase(v.substr(0, st)));
		if (tmp.empty())
			break;
		// Is it a number?
		if (isStrInt(tmp))
			l |= static_cast<type>(convert<int>(tmp));
		else
		// Search for an explicit name
		for (int i = 0 ; i < numErrorTags ; ++i)
			if (tmp == errorTags[i].name) {
				l |= errorTags[i].level;
				break;
			}
		if (st == string::npos) break;
		v.erase(0, st + 1);
	}
	return l;
}


void lyx_debug_trait::showLevel(ostream & os, lyx_debug_trait::type level)
{
	// Show what features are traced
	for (int i = 0; i < numErrorTags ; ++i) {
		if (errorTags[i].level != Debug::ANY
		    && errorTags[i].level != Debug::NONE
		    && errorTags[i].level & level) {
			// avoid _(...) re-entrance problem
			string const s = _(errorTags[i].desc);
			os << bformat(_("Debugging `%1$s' (%2$s)"),
					errorTags[i].name, s)
			   << '\n';
		}
	}
	os.flush();
}


void lyx_debug_trait::showTags(ostream & os)
{
	for (int i = 0; i < numErrorTags ; ++i)
		os << setw(7) << static_cast<unsigned int>(errorTags[i].level)
		   << setw(10) << errorTags[i].name
		   << "  " << _(errorTags[i].desc) << '\n';
	os.flush();
}

LyXErr lyxerr;
