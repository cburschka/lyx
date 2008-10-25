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
#include "TextClass.h"

#include "support/debug.h"
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
	multipar_(false), custompars_(false), forceplain_(true), 
	passthru_(false), needprotect_(false), freespacing_(false), 
	keepempty_(false), forceltr_(false)
{ 
	labelfont_.setColor(Color_error);
}


namespace {

InsetLayout::InsetDecoration translateDecoration(std::string const & str) 
{
	if (str == "classic" || str == "Classic")
		return InsetLayout::Classic;
	if (str == "minimalistic" || str == "Minimalistic")
		return InsetLayout::Minimalistic;
	if (str == "conglomerate" || str == "Conglomerate")
		return InsetLayout::Conglomerate;
	return InsetLayout::Default;
}

}


bool InsetLayout::read(Lexer & lex, TextClass & tclass)
{
	name_ = support::subst(lex.getDocString(), '_', ' ');
	// FIXME We need to check for name_.empty() here, and
	// take the same sort of action as in TextClass::read()
	// if it is empty. Or, better, we could read name_ there,
	// take action there, etc.

	enum {
		IL_BGCOLOR,
		IL_COPYSTYLE,
		IL_CUSTOMPARS,
		IL_DECORATION,
		IL_FONT,
		IL_FORCELTR,
		IL_FORCEPLAIN,
		IL_FREESPACING,
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


	LexerKeyword elementTags[] = {
		{ "bgcolor", IL_BGCOLOR },
		{ "copystyle", IL_COPYSTYLE }, 
		{ "custompars", IL_CUSTOMPARS },
		{ "decoration", IL_DECORATION },
		{ "end", IL_END },
		{ "font", IL_FONT },
		{ "forceltr", IL_FORCELTR },
		{ "forceplain", IL_FORCEPLAIN },
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

	lex.pushTable(elementTags);

	FontInfo font = inherit_font;
	labelfont_ = inherit_font;
	bgcolor_ = Color_background;
	bool getout = false;
	// whether we've read the CustomPars or ForcePlain tag
	// for issuing a warning in case MultiPars comes later
	bool readCustomOrPlain = false;

	string tmp;	
	while (!getout && lex.isOK()) {
		int le = lex.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lex.printError("Unknown InsetLayout tag");
			continue;
		default:
			break;
		}
		switch (le) {
		case IL_LYXTYPE:
			lex >> lyxtype_;
			break;
		case IL_LATEXTYPE:
			lex >> latextype_;
			break;
		case IL_LABELSTRING:
			lex >> labelstring_;
			break;
		case IL_DECORATION:
			lex >> tmp;
			decoration_ = translateDecoration(tmp);
			break;
		case IL_LATEXNAME:
			lex >> latexname_;
			break;
		case IL_LATEXPARAM:
			lex >> tmp;
			latexparam_ = support::subst(tmp, "&quot;", "\"");
			break;
		case IL_LABELFONT:
			labelfont_ = lyxRead(lex, inherit_font);
			break;
		case IL_FORCELTR:
			lex >> forceltr_;
			break;
		case IL_MULTIPAR:
			lex >> multipar_;
			// the defaults for these depend upon multipar_
			if (readCustomOrPlain)
				LYXERR0("Warning: Read MultiPar after CustomPars or ForcePlain. "
				        "Previous value may be overwritten!");
			readCustomOrPlain = false;
			custompars_ = multipar_;
			forceplain_ = !multipar_;
			break;
		case IL_CUSTOMPARS:
			lex >> custompars_;
			readCustomOrPlain = true;
			break;
		case IL_FORCEPLAIN:
			lex >> forceplain_;
			break;
		case IL_PASSTHRU:
			lex >> passthru_;
			readCustomOrPlain = true;
			break;
		case IL_KEEPEMPTY:
			lex >> keepempty_;
			break;
		case IL_FREESPACING:
			lex >> freespacing_;
			break;
		case IL_NEEDPROTECT:
			lex >> needprotect_;
			break;
		case IL_COPYSTYLE: {     // initialize with a known style
			docstring style;
			lex >> style;
			style = support::subst(style, '_', ' ');

			// We don't want to apply the algorithm in DocumentClass::insetLayout()
			// here. So we do it the long way.
			TextClass::InsetLayouts::const_iterator it = 
					tclass.insetLayouts().find(style);
			if (it != tclass.insetLayouts().end()) {
				docstring const tmpname = name_;
				this->operator=(it->second);
				name_ = tmpname;
			} else {
				LYXERR0("Cannot copy unknown InsetLayout `"
					<< style << "'\n"
					<< "All InsetLayouts so far:");
				TextClass::InsetLayouts::const_iterator lit = 
						tclass.insetLayouts().begin();
				TextClass::InsetLayouts::const_iterator len = 
						tclass.insetLayouts().end();
				for (; lit != len; ++lit)
					lyxerr << lit->second.name() << "\n";
			}
			break;
		}

		case IL_FONT: {
			font_ = lyxRead(lex, inherit_font);
			// If you want to define labelfont, you need to do so after
			// font is defined.
			labelfont_ = font_;
			break;
		}
		case IL_BGCOLOR:
			lex >> tmp;
			bgcolor_ = lcolor.getFromLyXName(tmp);
			break;
		case IL_PREAMBLE:
			preamble_ = lex.getLongString("EndPreamble");
			break;
		case IL_REQUIRES: {
			lex.eatLine();
			vector<string> const req 
				= support::getVectorFromString(lex.getString());
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

	lex.popTable();
	return true;
}

} //namespace lyx
