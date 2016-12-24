/**
 * \file InsetQuotes.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Jürgen Spitzmüller
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
#include "support/gettext.h"
#include "support/lstrings.h"

#include <string.h>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

/* codes used to read/write quotes to LyX files
 * available styles:
 * e    ``english''  (`inner quotation')
 * s    ''swedish''  ('inner quotation')
 * g    ,,german``   (,inner quotation`)
 * p    ,,polish''   (,inner quotation')
 * f    <<french>>   (<inner quotation>)
 * a    >>danish<<   (>inner quotation<)
 * q    "plain"      ('inner quotation')
 */

char const * const style_char = "esgpfaq";
char const * const side_char = "lr" ;
char const * const level_char = "sd";

} // namespace anon


/////////////////////////////////////////////////////////////////////
//
// InsetQuotesParams
//
///////////////////////////////////////////////////////////////////////

InsetQuotesParams quoteparams;


int InsetQuotesParams::stylescount() const
{
	return strlen(style_char);
}


char_type InsetQuotesParams::getQuoteChar(QuoteStyle const & style, QuoteLevel const & level,
				    QuoteSide const & side) const
{
	// main opening quotation mark
	char_type left_primary;
	// main closing quotation mark
	char_type right_primary;
	// secondary (inner, 'single') opening quotation mark
	char_type left_secondary;
	// secondary (inner, 'single') closing quotation mark
	char_type right_secondary;

	switch (style) {
	case EnglishQuotes: {
		left_primary = 0x201c; // ``
		right_primary = 0x201d; // ''
		left_secondary = 0x2018; // `
		right_secondary = 0x2019; // '
		break;
	}
	case SwedishQuotes: {
		left_primary = 0x201d; // ''
		right_primary = 0x201d; // ''
		left_secondary = 0x2019; // '
		right_secondary = 0x2019; // '
		break;
	}
	case GermanQuotes: {
		left_primary = 0x201e; // ,,
		right_primary = 0x201c; // ``
		left_secondary = 0x201a; // ,
		right_secondary = 0x2018; // `
		break;
	}
	case PolishQuotes: {
		left_primary =  0x201e; // ,,
		right_primary = 0x201d; // ''
		left_secondary = 0x201a; // ,
		right_secondary = 0x2019; // '
		break;
	}
	case FrenchQuotes: {
		left_primary = 0x00ab; // <<
		right_primary = 0x00bb; // >>
		left_secondary = 0x2039; // <
		right_secondary = 0x203a; // >
		break;
	}
	case DanishQuotes: {
		left_primary = 0x00bb; // >>
		right_primary = 0x00ab; // <<
		left_secondary = 0x203a; // >
		right_secondary = 0x2039; // <
		break;
	}
	case PlainQuotes: {
		left_primary = 0x0022; // "
		right_primary = 0x0022; // "
		left_secondary = 0x0027; // '
		right_secondary = 0x0027; // '
		break;
	}
	default:
		// should not happen
		left_primary = 0x003f; // ?
		right_primary = 0x003f; // ?
		left_secondary =  0x003f; // ?
		right_secondary = 0x003f; // ?
		break;
	}

	switch (level) {
	case SingleQuotes:
		return (side == LeftQuote) ? left_secondary : right_secondary;
	case DoubleQuotes:
		return (side == LeftQuote) ? left_primary : right_primary;
	default:
		break;
	}

	// should not happen
	return 0x003f;
}


