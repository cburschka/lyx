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
#include "Encoding.h"
#include "FontInfo.h"
#include "Language.h"
#include "Lexer.h"
#include "output_xhtml.h"
#include "TextClass.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Messages.h"
#include "support/textutils.h"


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
	LT_ARGUMENT,
	LT_MARGIN,
	LT_BOTTOMSEP,
	LT_CATEGORY,
	LT_COMMANDDEPTH,
	LT_COPYSTYLE,
	LT_DEPENDSON,
	LT_OBSOLETEDBY,
	LT_END,
	LT_FONT,
	LT_FREE_SPACING,
	LT_PASS_THRU,
	LT_PARBREAK_IS_NEWLINE,
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
	LT_LATEXTYPE,
	LT_LEFTDELIM,
	LT_LEFTMARGIN,
	LT_NEED_PROTECT,
	LT_NEWLINE,
	LT_NEXTNOINDENT,
	LT_PARINDENT,
	LT_PARSEP,
	LT_PARSKIP,
	LT_PREAMBLE,
	LT_LANGPREAMBLE,
	LT_BABELPREAMBLE,
	LT_REQUIRES,
	LT_RIGHTMARGIN,
	LT_SPACING,
	LT_TOPSEP,
	LT_TOCLEVEL,
	LT_INNERTAG,
	LT_LABELTAG,
	LT_ITEMTAG,
	LT_HTMLTAG,
	LT_HTMLATTR,
	LT_HTMLITEM,
	LT_HTMLITEMATTR,
	LT_HTMLLABEL,
	LT_HTMLLABELATTR, 
	LT_HTMLLABELFIRST,
	LT_HTMLPREAMBLE,
	LT_HTMLSTYLE,
	LT_HTMLFORCECSS,
	LT_INPREAMBLE,
	LT_HTMLTITLE,
	LT_SPELLCHECK,
	LT_REFPREFIX,
	LT_RESETARGS,
	LT_RIGHTDELIM,
	LT_INTITLE // keep this last!
};

/////////////////////

Layout::Layout()
{
	unknown_ = false;
	margintype = MARGIN_STATIC;
	latextype = LATEX_PARAGRAPH;
	intitle = false;
	inpreamble = false;
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
	newline_allowed = true;
	free_spacing = false;
	pass_thru = false;
	parbreak_is_newline = false;
	toclevel = NOT_IN_TOC;
	commanddepth = 0;
	htmllabelfirst_ = false;
	htmlforcecss_ = false;
	htmltitle_ = false;
	spellcheck = true;
}


