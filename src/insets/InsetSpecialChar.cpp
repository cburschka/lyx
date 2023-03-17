/**
 * \file InsetSpecialChar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetSpecialChar.h"

#include "Dimension.h"
#include "Encoding.h"
#include "Font.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "output_xhtml.h"
#include "xml.h"
#include "texstream.h"

#include "frontends/FontMetrics.h"
#include "frontends/NullPainter.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"

using namespace std;

namespace lyx {


InsetSpecialChar::InsetSpecialChar(Kind k)
	: Inset(nullptr), kind_(k)
{}


InsetSpecialChar::Kind InsetSpecialChar::kind() const
{
	return kind_;
}


docstring InsetSpecialChar::toolTip(BufferView const &, int, int) const
{
	docstring message;
	switch (kind_) {
		case ALLOWBREAK:
			message = from_ascii("Optional Line Break (ZWSP)");
			break;
		case LIGATURE_BREAK:
			message = from_ascii("Ligature Break (ZWNJ)");
			break;
		case END_OF_SENTENCE:
			message = from_ascii("End of Sentence");
			break;
		case HYPHENATION:
			message = from_ascii("Hyphenation Point");
			break;
		case SLASH:
			message = from_ascii("Breakable Slash");
			break;
		case NOBREAKDASH:
			message = from_ascii("Protected Hyphen (SHY)");
			break;
		case LDOTS:
		case MENU_SEPARATOR:
		case PHRASE_LYX:
		case PHRASE_TEX:
		case PHRASE_LATEX2E:
		case PHRASE_LATEX:
			// no tooltip for these ones.
			break;
	}
	return message;
}


int InsetSpecialChar::rowFlags() const
{
	switch (kind_) {
	case ALLOWBREAK:
	case HYPHENATION:
	case SLASH:
		// these are the elements that allow line breaking
		return CanBreakAfter;
	case NOBREAKDASH:
	case END_OF_SENTENCE:
	case LIGATURE_BREAK:
	case LDOTS:
	case MENU_SEPARATOR:
	case PHRASE_LYX:
	case PHRASE_TEX:
	case PHRASE_LATEX2E:
	case PHRASE_LATEX:
		break;
	}
	return Inline;
}


namespace {

// helper function: draw text and update x.
void drawChar(PainterInfo & pi, int & x, int const y, char_type ch)
{
	FontInfo font = pi.base.font;
	font.setPaintColor(pi.textColor(font.realColor()));
	pi.pain.text(x, y, ch, font);
	x += theFontMetrics(font).width(ch);
}


void drawLogo(PainterInfo & pi, int & x, int const y, InsetSpecialChar::Kind kind)
{
	FontInfo const & font = pi.base.font;
	int const em = theFontMetrics(font).em();
	switch (kind) {
	case InsetSpecialChar::PHRASE_LYX:
		/** Reference macro:
		 *  \providecommand{\LyX}{L\kern-.1667em\lower.25em\hbox{Y}\kern-.125emX\\@};
		 */
		drawChar(pi, x, y, 'L');
		x -= em / 6;
		drawChar(pi, x, y + em / 4, 'Y');
		x -= em / 8;
		drawChar(pi, x, y, 'X');
		break;

	case InsetSpecialChar::PHRASE_TEX: {
		/** Reference macro:
		 *  \def\TeX{T\kern-.1667em\lower.5ex\hbox{E}\kern-.125emX\@}
		 */
		int const ex = theFontMetrics(font).xHeight();
		drawChar(pi, x, y, 'T');
		x -= em / 6;
		drawChar(pi, x, y + ex / 2, 'E');
		x -= em / 8;
		drawChar(pi, x, y, 'X');
		break;
	}
	case InsetSpecialChar::PHRASE_LATEX2E:
		/** Reference macro:
		 *  \DeclareRobustCommand{\LaTeXe}{\mbox{\m@th
		 *    \if b\expandafter\@car\f@series\@nil\boldmath\fi
		 *    \LaTeX\kern.15em2$_{\textstyle\varepsilon}$}}
		 */
		drawLogo(pi, x, y, InsetSpecialChar::PHRASE_LATEX);
		x += 3 * em / 20;
		drawChar(pi, x, y, '2');
		// ε U+03B5 GREEK SMALL LETTER EPSILON
		drawChar(pi, x, y + em / 4, char_type(0x03b5));
		break;

	case InsetSpecialChar::PHRASE_LATEX: {
		/** Reference macro:
		 * \DeclareRobustCommand{\LaTeX}{L\kern-.36em%
		 *        {\sbox\z@ T%
		 *         \vbox to\ht\z@{\hbox{\check@mathfonts
		 *                              \fontsize\sf@size\z@
		 *                              \math@fontsfalse\selectfont
		 *                              A}%
		 *                        \vss}%
		 *        }%
		 *        \kern-.15em%
		 *        \TeX}
		 */
		drawChar(pi, x, y, 'L');
		x -= 9 * em / 25;
		PainterInfo pi2 = pi;
		pi2.base.font.decSize().decSize();
		drawChar(pi2, x, y - em / 5, 'A');
		x -= 3 * em / 20;
		drawLogo(pi, x, y, InsetSpecialChar::PHRASE_TEX);
		break;
	}
	default:
		LYXERR0("No information for drawing logo " << kind);
	}
}

} // namespace


void InsetSpecialChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = 0;
	dim.wid = 0;

	docstring s;
	switch (kind_) {
		case ALLOWBREAK:
			dim.asc = fm.xHeight();
			dim.des = fm.descent('g');
			dim.wid = fm.em() / 8;
			break;
		case LIGATURE_BREAK:
			s = from_ascii("|");
			break;
		case END_OF_SENTENCE:
			s = from_ascii(".");
			break;
		case LDOTS: {
			// see comment in draw().
			auto const fam = mi.base.font.family();
			// Multiplication by 3 is done here to limit rounding effects.
			int const spc3 = fam == TYPEWRITER_FAMILY ? 0 : 3 * fm.width(char_type(' ')) / 2;
			dim.wid = 3 * fm.width(char_type('.')) + spc3;
			break;
		}
		case MENU_SEPARATOR:
			// ▹  U+25B9 WHITE RIGHT-POINTING SMALL TRIANGLE
			// There is a \thinspace on each side of the triangle
			dim.wid = 2 * fm.em() / 6 + fm.width(char_type(0x25B9));
			break;
		case HYPHENATION:
			dim.wid = fm.width(from_ascii("-"));
			if (dim.wid > 5)
				dim.wid -= 2; // to make it look shorter
			break;
		case SLASH:
			s = from_ascii("/");
			dim.des = fm.descent(s[0]);
			break;
		case NOBREAKDASH:
			s = from_ascii("-");
			break;
		case PHRASE_LYX:
		case PHRASE_TEX:
		case PHRASE_LATEX2E:
		case PHRASE_LATEX:
			dim.asc = fm.maxAscent();
			dim.des = fm.maxDescent();
			frontend::NullPainter np;
			PainterInfo pi(mi.base.bv, np);
			pi.base.font = mi.base.font;
			drawLogo(pi, dim.wid, 0, kind_);
			break;
	}
	if (dim.wid == 0)
		dim.wid = fm.width(s);
}


