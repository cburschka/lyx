/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

/* Change log:
 * 
 *  14/11/1995,   Pascal André <andre@via.ecp.fr>
 *  Modified for external style definition. 
 * 
 *  15/11/1995,   Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Modified to use binary search and a small pseudo lexical analyzer.
 *  
 *  29/03/1996,  Dirk Niggeman
 *  Created classes LyXTextClass & LyXLayout.
 * 
 *  30/03/1996,  asierra
 *  Created class LyxLex and improved the lexical analyzer. 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "definitions.h"
#include <cstdlib>
#include "layout.h"
#include "lyxlex.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "error.h"
#include "gettext.h"

/* Global variable: textclass table */
LyXTextClassList lyxstyle;

// Reads the style files
void LyXSetStyle()
{
	lyxerr.debug("LyXSetStyle: parsing configuration...");
	
	if (!lyxstyle.Read()) {
		lyxerr.print("LyXSetStyle: an error occured during parsing.");
		lyxerr.print("             Exiting.");
		exit(1);
	}

	lyxerr.debug("LyXSetStyle: configuration parsed.");
}


//  The order of the LayoutTags enum is no more important. [asierra300396]
/* tags indexes */
enum _LayoutTags {
	LT_ALIGN, LT_ALIGNPOSSIBLE, 
	LT_BLOCK, LT_MARGIN,
	LT_BOTTOMSEP, LT_CENTER, LT_CENTERED_TOP_ENVIRONMENT, LT_COLUMNS,
	LT_COPYSTYLE, LT_OBSOLETEDBY,
	LT_COMMAND, LT_COUNTER_CHAPTER, LT_COUNTER_ENUMI, LT_COUNTER_ENUMII,
	LT_COUNTER_ENUMIII, LT_COUNTER_ENUMIV, LT_COUNTER_PARAGRAPH,
	LT_COUNTER_SECTION, LT_COUNTER_SUBPARAGRAPH, LT_COUNTER_SUBSECTION,
	LT_COUNTER_SUBSUBSECTION, LT_DEFAULTFONT, LT_DYNAMIC, LT_EMPTY,
	LT_END, LT_ENVIRONMENT, LT_ENVIRONMENT_DEFAULT, 
	LT_FANCYHDR, LT_FILL_BOTTOM, LT_FILL_TOP, LT_FIRST_COUNTER,
	LT_FIRST_DYNAMIC, LT_FONT, LT_FREE_SPACING, LT_HEADINGS, LT_INPUT,
	LT_ITEM_ENVIRONMENT, LT_ITEMSEP, LT_KEEPEMPTY,
	LT_LABEL_BOTTOMSEP, LT_LABELFONT, LT_TEXTFONT,
	LT_LABELINDENT, LT_LABELSEP, LT_LABELSTRING,
	LT_LABELSTRING_APPENDIX, LT_LABELTYPE,
	LT_LATEXNAME, LT_LATEXPARAM, LT_LATEXTYPE, LT_LAYOUT, LT_LEFT,
	LT_LEFTMARGIN,
	LT_LIST_ENVIRONMENT , LT_MANUAL, LT_MAXCOUNTER, 
	LT_NEED_PROTECT, LT_NEWLINE,
	LT_NEXTNOINDENT, LT_NO_LABEL, LT_NOSTYLE,
	LT_PAGESTYLE, LT_PARAGRAPH,
	LT_PARINDENT, LT_PARSEP, LT_PARSKIP, LT_PLAIN, LT_PREAMBLE, 
	LT_PROVIDESAMSMATH, LT_PROVIDESMAKEIDX, LT_PROVIDESURL, LT_RIGHT,
	LT_RIGHT_ADDRESS_BOX, LT_RIGHTMARGIN, LT_SENSITIVE, LT_SIDES,
	LT_SPACING, LT_SPACING_SINGLE, LT_SPACING_ONEHALF,
	LT_SPACING_DOUBLE, LT_OTHER,  LT_CLASSOPTIONS, LT_FONTSIZE,
	LT_STATIC, LT_STYLE, LT_TOP_ENVIRONMENT, LT_TOPSEP, LT_BIBLIO,
        LT_INTITLE, LT_SECNUMDEPTH, LT_TOCDEPTH,
	LT_OUTPUTTYPE, LT_OTLATEX, LT_OTLINUXDOC, LT_OTDOCBOOK, LT_OTLITERATE
};


