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
	LT_PASS_THRU_CHARS,
	LT_PARBREAK_IS_NEWLINE,
	LT_ITEMCOMMAND,
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
	LT_FORCELOCAL,
	LT_TOGGLE_INDENT,
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
	forcelocal = 0;
	itemcommand_ = "item";
	toggle_indent = ITOGGLE_DOCUMENT_DEFAULT;
}


bool Layout::read(Lexer & lex, TextClass const & tclass)
{
	// If this is an empty layout, or if no force local version is set,
	// we know that we will not discard the stuff to read
	if (forcelocal == 0)
		return readIgnoreForcelocal(lex, tclass);
	Layout tmp(*this);
	tmp.forcelocal = 0;
	bool const ret = tmp.readIgnoreForcelocal(lex, tclass);
	// Keep the stuff if
	// - the read version is higher
	// - both versions are infinity (arbitrary decision)
	// - the file did not contain any local version (needed for not
	//   skipping user defined local layouts)
	if (tmp.forcelocal <= 0 || tmp.forcelocal > forcelocal)
		*this = tmp;
	return ret;
}


bool Layout::readIgnoreForcelocal(Lexer & lex, TextClass const & tclass)
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
		{ "forcelocal",     LT_FORCELOCAL },
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
		{ "itemcommand",    LT_ITEMCOMMAND },
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
		{ "passthruchars",  LT_PASS_THRU_CHARS },
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
		{ "toggleindent",   LT_TOGGLE_INDENT },
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
				postcommandargs_.clear();
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

		case LT_TOGGLE_INDENT: {
			string tog;
			lex >> tog;
			tog = support::ascii_lowercase(tog);
			if (tog == "always")
				toggle_indent = ITOGGLE_ALWAYS;
			else if (tog == "never")
				toggle_indent = ITOGGLE_NEVER;
			else
				toggle_indent = ITOGGLE_DOCUMENT_DEFAULT;
			break;
		}

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

		case LT_ITEMCOMMAND:
			lex >> itemcommand_;
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
			// FIXME: this means LT_LABELSTRING_APPENDIX may only
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

		case LT_PASS_THRU_CHARS:
			lex >> pass_thru_chars;
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

		case LT_FORCELOCAL:
			lex >> forcelocal;
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
		LA_ABOVE,
		LA_CENTERED,
		LA_STATIC,
		LA_SENSITIVE,
		LA_ENUMERATE,
		LA_ITEMIZE,
		LA_BIBLIO
	};


	LexerKeyword labelTypeTags[] = {
	  { "above",        LA_ABOVE },
		{ "bibliography", LA_BIBLIO },
		{ "centered",     LA_CENTERED },
		{ "enumerate",    LA_ENUMERATE },
		{ "itemize",      LA_ITEMIZE },
		{ "manual",       LA_MANUAL },
		{ "no_label",     LA_NO_LABEL },
		{ "sensitive",    LA_SENSITIVE },
		{ "static",       LA_STATIC }
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
	case LA_ABOVE:
		labeltype = LABEL_ABOVE;
		break;
	case LA_CENTERED:
		labeltype = LABEL_CENTERED;
		break;
	case LA_STATIC:
		labeltype = LABEL_STATIC;
		break;
	case LA_SENSITIVE:
		labeltype = LABEL_SENSITIVE;
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
	// this should be const, but can't be because
	// of PushPopHelper.
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
	// writeArgument() makes use of these default values
	arg.mandatory = false;
	arg.autoinsert = false;
	arg.insertcotext = false;
	bool error = false;
	bool finished = false;
	arg.font = inherit_font;
	arg.labelfont = inherit_font;
	string id;
	lex >> id;
	bool const itemarg = prefixIs(id, "item:");
	bool const postcmd = prefixIs(id, "post:");

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
		} else if (tok == "menustring") {
			lex.next();
			arg.menustring = lex.getDocString();
		} else if (tok == "mandatory") {
			lex.next();
			arg.mandatory = lex.getBool();
		} else if (tok == "autoinsert") {
			lex.next();
			arg.autoinsert = lex.getBool();
		} else if (tok == "insertcotext") {
			lex.next();
			arg.insertcotext = lex.getBool();
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
		} else if (tok == "defaultarg") {
			lex.next();
			arg.defaultarg = lex.getDocString();
		} else if (tok == "presetarg") {
			lex.next();
			arg.presetarg = lex.getDocString();
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
		} else if (tok == "passthruchars") {
			lex.next();
			arg.pass_thru_chars = lex.getDocString();
		} else {
			lex.printError("Unknown tag");
			error = true;
		}
	}
	if (arg.labelstring.empty())
		LYXERR0("Incomplete Argument definition!");
	else if (itemarg)
		itemargs_[id] = arg;
	else if (postcmd)
		postcommandargs_[id] = arg;
	else
		latexargs_[id] = arg;
}


