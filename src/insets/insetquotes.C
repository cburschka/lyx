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
	parseString(str);
}


InsetQuotes::InsetQuotes(quote_language l,
			 quote_side s,
			 quote_times t)
	: language_(l), side_(s), times_(t)
{}


InsetQuotes::InsetQuotes(char c, BufferParams const & params)
	: language_(params.quotes_language), times_(params.quotes_times)
{
	// Decide whether left or right 
	switch (c) {
	case ' ': case '(': 
	case Paragraph::META_HFILL:
	case Paragraph::META_NEWLINE: 
		side_ = LeftQ;   // left quote 
		break;
	default:
		side_ = RightQ;  // right quote
	}
}


void InsetQuotes::parseString(string const & s)
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
			language_ = quote_language(i);
			break;
		}
	}
	if (i >= 6) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad language specification." << endl;
		language_ = EnglishQ; 
	}

	for (i = 0; i < 2; ++i) {
		if (str[1] == side_char[i]) {
			side_ = quote_side(i);
			break;
		}
	}
	if (i >= 2) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad side specification." << endl;
		side_ = LeftQ; 
	}

	for (i = 0; i < 2; ++i) {
		if (str[2] == times_char[i]) {
			times_ = quote_times(i);
			break;
		}
	}
	if (i >= 2) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad times specification." << endl;
		times_ = DoubleQ; 
	}
}


string const InsetQuotes::dispString(Language const * loclang) const
{
 	string disp;
	disp += quote_char[quote_index[side_][language_]];
	if (times_ == DoubleQ)
		disp += disp;

 	if (lyxrc.font_norm_type == LyXRC::ISO_8859_1
 	    || lyxrc.font_norm_type == LyXRC::ISO_8859_3
 	    || lyxrc.font_norm_type == LyXRC::ISO_8859_4
 	    || lyxrc.font_norm_type == LyXRC::ISO_8859_9) {
		if (disp == "'")
			disp = "´";
		else if (disp == "''")
			disp = "´´";
	}
 	if (lyxrc.font_norm_type == LyXRC::ISO_8859_1
 	    || lyxrc.font_norm_type == LyXRC::ISO_8859_9
 	    || lyxrc.font_norm_type == LyXRC::ISO_8859_15) {
		if (disp == "<<")
			disp = '«';
		else if (disp == ">>")
			disp = '»';
	}

	// in french, spaces are added inside double quotes
	if (times_ == DoubleQ && prefixIs(loclang->code(), "fr")) {
		if (side_ == LeftQ)
			disp += " ";
		else
			disp = " " + disp;
	}
	
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
	string const text = dispString(font.language());
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


#if 0
LyXFont const InsetQuotes::convertFont(LyXFont const & f) const
{
#if 1
	return f;
#else
	LyXFont font(f);
	return font;
#endif
}
#endif


void InsetQuotes::draw(BufferView * bv, LyXFont const & font,
		       int baseline, float & x, bool) const
{
	string const text = dispString(font.language());

	bv->painter().text(int(x), baseline, text, font);
	x += width(bv, font);
}


void InsetQuotes::write(Buffer const *, ostream & os) const
{
	string text;
	text += language_char[language_];
	text += side_char[side_];
	text += times_char[times_]; 
	os << "Quotes " << text;
}


void InsetQuotes::read(Buffer const *, LyXLex & lex)
{
	lex.nextToken();
	parseString(lex.getString());
	lex.next();
	if (lex.getString() != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point");
	}
}


extern bool use_babel;

int InsetQuotes::latex(Buffer const * buf, ostream & os,
		       bool /*fragile*/, bool) const
{
	// How do we get the local language here??

	string const doclang = buf->getLanguage()->babel();
	const int quoteind = quote_index[side_][language_];
	string qstr;
	
	if (language_ == FrenchQ && times_ == DoubleQ
	    && doclang == "frenchb") {
		if (side_ == LeftQ) 
			qstr = "\\og "; //the spaces are important here
		else 
			qstr = " \\fg{}"; //and here
	} else if (language_ == FrenchQ && times_ == DoubleQ
		   && doclang == "french") {
		if (side_ == LeftQ) 
			qstr = "<< "; //the spaces are important here
		else 
			qstr = " >>"; //and here
	} else if (lyxrc.fontenc == "T1") {
		qstr = latex_quote_t1[times_][quoteind];
#ifdef DO_USE_DEFAULT_LANGUAGE
	} else if (doclang == "default") {
#else
	} else if (!use_babel) {
#endif
		qstr = latex_quote_ot1[times_][quoteind];
	} else { 
		qstr = latex_quote_babel[times_][quoteind];
	}

	// Always guard against unfortunate ligatures (!` ?`)
	if (prefixIs(qstr, "`"))
		qstr.insert(0, "{}");

	os << qstr;
	return 0;
}


int InsetQuotes::ascii(Buffer const *, ostream & os, int) const
{
	os << "\"";
	return 0;
}


int InsetQuotes::linuxdoc(Buffer const *, ostream & os) const
{
	os << "\"";
	return 0;
}


int InsetQuotes::docbook(Buffer const *, ostream & os) const
{
	if (times_ == DoubleQ) {
		if (side_ == LeftQ)
			os << "&ldquo;";
		else
			os << "&rdquo;";
	} else {
		if (side_ == LeftQ)
			os << "&lsquo;";
		else
			os << "&rsquo;";
	}
	return 0;
}


void InsetQuotes::validate(LaTeXFeatures & features) const 
{
	char type = quote_char[quote_index[side_][language_]];

#ifdef DO_USE_DEFAULT_LANGUAGE
	if (features.bufferParams().language->lang() == "default" 
#else
	if (!use_babel
#endif
	    && lyxrc.fontenc != "T1") {
		if (times_ == SingleQ) 
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


Inset * InsetQuotes::clone(Buffer const &, bool) const
{
  return new InsetQuotes(language_, side_, times_);
}


Inset::Code InsetQuotes::lyxCode() const
{
  return Inset::QUOTE_CODE;
}