// This table is sorted alphabetically [asierra 30March96]
static keyword_item layoutTags[] = {
	{ "align",			LT_ALIGN },
	{ "alignpossible",		LT_ALIGNPOSSIBLE },
	{ "bibliography",		LT_BIBLIO },
	{ "block",			LT_BLOCK },
	{ "bottomsep",			LT_BOTTOMSEP },
	{ "center",			LT_CENTER },
	{ "centered_top_environment",	LT_CENTERED_TOP_ENVIRONMENT },
        { "classoptions",               LT_CLASSOPTIONS },
	{ "columns",			LT_COLUMNS },
	{ "command",			LT_COMMAND },
	{ "copystyle",                  LT_COPYSTYLE },
	{ "counter_chapter",		LT_COUNTER_CHAPTER },
	{ "counter_enumi",		LT_COUNTER_ENUMI },
	{ "counter_enumii",		LT_COUNTER_ENUMII },
	{ "counter_enumiii",		LT_COUNTER_ENUMIII },
	{ "counter_enumiv",		LT_COUNTER_ENUMIV },
	{ "counter_paragraph",		LT_COUNTER_PARAGRAPH },
	{ "counter_section",		LT_COUNTER_SECTION },
	{ "counter_subparagraph",	LT_COUNTER_SUBPARAGRAPH },
	{ "counter_subsection",		LT_COUNTER_SUBSECTION },
	{ "counter_subsubsection",	LT_COUNTER_SUBSUBSECTION },
	{ "defaultfont", 		LT_DEFAULTFONT },
	{ "docbook",                    LT_OTDOCBOOK },
	{ "double",                     LT_SPACING_DOUBLE },
	{ "dynamic",			LT_DYNAMIC },
	{ "empty",			LT_EMPTY },
	{ "end",			LT_END },
	{ "environment",		LT_ENVIRONMENT },
	{ "environment_default",	LT_ENVIRONMENT_DEFAULT },
	{ "fancyhdr",			LT_FANCYHDR },
	{ "fill_bottom",		LT_FILL_BOTTOM },
	{ "fill_top",			LT_FILL_TOP },
	{ "first_counter",              LT_FIRST_COUNTER },
	{ "first_dynamic",              LT_FIRST_DYNAMIC },
	{ "font",                  	LT_FONT },
	{ "fontsize",                  	LT_FONTSIZE },
	{ "freespacing",	   	LT_FREE_SPACING },
	{ "headings",              	LT_HEADINGS },
	{ "input", 			LT_INPUT },
	{ "intitle",                    LT_INTITLE },
	{ "item_environment",     	LT_ITEM_ENVIRONMENT },
	{ "itemsep",               	LT_ITEMSEP },
	{ "keepempty",                  LT_KEEPEMPTY },
	{ "labelbottomsep",             LT_LABEL_BOTTOMSEP },
	{ "labelfont",             	LT_LABELFONT },
	{ "labelindent",           	LT_LABELINDENT },
	{ "labelsep",              	LT_LABELSEP },
	{ "labelstring",           	LT_LABELSTRING },
	{ "labelstringappendix",           	LT_LABELSTRING_APPENDIX },
	{ "labeltype",             	LT_LABELTYPE },
	{ "latex",                      LT_OTLATEX },
	{ "latexname",             	LT_LATEXNAME },
	{ "latexparam",			LT_LATEXPARAM },    //arrae970411
	{ "latextype",             	LT_LATEXTYPE },
	{ "layout",                	LT_LAYOUT },
	{ "left",                  	LT_LEFT },
	{ "leftmargin",            	LT_LEFTMARGIN },
	{ "linuxdoc",                   LT_OTLINUXDOC },
	{ "list_environment",     	LT_LIST_ENVIRONMENT },
	{ "literate",                   LT_OTLITERATE },
	{ "manual",                	LT_MANUAL },
	{ "margin",                	LT_MARGIN },
	{ "maxcounter",            	LT_MAXCOUNTER },
	{ "needprotect",                LT_NEED_PROTECT },
	{ "newline",			LT_NEWLINE },
	{ "nextnoindent",		LT_NEXTNOINDENT },
	{ "no_label",			LT_NO_LABEL },
	{ "nostyle",                    LT_NOSTYLE },
	{ "obsoletedby",                LT_OBSOLETEDBY },
	{ "onehalf",                    LT_SPACING_ONEHALF },
	{ "other",                      LT_OTHER },
	{ "outputtype",                 LT_OUTPUTTYPE },
	{ "pagestyle",			LT_PAGESTYLE },
	{ "paragraph",			LT_PARAGRAPH },
	{ "parindent",			LT_PARINDENT },
	{ "parsep",			LT_PARSEP },
	{ "parskip",			LT_PARSKIP },
	{ "plain",			LT_PLAIN },
	{ "preamble",                   LT_PREAMBLE },
	{ "providesamsmath",		LT_PROVIDESAMSMATH },
	{ "providesmakeidx",		LT_PROVIDESMAKEIDX },
	{ "providesurl",		LT_PROVIDESURL },
	{ "right",			LT_RIGHT },
	{ "right_address_box",		LT_RIGHT_ADDRESS_BOX },
	{ "rightmargin",		LT_RIGHTMARGIN },
	{ "secnumdepth",                LT_SECNUMDEPTH },
	{ "sensitive",			LT_SENSITIVE },
	{ "sides",			LT_SIDES },
	{ "single",                     LT_SPACING_SINGLE },
	{ "spacing",                    LT_SPACING },
	{ "static",			LT_STATIC },
	{ "style",			LT_STYLE },
	{ "textfont",                   LT_TEXTFONT },
	{ "tocdepth",                   LT_TOCDEPTH },
	{ "top_environment",		LT_TOP_ENVIRONMENT },
	{ "topsep",			LT_TOPSEP }
};


