/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1999 The LyX Team.
*
* ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "debug.h"

struct error_item {
	Debug::type level;
	char const * name;
	char const * desc;
};

static error_item errorTags[] = {
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
	{ Debug::NONE,		"none",		"No debugging message"},
        { Debug::ANY,		"any",          "All debugging messages"}
};


static const int numErrorTags = sizeof(errorTags)/sizeof(error_item);


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
		os << "  " << errorTags[i].level
		   << "  " << errorTags[i].name
		   << "  " << errorTags[i].desc << '\n';
	os.flush();
}



