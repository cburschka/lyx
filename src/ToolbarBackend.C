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

#include <vector>

using namespace lyx::support;

using std::endl;
using std::vector;
using std::make_pair;

ToolbarBackend toolbarbackend;

namespace {

enum tooltags {
	TO_ADD = 1,
	TO_ENDTOOLBAR,
	TO_SEPARATOR,
	TO_LAYOUTS,
	TO_MINIBUFFER,
	TO_LAST
};

struct keyword_item toolTags[TO_LAST - 1] = {
	{ "end", TO_ENDTOOLBAR },
	{ "item", TO_ADD },
	{ "layouts", TO_LAYOUTS },
	{ "minibuffer", TO_MINIBUFFER },
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

		case TO_MINIBUFFER:
			add(tb, MINIBUFFER);
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


void ToolbarBackend::readToolbars(LyXLex & lex)
{
	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbars")) {
		lyxerr << "ToolbarBackend::read: ERROR wrong token:`"
		       << lex.getString() << '\'' << endl;
	}

	lex.next(true);

	while (lex.isOK()) {
		string name = lex.getString();
		lex.next(true);

		if (!compare_ascii_no_case(name, "end"))
			return;

		Toolbars::iterator tcit = toolbars.begin();
		Toolbars::iterator tend = toolbars.end();
		for (; tcit != tend; ++tcit) {
			if (tcit->name == name)
				break;
		}

		if (tcit == tend) {
			lyxerr << "ToolbarBackend: undefined toolbar "
				<< name << endl;
			return;
		}

		tcit->flags = static_cast<Flags>(0);
		string flagstr = lex.getString();
		lex.next(true);
		vector<string> flags = getVectorFromString(flagstr);

		vector<string>::const_iterator cit = flags.begin();
		vector<string>::const_iterator end = flags.end();

		for (; cit != end; ++cit) {
			int flag = 0;
			if (!compare_ascii_no_case(*cit, "off"))
				flag = OFF;
			else if (!compare_ascii_no_case(*cit, "on"))
				flag = ON;
			else if (!compare_ascii_no_case(*cit, "math"))
				flag = MATH;
			else if (!compare_ascii_no_case(*cit, "table"))
				flag = TABLE;
			else if (!compare_ascii_no_case(*cit, "top"))
				flag = TOP;
			else if (!compare_ascii_no_case(*cit, "bottom"))
				flag = BOTTOM;
			else if (!compare_ascii_no_case(*cit, "left"))
				flag = LEFT;
			else if (!compare_ascii_no_case(*cit, "right"))
				flag = RIGHT;
			else {
				lyxerr << "ToolbarBackend::read: unrecognised token:`"
				       << *cit << '\'' << endl;
			}
			tcit->flags = static_cast<Flags>(tcit->flags | flag);
		}

		usedtoolbars.push_back(*tcit);
	}
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

	if (f.action == LFUN_INSERT_MATH) {
		if (!f.argument.empty())
			fullname = find_xpm(f.argument.substr(1));
	} else if (f.action == LFUN_MATH_DELIM) {
		fullname = find_xpm(f.argument);
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

	lyxerr[Debug::GUI] << "Cannot find icon for command \""
			   << lyxaction.getActionName(f.action)
			   << ' ' << f.argument << '"' << endl;
	return LibFileSearch("images", "unknown", "xpm");
}
