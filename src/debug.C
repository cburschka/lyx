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
#include "debug.h"

#include <iomanip>

using std::ostream;
using std::setw;
using std::endl;

struct error_item {
	Debug::type level;
	char const * name;
	char const * desc;
};

static error_item errorTags[] = {
	{ Debug::NONE,		"none",		"No debugging message"},
	{ Debug::INFO,		"info",		"General information"},
	{ Debug::INIT,		"init",		"Program initialisation"},
	{ Debug::KEY,		"key",		"Keyboard events handling"},
	{ Debug::TOOLBAR,	"toolbar",	"Toolbar handling"},
	{ Debug::PARSER,	"parser",	"Lyxlex grammer parser"},
	{ Debug::LYXRC,		"lyxrc",	"Configuration files reading"},
	{ Debug::KBMAP,		"kbmap",	"Custom keyboard definition"},
	{ Debug::LATEX,		"latex",	"LaTeX generation/execution"},
	{ Debug::MATHED,	"mathed",	"Math editor"},
	{ Debug::FONT,		"font",		"Font handling"},
	{ Debug::TCLASS,	"tclass",	"Textclass files reading"},
	{ Debug::LYXVC,		"lyxvc",	"Version control"},
	{ Debug::LYXSERVER,	"lyxserver",	"External control interface"},
	{ Debug::ROFF,		"roff",		"Keep *roff temporary files"},
	{ Debug::ACTION,	"action",	"User commands"},
	{ Debug::LYXLEX,        "lyxlex",       "The LyX Lexxer"},
	{ Debug::DEPEND,        "depend",       "Dependency information"},
	{ Debug::INSETS,	"insets",	"LyX Insets"},
        { Debug::ANY,		"any",          "All debugging messages"}
};


static const int numErrorTags = sizeof(errorTags)/sizeof(error_item);

       
Debug::type const Debug::ANY = Debug::type(
       Debug::INFO | Debug::INIT | Debug::KEY |
       Debug::PARSER | Debug::LYXRC | Debug::KBMAP | Debug::LATEX |
       Debug::MATHED | Debug::FONT | Debug::TCLASS | Debug::LYXVC |
       Debug::LYXSERVER | Debug::ROFF | Debug::ACTION | Debug::LYXLEX |
       Debug::DEPEND | Debug::INSETS);

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
			o << "Debugging `" << errorTags[i].name
			  << "' (" << errorTags[i].desc << ')' << endl;
}


void Debug::showTags(ostream & os) 
{
	for (int i = 0 ; i < numErrorTags ; ++i)
		os << setw(7) << errorTags[i].level
		   << setw(10) << errorTags[i].name
		   << "  " << errorTags[i].desc << '\n';
	os.flush();
}
