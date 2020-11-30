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
#include "Encoding.h"
#include "Font.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "texstream.h"
#include "xml.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

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
 * c    <<swiss>>    (<inner quotation>)
 * a    >>danish<<   (>inner quotation<)
 * q    "plain"      ('inner quotation')
 * b    `british'    (``inner quotation'')
 * w    >>swedishg>> ('inner quotation') ["g" = Guillemets]
 * f    <<french>>   (``inner quotation'')
 * i    <<frenchin>> (<<inner quotation>>) ["in" = Imprimerie Nationale]
 * r    <<russian>>  (,,inner quotation``)
 * j    [U+300C]cjk[U+300D]  ([U+300E]inner quotation[U+300F]) [CORNER BRACKETS]
 * k    [U+300A]cjkangle[U+300B]  ([U+3008]inner quotation[U+3009]) [ANGLE BRACKETS]
 * x    dynamic style (inherits document settings)
 */

char const * const style_char = "esgpcaqbwfirjkx";
char const * const side_char = "lr" ;
char const * const level_char = "sd";

} // namespace


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


char InsetQuotesParams::getStyleChar(QuoteStyle const & style) const
{
	return style_char[static_cast<int>(style)];
}


QuoteStyle InsetQuotesParams::getQuoteStyle(string const & s,
			    bool const allow_wildcards, QuoteStyle fb) const
{
	QuoteStyle res = fb;

	string str = s;
	if (str.length() != 3) {
		LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
			" bad string length.");
		str = "eld";
	}

	// '.' wildcard means: keep current style
	if (!allow_wildcards || str[0] != '.') {
		int i;
		for (i = 0; i < stylescount(); ++i) {
			if (str[0] == style_char[i]) {
				res = QuoteStyle(i);
				break;
			}
		}
		if (i >= stylescount()) {
			LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
				" bad style specification.");
			res = QuoteStyle::English;
		}
	}

	return res;
}


QuoteSide InsetQuotesParams::getQuoteSide(string const & s,
			bool const allow_wildcards, QuoteSide fb) const
{
	QuoteSide res = fb;

	string str = s;
	if (str.length() != 3) {
		LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
			" bad string length.");
		str = "eld";
	}

	// '.' wildcard means: keep current side
	if (!allow_wildcards || str[1] != '.') {
		int i;
		for (i = 0; i < 2; ++i) {
			if (str[1] == side_char[i]) {
				res = QuoteSide(i);
				break;
			}
		}
		if (i >= 2) {
			LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
				" bad side specification.");
			res = QuoteSide::Opening;
		}
	}

	return res;
}


QuoteLevel InsetQuotesParams::getQuoteLevel(string const & s,
			bool const allow_wildcards, QuoteLevel fb) const
{
	QuoteLevel res = fb;

	string str = s;
	if (str.length() != 3) {
		LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
			" bad string length.");
		str = "eld";
	}

	// '.' wildcard means: keep current level
	if (!allow_wildcards || str[2] != '.') {
		int i;
		for (i = 0; i < 2; ++i) {
			if (str[2] == level_char[i]) {
				res = QuoteLevel(i);
				break;
			}
		}
		if (i >= 2) {
			LYXERR0("ERROR (InsetQuotes::InsetQuotes):"
				" bad level specification.");
			res = QuoteLevel::Primary;
		}
	}

	return res;
}


char_type InsetQuotesParams::getQuoteChar(QuoteStyle const & style, QuoteLevel const & level,
				    QuoteSide const & side, bool const rtl) const
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
	case QuoteStyle::English: {
		left_primary = 0x201c; // ``
		right_primary = 0x201d; // ''
		left_secondary = 0x2018; // `
		right_secondary = 0x2019; // '
		break;
	}
	case QuoteStyle::Swedish: {
		left_primary = 0x201d; // ''
		right_primary = 0x201d; // ''
		left_secondary = 0x2019; // '
		right_secondary = 0x2019; // '
		break;
	}
	case QuoteStyle::German: {
		left_primary = 0x201e; // ,,
		right_primary = 0x201c; // ``
		left_secondary = 0x201a; // ,
		right_secondary = 0x2018; // `
		break;
	}
	case QuoteStyle::Polish: {
		left_primary =  0x201e; // ,,
		right_primary = 0x201d; // ''
		left_secondary = 0x201a; // ,
		right_secondary = 0x2019; // '
		break;
	}
	case QuoteStyle::Swiss: {
		left_primary = 0x00ab; // <<
		right_primary = 0x00bb; // >>
		left_secondary = 0x2039; // <
		right_secondary = 0x203a; // >
		break;
	}
	case QuoteStyle::Danish: {
		left_primary = 0x00bb; // >>
		right_primary = 0x00ab; // <<
		left_secondary = 0x203a; // >
		right_secondary = 0x2039; // <
		break;
	}
	case QuoteStyle::Plain: {
		left_primary = 0x0022; // "
		right_primary = 0x0022; // "
		left_secondary = 0x0027; // '
		right_secondary = 0x0027; // '
		break;
	}
	case QuoteStyle::British: {
		left_primary = 0x2018; // `
		right_primary = 0x2019; // '
		left_secondary = 0x201c; // ``
		right_secondary = 0x201d; // ''
		break;
	}
	case QuoteStyle::SwedishG: {
		left_primary = 0x00bb; // >>
		right_primary = 0x00bb; // >>
		left_secondary = 0x2019; // '
		right_secondary = 0x2019; // '
		break;
	}
	case QuoteStyle::French: {
		left_primary = 0x00ab; // <<
		right_primary = 0x00bb; // >>
		left_secondary = 0x201c; // ``
		right_secondary = 0x201d; // ''
		break;
	}
	case QuoteStyle::FrenchIN:{
		left_primary = 0x00ab; // <<
		right_primary = 0x00bb; // >>
		left_secondary =  0x00ab; // <<
		right_secondary = 0x00bb; // >>
		break;
	}
	case QuoteStyle::Russian:{
		left_primary = 0x00ab; // <<
		right_primary = 0x00bb; // >>
		left_secondary =  0x201e; // ,,
		right_secondary = 0x201c; // ``
		break;
	}
	case QuoteStyle::CJK:{
		left_primary = 0x300c; // LEFT CORNER BRACKET
		right_primary = 0x300d; // RIGHT CORNER BRACKET
		left_secondary =  0x300e; // LEFT WHITE CORNER BRACKET
		right_secondary = 0x300f; // RIGHT WHITE CORNER BRACKET
		break;
	}
	case QuoteStyle::CJKAngle:{
		left_primary = 0x300a; // LEFT DOUBLE ANGLE BRACKET
		right_primary = 0x300b; // RIGHT DOUBLE ANGLE BRACKET
		left_secondary =  0x3008; // LEFT ANGLE BRACKET
		right_secondary = 0x3009; // RIGHT ANGLE BRACKET
		break;
	}
	case QuoteStyle::Dynamic:
	default:
		// should not happen
		left_primary = 0x003f; // ?
		right_primary = 0x003f; // ?
		left_secondary =  0x003f; // ?
		right_secondary = 0x003f; // ?
		break;
	}

	switch (level) {
	case QuoteLevel::Secondary:
		if (rtl)
			return (side == QuoteSide::Closing) ? left_secondary : right_secondary;
		return (side == QuoteSide::Opening) ? left_secondary : right_secondary;
	case QuoteLevel::Primary:
		if (rtl)
			return (side == QuoteSide::Closing) ? left_primary : right_primary;
		return (side == QuoteSide::Opening) ? left_primary : right_primary;
	default:
		break;
	}

	// should not happen
	return 0x003f;
}