void writeArgument(ostream & os, string const & id, Layout::latexarg const & arg)
{
	os << "\tArgument " << id << '\n';
	if (!arg.labelstring.empty())
		os << "\t\tLabelString \"" << to_utf8(arg.labelstring) << "\"\n";
	if (!arg.menustring.empty())
		os << "\t\tMenuString \"" << to_utf8(arg.menustring) << "\"\n";
	if (arg.mandatory)
		os << "\t\tMandatory " << arg.mandatory << '\n';
	if (arg.autoinsert)
		os << "\t\tAutoinsert " << arg.autoinsert << '\n';
	if (arg.insertcotext)
		os << "\t\tInsertCotext " << arg.insertcotext << '\n';
	if (!arg.ldelim.empty())
		os << "\t\tLeftDelim \""
		   << to_utf8(subst(arg.ldelim, from_ascii("\n"), from_ascii("<br/>")))
		   << "\"\n";
	if (!arg.rdelim.empty())
		os << "\t\tRightDelim \""
		   << to_utf8(subst(arg.rdelim, from_ascii("\n"), from_ascii("<br/>")))
		   << "\"\n";
	if (!arg.defaultarg.empty())
		os << "\t\tDefaultArg \"" << to_utf8(arg.defaultarg) << "\"\n";
	if (!arg.presetarg.empty())
		os << "\t\tPresetArg \"" << to_utf8(arg.presetarg) << "\"\n";
	if (!arg.tooltip.empty())
		os << "\t\tToolTip \"" << to_utf8(arg.tooltip) << "\"\n";
	if (!arg.requires.empty())
		os << "\t\tRequires \"" << arg.requires << "\"\n";
	if (!arg.decoration.empty())
		os << "\t\tDecoration \"" << arg.decoration << "\"\n";
	if (arg.font != inherit_font)
		lyxWrite(os, arg.font, "Font", 2);
	if (arg.labelfont != inherit_font)
		lyxWrite(os, arg.labelfont, "LabelFont", 2);
	if (!arg.pass_thru_chars.empty())
		os << "\t\tPassThruChars \"" << to_utf8(arg.pass_thru_chars) << "\"\n";
	os << "\tEndArgument\n";
}


