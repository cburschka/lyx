/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
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
#include "Painter.h"
#include "font.h"
#include "language.h"
#include "BufferView.h"

using std::ostream;
using std::endl;

// Quotes. Used for the various quotes. German, English, French,
// Danish, Polish, all either double or single.

namespace {

// codes used to read/write quotes to LyX files
char const * const language_char = "esgpfa";
char const * const side_char = "lr" ;
char const * const times_char = "sd";

// List of known quote chars
char const * const quote_char = ",'`<>";

// Index of chars used for the quote. Index is [side, language]
int quote_index[2][6] = {
	{ 2, 1, 0, 0, 3, 4 },    // "'',,<>" 
	{ 1, 1, 2, 1, 4, 3 } };  // "`'`'><"

// Corresponding LaTeX code, for double and single quotes.
char const * const latex_quote_t1[2][5] = 
{ { "\\quotesinglbase{}",  "'", "`", 
    "\\guilsinglleft{}", "\\guilsinglright{}" }, 
  { ",,", "''", "``", "<<", ">>" } };

char const * const latex_quote_ot1[2][5] = 
{ { "\\quotesinglbase{}",  "'", "`", 
    "\\guilsinglleft{}", "\\guilsinglright{}" }, 
  { "\\quotedblbase{}", "''", "``",
    "\\guillemotleft{}", "\\guillemotright{}" } };

char const * const latex_quote_babel[2][5] = 
{ { "\\glq{}",  "'", "`", "\\flq{}", "\\frq{}" },
  { "\\glqq{}", "''", "``", "\\flqq{}", "\\frqq{}" } };

} // namespace anon


InsetQuotes::InsetQuotes(string const & str)
{
	ParseString(str);
}


InsetQuotes::InsetQuotes(InsetQuotes::quote_language l,
			 InsetQuotes::quote_side s,
			 InsetQuotes::quote_times t)
	: language(l), side(s), times(t)
{}


InsetQuotes::InsetQuotes(char c, BufferParams const & params)
	: language(params.quotes_language), times(params.quotes_times)
{
	// Decide whether left or right 
	switch (c) {
	case ' ': case '(': case '{': case '[': case '-': case ':':
	case Paragraph::META_HFILL:
	case Paragraph::META_NEWLINE: 
		side = InsetQuotes::LeftQ;   // left quote 
		break;
	default:
		side = InsetQuotes::RightQ;  // right quote
	}
}


void InsetQuotes::ParseString(string const & s)
{
	string str(s);
	if (str.length() != 3) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad string length." << endl;
		str = "eld";
	}

	int i;
	
	for (i = 0; i < 6; ++i) {
		if (str[0] == language_char[i]) {
			language = InsetQuotes::quote_language(i);
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
			side = InsetQuotes::quote_side(i);
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
			times = InsetQuotes::quote_times(i);
			break;
		}
	}
	if (i >= 2) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad times specification." << endl;
		times = InsetQuotes::DoubleQ; 
	}
}


string const InsetQuotes::DispString() const
{
 	string disp;
	disp += quote_char[quote_index[side][language]];
	if (times == InsetQuotes::DoubleQ)
		disp += disp;

 	if (lyxrc.font_norm_type == LyXRC::ISO_8859_1)
		if (disp == "<<")
			disp = '«';
		else if (disp == ">>")
			disp = '»';
	
 	return disp;
}


int InsetQuotes::ascent(BufferView *, LyXFont const & font) const
{
	return lyxfont::maxAscent(font);
}


int InsetQuotes::descent(BufferView *, LyXFont const & font) const
{
	return lyxfont::maxDescent(font);
}


int InsetQuotes::width(BufferView *, LyXFont const & font) const
{
	string text = DispString();
	int w = 0;

	for (string::size_type i = 0; i < text.length(); ++i) {
		if (text[i] == ' ')
			w += lyxfont::width('i', font);
		else if (i == 0 || text[i] != text[i-1])
			w += lyxfont::width(text[i], font);
		else
			w += lyxfont::width(',', font);
	}

	return w;
}


LyXFont const InsetQuotes::ConvertFont(LyXFont const & f) const
{
	LyXFont font(f);
	// quotes-insets cannot be latex of any kind
	font.setLatex(LyXFont::OFF);
	return font;
}


void InsetQuotes::draw(BufferView * bv, LyXFont const & font,
		       int baseline, float & x, bool) const
{
	string text = DispString();

	bv->painter().text(int(x), baseline, text, font);
	x += width(bv, font);
}


void InsetQuotes::Write(Buffer const *, ostream & os) const
{
	string text;
	text += language_char[language];
	text += side_char[side];
	text += times_char[times]; 
	os << "Quotes " << text;
}


void InsetQuotes::Read(Buffer const *, LyXLex & lex)
{
	lex.nextToken();
	ParseString(lex.GetString());
	lex.next();
	string tmp(lex.GetString());
	if (tmp != "\\end_inset")
		lyxerr << "LyX Warning: Missing \\end_inset "
			"in InsetQuotes::Read." << endl;
}


extern bool use_babel;

int InsetQuotes::Latex(Buffer const * buf, ostream & os,
		       bool /*fragile*/, bool) const
{
	string const doclang = buf->GetLanguage()->lang();
	int quoteind = quote_index[side][language];
	string qstr;
	
	if (lyxrc.fontenc == "T1") {
		qstr = latex_quote_t1[times][quoteind];
#ifdef DO_USE_DEFAULT_LANGUAGE
	} else if (doclang == "default") {
#else
	} else if (!use_babel) {
#endif
		qstr = latex_quote_ot1[times][quoteind];
	} else if (language == InsetQuotes::FrenchQ 
		 && times == InsetQuotes::DoubleQ
		 && doclang == "frenchb") {
		if (side == InsetQuotes::LeftQ) 
			qstr = "\\og{}";
		else 
			qstr = " \\fg{}";
	} else 
		qstr = latex_quote_babel[times][quoteind];

	// Always guard against unfortunate ligatures (!` ?`)
	if (prefixIs(qstr, "`"))
		qstr.insert(0, "{}");

	os << qstr;
	return 0;
}


int InsetQuotes::Ascii(Buffer const *, ostream & os, int) const
{
	os << "\"";
	return 0;
}


int InsetQuotes::Linuxdoc(Buffer const *, ostream & os) const
{
	os << "\"";
	return 0;
}


int InsetQuotes::DocBook(Buffer const *, ostream & os) const
{
	if (times == InsetQuotes::DoubleQ) {
		if (side == InsetQuotes::LeftQ)
			os << "&ldquo;";
		else
			os << "&rdquo;";
	} else {
		if (side == InsetQuotes::LeftQ)
			os << "&lsquo;";
		else
			os << "&rsquo;";
	}
	return 0;
}


void InsetQuotes::Validate(LaTeXFeatures & features) const 
{
	char type = quote_char[quote_index[side][language]];

	if (features.bufferParams().language->lang() == "default" 
	    && lyxrc.fontenc != "T1") {
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


Inset * InsetQuotes::Clone(Buffer const &) const
{
  return new InsetQuotes(language, side, times);
}


Inset::Code InsetQuotes::LyxCode() const
{
  return Inset::QUOTE_CODE;
}