bool Layout::read(Lexer & lex, TextClass const & tclass)
{
	// This table is sorted alphabetically [asierra 30March96]
	LexerKeyword layoutTags[] = {
		{ "align",          LT_ALIGN },
		{ "alignpossible",  LT_ALIGNPOSSIBLE },
		{ "argument",       LT_ARGUMENT },
		{ "babelpreamble",  LT_BABELPREAMBLE },
		{ "bottomsep",      LT_BOTTOMSEP },
		{ "category",       LT_CATEGORY },
		{ "commanddepth",   LT_COMMANDDEPTH },
		{ "copystyle",      LT_COPYSTYLE },
		{ "dependson",      LT_DEPENDSON },
		{ "end",            LT_END },
		{ "endlabelstring", LT_ENDLABELSTRING },
		{ "endlabeltype",   LT_ENDLABELTYPE },
		{ "font",           LT_FONT },
		{ "freespacing",    LT_FREE_SPACING },
		{ "htmlattr",       LT_HTMLATTR },
		{ "htmlforcecss",   LT_HTMLFORCECSS },
		{ "htmlitem",       LT_HTMLITEM },
		{ "htmlitemattr",   LT_HTMLITEMATTR },
		{ "htmllabel",      LT_HTMLLABEL },
		{ "htmllabelattr",  LT_HTMLLABELATTR },
		{ "htmllabelfirst", LT_HTMLLABELFIRST },
		{ "htmlpreamble",   LT_HTMLPREAMBLE },
		{ "htmlstyle",      LT_HTMLSTYLE },
		{ "htmltag",        LT_HTMLTAG },
		{ "htmltitle",      LT_HTMLTITLE },
		{ "innertag",       LT_INNERTAG },
		{ "inpreamble",     LT_INPREAMBLE },
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
		{ "langpreamble",   LT_LANGPREAMBLE },
		{ "latexname",      LT_LATEXNAME },
		{ "latexparam",     LT_LATEXPARAM },
		{ "latextype",      LT_LATEXTYPE },
		{ "leftdelim",      LT_LEFTDELIM },
		{ "leftmargin",     LT_LEFTMARGIN },
		{ "margin",         LT_MARGIN },
		{ "needprotect",    LT_NEED_PROTECT },
		{ "newline",        LT_NEWLINE },
		{ "nextnoindent",   LT_NEXTNOINDENT },
		{ "obsoletedby",    LT_OBSOLETEDBY },
		{ "parbreakisnewline", LT_PARBREAK_IS_NEWLINE },
		{ "parindent",      LT_PARINDENT },
		{ "parsep",         LT_PARSEP },
		{ "parskip",        LT_PARSKIP },
		{ "passthru",       LT_PASS_THRU },
		{ "preamble",       LT_PREAMBLE },
		{ "refprefix",      LT_REFPREFIX },
		{ "requires",       LT_REQUIRES },
		{ "resetargs",      LT_RESETARGS },
		{ "rightdelim",     LT_RIGHTDELIM },
		{ "rightmargin",    LT_RIGHTMARGIN },
		{ "spacing",        LT_SPACING },
		{ "spellcheck",     LT_SPELLCHECK },
		{ "textfont",       LT_TEXTFONT },
		{ "toclevel",       LT_TOCLEVEL },
		{ "topsep",         LT_TOPSEP }
	};

	bool error = false;
	bool finished = false;
	lex.pushTable(layoutTags);

	// parse style section
	while (!finished && lex.isOK() && !error) {
		int le = lex.lex();
		// See comment in LyXRC.cpp.
		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		case Lexer::LEX_UNDEF:
			// parse error
			lex.printError("Unknown layout tag `$$Token'");
			error = true;
			continue;

		default: 
			break;
		}
		switch (static_cast<LayoutTags>(le)) {
		case LT_END:
			finished = true;
			break;

		case LT_CATEGORY:
			lex >> category_;
			break;

		case LT_COPYSTYLE: {
			docstring style;
			lex >> style;
			style = subst(style, '_', ' ');

			if (tclass.hasLayout(style)) {
				docstring const tmpname = name_;
				this->operator=(tclass[style]);
				name_ = tmpname;
			} else {
				LYXERR0("Cannot copy unknown style `"
					<< style << "'\n"
					<< "All layouts so far:");
				DocumentClass::const_iterator lit = tclass.begin();
				DocumentClass::const_iterator len = tclass.end();
				for (; lit != len; ++lit)
					LYXERR0(lit->name());
			}
			break;
			}

		case LT_OBSOLETEDBY: {
			docstring style;
			lex >> style;
			style = subst(style, '_', ' ');

			if (tclass.hasLayout(style)) {
				docstring const tmpname = name_;
				this->operator=(tclass[style]);
				name_ = tmpname;
				if (obsoleted_by().empty())
					obsoleted_by_ = style;
			} else {
				LYXERR0("Cannot replace with unknown style `" 
					<< style << '\'');

				//lex.printError("Cannot replace with"
				//		 " unknown style "
				//		 "`$$Token'");
			}
			break;
		}

		case LT_DEPENDSON:
			lex >> depends_on_;
			depends_on_ = subst(depends_on_, '_', ' ');
			break;

		case LT_MARGIN:
			readMargin(lex);
			break;

		case LT_LATEXTYPE:
			readLatexType(lex);
			break;

		case LT_INTITLE:
			lex >> intitle;
			break;

		case LT_INPREAMBLE:
			lex >> inpreamble;
			break;

		case LT_TOCLEVEL:
			lex >> toclevel;
			break;

		case LT_RESETARGS:
			bool reset;
			lex >> reset;
			if (reset) {
				latexargs_.clear();
				itemargs_.clear();
			}
			break;

		case LT_ARGUMENT:
			readArgument(lex);
			break;

		case LT_NEED_PROTECT:
			lex >> needprotect;
			break;

		case LT_KEEPEMPTY:
			lex >> keepempty;
			break;

		case LT_FONT:
			font = lyxRead(lex, font);
			labelfont = font;
			break;

		case LT_TEXTFONT:
			font = lyxRead(lex, font);
			break;

		case LT_LABELFONT:
			labelfont = lyxRead(lex, labelfont);
			break;

		case LT_NEXTNOINDENT:
			lex >> nextnoindent;
			break;

		case LT_COMMANDDEPTH:
			lex >> commanddepth;
			break;

		case LT_LATEXNAME:
			lex >> latexname_;
			break;

		case LT_LATEXPARAM:
			lex >> latexparam_;
			latexparam_ = subst(latexparam_, "&quot;", "\"");
			break;

		case LT_LEFTDELIM:
			lex >> leftdelim_;
			leftdelim_ = support::subst(leftdelim_, from_ascii("<br/>"),
						    from_ascii("\n"));
			break;

		case LT_RIGHTDELIM:
			lex >> rightdelim_;
			rightdelim_ = support::subst(rightdelim_, from_ascii("<br/>"),
						     from_ascii("\n"));
			break;

		case LT_INNERTAG:
			lex >> innertag_;
			break;

		case LT_LABELTAG:
			lex >> labeltag_;
			break;

		case LT_ITEMTAG:
			lex >> itemtag_;
			break;

		case LT_PREAMBLE:
			preamble_ = from_utf8(lex.getLongString("EndPreamble"));
			break;

		case LT_LANGPREAMBLE:
			langpreamble_ = from_utf8(lex.getLongString("EndLangPreamble"));
			break;

		case LT_BABELPREAMBLE:
			babelpreamble_ = from_utf8(lex.getLongString("EndBabelPreamble"));
			break;

		case LT_LABELTYPE:
			readLabelType(lex);
			break;

		case LT_ENDLABELTYPE:
			readEndLabelType(lex);
			break;

		case LT_LEFTMARGIN:
			lex >> leftmargin;
			break;

		case LT_RIGHTMARGIN:
			lex >> rightmargin;
			break;

		case LT_LABELINDENT:
			lex >> labelindent;
			break;

		case LT_PARINDENT:
			lex >> parindent;
			break;

		case LT_PARSKIP:
			lex >> parskip;
			break;

		case LT_ITEMSEP:
			lex >> itemsep;
			break;

		case LT_TOPSEP:
			lex >> topsep;
			break;

		case LT_BOTTOMSEP:
			lex >> bottomsep;
			break;

		case LT_LABEL_BOTTOMSEP:
			lex >> labelbottomsep;
			break;

		case LT_LABELSEP:
			lex >> labelsep;
			labelsep = subst(labelsep, 'x', ' ');
			break;

		case LT_PARSEP:
			lex >> parsep;
			break;

		case LT_NEWLINE:
			lex >> newline_allowed;
			break;

		case LT_ALIGN:
			readAlign(lex);
			break;
	
		case LT_ALIGNPOSSIBLE:
			readAlignPossible(lex);
			break;

		case LT_LABELSTRING:
			// FIXME: this means LT_ENDLABELSTRING may only
			// occur after LT_LABELSTRING
			lex >> labelstring_;
			labelstring_ = trim(labelstring_);
			labelstring_appendix_ = labelstring_;
			break;

		case LT_ENDLABELSTRING:
			lex >> endlabelstring_;	
			endlabelstring_ = trim(endlabelstring_);
			break;

		case LT_LABELSTRING_APPENDIX:
			lex >> labelstring_appendix_;	
			labelstring_appendix_ = trim(labelstring_appendix_);
			break;

		case LT_LABELCOUNTER:
			lex >> counter;	
			counter = trim(counter);
			break;

		case LT_FREE_SPACING:
			lex >> free_spacing;
			break;

		case LT_PASS_THRU:
			lex >> pass_thru;
			break;

		case LT_PARBREAK_IS_NEWLINE:
			lex >> parbreak_is_newline;
			break;

		case LT_SPACING:
			readSpacing(lex);
			break;

		case LT_REQUIRES: {
			lex.eatLine();
			vector<string> const req = 
				getVectorFromString(lex.getString());
			requires_.insert(req.begin(), req.end());
			break;
		}
			
		case LT_REFPREFIX: {
			docstring arg;
			lex >> arg;
			if (arg == "OFF")
				refprefix.clear();
			else
				refprefix = arg;
			break;
		}

		case LT_HTMLTAG:
			lex >> htmltag_;
			break;
	
		case LT_HTMLATTR:
			lex >> htmlattr_;
			break;

		case LT_HTMLITEM:
			lex >> htmlitemtag_;
			break;
	
		case LT_HTMLITEMATTR:
			lex >> htmlitemattr_;
			break;
	
		case LT_HTMLLABEL:
			lex >> htmllabeltag_;
			break;

		case LT_HTMLLABELATTR: 
			lex >> htmllabelattr_;
			break;

		case LT_HTMLLABELFIRST:
			lex >> htmllabelfirst_;
			break;
			
		case LT_HTMLSTYLE:
			htmlstyle_ = from_utf8(lex.getLongString("EndHTMLStyle"));
			break;

		case LT_HTMLFORCECSS:
			lex >> htmlforcecss_;
			break;

		case LT_HTMLPREAMBLE:
			htmlpreamble_ = from_utf8(lex.getLongString("EndPreamble"));
			break;
		
		case LT_HTMLTITLE:
			lex >> htmltitle_;
			break;

		case LT_SPELLCHECK:
			lex >> spellcheck;
			break;
		}
	}
	lex.popTable();
	// make sure we only have inpreamble = true for commands
	if (inpreamble && latextype != LATEX_COMMAND && latextype != LATEX_PARAGRAPH) {
		LYXERR0("InPreamble not permitted except with command and paragraph layouts.");
		LYXERR0("Layout name: " << name());
		inpreamble = false;
	}

	return finished && !error;
}