docstring InsetQuotesParams::getLaTeXQuote(char_type c, string const & op) const
{
	string res;

	switch (c){
	case 0x201a: {// ,
		if (op == "babel")
			res = "\\glq";
		else
			res = "\\quotesinglbase";
		break;
	}
	case 0x2019: {// '
		if (op == "int")
			res = "\\textquoteleft";
		else
			res = "'";
		break;
	}
	case 0x2018: {// `
		if (op == "int")
			res = "\\textquoteright";
		else
			res = "`";
		break;
	}
	case 0x2039: {// <
		if (op == "babel")
			res = "\\flq";
		else
			res = "\\guilsinglleft";
		break;
	}
	case 0x203a: {// >
		if (op == "babel")
			res = "\\frq";
		else
			res = "\\guilsinglright";
		break;
	}
	case 0x0027: {// ' (plain)
		res = "\\textquotesingle";
		break;
	}
	case 0x201e: {// ,,
		if (op == "t1")
			res = ",,";
		else if (op == "babel")
			res = "\\glqq";
		else
			res = "\\quotedblbase";
		break;
	}
	case 0x201d: {// ''
		if (op == "int")
			res = "\\textquotedblleft";
		else
			res = "''";
		break;
	}
	case 0x201c: {// ``
		if (op == "int")
			res = "\\textquotedblright";
		else
			res = "``";
		break;
	}
	case 0x00ab: {// <<
		if (op == "t1")
			res = "<<";
		else if (op == "babel")
			res = "\\flqq";
		else
			res = "\\guillemotleft";
		break;
	}
	case 0x00bb: {// >>
		if (op == "t1")
			res = ">>";
		else if (op == "babel")
			res = "\\frqq";
		else
			res = "\\guillemotright";
		break;
	}
	case 0x0022: {// "
		res = "\\textquotedbl";
		break;
	}
	default:
		break;
	}
	
	return from_ascii(res);
}


docstring InsetQuotesParams::getHTMLQuote(char_type c) const
{
	string res;

	switch (c){
	case 0x201a: // ,
		res = "&sbquo;";
		break;
	case 0x2019: // '
		res = "&rsquo;";
		break;
	case 0x2018: // `
		res = "&lsquo;";
		break;
	case 0x2039: // <
		res = "&lsaquo;";
		break;
	case 0x203a: // >
		res = "&rsaquo;";
		break;
	case 0x0027: // ' (plain)
		res = "&#x27;";
		break;
	case 0x201e: // ,,
		res = "&bdquo;";
		break;
	case 0x201d: // ''
		res = "&rdquo;";
		break;
	case 0x201c: // ``
		res = "&ldquo;";
		break;
	case 0x00ab: // <<
		res = "&laquo;";
		break;
	case 0x00bb: // >>
		res = "&raquo;";
		break;
	case 0x0022: // "
		res = "&quot;";
		break;
	default:
		break;
	}
	
	return from_ascii(res);
}


map<string, docstring> InsetQuotesParams::getTypes() const
{
	map<string, docstring> res;

	int sty, sid, lev;
	QuoteStyle style;
	QuoteSide side;
	QuoteLevel level;
	string type;

	// get all quote types
	for (sty = 0; sty < stylescount(); ++sty) {
		style = QuoteStyle(sty);
		for (sid = 0; sid < 2; ++sid) {
			side = QuoteSide(sid);
			for (lev = 0; lev < 2; ++lev) {
				type += style_char[style];
				type += side_char[sid];
				level = QuoteLevel(lev);
				type += level_char[lev];
				res[type] = docstring(1, getQuoteChar(style, level, side));
				type.clear();
			}
		}
	}
	return res;
}


docstring const InsetQuotesParams::getGuiLabel(QuoteStyle const & qs)
{
	return bformat(_("%1$souter%2$s and %3$sinner%4$s[[quotation marks]]"),
		docstring(1, quoteparams.getQuoteChar(qs, DoubleQuotes, LeftQuote)),
		docstring(1, quoteparams.getQuoteChar(qs, DoubleQuotes, RightQuote)),
		docstring(1, quoteparams.getQuoteChar(qs, SingleQuotes, LeftQuote)),
		docstring(1, quoteparams.getQuoteChar(qs, SingleQuotes, RightQuote))
		);
}


/////////////////////////////////////////////////////////////////////
//
// InsetQuotes
//
///////////////////////////////////////////////////////////////////////

InsetQuotes::InsetQuotes(Buffer * buf, string const & str) : Inset(buf)
{
	parseString(str);
}


