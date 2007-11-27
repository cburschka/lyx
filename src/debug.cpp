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

#include "debug.h"
#include "gettext.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <iostream>
#include <iomanip>

using std::setw;
using std::string;
using std::ostream;


namespace lyx {

using support::ascii_lowercase;
using support::bformat;
using support::isStrInt;

namespace {

struct ErrorItem {
	Debug::type level;
	char const * name;
	char const * desc;
};


ErrorItem errorTags[] = {
	{ Debug::NONE,      "none",      N_("No debugging message")},
	{ Debug::INFO,      "info",      N_("General information")},
	{ Debug::INIT,      "init",      N_("Program initialisation")},
	{ Debug::KEY,       "key",       N_("Keyboard events handling")},
	{ Debug::GUI,       "gui",       N_("GUI handling")},
	{ Debug::PARSER,    "parser",    N_("Lyxlex grammar parser")},
	{ Debug::LYXRC,     "lyxrc",     N_("Configuration files reading")},
	{ Debug::KBMAP,     "kbmap",     N_("Custom keyboard definition")},
	{ Debug::LATEX,     "latex",     N_("LaTeX generation/execution")},
	{ Debug::MATHED,    "mathed",    N_("Math editor")},
	{ Debug::FONT,      "font",      N_("Font handling")},
	{ Debug::TCLASS,    "tclass",    N_("Textclass files reading")},
	{ Debug::LYXVC,     "lyxvc",     N_("Version control")},
	{ Debug::LYXSERVER, "lyxserver", N_("External control interface")},
	{ Debug::ROFF,      "roff",      N_("Keep *roff temporary files")},
	{ Debug::ACTION,    "action",    N_("User commands")},
	{ Debug::LYXLEX,    "lyxlex",    N_("The LyX Lexxer")},
	{ Debug::DEPEND,    "depend",    N_("Dependency information")},
	{ Debug::INSETS,    "insets",    N_("LyX Insets")},
	{ Debug::FILES,     "files",     N_("Files used by LyX")},
	{ Debug::WORKAREA,  "workarea",  N_("Workarea events")},
	{ Debug::INSETTEXT, "insettext", N_("Insettext/tabular messages")},
	{ Debug::GRAPHICS,  "graphics",  N_("Graphics conversion and loading")},
	{ Debug::CHANGES,   "changes",   N_("Change tracking")},
	{ Debug::EXTERNAL,  "external",  N_("External template/inset messages")},
	{ Debug::PAINTING,  "painting",  N_("RowPainter profiling")},
	{ Debug::DEBUG,     "debug",     N_("Developers' general debug messages")},
	{ Debug::ANY,       "any",       N_("All debugging messages")}
};


int const numErrorTags = sizeof(errorTags)/sizeof(errorTags[0]);

} // namespace anon


Debug::type Debug::value(string const & val)
{
	type l = Debug::NONE;
	string v = val;
	while (!v.empty()) {
		size_t const st = v.find(',');
		string const tmp(ascii_lowercase(v.substr(0, st)));
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


void Debug::showLevel(ostream & os, Debug::type level)
{
	// Show what features are traced
	for (int i = 0; i != numErrorTags; ++i) {
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
	for (int i = 0; i != numErrorTags ; ++i)
		os << setw(10) << static_cast<unsigned int>(errorTags[i].level)
		   << setw(13) << errorTags[i].name
		   << "  " << to_utf8(_(errorTags[i].desc)) << '\n';
	os.flush();
}


LyXErr lyxerr;


} // namespace lyx