void InsetSpecialChar::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo font = pi.base.font;

	switch (kind_) {
	case HYPHENATION:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('-'), font);
		break;
	}
	case ALLOWBREAK:
	{
		// A small vertical line
		int const asc = theFontMetrics(pi.base.font).xHeight();
		int const desc = theFontMetrics(pi.base.font).descent('g');
		int const x0 = x; // x + 1; // FIXME: incline,
		int const x1 = x; // x - 1; // similar to LibreOffice?
		int const y0 = y + desc;
		int const y1 = y - asc / 3;
		pi.pain.line(x0, y1, x1, y0, Color_special);
		break;
	}
	case LIGATURE_BREAK:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('|'), font);
		break;
	}
	case END_OF_SENTENCE:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('.'), font);
		break;
	}
	case LDOTS:
	{
		font.setColor(Color_special);
		/* \textellipsis uses a \fontdimen3 is spacing. The TeXbook
		 * tells us that \fontdimen3 is the interword stretch, and
		 * that this is usually half a space.
		 */
		frontend::FontMetrics const & fm = theFontMetrics(font);
		auto const fam = pi.base.font.family();
		int const spc = fam == TYPEWRITER_FAMILY ? 0 : fm.width(char_type(' ')) / 2;
		int wid1 = fm.width(char_type('.')) + spc;
		pi.pain.text(x, y, char_type('.'), font);
		pi.pain.text(x + wid1, y, char_type('.'), font);
		pi.pain.text(x + 2 * wid1, y, char_type('.'), font);
		break;
	}
	case MENU_SEPARATOR:
	{
		frontend::FontMetrics const & fm = theFontMetrics(font);

		// There is a \thinspace on each side of the triangle
		x += fm.em() / 6;
		// ▹ U+25B9 WHITE RIGHT-POINTING SMALL TRIANGLE
		// ◃ U+25C3 WHITE LEFT-POINTING SMALL TRIANGLE
		char_type const c = pi.ltr_pos ? 0x25B9 : 0x25C3;
		font.setColor(Color_special);
		pi.pain.text(x, y, c, font);
		break;
	}
	case SLASH:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('/'), font);
		break;
	}
	case NOBREAKDASH:
	{
		font.setColor(Color_latex);
		pi.pain.text(x, y, char_type('-'), font);
		break;
	}
	case PHRASE_LYX:
	case PHRASE_TEX:
	case PHRASE_LATEX2E:
	case PHRASE_LATEX:
		drawLogo(pi, x, y, kind_);
		break;
	}
}


void InsetSpecialChar::write(ostream & os) const
{
	string command;
	switch (kind_) {
	case HYPHENATION:
		command = "softhyphen";
		break;
	case ALLOWBREAK:
		command = "allowbreak";
		break;
	case LIGATURE_BREAK:
		command = "ligaturebreak";
		break;
	case END_OF_SENTENCE:
		command = "endofsentence";
		break;
	case LDOTS:
		command = "ldots";
		break;
	case MENU_SEPARATOR:
		command = "menuseparator";
		break;
	case SLASH:
		command = "breakableslash";
		break;
	case NOBREAKDASH:
		command = "nobreakdash";
		break;
	case PHRASE_LYX:
		command = "LyX";
		break;
	case PHRASE_TEX:
		command = "TeX";
		break;
	case PHRASE_LATEX2E:
		command = "LaTeX2e";
		break;
	case PHRASE_LATEX:
		command = "LaTeX";
		break;
	}
	os << "\\SpecialChar " << command << "\n";
}


