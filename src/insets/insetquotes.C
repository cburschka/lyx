/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetquotes.h"
#include "lyxlib.h"
#include "error.h"
#include "lyxfont.h"
#include "lyxrc.h"
#include "buffer.h"
#include "LaTeXFeatures.h"

// 	$Id: insetquotes.C,v 1.2 1999/10/02 14:01:04 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: insetquotes.C,v 1.2 1999/10/02 14:01:04 larsbj Exp $";
#endif /* lint */

// Quotes. Used for the various quotes. German, English, French,
// Danish, Polish, all either double or single.

extern LyXRC *lyxrc;
extern BufferView *current_view;

// codes used to read/write quotes to LyX files
static char const *language_char = "esgpfa";
static char const *side_char = "lr" ;
static char const *times_char ="sd";

// List of known quote chars
static char const *quote_char = ",'`<>";

// Index of chars used for the quote. Index is [side,language]
int quote_index[2][6] = 
{ { 2, 1, 0, 0, 3, 4 },    // "'',,<>" 
  { 1, 1, 2, 1, 4, 3 } };  // "`'`'><"

// Corresponding LaTeX code, for double and single quotes.
static char const *latex_quote_t1[2][5] =
{ { "\\quotesinglbase{}",  "'", "`", 
    "\\guilsinglleft{}", "\\guilsinglright{}" }, 
  { ",,", "''", "``", "<<", ">>" } };

static char const *latex_quote_ot1[2][5] =
{ { "\\quotesinglbase{}",  "'", "`", 
    "\\guilsinglleft{}", "\\guilsinglright{}" }, 
  { "\\quotedblbase{}", "''", "``",
    "\\guillemotleft{}", "\\guillemotright{}" } };

static char const *latex_quote_babel[2][5] =
{ { "\\glq{}",  "'", "`", "\\flq{}", "\\frq{}" },
  { "\\glqq{}", "''", "``", "\\flqq{}", "\\frqq{}" } };


InsetQuotes::InsetQuotes(LString const &string)
{
	ParseString(string);
}

InsetQuotes::InsetQuotes(InsetQuotes::quote_language l,
			 InsetQuotes::quote_side s,
			 InsetQuotes::quote_times t)
	: language(l), side(s), times(t)
{
}


InsetQuotes::InsetQuotes(char c, BufferParams const &params)
	: language(params.quotes_language), times(params.quotes_times)
{
	// Decide whether left or right 
	switch(c) {
	case ' ': case '(': case '{': case '[': case '-': case ':':
	case LYX_META_HFILL: case LYX_META_PROTECTED_SEPARATOR:
	case LYX_META_NEWLINE: 
		side = InsetQuotes::LeftQ;   // left quote 
		break;
	default:
		side = InsetQuotes::RightQ;  // right quote
	}
}


void InsetQuotes::ParseString(LString string)
{
	int i;
	if (string.length() != 3) {
		lyxerr.print("ERROR (InsetQuotes::InsetQuotes):"
			      " bad string length.");
		string = "eld";
	}

	for (i=0;i<6;i++) {
		if (string[0] == language_char[i]) {
			language = (InsetQuotes::quote_language)i;
			break;
		}
	}
	if (i>=6) {
		lyxerr.print("ERROR (InsetQuotes::InsetQuotes):"
			      " bad language specification.");
		language = InsetQuotes::EnglishQ; 
	}

	for (i=0;i<2;i++) {
		if (string[1] == side_char[i]) {
			side = (InsetQuotes::quote_side)i;
			break;
		}
	}
	if (i>=2) {
		lyxerr.print("ERROR (InsetQuotes::InsetQuotes):"
			      " bad side specification.");
		side = InsetQuotes::LeftQ; 
	}

	for (i=0;i<2;i++) {
		if (string[2] == times_char[i]) {
			times = (InsetQuotes::quote_times)i;
			break;
		}
	}
	if (i>=2) {
		lyxerr.print("ERROR (InsetQuotes::InsetQuotes):"
			      " bad times specification.");
		times = InsetQuotes::DoubleQ; 
	}
}

LString InsetQuotes::DispString() const
{
 	LString disp;

	disp += quote_char[quote_index[side][language]];
		
	if (times == InsetQuotes::DoubleQ)
		disp += disp;

 	if (lyxrc->font_norm == "iso8859-1") 
		if (disp == "<<")
			disp = '«';
		else if (disp == ">>")
			disp = '»';
	
 	return disp;
}