/* ******************************************************************* */

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
	// Should or should not. That is the question.
	// spacing.set(Spacing::OneHalf);
	fill_top = false;
	fill_bottom = false;
	newline_allowed = true;
	free_spacing = false;
}


LyXLayout::~LyXLayout ()
{
}


void LyXLayout::Copy (LyXLayout const &l)
{
	name = l.name;
	obsoleted_by = l.obsoleted_by;
	margintype = l.margintype;
	latextype = l.latextype;
	intitle = l.intitle;
	needprotect = l.needprotect;
	keepempty = l.keepempty;
	latexname = l.latexname;
	latexparam = l.latexparam;   //arrae970411
	preamble = l.preamble;
	font = l.font;
	labelfont = l.labelfont;
	resfont = l.resfont;
	reslabelfont = l.reslabelfont;
	nextnoindent = l.nextnoindent;
	leftmargin = l.leftmargin;
	rightmargin = l.rightmargin;
	labelsep = l.labelsep;
	labelindent = l.labelindent;
	parindent = l.parindent;
	parskip = l.parskip;
	itemsep = l.itemsep;
	topsep = l.topsep;
	bottomsep = l.bottomsep;
	labelbottomsep = l.labelbottomsep;
	parsep = l.parsep;
	align = l.align;
	alignpossible = l.alignpossible;
	labeltype = l.labeltype;
	spacing = l.spacing;
	labelstring = l.labelstring;
	labelstring_appendix = l.labelstring_appendix;
	fill_top = l.fill_top;
	fill_bottom = l.fill_bottom;
	newline_allowed = l.newline_allowed;
	free_spacing = l.free_spacing;
}


