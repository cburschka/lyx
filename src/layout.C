/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include "layout.h"
#include "lyxlex.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "debug.h"
#include "gettext.h"
#include "support/LAssert.h"

using std::pair;
using std::make_pair;
using std::sort;
using std::endl;

// Global variable: textclass table.
LyXTextClassList textclasslist;


// Reads the style files
void LyXSetStyle()
{
	lyxerr[Debug::TCLASS] << "LyXSetStyle: parsing configuration...\n";
	
	if (!textclasslist.Read()) {
		lyxerr[Debug::TCLASS] << "LyXSetStyle: an error occured "
			"during parsing.\n             Exiting." << endl;
		exit(1);
	}

	lyxerr[Debug::TCLASS] << "LyXSetStyle: configuration parsed." << endl;
}


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


#if 0
// This table is sorted alphabetically [asierra 30March96]
static keyword_item layoutTags[] = {
	{ "align",			LT_ALIGN },
	{ "alignpossible",		LT_ALIGNPOSSIBLE },
	{ "bottomsep",			LT_BOTTOMSEP },
	{ "copystyle",                  LT_COPYSTYLE },
	{ "end",			LT_END },
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
	{ "preamble",                   LT_PREAMBLE },
	{ "rightmargin",		LT_RIGHTMARGIN },
	{ "spacing",                    LT_SPACING },
	{ "textfont",                   LT_TEXTFONT },
	{ "topsep",			LT_TOPSEP }
};
#endif


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
}