enum {
	AT_BLOCK = 1,
	AT_LEFT,
	AT_RIGHT,
	AT_CENTER,
	AT_LAYOUT
};


LexerKeyword alignTags[] = {
	{ "block",  AT_BLOCK },
	{ "center", AT_CENTER },
	{ "layout", AT_LAYOUT },
	{ "left",   AT_LEFT },
	{ "right",  AT_RIGHT }
};


void Layout::readAlign(Lexer & lex)
{
	PushPopHelper pph(lex, alignTags);
	int le = lex.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lex.printError("Unknown alignment `$$Token'");
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


void Layout::readAlignPossible(Lexer & lex)
{
	lex.pushTable(alignTags);
	alignpossible = LYX_ALIGN_NONE | LYX_ALIGN_LAYOUT;
	int lineno = lex.lineNumber();
	do {
		int le = lex.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lex.printError("Unknown alignment `$$Token'");
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
	} while (lineno == lex.lineNumber());
	lex.popTable();
}


void Layout::readLabelType(Lexer & lex)
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

	PushPopHelper pph(lex, labelTypeTags);
	int le = lex.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lex.printError("Unknown labeltype tag `$$Token'");
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


void Layout::readEndLabelType(Lexer & lex)
{
	static LexerKeyword endlabelTypeTags[] = {
		{ "box",	      END_LABEL_BOX },
		{ "filled_box",	END_LABEL_FILLED_BOX },
		{ "no_label",	  END_LABEL_NO_LABEL },
		{ "static",     END_LABEL_STATIC }
	};

	PushPopHelper pph(lex, endlabelTypeTags);
	int le = lex.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lex.printError("Unknown labeltype tag `$$Token'");
		break;
	case END_LABEL_STATIC:
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX:
	case END_LABEL_NO_LABEL:
		endlabeltype = static_cast<EndLabelType>(le);
		break;
	default:
		LYXERR0("Unhandled value " << le);
		break;
	}
}


