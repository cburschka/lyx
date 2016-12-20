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
#include "BufferView.h"
#include "Cursor.h"
#include "Dimension.h"
#include "Font.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "texstream.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstream.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

/* codes used to read/write quotes to LyX files
 * e    ``english''
 * s    ''swedish''
 * g    ,,german``
 * p    ,,polish''
 * f    <<french>>
 * a    >>danish<<
 */

char const * const style_char = "esgpfa";
char const * const side_char = "lr" ;
char const * const level_char = "sd";

// List of known quote chars
char const * const quote_char = ",'`<>";

// Unicode characters needed by each quote type
char_type const display_quote_char[2][5] = {
	{ 0x201a, 0x2019, 0x2018, 0x2039, 0x203a},
	{ 0x201e, 0x201d, 0x201c, 0x00ab, 0x00bb}
};

// Index of chars used for the quote. Index is [side, style]
int quote_index[2][6] = {
	{ 2, 1, 0, 0, 3, 4 },    // "'',,<>"
	{ 1, 1, 2, 1, 4, 3 }     // "`'`'><"
};

// Corresponding LaTeX code, for double and single quotes.
char const * const latex_quote_t1[2][5] = {
	{ "\\quotesinglbase",  "'", "`",
    "\\guilsinglleft", "\\guilsinglright" },
  { ",,", "''", "``", "<<", ">>" }
};

char const * const latex_quote_ot1[2][5] = {
	{ "\\quotesinglbase",  "'", "`",
    "\\guilsinglleft", "\\guilsinglright" },
  { "\\quotedblbase", "''", "``",
    "\\guillemotleft", "\\guillemotright" }
};

char const * const latex_quote_noligatures[2][5] = {
	{ "\\quotesinglbase",  "\\textquoteleft", "\\textquoteright",
    "\\guilsinglleft", "\\guilsinglright" },
  { "\\quotedblbase", "\\textquotedblleft", "\\textquotedblright",
    "\\guillemotleft", "\\guillemotright" }
};

char const * const latex_quote_babel[2][5] = {
	{ "\\glq",  "'", "`", "\\flq", "\\frq" },
  { "\\glqq", "''", "``", "\\flqq", "\\frqq" }
};

char const * const html_quote[2][5] = {
	{ "&sbquo;",  "&rsquo;", "&lsquo;",
	  "&lsaquo;", "&rsaquo;" },
  { "&bdquo;", "&rdquo;", "&ldquo;", "&laquo;", "&raquo;" }
};

} // namespace anon


InsetQuotes::InsetQuotes(Buffer * buf, string const & str) : Inset(buf)
{
	parseString(str);
}


InsetQuotes::InsetQuotes(Buffer * buf, char_type c, QuoteLevel level,
			 string const & side, string const & style)
	: Inset(buf), level_(level), pass_thru_(false)
{
	if (buf) {
		style_ = style.empty() ? buf->params().quotes_style : getStyle(style);
		fontenc_ = (buf->params().fontenc == "global")
			? lyxrc.fontenc : buf->params().fontenc;
	} else {
		style_ = style.empty() ? EnglishQuotes : getStyle(style);
		fontenc_ = lyxrc.fontenc;
	}

	if (side == "left")
		side_ = LeftQuote;
	else if (side == "right")
		side_ = RightQuote;
	else
		setSide(c);
}


docstring InsetQuotes::layoutName() const
{
	return from_ascii("Quotes");
}


void InsetQuotes::setSide(char_type c)
{
	// Decide whether left or right
	switch (c) {
	case ' ':
	case '(':
	case '[':
		side_ = LeftQuote;   // left quote
		break;
	default:
		side_ = RightQuote;  // right quote
	}
}


void InsetQuotes::parseString(string const & s, bool const allow_wildcards)
{
	string str = s;
	if (str.length() != 3) {
		lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
			" bad string length." << endl;
		str = "eld";
	}

	int i;

	// '.' wildcard means: keep current stylee
	if (!allow_wildcards || str[0] != '.') {
		for (i = 0; i < 6; ++i) {
			if (str[0] == style_char[i]) {
				style_ = QuoteStyle(i);
				break;
			}
		}
		if (i >= 6) {
			lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
				" bad style specification." << endl;
			style_ = EnglishQuotes;
		}
	}

	// '.' wildcard means: keep current side
	if (!allow_wildcards || str[1] != '.') {
		for (i = 0; i < 2; ++i) {
			if (str[1] == side_char[i]) {
				side_ = QuoteSide(i);
				break;
			}
		}
		if (i >= 2) {
			lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
				" bad side specification." << endl;
			side_ = LeftQuote;
		}
	}

	// '.' wildcard means: keep current level
	if (!allow_wildcards || str[2] != '.') {
		for (i = 0; i < 2; ++i) {
			if (str[2] == level_char[i]) {
				level_ = QuoteLevel(i);
				break;
			}
		}
		if (i >= 2) {
			lyxerr << "ERROR (InsetQuotes::InsetQuotes):"
				" bad level specification." << endl;
			level_ = DoubleQuotes;
		}
	}
}