void InsetSpecialChar::read(Lexer & lex)
{
	lex.next();
	string const command = lex.getString();

	if (command == "softhyphen")
		kind_ = HYPHENATION;
	else if (command == "allowbreak")
		kind_ = ALLOWBREAK;
	else if (command == "ligaturebreak")
		kind_ = LIGATURE_BREAK;
	else if (command == "endofsentence")
		kind_ = END_OF_SENTENCE;
	else if (command == "ldots")
		kind_ = LDOTS;
	else if (command == "menuseparator")
		kind_ = MENU_SEPARATOR;
	else if (command == "breakableslash")
		kind_ = SLASH;
	else if (command == "nobreakdash")
		kind_ = NOBREAKDASH;
	else if (command == "LyX")
		kind_ = PHRASE_LYX;
	else if (command == "TeX")
		kind_ = PHRASE_TEX;
	else if (command == "LaTeX2e")
		kind_ = PHRASE_LATEX2E;
	else if (command == "LaTeX")
		kind_ = PHRASE_LATEX;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


void InsetSpecialChar::latex(otexstream & os,
			     OutputParams const & rp) const
{
	bool const rtl = rp.local_font->isRightToLeft();
	bool const utf8 = rp.encoding->iconvName() == "UTF-8";
	string lswitch = "";
	string lswitche = "";
	if (rtl && !rp.use_polyglossia) {
		lswitch = "\\L{";
		lswitche = "}";
		if (rp.local_font->language()->lang() == "arabic_arabi"
		    || rp.local_font->language()->lang() == "farsi")
			lswitch = "\\textLR{";
	}

	switch (kind_) {
	case HYPHENATION:
		os << "\\-";
		break;
	case ALLOWBREAK:
		// U+200B not yet supported by utf8 inputenc
		os << "\\LyXZeroWidthSpace" << termcmd;
		break;
	case LIGATURE_BREAK:
		if (utf8)
			// U+200C ZERO WIDTH NON-JOINER
			os.put(0x200c);
		else
			os << "\\textcompwordmark" << termcmd;
		break;
	case END_OF_SENTENCE:
		os << "\\@.";
		break;
	case LDOTS:
		os << "\\ldots" << termcmd;
		break;
	case MENU_SEPARATOR:
		if (rtl)
			os << "\\lyxarrow*";
		else
			os << "\\lyxarrow";
		os << termcmd;
		break;
	case SLASH:
		os << "\\slash" << termcmd;
		break;
	case NOBREAKDASH:
		if (rp.moving_arg)
			os << "\\protect";
		os << "\\nobreakdash-";
		break;
	case PHRASE_LYX:
		if (rp.moving_arg)
			os << "\\protect";
		os << lswitch << "\\LyX" << termcmd << lswitche;
		break;
	case PHRASE_TEX:
		if (rp.moving_arg)
			os << "\\protect";
		os << lswitch << "\\TeX" << termcmd << lswitche;
		break;
	case PHRASE_LATEX2E:
		if (rp.moving_arg)
			os << "\\protect";
		os << lswitch << "\\LaTeXe" << termcmd << lswitche;
		break;
	case PHRASE_LATEX:
		if (rp.moving_arg)
			os << "\\protect";
		os << lswitch << "\\LaTeX" << termcmd << lswitche;
		break;
	}
}


int InsetSpecialChar::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	switch (kind_) {
	case HYPHENATION:
		return 0;
	case ALLOWBREAK:
		// U+200B ZERO WIDTH SPACE (ZWSP)
		os.put(0x200b);
		return 1;
	case LIGATURE_BREAK:
		// U+200C ZERO WIDTH NON-JOINER
		os.put(0x200c);
		return 1;
	case END_OF_SENTENCE:
		os << '.';
		return 1;
	case LDOTS:
		// … U+2026 HORIZONTAL ELLIPSIS
		os.put(0x2026);
		return 1;
	case MENU_SEPARATOR:
		os << "->";
		return 2;
	case SLASH:
		os << '/';
		return 1;
	case NOBREAKDASH:
		// ‑ U+2011 NON-BREAKING HYPHEN
		os.put(0x2011);
		return 1;
	case PHRASE_LYX:
		os << "LyX";
		return 3;
	case PHRASE_TEX:
		os << "TeX";
		return 3;
	case PHRASE_LATEX2E:
		os << "LaTeX2";
		// ε U+03B5 GREEK SMALL LETTER EPSILON
		os.put(0x03b5);
		return 7;
	case PHRASE_LATEX:
		os << "LaTeX";
		return 5;
	}
	return 0;
}


