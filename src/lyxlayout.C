
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxlayout.h"
#include "lyxtextclass.h"
#include "lyxlex.h"
#include "debug.h"

#include "support/lstrings.h"

using std::endl;

//  The order of the LayoutTags enum is no more important. [asierra300396]
// Tags indexes.
enum LayoutTags {
	LT_ALIGN = 1, 
	LT_ALIGNPOSSIBLE, 
	LT_MARGIN, 
	LT_BOTTOMSEP, 
	LT_COPYSTYLE, 
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
	LT_LABELTYPE,
	LT_ENDLABELSTRING,
	LT_ENDLABELTYPE,
	LT_LATEXNAME, 
	LT_LATEXPARAM, 
	LT_LATEXTYPE, 
	LT_LEFTMARGIN, 
	LT_NEED_PROTECT, 
	LT_NEWLINE, 
	LT_NEXTNOINDENT, 
	LT_PARINDENT, 
	LT_PARSEP, 
	LT_PARSKIP, 
	//LT_PLAIN,
	LT_PREAMBLE, 
	LT_RIGHTMARGIN, 
	LT_SPACING, 
	LT_TOPSEP, 
        LT_INTITLE 
};

/////////////////////

// Constructor for layout
LyXLayout::LyXLayout ()
{
	margintype = MARGIN_STATIC;
	latextype = LATEX_PARAGRAPH;
	intitle = false;
	needprotect = false;
	keepempty = false;
	font = LyXFont(LyXFont::ALL_INHERIT);
	labelfont = LyXFont(LyXFont::ALL_INHERIT);
	resfont = LyXFont(LyXFont::ALL_SANE);
	reslabelfont = LyXFont(LyXFont::ALL_SANE);
	nextnoindent = false;
	parskip = 0.0;
	itemsep = 0;
	topsep = 0.0;
	bottomsep = 0.0;
	labelbottomsep = 0.0;
	parsep = 0;
	align = LYX_ALIGN_BLOCK;
	alignpossible = LYX_ALIGN_BLOCK;
	labeltype = LABEL_NO_LABEL;
	endlabeltype = END_LABEL_NO_LABEL;
	// Should or should not. That is the question.
	// spacing.set(Spacing::OneHalf);
	fill_top = false;
	fill_bottom = false;
	newline_allowed = true;
	free_spacing = false;
	pass_thru = false;
}