InsetQuotes::QuoteStyle InsetQuotes::getStyle(string const & s)
{
	QuoteStyle qs = EnglishQuotes;
	
	if (s == "english")
		qs = EnglishQuotes;
	else if (s == "swedish")
		qs = SwedishQuotes;
	else if (s == "german")
		qs = GermanQuotes;
	else if (s == "polish")
		qs = PolishQuotes;
	else if (s == "french")
		qs = FrenchQuotes;
	else if (s == "danish")
		qs = DanishQuotes;

	return qs;
}


map<string, docstring> InsetQuotes::getTypes() const
{
	map<string, docstring> res;

	int sty, sid, lev;
	QuoteStyle style;
	QuoteSide side;
	QuoteLevel level;
	string type;

	// get all quote types
	for (sty = 0; sty < 6; ++sty) {
		style = QuoteStyle(sty);
		for (sid = 0; sid < 2; ++sid) {
			side = QuoteSide(sid);
			for (lev = 0; lev < 2; ++lev) {
				type += style_char[style];
				type += side_char[sid];
				level = QuoteLevel(lev);
				type += level_char[lev];
				res[type] = docstring(1, display_quote_char[level][quote_index[side][style]]);
				type.clear();
			}
		}
	}
	return res;
}


docstring InsetQuotes::displayString() const
{
	// In PassThru, we use straight quotes
	if (pass_thru_)
		return (level_ == DoubleQuotes) ? from_ascii("\"") : from_ascii("'");

	int const index = quote_index[side_][style_];
	docstring retdisp = docstring(1, display_quote_char[level_][index]);

	// in French, thin spaces are added inside double guillemets
	// FIXME: this should be done by a separate quote type.
	if (prefixIs(context_lang_, "fr")
	    && level_ == DoubleQuotes && style_ == FrenchQuotes) {
		// THIN SPACE (U+2009)
		char_type const thin_space = 0x2009;
		if (side_ == LeftQuote)
			retdisp += thin_space;
		else
			retdisp = thin_space + retdisp;
	}

	return retdisp;
}


void InsetQuotes::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontInfo & font = mi.base.font;
	frontend::FontMetrics const & fm = theFontMetrics(font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();
	dim.wid = fm.width(displayString());
}


void InsetQuotes::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo font = pi.base.font;
	font.setPaintColor(pi.textColor(font.realColor()));
	pi.pain.text(x, y, displayString(), font);
}


string InsetQuotes::getType() const
{
	string text;
	text += style_char[style_];
	text += side_char[side_];
	text += level_char[level_];
	return text;
}


void InsetQuotes::write(ostream & os) const
{
	os << "Quotes " << getType();
}


void InsetQuotes::read(Lexer & lex)
{
	lex.setContext("InsetQuotes::read");
	lex.next();
	parseString(lex.getString());
	lex >> "\\end_inset";
}


void InsetQuotes::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY: {
		string const first_arg = cmd.getArg(0);
		bool const change_type = first_arg == "changetype";
		if (!change_type) {
			// not for us
			// this will not be handled higher up
			cur.undispatched();
			return;
		}
		cur.recordUndoInset(this);
		parseString(cmd.getArg(1), true);
		cur.buffer()->updateBuffer();
		break;
	}
	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetQuotes::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		string const first_arg = cmd.getArg(0);
		if (first_arg == "changetype") {
			string const type = cmd.getArg(1);
			flag.setOnOff(type == getType());
			flag.setEnabled(!pass_thru_);
			return true;
		}
		return Inset::getStatus(cur, cmd, flag);
	}

	default:
		return Inset::getStatus(cur, cmd, flag);
	}
}