void Layout::write(ostream & os) const
{
	os << "Style " << to_utf8(name_) << '\n';
	if (!category_.empty() && obsoleted_by_.empty())
		os << "\tCategory \"" << to_utf8(category_) << "\"\n";
	// Can't deduce Copystyle here :-(
	if (!obsoleted_by_.empty()) {
		os << "\tObsoletedBy \"" << to_utf8(obsoleted_by_)
		   << "\"\nEnd\n";
		return;
	}
	if (!depends_on_.empty())
		os << "\tDependsOn " << to_utf8(depends_on_) << '\n';
	switch (margintype) {
		case MARGIN_DYNAMIC:
			os << "\tMargin Dynamic\n";
			break;
		case MARGIN_FIRST_DYNAMIC:
			os << "\tMargin First_Dynamic\n";
			break;
		case MARGIN_MANUAL:
			os << "\tMargin Manual\n";
			break;
		case MARGIN_RIGHT_ADDRESS_BOX:
			os << "\tMargin Right_Address_Box\n";
			break;
		case MARGIN_STATIC:
			os << "\tMargin Static\n";
			break;
	}
	switch (latextype) {
		case LATEX_BIB_ENVIRONMENT:
			os << "\tLatexType Bib_Environment\n";
			break;
		case LATEX_COMMAND:
			os << "\tLatexType Command\n";
			break;
		case LATEX_ENVIRONMENT:
			os << "\tLatexType Environment\n";
			break;
		case LATEX_ITEM_ENVIRONMENT:
			os << "\tLatexType Item_Environment\n";
			break;
		case LATEX_LIST_ENVIRONMENT:
			os << "\tLatexType List_Environment\n";
			break;
		case LATEX_PARAGRAPH:
			os << "\tLatexType Paragraph\n";
			break;
	}
	os << "\tInTitle " << intitle << "\n"
	      "\tInPreamble " << inpreamble << "\n"
	      "\tTocLevel " << toclevel << '\n';
	// ResetArgs does not make sense here
	for (LaTeXArgMap::const_iterator it = latexargs_.begin();
	     it != latexargs_.end(); ++it)
		writeArgument(os, it->first, it->second);
	for (LaTeXArgMap::const_iterator it = itemargs_.begin();
	     it != itemargs_.end(); ++it)
		writeArgument(os, it->first, it->second);
	for (LaTeXArgMap::const_iterator it = postcommandargs_.begin();
	     it != postcommandargs_.end(); ++it)
		writeArgument(os, it->first, it->second);
	os << "\tNeedProtect " << needprotect << "\n"
	      "\tKeepEmpty " << keepempty << '\n';
	if (labelfont == font)
		lyxWrite(os, font, "Font", 1);
	else {
		lyxWrite(os, font, "TextFont", 1);
		lyxWrite(os, labelfont, "LabelFont", 1);
	}
	os << "\tNextNoIndent " << nextnoindent << "\n"
	      "\tCommandDepth " << commanddepth << '\n';
	if (!latexname_.empty())
		os << "\tLatexName \"" << latexname_ << "\"\n";
	if (!latexparam_.empty())
		os << "\tLatexParam \"" << subst(latexparam_, "\"", "&quot;")
		   << "\"\n";
	if (!leftdelim_.empty())
		os << "\tLeftDelim "
		   << to_utf8(subst(leftdelim_, from_ascii("\n"), from_ascii("<br/>")))
		   << '\n';
	if (!rightdelim_.empty())
		os << "\tRightDelim "
		   << to_utf8(subst(rightdelim_, from_ascii("\n"), from_ascii("<br/>")))
		   << '\n';
	if (!innertag_.empty())
		os << "\tInnerTag \"" << innertag_ << "\"\n";
	if (!labeltag_.empty())
		os << "\tLabelTag \"" << labeltag_ << "\"\n";
	if (!itemtag_.empty())
		os << "\tItemTag \"" << itemtag_ << "\"\n";
	if (!itemcommand_.empty())
		os << "\tItemCommand " << itemcommand_ << '\n';
	if (!preamble_.empty())
		os << "\tPreamble\n\t"
		   << to_utf8(subst(rtrim(preamble_, "\n"),
		                    from_ascii("\n"), from_ascii("\n\t")))
		   << "\n\tEndPreamble\n";
	if (!langpreamble_.empty())
		os << "\tLangPreamble\n\t"
		   << to_utf8(subst(rtrim(langpreamble_, "\n"),
		                    from_ascii("\n"), from_ascii("\n\t")))
		   << "\n\tEndLangPreamble\n";
	if (!babelpreamble_.empty())
		os << "\tBabelPreamble\n\t"
		   << to_utf8(subst(rtrim(babelpreamble_, "\n"),
		                    from_ascii("\n"), from_ascii("\n\t")))
		   << "\n\tEndBabelPreamble\n";
	switch (labeltype) {
	case LABEL_ABOVE:
		os << "\tLabelType Above\n";
		break;
	case LABEL_BIBLIO:
		os << "\tLabelType Bibliography\n";
		break;
	case LABEL_CENTERED:
		os << "\tLabelType Centered\n";
		break;
	case LABEL_ENUMERATE:
		os << "\tLabelType Enumerate\n";
		break;
	case LABEL_ITEMIZE:
		os << "\tLabelType Itemize\n";
		break;
	case LABEL_MANUAL:
		os << "\tLabelType Manual\n";
		break;
	case LABEL_NO_LABEL:
		os << "\tLabelType No_Label\n";
		break;
	case LABEL_SENSITIVE:
		os << "\tLabelType Sensitive\n";
		break;
	case LABEL_STATIC:
		os << "\tLabelType Static\n";
		break;
	}
	switch (endlabeltype) {
	case END_LABEL_BOX:
		os << "\tEndLabelType Box\n";
		break;
	case END_LABEL_FILLED_BOX:
		os << "\tEndLabelType Filled_Box\n";
		break;
	case END_LABEL_NO_LABEL:
		os << "\tEndLabelType No_Label\n";
		break;
	case END_LABEL_STATIC:
		os << "\tEndLabelType Static\n";
		break;
	}
	if (!leftmargin.empty())
		os << "\tLeftMargin \"" << to_utf8(leftmargin) << "\"\n";
	if (!rightmargin.empty())
		os << "\tRightMargin \"" << to_utf8(rightmargin) << "\"\n";
	if (!labelindent.empty())
		os << "\tLabelIndent " << to_utf8(labelindent) << '\n';
	if (!parindent.empty())
		os << "\tParIndent " << to_utf8(parindent) << '\n';
	os << "\tParSkip " << parskip << "\n"
	      "\tItemSep " << itemsep << "\n"
	      "\tTopSep " << topsep << "\n"
	      "\tBottomSep " << bottomsep << "\n"
	      "\tLabelBottomSep " << labelbottomsep << '\n';
	if (!labelsep.empty())
		os << "\tLabelSep " << to_utf8(subst(labelsep, ' ', 'x'))
		   << '\n';
	os << "\tParSep " << parsep << "\n"
	      "\tNewLine " << newline_allowed << '\n';
	switch (align) {
	case LYX_ALIGN_BLOCK:
		os << "\tAlign Block\n";
		break;
	case LYX_ALIGN_CENTER:
		os << "\tAlign Center\n";
		break;
	case LYX_ALIGN_LAYOUT:
		os << "\tAlign Layout\n";
		break;
	case LYX_ALIGN_LEFT:
		os << "\tAlign Left\n";
		break;
	case LYX_ALIGN_RIGHT:
		os << "\tAlign Right\n";
		break;
	case LYX_ALIGN_DECIMAL:
	case LYX_ALIGN_SPECIAL:
	case LYX_ALIGN_NONE:
		break;
	}
	if (alignpossible & (LYX_ALIGN_BLOCK | LYX_ALIGN_CENTER |
	                     LYX_ALIGN_LAYOUT | LYX_ALIGN_LEFT | LYX_ALIGN_RIGHT)) {
		bool first = true;
		os << "\tAlignPossible";
		if (alignpossible & LYX_ALIGN_BLOCK) {
			if (!first)
				os << ',';
			os << " Block";
			first = false;
		}
		if (alignpossible & LYX_ALIGN_CENTER) {
			if (!first)
				os << ',';
			os << " Center";
			first = false;
		}
		if (alignpossible & LYX_ALIGN_LAYOUT) {
			if (!first)
				os << ',';
			os << " Layout";
			first = false;
		}
		if (alignpossible & LYX_ALIGN_LEFT) {
			if (!first)
				os << ',';
			os << " Left";
			first = false;
		}
		if (alignpossible & LYX_ALIGN_RIGHT) {
			if (!first)
				os << ',';
			os << " Right";
			first = false;
		}
		os << '\n';
	}
	// LabelString must come before LabelStringAppendix
	if (!labelstring_.empty())
		os << "\tLabelString \"" << to_utf8(labelstring_) << "\"\n";
	if (!endlabelstring_.empty())
		os << "\tEndLabelString \"" << to_utf8(endlabelstring_) << "\"\n";
	if (!labelstring_appendix_.empty() && labelstring_appendix_ != labelstring_)
		os << "\tLabelStringAppendix \""
		   << to_utf8(labelstring_appendix_) << "\"\n";
	if (!counter.empty())
		os << "\tLabelCounter \"" << to_utf8(counter) << "\"\n";
	os << "\tFreeSpacing " << free_spacing << '\n';
	os << "\tPassThru " << pass_thru << '\n';
	if (!pass_thru_chars.empty())
		os << "\tPassThruChars " << to_utf8(pass_thru_chars) << '\n';
	os << "\tParbreakIsNewline " << parbreak_is_newline << '\n';
	switch (spacing.getSpace()) {
	case Spacing::Double:
		os << "\tSpacing Double\n";
		break;
	case Spacing::Onehalf:
		os << "\tSpacing Onehalf\n";
		break;
	case Spacing::Other:
		os << "\tSpacing Other " << spacing.getValueAsString() << '\n';
		break;
	case Spacing::Single:
		os << "\tSpacing Single\n";
		break;
	case Spacing::Default:
		break;
	}
	if (!requires_.empty()) {
		os << "\tRequires ";
		for (set<string>::const_iterator it = requires_.begin();
		     it != requires_.end(); ++it) {
			if (it != requires_.begin())
				os << ',';
			os << *it;
		}
		os << '\n';
	}
	if (refprefix.empty())
		os << "\tRefPrefix OFF\n";
	else
		os << "\tRefPrefix " << to_utf8(refprefix) << '\n';
	if (!htmltag_.empty())
		os << "\tHTMLTag " << htmltag_ << '\n';
	if (!htmlattr_.empty())
		os << "\tHTMLAttr " << htmlattr_ << '\n';
	if (!htmlitemtag_.empty())
		os << "\tHTMLItem " << htmlitemtag_ << '\n';
	if (!htmlitemattr_.empty())
		os << "\tHTMLItemAttr " << htmlitemattr_ << '\n';
	if (!htmllabeltag_.empty())
		os << "\tHTMLLabel " << htmllabeltag_ << '\n';
	if (!htmllabelattr_.empty())
		os << "\tHTMLLabelAttr " << htmllabelattr_ << '\n';
	os << "\tHTMLLabelFirst " << htmllabelfirst_ << '\n';
	if (!htmlstyle_.empty())
		os << "\tHTMLStyle\n"
		   << to_utf8(rtrim(htmlstyle_, "\n"))
		   << "\n\tEndHTMLStyle\n";
	os << "\tHTMLForceCSS " << htmlforcecss_ << '\n';
	if (!htmlpreamble_.empty())
		os << "\tHTMLPreamble\n"
		   << to_utf8(rtrim(htmlpreamble_, "\n"))
		   << "\n\tEndPreamble\n";
	os << "\tHTMLTitle " << htmltitle_ << "\n"
	      "\tSpellcheck " << spellcheck << "\n"
	      "\tForceLocal " << forcelocal << "\n"
	      "End\n";
}