/* Reads a layout definition from file */
bool LyXLayout::Read (LyXLex & lexrc, LyXLayoutList * list)
{
	bool error = false;
	bool finished = false;
	
	/* parse style section */
	while (!finished && lexrc.IsOK() && !error) {
		switch(lexrc.lex()) {

		case -2:
			break;

		case -1:		/* parse error */
			lexrc.printError("Unknown tag `$$Token'");
			error = true;
			break;

		case LT_END:		/* end of structure */
			finished = true;
			break;

		case LT_COPYSTYLE:     // initialize with a known style
		        if (lexrc.next()) {
		        	LyXLayout * layout = list->GetLayout(lexrc.GetString());
				if (layout) {
					string tmpname = name;
					Copy(*layout);
					name = tmpname;
				} else {
					lexrc.printError("Cannot copy unknown "
                                                 "style `$$Token'");
				}
			}
			break;

		case LT_OBSOLETEDBY:     // replace with a known style
		        if (lexrc.next()) {
		        	LyXLayout * layout = list->GetLayout(lexrc.GetString());
				if (layout) {
					string tmpname = name;
					Copy(*layout);
					name = tmpname;
					if (obsoleted_by.empty())
						obsoleted_by = lexrc.GetString();
				} else {
					lexrc.printError("Cannot replace with" 
							 " unknown style "
							 "`$$Token'");
				}
			}
			break;

		case LT_MARGIN:		/* margin style definition */
		       
			switch(lexrc.lex()) {
			case LT_STATIC:
				margintype = MARGIN_STATIC;
				break;
			case LT_MANUAL:
				margintype = MARGIN_MANUAL;
				break;
			case LT_DYNAMIC:
				margintype = MARGIN_DYNAMIC;
				break;
			case LT_FIRST_DYNAMIC:
				margintype = MARGIN_FIRST_DYNAMIC;
				break;
			case LT_RIGHT_ADDRESS_BOX:
				margintype = MARGIN_RIGHT_ADDRESS_BOX;
				break;
			default:
				lexrc.printError("Unknown margin type `$$Token'");
				break;
			}
			break;

		case LT_LATEXTYPE:	/* latex style definition */
		        switch (lexrc.lex()) {
			case LT_PARAGRAPH:
				latextype=LATEX_PARAGRAPH;
				break;
			case LT_COMMAND:
				latextype=LATEX_COMMAND;
				break;
			case LT_ENVIRONMENT:
				latextype=LATEX_ENVIRONMENT;
				break;
			case LT_ITEM_ENVIRONMENT:
				latextype=LATEX_ITEM_ENVIRONMENT;
				break;
			case LT_LIST_ENVIRONMENT:
				latextype=LATEX_LIST_ENVIRONMENT;
				break;
			default:
				lexrc.printError("Unknown latextype `$$Token'");
				break;
			}
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
			labelfont=font;
			break;

		case LT_TEXTFONT:
			font.lyxRead(lexrc);
			break;

		case LT_LABELFONT:
			labelfont.lyxRead(lexrc);
			break;

		case LT_NEXTNOINDENT:	/* indent next paragraph ? */
			if (lexrc.next() && lexrc.GetInteger())
				nextnoindent = true;
			else
				nextnoindent = false;
			break;

		case LT_LATEXNAME:	/* latex name */
		        if (lexrc.next())
		                latexname = lexrc.GetString();
			break;
                        
                //arrae970411
		case LT_LATEXPARAM:	/* latex parameter */
			if (lexrc.next())
				latexparam = lexrc.GetString();
			break;

		case LT_PREAMBLE:
			preamble = lexrc.getLongString("EndPreamble");
			break;

		case LT_LABELTYPE:	/* label type */
		        switch (lexrc.lex()) {
			case LT_NO_LABEL:
				labeltype = LABEL_NO_LABEL;
				break;
			case LT_MANUAL:
				labeltype = LABEL_MANUAL;
				break;
			case LT_TOP_ENVIRONMENT:
				labeltype = LABEL_TOP_ENVIRONMENT;
				break;
			case LT_CENTERED_TOP_ENVIRONMENT:
				labeltype = LABEL_CENTERED_TOP_ENVIRONMENT;
				break;
			case LT_STATIC:
				labeltype = LABEL_STATIC;
				break;
			case LT_SENSITIVE:
				labeltype = LABEL_SENSITIVE;
				break;
			case LT_COUNTER_CHAPTER:
				labeltype = LABEL_COUNTER_CHAPTER;
				break;
			case LT_COUNTER_SECTION:
				labeltype = LABEL_COUNTER_SECTION;
				break;
			case LT_COUNTER_SUBSECTION:
				labeltype = LABEL_COUNTER_SUBSECTION;
				break;
			case LT_COUNTER_SUBSUBSECTION:
				labeltype = LABEL_COUNTER_SUBSUBSECTION;
				break;
			case LT_COUNTER_PARAGRAPH:
				labeltype = LABEL_COUNTER_PARAGRAPH;
				break;
			case LT_COUNTER_SUBPARAGRAPH:
				labeltype = LABEL_COUNTER_SUBPARAGRAPH;
				break;
			case LT_COUNTER_ENUMI:
				labeltype = LABEL_COUNTER_ENUMI;
				break;
			case LT_COUNTER_ENUMII:
				labeltype = LABEL_COUNTER_ENUMII;
				break;
			case LT_COUNTER_ENUMIII:
				labeltype = LABEL_COUNTER_ENUMIII;
				break;
			case LT_COUNTER_ENUMIV:
				labeltype = LABEL_COUNTER_ENUMIV;
				break;
			case LT_BIBLIO:
				labeltype = LABEL_BIBLIO;
				break;
			default:
				lexrc.printError("Unknown labeltype `$$Token'");
			}
			break;

		case LT_LEFTMARGIN:	/* left margin type */
		        if (lexrc.next())
				leftmargin = lexrc.GetString();
			break;			

		case LT_RIGHTMARGIN:	/* right margin type */
			if (lexrc.next())
				rightmargin = lexrc.GetString();
			break;

		case LT_LABELINDENT:	/* label indenting flag */
			if (lexrc.next())
				labelindent = lexrc.GetString();
			break;

		case LT_PARINDENT:	/* paragraph indent. flag */
			if (lexrc.next())
				parindent = lexrc.GetString();
			break;

		case LT_PARSKIP:	/* paragraph skip size */
			if (lexrc.next())
				parskip = lexrc.GetFloat();
			break;

		case LT_ITEMSEP:	/* item separation size */
			if (lexrc.next())
				itemsep = lexrc.GetFloat();
			break;

		case LT_TOPSEP:		/* top separation size */
			if (lexrc.next())
				topsep = lexrc.GetFloat();
			break;

		case LT_BOTTOMSEP:	/* bottom separation size */
			if (lexrc.next())
				bottomsep = lexrc.GetFloat();
			break;

		case LT_LABEL_BOTTOMSEP:/* label bottom separation size */
			if (lexrc.next())
				labelbottomsep = lexrc.GetFloat();
			break;

		case LT_LABELSEP:	/* label separator */
			if (lexrc.next()) {
				labelsep = lexrc.GetString();
				subst(labelsep, 'x', ' ');
			}
			break;

		case LT_PARSEP:		/* par. separation size */
			if (lexrc.next())
				parsep = lexrc.GetFloat();
			break;

		case LT_FILL_TOP:	/* fill top flag */
			if (lexrc.next())
				fill_top = lexrc.GetInteger();
			break;

		case LT_FILL_BOTTOM:	/* fill bottom flag */
			if (lexrc.next())
				fill_bottom = lexrc.GetInteger();
			break;

		case LT_NEWLINE:	/* newlines allowed ? */
			if (lexrc.next())
				newline_allowed = lexrc.GetInteger();
			break;

		case LT_ALIGN:		/* paragraph align */
			switch (lexrc.lex()) {
			case LT_BLOCK:
				align = LYX_ALIGN_BLOCK;
				break;
			case LT_LEFT:
				align = LYX_ALIGN_LEFT;
				break;
			case LT_RIGHT:
				align = LYX_ALIGN_RIGHT;
				break;
			case LT_CENTER:
				align = LYX_ALIGN_CENTER;
				break;
			case LT_LAYOUT:
				align = LYX_ALIGN_LAYOUT;
				break;
			default:
				lexrc.printError("Unknown alignment `$$Token'");
			}
			break;

		case LT_ALIGNPOSSIBLE:	/* paragraph allowed align */
		{	alignpossible = 0;
		      
		int lineno = lexrc.GetLineNo();
		do {
			switch (lexrc.lex()) {
			case LT_BLOCK:
				alignpossible |= LYX_ALIGN_BLOCK;
				break;
			case LT_LEFT:
				alignpossible |= LYX_ALIGN_LEFT;
				break;
			case LT_RIGHT:
				alignpossible |= LYX_ALIGN_RIGHT;
				break;
			case LT_CENTER:
				alignpossible |= LYX_ALIGN_CENTER;
				break;
			case LT_LAYOUT:
				alignpossible |= LYX_ALIGN_LAYOUT;
				break;
			default:
				lexrc.printError("Unknown alignment `$$Token'");

			}
		} while (lineno==lexrc.GetLineNo());
		break;
		}

		case LT_LABELSTRING:	/* label string definition */
			if (lexrc.next())
				labelstring = lexrc.GetString();
			break;

		case LT_LABELSTRING_APPENDIX:	/* label string appendix definition */
			if (lexrc.next())
				labelstring_appendix = lexrc.GetString();
			break;

		case LT_FREE_SPACING:	/* Allow for free spacing. */
			if (lexrc.next())
				free_spacing = lexrc.GetInteger();
			break;

		case LT_SPACING: // setspace.sty
			switch(lexrc.lex()) {
			case LT_SPACING_SINGLE:
				spacing.set(Spacing::Single);
				//spacing_value = 1.0;
				break;
			case LT_SPACING_ONEHALF:
				spacing.set(Spacing::Onehalf);
				//spacing_value = 1.25;
				break;
			case LT_SPACING_DOUBLE:
				spacing.set(Spacing::Double);
				//spacing_value = 1.667;
				break;
			case LT_OTHER:
				lexrc.next();
				spacing.set(Spacing::Other, lexrc.GetFloat());
				break;
			default:
				lexrc.printError("Unknown spacing `$$Token'");
			}
			break;
		default:		/* context error */
			lexrc.printError("Tag `$$Token' is not "
					 "allowed in layout");
			error = true;
			break;
		}
	}

	return error;
}