void InsetQuotes::latex(otexstream & os, OutputParams const & runparams) const
{
	const int quoteind = quote_index[side_][style_];
	docstring qstr;

	// In pass-thru context, we output plain quotes
	if (runparams.pass_thru)
		qstr = (level_ == DoubleQuotes) ? from_ascii("\"") : from_ascii("'");
	else if (runparams.use_polyglossia) {
		// For polyglossia, we directly output the respective unicode chars 
		// (spacing and kerning is then handled respectively)
		qstr = docstring(1, display_quote_char[level_][quoteind]);
	}
	else if (style_ == FrenchQuotes && level_ == DoubleQuotes
		 && prefixIs(runparams.local_font->language()->code(), "fr")) {
		// Specific guillemets of French babel
		// including correct French spacing
		if (side_ == LeftQuote)
			qstr = from_ascii("\\og");
		else
			qstr = from_ascii("\\fg");
	} else if (fontenc_ == "T1"
		   && !runparams.local_font->language()->internalFontEncoding()) {
		// Quotation marks for T1 font encoding
		// (using ligatures)
		qstr = from_ascii(latex_quote_t1[level_][quoteind]);
	} else if (runparams.local_font->language()->internalFontEncoding()) {
		// Quotation marks for internal font encodings
		// (ligatures not featured)
		qstr = from_ascii(latex_quote_noligatures[level_][quoteind]);
#ifdef DO_USE_DEFAULT_LANGUAGE
	} else if (doclang == "default") {
#else
	} else if (!runparams.use_babel || runparams.isFullUnicode()) {
#endif
		// Standard quotation mark macros
		// These are also used by babel
		// without fontenc (XeTeX/LuaTeX)
		qstr = from_ascii(latex_quote_ot1[level_][quoteind]);
	} else {
		// Babel shorthand quotation marks (for T1/OT1)
		qstr = from_ascii(latex_quote_babel[level_][quoteind]);
	}

	if (!runparams.pass_thru) {
		// Guard against unwanted ligatures with preceding text
		char_type const lastchar = os.lastChar();
		// !` ?` => !{}` ?{}`
		if (prefixIs(qstr, from_ascii("`"))
		    && (lastchar == '!' || lastchar == '?'))
			os << "{}";
		// ``` ''' ,,, <<< >>>
		// => `{}`` '{}'' ,{},, <{}<< >{}>>
		if (contains(from_ascii(",'`<>"), lastchar)
		    && prefixIs(qstr, lastchar))
			os << "{}";
	}

	os << qstr;

	if (prefixIs(qstr, from_ascii("\\")))
		// properly terminate the command depending on the context
		os << termcmd;
}


int InsetQuotes::plaintext(odocstringstream & os, 
        OutputParams const &, size_t) const
{
	docstring const str = displayString();
	os << str;
	return str.size();
}


docstring InsetQuotes::getQuoteEntity() const {
	const int quoteind = quote_index[side_][style_];
	docstring res = from_ascii(html_quote[level_][quoteind]);
	// in French, thin spaces are added inside double guillemets
	// FIXME: this should be done by a separate quote type.
	if (prefixIs(context_lang_, "fr")
	    && level_ == DoubleQuotes && style_ == FrenchQuotes) {
		// THIN SPACE (U+2009)
		docstring const thin_space = from_ascii("&#x2009;");
		if (side_ == LeftQuote)
			res += thin_space;
		else
			res = thin_space + res;
	}
	return res;
}


int InsetQuotes::docbook(odocstream & os, OutputParams const &) const
{
	os << getQuoteEntity();
	return 0;
}


docstring InsetQuotes::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << XHTMLStream::ESCAPE_NONE << getQuoteEntity();
	return docstring();
}


void InsetQuotes::toString(odocstream & os) const
{
	os << displayString();
}


void InsetQuotes::forOutliner(docstring & os, size_t const, bool const) const
{
	os += displayString();
}


void InsetQuotes::updateBuffer(ParIterator const & it, UpdateType /* utype*/)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	pass_thru_ = it.paragraph().isPassThru();
	context_lang_ = it.paragraph().getFontSettings(bp, it.pos()).language()->code();
}


void InsetQuotes::validate(LaTeXFeatures & features) const
{
	char type = quote_char[quote_index[side_][style_]];

#ifdef DO_USE_DEFAULT_LANGUAGE
	if (features.bufferParams().language->lang() == "default"
#else
	if (!features.useBabel()
#endif
	    && !features.usePolyglossia() && fontenc_ != "T1") {
		if (level_ == SingleQuotes)
			switch (type) {
			case ',': features.require("quotesinglbase"); break;
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


string InsetQuotes::contextMenuName() const
{
	return "context-quote";
}

} // namespace lyx