void Layout::readMargin(Lexer & lex)
{
	LexerKeyword marginTags[] = {
		{ "dynamic",           MARGIN_DYNAMIC },
		{ "first_dynamic",     MARGIN_FIRST_DYNAMIC },
		{ "manual",            MARGIN_MANUAL },
		{ "right_address_box", MARGIN_RIGHT_ADDRESS_BOX },
		{ "static",            MARGIN_STATIC }
	};

	PushPopHelper pph(lex, marginTags);

	int le = lex.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lex.printError("Unknown margin type tag `$$Token'");
		return;
	case MARGIN_STATIC:
	case MARGIN_MANUAL:
	case MARGIN_DYNAMIC:
	case MARGIN_FIRST_DYNAMIC:
	case MARGIN_RIGHT_ADDRESS_BOX:
		margintype = static_cast<MarginType>(le);
		break;
	default:
		LYXERR0("Unhandled value " << le);
		break;
	}
}


void Layout::readLatexType(Lexer & lex)
{
	LexerKeyword latexTypeTags[] = {
		{ "bib_environment",  LATEX_BIB_ENVIRONMENT },
		{ "command",          LATEX_COMMAND },
		{ "environment",      LATEX_ENVIRONMENT },
		{ "item_environment", LATEX_ITEM_ENVIRONMENT },
		{ "list_environment", LATEX_LIST_ENVIRONMENT },
		{ "paragraph",        LATEX_PARAGRAPH }
	};

	PushPopHelper pph(lex, latexTypeTags);
	int le = lex.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lex.printError("Unknown latextype tag `$$Token'");
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
		LYXERR0("Unhandled value " << le);
		break;
	}
}