/* ******************************************************************* */

LyXLayoutList::LyXLayoutList()
{
	l = 0;
	eol = 0;
	num_layouts = 0;
}


LyXLayoutList::~LyXLayoutList()
{
	//don't do anything. the layouts will be extracted by ToAr.
	//destruction is done by Clean in emergencies
}


int LyXLayoutList::GetNum ()
{
	return num_layouts;
}


void LyXLayoutList::Add (LyXLayout *lay)
{
	LyXLayoutL * tmp = new LyXLayoutL;
	tmp->layout = lay;
	tmp->next = 0;
	if (!eol) l = tmp; 
	else eol->next = tmp;
	eol = tmp;
	num_layouts++;
}


bool LyXLayoutList::Delete (string const &name)
{
	LyXLayoutL * layoutl = l;
	while(layoutl) {
	  	if (layoutl->layout && layoutl->layout->name == name) {
			delete layoutl->layout;
			layoutl->layout = 0; // not sure it is necessary
			num_layouts--;
			return true;
		}
		layoutl = layoutl->next;
	}
	return false;
}


LyXLayout * LyXLayoutList::GetLayout (string const &name)
{
	LyXLayoutL * layoutl = l;
	while(layoutl) {
	  	if (layoutl->layout && layoutl->layout->name == name) 
			return layoutl->layout;
		layoutl = layoutl->next;
	}
	return 0;
}


LyXLayout * LyXLayoutList::ToAr ()
{
	LyXLayoutL * lp, * op;
	int idx = 0;
	LyXLayout* ar = new LyXLayout [num_layouts];
	lp = l;
	while (lp) {
		if (lp->layout) {
			ar[idx].Copy (*lp->layout);
			idx++;
			delete lp->layout;
		}
		op = lp;
		lp = lp->next;
		delete op;
	}
	return ar;
}


//wipe up any dead layouts
void LyXLayoutList::Clean ()
{
	LyXLayoutL * lp, * op;
	lp = l;
	while (lp) {
		delete lp->layout;
		op = lp;
		lp = lp->next;
		delete op;
	}
}

/* ******************************************************************* */

LyXTextClass::LyXTextClass(string const &fn, string const &cln,
			   string const &desc)
{
	name = fn;
	latexname = cln;
	description = desc;
	output_type = LATEX;
	style = 0;
	columns = 1;
	sides = 1;
	secnumdepth = 3;
	tocdepth = 3;
	pagestyle = "default";
	maxcounter = LABEL_COUNTER_CHAPTER;
	defaultfont = LyXFont(LyXFont::ALL_SANE);
	number_of_defined_layouts = 0;
	opt_fontsize = "10|11|12";
	opt_pagestyle = "empty|plain|headings|fancy";
	provides_amsmath = false;
	provides_makeidx = false;
	provides_url = false;
	loaded = false;
}