InsetQuotes::InsetQuotes(Buffer * buf, char_type c, InsetQuotesParams::QuoteLevel level,
			 string const & side, string const & style)
	: Inset(buf), level_(level), pass_thru_(false)
{
	if (buf) {
		style_ = style.empty() ? buf->params().quotes_style : getStyle(style);
		fontenc_ = (buf->params().fontenc == "global")
			? lyxrc.fontenc : buf->params().fontenc;
	} else {
		style_ = style.empty() ? InsetQuotesParams::EnglishQuotes : getStyle(style);
		fontenc_ = lyxrc.fontenc;
	}

	if (side == "left")
		side_ = InsetQuotesParams::LeftQuote;
	else if (side == "right")
		side_ = InsetQuotesParams::RightQuote;
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
		side_ = InsetQuotesParams::LeftQuote;   // left quote
		break;
	default:
		side_ = InsetQuotesParams::RightQuote;  // right quote
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
		for (i = 0; i < quoteparams.stylescount(); ++i) {
			if (str[0] == style_char[i]) {
				style_ = InsetQuotesParams::QuoteStyle(i);
				break;
			}
		}
		if (i >= quoteparams.stylescount()) {
			LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
				" bad style specification.");
			style_ = InsetQuotesParams::EnglishQuotes;
		}
	}

	// '.' wildcard means: keep current side
	if (!allow_wildcards || str[1] != '.') {
		for (i = 0; i < 2; ++i) {
			if (str[1] == side_char[i]) {
				side_ = InsetQuotesParams::QuoteSide(i);
				break;
			}
		}
		if (i >= 2) {
			LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
				" bad side specification.");
			side_ = InsetQuotesParams::LeftQuote;
		}
	}

	// '.' wildcard means: keep current level
	if (!allow_wildcards || str[2] != '.') {
		for (i = 0; i < 2; ++i) {
			if (str[2] == level_char[i]) {
				level_ = InsetQuotesParams::QuoteLevel(i);
				break;
			}
		}
		if (i >= 2) {
			LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
				" bad level specification.");
			level_ = InsetQuotesParams::DoubleQuotes;
		}
	}
}


InsetQuotesParams::QuoteStyle InsetQuotes::getStyle(string const & s)
{
	InsetQuotesParams::QuoteStyle qs = InsetQuotesParams::EnglishQuotes;
	
	if (s == "english")
		qs = InsetQuotesParams::EnglishQuotes;
	else if (s == "swedish")
		qs = InsetQuotesParams::SwedishQuotes;
	else if (s == "german")
		qs = InsetQuotesParams::GermanQuotes;
	else if (s == "polish")
		qs = InsetQuotesParams::PolishQuotes;
	else if (s == "french")
		qs = InsetQuotesParams::FrenchQuotes;
	else if (s == "danish")
		qs = InsetQuotesParams::DanishQuotes;
	else if (s == "plain")
		qs = InsetQuotesParams::PlainQuotes;

	return qs;
}


