/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ToolbarDefaults.h"
#include "LyXAction.h"
#include "lyxlex.h"
#include "debug.h"
#include "lyxlex.h"

using std::endl;

extern LyXAction lyxaction;
ToolbarDefaults toolbardefaults;


ToolbarDefaults::ToolbarDefaults()
{
	init();
}


void ToolbarDefaults::add(int action)
{
	defaults.push_back(action);
}


void ToolbarDefaults::init() 
{
	add(LAYOUTS);
	add(LFUN_FILE_OPEN);
	//add(LFUN_CLOSEBUFFER);
	add(LFUN_MENUWRITE);
	add(LFUN_MENUPRINT);
	add(SEPARATOR);

	add(LFUN_CUT);
	add(LFUN_COPY);
	add(LFUN_PASTE);
	add(SEPARATOR);
	
	add(LFUN_EMPH);
	add(LFUN_NOUN);
	add(LFUN_FREE);
	add(SEPARATOR);
	
	add(LFUN_INSET_FOOTNOTE);
	add(LFUN_INSET_MARGINAL);

	add(LFUN_DEPTH);
	add(SEPARATOR);

	add(LFUN_TEX);
        add(LFUN_MATH_MODE);
	add(SEPARATOR);

	add(LFUN_FIGURE);
	add(LFUN_DIALOG_TABULAR_INSERT);
}


enum _tooltags {
	TO_ADD = 1,
	TO_ENDTOOLBAR,
        TO_SEPARATOR,
        TO_LAYOUTS,
        TO_NEWLINE,
	TO_LAST
};


struct keyword_item toolTags[TO_LAST - 1] = {
	{ "end", TO_ENDTOOLBAR },
	{ "icon", TO_ADD },
        { "layouts", TO_LAYOUTS },
        { "newline", TO_NEWLINE },
        { "separator", TO_SEPARATOR }
};


void ToolbarDefaults::read(LyXLex & lex) 
{
	//consistency check
	if (compare_no_case(lex.GetString(), "toolbar"))
		lyxerr << "Toolbar::read: ERROR wrong token:`"
		       << lex.GetString() << '\'' << endl;

	defaults.clear();
	
	string func;
	bool quit = false;
	
	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.IsOK() && !quit) {
		switch (lex.lex()) {
		case TO_ADD:
			if (lex.next()) {
				func = lex.GetString();
				lyxerr[Debug::PARSER]
					<< "Toolbar::read TO_ADD func: `"
					<< func << "'" << endl;
				add(func);
			}
			break;
		   
		case TO_SEPARATOR:
			add(SEPARATOR);
			break;
		   
		case TO_LAYOUTS:
			add(LAYOUTS);
			break;
		   
		case TO_NEWLINE:
			add(NEWLINE);
			break;
			
		case TO_ENDTOOLBAR:
			// should not set automatically
			//set();
			quit = true;
			break;
		default:
			lex.printError("Toolbar::read: "
				       "Unknown toolbar tag: `$$Token'");
			break;
		}
	}
	lex.popTable();
}


void ToolbarDefaults::add(string const & func)
{
	int const tf = lyxaction.LookupFunc(func);

	if (tf == -1) {
		lyxerr << "Toolbar::add: no LyX command called`"
		       << func << "'exists!" << endl; 
	} else {
		add(tf);
	}
}