// Reads a layout definition from file
bool LyXLayout::Read (LyXLex & lexrc, LyXTextClass const & tclass)
{
	// This table is sorted alphabetically [asierra 30March96]
	keyword_item layoutTags[] = {
		{ "align",			LT_ALIGN },
		{ "alignpossible",		LT_ALIGNPOSSIBLE },
		{ "bottomsep",			LT_BOTTOMSEP },
		{ "copystyle",                  LT_COPYSTYLE },
		{ "end",			LT_END },
		{ "endlabelstring",            	LT_ENDLABELSTRING },
		{ "endlabeltype",             	LT_ENDLABELTYPE },
		{ "fill_bottom",		LT_FILL_BOTTOM },
		{ "fill_top",			LT_FILL_TOP },
		{ "font",                  	LT_FONT },
		{ "freespacing",	   	LT_FREE_SPACING },
		{ "intitle",                    LT_INTITLE },
		{ "itemsep",               	LT_ITEMSEP },
		{ "keepempty",                  LT_KEEPEMPTY },
		{ "labelbottomsep",             LT_LABEL_BOTTOMSEP },
		{ "labelfont",             	LT_LABELFONT },
		{ "labelindent",           	LT_LABELINDENT },
		{ "labelsep",              	LT_LABELSEP },
		{ "labelstring",           	LT_LABELSTRING },
		{ "labelstringappendix",        LT_LABELSTRING_APPENDIX },
		{ "labeltype",             	LT_LABELTYPE },
		{ "latexname",             	LT_LATEXNAME },
		{ "latexparam",			LT_LATEXPARAM },
		{ "latextype",             	LT_LATEXTYPE },
		{ "leftmargin",            	LT_LEFTMARGIN },
		{ "margin",                	LT_MARGIN },
		{ "needprotect",                LT_NEED_PROTECT },
		{ "newline",			LT_NEWLINE },
		{ "nextnoindent",		LT_NEXTNOINDENT },
		{ "obsoletedby",                LT_OBSOLETEDBY },
		{ "parindent",			LT_PARINDENT },
		{ "parsep",			LT_PARSEP },
		{ "parskip",			LT_PARSKIP },
		{ "passthru",		   	LT_PASS_THRU },
		{ "preamble",                   LT_PREAMBLE },
		{ "rightmargin",		LT_RIGHTMARGIN },
		{ "spacing",                    LT_SPACING },
		{ "textfont",                   LT_TEXTFONT },
		{ "topsep",			LT_TOPSEP }
	};

	bool error = false;
	bool finished = false;
	lexrc.pushTable(layoutTags, LT_INTITLE);
	// parse style section
	while (!finished && lexrc.isOK() && !error) {
		int le = lexrc.lex();
		// See comment in lyxrc.C.
		switch (le) {
		case LyXLex::LEX_FEOF:
			continue; 

		case LyXLex::LEX_UNDEF:		// parse error
			lexrc.printError("Unknown layout tag `$$Token'");
			error = true;
			continue; 
		default: break;
		}
		switch (static_cast<LayoutTags>(le)) {
		case LT_END:		// end of structure
			finished = true;
			break;

		case LT_COPYSTYLE:     // initialize with a known style
		        if (lexrc.next()) {
				string const style = lowercase(lexrc.getString());
			
				if (tclass.hasLayout(style)) {
					string const tmpname = lowercase(name_);
					this->operator=(tclass[style]);
					name_ = lowercase(tmpname);
				} else {
					lyxerr << "Cannot copy unknown style `" << style << "'" << endl;
					LyXTextClass::const_iterator it = tclass.begin();
					LyXTextClass::const_iterator end = tclass.end();
					lyxerr << "All layouts so far:" << endl;
					for (; it != end; ++it) {
						lyxerr << it->name() << endl;
					}
					
					//lexrc.printError("Cannot copy known "
					//		 "style `$$Token'");
				}
			}
			break;

		case LT_OBSOLETEDBY:     // replace with a known style
		        if (lexrc.next()) {
				string const style = lowercase(lexrc.getString());
				
				if (tclass.hasLayout(style)) {
					string const tmpname = name_;
					this->operator=(tclass[style]);
					name_ = tmpname;
					if (obsoleted_by().empty())
						obsoleted_by_ = style;
				} else {
					lyxerr << "Cannot replace with unknown style `" << style << "'" << endl;
					
					//lexrc.printError("Cannot replace with" 
					//		 " unknown style "
					//		 "`$$Token'");
				}
			}
			break;

		case LT_MARGIN:		// Margin style definition.
			readMargin(lexrc);
			break;

		case LT_LATEXTYPE:	// Latex style definition.
			readLatexType(lexrc);
			break;

		case LT_INTITLE:
			intitle = lexrc.next() && lexrc.getInteger();
			break;
			
		case LT_NEED_PROTECT:
			needprotect = lexrc.next() && lexrc.getInteger();
			break;
			
		case LT_KEEPEMPTY:
			keepempty = lexrc.next() && lexrc.getInteger();
			break;

		case LT_FONT:
			font.lyxRead(lexrc);
			labelfont= font;
			break;

		case LT_TEXTFONT:
			font.lyxRead(lexrc);
			break;

		case LT_LABELFONT:
			labelfont.lyxRead(lexrc);
			break;

		case LT_NEXTNOINDENT:	// Indent next paragraph?
			if (lexrc.next() && lexrc.getInteger())
				nextnoindent = true;
			else
				nextnoindent = false;
			break;

		case LT_LATEXNAME:
		        if (lexrc.next())
		                latexname_ = lexrc.getString();
			break;
                        
		case LT_LATEXPARAM:
			if (lexrc.next())
				latexparam_ = lexrc.getString();
			break;

		case LT_PREAMBLE:
			preamble_ = lexrc.getLongString("EndPreamble");
			break;

		case LT_LABELTYPE:
			readLabelType(lexrc);
			break;

		case LT_ENDLABELTYPE:
			readEndLabelType(lexrc);
			break;
			
		case LT_LEFTMARGIN:	// left margin type
		        if (lexrc.next())
				leftmargin = lexrc.getString();
			break;			

		case LT_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin = lexrc.getString();
			break;

		case LT_LABELINDENT:	// label indenting flag
			if (lexrc.next())
				labelindent = lexrc.getString();
			break;

		case LT_PARINDENT:	// paragraph indent. flag
			if (lexrc.next())
				parindent = lexrc.getString();
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
				labelsep = subst(lexrc.getString(), 'x', ' ');
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
			if (lexrc.next())
				labelstring_ = lexrc.getString();
			break;

		case LT_ENDLABELSTRING:	// endlabel string definition
			if (lexrc.next())
				endlabelstring_ = lexrc.getString();
			break;

		case LT_LABELSTRING_APPENDIX: // label string appendix definition
			if (lexrc.next())
				labelstring_appendix_ = lexrc.getString();
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
		}
	}
	lexrc.popTable();
	return error;
}


