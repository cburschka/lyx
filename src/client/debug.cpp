/**
 * \file debug.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"
#include "support/gettext.h"

#include "support/convert.h"
#include "support/lstrings.h"
#include "support/FileName.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

struct ErrorItem {
	Debug::Type level;
	char const * name;
	char const * desc;
};


ErrorItem errorTags[] = {
	{ Debug::NONE,      "none",      N_("No debugging message")},
	{ Debug::INFO,      "info",      N_("General information")},
	{ Debug::DEBUG,     "debug",     N_("Developers' general debug messages")},
	{ Debug::ANY,       "any",       N_("All debugging messages")}
};


int const numErrorTags = sizeof(errorTags)/sizeof(errorTags[0]);

} // namespace anon


Debug::Type Debug::value(string const & val)
{
	Type l = Debug::NONE;
	string v(val);
	while (!v.empty()) {
		string::size_type const st = v.find(',');
		string const tmp(ascii_lowercase(v.substr(0, st)));
		if (tmp.empty())
			break;
		// Is it a number?
		if (isStrInt(tmp))
			l |= static_cast<Type>(convert<int>(tmp));
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


void Debug::showLevel(ostream & os, Debug::Type level)
{
	// Show what features are traced
	for (int i = 0; i < numErrorTags ; ++i) {
		if (errorTags[i].level != Debug::ANY
		    && errorTags[i].level != Debug::NONE
		    && errorTags[i].level & level) {
			// avoid to_utf8(_(...)) re-entrance problem
			docstring const s = _(errorTags[i].desc);
			os << to_utf8(bformat(_("Debugging `%1$s' (%2$s)"),
					from_utf8(errorTags[i].name), s))
			   << '\n';
		}
	}
	os.flush();
}


void Debug::showTags(ostream & os)
{
	for (int i = 0; i < numErrorTags ; ++i)
		os << setw(10) << static_cast<unsigned int>(errorTags[i].level)
		   << setw(13) << errorTags[i].name
		   << "  " << to_utf8(_(errorTags[i].desc)) << '\n';
	os.flush();
}


void LyXErr::disable()
{
	enabled_ = false;
}


void LyXErr::enable()
{
	enabled_ = true;
}


bool LyXErr::debugging(Debug::Type t) const
{
	return (dt & t);
}


void LyXErr::endl()
{
	stream() << endl;
}


LyXErr & operator<<(LyXErr & l, void const * t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, char const * t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, char t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, int t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, unsigned int t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, long t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, unsigned long t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, double t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, string const & t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, docstring const & t)
{ l.stream() << to_utf8(t); return l; }
LyXErr & operator<<(LyXErr & l, FileName const & t)
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, ostream &(*t)(ostream &))
{ l.stream() << t; return l; }
LyXErr & operator<<(LyXErr & l, ios_base &(*t)(ios_base &))
{ l.stream() << t; return l; }

LyXErr lyxerr;


} // namespace lyx