void Layout::readSpacing(Lexer & lex)
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

	PushPopHelper pph(lex, spacingTags);
	int le = lex.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lex.printError("Unknown spacing token `$$Token'");
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
		lex.next();
		spacing.set(Spacing::Other, lex.getString());
		break;
	}
}


void Layout::readArgument(Lexer & lex)
{
	latexarg arg;
	arg.mandatory = false;
	bool error = false;
	bool finished = false;
	arg.font = inherit_font;
	arg.labelfont = inherit_font;
	string id;
	lex >> id;
	bool const itemarg = prefixIs(id, "item:");

	while (!finished && lex.isOK() && !error) {
		lex.next();
		string const tok = ascii_lowercase(lex.getString());

		if (tok.empty()) {
			continue;
		} else if (tok == "endargument") {
			finished = true;
		} else if (tok == "labelstring") {
			lex.next();
			arg.labelstring = lex.getDocString();
		} else if (tok == "mandatory") {
			lex.next();
			arg.mandatory = lex.getBool();
		} else if (tok == "leftdelim") {
			lex.next();
			arg.ldelim = lex.getDocString();
			arg.ldelim = support::subst(arg.ldelim, from_ascii("<br/>"),
						    from_ascii("\n"));
		} else if (tok == "rightdelim") {
			lex.next();
			arg.rdelim = lex.getDocString();
			arg.rdelim = support::subst(arg.rdelim, from_ascii("<br/>"),
						    from_ascii("\n"));
		} else if (tok == "tooltip") {
			lex.next();
			arg.tooltip = lex.getDocString();
		} else if (tok == "requires") {
			lex.next();
			arg.requires = lex.getString();
		} else if (tok == "decoration") {
			lex.next();
			arg.decoration = lex.getString();
		} else if (tok == "font") {
			arg.font = lyxRead(lex, arg.font);
		} else if (tok == "labelfont") {
			arg.labelfont = lyxRead(lex, arg.labelfont);
		} else {
			lex.printError("Unknown tag");
			error = true;
		}
	}
	if (arg.labelstring.empty())
		LYXERR0("Incomplete Argument definition!");
	else if (itemarg)
		itemargs_[id] = arg;
	else
		latexargs_[id] = arg;
}


int Layout::optArgs() const
{
	int nr = 0;
	LaTeXArgMap::const_iterator it = latexargs_.begin();
	for (; it != latexargs_.end(); ++it) {
		if (!(*it).second.mandatory)
			++nr;
	}
	return nr;
}


int Layout::requiredArgs() const
{
	int nr = 0;
	LaTeXArgMap::const_iterator it = latexargs_.begin();
	for (; it != latexargs_.end(); ++it) {
		if ((*it).second.mandatory)
			++nr;
	}
	return nr;
}


string const & Layout::htmltag() const 
{ 
	if (htmltag_.empty())
		htmltag_ =  "div";
	return htmltag_;
}


string const & Layout::htmlattr() const 
{ 
	if (htmlattr_.empty())
		htmlattr_ = "class=\"" + defaultCSSClass() + "\"";
	return htmlattr_; 
}


string const & Layout::htmlitemtag() const 
{ 
	if (htmlitemtag_.empty())
		htmlitemtag_ = "div";
	return htmlitemtag_; 
}


string const & Layout::htmlitemattr() const 
{ 
	if (htmlitemattr_.empty())
		htmlitemattr_ = "class=\"" + defaultCSSItemClass() + "\"";
	return htmlitemattr_; 
}