enum AlignTags {
	AT_BLOCK = 1,
	AT_LEFT,
	AT_RIGHT,
	AT_CENTER,
	AT_LAYOUT
};


void LyXLayout::readAlign(LyXLex & lexrc)
{
	keyword_item alignTags[] = {
		{ "block",  AT_BLOCK },
		{ "center", AT_CENTER },
		{ "layout", AT_LAYOUT },
		{ "left",   AT_LEFT },
		{ "right",  AT_RIGHT }
	};

	pushpophelper pph(lexrc, alignTags, AT_LAYOUT);
	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown alignment `$$Token'");
		return; 
	default: break;
	};
	switch (static_cast<AlignTags>(le)) {
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


void LyXLayout::readAlignPossible(LyXLex & lexrc)
{
	keyword_item alignTags[] = {
		{ "block",  AT_BLOCK },
		{ "center", AT_CENTER },
		{ "layout", AT_LAYOUT },
		{ "left",   AT_LEFT },
		{ "right",  AT_RIGHT }
	};

	lexrc.pushTable(alignTags, AT_LAYOUT);
	alignpossible = LYX_ALIGN_NONE;
	int lineno = lexrc.getLineNo();
	do {
		int le = lexrc.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown alignment `$$Token'");
			continue; 
		default: break;
		};
		switch (static_cast<AlignTags>(le)) {
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
	} while (lineno == lexrc.getLineNo());
	lexrc.popTable();
}


enum LabelTypeTags {
	LA_NO_LABEL = 1,
	LA_MANUAL,
	LA_TOP_ENVIRONMENT,
	LA_CENTERED_TOP_ENVIRONMENT,
	LA_STATIC,
	LA_SENSITIVE,
	LA_COUNTER_CHAPTER,
	LA_COUNTER_SECTION,
	LA_COUNTER_SUBSECTION,
	LA_COUNTER_SUBSUBSECTION,
	LA_COUNTER_PARAGRAPH,
	LA_COUNTER_SUBPARAGRAPH,
	LA_COUNTER_ENUMI,
	LA_COUNTER_ENUMII,
	LA_COUNTER_ENUMIII,
	LA_COUNTER_ENUMIV,
	LA_BIBLIO
};


void LyXLayout::readLabelType(LyXLex & lexrc)
{
	keyword_item labelTypeTags[] = {
	{ "bibliography",             LA_BIBLIO },
	{ "centered_top_environment", LA_CENTERED_TOP_ENVIRONMENT },
	{ "counter_chapter",	      LA_COUNTER_CHAPTER },
	{ "counter_enumi",            LA_COUNTER_ENUMI },
	{ "counter_enumii",           LA_COUNTER_ENUMII },
	{ "counter_enumiii",          LA_COUNTER_ENUMIII },
	{ "counter_enumiv",           LA_COUNTER_ENUMIV },
	{ "counter_paragraph",        LA_COUNTER_PARAGRAPH },
	{ "counter_section",          LA_COUNTER_SECTION },
	{ "counter_subparagraph",     LA_COUNTER_SUBPARAGRAPH },
	{ "counter_subsection",	      LA_COUNTER_SUBSECTION },
	{ "counter_subsubsection",    LA_COUNTER_SUBSUBSECTION },
	{ "manual",                   LA_MANUAL },
	{ "no_label",                 LA_NO_LABEL },
	{ "sensitive",                LA_SENSITIVE },
	{ "static",                   LA_STATIC },
	{ "top_environment",          LA_TOP_ENVIRONMENT }
};

	pushpophelper pph(lexrc, labelTypeTags, LA_BIBLIO);
	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown labeltype tag `$$Token'");
		return; 
	default: break;
	}
	switch (static_cast<LabelTypeTags>(le)) {
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
	case LA_COUNTER_CHAPTER:
		labeltype = LABEL_COUNTER_CHAPTER;
		break;
	case LA_COUNTER_SECTION:
		labeltype = LABEL_COUNTER_SECTION;
		break;
	case LA_COUNTER_SUBSECTION:
		labeltype = LABEL_COUNTER_SUBSECTION;
		break;
	case LA_COUNTER_SUBSUBSECTION:
		labeltype = LABEL_COUNTER_SUBSUBSECTION;
		break;
	case LA_COUNTER_PARAGRAPH:
		labeltype = LABEL_COUNTER_PARAGRAPH;
		break;
	case LA_COUNTER_SUBPARAGRAPH:
		labeltype = LABEL_COUNTER_SUBPARAGRAPH;
		break;
	case LA_COUNTER_ENUMI:
		labeltype = LABEL_COUNTER_ENUMI;
		break;
	case LA_COUNTER_ENUMII:
		labeltype = LABEL_COUNTER_ENUMII;
		break;
	case LA_COUNTER_ENUMIII:
		labeltype = LABEL_COUNTER_ENUMIII;
		break;
	case LA_COUNTER_ENUMIV:
		labeltype = LABEL_COUNTER_ENUMIV;
		break;
	case LA_BIBLIO:
		labeltype = LABEL_BIBLIO;
		break;
	}
}


namespace {

keyword_item endlabelTypeTags[] = {
	{ "box",	END_LABEL_BOX },
	{ "filled_box",	END_LABEL_FILLED_BOX },
	{ "no_label",	END_LABEL_NO_LABEL },
	{ "static",     END_LABEL_STATIC }
};

} // namespace anon


void LyXLayout::readEndLabelType(LyXLex & lexrc)
{
	pushpophelper pph(lexrc, endlabelTypeTags,
			  END_LABEL_ENUM_LAST-END_LABEL_ENUM_FIRST+1);
	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown labeltype tag `$$Token'");
		break;
	case END_LABEL_STATIC:
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX:
	case END_LABEL_NO_LABEL:
		endlabeltype = static_cast<LYX_END_LABEL_TYPES>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in LyXLayout::readEndLabelType." << endl;
		break;
	}
}