// This is not a proper copy.
// It just references the style rather than copying it!
void LyXTextClass::Copy (LyXTextClass const &l)
{
	name = l.name;
	latexname = l.latexname;
	description = l.description;
	output_type = l.output_type;
	preamble = l.preamble;
	options = l.options;
	if (style) delete style;
	style = l.style; //just aliases NO COPY
	number_of_defined_layouts = l.number_of_defined_layouts;
	columns = l.columns;
	sides = l.sides;
	secnumdepth = l.secnumdepth;
	tocdepth = l.tocdepth;
	pagestyle = l.pagestyle;
	maxcounter = l.maxcounter;
	defaultfont = l.defaultfont;
        opt_fontsize = l.opt_fontsize;
        opt_pagestyle = l.opt_pagestyle;
	provides_amsmath = l.provides_amsmath;
	provides_makeidx = l.provides_makeidx;
	provides_url = l.provides_url;
        loaded = l.loaded;

	leftmargin = l.leftmargin;
	rightmargin = l.rightmargin;
	  
}


LyXTextClass::~LyXTextClass()
{
	//we can't delete the style here because otherwise 
	//our list classes wouldn't work
}


/* Reads a textclass structure from file */
int LyXTextClass::Read (string const &filename, LyXLayoutList *list)
{
	if (!list)
		lyxerr.debug("Reading textclass "
			     + MakeDisplayPath(filename), Error::TCLASS);
	else 
		lyxerr.debug("Reading input file "
			     + MakeDisplayPath(filename), Error::TCLASS);

	LyXLex lexrc(layoutTags, sizeof(layoutTags)/sizeof(keyword_item));
	bool error = false;

        lexrc.setFile(filename);
	if (!lexrc.IsOK()) return -2; 

	LyXLayoutList * l;
	LyXLayout * tmpl;

	if (list) 
		l = list;
	else 
		l = new LyXLayoutList;

	/* parsing */
	while (lexrc.IsOK() && !error) {
		switch(lexrc.lex()) {
		case -2:
			break;

		case -1:                                 
			lexrc.printError("Unknown tag `$$Token'");
			error = true;
			break;

		case LT_OUTPUTTYPE:   // output type definition
		        switch(lexrc.lex()) {
			case LT_OTLATEX:
			        output_type=LATEX;
				break;
			case LT_OTLINUXDOC:
			        output_type=LINUXDOC;
				break;
			case LT_OTDOCBOOK:
			        output_type=DOCBOOK;
				break;
			case LT_OTLITERATE:
			        output_type=LITERATE;
				break;
			default:
			        lexrc.printError("Unknown output type `$$Token'");
				break;
			}
			break;
			
		case LT_INPUT: // Include file
		        if (lexrc.next()) {
		        	string tmp = LibFileSearch("layouts",
							    lexrc.GetString(), 
							    "layout");
				
				if (Read(tmp, l)) {
					lexrc.printError("Error reading input"
							 "file: "+tmp);
					error = true;
				}
			}
			break;

		case LT_STYLE:
			if (lexrc.next()) {
				string name = lexrc.GetString();
				bool is_new = false;

				subst(name, '_',' ');
				tmpl = l->GetLayout(name);
				if (!tmpl) {
					is_new = true;
					tmpl = new LyXLayout;
					tmpl->name = name;
				}

				lyxerr.debug("  Reading style "+tmpl->name, Error::TCLASS);

				if (!tmpl->Read(lexrc, l)) {
					// Resolve fonts
					tmpl->resfont = tmpl->font;
					tmpl->resfont.realize(defaultfont);
					tmpl->reslabelfont = tmpl->labelfont;
					tmpl->reslabelfont.realize(defaultfont);
					if (is_new) {
						l->Add (tmpl);
						// NB! we don't delete because 
						// we just pass it in.... 
					}
				} else {
				        lexrc.printError(
					               "Error parsing style `"
						       +tmpl->name+'\'');
					error = true;
					if (is_new) {
						delete tmpl;  
						//we delete dead ones here
					}
				}
			}
			else {
				lexrc.printError("No name given for style: `$$Token'.");
				error = true;
			}
			break;

		case LT_NOSTYLE:
			if (lexrc.next()) {
				string style = lexrc.GetString();
				if (!l->Delete(subst(style, '_', ' ')))
					lexrc.printError("Cannot delete style `$$Token'");
			}
			break;

		case LT_COLUMNS:
			if (lexrc.next())
				columns = lexrc.GetInteger();
			break;
			
		case LT_SIDES:
			if (lexrc.next())
				sides = lexrc.GetInteger();
			break;
			
		case LT_PAGESTYLE:
		        lexrc.next();
			pagestyle = strip(lexrc.GetString());
			break;
			
		case LT_DEFAULTFONT:
			defaultfont.lyxRead(lexrc);
			if (!defaultfont.resolved()) {
				lexrc.printError("Warning: defaultfont should "
						 "be fully instantiated!");
				defaultfont.realize(LyXFont::ALL_SANE);
			}
			break;

		case LT_MAXCOUNTER:
			switch (lexrc.lex()) {
			case LT_COUNTER_CHAPTER:
				maxcounter = LABEL_COUNTER_CHAPTER;
				break;
			case LT_COUNTER_SECTION:
				maxcounter = LABEL_COUNTER_SECTION;
				break;
			case LT_COUNTER_SUBSECTION:
				maxcounter = LABEL_COUNTER_SUBSECTION;
				break;
			case LT_COUNTER_SUBSUBSECTION:
				maxcounter = LABEL_COUNTER_SUBSUBSECTION;
				break;
			case LT_COUNTER_PARAGRAPH:
				maxcounter = LABEL_COUNTER_PARAGRAPH;
				break;
			case LT_COUNTER_SUBPARAGRAPH:
				maxcounter = LABEL_COUNTER_SUBPARAGRAPH;
				break;
			case LT_COUNTER_ENUMI:
				maxcounter = LABEL_COUNTER_ENUMI;
				break;
			case LT_COUNTER_ENUMII:
				maxcounter = LABEL_COUNTER_ENUMII;
				break;
			case LT_COUNTER_ENUMIII:
				maxcounter = LABEL_COUNTER_ENUMIII;
				break;
			case LT_COUNTER_ENUMIV:
				maxcounter = LABEL_COUNTER_ENUMIV;
				break;
			}
			break;

		case LT_SECNUMDEPTH:
			lexrc.next();
			secnumdepth = lexrc.GetInteger();
			break;

		case LT_TOCDEPTH:
			lexrc.next();
			tocdepth = lexrc.GetInteger();
			break;

	 // First step to support options 
	        case LT_CLASSOPTIONS:
	        {
	                bool getout = true;
	                while (getout && lexrc.IsOK()) { 
				switch (lexrc.lex()) {
				case LT_FONTSIZE:
					lexrc.next();
					opt_fontsize = strip(lexrc.GetString());
					break;
				case LT_PAGESTYLE:
					lexrc.next();
					opt_pagestyle = strip(lexrc.GetString()); 
					break;
				case LT_OTHER:
					lexrc.next();
					options = lexrc.GetString();
					break;
				case LT_END: getout = false; break;
				default:
					lexrc.printError("Out of context tag `$$Token'");
					break;
				}
			}
		        break;
		}

		case LT_PREAMBLE:
			preamble = lexrc.getLongString("EndPreamble");
			break;

		case LT_PROVIDESAMSMATH:
			if (lexrc.next())
				provides_amsmath = lexrc.GetInteger();
			break;

		case LT_PROVIDESMAKEIDX:
			if (lexrc.next())
				provides_makeidx = lexrc.GetInteger();
			break;

		case LT_PROVIDESURL:
			if (lexrc.next())
				provides_url = lexrc.GetInteger();
			break;

		case LT_LEFTMARGIN:	/* left margin type */
		        if (lexrc.next())
				leftmargin = lexrc.GetString();
			break;			

		case LT_RIGHTMARGIN:	/* right margin type */
			if (lexrc.next())
				rightmargin = lexrc.GetString();
			break;

		default:
			lexrc.printError("Out of context tag `$$Token'");
			break;
		}
	}	

	if (!list) { // we are at top level here.
		if (error) {
			number_of_defined_layouts = 0;
			l->Clean(); //wipe any we may have found
			delete l;
		}
		else {
			style = l->ToAr();
			number_of_defined_layouts = l->GetNum();
			delete l;
		}
		lyxerr.debug("Finished reading textclass " 
			     + MakeDisplayPath(filename), Error::TCLASS);
	}
	else
		lyxerr.debug("Finished reading input file " 
			     + MakeDisplayPath(filename), Error::TCLASS);

	return error;
}