docstring InsetQuotes::displayString() const
{
	// In PassThru, we use straight quotes
	if (pass_thru_)
		return (level_ == InsetQuotesParams::DoubleQuotes) ?
					from_ascii("\"") : from_ascii("'");

	docstring retdisp = docstring(1, quoteparams.getQuoteChar(style_, level_, side_));

	// in French, thin spaces are added inside double guillemets
	if (prefixIs(context_lang_, "fr")
	    && level_ == InsetQuotesParams::DoubleQuotes
	    && style_ == InsetQuotesParams::FrenchQuotes) {
		// THIN SPACE (U+2009)
		char_type const thin_space = 0x2009;
		if (side_ == InsetQuotesParams::LeftQuote)
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
	char_type quotechar = quoteparams.getQuoteChar(style_, level_, side_);
	docstring qstr;

	// In pass-thru context, we output plain quotes
	if (runparams.pass_thru)
		qstr = (level_ == InsetQuotesParams::DoubleQuotes) ? from_ascii("\"") : from_ascii("'");
	else if (style_ == InsetQuotesParams::PlainQuotes && runparams.isFullUnicode()) {
		// For XeTeX and LuaTeX,we need to disable mapping to get straight
		// quotes. We define our own commands that do this
		qstr = (level_ == InsetQuotesParams::DoubleQuotes) ?
			from_ascii("\\textquotedblplain") : from_ascii("\\textquotesingleplain");
	}
	else if (runparams.use_polyglossia) {
		// For polyglossia, we directly output the respective unicode chars 
		// (spacing and kerning is then handled respectively)
		qstr = docstring(1, quotechar);
	}
	else if (style_ == InsetQuotesParams::FrenchQuotes
		 && level_ == InsetQuotesParams::DoubleQuotes
		 && prefixIs(runparams.local_font->language()->code(), "fr")) {
		// Specific guillemets of French babel
		// including correct French spacing
		if (side_ == InsetQuotesParams::LeftQuote)
			qstr = from_ascii("\\og");
		else
			qstr = from_ascii("\\fg");
	} else if (fontenc_ == "T1"
		   && !runparams.local_font->language()->internalFontEncoding()) {
		// Quotation marks for T1 font encoding
		// (using ligatures)
		qstr = quoteparams.getLaTeXQuote(quotechar, "t1");
	} else if (runparams.local_font->language()->internalFontEncoding()) {
		// Quotation marks for internal font encodings
		// (ligatures not featured)
		qstr = quoteparams.getLaTeXQuote(quotechar, "int");
#ifdef DO_USE_DEFAULT_LANGUAGE
	} else if (doclang == "default") {
#else
	} else if (!runparams.use_babel || runparams.isFullUnicode()) {
#endif
		// Standard quotation mark macros
		// These are also used by babel
		// without fontenc (XeTeX/LuaTeX)
		qstr = quoteparams.getLaTeXQuote(quotechar, "ot1");
	} else {
		// Babel shorthand quotation marks (for T1/OT1)
		qstr = quoteparams.getLaTeXQuote(quotechar, "babel");
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
	docstring res = quoteparams.getHTMLQuote(quoteparams.getQuoteChar(style_, level_, side_));
	// in French, thin spaces are added inside double guillemets
	if (prefixIs(context_lang_, "fr")
	    && level_ == InsetQuotesParams::DoubleQuotes
	    && style_ == InsetQuotesParams::FrenchQuotes) {
		// THIN SPACE (U+2009)
		docstring const thin_space = from_ascii("&#x2009;");
		if (side_ == InsetQuotesParams::LeftQuote)
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
	fontenc_ = (bp.fontenc == "global") ? lyxrc.fontenc : bp.fontenc;
}


void InsetQuotes::validate(LaTeXFeatures & features) const
{
	char_type type = quoteparams.getQuoteChar(style_, level_, side_);

	// Handle characters that are not natively supported by
	// specific font encodings (we roll our own definitions)
#ifdef DO_USE_DEFAULT_LANGUAGE
	if (features.bufferParams().language->lang() == "default"
#else
	if (!features.useBabel()
#endif
	    && !features.runparams().isFullUnicode() && fontenc_ != "T1") {
		switch (type) {
		case 0x201a:
			features.require("quotesinglbase");
			break;
		case 0x2039:
			features.require("guilsinglleft");
			break;
		case 0x203a:
			features.require("guilsinglright");
			break;
		case 0x201e:
			features.require("quotedblbase");
			break;
		case 0x00ab:
			features.require("guillemotleft");
			break;
		case 0x00bb:
			features.require("guillemotright");
			break;
		default:
			break;
		}
	}
	// Handle straight quotation marks. These need special care
	// in most output formats
	switch (type) {
	case 0x0027: {
		if (features.runparams().isFullUnicode())
				features.require("textquotesinglep");
			else
				features.require("textcomp");
			break;
	}
	case 0x0022: {
		if (features.runparams().isFullUnicode())
			features.require("textquotedblp");
		else if (fontenc_ != "T1")
			features.require("textquotedbl");
		break;
	}
	default:
		break;
	}
}


string InsetQuotes::contextMenuName() const
{
	return "context-quote";
}

} // namespace lyx
