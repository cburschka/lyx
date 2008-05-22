/**
 * \file InsetQuotes.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetQuotes.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "debug.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lstrings.h"


namespace lyx {

using support::prefixIs;

using std::endl;
using std::string;
using std::auto_ptr;
using std::ostream;


namespace {

/* codes used to read/write quotes to LyX files
 * e    ``english''
 * s    ''spanish''
 * g    ,,german``
 * p    ,,polish''
 * f    <<french>>
 * a    >>danish<<
 */

char const * const language_char = "esgpfa";
char const * const side_char = "lr" ;
char const * const times_char = "sd";

// List of known quote chars
char const * const quote_char = ",'`<>";

// Unicode characters needed by each quote type
char_type const display_quote_char[2][5] = {
	{ 0x201a, 0x2019, 0x2018, 0x2039, 0x203a},
	{ 0x201e, 0x201d, 0x201c, 0x00ab, 0x00bb}
};

// Index of chars used for the quote. Index is [side, language]
int quote_index[2][6] = {
	{ 2, 1, 0, 0, 3, 4 },    // "'',,<>"
	{ 1, 1, 2, 1, 4, 3 } };  // "`'`'><"

// Corresponding LaTeX code, for double and single quotes.
char const * const latex_quote_t1[2][5] =
{ { "\\quotesinglbase ",  "'", "`",
    "\\guilsinglleft{}", "\\guilsinglright{}" },
  { ",,", "''", "``", "<<", ">>" } };

char const * const latex_quote_ot1[2][5] =
{ { "\\quotesinglbase ",  "'", "`",
    "\\guilsinglleft{}", "\\guilsinglright{}" },
  { "\\quotedblbase ", "''", "``",
    "\\guillemotleft{}", "\\guillemotright{}" } };

char const * const latex_quote_babel[2][5] =
{ { "\\glq ",  "'", "`", "\\flq{}", "\\frq{}" },
  { "\\glqq ", "''", "``", "\\flqq{}", "\\frqq{}" } };

} // namespace anon


InsetQuotes::InsetQuotes(string const & str)
{
	parseString(str);
}


InsetQuotes::InsetQuotes(quote_language l, quote_side s, quote_times t)
	: language_(l), side_(s), times_(t)
{
}


InsetQuotes::InsetQuotes(char_type c, BufferParams const & params)
	: language_(params.quotes_language), times_(params.quotes_times)
{
	getPosition(c);
}


InsetQuotes::InsetQuotes(char_type c, quote_language l, quote_times t)
	: language_(l), times_(t)
{
	getPosition(c);
}


void InsetQuotes::getPosition(char_type c)
{
	// Decide whether left or right
	switch (c) {
	case ' ': case '(': case '[':
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


docstring const InsetQuotes::dispString(Language const * loclang) const
{
	int const index = quote_index[side_][language_];
	docstring retdisp = docstring(1, display_quote_char[times_][index]);

	// in french, spaces are added inside double quotes
	if (times_ == DoubleQ && prefixIs(loclang->code(), "fr")) {
		if (side_ == LeftQ)
			retdisp += ' ';
		else
			retdisp.insert(docstring::size_type(0), 1, ' ');
	}

	return retdisp;
}


bool InsetQuotes::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font & font = mi.base.font;
	frontend::FontMetrics const & fm =
		theFontMetrics(font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();
	dim.wid = 0;

	docstring const text = dispString(font.language());
	for (string::size_type i = 0; i < text.length(); ++i) {
		if (text[i] == ' ')
			dim.wid += fm.width('i');
		else if (i == 0 || text[i] != text[i - 1])
			dim.wid += fm.width(text[i]);
		else
			dim.wid += fm.width(',');
	}
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


#if 0
Font const InsetQuotes::convertFont(Font const & f) const
{
#if 1
	return f;
#else
	Font font(f);
	return font;
#endif
}
#endif


void InsetQuotes::draw(PainterInfo & pi, int x, int y) const
{
	docstring const text = dispString(pi.base.font.language());

	if (text.length() == 2 && text[0] == text[1]) {
		pi.pain.text(x, y, text[0], pi.base.font);
		int const t = theFontMetrics(pi.base.font)
			.width(',');
		pi.pain.text(x + t, y, text[0], pi.base.font);
	} else {
		pi.pain.text(x, y, text, pi.base.font);
	}
	setPosCache(pi, x, y);
}


void InsetQuotes::write(Buffer const &, ostream & os) const
{
	string text;
	text += language_char[language_];
	text += side_char[side_];
	text += times_char[times_];
	os << "Quotes " << text;
}


void InsetQuotes::read(Buffer const &, Lexer & lex)
{
	lex.next();
	parseString(lex.getString());
	lex.next();
	if (lex.getString() != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point");
	}
}


int InsetQuotes::latex(Buffer const &, odocstream & os,
		       OutputParams const & runparams) const
{
	const int quoteind = quote_index[side_][language_];
	string qstr;

	if (language_ == FrenchQ && times_ == DoubleQ
	    && prefixIs(runparams.local_font->language()->code(), "fr")) {
		if (side_ == LeftQ)
			qstr = "\\og "; //the spaces are important here
		else
			qstr = " \\fg{}"; //and here
	} else if (lyxrc.fontenc == "T1") {
		qstr = latex_quote_t1[times_][quoteind];
#ifdef DO_USE_DEFAULT_LANGUAGE
	} else if (doclang == "default") {
#else
	} else if (!runparams.use_babel) {
#endif
		qstr = latex_quote_ot1[times_][quoteind];
	} else {
		qstr = latex_quote_babel[times_][quoteind];
	}

	// Always guard against unfortunate ligatures (!` ?`)
	if (prefixIs(qstr, "`"))
		qstr.insert(0, "{}");

	os << from_ascii(qstr);
	return 0;
}


int InsetQuotes::plaintext(Buffer const & buf, odocstream & os,
			   OutputParams const &) const
{
	docstring const str = dispString(buf.params().language);
	os << str;
	return str.size();
}


int InsetQuotes::docbook(Buffer const &, odocstream & os,
			 OutputParams const &) const
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


void InsetQuotes::textString(Buffer const & buf, odocstream & os) const
{
	os << dispString(buf.params().language);
}


void InsetQuotes::validate(LaTeXFeatures & features) const
{
	bool const use_babel = features.useBabel();
	char type = quote_char[quote_index[side_][language_]];

#ifdef DO_USE_DEFAULT_LANGUAGE
	if (features.bufferParams().language->lang() == "default"
#else
	if (!use_babel
#endif
	    && lyxrc.fontenc != "T1") {
		if (times_ == SingleQ)
			switch (type) {
				case ',': features.require("quotesinglbase");  break;
			case '<': features.require("guilsinglleft");  break;
			case '>': features.require("guilsinglright"); break;
			default: break;
			}
		else
			switch (type) {
			case ',': features.require("quotedblbase");   break;
			case '<': features.require("guillemotleft");  break;
			case '>': features.require("guillemotright"); break;
			default: break;
			}
	}
}


auto_ptr<Inset> InsetQuotes::doClone() const
{
	return auto_ptr<Inset>(new InsetQuotes(language_, side_, times_));
}


Inset::Code InsetQuotes::lyxCode() const
{
	return Inset::QUOTE_CODE;
}


} // namespace lyx
