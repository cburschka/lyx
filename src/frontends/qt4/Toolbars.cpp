/**
 * \file Toolbars.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Toolbars.h"
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
namespace frontend {

namespace {

} // namespace anon


/////////////////////////////////////////////////////////////////////////
//
// ToolbarItem
//
/////////////////////////////////////////////////////////////////////////

ToolbarItem::ToolbarItem(Type type, FuncRequest const & func, docstring const & label)
	: type_(type), func_(func), label_(label)
{
}


ToolbarItem::ToolbarItem(Type type, string const & name, docstring const & label)
	: type_(type), label_(label), name_(name)
{
}


void ToolbarInfo::add(ToolbarItem const & item)
{
	items.push_back(item);
	items.back().func_.origin = FuncRequest::TOOLBAR;
}


ToolbarInfo & ToolbarInfo::read(Lexer & lex)
{
	enum {
		TO_COMMAND = 1,
		TO_ENDTOOLBAR,
		TO_SEPARATOR,
		TO_LAYOUTS,
		TO_MINIBUFFER,
		TO_TABLEINSERT,
		TO_POPUPMENU,
		TO_ICONPALETTE,
	};

	struct LexerKeyword toolTags[] = {
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

	lex.pushTable(toolTags);

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



/////////////////////////////////////////////////////////////////////////
//
// Toolbars
//
/////////////////////////////////////////////////////////////////////////

void Toolbars::reset()
{
	toolbar_info_.clear();
	toolbar_visibility_.clear();
}


void Toolbars::readToolbars(Lexer & lex)
{
	enum {
		TO_TOOLBAR = 1,
		TO_ENDTOOLBARSET,
	};

	struct LexerKeyword toolTags[] = {
		{ "end", TO_ENDTOOLBARSET },
		{ "toolbar", TO_TOOLBAR }
	};

	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbarset")) {
		LYXERR0("Toolbars::readToolbars: ERROR wrong token:`"
		       << lex.getString() << '\'');
	}

	lex.pushTable(toolTags);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;
	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case TO_TOOLBAR: {
			ToolbarInfo tbinfo;
			tbinfo.read(lex);
			toolbar_info_.push_back(tbinfo);
			break;
			}
		case TO_ENDTOOLBARSET:
			quit = true;
			break;
		default:
			lex.printError("Toolbars::readToolbars: "
				       "Unknown toolbar tag: `$$Token'");
			break;
		}
	}

	lex.popTable();
}


void Toolbars::readToolbarSettings(Lexer & lex)
{
	//consistency check
	if (compare_ascii_no_case(lex.getString(), "toolbars")) {
		LYXERR0("Toolbars::readToolbarSettings: ERROR wrong token:`"
		       << lex.getString() << '\'');
	}

	lex.next(true);

	while (lex.isOK()) {
		string name = lex.getString();
		lex.next(true);

		if (!compare_ascii_no_case(name, "end"))
			return;

		int visibility = 0;
		string flagstr = lex.getString();
		lex.next(true);
		vector<string> flags = getVectorFromString(flagstr);

		vector<string>::const_iterator cit = flags.begin();
		vector<string>::const_iterator end = flags.end();

		for (; cit != end; ++cit) {
			Visibility flag = ON;
			if (!compare_ascii_no_case(*cit, "off"))
				flag = OFF;
			else if (!compare_ascii_no_case(*cit, "on"))
				flag = ON;
			else if (!compare_ascii_no_case(*cit, "math"))
				flag = MATH;
			else if (!compare_ascii_no_case(*cit, "table"))
				flag = TABLE;
			else if (!compare_ascii_no_case(*cit, "mathmacrotemplate"))
				flag = MATHMACROTEMPLATE;
			else if (!compare_ascii_no_case(*cit, "review"))
				flag = REVIEW;
			else if (!compare_ascii_no_case(*cit, "top"))
				flag = TOP;
			else if (!compare_ascii_no_case(*cit, "bottom"))
				flag = BOTTOM;
			else if (!compare_ascii_no_case(*cit, "left"))
				flag = LEFT;
			else if (!compare_ascii_no_case(*cit, "right"))
				flag = RIGHT;
			else if (!compare_ascii_no_case(*cit, "auto"))
				flag = AUTO;
			else {
				LYXERR(Debug::ANY,
					"Toolbars::readToolbarSettings: unrecognised token:`"
					<< *cit << '\'');
				continue;
			}
			visibility |= flag;
		}
		toolbar_visibility_[name] = visibility;

		if (visibility >= MATH) {
			if (ToolbarInfo const * ti = info(name))
				const_cast<ToolbarInfo *>(ti)->gui_name += " (auto)";
		}
	}
}


ToolbarInfo const * Toolbars::info(std::string const & name) const
{
	Infos::const_iterator end = toolbar_info_.end();
	for (Infos::const_iterator it = toolbar_info_.begin(); it != end; ++it)
		if (it->name == name)
			return &(*it);
	return 0;
}


int Toolbars::defaultVisibility(std::string const & name) const
{
	map<string, int>::const_iterator it = toolbar_visibility_.find(name);
	if (it != toolbar_visibility_.end())
		return it->second;
	return OFF | BOTTOM;
}


bool Toolbars::isMainToolbar(std::string const & name) const
{
	return toolbar_visibility_.find(name) != toolbar_visibility_.end();
}


} // namespace frontend
} // namespace lyx
