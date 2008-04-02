/**
 * \file ToolbarBackend.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ToolbarBackend.h"
#include "FuncRequest.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "support/lstrings.h"

#include "support/debug.h"
#include "support/gettext.h"

#include <boost/bind.hpp>

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

class ToolbarNamesEqual
{
public:
	ToolbarNamesEqual(string const & name) : name_(name) {}
	bool operator()(ToolbarInfo const & tbinfo) const
	{
		return tbinfo.name == name_;
	}
private:
	string name_;
};

} // namespace anon


ToolbarBackend toolbarbackend;


ToolbarItem::ToolbarItem(Type type, FuncRequest const & func, docstring const & label)
	: type_(type), func_(func), label_(label)
{
}


ToolbarItem::ToolbarItem(Type type, string const & name, docstring const & label)
	: type_(type), label_(label), name_(name)
{
}


ToolbarItem::~ToolbarItem()
{}


void ToolbarInfo::add(ToolbarItem const & item)
{
	items.push_back(item);
	items.back().func_.origin = FuncRequest::TOOLBAR;
}


ToolbarInfo & ToolbarInfo::read(Lexer & lex)
{
	enum tooltags {
		TO_COMMAND = 1,
		TO_ENDTOOLBAR,
		TO_SEPARATOR,
		TO_LAYOUTS,
		TO_MINIBUFFER,
		TO_TABLEINSERT,
		TO_POPUPMENU,
		TO_ICONPALETTE,
		TO_LAST
	};

	struct keyword_item toolTags[TO_LAST - 1] = {
		{ "end", TO_ENDTOOLBAR },
		{ "iconpalette", TO_ICONPALETTE },
		{ "item", TO_COMMAND },
		{ "layouts", TO_LAYOUTS },
		{ "minibuffer", TO_MINIBUFFER },
		{ "popupmenu", TO_POPUPMENU },
		{ "separator", TO_SEPARATOR },
		{ "tableinsert", TO_TABLEINSERT }
	};

	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbar")) {
		LYXERR0("ToolbarInfo::read: ERROR wrong token:`"
		       << lex.getString() << '\'');
	}

	lex.next(true);
	name = lex.getString();

	lex.next(true);
	gui_name = lex.getString();

	// FIXME what to do here?
	if (!lex) {
		LYXERR0("ToolbarInfo::read: Malformed toolbar "
			"description " <<  lex.getString());
		return *this;
	}

	bool quit = false;

	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case TO_COMMAND:
			if (lex.next(true)) {
				docstring const tooltip = translateIfPossible(lex.getDocString());
				lex.next(true);
				string const func_arg = lex.getString();
				LYXERR(Debug::PARSER, "ToolbarInfo::read TO_COMMAND func: `"
					<< func_arg << '\'');

				FuncRequest func =
					lyxaction.lookupFunc(func_arg);
				add(ToolbarItem(ToolbarItem::COMMAND, func, tooltip));
			}
			break;

		case TO_MINIBUFFER:
			add(ToolbarItem(ToolbarItem::MINIBUFFER,
				FuncRequest(FuncCode(ToolbarItem::MINIBUFFER))));
			break;

		case TO_SEPARATOR:
			add(ToolbarItem(ToolbarItem::SEPARATOR,
				FuncRequest(FuncCode(ToolbarItem::SEPARATOR))));
			break;

		case TO_POPUPMENU:
			if (lex.next(true)) {
				string const name = lex.getString();
				lex.next(true);
				docstring const label = lex.getDocString();
				add(ToolbarItem(ToolbarItem::POPUPMENU, name, label));
			}
			break;

		case TO_ICONPALETTE:
			if (lex.next(true)) {
				string const name = lex.getString();
				lex.next(true);
				docstring const label = lex.getDocString();
				add(ToolbarItem(ToolbarItem::ICONPALETTE, name, label));
			}
			break;

		case TO_LAYOUTS:
			add(ToolbarItem(ToolbarItem::LAYOUTS,
				FuncRequest(FuncCode(ToolbarItem::LAYOUTS))));
			break;

		case TO_TABLEINSERT:
			if (lex.next(true)) {
				docstring const tooltip = lex.getDocString();
				add(ToolbarItem(ToolbarItem::TABLEINSERT,
					FuncRequest(FuncCode(ToolbarItem::TABLEINSERT)), tooltip));
			}
			break;

		case TO_ENDTOOLBAR:
			quit = true;
			break;

		default:
			lex.printError("ToolbarInfo::read: "
				       "Unknown toolbar tag: `$$Token'");
			break;
		}
	}

	lex.popTable();
	return *this;
}




ToolbarBackend::ToolbarBackend()
{
	fullScreenWindows = 0;
}


void ToolbarBackend::readToolbars(Lexer & lex)
{
	enum tooltags {
		TO_TOOLBAR = 1,
		TO_ENDTOOLBARSET,
		TO_LAST
	};

	struct keyword_item toolTags[TO_LAST - 1] = {
		{ "end", TO_ENDTOOLBARSET },
		{ "toolbar", TO_TOOLBAR }
	};

	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbarset")) {
		LYXERR0("ToolbarBackend::readToolbars: ERROR wrong token:`"
		       << lex.getString() << '\'');
	}

	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;
	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case TO_TOOLBAR: {
			ToolbarInfo tbinfo;
			tbinfo.read(lex);
			toolbars.push_back(tbinfo);
			break;
			}
		case TO_ENDTOOLBARSET:
			quit = true;
			break;
		default:
			lex.printError("ToolbarBackend::readToolbars: "
				       "Unknown toolbar tag: `$$Token'");
			break;
		}
	}

	lex.popTable();
}


void ToolbarBackend::readToolbarSettings(Lexer & lex)
{
	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbars")) {
		LYXERR0("ToolbarBackend::readToolbarSettings: ERROR wrong token:`"
		       << lex.getString() << '\'');
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
			LYXERR0("ToolbarBackend: undefined toolbar " << name);
			return;
		}

		tcit->flags = static_cast<ToolbarInfo::Flags>(0);
		string flagstr = lex.getString();
		lex.next(true);
		vector<string> flags = getVectorFromString(flagstr);

		vector<string>::const_iterator cit = flags.begin();
		vector<string>::const_iterator end = flags.end();

		for (; cit != end; ++cit) {
			int flag = 0;
			if (!compare_ascii_no_case(*cit, "off"))
				flag = ToolbarInfo::OFF;
			else if (!compare_ascii_no_case(*cit, "on"))
				flag = ToolbarInfo::ON;
			else if (!compare_ascii_no_case(*cit, "math"))
				flag = ToolbarInfo::MATH;
			else if (!compare_ascii_no_case(*cit, "table"))
				flag = ToolbarInfo::TABLE;
			else if (!compare_ascii_no_case(*cit, "mathmacrotemplate"))
				flag = ToolbarInfo::MATHMACROTEMPLATE;
			else if (!compare_ascii_no_case(*cit, "review"))
				flag = ToolbarInfo::REVIEW;
			else if (!compare_ascii_no_case(*cit, "top"))
				flag = ToolbarInfo::TOP;
			else if (!compare_ascii_no_case(*cit, "bottom"))
				flag = ToolbarInfo::BOTTOM;
			else if (!compare_ascii_no_case(*cit, "left"))
				flag = ToolbarInfo::LEFT;
			else if (!compare_ascii_no_case(*cit, "right"))
				flag = ToolbarInfo::RIGHT;
			else if (!compare_ascii_no_case(*cit, "auto"))
				flag = ToolbarInfo::AUTO;
			else {
				LYXERR(Debug::ANY,
					"ToolbarBackend::readToolbarSettings: unrecognised token:`"
					<< *cit << '\'');
			}
			tcit->flags = static_cast<ToolbarInfo::Flags>(tcit->flags | flag);
		}

		usedtoolbars.push_back(*tcit);
	}
}


ToolbarInfo const * ToolbarBackend::getDefinedToolbarInfo(string const & name) const
{
	Toolbars::const_iterator it = find_if(toolbars.begin(), toolbars.end(), ToolbarNamesEqual(name));
	if (it == toolbars.end())
		return 0;
	return &(*it);
}


ToolbarInfo * ToolbarBackend::getUsedToolbarInfo(string const &name)
{
	Toolbars::iterator it = find_if(usedtoolbars.begin(), usedtoolbars.end(), ToolbarNamesEqual(name));
	if (it == usedtoolbars.end())
		return 0;
	return &(*it);
}

} // namespace lyx