// Reads a layout definition from file
bool LyXLayout::Read (LyXLex & lexrc, LyXTextClass const & tclass)
{
#if 1
	// This table is sorted alphabetically [asierra 30March96]
	keyword_item layoutTags[] = {
		{ "align",			LT_ALIGN },
		{ "alignpossible",		LT_ALIGNPOSSIBLE },
		{ "bottomsep",			LT_BOTTOMSEP },
		{ "copystyle",                  LT_COPYSTYLE },
		{ "end",			LT_END },
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
		{ "preamble",                   LT_PREAMBLE },
		{ "rightmargin",		LT_RIGHTMARGIN },
		{ "spacing",                    LT_SPACING },
		{ "textfont",                   LT_TEXTFONT },
		{ "topsep",			LT_TOPSEP }
	};
#endif
	bool error = false;
	bool finished = false;
	lexrc.pushTable(layoutTags, LT_INTITLE);
	// parse style section
	while (!finished && lexrc.IsOK() && !error) {
		int le = lexrc.lex();
		// See comment in lyxrc.C.
		switch(le) {
		case LyXLex::LEX_FEOF:
			continue; 

		case LyXLex::LEX_UNDEF:		// parse error
			lexrc.printError("Unknown layout tag `$$Token'");
			error = true;
			continue; 
		default: break;
		}
		switch(static_cast<LayoutTags>(le)) {
		case LT_END:		// end of structure
			finished = true;
			break;

		case LT_COPYSTYLE:     // initialize with a known style
		        if (lexrc.next()) {
				if (tclass.hasLayout(lexrc.GetString())) {
					string tmpname = name_;
					this->operator= (tclass.GetLayout(lexrc.GetString()));
					name_ = tmpname;
				} else {
					lexrc.printError("Cannot copy known "
							 "style `$$Token'");
				}
			}
			break;

		case LT_OBSOLETEDBY:     // replace with a known style
		        if (lexrc.next()) {
				if (tclass.hasLayout(lexrc.GetString())) {
					string tmpname = name_;
					this->operator= (tclass.GetLayout(lexrc.GetString()));
					name_ = tmpname;
					if (obsoleted_by().empty())
					  obsoleted_by_ = lexrc.GetString();
				} else {
					lexrc.printError("Cannot replace with" 
							 " unknown style "
							 "`$$Token'");
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
			intitle = lexrc.next() && lexrc.GetInteger();
			break;
			
		case LT_NEED_PROTECT:
			needprotect = lexrc.next() && lexrc.GetInteger();
			break;
			
		case LT_KEEPEMPTY:
			keepempty = lexrc.next() && lexrc.GetInteger();
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
			if (lexrc.next() && lexrc.GetInteger())
				nextnoindent = true;
			else
				nextnoindent = false;
			break;

		case LT_LATEXNAME:
		        if (lexrc.next())
		                latexname_ = lexrc.GetString();
			break;
                        
		case LT_LATEXPARAM:
			if (lexrc.next())
				latexparam_ = lexrc.GetString();
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
				leftmargin = lexrc.GetString();
			break;			

		case LT_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin = lexrc.GetString();
			break;

		case LT_LABELINDENT:	// label indenting flag
			if (lexrc.next())
				labelindent = lexrc.GetString();
			break;

		case LT_PARINDENT:	// paragraph indent. flag
			if (lexrc.next())
				parindent = lexrc.GetString();
			break;

		case LT_PARSKIP:	// paragraph skip size
			if (lexrc.next())
				parskip = lexrc.GetFloat();
			break;

		case LT_ITEMSEP:	// item separation size
			if (lexrc.next())
				itemsep = lexrc.GetFloat();
			break;

		case LT_TOPSEP:		// top separation size
			if (lexrc.next())
				topsep = lexrc.GetFloat();
			break;

		case LT_BOTTOMSEP:	// bottom separation size
			if (lexrc.next())
				bottomsep = lexrc.GetFloat();
			break;

		case LT_LABEL_BOTTOMSEP: // label bottom separation size
			if (lexrc.next())
				labelbottomsep = lexrc.GetFloat();
			break;

		case LT_LABELSEP:	// label separator
			if (lexrc.next()) {
				labelsep = subst(lexrc.GetString(), 'x', ' ');
			}
			break;

		case LT_PARSEP:		// par. separation size
			if (lexrc.next())
				parsep = lexrc.GetFloat();
			break;

		case LT_FILL_TOP:	// fill top flag
			if (lexrc.next())
				fill_top = lexrc.GetInteger();
			break;

		case LT_FILL_BOTTOM:	// fill bottom flag
			if (lexrc.next())
				fill_bottom = lexrc.GetInteger();
			break;

		case LT_NEWLINE:	// newlines allowed?
			if (lexrc.next())
				newline_allowed = lexrc.GetInteger();
			break;

		case LT_ALIGN:		// paragraph align
			readAlign(lexrc);
			break;
		case LT_ALIGNPOSSIBLE:	// paragraph allowed align
			readAlignPossible(lexrc);
			break;

		case LT_LABELSTRING:	// label string definition
			if (lexrc.next())
				labelstring_ = lexrc.GetString();
			break;

		case LT_LABELSTRING_APPENDIX: // label string appendix definition
			if (lexrc.next())
				labelstring_appendix_ = lexrc.GetString();
			break;

		case LT_FREE_SPACING:	// Allow for free spacing.
			if (lexrc.next())
				free_spacing = lexrc.GetInteger();
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


#if 0
static keyword_item alignTags[] = {
	{ "block",  AT_BLOCK },
	{ "center", AT_CENTER },
	{ "layout", AT_LAYOUT },
	{ "left",   AT_LEFT },
	{ "right",  AT_RIGHT }
};
#endif

void LyXLayout::readAlign(LyXLex & lexrc)
{
#if 1
	keyword_item alignTags[] = {
		{ "block",  AT_BLOCK },
		{ "center", AT_CENTER },
		{ "layout", AT_LAYOUT },
		{ "left",   AT_LEFT },
		{ "right",  AT_RIGHT }
	};
#endif
	pushpophelper pph(lexrc, alignTags, AT_LAYOUT);
	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown alignment `$$Token'");
		return; 
	default: break;
	};
	switch(static_cast<AlignTags>(le)) {
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
#if 1
	keyword_item alignTags[] = {
		{ "block",  AT_BLOCK },
		{ "center", AT_CENTER },
		{ "layout", AT_LAYOUT },
		{ "left",   AT_LEFT },
		{ "right",  AT_RIGHT }
	};
#endif
	lexrc.pushTable(alignTags, AT_LAYOUT);
	alignpossible = LYX_ALIGN_NONE;
	int lineno = lexrc.GetLineNo();
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
	} while (lineno == lexrc.GetLineNo());
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


#if 0
static keyword_item labelTypeTags[] = {
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
#endif


void LyXLayout::readLabelType(LyXLex & lexrc)
{
#if 1
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
#endif
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

static keyword_item endlabelTypeTags[] = {
	{ "box",	END_LABEL_BOX },
	{ "filled_box",	END_LABEL_FILLED_BOX },
	{ "no_label",	END_LABEL_NO_LABEL }
};

void LyXLayout::readEndLabelType(LyXLex & lexrc)
{
	pushpophelper pph(lexrc, endlabelTypeTags,
			  END_LABEL_ENUM_LAST-END_LABEL_ENUM_FIRST+1);
	int le = lexrc.lex();
	switch(le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown labeltype tag `$$Token'");
		break;
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

#if 0
static keyword_item marginTags[] = {
	{ "dynamic",           MARGIN_DYNAMIC },
	{ "first_dynamic",     MARGIN_FIRST_DYNAMIC },
	{ "manual",            MARGIN_MANUAL },
	{ "right_address_box", MARGIN_RIGHT_ADDRESS_BOX },
	{ "static",            MARGIN_STATIC }
};
#endif

void LyXLayout::readMargin(LyXLex & lexrc)
{
#if 1
	keyword_item marginTags[] = {
		{ "dynamic",           MARGIN_DYNAMIC },
		{ "first_dynamic",     MARGIN_FIRST_DYNAMIC },
		{ "manual",            MARGIN_MANUAL },
		{ "right_address_box", MARGIN_RIGHT_ADDRESS_BOX },
		{ "static",            MARGIN_STATIC }
	};
#endif
	pushpophelper pph(lexrc, marginTags, MARGIN_RIGHT_ADDRESS_BOX);

	int le = lexrc.lex();
	switch(le) {
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


#if 0
static keyword_item latexTypeTags[] = {
	{ "command",          LATEX_COMMAND },
	{ "environment",      LATEX_ENVIRONMENT },
	{ "item_environment", LATEX_ITEM_ENVIRONMENT },
	{ "list_environment", LATEX_LIST_ENVIRONMENT },
	{ "paragraph",        LATEX_PARAGRAPH }
};
#endif


void LyXLayout::readLatexType(LyXLex & lexrc)
{
#if 1
	keyword_item latexTypeTags[] = {
		{ "command",          LATEX_COMMAND },
		{ "environment",      LATEX_ENVIRONMENT },
		{ "item_environment", LATEX_ITEM_ENVIRONMENT },
		{ "list_environment", LATEX_LIST_ENVIRONMENT },
		{ "paragraph",        LATEX_PARAGRAPH }
};
#endif
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


#if 0
static keyword_item spacingTags[] = {
	{"double",  ST_SPACING_DOUBLE },
	{"onehalf", ST_SPACING_ONEHALF },
	{"other",   ST_OTHER },
	{"single",  ST_SPACING_SINGLE }
};
#endif


void LyXLayout::readSpacing(LyXLex & lexrc)
{
#if 1
	keyword_item spacingTags[] = {
		{"double",  ST_SPACING_DOUBLE },
		{"onehalf", ST_SPACING_ONEHALF },
		{"other",   ST_OTHER },
		{"single",  ST_SPACING_SINGLE }
	};
#endif
	pushpophelper pph(lexrc, spacingTags, ST_OTHER);
	int le = lexrc.lex();
	switch(le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown spacing token `$$Token'");
		return;
	default: break;
	}
	switch(static_cast<SpacingTags>(le)) {
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
		spacing.set(Spacing::Other, lexrc.GetFloat());
		break;
	}
}


/* ******************************************************************* */

LyXTextClass::LyXTextClass(string const & fn, string const & cln,
			   string const & desc)
	: name_(fn), latexname_(cln), description_(desc)
{
	outputType_ = LATEX;
	columns_ = 1;
	sides_ = OneSide;
	secnumdepth_ = 3;
	tocdepth_ = 3;
	pagestyle_ = "default";
	maxcounter_ = LABEL_COUNTER_CHAPTER;
	defaultfont_ = LyXFont(LyXFont::ALL_SANE);
	opt_fontsize_ = "10|11|12";
	opt_pagestyle_ = "empty|plain|headings|fancy";
	provides_ = nothing;
	loaded = false;
}


bool LyXTextClass::do_readStyle(LyXLex & lexrc, LyXLayout & lay)
{
	lyxerr[Debug::TCLASS] << "Reading style " << lay.name() << endl;
	if (!lay.Read(lexrc, *this)) {
		// Reslove fonts
		lay.resfont = lay.font;
		lay.resfont.realize(defaultfont());
		lay.reslabelfont = lay.labelfont;
		lay.reslabelfont.realize(defaultfont());
		return false; // no errors
	} 
	lyxerr << "Error parsing style `" << lay.name() << "'" << endl;
	return true;
}


enum TextClassTags {
	TC_OUTPUTTYPE = 1,
	TC_INPUT,
	TC_STYLE,
	TC_NOSTYLE,
	TC_COLUMNS,
	TC_SIDES,
	TC_PAGESTYLE,
	TC_DEFAULTFONT,
	TC_MAXCOUNTER,
	TC_SECNUMDEPTH,
	TC_TOCDEPTH,
	TC_CLASSOPTIONS,
	TC_PREAMBLE,
	TC_PROVIDESAMSMATH,
	TC_PROVIDESMAKEIDX,
	TC_PROVIDESURL,
	TC_LEFTMARGIN,
	TC_RIGHTMARGIN
};


#if 0
static keyword_item textClassTags[] = {
	{ "classoptions",    TC_CLASSOPTIONS },
	{ "columns",         TC_COLUMNS },
	{ "defaultfont",     TC_DEFAULTFONT },
	{ "input",           TC_INPUT },
	{ "leftmargin",      TC_LEFTMARGIN },
	{ "maxcounter",      TC_MAXCOUNTER },
	{ "nostyle",         TC_NOSTYLE },
	{ "outputtype",      TC_OUTPUTTYPE },
	{ "pagestyle",       TC_PAGESTYLE },
	{ "preamble",        TC_PREAMBLE },
	{ "providesamsmath", TC_PROVIDESAMSMATH },
	{ "providesmakeidx", TC_PROVIDESMAKEIDX },
	{ "providesurl",     TC_PROVIDESURL },
	{ "rightmargin",     TC_RIGHTMARGIN },
	{ "secnumdepth",     TC_SECNUMDEPTH },
	{ "sides",           TC_SIDES },
	{ "style",           TC_STYLE },
	{ "tocdepth",        TC_TOCDEPTH }
};
#endif


// Reads a textclass structure from file.
bool LyXTextClass::Read(string const & filename, bool merge)
{
#if 1
	keyword_item textClassTags[] = {
		{ "classoptions",    TC_CLASSOPTIONS },
		{ "columns",         TC_COLUMNS },
		{ "defaultfont",     TC_DEFAULTFONT },
		{ "input",           TC_INPUT },
		{ "leftmargin",      TC_LEFTMARGIN },
		{ "maxcounter",      TC_MAXCOUNTER },
		{ "nostyle",         TC_NOSTYLE },
		{ "outputtype",      TC_OUTPUTTYPE },
		{ "pagestyle",       TC_PAGESTYLE },
		{ "preamble",        TC_PREAMBLE },
		{ "providesamsmath", TC_PROVIDESAMSMATH },
		{ "providesmakeidx", TC_PROVIDESMAKEIDX },
		{ "providesurl",     TC_PROVIDESURL },
		{ "rightmargin",     TC_RIGHTMARGIN },
		{ "secnumdepth",     TC_SECNUMDEPTH },
		{ "sides",           TC_SIDES },
		{ "style",           TC_STYLE },
		{ "tocdepth",        TC_TOCDEPTH }
};
#endif
	if (!merge)
		lyxerr[Debug::TCLASS] << "Reading textclass "
				      << MakeDisplayPath(filename)
				      << endl;
	else
		lyxerr[Debug::TCLASS] << "Reading input file "
				     << MakeDisplayPath(filename)
				     << endl;
	
	LyXLex lexrc(textClassTags, TC_RIGHTMARGIN);
	bool error = false;

        lexrc.setFile(filename);
	if (!lexrc.IsOK()) error = true; 

	// parsing
	while (lexrc.IsOK() && !error) {
		int le = lexrc.lex();
		switch(le) {
		case LyXLex::LEX_FEOF:
			continue; 

		case LyXLex::LEX_UNDEF:                                 
			lexrc.printError("Unknown TextClass tag `$$Token'");
			error = true;
			continue; 
		default: break;
		}
		switch(static_cast<TextClassTags>(le)) {
		case TC_OUTPUTTYPE:   // output type definition
			readOutputType(lexrc);
			break;
			
		case TC_INPUT: // Include file
		        if (lexrc.next()) {
		        	string tmp = LibFileSearch("layouts",
							    lexrc.GetString(), 
							    "layout");
				
				if (Read(tmp, true)) {
					lexrc.printError("Error reading input"
							 "file: "+tmp);
					error = true;
				}
			}
			break;

		case TC_STYLE:
			if (lexrc.next()) {
				string name = subst(lexrc.GetString(),
						    '_', ' ');
				if (hasLayout(name)) {
					LyXLayout & lay = GetLayout(name);
					error = do_readStyle(lexrc, lay);
				} else {
					LyXLayout lay;
					lay.name(name);
					if (!(error = do_readStyle(lexrc, lay)))
						layoutlist.push_back(lay);
				}
			}
			else {
				lexrc.printError("No name given for style: `$$Token'.");
				error = true;
			}
			break;

		case TC_NOSTYLE:
			if (lexrc.next()) {
				string style = subst(lexrc.GetString(),
						     '_', ' ');
				if (!delete_layout(style))
					lexrc.printError("Cannot delete style"
							 " `$$Token'");
			}
			break;

		case TC_COLUMNS:
			if (lexrc.next())
				columns_ = lexrc.GetInteger();
			break;
			
		case TC_SIDES:
			if (lexrc.next()) {
				switch(lexrc.GetInteger()) {
				case 1: sides_ = OneSide; break;
				case 2: sides_ = TwoSides; break;
				default:
					lyxerr << "Impossible number of page"
						" sides, setting to one."
					       << endl;
					sides_ = OneSide;
					break;
				}
			}
			break;
			
		case TC_PAGESTYLE:
		        lexrc.next();
			pagestyle_ = strip(lexrc.GetString());
			break;
			
		case TC_DEFAULTFONT:
			defaultfont_.lyxRead(lexrc);
			if (!defaultfont_.resolved()) {
				lexrc.printError("Warning: defaultfont should "
						 "be fully instantiated!");
				defaultfont_.realize(LyXFont::ALL_SANE);
			}
			break;

		case TC_MAXCOUNTER:
			readMaxCounter(lexrc);
			break;

		case TC_SECNUMDEPTH:
			lexrc.next();
			secnumdepth_ = lexrc.GetInteger();
			break;

		case TC_TOCDEPTH:
			lexrc.next();
			tocdepth_ = lexrc.GetInteger();
			break;

			// First step to support options 
	        case TC_CLASSOPTIONS:
			readClassOptions(lexrc);
		        break;

		case TC_PREAMBLE:
			preamble_ = lexrc.getLongString("EndPreamble");
			break;

		case TC_PROVIDESAMSMATH:
			if (lexrc.next() && lexrc.GetInteger())
				provides_ |= amsmath;
			break;

		case TC_PROVIDESMAKEIDX:
			if (lexrc.next() && lexrc.GetInteger())
				provides_ |= makeidx;
			break;

		case TC_PROVIDESURL:
			if (lexrc.next() && lexrc.GetInteger())
				provides_ = url;
			break;

		case TC_LEFTMARGIN:	// left margin type
		        if (lexrc.next())
				leftmargin_ = lexrc.GetString();
			break;			

		case TC_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin_ = lexrc.GetString();
			break;
		}
	}	

	if (!merge) { // we are at top level here.
		lyxerr[Debug::TCLASS] << "Finished reading textclass " 
				      << MakeDisplayPath(filename)
				      << endl;
	} else
		lyxerr[Debug::TCLASS] << "Finished reading input file " 
				      << MakeDisplayPath(filename)
				      << endl;

	return error;
}


#if 0
static keyword_item outputTypeTags[] = {
	{ "docbook", DOCBOOK },
	{ "latex", LATEX },
	{ "linuxdoc", LINUXDOC },
	{ "literate", LITERATE }
};
#endif

void LyXTextClass::readOutputType(LyXLex & lexrc)
{
#if 1
	keyword_item outputTypeTags[] = {
		{ "docbook", DOCBOOK },
		{ "latex", LATEX },
		{ "linuxdoc", LINUXDOC },
		{ "literate", LITERATE }
};
#endif
	pushpophelper pph(lexrc, outputTypeTags, LITERATE);

	int le = lexrc.lex();
	switch(le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown output type `$$Token'");
		return;
	case LATEX:
	case LINUXDOC:
	case DOCBOOK:
	case LITERATE:
		outputType_ = static_cast<OutputType>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in LyXTextClass::readOutputType." << endl;

		break;
	}
}


enum MaxCounterTags {
	MC_COUNTER_CHAPTER = 1,
	MC_COUNTER_SECTION,
	MC_COUNTER_SUBSECTION,
	MC_COUNTER_SUBSUBSECTION,
	MC_COUNTER_PARAGRAPH,
	MC_COUNTER_SUBPARAGRAPH,
	MC_COUNTER_ENUMI,
	MC_COUNTER_ENUMII,
	MC_COUNTER_ENUMIII,
	MC_COUNTER_ENUMIV
};


#if 0
static keyword_item maxCounterTags[] = {
	{"counter_chapter", MC_COUNTER_CHAPTER },
	{"counter_enumi", MC_COUNTER_ENUMI },
	{"counter_enumii", MC_COUNTER_ENUMII },
	{"counter_enumiii", MC_COUNTER_ENUMIII },
	{"counter_enumiv", MC_COUNTER_ENUMIV },
	{"counter_paragraph", MC_COUNTER_PARAGRAPH },
	{"counter_section", MC_COUNTER_SECTION },
	{"counter_subparagraph", MC_COUNTER_SUBPARAGRAPH },
	{"counter_subsection", MC_COUNTER_SUBSECTION },
	{"counter_subsubsection", MC_COUNTER_SUBSUBSECTION }
};
#endif


void LyXTextClass::readMaxCounter(LyXLex & lexrc)
{
#if 1
	keyword_item maxCounterTags[] = {
		{"counter_chapter", MC_COUNTER_CHAPTER },
		{"counter_enumi", MC_COUNTER_ENUMI },
		{"counter_enumii", MC_COUNTER_ENUMII },
		{"counter_enumiii", MC_COUNTER_ENUMIII },
		{"counter_enumiv", MC_COUNTER_ENUMIV },
		{"counter_paragraph", MC_COUNTER_PARAGRAPH },
		{"counter_section", MC_COUNTER_SECTION },
		{"counter_subparagraph", MC_COUNTER_SUBPARAGRAPH },
		{"counter_subsection", MC_COUNTER_SUBSECTION },
		{"counter_subsubsection", MC_COUNTER_SUBSUBSECTION }
	};
#endif
	pushpophelper pph(lexrc, maxCounterTags, MC_COUNTER_ENUMIV);
	int le = lexrc.lex();
	switch(le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown MaxCounter tag `$$Token'");
		return; 
	default: break;
	}
	switch (static_cast<MaxCounterTags>(le)) {
	case MC_COUNTER_CHAPTER:
		maxcounter_ = LABEL_COUNTER_CHAPTER;
		break;
	case MC_COUNTER_SECTION:
		maxcounter_ = LABEL_COUNTER_SECTION;
		break;
	case MC_COUNTER_SUBSECTION:
		maxcounter_ = LABEL_COUNTER_SUBSECTION;
		break;
	case MC_COUNTER_SUBSUBSECTION:
		maxcounter_ = LABEL_COUNTER_SUBSUBSECTION;
		break;
	case MC_COUNTER_PARAGRAPH:
		maxcounter_ = LABEL_COUNTER_PARAGRAPH;
		break;
	case MC_COUNTER_SUBPARAGRAPH:
		maxcounter_ = LABEL_COUNTER_SUBPARAGRAPH;
		break;
	case MC_COUNTER_ENUMI:
		maxcounter_ = LABEL_COUNTER_ENUMI;
		break;
	case MC_COUNTER_ENUMII:
		maxcounter_ = LABEL_COUNTER_ENUMII;
		break;
	case MC_COUNTER_ENUMIII:
		maxcounter_ = LABEL_COUNTER_ENUMIII;
		break;
	case MC_COUNTER_ENUMIV:
		maxcounter_ = LABEL_COUNTER_ENUMIV;
		break;
	}
}


enum ClassOptionsTags {
	CO_FONTSIZE = 1,
	CO_PAGESTYLE,
	CO_OTHER,
	CO_END
};


#if 0
static keyword_item classOptionsTags[] = {
	{"end", CO_END },
	{"fontsize", CO_FONTSIZE },
	{"other", CO_OTHER },
	{"pagestyle", CO_PAGESTYLE }
};
#endif


void LyXTextClass::readClassOptions(LyXLex & lexrc)
{
#if 1
	keyword_item classOptionsTags[] = {
		{"end", CO_END },
		{"fontsize", CO_FONTSIZE },
		{"other", CO_OTHER },
		{"pagestyle", CO_PAGESTYLE }
	};
#endif
	lexrc.pushTable(classOptionsTags, CO_END);
	bool getout = false;
	while (!getout && lexrc.IsOK()) {
		int le = lexrc.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue; 
		default: break;
		}
		switch (static_cast<ClassOptionsTags>(le)) {
		case CO_FONTSIZE:
			lexrc.next();
			opt_fontsize_ = strip(lexrc.GetString());
			break;
		case CO_PAGESTYLE:
			lexrc.next();
			opt_pagestyle_ = strip(lexrc.GetString()); 
			break;
		case CO_OTHER:
			lexrc.next();
			options_ = lexrc.GetString();
			break;
		case CO_END:
			getout = true;
			break;
		}
	}
	lexrc.popTable();
}


bool LyXTextClass::hasLayout(string const & name) const
{
	for (LayoutList::const_iterator cit = layoutlist.begin();
	     cit != layoutlist.end(); ++cit) {
		if ((*cit).name() == name)
			return true;
	}
	return false;
}


LyXLayout const & LyXTextClass::GetLayout (string const & name) const
{
	for (LayoutList::const_iterator cit = layoutlist.begin();
	     cit != layoutlist.end(); ++cit) {
		if ((*cit).name() == name)
			return (*cit);
	}
	Assert(false); // we actually require the name to exist.
	return layoutlist.front();
}


LyXLayout & LyXTextClass::GetLayout(string const & name)
{
	for (LayoutList::iterator it = layoutlist.begin();
	     it != layoutlist.end(); ++it) {
		if ((*it).name() == name)
			return (*it);
	}
	Assert(false); // we actually require the name to exist.
	return layoutlist.front();
}


bool LyXTextClass::delete_layout (string const & name)
{
	for(LayoutList::iterator it = layoutlist.begin();
	    it != layoutlist.end(); ++it) {
		if ((*it).name() == name) {
			layoutlist.erase(it);
			return true;
		}
	}
	return false;
}


// Load textclass info if not loaded yet
void LyXTextClass::load()
{
	if (loaded) return;

	// Read style-file
	string real_file = LibFileSearch("layouts", name_, "layout");

	if (Read(real_file)) {
		lyxerr << "Error reading `"
		       << MakeDisplayPath(real_file)
		       << "'\n(Check `" << name_
		       << "')\nCheck your installation and "
			"try Options/Reconfigure..." << endl;
	}
	loaded = true;
}


//////////////////////////////////////////

// Gets textclass number from name
pair<bool, LyXTextClassList::size_type>
LyXTextClassList::NumberOfClass(string const & textclass) const
{
	for (ClassList::const_iterator cit = classlist.begin();
	     cit != classlist.end(); ++cit) {
		if ((*cit).name() == textclass)
			return make_pair(true, cit - classlist.begin());
	}
	return make_pair(false, size_type(0));
}


// Gets layout structure from style number and textclass number
LyXLayout const &
LyXTextClassList::Style(LyXTextClassList::size_type textclass,
			LyXTextClass::size_type layout) const
{
	classlist[textclass].load();
	if (layout < classlist[textclass].numLayouts())
		return classlist[textclass][layout];
	return classlist[textclass][0];
}


// Gets layout number from name and textclass number
pair<bool, LyXTextClass::size_type>
LyXTextClassList::NumberOfLayout(LyXTextClassList::size_type textclass,
				 string const & name) const
{
	classlist[textclass].load();
	for(unsigned int i = 0; i < classlist[textclass].numLayouts(); ++i) {
		if (classlist[textclass][i].name() == name)
			return make_pair(true, i);
	}
	if (name == "dummy")
		return make_pair(true, LYX_DUMMY_LAYOUT);
	return make_pair(false, LyXTextClass::LayoutList::size_type(0)); // not found
}


// Gets a layout (style) name from layout number and textclass number
string const &
LyXTextClassList::NameOfLayout(LyXTextClassList::size_type textclass,
			       LyXTextClass::size_type layout) const
{
	static string dummy("dummy");
	classlist[textclass].load();
	if (layout < classlist[textclass].numLayouts())
		return classlist[textclass][layout].name();
	return dummy;
}


// Gets a textclass name from number
string const &
LyXTextClassList::NameOfClass(LyXTextClassList::size_type number) const
{
	static string dummy("dummy");
	if (classlist.size() == 0) {
		return dummy;
	}
	Assert(number < classlist.size());
	return classlist[number].name();
}


// Gets a textclass latexname from number
string const &
LyXTextClassList::LatexnameOfClass(LyXTextClassList::size_type number) const
{
	static string dummy("dummy");
	classlist[number].load();
	if (classlist.size() == 0) {
		return dummy;
	}
	Assert(number < classlist.size());
	return classlist[number].latexname();
}


// Gets a textclass description from number
string const &
LyXTextClassList::DescOfClass(LyXTextClassList::size_type number) const
{
	static string dummy("dummy");
	if (classlist.size() == 0) {
		return dummy;
	}
	Assert(number < classlist.size());
	return classlist[number].description();
}


// Gets a textclass structure from number
LyXTextClass const &
LyXTextClassList::TextClass(LyXTextClassList::size_type textclass) const
{
	classlist[textclass].load();
	if (textclass < classlist.size())
		return classlist[textclass];
	else
		return classlist[0];
}


void LyXTextClassList::Add(LyXTextClass const & t)
{
	classlist.push_back(t);
}


// used when sorting the textclass list.
class less_textclass_desc {
public:
	int operator()(LyXTextClass const & tc1, LyXTextClass const & tc2) {
		return tc1.description() < tc2.description();
	}
};


// Reads LyX textclass definitions according to textclass config file
bool LyXTextClassList::Read ()
{
	LyXLex lex(0, 0);
	string real_file = LibFileSearch("", "textclass.lst");
	lyxerr[Debug::TCLASS] << "Reading textclasses from `"
			      << real_file << "'" << endl;

	if (real_file.empty()) {
		lyxerr << "LyXTextClassList::Read: unable to find "
			"textclass file  `" << MakeDisplayPath(real_file, 1000)
		       << "'. Exiting." << endl;

		WriteAlert(_("LyX wasn't able to find its layout descriptions!"),
			   _("Check that the file \"textclass.lst\""),
			   _("is installed correctly. Sorry, has to exit :-("));
		return false;
		// This causes LyX to end... Not a desirable behaviour. Lgb
		// What do you propose? That the user gets a file dialog
		// and is allowed to hunt for the file? (Asger)
		// more that we have a layout for minimal.cls statically
		// compiled in... (Lgb)
	}

	if (!lex.setFile(real_file)) {
		lyxerr << "LyXTextClassList::Read: "
			"lyxlex was not able to set file: "
		       << real_file << endl;
	}
	
	if (!lex.IsOK()) {
		lyxerr << "LyXTextClassList::Read: unable to open "
			"textclass file  `" << MakeDisplayPath(real_file, 1000)
		       << "'\nCheck your installation. LyX can't continue."
		       << endl;
 		return false;
	}
	bool finished = false;
	string fname, clname, desc;
	// Parse config-file
	lyxerr[Debug::TCLASS] << "Starting parsing of textclass.lst" << endl;
	while (lex.IsOK() && !finished) {
		lyxerr[Debug::TCLASS] << "\tline by line" << endl;
		switch (lex.lex()) {
		case LyXLex::LEX_FEOF:
			finished = true;
			break;
		default:
			fname = lex.GetString();
			lyxerr[Debug::TCLASS] << "Fname: " << fname << endl;
			if (lex.next()) {
				clname = lex.GetString();
				lyxerr[Debug::TCLASS]
					<< "Clname: " << clname << endl;
				if (lex.next()) {
					      desc = lex.GetString();
					      lyxerr[Debug::TCLASS]
						      << "Desc: " << desc << endl;
					      // This code is run when we have
					      // fname, clname and desc
					      LyXTextClass tmpl(fname,
								clname,
								desc);
					      if (lyxerr.
						  debugging(Debug::TCLASS)) {
						      tmpl.load();
					      }
					      Add (tmpl);
				}
			}
		}
	}
	lyxerr[Debug::TCLASS] << "End of parsing of textclass.lst" << endl;

	if (classlist.size() == 0) {
		lyxerr << "LyXTextClassList::Read: no textclasses found!"
		       << endl;
		WriteAlert(_("LyX wasn't able to find any layout description!"),
			   _("Check the contents of  the file \"textclass.lst\""),
			   _("Sorry, has to exit :-("));
		return false;
	}
	// Ok everything loaded ok, now sort the list.
	sort(classlist.begin(), classlist.end(), less_textclass_desc());
	return true;
}

	
/* Load textclass
   Returns false if this fails
*/
bool
LyXTextClassList::Load (LyXTextClassList::size_type number) const
{
	bool result = true;
	if (number < classlist.size()) {
		classlist[number].load();
		if (classlist[number].numLayouts() == 0) {
			result = false;
		}
	} else {
		result = false;
	}
	return result;
}