namespace {
string specialCharKindToXMLEntity(InsetSpecialChar::Kind kind) {
	switch (kind) {
	case InsetSpecialChar::Kind::HYPHENATION:
		// Soft hyphen.
		return "&#xAD;";
	case InsetSpecialChar::Kind::ALLOWBREAK:
		// Zero-width space
		return "&#x200B;";
	case InsetSpecialChar::Kind::LIGATURE_BREAK:
		// Zero width non-joiner
		return "&#x200C;";
	case InsetSpecialChar::Kind::END_OF_SENTENCE:
		return ".";
	case InsetSpecialChar::Kind::LDOTS:
		// &hellip;
		return "&#x2026;";
	case InsetSpecialChar::Kind::MENU_SEPARATOR:
		// &rArr;, right arrow.
		return "&#x21D2;";
	case InsetSpecialChar::Kind::SLASH:
		// &frasl;, fractional slash.
		return "&#x2044;";
	case InsetSpecialChar::Kind::NOBREAKDASH:
		// Non-breaking hyphen.
		return "&#x2011;";
	case InsetSpecialChar::Kind::PHRASE_LYX:
		return "LyX";
	case InsetSpecialChar::Kind::PHRASE_TEX:
		return "TeX";
	case InsetSpecialChar::Kind::PHRASE_LATEX2E:
		// Lower-case epsilon.
		return "LaTeX2&#x03b5;";
	case InsetSpecialChar::Kind::PHRASE_LATEX:
		return "LaTeX";
	default:
		return "";
	}
}
}


void InsetSpecialChar::docbook(XMLStream & xs, OutputParams const &) const
{
	xs << XMLStream::ESCAPE_NONE << from_ascii(specialCharKindToXMLEntity(kind_));
}


docstring InsetSpecialChar::xhtml(XMLStream & xs, OutputParams const &) const
{
	xs << XMLStream::ESCAPE_NONE << from_ascii(specialCharKindToXMLEntity(kind_));
	return docstring();
}


void InsetSpecialChar::toString(odocstream & os) const
{
	switch (kind_) {
	case ALLOWBREAK:
	case LIGATURE_BREAK:
		// Do not output ZERO WIDTH SPACE and ZERO WIDTH NON JOINER here
		// Spell checker would choke on it.
		return;
	default:
		break;
	}
	odocstringstream ods;
	plaintext(ods, OutputParams(nullptr));
	os << ods.str();
}


void InsetSpecialChar::forOutliner(docstring & os, size_t const,
								   bool const) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(nullptr));
	os += ods.str();
}


void InsetSpecialChar::validate(LaTeXFeatures & features) const
{
	if (kind_ == ALLOWBREAK)
		features.require("lyxzerowidthspace");
	if (kind_ == MENU_SEPARATOR)
		features.require("lyxarrow");
	if (kind_ == NOBREAKDASH)
		features.require("amsmath");
	if (kind_ == PHRASE_LYX)
		features.require("LyX");
}


bool InsetSpecialChar::isChar() const
{
	return kind_ != HYPHENATION && kind_ != LIGATURE_BREAK;
}


bool InsetSpecialChar::isLetter() const
{
	return kind_ == HYPHENATION || kind_ == LIGATURE_BREAK
		|| kind_ == NOBREAKDASH
		|| kind_ == PHRASE_LYX || kind_ == PHRASE_LATEX
		|| kind_ == PHRASE_TEX || kind_ == PHRASE_LATEX2E;
}


bool InsetSpecialChar::isLineSeparator() const
{
#if 0
	// this would be nice, but it does not work, since
	// Paragraph::stripLeadingSpaces nukes the characters which
	// have this property. I leave the code here, since it should
	// eventually be made to work. (JMarc 20020327)
	return kind_ == HYPHENATION || kind_ == ALLOWBREAK
	    || kind_ == MENU_SEPARATOR || kind_ == SLASH;
#else
	return false;
#endif
}


} // namespace lyx
