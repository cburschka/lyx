/**
 * \file ToolbarBackend.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ToolbarBackend.h"
#include "LyXAction.h"
#include "lyxlex.h"
#include "debug.h"
#include "lyxlex.h"
#include "support/lstrings.h"

using std::endl;

ToolbarBackend toolbarbackend;

namespace {

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

} // end of anon namespace


ToolbarBackend::ToolbarBackend()
{
}


void ToolbarBackend::add(int action)
{
	items.push_back(action);
}


void ToolbarBackend::read(LyXLex & lex)
{
	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbar")) {
		lyxerr << "ToolbarBackend::read: ERROR wrong token:`"
		       << lex.getString() << '\'' << endl;
	}

	bool quit = false;

	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case TO_ADD:
			if (lex.next(true)) {
				string const func = lex.getString();
				lyxerr[Debug::PARSER]
					<< "ToolbarBackend::read TO_ADD func: `"
					<< func << '\'' << endl;
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
			quit = true;
			break;
		default:
			lex.printError("ToolbarBackend::read: "
				       "Unknown toolbar tag: `$$Token'");
			break;
		}
	}
	lex.popTable();
}


void ToolbarBackend::add(string const & func)
{
	int const tf = lyxaction.LookupFunc(func);

	if (tf == -1) {
		lyxerr << "ToolbarBackend::add: no LyX command called `"
		       << func << "' exists!" << endl;
	} else {
		add(tf);
	}
}