docstring InsetQuotesParams::getLaTeXQuote(char_type c, string const & op,
					   bool const rtl) const
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
			// This macro is redefined in rtl mode
			res = rtl ? "\\textquoteleft" : "\\textquoteright";
		else
			res = "'";
		break;
	}
	case 0x2018: {// `
		if (op == "int")
			// This macro is redefined in rtl mode
			res = rtl ? "\\textquoteright" : "\\textquoteleft";
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
			// This macro is redefined in rtl mode
			res = rtl ? "\\textquotedblleft" : "\\textquotedblright";
		else
			res = "''";
		break;
	}
	case 0x201c: {// ``
		if (op == "int")
			// This macro is redefined in rtl mode
			res = rtl ? "\\textquotedblright" : "\\textquotedblleft";
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
	// The following are fakes
	// This is just to get something symbolic
	// in encodings where this chars would not be used anyway
	case 0x300c: // LEFT CORNER BRACKET
		res = "\\ensuremath{\\lceil}";
		break;
	case 0x300d: // RIGHT CORNER BRACKET
		res = "\\ensuremath{\\rfloor}";
		break;
	case 0x300e: // LEFT WHITE CORNER BRACKET
		res = "\\ensuremath{\\llceil}";
		break;
	case 0x300f: // RIGHT WHITE CORNER BRACKET
		res = "\\ensuremath{\\rrfloor}";
		break;
	case 0x300a: // LEFT DOUBLE ANGLE BRACKET
		res = "\\ensuremath{\\langle\\kern-2.5pt\\langle}";
		break;
	case 0x300b: // RIGHT DOUBLE ANGLE BRACKET
		res = "\\ensuremath{\\rangle\\kern-2.5pt\\rangle}";
		break;
	case 0x3008: // LEFT ANGLE BRACKET
		res = "\\ensuremath{\\langle}";
		break;
	case 0x3009: // RIGHT ANGLE BRACKET
		res = "\\ensuremath{\\rangle}";
		break;
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
	case 0x300c: // LEFT CORNER BRACKET
		res = "&#x300c;";
		break;
	case 0x300d: // RIGHT CORNER BRACKET
		res = "&#x300d;";
		break;
	case 0x300e: // LEFT WHITE CORNER BRACKET
		res = "&#x300e;";
		break;
	case 0x300f: // RIGHT WHITE CORNER BRACKET
		res = "&#x300f;";
		break;
	case 0x300a: // LEFT DOUBLE ANGLE BRACKET
		res = "&#x300a;";
		break;
	case 0x300b: // RIGHT DOUBLE ANGLE BRACKET
		res = "&#x300b;";
		break;
	case 0x3008: // LEFT ANGLE BRACKET
		res = "&#x3008;";
		break;
	case 0x3009: // RIGHT ANGLE BRACKET
		res = "&#x3009;";
		break;
	default:
		break;
	}

	return from_ascii(res);
}