int InsetQuotes::Ascent(LyXFont const &font) const
{
	return font.maxAscent();
}


int InsetQuotes::Descent(LyXFont const &font) const
{
	return font.maxDescent();
}


int InsetQuotes::Width(LyXFont const &font) const
{
	LString text = DispString();
	int w = 0;

	for (int i = 0; i < text.length(); i++) {
		if (text[i] == ' ')
			w += font.width('i');
		else if (i == 0 || text[i] != text[i-1])
			w += font.width(text[i]);
		else
			w += font.width(',');
	}

	return w;
}


LyXFont InsetQuotes::ConvertFont(LyXFont font)
{
	/* quotes-insets cannot be latex of any kind */
	font.setLatex(LyXFont::OFF);
	return font;
}


void InsetQuotes::Draw(LyXFont font, LyXScreen &scr,
		       int baseline, float &x)
{
	LString text = DispString();

	for (int i = 0; i < text.length(); i++) {
		if (text[i] == ' ') 
			x += font.width('i');
		else if (i == text.length()-1 || text[i] != text[i+1]) {
			scr.drawString(font, &text[i], baseline, int(x));
			x += font.width(text[i]);
		} else {
			scr.drawString(font, &text[i+1], baseline, int(x));
			x += font.width(',');
		}
	}
	
}


void InsetQuotes::Write(FILE *file)
{
	LString text = LString(language_char[language]) + side_char[side] +
		       times_char[times]; 
	fprintf(file, "Quotes %s", text.c_str());
}


void InsetQuotes::Read(LyXLex &lex)
{
	lex.nextToken();
	ParseString(lex.GetString());
}


int InsetQuotes::Latex(FILE *file, signed char /*fragile*/)
{
	LString quote;
	int res = Latex(quote, 0);
	fprintf(file, "%s", quote.c_str()); 
	return res;
}

int InsetQuotes::Latex(LString &file, signed char /*fragile*/)
{
	LString doclang =
		current_view->currentBuffer()->GetLanguage();
	int quoteind = quote_index[side][language];
	LString qstr;
	
	if (lyxrc->fontenc == "T1") {
		qstr = latex_quote_t1[times][quoteind];
	}
	else if (doclang == "default") {
		qstr = latex_quote_ot1[times][quoteind];
	} 
	else if (language == InsetQuotes::FrenchQ 
		 && times == InsetQuotes::DoubleQ
		 && doclang == "frenchb") {
		if (side == InsetQuotes::LeftQ) 
			qstr = "\\og{}";
		else 
			qstr = " \\fg{}";
	} 
	else 
		qstr = latex_quote_babel[times][quoteind];

	// protect against !` and ?` ligatures.
	if ((file.suffixIs('?') || file.suffixIs('!'))
	    && qstr[0] == '`')
		qstr = "{}" + qstr;

	file += qstr;
	return 0;
}


int InsetQuotes::Linuxdoc(LString &file)
{
	file += "\"";

	return 0;
}


int InsetQuotes::DocBook(LString &file)
{
	if(times == InsetQuotes::DoubleQ) {
		if (side == InsetQuotes::LeftQ)
			file += "&ldquo;";
		else
			file += "&rdquo;";
	} else {
		if (side == InsetQuotes::LeftQ)
			file += "&lsquo;";
		else
			file += "&rsquo;";
	}
	return 0;
}


void InsetQuotes::Validate(LaTeXFeatures &features) const 
{
	char type = quote_char[quote_index[side][language]];

	if (current_view->currentBuffer()->GetLanguage() == "default" 
	    && lyxrc->fontenc != "T1") {
		if (times == InsetQuotes::SingleQ) 
			switch (type) {
			case ',': features.quotesinglbase = true; break;
			case '<': features.guilsinglleft = true; break;
			case '>': features.guilsinglright = true; break;
			default: break;
			}
		else 
			switch (type) {
			case ',': features.quotedblbase = true; break;
			case '<': features.guillemotleft = true; break;
			case '>': features.guillemotright = true; break;
			default: break;
			}
	}
}

Inset* InsetQuotes::Clone()
{
  return new InsetQuotes(language, side, times);
}


Inset::Code InsetQuotes::LyxCode() const
{
  return Inset::QUOTE_CODE;
}