void LyXLayout::readMargin(LyXLex & lexrc)
{
	keyword_item marginTags[] = {
		{ "dynamic",           MARGIN_DYNAMIC },
		{ "first_dynamic",     MARGIN_FIRST_DYNAMIC },
		{ "manual",            MARGIN_MANUAL },
		{ "right_address_box", MARGIN_RIGHT_ADDRESS_BOX },
		{ "static",            MARGIN_STATIC }
	};

	pushpophelper pph(lexrc, marginTags, MARGIN_RIGHT_ADDRESS_BOX);

	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown margin type tag `$$Token'");
		return;
	case MARGIN_STATIC:
	case MARGIN_MANUAL:
	case MARGIN_DYNAMIC:
	case MARGIN_FIRST_DYNAMIC:
	case MARGIN_RIGHT_ADDRESS_BOX:
		margintype = static_cast<LYX_MARGIN_TYPE>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in LyXLayout::readMargin." << endl;
		break;
	}
}


void LyXLayout::readLatexType(LyXLex & lexrc)
{
	keyword_item latexTypeTags[] = {
		{ "command",          LATEX_COMMAND },
		{ "environment",      LATEX_ENVIRONMENT },
		{ "item_environment", LATEX_ITEM_ENVIRONMENT },
		{ "list_environment", LATEX_LIST_ENVIRONMENT },
		{ "paragraph",        LATEX_PARAGRAPH }
	};

	pushpophelper pph(lexrc, latexTypeTags, LATEX_LIST_ENVIRONMENT);
	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown latextype tag `$$Token'");
		return;
	case LATEX_PARAGRAPH:
	case LATEX_COMMAND:
	case LATEX_ENVIRONMENT:
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		latextype = static_cast<LYX_LATEX_TYPES>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in LyXLayout::readLatexType." << endl;
		break;
	}
}


enum SpacingTags {
	ST_SPACING_SINGLE = 1,
	ST_SPACING_ONEHALF,
	ST_SPACING_DOUBLE,
	ST_OTHER
};


void LyXLayout::readSpacing(LyXLex & lexrc)
{
	keyword_item spacingTags[] = {
		{"double",  ST_SPACING_DOUBLE },
		{"onehalf", ST_SPACING_ONEHALF },
		{"other",   ST_OTHER },
		{"single",  ST_SPACING_SINGLE }
	};

	pushpophelper pph(lexrc, spacingTags, ST_OTHER);
	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown spacing token `$$Token'");
		return;
	default: break;
	}
	switch (static_cast<SpacingTags>(le)) {
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
		spacing.set(Spacing::Other, lexrc.getFloat());
		break;
	}
}


string const & LyXLayout::name() const
{
	static string name_t;
	name_t = lowercase(name_);
	return name_t;
}


void LyXLayout::setName(string const & n)
{
	name_ = lowercase(n);
}


string const & LyXLayout::obsoleted_by() const
{
	static string obsoleted_by_t;
	obsoleted_by_t = lowercase(obsoleted_by_);
	return obsoleted_by_t;
}