docstring InsetQuotesParams::getXMLQuote(char_type c) const
{
	// Directly output the character Unicode form.
	return from_ascii("&#" + to_string(c) + ";");
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
		if (style == QuoteStyle::Dynamic)
			continue;
		for (sid = 0; sid < 2; ++sid) {
			side = QuoteSide(sid);
			for (lev = 0; lev < 2; ++lev) {
				type += style_char[static_cast<int>(style)];
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


docstring const InsetQuotesParams::getGuiLabel(QuoteStyle const & qs, bool langdef) const
{
	docstring const styledesc =
		bformat(_("%1$souter%2$s and %3$sinner%4$s[[quotation marks]]"),
		    	docstring(1, getQuoteChar(qs, QuoteLevel::Primary, QuoteSide::Opening)),
			docstring(1, getQuoteChar(qs, QuoteLevel::Primary, QuoteSide::Closing)),
			docstring(1, getQuoteChar(qs, QuoteLevel::Secondary, QuoteSide::Opening)),
			docstring(1, getQuoteChar(qs, QuoteLevel::Secondary, QuoteSide::Closing))
			);

	if (!langdef)
		return styledesc;

	return bformat(_("%1$s[[quot. mark description]] (language default)"),
			styledesc);
}


docstring const InsetQuotesParams::getShortGuiLabel(docstring const & str) const
{
	string const s = to_ascii(str);
	QuoteStyle const style = getQuoteStyle(s);
	QuoteSide const side = getQuoteSide(s);
	QuoteLevel const level = getQuoteLevel(s);

	return (side == QuoteSide::Opening) ?
		bformat(_("%1$stext"),
		       docstring(1, getQuoteChar(style, level, side))) :
		bformat(_("text%1$s"),
		       docstring(1, getQuoteChar(style, level, side)));
}


/////////////////////////////////////////////////////////////////////
//
// InsetQuotes
//
///////////////////////////////////////////////////////////////////////

InsetQuotes::InsetQuotes(Buffer * buf, string const & str)
	: Inset(buf)
{
	if (buf) {
		global_style_ = buf->masterBuffer()->params().quotes_style;
		fontspec_ = buf->masterBuffer()->params().useNonTeXFonts;
	}

	parseString(str);
}


InsetQuotes::InsetQuotes(Buffer * buf, char_type c, QuoteLevel level,
			 string const & side, string const & style)
	: Inset(buf), level_(level)
{
	bool dynamic = false;
	if (buf) {
		global_style_ = buf->masterBuffer()->params().quotes_style;
		fontenc_ = buf->masterBuffer()->params().main_font_encoding();
		dynamic = buf->masterBuffer()->params().dynamic_quotes;
		fontspec_ = buf->masterBuffer()->params().useNonTeXFonts;
	} else {
		fontenc_ = "OT1";
	}
	if (style.empty())
		style_ = dynamic ? QuoteStyle::Dynamic : global_style_;
	else
		style_ = getStyle(style);

	if (side == "left" || side == "opening")
		side_ = QuoteSide::Opening;
	else if (side == "right" || side == "closing")
		side_ = QuoteSide::Closing;
	else
		setSide(c);
}


docstring InsetQuotes::layoutName() const
{
	return from_ascii("Quotes");
}


void InsetQuotes::setSide(char_type c)
{
	// Decide whether opening or closing quote
	if (lyx::isSpace(c) || isOpenPunctuation(c))
		side_ = QuoteSide::Opening;// opening quote
	else
		side_ = QuoteSide::Closing;// closing quote
}


void InsetQuotes::parseString(string const & s, bool const allow_wildcards)
{
	style_ = quoteparams.getQuoteStyle(s, allow_wildcards, style_);
	side_ = quoteparams.getQuoteSide(s, allow_wildcards, side_);
	level_ = quoteparams.getQuoteLevel(s, allow_wildcards, level_);
}


QuoteStyle InsetQuotes::getStyle(string const & s)
{
	QuoteStyle qs = QuoteStyle::English;

	if (s == "english")
		qs = QuoteStyle::English;
	else if (s == "swedish")
		qs = QuoteStyle::Swedish;
	else if (s == "german")
		qs = QuoteStyle::German;
	else if (s == "polish")
		qs = QuoteStyle::Polish;
	else if (s == "swiss")
		qs = QuoteStyle::Swiss;
	else if (s == "danish")
		qs = QuoteStyle::Danish;
	else if (s == "plain")
		qs = QuoteStyle::Plain;
	else if (s == "british")
		qs = QuoteStyle::British;
	else if (s == "swedishg")
		qs = QuoteStyle::SwedishG;
	else if (s == "french")
		qs = QuoteStyle::French;
	else if (s == "frenchin")
		qs = QuoteStyle::FrenchIN;
	else if (s == "russian")
		qs = QuoteStyle::Russian;
	else if (s == "cjk")
		qs = QuoteStyle::CJK;
	else if (s == "cjkangle")
		qs = QuoteStyle::CJKAngle;
	else if (s == "dynamic")
		qs = QuoteStyle::Dynamic;

	return qs;
}


docstring InsetQuotes::displayString() const
{
	// In PassThru, we use straight quotes
	if (pass_thru_)
		return (level_ == QuoteLevel::Primary) ?
					from_ascii("\"") : from_ascii("'");

	QuoteStyle style =
			(style_ == QuoteStyle::Dynamic) ? global_style_ : style_;

	docstring retdisp = docstring(1, quoteparams.getQuoteChar(style, level_, side_, rtl_));

	// in French, thin spaces are added inside double guillemets
	if (prefixIs(context_lang_, "fr")
	    && level_ == QuoteLevel::Primary
	    && (style == QuoteStyle::Swiss
		|| style == QuoteStyle::French
		|| style == QuoteStyle::FrenchIN)) {
		// THIN SPACE (U+2009)
		char_type const thin_space = 0x2009;
		if (side_ == QuoteSide::Opening)
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
	if (style_ == QuoteStyle::Dynamic)
		font.setPaintColor(Color_special);
	else
		font.setPaintColor(pi.textColor(font.realColor()));
	pi.pain.text(x, y, displayString(), font);
}


string InsetQuotes::getType() const
{
	string text;
	text += style_char[static_cast<int>(style_)];
	text += side_char[static_cast<int>(side_)];
	text += level_char[static_cast<int>(level_)];
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
		cur.forceBufferUpdate();
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
	QuoteStyle style =
			(style_ == QuoteStyle::Dynamic) ? global_style_ : style_;
	char_type quotechar = quoteparams.getQuoteChar(style, level_, side_, rtl_);
	docstring qstr;

	// In pass-thru context, we output plain quotes
	if (runparams.pass_thru)
		qstr = (level_ == QuoteLevel::Primary) ? from_ascii("\"") : from_ascii("'");
	else if (style == QuoteStyle::Plain && fontspec_) {
		// For XeTeX and LuaTeX,we need to disable mapping to get straight
		// quotes. We define our own commands that do this
		qstr = (level_ == QuoteLevel::Primary) ?
			from_ascii("\\textquotedblplain") : from_ascii("\\textquotesingleplain");
	}
	else if (runparams.use_polyglossia) {
		// For polyglossia, we directly output the respective unicode chars
		// (spacing and kerning is then handled respectively)
		qstr = docstring(1, quotechar);
	}
	// The CJK marks are not yet covered by utf8 inputenc (we don't have the entry in
	// unicodesymbols, since we don't want to add fake synbols there).
	else if (style == QuoteStyle::CJK || style  == QuoteStyle::CJKAngle) {
		if (runparams.encoding && runparams.encoding->name() != "utf8"
		    && runparams.encoding->encodable(quotechar))
			qstr = docstring(1, quotechar);
		else
			qstr = quoteparams.getLaTeXQuote(quotechar, "int");
	}
	else if ((style == QuoteStyle::Swiss
		 || style == QuoteStyle::French
		 || style == QuoteStyle::FrenchIN)
		 && level_ == QuoteLevel::Primary
		 && prefixIs(runparams.local_font->language()->code(), "fr")) {
		// Specific guillemets of French babel
		// including correct French spacing
		if (side_ == QuoteSide::Opening)
			qstr = from_ascii("\\og");
		else
			qstr = from_ascii("\\fg");
	} else if (runparams.use_hyperref && runparams.moving_arg) {
		// Use internal commands in headings with hyperref
		// (ligatures not featured in PDF strings)
		qstr = quoteparams.getLaTeXQuote(quotechar, "int", rtl_);
	} else if (fontenc_ == "T1"
		   && !runparams.local_font->language()->internalFontEncoding()) {
		// Quotation marks for T1 font encoding
		// (using ligatures)
		qstr = quoteparams.getLaTeXQuote(quotechar, "t1");
	} else if (runparams.local_font->language()->internalFontEncoding()) {
		// Quotation marks for internal font encodings
		// (ligatures not featured)
		qstr = quoteparams.getLaTeXQuote(quotechar, "int", rtl_);
#ifdef DO_USE_DEFAULT_LANGUAGE
	} else if ((doclang == "default"
#else
	} else if ((!runparams.use_babel
#endif
		   || (fontenc_ != "T1" && fontenc_ != "OT1"))
		   || runparams.isFullUnicode()) {
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
		// LuaTeX does not respect {} as ligature breaker by design,
		// see https://tex.stackexchange.com/q/349725/19291
		docstring const nolig =
				(runparams.flavor == Flavor::LuaTeX
				 || runparams.flavor == Flavor::DviLuaTeX) ?
					from_ascii("\\/") : from_ascii("{}");
		// !` ?` => !{}` ?{}`
		if (prefixIs(qstr, from_ascii("`"))
		    && (lastchar == '!' || lastchar == '?'))
			os << nolig;
		// ``` ''' ,,, <<< >>>
		// => `{}`` '{}'' ,{},, <{}<< >{}>>
		if (contains(from_ascii(",'`<>"), lastchar)
		    && prefixIs(qstr, lastchar))
			os << nolig;
	}

	os << qstr;

	if (prefixIs(qstr, from_ascii("\\")) && !suffixIs(qstr, '}'))
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


docstring InsetQuotes::getQuoteEntity(bool isHTML) const {
	QuoteStyle style =
			(style_ == QuoteStyle::Dynamic) ? global_style_ : style_;
	docstring res = isHTML ? quoteparams.getHTMLQuote(quoteparams.getQuoteChar(style, level_, side_)) :
					quoteparams.getXMLQuote(quoteparams.getQuoteChar(style, level_, side_));

	// in French, thin spaces are added inside double guillemets
	if (prefixIs(context_lang_, "fr")
	    && level_ == QuoteLevel::Primary
	    && (style == QuoteStyle::French
		|| style == QuoteStyle::FrenchIN
		|| style == QuoteStyle::Swiss)) {
		// THIN SPACE (U+2009)
		docstring const thin_space = from_ascii("&#x2009;");
		if (side_ == QuoteSide::Opening) // Open quote: space after
			res += thin_space;
		else // Close quote: space before
			res = thin_space + res;
	}
	return res;
}


void InsetQuotes::docbook(XMLStream & xs, OutputParams const &) const
{
	xs << XMLStream::ESCAPE_NONE << getQuoteEntity(false);
}


docstring InsetQuotes::xhtml(XMLStream & xs, OutputParams const &) const
{
	xs << XMLStream::ESCAPE_NONE << getQuoteEntity(true);
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


void InsetQuotes::updateBuffer(ParIterator const & it, UpdateType /* utype*/, bool const /*deleted*/)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	Font const & font = it.paragraph().getFontSettings(bp, it.pos());
	pass_thru_ = it.paragraph().isPassThru();
	context_lang_ = font.language()->code();
	internal_fontenc_ = font.language()->internalFontEncoding();
	fontenc_ = bp.main_font_encoding();
	global_style_ = bp.quotes_style;
	fontspec_ = bp.useNonTeXFonts;
	rtl_ = font.isRightToLeft();
}


void InsetQuotes::validate(LaTeXFeatures & features) const
{
	QuoteStyle style =
			(style_ == QuoteStyle::Dynamic) ? global_style_ : style_;
	char_type type = quoteparams.getQuoteChar(style, level_, side_);

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
		if (features.runparams().isFullUnicode() && fontspec_)
			features.require("textquotesinglep");
		else
			features.require("textcomp");
		break;
	}
	case 0x0022: {
		if (features.runparams().isFullUnicode() && fontspec_)
			features.require("textquotedblp");
		else if (fontenc_ != "T1" || internal_fontenc_)
			features.require("textquotedbl");
		break;
	}
	// we fake these from math (also for utf8 inputenc
	// currently; see above)
	case 0x300e: // LEFT WHITE CORNER BRACKET
	case 0x300f: // RIGHT WHITE CORNER BRACKET
		if (!features.runparams().encoding
		    || features.runparams().encoding->name() == "utf8"
		    || !features.runparams().encoding->encodable(type))
			features.require("stmaryrd");
		break;
	default:
		break;
	}
}


string InsetQuotes::contextMenuName() const
{
	return "context-quote";
}

} // namespace lyx