string const & Layout::htmllabeltag() const 
{ 
	if (htmllabeltag_.empty()) {
		if (labeltype != LABEL_TOP_ENVIRONMENT &&
		    labeltype != LABEL_CENTERED_TOP_ENVIRONMENT)
			htmllabeltag_ = "span";
		else
			htmllabeltag_ = "div";
	}
	return htmllabeltag_; 
}


string const & Layout::htmllabelattr() const 
{ 
	if (htmllabelattr_.empty())
		htmllabelattr_ = "class=\"" + defaultCSSLabelClass() + "\"";
	return htmllabelattr_; 
}


docstring Layout::htmlstyle() const
{
	if (!htmlstyle_.empty() && !htmlforcecss_)
		return htmlstyle_;
	if (htmldefaultstyle_.empty()) 
		makeDefaultCSS();
	docstring retval = htmldefaultstyle_;
	if (!htmlstyle_.empty())
		retval += '\n' + htmlstyle_;
	return retval;
}


string Layout::defaultCSSClass() const
{ 
	if (!defaultcssclass_.empty())
		return defaultcssclass_;
	docstring d;
	docstring::const_iterator it = name().begin();
	docstring::const_iterator en = name().end();
	for (; it != en; ++it) {
		char_type const c = *it;
		if (!isAlphaASCII(c)) {
			if (d.empty())
				// make sure we don't start with an underscore,
				// as that sometimes causes problems.
				d = from_ascii("lyx_");
			else
				d += '_';
		} else if (isLower(c))
			d += c;
		else
			// this is slow, so do it only if necessary
			d += lowercase(c);
	}
	defaultcssclass_ = to_utf8(d);
	return defaultcssclass_;
}


namespace {

string makeMarginValue(char const * side, double d)
{
	ostringstream os;
	os << "margin-" << side << ": " << d << "ex;\n";
	return os.str();
}

}


void Layout::makeDefaultCSS() const
{
	// this never needs to be redone, since reloading layouts will
	// wipe out what we did before.
	if (!htmldefaultstyle_.empty()) 
		return;
	
	// main font
	htmldefaultstyle_ = font.asCSS();
	
	// bottom margins
	string tmp;
	if (topsep > 0)
		tmp += makeMarginValue("top", topsep);
	if (bottomsep > 0)
		tmp += makeMarginValue("bottom", bottomsep);
	if (!leftmargin.empty()) {
		// we can't really do what LyX does with the margin, so 
		// we'll just figure out how many characters it is
		int const len = leftmargin.length();
		tmp += makeMarginValue("left", len);
	}
	if (!rightmargin.empty()) {
		int const len = rightmargin.length();
		tmp += makeMarginValue("right", len);
	}
		
	if (!tmp.empty()) {
		if (!htmldefaultstyle_.empty())
			htmldefaultstyle_ += from_ascii("\n");
		htmldefaultstyle_ += from_ascii(tmp);
	}

// tex2lyx does not see output_xhtml.cpp
#ifndef TEX2LYX
	// alignment
	string where = alignmentToCSS(align);
	if (!where.empty()) {
		htmldefaultstyle_ += from_ascii("text-align: " + where + ";\n");
	}
#endif
	
	// wrap up what we have, if anything
	if (!htmldefaultstyle_.empty())
		htmldefaultstyle_ = 
			from_ascii(htmltag() + "." + defaultCSSClass() + " {\n") +
			htmldefaultstyle_ + from_ascii("\n}\n");
	
	if (labeltype == LABEL_NO_LABEL || htmllabeltag() == "NONE")
		return;
	
	docstring labelCSS;
	
	// label font
	if (labelfont != font)
		labelCSS = labelfont.asCSS() + from_ascii("\n");
	if (labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		labelCSS += from_ascii("text-align: center;\n");
	
	if (!labelCSS.empty())
		htmldefaultstyle_ +=
			from_ascii(htmllabeltag() + "." + defaultCSSLabelClass() + " {\n") +
			labelCSS + from_ascii("\n}\n");
}


bool Layout::operator==(Layout const & rhs) const
{
	// This is enough for the applications we actually make,
	// at least at the moment. But we could check more.
	return name() == rhs.name()
		&& latexname() == rhs.latexname()
		&& latextype == rhs.latextype;
}


} // namespace lyx
