/**
 * \file Layout.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Layout.h"
#include "TextClass.h"
#include "Lexer.h"
#include "Font.h"

#include "support/debug.h"
#include "support/lstrings.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {

/// Special value of toclevel for layouts that to not belong in a TOC
const int Layout::NOT_IN_TOC = -1000;

//  The order of the LayoutTags enum is no more important. [asierra300396]
// Tags indexes.
enum LayoutTags {
	LT_ALIGN = 1,
	LT_ALIGNPOSSIBLE,
	LT_MARGIN,
	LT_BOTTOMSEP,
	LT_CATEGORY,
	LT_COMMANDDEPTH,
	LT_COPYSTYLE,
	LT_DEPENDSON,
	LT_OBSOLETEDBY,
	//LT_EMPTY,
	LT_END,
	//LT_ENVIRONMENT_DEFAULT,
	//LT_FANCYHDR,
	LT_FILL_BOTTOM,
	LT_FILL_TOP,
	//LT_FIRST_COUNTER,
	LT_FONT,
	LT_FREE_SPACING,
	LT_PASS_THRU,
	//LT_HEADINGS,
	LT_ITEMSEP,
	LT_KEEPEMPTY,
	LT_LABEL_BOTTOMSEP,
	LT_LABELFONT,
	LT_TEXTFONT,
	LT_LABELINDENT,
	LT_LABELSEP,
	LT_LABELSTRING,
	LT_LABELSTRING_APPENDIX,
	LT_LABELCOUNTER,
	LT_LABELTYPE,
	LT_ENDLABELSTRING,
	LT_ENDLABELTYPE,
	LT_LATEXNAME,
	LT_LATEXPARAM,
	LT_OPTARGS,
	LT_LATEXTYPE,
	LT_LATEXHEADER,
	LT_LATEXFOOTER,
	LT_LATEXPARAGRAPH,
	LT_LEFTMARGIN,
	LT_NEED_PROTECT,
	LT_NEWLINE,
	LT_NEXTNOINDENT,
	LT_PARINDENT,
	LT_PARSEP,
	LT_PARSKIP,
	//LT_PLAIN,
	LT_PREAMBLE,
	LT_REQUIRES,
	LT_RIGHTMARGIN,
	LT_SPACING,
	LT_TOPSEP,
	LT_TOCLEVEL,
	LT_INNERTAG,
	LT_LABELTAG,
	LT_ITEMTAG,
	LT_INTITLE // keep this last!
};

/////////////////////

Layout::Layout()
{
	margintype = MARGIN_STATIC;
	latextype = LATEX_PARAGRAPH;
	intitle = false;
	optionalargs = 0;
	needprotect = false;
	keepempty = false;
	font = inherit_font;
	labelfont = inherit_font;
	resfont = sane_font;
	reslabelfont = sane_font;
	nextnoindent = false;
	parskip = 0.0;
	itemsep = 0;
	topsep = 0.0;
	bottomsep = 0.0;
	labelbottomsep = 0.0;
	parsep = 0;
	align = LYX_ALIGN_BLOCK;
	alignpossible = LYX_ALIGN_NONE | LYX_ALIGN_LAYOUT;
	labeltype = LABEL_NO_LABEL;
	endlabeltype = END_LABEL_NO_LABEL;
	// Should or should not. That is the question.
	// spacing.set(Spacing::OneHalf);
	fill_top = false;
	fill_bottom = false;
	newline_allowed = true;
	free_spacing = false;
	pass_thru = false;
	is_environment = false;
	toclevel = NOT_IN_TOC;
	commanddepth = 0;
}


bool Layout::read(Lexer & lexrc, TextClass const & tclass)
{
	// This table is sorted alphabetically [asierra 30March96]
	LexerKeyword layoutTags[] = {
		{ "align",          LT_ALIGN },
		{ "alignpossible",  LT_ALIGNPOSSIBLE },
		{ "bottomsep",      LT_BOTTOMSEP },
		{ "category",       LT_CATEGORY },
		{ "commanddepth",   LT_COMMANDDEPTH },
		{ "copystyle",      LT_COPYSTYLE },
		{ "dependson",      LT_DEPENDSON },
		{ "end",            LT_END },
		{ "endlabelstring", LT_ENDLABELSTRING },
		{ "endlabeltype",   LT_ENDLABELTYPE },
		{ "fill_bottom",    LT_FILL_BOTTOM },
		{ "fill_top",       LT_FILL_TOP },
		{ "font",           LT_FONT },
		{ "freespacing",    LT_FREE_SPACING },
		{ "innertag",       LT_INNERTAG },
		{ "intitle",        LT_INTITLE },
		{ "itemsep",        LT_ITEMSEP },
		{ "itemtag",        LT_ITEMTAG },
		{ "keepempty",      LT_KEEPEMPTY },
		{ "labelbottomsep", LT_LABEL_BOTTOMSEP },
		{ "labelcounter",   LT_LABELCOUNTER },
		{ "labelfont",      LT_LABELFONT },
		{ "labelindent",    LT_LABELINDENT },
		{ "labelsep",       LT_LABELSEP },
		{ "labelstring",    LT_LABELSTRING },
		{ "labelstringappendix", LT_LABELSTRING_APPENDIX },
		{ "labeltag",       LT_LABELTAG },
		{ "labeltype",      LT_LABELTYPE },
		{ "latexfooter",    LT_LATEXFOOTER },
		{ "latexheader",    LT_LATEXHEADER },
		{ "latexname",      LT_LATEXNAME },
		{ "latexparagraph", LT_LATEXPARAGRAPH },
		{ "latexparam",     LT_LATEXPARAM },
		{ "latextype",      LT_LATEXTYPE },
		{ "leftmargin",     LT_LEFTMARGIN },
		{ "margin",         LT_MARGIN },
		{ "needprotect",    LT_NEED_PROTECT },
		{ "newline",        LT_NEWLINE },
		{ "nextnoindent",   LT_NEXTNOINDENT },
		{ "obsoletedby",    LT_OBSOLETEDBY },
		{ "optionalargs",   LT_OPTARGS },
		{ "parindent",      LT_PARINDENT },
		{ "parsep",         LT_PARSEP },
		{ "parskip",        LT_PARSKIP },
		{ "passthru",       LT_PASS_THRU },
		{ "preamble",       LT_PREAMBLE },
		{ "requires",       LT_REQUIRES },
		{ "rightmargin",    LT_RIGHTMARGIN },
		{ "spacing",        LT_SPACING },
		{ "textfont",       LT_TEXTFONT },
		{ "toclevel",       LT_TOCLEVEL },
		{ "topsep",         LT_TOPSEP }
	};

	bool error = false;
	bool finished = false;
	lexrc.pushTable(layoutTags);
	// parse style section
	while (!finished && lexrc.isOK() && !error) {
		int le = lexrc.lex();
		// See comment in LyXRC.cpp.
		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		case Lexer::LEX_UNDEF:		// parse error
			lexrc.printError("Unknown layout tag `$$Token'");
			error = true;
			continue;
		default: break;
		}
		switch (static_cast<LayoutTags>(le)) {
		case LT_END:		// end of structure
			finished = true;
			break;

		case LT_CATEGORY:
			if (lexrc.next())
				category_ = lexrc.getDocString();
			break;

		case LT_COPYSTYLE:     // initialize with a known style
			if (lexrc.next()) {
				docstring const style = subst(lexrc.getDocString(),
								'_', ' ');

				if (tclass.hasLayout(style)) {
					docstring const tmpname = name_;
					this->operator=(tclass[style]);
					name_ = tmpname;
				} else {
					lyxerr << "Cannot copy unknown style `"
					       << to_utf8(style) << "'\n"
					       << "All layouts so far:"
					       << endl;
					DocumentClass::const_iterator lit = tclass.begin();
					DocumentClass::const_iterator len = tclass.end();
					for (; lit != len; ++lit)
						lyxerr << to_utf8(lit->name()) << endl;

					//lexrc.printError("Cannot copy known "
					//		 "style `$$Token'");
				}
			}
			break;

		case LT_OBSOLETEDBY:     // replace with a known style
			if (lexrc.next()) {
				docstring const style = 
					subst(lexrc.getDocString(), '_', ' ');

				if (tclass.hasLayout(style)) {
					docstring const tmpname = name_;
					this->operator=(tclass[style]);
					name_ = tmpname;
					if (obsoleted_by().empty())
						obsoleted_by_ = style;
				} else {
					lyxerr << "Cannot replace with unknown style `" 
						<< to_utf8(style) << '\'' << endl;

					//lexrc.printError("Cannot replace with"
					//		 " unknown style "
					//		 "`$$Token'");
				}
			}
			break;

		case LT_DEPENDSON:
			if (lexrc.next())
			depends_on_ = subst(lexrc.getDocString(), '_', ' ');
			break;

		case LT_MARGIN:		// margin style definition.
			readMargin(lexrc);
			break;

		case LT_LATEXTYPE:	// LaTeX style definition.
			readLatexType(lexrc);
			break;

		case LT_LATEXHEADER:	// header for environments
			lexrc.next();
			latexheader = lexrc.getString();
			break;

		case LT_LATEXFOOTER:	// footer for environments
			lexrc.next();
			latexfooter = lexrc.getString();
			break;

		case LT_LATEXPARAGRAPH:
			lexrc.next();
			latexparagraph = lexrc.getString();
			break;

		case LT_INTITLE:
			intitle = lexrc.next() && lexrc.getInteger();
			break;

		case LT_TOCLEVEL:
			lexrc.next();
			toclevel = lexrc.getInteger();
			break;

		case LT_OPTARGS:
			if (lexrc.next())
				optionalargs = lexrc.getInteger();
			break;

		case LT_NEED_PROTECT:
			needprotect = lexrc.next() && lexrc.getInteger();
			break;

		case LT_KEEPEMPTY:
			keepempty = lexrc.next() && lexrc.getInteger();
			break;

		case LT_FONT:
			font = lyxRead(lexrc, font);
			labelfont = font;
			break;

		case LT_TEXTFONT:
			font = lyxRead(lexrc, font);
			break;

		case LT_LABELFONT:
			labelfont = lyxRead(lexrc, labelfont);
			break;

		case LT_NEXTNOINDENT:	// Indent next paragraph?
			if (lexrc.next() && lexrc.getInteger())
				nextnoindent = true;
			else
				nextnoindent = false;
			break;

		case LT_COMMANDDEPTH:
			lexrc.next();
			commanddepth = lexrc.getInteger();
			break;

		case LT_LATEXNAME:
			if (lexrc.next())
				latexname_ = lexrc.getString();
			break;

		case LT_LATEXPARAM:
			if (lexrc.next())
				latexparam_ = subst(lexrc.getString(), "&quot;", "\"");
			break;

		case LT_INNERTAG:
			if (lexrc.next())
				innertag_ = lexrc.getString();
			break;

		case LT_LABELTAG:
			if (lexrc.next())
				labeltag_ = lexrc.getString();
			break;

		case LT_ITEMTAG:
			if (lexrc.next())
				itemtag_ = lexrc.getString();
			break;

		case LT_PREAMBLE:
			preamble_ = from_utf8(lexrc.getLongString("EndPreamble"));
			break;

		case LT_LABELTYPE:
			readLabelType(lexrc);
			break;

		case LT_ENDLABELTYPE:
			readEndLabelType(lexrc);
			break;

		case LT_LEFTMARGIN:	// left margin type
			if (lexrc.next())
				leftmargin = lexrc.getDocString();
			break;

		case LT_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin = lexrc.getDocString();
			break;

		case LT_LABELINDENT:	// label indenting flag
			if (lexrc.next())
				labelindent = lexrc.getDocString();
			break;

		case LT_PARINDENT:	// paragraph indent. flag
			if (lexrc.next())
				parindent = lexrc.getDocString();
			break;

		case LT_PARSKIP:	// paragraph skip size
			if (lexrc.next())
				parskip = lexrc.getFloat();
			break;

		case LT_ITEMSEP:	// item separation size
			if (lexrc.next())
				itemsep = lexrc.getFloat();
			break;

		case LT_TOPSEP:		// top separation size
			if (lexrc.next())
				topsep = lexrc.getFloat();
			break;

		case LT_BOTTOMSEP:	// bottom separation size
			if (lexrc.next())
				bottomsep = lexrc.getFloat();
			break;

		case LT_LABEL_BOTTOMSEP: // label bottom separation size
			if (lexrc.next())
				labelbottomsep = lexrc.getFloat();
			break;

		case LT_LABELSEP:	// label separator
			if (lexrc.next()) {
				labelsep = from_utf8(subst(lexrc.getString(), 'x', ' '));
			}
			break;

		case LT_PARSEP:		// par. separation size
			if (lexrc.next())
				parsep = lexrc.getFloat();
			break;

		case LT_FILL_TOP:	// fill top flag
			if (lexrc.next())
				fill_top = lexrc.getInteger();
			break;

		case LT_FILL_BOTTOM:	// fill bottom flag
			if (lexrc.next())
				fill_bottom = lexrc.getInteger();
			break;

		case LT_NEWLINE:	// newlines allowed?
			if (lexrc.next())
				newline_allowed = lexrc.getInteger();
			break;

		case LT_ALIGN:		// paragraph align
			readAlign(lexrc);
			break;
		case LT_ALIGNPOSSIBLE:	// paragraph allowed align
			readAlignPossible(lexrc);
			break;

		case LT_LABELSTRING:	// label string definition
			if (lexrc.next()) {
				labelstring_ = trim(lexrc.getDocString());
				labelstring_appendix_ = labelstring_;
			}
			break;

		case LT_ENDLABELSTRING:	// endlabel string definition
			if (lexrc.next())
				endlabelstring_ = trim(lexrc.getDocString());
			break;

		case LT_LABELSTRING_APPENDIX: // label string appendix definition
			if (lexrc.next())
				labelstring_appendix_ = trim(lexrc.getDocString());
			break;

		case LT_LABELCOUNTER: // name of counter to use
			if (lexrc.next())
				counter = lyx::from_ascii(trim(lexrc.getString()));
			break;

		case LT_FREE_SPACING:	// Allow for free spacing.
			if (lexrc.next())
				free_spacing = lexrc.getInteger();
			break;

		case LT_PASS_THRU:	// Allow for pass thru.
			if (lexrc.next())
				pass_thru = lexrc.getInteger();
			break;

		case LT_SPACING: // setspace.sty
			readSpacing(lexrc);
			break;

		case LT_REQUIRES:
			lexrc.eatLine();
			vector<string> const req = 
				getVectorFromString(lexrc.getString());
			requires_.insert(req.begin(), req.end());
			break;

		}
	}
	lexrc.popTable();

	return !error;
}


enum {
	AT_BLOCK = 1,
	AT_LEFT,
	AT_RIGHT,
	AT_CENTER,
	AT_LAYOUT
};

void Layout::readAlign(Lexer & lexrc)
{
	LexerKeyword alignTags[] = {
		{ "block",  AT_BLOCK },
		{ "center", AT_CENTER },
		{ "layout", AT_LAYOUT },
		{ "left",   AT_LEFT },
		{ "right",  AT_RIGHT }
	};

	PushPopHelper pph(lexrc, alignTags);
	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown alignment `$$Token'");
		return;
	default: break;
	};
	switch (le) {
	case AT_BLOCK:
		align = LYX_ALIGN_BLOCK;
		break;
	case AT_LEFT:
		align = LYX_ALIGN_LEFT;
		break;
	case AT_RIGHT:
		align = LYX_ALIGN_RIGHT;
		break;
	case AT_CENTER:
		align = LYX_ALIGN_CENTER;
		break;
	case AT_LAYOUT:
		align = LYX_ALIGN_LAYOUT;
		break;
	}
}


void Layout::readAlignPossible(Lexer & lexrc)
{
	LexerKeyword alignTags[] = {
		{ "block",  AT_BLOCK },
		{ "center", AT_CENTER },
		{ "layout", AT_LAYOUT },
		{ "left",   AT_LEFT },
		{ "right",  AT_RIGHT }
	};

	lexrc.pushTable(alignTags);
	alignpossible = LYX_ALIGN_NONE | LYX_ALIGN_LAYOUT;
	int lineno = lexrc.lineNumber();
	do {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown alignment `$$Token'");
			continue;
		default: break;
		};
		switch (le) {
		case AT_BLOCK:
			alignpossible |= LYX_ALIGN_BLOCK;
			break;
		case AT_LEFT:
			alignpossible |= LYX_ALIGN_LEFT;
			break;
		case AT_RIGHT:
			alignpossible |= LYX_ALIGN_RIGHT;
			break;
		case AT_CENTER:
			alignpossible |= LYX_ALIGN_CENTER;
			break;
		case AT_LAYOUT:
			alignpossible |= LYX_ALIGN_LAYOUT;
			break;
		}
	} while (lineno == lexrc.lineNumber());
	lexrc.popTable();
}


void Layout::readLabelType(Lexer & lexrc)
{
	enum {
		LA_NO_LABEL = 1,
		LA_MANUAL,
		LA_TOP_ENVIRONMENT,
		LA_CENTERED_TOP_ENVIRONMENT,
		LA_STATIC,
		LA_SENSITIVE,
		LA_COUNTER,
		LA_ENUMERATE,
		LA_ITEMIZE,
		LA_BIBLIO
	};


	LexerKeyword labelTypeTags[] = {
		{ "bibliography",             LA_BIBLIO },
		{ "centered_top_environment", LA_CENTERED_TOP_ENVIRONMENT },
		{ "counter",                  LA_COUNTER },
		{ "enumerate",                LA_ENUMERATE },
		{ "itemize",                  LA_ITEMIZE },
		{ "manual",                   LA_MANUAL },
		{ "no_label",                 LA_NO_LABEL },
		{ "sensitive",                LA_SENSITIVE },
		{ "static",                   LA_STATIC },
		{ "top_environment",          LA_TOP_ENVIRONMENT }
	};

	PushPopHelper pph(lexrc, labelTypeTags);
	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown labeltype tag `$$Token'");
		return;
	default: break;
	}
	switch (le) {
	case LA_NO_LABEL:
		labeltype = LABEL_NO_LABEL;
		break;
	case LA_MANUAL:
		labeltype = LABEL_MANUAL;
		break;
	case LA_TOP_ENVIRONMENT:
		labeltype = LABEL_TOP_ENVIRONMENT;
		break;
	case LA_CENTERED_TOP_ENVIRONMENT:
		labeltype = LABEL_CENTERED_TOP_ENVIRONMENT;
		break;
	case LA_STATIC:
		labeltype = LABEL_STATIC;
		break;
	case LA_SENSITIVE:
		labeltype = LABEL_SENSITIVE;
		break;
	case LA_COUNTER:
		labeltype = LABEL_COUNTER;
		break;
	case LA_ENUMERATE:
		labeltype = LABEL_ENUMERATE;
		break;
	case LA_ITEMIZE:
		labeltype = LABEL_ITEMIZE;
		break;
	case LA_BIBLIO:
		labeltype = LABEL_BIBLIO;
		break;
	}
}


void Layout::readEndLabelType(Lexer & lexrc)
{
	static LexerKeyword endlabelTypeTags[] = {
		{ "box",	      END_LABEL_BOX },
		{ "filled_box",	END_LABEL_FILLED_BOX },
		{ "no_label",	  END_LABEL_NO_LABEL },
		{ "static",     END_LABEL_STATIC }
	};

	PushPopHelper pph(lexrc, endlabelTypeTags);
	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown labeltype tag `$$Token'");
		break;
	case END_LABEL_STATIC:
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX:
	case END_LABEL_NO_LABEL:
		endlabeltype = static_cast<EndLabelType>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in Layout::readEndLabelType." << endl;
		break;
	}
}


void Layout::readMargin(Lexer & lexrc)
{
	LexerKeyword marginTags[] = {
		{ "dynamic",           MARGIN_DYNAMIC },
		{ "first_dynamic",     MARGIN_FIRST_DYNAMIC },
		{ "manual",            MARGIN_MANUAL },
		{ "right_address_box", MARGIN_RIGHT_ADDRESS_BOX },
		{ "static",            MARGIN_STATIC }
	};

	PushPopHelper pph(lexrc, marginTags);

	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown margin type tag `$$Token'");
		return;
	case MARGIN_STATIC:
	case MARGIN_MANUAL:
	case MARGIN_DYNAMIC:
	case MARGIN_FIRST_DYNAMIC:
	case MARGIN_RIGHT_ADDRESS_BOX:
		margintype = static_cast<MarginType>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in Layout::readMargin." << endl;
		break;
	}
}


void Layout::readLatexType(Lexer & lexrc)
{
	LexerKeyword latexTypeTags[] = {
		{ "bib_environment",  LATEX_BIB_ENVIRONMENT },
		{ "command",          LATEX_COMMAND },
		{ "environment",      LATEX_ENVIRONMENT },
		{ "item_environment", LATEX_ITEM_ENVIRONMENT },
		{ "list_environment", LATEX_LIST_ENVIRONMENT },
		{ "paragraph",        LATEX_PARAGRAPH }
	};

	PushPopHelper pph(lexrc, latexTypeTags);
	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown latextype tag `$$Token'");
		return;
	case LATEX_PARAGRAPH:
	case LATEX_COMMAND:
	case LATEX_ENVIRONMENT:
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_BIB_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		latextype = static_cast<LatexType>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in Layout::readLatexType." << endl;
		break;
	}
}


void Layout::readSpacing(Lexer & lexrc)
{
	enum {
		ST_SPACING_SINGLE = 1,
		ST_SPACING_ONEHALF,
		ST_SPACING_DOUBLE,
		ST_OTHER
	};

	LexerKeyword spacingTags[] = {
		{"double",  ST_SPACING_DOUBLE },
		{"onehalf", ST_SPACING_ONEHALF },
		{"other",   ST_OTHER },
		{"single",  ST_SPACING_SINGLE }
	};

	PushPopHelper pph(lexrc, spacingTags);
	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown spacing token `$$Token'");
		return;
	default: break;
	}
	switch (le) {
	case ST_SPACING_SINGLE:
		spacing.set(Spacing::Single);
		break;
	case ST_SPACING_ONEHALF:
		spacing.set(Spacing::Onehalf);
		break;
	case ST_SPACING_DOUBLE:
		spacing.set(Spacing::Double);
		break;
	case ST_OTHER:
		lexrc.next();
		spacing.set(Spacing::Other, lexrc.getString());
		break;
	}
}


docstring const & Layout::name() const
{
	return name_;
}


void Layout::setName(docstring const & name)
{
	name_ = name;
}


docstring const & Layout::obsoleted_by() const
{
	return obsoleted_by_;
}


docstring const & Layout::depends_on() const
{
	return depends_on_;
}


bool Layout::operator==(Layout const & rhs) const
{
	// This is enough for the applications we actually make,
	// at least at the moment. But we could check more.
	return name() == rhs.name()
		&& latexname() == rhs.latexname()
		&& latextype == rhs.latextype;
}


Layout * Layout::forCaption()
{
	Layout * lay = new Layout();
	lay->name_ = from_ascii("Caption");
	lay->latexname_ = "caption";
	lay->latextype = LATEX_COMMAND;
	lay->optionalargs = 1;
	return lay;
}

} // namespace lyx