// Load textclass info if not loaded yet
void LyXTextClass::load()
{
	if (loaded)
		return;

	// Read style-file
	string real_file = LibFileSearch("layouts", name, "layout");

	if (Read(real_file)) {
		lyxerr.print("Error reading `"
			     + MakeDisplayPath(real_file) + '\'');
		lyxerr.print("(Check `" + name + "')");
		lyxerr.print("Check your installation and "
			     "try Options/Reconfigure...");
	}
	loaded = true;
}

/* ******************************************************************* */

LyXTextClassList::LyXTextClassList()
{
	l = 0;
	ar = 0;
	num_textclass = 0;
}


LyXTextClassList::~LyXTextClassList()
{
	// The textclass list is in ar.
	if (ar) {
		delete [] ar;
	}
}


// Gets textclass number from name
signed char LyXTextClassList::NumberOfClass(string const &textclass) 
{
	int i = 0;
   
	while (i < num_textclass && textclass != ar[i].name)
		i++;
   
	if (i >= num_textclass)
		i = -1;

	return i;
}


// Gets layout structure from style number and textclass number
LyXLayout *LyXTextClassList::Style(char textclass, char layout) 
{
	ar[textclass].load();

	if (layout < ar[textclass].number_of_defined_layouts)
		return &ar[textclass].style[layout];
	else {
		return &ar[textclass].style[0];
	};
}


// Gets layout number from name and textclass number
char LyXTextClassList::NumberOfLayout(char textclass, string const &name) 
{
	ar[textclass].load();

	int i = 0;
	while (i < ar[textclass].number_of_defined_layouts 
	       && name != ar[textclass].style[i].name)
		i++;

	if (i >= ar[textclass].number_of_defined_layouts) {
		if (name == "dummy")
			i = LYX_DUMMY_LAYOUT;
		else
			// so that we can detect if the layout doesn't exist.
			i = -1; // not found
	} 
	return i;
}