Layout::LaTeXArgMap Layout::args() const
{
	LaTeXArgMap args = latexargs_;
	if (!postcommandargs_.empty())
		args.insert(postcommandargs_.begin(), postcommandargs_.end());
	if (!itemargs_.empty())
		args.insert(itemargs_.begin(), itemargs_.end());
	return args;
}


int Layout::optArgs() const
{
	int nr = 0;
	LaTeXArgMap::const_iterator it = latexargs_.begin();
	for (; it != latexargs_.end(); ++it) {
		if (!(*it).second.mandatory)
			++nr;
	}
	LaTeXArgMap::const_iterator iit = postcommandargs_.begin();
	for (; iit != postcommandargs_.end(); ++iit) {
		if (!(*iit).second.mandatory)
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
	LaTeXArgMap::const_iterator iit = postcommandargs_.begin();
	for (; iit != postcommandargs_.end(); ++iit) {
		if (!(*iit).second.mandatory)
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
		if (labeltype != LABEL_ABOVE &&
		    labeltype != LABEL_CENTERED)
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

	// alignment
	string where = alignmentToCSS(align);
	if (!where.empty()) {
		htmldefaultstyle_ += from_ascii("text-align: " + where + ";\n");
	}

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
	if (labeltype == LABEL_CENTERED)
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
