#include <config.h>

#include "ToolbarDefaults.h"
#include "LyXAction.h"
#include "toolbar.h"

extern LyXAction lyxaction;


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
	add(Toolbar::TOOL_LAYOUTS);
	add(LFUN_MENUOPEN);
	//add(LFUN_CLOSEBUFFER);
	add(LFUN_MENUWRITE);
	add(LFUN_MENUPRINT);
	add(Toolbar::TOOL_SEPARATOR);

	add(LFUN_CUT);
	add(LFUN_COPY);
	add(LFUN_PASTE);
	add(Toolbar::TOOL_SEPARATOR);
	
	add(LFUN_EMPH);
	add(LFUN_NOUN);
	add(LFUN_FREE);
	add(Toolbar::TOOL_SEPARATOR);
	
	add(LFUN_FOOTMELT);
	add(LFUN_MARGINMELT);
	add(LFUN_DEPTH);
	add(Toolbar::TOOL_SEPARATOR);

	add(LFUN_TEX);
        add(LFUN_MATH_MODE);
	add(Toolbar::TOOL_SEPARATOR);

	add(LFUN_FIGURE);
	add(LFUN_TABLE);
	//add(LFUN_MELT);
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
	{ "\\add", TO_ADD },
	{ "\\end_toolbar", TO_ENDTOOLBAR },
        { "\\layouts", TO_LAYOUTS },
        { "\\newline", TO_NEWLINE },
        { "\\separator", TO_SEPARATOR }
};


void ToolbarDefaults::read(LyXLex & lex) 
{
		//consistency check
	if (lex.GetString() != "\\begin_toolbar")
		lyxerr << "Toolbar::read: ERROR wrong token:`"
		       << lex.GetString() << '\'' << endl;

	defaults.clear();
	
	string func;
	bool quit = false;
	
	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);
	
	while (lex.IsOK() && !quit) {
		
		lyxerr[Debug::TOOLBAR] << "Toolbar::read: current lex text: `"
				       << lex.GetString() << '\'' << endl;

		switch(lex.lex()) {
		case TO_ADD:
			if (lex.EatLine()) {
				func = lex.GetString();
				lyxerr[Debug::TOOLBAR]
					<< "Toolbar::read TO_ADD func: `"
					<< func << "'" << endl;
				add(func);
			}
			break;
		   
		case TO_SEPARATOR:
			add(Toolbar::TOOL_SEPARATOR);
			break;
		   
		case TO_LAYOUTS:
			add(Toolbar::TOOL_LAYOUTS);
			break;
		   
		case TO_NEWLINE:
			add(Toolbar::TOOL_NEWLINE);
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
	int tf = lyxaction.LookupFunc(func);

	if (tf == -1) {
		lyxerr << "Toolbar::add: no LyX command called`"
		       << func << "'exists!" << endl; 
	} else {
		add(tf);
	}
}
