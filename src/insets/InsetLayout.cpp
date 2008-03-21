// -*- C++ -*-
/**
 * \file InsetLayout.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLayout.h"

#include "Color.h"
#include "Font.h"
#include "Lexer.h"
#include "support/lstrings.h"

#include <vector>

using std::string;
using std::set;
using std::vector;

namespace lyx {

InsetLayout::InsetLayout() :
	name_(from_ascii("undefined")), labelstring_(from_ascii("UNDEFINED")),
	decoration_(InsetLayout::Default),
	font_(sane_font), labelfont_(sane_font), bgcolor_(Color_error), 
	multipar_(false), passthru_(false), needprotect_(false),
	freespacing_(false), keepempty_(false), forceltr_(false)
{ 
	labelfont_.setColor(Color_error); 
}


enum InsetLayoutTags {
	IL_FONT = 1,
	IL_BGCOLOR,
	IL_DECORATION,
	IL_FREESPACING,
	IL_FORCELTR,
	IL_LABELFONT,
	IL_LABELSTRING,
	IL_LATEXNAME,
	IL_LATEXPARAM,
	IL_LATEXTYPE,
	IL_LYXTYPE,
	IL_KEEPEMPTY,
	IL_MULTIPAR,
	IL_NEEDPROTECT,
	IL_PASSTHRU,
	IL_PREAMBLE,
	IL_REQUIRES,
	IL_END
};


namespace {
	InsetLayout::InsetDecoration translateDecoration(std::string const & str) 
	{
		if (str == "classic")
			return InsetLayout::Classic;
		if (str == "minimalistic")
			return InsetLayout::Minimalistic;
		if (str == "conglomerate")
			return InsetLayout::Conglomerate;
		return InsetLayout::Default;
	}
}


bool InsetLayout::read(Lexer & lexrc)
{
	name_ = support::subst(lexrc.getDocString(), '_', ' ');

	keyword_item elementTags[] = {
		{ "bgcolor", IL_BGCOLOR },
		{ "decoration", IL_DECORATION },
		{ "end", IL_END },
		{ "font", IL_FONT },
		{ "forceltr", IL_FORCELTR },
		{ "freespacing", IL_FREESPACING },
		{ "keepempty", IL_KEEPEMPTY },
		{ "labelfont", IL_LABELFONT },
		{ "labelstring", IL_LABELSTRING },
		{ "latexname", IL_LATEXNAME },
		{ "latexparam", IL_LATEXPARAM },
		{ "latextype", IL_LATEXTYPE },
		{ "lyxtype", IL_LYXTYPE },
		{ "multipar", IL_MULTIPAR },
		{ "needprotect", IL_NEEDPROTECT },
		{ "passthru", IL_PASSTHRU },
		{ "preamble", IL_PREAMBLE },
		{ "requires", IL_REQUIRES }
	};

	lexrc.pushTable(elementTags, IL_END);

	FontInfo font = inherit_font;
	labelfont_ = inherit_font;
	bgcolor_ = Color_background;
	bool getout = false;
	
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown InsetLayout tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<InsetLayoutTags>(le)) {
		case IL_LYXTYPE:
			lexrc.next();
			lyxtype_ = lexrc.getString();
			break;
		case IL_LATEXTYPE:
			lexrc.next();
			latextype_ = lexrc.getString();
			break;
		case IL_LABELSTRING:
			lexrc.next();
			labelstring_ = lexrc.getDocString();
			break;
		case IL_DECORATION:
			lexrc.next();
			decoration_ = translateDecoration(lexrc.getString());
			break;
		case IL_LATEXNAME:
			lexrc.next();
			latexname_ = lexrc.getString();
			break;
		case IL_LATEXPARAM:
			lexrc.next();
			latexparam_ = support::subst(lexrc.getString(), "&quot;", "\"");
			break;
		case IL_LABELFONT:
			labelfont_ = lyxRead(lexrc, inherit_font);
			break;
		case IL_FORCELTR:
			lexrc.next();
			forceltr_ = lexrc.getBool();
			break;
		case IL_MULTIPAR:
			lexrc.next();
			multipar_ = lexrc.getBool();
			break;
		case IL_PASSTHRU:
			lexrc.next();
			passthru_ = lexrc.getBool();
			break;
		case IL_KEEPEMPTY:
			lexrc.next();
			keepempty_ = lexrc.getBool();
			break;
		case IL_FREESPACING:
			lexrc.next();
			freespacing_ = lexrc.getBool();
			break;
		case IL_NEEDPROTECT:
			lexrc.next();
			needprotect_ = lexrc.getBool();
			break;
		case IL_FONT: {
			font_ = lyxRead(lexrc, inherit_font);
			// If you want to define labelfont, you need to do so after
			// font is defined.
			labelfont_ = font_;
			break;
		}
		case IL_BGCOLOR: {
			lexrc.next();
			string const token = lexrc.getString();
			bgcolor_ = lcolor.getFromLyXName(token);
			break;
		}
		case IL_PREAMBLE:
			preamble_ = lexrc.getLongString("EndPreamble");
			break;
		case IL_REQUIRES: {
			lexrc.eatLine();
			vector<string> const req 
				= support::getVectorFromString(lexrc.getString());
			requires_.insert(req.begin(), req.end());
			break;
		}
		case IL_END:
			getout = true;
			break;
		}
	}

	// Here add element to list if getout == true
	if (!getout)
		return false;
	
	// The label font is generally used as-is without
	// any realization against a given context.
	labelfont_.realize(sane_font);

	lexrc.popTable();
	return true;
}

} //namespace lyx