// Gets a layout (style) name from layout number and textclass number
string LyXTextClassList::NameOfLayout(char textclass, char layout) 
{
	ar[textclass].load();

	if (layout < ar[textclass].number_of_defined_layouts)
		return ar[textclass].style[layout].name;
	else if (layout == LYX_DUMMY_LAYOUT)
		return "dummy";
	else
		return "@@end@@";
}


// Gets a textclass name from number
string LyXTextClassList::NameOfClass(char number) 
{
	if (num_textclass == 0) { 
		if (number == 0) return "dummy";
		else return "@@end@@";
	}
	if (number < num_textclass)
		return ar[number].name;
	else
		return "@@end@@";
}

// Gets a textclass latexname from number
string LyXTextClassList::LatexnameOfClass(char number) 
{
	ar[number].load();

	if (num_textclass == 0) { 
		if (number == 0) return "dummy";
		else return "@@end@@";
	}
	if (number < num_textclass)
		return ar[number].latexname;
	else
		return "@@end@@";
}

// Gets a textclass description from number
string LyXTextClassList::DescOfClass(char number) 
{
	if (num_textclass == 0) { 
		if (number == 0) return "dummy";
		else return "@@end@@";
	}
	if (number < num_textclass)
		return ar[number].description;
	else
		return "@@end@@";
}


// Gets a textclass structure from number
LyXTextClass * LyXTextClassList::TextClass(char textclass) 
{
	ar[textclass].load();
	if (textclass < num_textclass)
		return &ar[textclass];
	else
		return &ar[0];
}


void LyXTextClassList::Add (LyXTextClass *t)
{
	LyXTextClassL ** h = &l;
	string const desc = t->description;
	while (*h && compare_no_case((*h)->textclass->description, desc) < 0)
		h = &((*h)->next);
	LyXTextClassL * tmp = new LyXTextClassL;
	tmp->textclass = t;
	tmp->next = *h;
	*h = tmp;
	num_textclass++;
}


void LyXTextClassList::ToAr ()
{
	LyXTextClassL * lp, *op;
	int idx = 0;
	ar = new LyXTextClass [num_textclass];
	lp = l;
	while (lp) {
		ar[idx].Copy (*lp->textclass);
		idx++;
		delete lp->textclass; // note we don't delete layouts
				      // here at all 
		op = lp;
		lp = lp->next;
		delete op;
	}
}


// Reads LyX textclass definitions according to textclass config file
bool LyXTextClassList::Read ()
{
	LyXLex lex(0, 0);
	string real_file = LibFileSearch("", "textclass.lst");
	lyxerr.debug("Reading textclasses from "+real_file,Error::TCLASS);

	if (real_file.empty()) {
		lyxerr.print("LyXTextClassList::Read: unable to find "
			      "textclass file  `" +
			      MakeDisplayPath(real_file, 1000) + "'. Exiting.");

		WriteAlert(_("LyX wasn't able to find its layout descriptions!"),
			   _("Check that the file \"textclass.lst\""),
			   _("is installed correctly. Sorry, has to exit :-("));
		return false;
		// This causes LyX to end... Not a desirable behaviour. Lgb
		// What do you propose? That the user gets a file dialog
		// and is allowed to hunt for the file? (Asger)
	}

	lex.setFile(real_file);
	
	if (!lex.IsOK()) {
		lyxerr.print("LyXTextClassList::Read: unable to open "
			      "textclass file  `" +
			      MakeDisplayPath(real_file, 1000) + '\'');
		lyxerr.print("Check your installation. LyX can't continue.");
 		return false;
	}
	bool finished = false;
	string fname, clname, desc;
	LyXTextClass * tmpl;

	// Parse config-file
	while (lex.IsOK() && !finished) {
		switch (lex.lex()) {
		case LyXLex::LEX_FEOF:
			finished = true;
			break;
		default:
			fname = lex.GetString();
			lyxerr.debug("Fname: " + fname, Error::TCLASS);
			if (lex.next()) {
				clname = lex.GetString();
				lyxerr.debug("Clname: " + clname,
					     Error::TCLASS);
				if (lex.next()) {
					      desc = lex.GetString();
					      lyxerr.debug("Desc: " + desc,
							   Error::TCLASS);
					      // This code is run when we have
					      // fname, clname and desc
					      tmpl =new LyXTextClass(fname,
								     clname,
								     desc);
					      Add (tmpl);
					      if (lyxerr.
						  debugging(Error::TCLASS)) {
					            tmpl->load();
					      }
				}
			}
		}
	}
	
	if (num_textclass == 0) {
		lyxerr.print("LyXTextClassList::Read: no textclass found!");
		WriteAlert(_("LyX wasn't able to find any layout description!"),
			   _("Check the contents of  the file \"textclass.lst\""),
			   _("Sorry, has to exit :-("));
		return false;
	}
	else { 
		ToAr();
		return true;
	}
}

// Load textclass
/* Returns false if this fails */
bool LyXTextClassList::Load (char const number)
{
	bool result = 1;
	
	if (number < num_textclass) {
		ar[number].load();
		if (!ar[number].number_of_defined_layouts) {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}
