/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright 1999-2000 The LyX Team.
*
* ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include <iomanip>

#include "debug.h"
#include "gettext.h"

using std::ostream;
using std::setw;
using std::endl;

struct error_item {
	Debug::type level;
	char const * name;
	char const * desc;
};

static error_item errorTags[] = {
	{ Debug::NONE,      "none",      N_("No debugging message")},
	{ Debug::INFO,      "info",      N_("General information")},
	{ Debug::INIT,      "init",      N_("Program initialisation")},
	{ Debug::KEY,       "key",       N_("Keyboard events handling")},
	{ Debug::GUI,       "gui",       N_("GUI handling")},
	{ Debug::PARSER,    "parser",    N_("Lyxlex grammer parser")},
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
        { Debug::ANY,       "any",       N_("All debugging messages")}
};


static const int numErrorTags = sizeof(errorTags)/sizeof(error_item);

	
Debug::type const Debug::ANY = Debug::type(
	Debug::INFO | Debug::INIT | Debug::KEY | Debug::GUI |
	Debug::PARSER | Debug::LYXRC | Debug::KBMAP | Debug::LATEX |
	Debug::MATHED | Debug::FONT | Debug::TCLASS | Debug::LYXVC |
	Debug::LYXSERVER | Debug::ROFF | Debug::ACTION | Debug::LYXLEX |
	Debug::DEPEND | Debug::INSETS | Debug::FILES);


Debug::type Debug::value(string const & val) 
{
	type l = Debug::NONE;
	string v(val);
	while (!v.empty()) {
		string::size_type st = v.find(',');
		string tmp(lowercase(v.substr(0, st)));
		if (tmp.empty())
			break;
		// Is it a number?
		if (isStrInt(tmp)) 
			l |= static_cast<type>(strToInt(tmp));
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


void Debug::showLevel(ostream & o, Debug::type level)
{
	// Show what features are traced
	for (int i = 0 ; i < numErrorTags ; ++i)
		if (errorTags[i].level != Debug::ANY
		    && errorTags[i].level != Debug::NONE
		    && errorTags[i].level & level)
			o << _("Debugging `") << errorTags[i].name
			  << "' (" << _(errorTags[i].desc) << ')' << endl;
}


void Debug::showTags(ostream & os) 
{
	for (int i = 0 ; i < numErrorTags ; ++i)
		os << setw(7) << errorTags[i].level
		   << setw(10) << errorTags[i].name
		   << "  " << _(errorTags[i].desc) << '\n';
	os.flush();
}
