/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetquotes.h"
#include "support/lyxlib.h"
#include "debug.h"
#include "lyxfont.h"
#include "lyxrc.h"
#include "buffer.h"
#include "LaTeXFeatures.h"
#include "support/lstrings.h"

// Quotes. Used for the various quotes. German, English, French,
// Danish, Polish, all either double or single.

extern LyXRC * lyxrc;
extern BufferView * current_view;

// codes used to read/write quotes to LyX files
static char const * const language_char = "esgpfa";
static char const * const side_char = "lr" ;
static char const * const times_char = "sd";

// List of known quote chars
static char const * const quote_char = ",'`<>";

// Index of chars used for the quote. Index is [side, language]
int quote_index[2][6] = {
	{ 2, 1, 0, 0, 3, 4 },    // "'',,<>" 
	{ 1, 1, 2, 1, 4, 3 } };  // "`'`'><"

// Corresponding LaTeX code, for double and single quotes.
static char const * const latex_quote_t1[2][5] = 
{ { "\\quotesinglbase{}",  "'", "`", 
    "\\guilsinglleft{}", "\\guilsinglright{}" }, 
  { ",,", "''", "``", "<<", ">>" } };

static char const * const latex_quote_ot1[2][5] = 
{ { "\\quotesinglbase{}",  "'", "`", 
    "\\guilsinglleft{}", "\\guilsinglright{}" }, 
  { "\\quotedblbase{}", "''", "``",
    "\\guillemotleft{}", "\\guillemotright{}" } };

static char const * const latex_quote_babel[2][5] = 
{ { "\\glq{}",  "'", "`", "\\flq{}", "\\frq{}" },
  { "\\glqq{}", "''", "``", "\\flqq{}", "\\frqq{}" } };


InsetQuotes::InsetQuotes(string const & str)
{
	ParseString(str);
}


InsetQuotes::InsetQuotes(InsetQuotes::quote_language l,
			 InsetQuotes::quote_side s,
			 InsetQuotes::quote_times t)
	: language(l), side(s), times(t)
{
}


InsetQuotes::InsetQuotes(char c, BufferParams const & params)
	: language(params.quotes_language), times(params.quotes_times)
{
	// Decide whether left or right 
	switch(c) {
	case ' ': case '(': case '{': case '[': case '-': case ':':
	case LyXParagraph::META_HFILL:
	case LyXParagraph::META_PROTECTED_SEPARATOR:
	case LyXParagraph::META_NEWLINE: 
		side = InsetQuotes::LeftQ;   // left quote 
		break;
	default:
		side = InsetQuotes::RightQ;  // right quote
	}
}


void InsetQuotes::ParseString(string const & s)
{
	int i;
	string str(s);
	if (str.length() != 3) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad string length." << endl;
		str = "eld";
	}

	for (i = 0; i < 6; ++i) {
		if (str[0] == language_char[i]) {
			language = (InsetQuotes::quote_language)i;
			break;
		}
	}
	if (i >= 6) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad language specification." << endl;
		language = InsetQuotes::EnglishQ; 
	}

	for (i = 0; i < 2; ++i) {
		if (str[1] == side_char[i]) {
			side = (InsetQuotes::quote_side)i;
			break;
		}
	}
	if (i >= 2) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad side specification." << endl;
		side = InsetQuotes::LeftQ; 
	}

	for (i = 0; i < 2; ++i) {
		if (str[2] == times_char[i]) {
			times = (InsetQuotes::quote_times)i;
			break;
		}
	}
	if (i >= 2) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad times specification." << endl;
		times = InsetQuotes::DoubleQ; 
	}
}


string InsetQuotes::DispString() const
{
 	string disp;
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


int InsetQuotes::Ascent(LyXFont const & font) const
{
	return font.maxAscent();
}


int InsetQuotes::Descent(LyXFont const & font) const
{
	return font.maxDescent();
}


int InsetQuotes::Width(LyXFont const & font) const
{
	string text = DispString();
	int w = 0;

	for (string::size_type i = 0; i < text.length(); ++i) {
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


void InsetQuotes::Draw(LyXFont font, LyXScreen & scr,
		       int baseline, float & x)
{
	string text = DispString();

	scr.drawString(font, text, baseline, int(x));
	x += Width(font);
}


void InsetQuotes::Write(ostream & os)
{
	string text;
	text += language_char[language];
	text += side_char[side];
	text += times_char[times]; 
	os << "Quotes " << text;
}


void InsetQuotes::Read(LyXLex & lex)
{
	lex.nextToken();
	ParseString(lex.GetString());
}


int InsetQuotes::Latex(ostream & os, signed char /*fragile*/)
{
	string quote;
	int res = Latex(quote, 0);
	os << quote;
	return res;
}


int InsetQuotes::Latex(string & file, signed char /*fragile*/)
{
	string doclang = 
		current_view->buffer()->GetLanguage();
	int quoteind = quote_index[side][language];
	string qstr;
	
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
	if ((suffixIs(file, '?') || suffixIs(file, '!'))
	    && qstr[0] == '`')
		qstr = "{}" + qstr;

	file += qstr;
	return 0;
}


int InsetQuotes::Linuxdoc(string & file)
{
	file += "\"";

	return 0;
}


int InsetQuotes::DocBook(string & file)
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


void InsetQuotes::Validate(LaTeXFeatures & features) const 
{
	char type = quote_char[quote_index[side][language]];

	if (current_view->buffer()->GetLanguage() == "default" 
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


Inset * InsetQuotes::Clone() const
{
  return new InsetQuotes(language, side, times);
}


Inset::Code InsetQuotes::LyxCode() const
{
  return Inset::QUOTE_CODE;
}
