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
#include "gettext.h"

#include "support/lstrings.h"
#include "support/filetools.h"

#include "frontends/controllers/ControlMath.h"

using std::endl;

ToolbarBackend toolbarbackend;

namespace {

enum tooltags {
	TO_ADD = 1,
	TO_ENDTOOLBAR,
	TO_SEPARATOR,
	TO_LAYOUTS,
	TO_LAST
};


struct keyword_item toolTags[TO_LAST - 1] = {
	{ "end", TO_ENDTOOLBAR },
	{ "item", TO_ADD },
	{ "layouts", TO_LAYOUTS },
	{ "separator", TO_SEPARATOR }
};

} // end of anon namespace


ToolbarBackend::ToolbarBackend()
{
}


void ToolbarBackend::read(LyXLex & lex)
{
	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbar")) {
		lyxerr << "ToolbarBackend::read: ERROR wrong token:`"
		       << lex.getString() << '\'' << endl;
	}

	lex.next(true);

	Toolbar tb;
	tb.name = lex.getString();

	bool quit = false;

	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case TO_ADD:
			if (lex.next(true)) {
				string const tooltip = _(lex.getString());
				lex.next(true);
				string const func = lex.getString();
				lyxerr[Debug::PARSER]
					<< "ToolbarBackend::read TO_ADD func: `"
					<< func << '\'' << endl;
				add(tb, func, tooltip);
			}
			break;

		case TO_SEPARATOR:
			add(tb, SEPARATOR);
			break;

		case TO_LAYOUTS:
			add(tb, LAYOUTS);
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

	toolbars.push_back(tb);

	lex.popTable();
}


void ToolbarBackend::add(Toolbar & tb, int action, string const & tooltip)
{
	tb.items.push_back(make_pair(action, tooltip));
}


void ToolbarBackend::add(Toolbar & tb, string const & func, string const & tooltip)
{
	int const tf = lyxaction.LookupFunc(func);

	if (tf == -1) {
		lyxerr << "ToolbarBackend::add: no LyX command called `"
		       << func << "' exists!" << endl;
	} else {
		add(tb, tf, tooltip);
	}
}


string const ToolbarBackend::getIcon(int action)
{
	string fullname;
	FuncRequest f = lyxaction.retrieveActionArg(action);

	if (f.action == LFUN_INSERT_MATH && !f.argument.empty()) {
		fullname = find_xpm(f.argument.substr(1));
	} else {
		string const name = lyxaction.getActionName(f.action);
		string xpm_name(name);

		if (!f.argument.empty())
			xpm_name = subst(name + ' ' + f.argument, ' ', '_');

		fullname = LibFileSearch("images", xpm_name, "xpm");

		if (fullname.empty()) {
			// try without the argument
			fullname = LibFileSearch("images", name, "xpm");
		}
	}

	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Full icon name is `"
				   << fullname << '\'' << endl;
		return fullname;
	}

	return LibFileSearch("images", "unknown", "xpm");
}
